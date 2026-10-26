"""Auto-training pipeline: validate -> build .lstmf -> fine-tune -> evaluate (CER) -> deploy only if better."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from datetime import datetime, timedelta, timezone
from pathlib import Path

from PIL import Image
from sqlalchemy import func, select, update
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import Session

from ..config import settings
from ..models import ModelVersion, TrainingRun, TrainingSample
from . import appsettings, modelstore
from .ocr import RUNTIME, run_tesseract

LOG_TAIL = 2000


class Rejected(Exception):
    """The run finished normally but the candidate model must not be deployed."""


def edit_distance(a: str, b: str) -> int:
    prev = list(range(len(b) + 1))
    for i, ca in enumerate(a, 1):
        cur = [i]
        for j, cb in enumerate(b, 1):
            cur.append(min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + (ca != cb)))
        prev = cur
    return prev[-1]


def cer(pairs) -> float:
    """pairs: [(reference, hypothesis)] -> character error rate, ignoring whitespace."""
    err = tot = 0
    for ref, hyp in pairs:
        ref, hyp = "".join(ref.split()), "".join(hyp.split())
        err += edit_distance(ref, hyp)
        tot += max(len(ref), 1)
    return err / tot if tot else 1.0


def _run(cmd, cwd=None, timeout=3600):
    r = subprocess.run([str(c) for c in cmd], capture_output=True, text=True, cwd=cwd, timeout=timeout)
    if r.returncode != 0:
        raise RuntimeError(f"{Path(str(cmd[0])).name} failed: {(r.stderr or r.stdout)[-LOG_TAIL:]}")
    return r.stdout + r.stderr


def make_lstmf(sample: TrainingSample, out_dir: Path, lang: str, tessdata: Path) -> Path:
    """Write image + ground truth + line box for one sample and have Tesseract emit its .lstmf."""
    stem = out_dir / f"s{sample.id}"
    with Image.open(sample.image_path) as im:
        im = im.convert("L")
        w, h = im.size
        im.save(f"{stem}.png")
    Path(f"{stem}.gt.txt").write_text(sample.text + "\n", encoding="utf-8")
    Path(f"{stem}.box").write_text(f"WordStr 0 0 {w} {h} 0 #{sample.text}\n\t 0 0 {w} {h} 0\n", encoding="utf-8")
    _run([settings.tesseract_bin, f"{stem}.png", stem, "--psm", "13", "--tessdata-dir", tessdata, "-l", lang,
          "lstm.train"], timeout=120)
    return Path(f"{stem}.lstmf")


def evaluate(test: list, lang: str, tessdata: Path) -> float:
    pairs = []
    for s in test:
        try:
            hyp = run_tesseract(s.image_path, lang, psm=7, tessdata=tessdata)
        except Exception:  # noqa: BLE001 - count a failed read as total miss
            hyp = ""
        pairs.append((s.text, hyp))
    return cer(pairs)


def train_language(db: Session, run: TrainingRun) -> ModelVersion:
    lang, cfg = run.lang, appsettings.get_all(db)
    base = settings.base_tessdata / f"{lang}.traineddata"
    if not base.exists():
        raise RuntimeError(f"Base model for '{lang}' not found at {base}")

    approved = db.scalars(select(TrainingSample).where(TrainingSample.lang == lang, TrainingSample.status == "approved")).all()
    train = [s for s in approved if not s.is_test and Path(s.image_path).exists()]
    test = [s for s in approved if s.is_test and Path(s.image_path).exists()]
    if not train:
        raise Rejected("No approved training samples")
    if len(test) < cfg["retrain_min_test_samples"]:
        raise Rejected(f"Only {len(test)} held-out test samples; at least {cfg['retrain_min_test_samples']} are needed "
                       "to judge a model fairly. Approve more samples.")

    work = settings.storage_dir / "train_work" / f"run_{run.id}"
    shutil.rmtree(work, ignore_errors=True)
    (work / "data").mkdir(parents=True)
    log = []
    try:
        lstmfs = [make_lstmf(s, work / "data", lang, settings.base_tessdata) for s in train]
        listfile = work / "train.list"
        listfile.write_text("\n".join(str(p) for p in lstmfs) + "\n")

        _run([settings.combine_tessdata_bin, "-e", base, work / "base.lstm"])
        out = work / "out"
        out.mkdir()
        log.append(_run([settings.lstmtraining_bin, "--model_output", out / "ckpt", "--continue_from", work / "base.lstm",
                         "--traineddata", base, "--train_listfile", listfile, "--max_iterations",
                         cfg["retrain_max_iterations"], "--debug_interval", 0, "--learning_rate", 0.0001]))
        cand_dir = work / "candidate"
        cand_dir.mkdir()
        _run([settings.lstmtraining_bin, "--stop_training", "--continue_from", out / "ckpt_checkpoint",
              "--traineddata", base, "--model_output", cand_dir / f"{lang}.traineddata"])

        # Evaluate the active model and the candidate on the same held-out lines.
        old_cer = evaluate(test, lang, RUNTIME)
        new_cer = evaluate(test, lang, cand_dir)
    except Exception:
        shutil.rmtree(work, ignore_errors=True)
        raise

    n = modelstore.next_version_number(db, lang)
    dest = modelstore.version_dir(lang, n) / f"{lang}.traineddata"
    shutil.copy(cand_dir / f"{lang}.traineddata", dest)
    improved = new_cer < old_cer - float(cfg["min_improvement"])
    mv = ModelVersion(lang=lang, version=n, path=str(dest), trigger=run.trigger, cer=new_cer, baseline_cer=old_cer,
                      train_samples=len(train), test_samples=len(test), status="candidate",
                      notes=f"CER {old_cer:.3f} -> {new_cer:.3f} on {len(test)} held-out lines")
    db.add(mv)
    db.flush()
    for s in train:
        s.used_in_version_id = mv.id
    db.commit()
    shutil.rmtree(work, ignore_errors=True)
    if improved:
        modelstore.activate(db, mv)
    else:
        mv.status = "rejected"
        mv.notes += " - not deployed: no improvement over the active model"
        db.commit()
    return mv


def execute_run(db: Session, run_id: int):
    claimed = db.execute(update(TrainingRun).where(TrainingRun.id == run_id, TrainingRun.status == "queued")
                         .values(status="running", started_at=datetime.now(timezone.utc)))
    db.commit()
    if claimed.rowcount != 1:  # another worker already took this run
        return
    run = db.get(TrainingRun, run_id)
    try:
        mv = train_language(db, run)
        run.version_id = mv.id
        run.status = "deployed" if mv.status == "active" else "rejected"
        run.message = mv.notes
    except Rejected as e:
        run.status, run.message = "rejected", str(e)
    except Exception as e:  # noqa: BLE001
        run.status, run.message = "failed", str(e)[-LOG_TAIL:]
    run.finished_at = datetime.now(timezone.utc)
    db.commit()


def enqueue_run(db: Session, lang: str, trigger: str, user_id=None) -> TrainingRun:
    busy = db.scalar(select(TrainingRun).where(TrainingRun.lang == lang, TrainingRun.status.in_(("queued", "running"))))
    if busy:
        return busy
    run = TrainingRun(lang=lang, trigger=trigger, requested_by=user_id)
    db.add(run)
    try:
        db.commit()
    except IntegrityError:  # lost a race with another request: reuse the run that won
        db.rollback()
        return db.scalar(select(TrainingRun).where(TrainingRun.lang == lang, TrainingRun.status.in_(("queued", "running"))))
    return run


def due_runs(db: Session):
    """Languages that should retrain now: enough new approved samples, or the schedule elapsed with any new sample."""
    cfg = appsettings.get_all(db)
    if not cfg["auto_retrain_enabled"]:
        return []
    out = []
    for lang in settings.trainable_list:
        new = db.scalar(select(func.count()).select_from(TrainingSample).where(
            TrainingSample.lang == lang, TrainingSample.status == "approved", TrainingSample.is_test.is_(False),
            TrainingSample.used_in_version_id.is_(None))) or 0
        if not new:
            continue
        if new >= cfg["retrain_threshold"]:
            out.append((lang, "auto"))
            continue
        last = db.scalar(select(func.max(TrainingRun.created_at)).where(TrainingRun.lang == lang))
        if last is None or last < datetime.now(timezone.utc) - timedelta(hours=cfg["retrain_schedule_hours"]):
            out.append((lang, "schedule"))
    return out
