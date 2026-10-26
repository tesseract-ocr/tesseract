from __future__ import annotations

import shutil
from pathlib import Path
from typing import List, Optional

from fastapi import APIRouter, Depends, File, Form, Query, UploadFile
from fastapi.responses import FileResponse, Response
from pydantic import BaseModel, Field
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..db import get_db
from ..deps import current_user
from ..errors import ApiError
from ..models import OcrJob, User
from ..services import export, samples
from ..services.jobs import create_job, job_payload

router = APIRouter(prefix="/api/web", tags=["web: user"])


def own_job(db: Session, job_id: str, user: User) -> OcrJob:
    job = db.get(OcrJob, job_id)
    if not job or job.user_id != user.id:  # same answer for "missing" and "someone else's": no enumeration
        raise ApiError(404, "not_found", "Job not found")
    return job


@router.post("/jobs", status_code=201)
async def upload(file: UploadFile = File(...), lang: str = Form(""), db: Session = Depends(get_db),
                 user: User = Depends(current_user)):
    data = await file.read(settings.max_upload_mb * 1024 * 1024 + 1)
    job = create_job(db, user_id=user.id, api_key_id=None, source="web", filename=file.filename or "upload",
                     data=data, lang=lang, force_async=False)
    return job_payload(job)


@router.get("/jobs")
def history(q: str = "", page: int = Query(1, ge=1), per_page: int = Query(20, ge=1, le=100),
            db: Session = Depends(get_db), user: User = Depends(current_user)):
    stmt = select(OcrJob).where(OcrJob.user_id == user.id)
    if q:
        like = f"%{q}%"
        stmt = stmt.where(OcrJob.filename.ilike(like) | OcrJob.text.ilike(like))
    total = db.scalar(select(func.count()).select_from(stmt.subquery()))
    rows = db.scalars(stmt.order_by(OcrJob.created_at.desc()).offset((page - 1) * per_page).limit(per_page)).all()
    return {"total": total, "page": page, "per_page": per_page,
            "items": [{**job_payload(j, with_result=False), "confidence": j.avg_confidence,
                       "preview": (j.edited_text if j.edited_text is not None else j.text)[:160]} for j in rows]}


@router.get("/jobs/{job_id}")
def get_job(job_id: str, db: Session = Depends(get_db), user: User = Depends(current_user)):
    job = own_job(db, job_id, user)
    out = job_payload(job)
    out["edited"] = job.edited_text is not None
    return out


@router.delete("/jobs/{job_id}", status_code=204)
def delete_job(job_id: str, db: Session = Depends(get_db), user: User = Depends(current_user)):
    job = own_job(db, job_id, user)
    shutil.rmtree(settings.storage_dir / "uploads" / job.id, ignore_errors=True)
    db.delete(job)
    db.commit()
    return Response(status_code=204)


class TextBody(BaseModel):
    text: str = Field(max_length=2_000_000)


@router.put("/jobs/{job_id}/text")
def edit_text(job_id: str, body: TextBody, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Free-form edit of the whole text (does not produce training data: use line corrections for that)."""
    job = own_job(db, job_id, user)
    job.edited_text = body.text
    db.commit()
    return {"ok": True}


class LineEdit(BaseModel):
    page: int
    index: int
    text: str = Field(max_length=300)


class LineEdits(BaseModel):
    edits: List[LineEdit] = Field(max_length=500)


@router.patch("/jobs/{job_id}/lines")
def correct_lines(job_id: str, body: LineEdits, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Correct individual OCR lines. Each changed line is stored as a (line image, corrected text) training sample
    that waits for admin approval."""
    job = own_job(db, job_id, user)
    if job.status != "done" or not job.result:
        raise ApiError(409, "not_ready", "The job has not finished")
    result = job.result
    train_lang = samples.train_lang_for(job.lang)
    created = 0
    for e in body.edits:
        page = next((p for p in result["pages"] if p["page"] == e.page), None)
        if not page or not (0 <= e.index < len(page["lines"])):
            raise ApiError(422, "bad_line", f"No line {e.index} on page {e.page}")
        line = page["lines"][e.index]
        new = " ".join(e.text.split())
        if not new or new == line["text"]:
            continue
        if page.get("image") and train_lang:
            try:
                img = samples.crop_line(str(settings.storage_dir / "uploads" / job.id / page["image"]), line["bbox"])
                samples.add_sample(db, source="correction", lang=train_lang, image_path=samples.save_line_image(img),
                                   text=new, original=line["text"], user_id=user.id, job_id=job.id)
                created += 1
            except Exception:  # noqa: BLE001 - a bad crop must not lose the user's edit
                pass
        line["text"] = new
    job.result = None  # force SQLAlchemy to see the JSON change
    db.flush()
    job.result = result
    job.edited_text = "\n\n".join("\n".join(ln["text"] for ln in p["lines"]) for p in result["pages"]).strip()
    db.commit()
    return {"ok": True, "training_samples_created": created, "text": job.edited_text}


@router.get("/jobs/{job_id}/export")
def export_job(job_id: str, format: str = Query("txt", pattern="^(txt|json|pdf)$"), db: Session = Depends(get_db),
               user: User = Depends(current_user)):
    job = own_job(db, job_id, user)
    if job.status != "done":
        raise ApiError(409, "not_ready", "The job has not finished")
    stem = Path(job.filename).stem or "result"
    if format == "txt":
        body, mime = export.final_text(job).encode("utf-8"), "text/plain; charset=utf-8"
    elif format == "json":
        body, mime = export.to_json(job).encode("utf-8"), "application/json"
    else:
        body, mime = export.to_pdf(job), "application/pdf"
    from urllib.parse import quote
    return Response(body, media_type=mime,
                    headers={"Content-Disposition": f"attachment; filename*=UTF-8''{quote(stem)}.{format}"})


@router.get("/jobs/{job_id}/pages/{page}/image")
def page_image(job_id: str, page: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    job = own_job(db, job_id, user)
    p = next((p for p in (job.result or {}).get("pages", []) if p["page"] == page), None)
    if not p or not p.get("image"):
        raise ApiError(404, "not_found", "No image for this page")
    return FileResponse(settings.storage_dir / "uploads" / job.id / p["image"])
