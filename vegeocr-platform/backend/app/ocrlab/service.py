"""Lab runs: validate an upload, store it, execute it (in a Celery worker), keep the before/after images, and serve the results."""
from __future__ import annotations

import json
import shutil
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

import cv2
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import Dataset, LabRun, User
from ..services.files import validate_upload
from ..services.jobs import ALLOWED_LANGS
from ..worker.dispatch import dispatch
from . import datasets, pipeline as P, train

MAX_UPLOAD = 15 * 1024 * 1024
MAX_ACTIVE_PER_USER = 3
KEEP_RUNS = 30
STALE_AFTER_SECONDS = 15 * 60  # a run still "queued/processing" after this is assumed lost (worker killed, out of memory...)


def _plain(o):
    """json.dumps fallback: numpy scalars/arrays and anything else an engine might leak into a result."""
    if hasattr(o, "item"):
        return o.item()
    if hasattr(o, "tolist"):
        return o.tolist()
    return str(o)


def reap_stale(db: Session, user_id: int) -> None:
    """Mark the user's abandoned runs as failed so the UI never spins forever and the 'too many runs' limit frees up."""
    cutoff = datetime.now(timezone.utc).timestamp() - STALE_AFTER_SECONDS
    for r in db.scalars(select(LabRun).where(LabRun.user_id == user_id, LabRun.status.in_(("queued", "processing")))):
        if r.created_at.timestamp() < cutoff:
            r.status, r.error_code, r.error, r.finished_at = "failed", "timed_out", "The extraction did not finish (the server may have restarted). Please try again.", datetime.now(timezone.utc)
    db.commit()


def run_dir(run_id: str) -> Path:
    d = settings.storage_dir / "lab" / run_id
    d.mkdir(parents=True, exist_ok=True)
    return d


def own_run(db: Session, run_id: str, user: User) -> LabRun:
    r = db.get(LabRun, run_id)
    if not r or r.user_id != user.id:
        raise ApiError(404, "run_not_found", "Extraction not found")
    return r


def out(r: LabRun, with_result: bool = True) -> dict:
    d = {"id": r.id, "filename": r.filename, "mode": r.mode, "lang": r.lang, "enhance": r.enhance, "engine": r.engine, "dataset_id": r.dataset_id,
         "status": r.status, "created_at": r.created_at, "finished_at": r.finished_at}
    if r.status == "failed":
        d["error"] = {"code": r.error_code, "message": r.error}
    if r.status == "done" and r.result:
        d["summary"] = {"engine": r.result["engine"], "pages": len(r.result["pages"]), "avg_confidence": r.result["avg_confidence"], "flagged_count": r.result["flagged_count"],
                        "enhanced": r.result["enhanced"], "text_preview": r.result["text"][:140]}
        if with_result:
            d["result"] = r.result
    return d


def capabilities() -> dict:
    t, h = P.ENGINES["tesseract"], P.ENGINES["trocr"]
    return {
        "modes": list(P.MODES), "enhance_modes": list(P.ENHANCE_MODES), "low_confidence_threshold": P.LOW_CONFIDENCE,
        "engines": {"tesseract": {"available": t.available(), "languages": sorted(ALLOWED_LANGS), "note": "Printed text. Can be fine-tuned on your own data."},
                    "trocr": {"available": h.available(), "languages": ["eng"], "reason": h.unavailable_reason(),
                              "note": "English handwriting (print and cursive), read line by line."}},
        "handwriting_engine": "trocr" if h.available() else "tesseract",
        "limits": {"max_upload_mb": MAX_UPLOAD // 1024 // 1024, "max_pages": P.MAX_PAGES, "file_types": ["PNG", "JPEG", "TIFF", "BMP", "WEBP", "PDF"],
                   "max_active_runs": MAX_ACTIVE_PER_USER},
        "training": {"min_samples": train.MIN_SAMPLES, "max_samples_per_dataset": datasets.MAX_SAMPLES, "max_datasets": datasets.MAX_DATASETS,
                     "categories": datasets.CATEGORY_HINTS, "languages": settings.trainable_list},
    }


def create_run(db: Session, user: User, filename: str, data: bytes, mode: str, lang: str, enhance: str, engine: str, dataset_id: Optional[int]) -> LabRun:
    if len(data) > MAX_UPLOAD:
        raise ApiError(413, "file_too_large", f"The file is larger than {MAX_UPLOAD // 1024 // 1024} MB")
    if not data:
        raise ApiError(400, "empty_file", "The uploaded file is empty")
    if mode not in P.MODES:
        raise ApiError(422, "invalid_mode", f"mode must be one of: {', '.join(P.MODES)}")
    if enhance not in P.ENHANCE_MODES:
        raise ApiError(422, "invalid_enhance", f"enhance must be one of: {', '.join(P.ENHANCE_MODES)}")
    if engine not in ("auto", *P.ENGINES):
        raise ApiError(422, "invalid_engine", f"engine must be auto, {', '.join(P.ENGINES)}")
    if not lang or not all(p in ALLOWED_LANGS for p in lang.split("+")):
        raise ApiError(422, "invalid_lang", f"lang must combine these codes with '+': {', '.join(sorted(ALLOWED_LANGS))}")
    mime, ext, _ = validate_upload(data)  # magic bytes: raises 415 for anything that is not an image or PDF
    if dataset_id is not None:
        datasets.owned(db, dataset_id, user)
    active = db.scalar(select(func.count()).select_from(LabRun).where(LabRun.user_id == user.id, LabRun.status.in_(("queued", "processing"))))
    if active >= MAX_ACTIVE_PER_USER:
        raise ApiError(429, "too_many_runs", f"You already have {active} extractions in progress. Wait for one to finish.")
    r = LabRun(user_id=user.id, filename=(filename or "upload")[:255], mime=mime, mode=mode, lang=lang, enhance=enhance, engine=engine, dataset_id=dataset_id)
    db.add(r)
    db.flush()
    (run_dir(r.id) / f"source{ext}").write_bytes(data)
    # keep disk bounded: drop the user's oldest runs beyond KEEP_RUNS
    old = db.scalars(select(LabRun).where(LabRun.user_id == user.id).order_by(LabRun.created_at.desc()).offset(KEEP_RUNS)).all()
    for o in old:
        shutil.rmtree(settings.storage_dir / "lab" / o.id, ignore_errors=True)
        db.delete(o)
    db.commit()
    dispatch("run_lab_run", r.id)
    return r


