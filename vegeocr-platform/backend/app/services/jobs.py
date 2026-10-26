"""Create + run OCR jobs; shared by the web app and the public API."""
from __future__ import annotations

from pathlib import Path

from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import OcrJob, new_id
from ..worker.dispatch import dispatch
from . import ocr
from .files import validate_upload

ALLOWED_LANGS = {"jpn", "jpn_vert", "chi_sim", "chi_tra"}


def check_lang(lang: str) -> str:
    lang = (lang or settings.default_lang).strip()
    parts = lang.split("+")
    if not parts or any(p not in ALLOWED_LANGS for p in parts):
        raise ApiError(422, "invalid_lang", f"lang must combine these codes with '+': {', '.join(sorted(ALLOWED_LANGS))}")
    return lang


def create_job(db: Session, *, user_id: int, api_key_id, source: str, filename: str, data: bytes,
               lang: str, force_async: bool) -> OcrJob:
    mime, ext, pages = validate_upload(data)
    lang = check_lang(lang)
    job = OcrJob(id=new_id(), user_id=user_id, api_key_id=api_key_id, source=source, filename=(filename or "upload")[:255],
                 mime=mime, file_path="", lang=lang, page_count=pages)
    d = settings.storage_dir / "uploads" / job.id
    d.mkdir(parents=True, exist_ok=True)
    path = d / f"original{ext}"
    path.write_bytes(data)
    job.file_path = str(path)
    db.add(job)
    db.commit()
    big = pages > settings.sync_max_pages or len(data) > settings.sync_max_mb * 1024 * 1024
    if force_async or big:
        dispatch("run_ocr_job", job.id)
    else:
        ocr.run_job(db, job)
    return job


def job_payload(job: OcrJob, with_result: bool = True) -> dict:
    out = {"id": job.id, "status": job.status, "filename": job.filename, "lang": job.lang,
           "pages": job.page_count, "created_at": job.created_at, "finished_at": job.finished_at}
    if job.status == "failed":
        out["error"] = job.error
    if job.status == "done" and with_result:
        out.update(text=job.edited_text if job.edited_text is not None else job.text,
                   confidence=job.avg_confidence,
                   result=[{"page": p["page"], "source": p["source"],
                            "lines": [{"text": ln["text"], "confidence": ln["conf"], "bbox": ln["bbox"]}
                                      for ln in p["lines"]]} for p in (job.result or {}).get("pages", [])])
    return out
