from __future__ import annotations

from datetime import datetime, timedelta, timezone
from typing import List, Optional

from fastapi import APIRouter, Depends, Query
from fastapi.responses import FileResponse, Response
from pydantic import BaseModel, EmailStr, Field
from sqlalchemy import case, func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..db import get_db
from ..deps import admin_user
from ..errors import ApiError
from ..models import (ApiKey, ApiRequestLog, ModelVersion, OcrJob, TrainingRun, TrainingSample, User)
from ..security import hash_password
from ..services import appsettings, modelstore, training
from ..services.jobs import ALLOWED_LANGS
from ..worker.dispatch import dispatch
from .auth import strong, user_out

router = APIRouter(prefix="/api/admin", tags=["admin"], dependencies=[Depends(admin_user)])


def page_of(db, stmt, page, per_page):
    total = db.scalar(select(func.count()).select_from(stmt.subquery()))
    return total, db.scalars(stmt.offset((page - 1) * per_page).limit(per_page)).all()


# ---- users ------------------------------------------------------------------------------------

class UserCreate(BaseModel):
    email: EmailStr
    password: str = Field(max_length=128)
    full_name: str = Field(default="", max_length=120)
    role: str = Field(default="user", pattern="^(user|admin)$")


class UserPatch(BaseModel):
    full_name: Optional[str] = Field(default=None, max_length=120)
    role: Optional[str] = Field(default=None, pattern="^(user|admin)$")
    status: Optional[str] = Field(default=None, pattern="^(active|suspended)$")
    password: Optional[str] = Field(default=None, max_length=128)


@router.get("/users")
def list_users(q: str = "", page: int = Query(1, ge=1), per_page: int = Query(25, ge=1, le=100), db: Session = Depends(get_db)):
    stmt = select(User).order_by(User.id)
    if q:
        stmt = stmt.where(User.email.ilike(f"%{q}%") | User.full_name.ilike(f"%{q}%"))
    total, rows = page_of(db, stmt, page, per_page)
    counts = dict(db.execute(select(OcrJob.user_id, func.count()).group_by(OcrJob.user_id)).all())
    return {"total": total, "items": [{**user_out(u), "created_at": u.created_at, "last_login_at": u.last_login_at,
                                      "jobs": counts.get(u.id, 0)} for u in rows]}


@router.post("/users", status_code=201)
def create_user(body: UserCreate, db: Session = Depends(get_db)):
    strong(body.password)
    if db.scalar(select(User).where(User.email == body.email.lower())):
        raise ApiError(409, "email_taken", "An account with this email already exists")
    u = User(email=body.email.lower(), password_hash=hash_password(body.password), full_name=body.full_name, role=body.role)
    db.add(u)
    db.commit()
    return user_out(u)


@router.patch("/users/{uid}")
def patch_user(uid: int, body: UserPatch, db: Session = Depends(get_db), me: User = Depends(admin_user)):
    u = db.get(User, uid)
    if not u:
        raise ApiError(404, "not_found", "User not found")
    if u.id == me.id and (body.status == "suspended" or body.role == "user"):
        raise ApiError(409, "self_lockout", "You cannot suspend or demote your own account")
    if body.full_name is not None:
        u.full_name = body.full_name
    if body.role:
        u.role = body.role
    if body.status:
        u.status = body.status
    if body.password:
        strong(body.password)
        u.password_hash = hash_password(body.password)
        u.token_version = (u.token_version or 0) + 1
        u.password_changed_at = datetime.now(timezone.utc)
    db.commit()
    return user_out(u)


@router.delete("/users/{uid}", status_code=204)
def delete_user(uid: int, db: Session = Depends(get_db), me: User = Depends(admin_user)):
    u = db.get(User, uid)
    if not u:
        raise ApiError(404, "not_found", "User not found")
    if u.id == me.id:
        raise ApiError(409, "self_lockout", "You cannot delete your own account")
    db.delete(u)
    db.commit()
    return Response(status_code=204)


# ---- API keys ---------------------------------------------------------------------------------

class KeyPatch(BaseModel):
    rate_limit_per_min: Optional[int] = Field(default=None, ge=1, le=100000)
    monthly_quota: Optional[int] = Field(default=None, ge=0, le=100_000_000)