def execute_run(db: Session, run_id: str) -> None:
    """Worker side. Never raises, and never leaves the run stuck: every failure (including a failure to SAVE the result) is recorded on it."""
    r = db.get(LabRun, run_id)
    if not r or r.status != "queued":
        return
    r.status = "processing"
    db.commit()
    try:
        _execute(db, r)
        db.commit()
    except Exception as e:  # noqa: BLE001
        db.rollback()
        r = db.get(LabRun, run_id)
        r.result = None
        if isinstance(e, P.LabError):
            r.status, r.error_code, r.error = "failed", e.code, e.message
        else:
            r.status, r.error_code, r.error = "failed", "extraction_failed", f"Extraction failed unexpectedly ({type(e).__name__})"
        r.finished_at = datetime.now(timezone.utc)
        db.commit()


def _execute(db: Session, r: LabRun) -> None:
    d = run_dir(r.id)
    src = next(p for p in d.iterdir() if p.name.startswith("source"))
    pages, warnings = P.load_pages(src.read_bytes(), r.mime)
    engine, more = P.choose_engine(r.mode, r.lang, r.engine)
    warnings += more
    tessdata = None
    if r.dataset_id and engine.name == "tesseract":
        ds = db.get(Dataset, r.dataset_id)
        tessdata = train.tessdata_for(db, ds) if ds else None
        if tessdata is None:
            warnings.append("The selected dataset has no active model yet, so the standard model was used")
    elif r.dataset_id:
        warnings.append("Custom models fine-tune Tesseract only; the TrOCR handwriting engine was used as-is")
    results = []
    for n, bgr in enumerate(pages, 1):
        out_ = P.process_page(bgr, mode=r.mode, lang=r.lang, enhance_mode=r.enhance, engine=engine, tessdata=tessdata)
        cv2.imwrite(str(d / f"page{n}_original.png"), bgr)
        if out_.enhanced is not None:
            cv2.imwrite(str(d / f"page{n}_enhanced.png"), out_.enhanced)
        results.append(P.page_result(n, out_))
    lines = [l for p in results for l in p["lines"]]
    confs = [l["confidence"] for l in lines]
    if not lines:
        warnings.append("No text was found. Try enhancement 'on', a different mode, or a sharper image.")
    result = {
        "engine": engine.name, "mode": r.mode, "lang": r.lang, "pages": results, "text": "\n\n".join(p["text"] for p in results).strip(),
        "avg_confidence": round(sum(confs) / len(confs), 1) if confs else None, "low_confidence_threshold": P.LOW_CONFIDENCE,
        "flagged_count": sum(p["flagged_count"] for p in results), "enhanced": any(p["enhanced"] for p in results), "warnings": warnings,
        "used_dataset_model": tessdata is not None,
    }
    r.result = json.loads(json.dumps(result, default=_plain))  # guarantees it is plain JSON before it reaches the database
    r.status = "done"
    r.finished_at = datetime.now(timezone.utc)


def page_image(r: LabRun, page: int, which: str) -> Path:
    if which not in ("original", "enhanced"):
        raise ApiError(404, "not_found", "No such image")
    p = settings.storage_dir / "lab" / r.id / f"page{page}_{which}.png"
    if not p.exists():
        raise ApiError(404, "not_found", "That image does not exist for this extraction")
    return p


def save_correction(db: Session, user: User, r: LabRun, page: int, index: int, text: str, ds: Dataset) -> "dict":
    if r.status != "done" or not r.result:
        raise ApiError(409, "not_ready", "The extraction has not finished")
    pg = next((p for p in r.result["pages"] if p["page"] == page), None)
    if not pg or not (0 <= index < len(pg["lines"])):
        raise ApiError(422, "bad_line", f"No line {index} on page {page}")
    line = pg["lines"][index]
    which = "enhanced" if pg["enhanced"] else "original"  # the boxes refer to the image the engine actually read
    img = cv2.imread(str(page_image(r, page, which)), cv2.IMREAD_GRAYSCALE)
    s = datasets.add_sample(db, ds, datasets.crop_line(img, line["bbox"]), text, source="correction", original=line["text"])
    return datasets.sample_out(s)
