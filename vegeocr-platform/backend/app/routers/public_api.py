from __future__ import annotations

from datetime import datetime, timedelta, timezone

from fastapi.encoders import jsonable_encoder
from fastapi import APIRouter, Depends, File, Form, Response, UploadFile
from fastapi.responses import JSONResponse
from sqlalchemy import case, func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..db import get_db
from ..deps import require_scope
from ..errors import ApiError
from ..models import ApiKey, ApiRequestLog, OcrJob
from ..services import ratelimit, samples
from ..services.jobs import check_lang, create_job, job_payload

router = APIRouter(prefix="/api/v1", tags=["public API"])
ERR = {401: {"description": "Missing / invalid / revoked / disabled / expired API key"}, 403: {"description": "The key lacks the required scope"}, 429: {"description": "Rate limit or quota exceeded"}}


@router.post("/ocr", responses=ERR, summary="Extract text from an image or PDF")
async def ocr(file: UploadFile = File(..., description="PNG, JPEG, TIFF, BMP, WEBP or PDF"),
              lang: str = Form("jpn", description="Tesseract languages joined by '+': jpn, jpn_vert, chi_sim, chi_tra"),
              async_mode: bool = Form(False, alias="async", description="Queue the job and poll /jobs/{id}"),
              response: Response = None, key: ApiKey = Depends(require_scope("write")), db: Session = Depends(get_db)):
    """Small files are processed immediately (200). Large PDFs, big files, or `async=true` return 202 with a job id."""
    data = await file.read(settings.max_upload_mb * 1024 * 1024 + 1)
    job = create_job(db, user_id=key.user_id, api_key_id=key.id, source="api", filename=file.filename or "upload",
                     data=data, lang=lang, force_async=async_mode)
    payload = job_payload(job)
    return JSONResponse(jsonable_encoder(payload), status_code=200 if job.status in ("done", "failed") else 202,
                        headers={**dict(response.headers), "Location": f"/api/v1/jobs/{job.id}"})


@router.post("/training-data", status_code=201, responses=ERR, summary="Submit an image + correct text to improve the model")
async def training_data(image: UploadFile = File(..., description="Crop of ONE line of text"),
                        text: str = Form(..., description="The exact text in the image"),
                        lang: str = Form("jpn", description="Language of the line (jpn, chi_sim, chi_tra)"),
                        key: ApiKey = Depends(require_scope("write")), db: Session = Depends(get_db)):
    """Samples are queued for admin review and only used for training once approved."""
    if lang not in settings.trainable_list:
        raise ApiError(422, "invalid_lang", f"lang must be one of: {', '.join(settings.trainable_list)}")
    label = samples.clean_label(text)
    data = await image.read(5 * 1024 * 1024)
    im = samples.check_line_image(data)
    s = samples.add_sample(db, source="api", lang=lang, image_path=samples.save_line_image(im), text=label,
                           user_id=key.user_id, api_key_id=key.id)
    return {"id": s.id, "status": s.status}


@router.get("/jobs/{job_id}", responses=ERR, summary="Check an OCR job")
def job_status(job_id: str, key: ApiKey = Depends(require_scope("read")), db: Session = Depends(get_db)):
    job = db.get(OcrJob, job_id)
    if not job or job.user_id != key.user_id:
        raise ApiError(404, "not_found", "Job not found")
    return job_payload(job)


@router.get("/usage", responses=ERR, summary="Usage of the calling API key")
def usage(key: ApiKey = Depends(require_scope("read")), db: Session = Depends(get_db)):
    since = datetime.now(timezone.utc) - timedelta(days=7)
    day = func.date_trunc("day", ApiRequestLog.created_at)
    rows = db.execute(select(day, func.count(), func.sum(case((ApiRequestLog.status_code >= 400, 1), else_=0)))
                      .where(ApiRequestLog.api_key_id == key.id, ApiRequestLog.created_at >= since)
                      .group_by(day).order_by(day)).all()
    total, errors = db.execute(select(func.count(), func.coalesce(func.sum(case((ApiRequestLog.status_code >= 400, 1), else_=0)), 0))
                               .where(ApiRequestLog.api_key_id == key.id)).one()
    return {"key": {"name": key.name, "prefix": key.prefix},
            "rate_limit_per_min": key.rate_limit_per_min,
            "quota": {"monthly": key.monthly_quota, "used": ratelimit.monthly_usage(db, key.id),
                      "resets_at": (ratelimit.month_start() + timedelta(days=32)).replace(day=1)},
            "total_requests": total, "error_requests": int(errors),
            "last_7_days": [{"date": d.date().isoformat(), "requests": c, "errors": int(e or 0)} for d, c, e in rows]}