@router.get("/keys")
def list_keys(db: Session = Depends(get_db)):
    from ..services.ratelimit import month_start
    ms = month_start()
    used = dict(db.execute(select(ApiRequestLog.api_key_id, func.count()).where(
        ApiRequestLog.created_at >= ms, ApiRequestLog.method == "POST", ApiRequestLog.status_code < 400,
        ApiRequestLog.path.in_(("/api/v1/ocr", "/api/v1/training-data"))).group_by(ApiRequestLog.api_key_id)).all())
    rows = db.execute(select(ApiKey, User.email).join(User, User.id == ApiKey.user_id).order_by(ApiKey.id.desc())).all()
    from ..services import keys as keysvc
    return [{**keysvc.serialize(k), "prefix": k.prefix, "owner": email, "user_id": k.user_id, "used_this_month": used.get(k.id, 0),
             "revoked": k.revoked_at is not None} for k, email in rows]


@router.patch("/keys/{kid}")
def patch_key(kid: int, body: KeyPatch, db: Session = Depends(get_db)):
    k = db.get(ApiKey, kid)
    if not k:
        raise ApiError(404, "not_found", "API key not found")
    if body.rate_limit_per_min is not None:
        k.rate_limit_per_min = body.rate_limit_per_min
    if body.monthly_quota is not None:
        k.monthly_quota = body.monthly_quota
    db.commit()
    return {"ok": True}


@router.post("/keys/{kid}/revoke")
def revoke_key(kid: int, db: Session = Depends(get_db)):
    k = db.get(ApiKey, kid)
    if not k:
        raise ApiError(404, "not_found", "API key not found")
    k.revoked_at = k.revoked_at or datetime.now(timezone.utc)
    db.commit()
    return {"ok": True}


# ---- usage ------------------------------------------------------------------------------------

@router.get("/usage")
def usage(days: int = Query(14, ge=1, le=90), db: Session = Depends(get_db)):
    since = datetime.now(timezone.utc) - timedelta(days=days)
    L = ApiRequestLog
    err = func.coalesce(func.sum(case((L.status_code >= 400, 1), else_=0)), 0)
    day = func.date_trunc("day", L.created_at)
    daily = db.execute(select(day, func.count(), err).where(L.created_at >= since).group_by(day).order_by(day)).all()
    by_key = db.execute(select(L.api_key_id, ApiKey.name, ApiKey.prefix, User.email, func.count(), err)
                        .join(ApiKey, ApiKey.id == L.api_key_id, isouter=True).join(User, User.id == L.user_id, isouter=True)
                        .where(L.created_at >= since).group_by(L.api_key_id, ApiKey.name, ApiKey.prefix, User.email)
                        .order_by(func.count().desc()).limit(50)).all()
    by_user = db.execute(select(L.user_id, User.email, func.count(), err).join(User, User.id == L.user_id, isouter=True)
                         .where(L.created_at >= since).group_by(L.user_id, User.email).order_by(func.count().desc()).limit(50)).all()
    total = sum(r[1] for r in daily)
    errors = sum(int(r[2]) for r in daily)
    return {"days": days, "total": total, "errors": errors, "success_rate": round(100 * (total - errors) / total, 1) if total else None,
            "daily": [{"date": d.date().isoformat(), "requests": c, "errors": int(e)} for d, c, e in daily],
            "by_key": [{"key_id": k, "name": n, "prefix": p, "owner": o, "requests": c, "errors": int(e)} for k, n, p, o, c, e in by_key],
            "by_user": [{"user_id": u, "email": m, "requests": c, "errors": int(e)} for u, m, c, e in by_user]}


@router.get("/overview")
def overview(db: Session = Depends(get_db)):
    jobs = dict(db.execute(select(OcrJob.status, func.count()).group_by(OcrJob.status)).all())
    samples = dict(db.execute(select(TrainingSample.status, func.count()).group_by(TrainingSample.status)).all())
    return {"users": db.scalar(select(func.count()).select_from(User)),
            "active_keys": db.scalar(select(func.count()).select_from(ApiKey).where(ApiKey.revoked_at.is_(None))),
            "jobs": jobs, "samples": samples,
            "models": [{"lang": m.lang, "version": m.version, "cer": m.cer} for m in db.scalars(
                select(ModelVersion).where(ModelVersion.status == "active"))]}


# ---- models & training ------------------------------------------------------------------------

def model_out(m: ModelVersion):
    return {"id": m.id, "lang": m.lang, "version": m.version, "status": m.status, "trigger": m.trigger, "cer": m.cer,
            "baseline_cer": m.baseline_cer, "train_samples": m.train_samples, "test_samples": m.test_samples,
            "notes": m.notes, "created_at": m.created_at, "activated_at": m.activated_at}


@router.get("/models")
def list_models(db: Session = Depends(get_db)):
    return [model_out(m) for m in db.scalars(select(ModelVersion).order_by(ModelVersion.lang, ModelVersion.version.desc()))]


