"""On-demand fine-tuning of a dataset's own model, with accuracy before vs after.

Reuses the building blocks of the shared training pipeline (services/training.py): .lstmf generation, lstmtraining, CER. Differences:
  * the data is ONE user's dataset (no admin approval step), and the model belongs to that dataset only;
  * the split is fixed and visible: every 5th sample (by id) is held out, so the same lines are never trained on and scored;
  * "before" is the model OCR uses today; "after" is the fine-tuned candidate, both scored on the held-out lines.
"""
from __future__ import annotations

import os
import shutil
from datetime import datetime, timezone
from pathlib import Path
from typing import List

from sqlalchemy import func, select, update
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import Dataset, DatasetModel, DatasetSample
from ..services import modelstore, training
from ..worker.dispatch import dispatch

MIN_SAMPLES = int(os.environ.get("LAB_MIN_SAMPLES", 10))   # below this a held-out score means nothing
HOLD_OUT_EVERY = 5                                          # 1 in 5 samples is held out for the accuracy numbers
MIN_TEST = 2


def split(samples: List[DatasetSample]):
    ordered = sorted(samples, key=lambda s: s.id)
    test = [s for i, s in enumerate(ordered) if i % HOLD_OUT_EVERY == 0]
    train = [s for i, s in enumerate(ordered) if i % HOLD_OUT_EVERY != 0]
    return train, test


def requirements(ds: Dataset) -> dict:
    n = len([s for s in ds.samples if Path(s.image_path).exists()])
    train, test = split(ds.samples)
    return {"samples": n, "minimum": MIN_SAMPLES, "ready": n >= MIN_SAMPLES and len(test) >= MIN_TEST, "train_samples": len(train), "test_samples": len(test)}


MAX_ACTIVE_PER_USER = 2   # trainings a user may have queued or running at once (the worker runs them one at a time)


def start(db: Session, ds: Dataset) -> DatasetModel:
    busy = db.scalar(select(func.count()).select_from(DatasetModel).join(Dataset, Dataset.id == DatasetModel.dataset_id)
                     .where(Dataset.user_id == ds.user_id, DatasetModel.status.in_(("queued", "training"))))
    if busy >= MAX_ACTIVE_PER_USER:
        raise ApiError(429, "too_many_trainings", f"You already have {busy} trainings in progress. Wait for one to finish.")
    req = requirements(ds)
    if not req["ready"]:
        raise ApiError(422, "not_enough_samples", f"Training needs at least {MIN_SAMPLES} samples (you have {req['samples']}). "
                                                   "A few of them are held out to measure accuracy honestly.")
    nxt = (db.scalar(select(func.max(DatasetModel.version)).where(DatasetModel.dataset_id == ds.id)) or 0) + 1
    m = DatasetModel(dataset_id=ds.id, version=nxt, status="queued", train_samples=req["train_samples"], test_samples=req["test_samples"])
    db.add(m)
    try:
        db.commit()
    except IntegrityError:  # the partial unique index: one run in flight per dataset
        db.rollback()
        raise ApiError(409, "training_in_progress", "This dataset is already being trained")
    dispatch("train_dataset_model", m.id)
    return m


def tessdata_for(db: Session, ds: Dataset) -> "Path | None":
    """A tessdata directory in which the dataset's active model replaces the base language file (everything else is linked)."""
    m = next((x for x in ds.models if x.active and x.path and Path(x.path).exists()), None)
    if not m:
        return None
    d = Path(m.path).parent / "tessdata"
    d.mkdir(exist_ok=True)
    for f in modelstore.RUNTIME.iterdir():
        link = d / f.name
        if f.name != f"{ds.lang}.traineddata" and not link.exists():
            link.symlink_to(f.resolve())
    tuned = d / f"{ds.lang}.traineddata"
    if not tuned.exists():
        tuned.symlink_to(Path(m.path).resolve())
    return d


def activate(db: Session, ds: Dataset, m: DatasetModel, on: bool) -> None:
    if on and m.status not in ("ready", "no_gain"):
        raise ApiError(409, "model_not_ready", "Only a finished model can be activated")
    if on:
        for other in ds.models:
            other.active = False
    m.active = on
    db.commit()


def execute(db: Session, model_id: int) -> None:
    """Run in a Celery worker. Never raises: failures are recorded on the model row."""
    claimed = db.execute(update(DatasetModel).where(DatasetModel.id == model_id, DatasetModel.status == "queued").values(status="training"))
    db.commit()
    if claimed.rowcount != 1:
        return
    m = db.get(DatasetModel, model_id)
    ds = db.get(Dataset, m.dataset_id)
    work = settings.storage_dir / "train_work" / f"ds{ds.id}_v{m.version}"
    try:
        samples = [s for s in ds.samples if Path(s.image_path).exists()]
        train, test = split(samples)
        if len(samples) < MIN_SAMPLES or len(test) < MIN_TEST:
            raise training.Rejected(f"Only {len(samples)} usable samples: at least {MIN_SAMPLES} are needed")
        base = settings.base_tessdata / f"{ds.lang}.traineddata"
        if not base.exists():
            raise RuntimeError(f"Base model for '{ds.lang}' not found at {base}")
        iterations = max(300, min(3000, 60 * len(train)))
        shutil.rmtree(work, ignore_errors=True)
        (work / "data").mkdir(parents=True)
        lstmfs = [training.make_lstmf(s, work / "data", ds.lang, settings.base_tessdata) for s in train]
        (work / "train.list").write_text("\n".join(str(p) for p in lstmfs) + "\n")
        training._run([settings.combine_tessdata_bin, "-e", base, work / "base.lstm"])
        (work / "out").mkdir()
        training._run([settings.lstmtraining_bin, "--model_output", work / "out" / "ckpt", "--continue_from", work / "base.lstm", "--traineddata", base,
                       "--train_listfile", work / "train.list", "--max_iterations", iterations, "--debug_interval", 0, "--learning_rate", 0.0001])
        cand = work / "candidate"
        cand.mkdir()
        training._run([settings.lstmtraining_bin, "--stop_training", "--continue_from", work / "out" / "ckpt_checkpoint", "--traineddata", base,
                       "--model_output", cand / f"{ds.lang}.traineddata"])
        before = training.evaluate(test, ds.lang, modelstore.RUNTIME)   # what OCR gives today
        after = training.evaluate(test, ds.lang, cand)
        dest_dir = settings.storage_dir / "dataset_models" / str(ds.id) / f"v{m.version}"
        dest_dir.mkdir(parents=True, exist_ok=True)
        dest = dest_dir / f"{ds.lang}.traineddata"
        shutil.copy(cand / f"{ds.lang}.traineddata", dest)
        improved = after < before
        m.path, m.cer_before, m.cer_after, m.iterations, m.train_samples, m.test_samples = str(dest), before, after, iterations, len(train), len(test)
        m.status = "ready" if improved else "no_gain"
        m.message = (f"Error rate on {len(test)} held-out lines: {before * 100:.1f}% before, {after * 100:.1f}% after"
                     + ("" if improved else ". No improvement, so this model is not switched on. Add more or more varied samples and retrain."))
        if improved:
            for other in ds.models:
                other.active = False
            m.active = True
    except training.Rejected as e:
        m.status, m.message = "failed", str(e)
    except Exception as e:  # noqa: BLE001
        m.status, m.message = "failed", f"Training failed: {str(e)[-600:]}"
    finally:
        shutil.rmtree(work, ignore_errors=True)
    m.finished_at = datetime.now(timezone.utc)
    db.commit()