@router.get("/training-runs")
def list_runs(db: Session = Depends(get_db)):
    rows = db.scalars(select(TrainingRun).order_by(TrainingRun.id.desc()).limit(50)).all()
    return [{"id": r.id, "lang": r.lang, "status": r.status, "trigger": r.trigger, "created_at": r.created_at,
             "started_at": r.started_at, "finished_at": r.finished_at, "version_id": r.version_id, "message": r.message}
            for r in rows]


class Retrain(BaseModel):
    lang: str


@router.post("/models/retrain", status_code=202)
def retrain(body: Retrain, db: Session = Depends(get_db), me: User = Depends(admin_user)):
    if body.lang not in settings.trainable_list:
        raise ApiError(422, "invalid_lang", f"lang must be one of: {', '.join(settings.trainable_list)}")
    run = training.enqueue_run(db, body.lang, "manual", me.id)
    if run.status == "queued":
        dispatch("train_model", run.id)
    return {"run_id": run.id, "status": run.status}


@router.post("/models/{mid}/activate")
def activate_model(mid: int, db: Session = Depends(get_db)):
    """Make this version live. Activating an older version is a rollback."""
    m = db.get(ModelVersion, mid)
    if not m:
        raise ApiError(404, "not_found", "Model version not found")
    modelstore.activate(db, m)
    return model_out(m)


class TrainingSettings(BaseModel):
    retrain_threshold: Optional[int] = Field(default=None, ge=1)
    retrain_min_test_samples: Optional[int] = Field(default=None, ge=1)
    retrain_max_iterations: Optional[int] = Field(default=None, ge=10, le=100000)
    retrain_schedule_hours: Optional[int] = Field(default=None, ge=1)
    auto_retrain_enabled: Optional[bool] = None
    min_improvement: Optional[float] = Field(default=None, ge=0, le=1)


@router.get("/training-settings")
def get_training_settings(db: Session = Depends(get_db)):
    return appsettings.get_all(db)


@router.put("/training-settings")
def put_training_settings(body: TrainingSettings, db: Session = Depends(get_db)):
    return appsettings.update(db, body.model_dump(exclude_none=True))


# ---- training data review ---------------------------------------------------------------------

@router.get("/training-data")
def list_samples(status: str = "pending", source: str = "", lang: str = "", page: int = Query(1, ge=1),
                 per_page: int = Query(25, ge=1, le=100), db: Session = Depends(get_db)):
    stmt = select(TrainingSample).order_by(TrainingSample.id.desc())
    if status:
        stmt = stmt.where(TrainingSample.status == status)
    if source:
        stmt = stmt.where(TrainingSample.source == source)
    if lang:
        stmt = stmt.where(TrainingSample.lang == lang)
    total, rows = page_of(db, stmt, page, per_page)
    return {"total": total, "items": [{"id": s.id, "source": s.source, "lang": s.lang, "text": s.text,
                                      "original_text": s.original_text, "status": s.status, "is_test": s.is_test,
                                      "created_at": s.created_at} for s in rows]}


@router.get("/training-data/{sid}/image")
def sample_image(sid: int, db: Session = Depends(get_db)):
    s = db.get(TrainingSample, sid)
    if not s:
        raise ApiError(404, "not_found", "Sample not found")
    return FileResponse(s.image_path)


class Review(BaseModel):
    ids: List[int] = Field(min_length=1, max_length=500)
    action: str = Field(pattern="^(approve|reject)$")
    text: Optional[str] = Field(default=None, max_length=300)  # admin can fix the label while approving a single sample


@router.post("/training-data/review")
def review(body: Review, db: Session = Depends(get_db)):
    rows = db.scalars(select(TrainingSample).where(TrainingSample.id.in_(body.ids))).all()
    now = datetime.now(timezone.utc)
    for s in rows:
        s.status = "approved" if body.action == "approve" else "rejected"
        s.reviewed_at = now
        if body.action == "approve":
            # ~10% hold-out, deterministic by id. Identical text always lands on the same side of the split, so a
            # test line is never a near-copy of a training line (that would inflate the measured accuracy).
            twin = db.scalar(select(TrainingSample).where(
                TrainingSample.lang == s.lang, TrainingSample.text == s.text, TrainingSample.status == "approved",
                TrainingSample.id != s.id).order_by(TrainingSample.id))
            s.is_test = twin.is_test if twin else s.id % 10 == 0
            if body.text and len(rows) == 1:
                s.text = " ".join(body.text.split())
    db.commit()
    if body.action == "approve":
        dispatch("check_auto_retrain")
    return {"updated": len(rows)}
