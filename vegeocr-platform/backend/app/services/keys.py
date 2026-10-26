"""API-key lifecycle: create / list / rename / rotate / enable / disable / delete, plus usage statistics."""
from __future__ import annotations

from datetime import datetime, timedelta, timezone
from typing import List, Optional

from sqlalchemy import case, func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import ApiKey, ApiRequestLog, User
from ..scopes import ALL_SCOPES, DEFAULT_SCOPES
from ..security import API_KEY_PREFIX, generate_api_key
from . import ratelimit

EXPIRY_PRESETS = (30, 90, 365)
MAX_ACTIVE_KEYS = 10
MAX_EXPIRY_DAYS = 3650


def aware(dt: datetime) -> datetime:
    return dt if dt.tzinfo else dt.replace(tzinfo=timezone.utc)


def key_status(k: ApiKey) -> str:
    if k.revoked_at:
        return "revoked"
    if not k.enabled:
        return "disabled"
    if k.expires_at and aware(k.expires_at) <= datetime.now(timezone.utc):
        return "expired"
    return "active"


def mask(k: ApiKey) -> str:
    """vocr_****abcd. Keys created before last4 was stored show just vocr_****."""
    return f"{API_KEY_PREFIX}****{k.last4 or ''}"


def serialize(k: ApiKey, request_count: Optional[int] = None) -> dict:
    out = {
        "id": k.id, "name": k.name, "masked": mask(k), "scopes": k.scopes or DEFAULT_SCOPES, "status": key_status(k),
        "enabled": bool(k.enabled), "created_at": k.created_at, "last_used_at": k.last_used_at, "expires_at": k.expires_at,
        "rotated_at": k.rotated_at, "rate_limit_per_min": k.rate_limit_per_min, "monthly_quota": k.monthly_quota,
    }
    if request_count is not None:
        out["request_count"] = request_count
    return out


def normalise_scopes(scopes: Optional[List[str]], owner: User) -> List[str]:
    chosen = list(dict.fromkeys(scopes if scopes is not None else DEFAULT_SCOPES))  # de-duplicated, order kept
    if not chosen:
        raise ApiError(422, "no_scopes", "Choose at least one scope")
    unknown = [s for s in chosen if s not in ALL_SCOPES]
    if unknown:
        raise ApiError(422, "invalid_scope", f"Unknown scope(s): {', '.join(unknown)}. Valid scopes: {', '.join(ALL_SCOPES)}")
    if "admin" in chosen and owner.role != "admin":
        raise ApiError(403, "admin_scope_forbidden", "Only administrators can create keys with the admin scope")
    return chosen


def resolve_expiry(expires_in_days: Optional[int], expires_at: Optional[datetime]) -> Optional[datetime]:
    if expires_in_days is not None and expires_at is not None:
        raise ApiError(422, "invalid_expiry", "Send either expires_in_days or expires_at, not both")
    now = datetime.now(timezone.utc)
    if expires_in_days is not None:
        if expires_in_days not in EXPIRY_PRESETS:
            raise ApiError(422, "invalid_expiry", f"expires_in_days must be one of {list(EXPIRY_PRESETS)}; use expires_at for a custom date")
        return now + timedelta(days=expires_in_days)
    if expires_at is not None:
        at = aware(expires_at)
        if at <= now:
            raise ApiError(422, "invalid_expiry", "The expiration must be in the future")
        if at > now + timedelta(days=MAX_EXPIRY_DAYS):
            raise ApiError(422, "invalid_expiry", f"The expiration can be at most {MAX_EXPIRY_DAYS // 365} years away")
        return at
    return None  # never


def own_key(db: Session, key_id: int, user: User) -> ApiKey:
    k = db.get(ApiKey, key_id)
    if not k or k.user_id != user.id:  # same answer for "missing" and "someone else's"
        raise ApiError(404, "not_found", "API key not found")
    return k


def create_key(db: Session, user: User, name: str, scopes: Optional[List[str]], expires_in_days: Optional[int],
               expires_at: Optional[datetime]) -> dict:
    chosen = normalise_scopes(scopes, user)
    expiry = resolve_expiry(expires_in_days, expires_at)
    live = db.scalar(select(func.count()).select_from(ApiKey).where(ApiKey.user_id == user.id, ApiKey.revoked_at.is_(None)))
    if live >= MAX_ACTIVE_KEYS:
        raise ApiError(409, "too_many_keys", f"You can have at most {MAX_ACTIVE_KEYS} API keys")
    plain, prefix, digest = generate_api_key()
    k = ApiKey(user_id=user.id, name=name.strip(), prefix=prefix, key_hash=digest, last4=plain[-4:], scopes=chosen, expires_at=expiry,
               enabled=True, rate_limit_per_min=settings.default_rate_limit_per_min, monthly_quota=settings.default_monthly_quota)
    db.add(k)
    db.commit()
    return {**serialize(k), "key": plain, "warning": "Copy this key now. It will not be shown again."}


def rotate_key(db: Session, k: ApiKey) -> dict:
    """New secret on the same record (same id, name, scopes, limits). The old secret stops working immediately."""
    if k.revoked_at:
        raise ApiError(409, "key_revoked", "A revoked key cannot be rotated")
    plain, prefix, digest = generate_api_key()
    k.key_hash, k.prefix, k.last4, k.rotated_at = digest, prefix, plain[-4:], datetime.now(timezone.utc)
    db.commit()
    return {**serialize(k), "key": plain, "warning": "Copy this key now. It will not be shown again. The previous key no longer works."}


def usage(db: Session, k: ApiKey) -> dict:
    now = datetime.now(timezone.utc)
    L = ApiRequestLog
    err = func.coalesce(func.sum(case((L.status_code >= 400, 1), else_=0)), 0)

    def count_since(delta: Optional[timedelta]):
        stmt = select(func.count(), err).where(L.api_key_id == k.id)
        if delta:
            stmt = stmt.where(L.created_at >= now - delta)
        total, errors = db.execute(stmt).one()
        return int(total), int(errors)

    total, errors_total = count_since(None)
    day = func.date_trunc("day", L.created_at)
    rows = db.execute(select(day, func.count(), err).where(L.api_key_id == k.id, L.created_at >= now - timedelta(days=13))
                      .group_by(day)).all()
    by_day = {d.date().isoformat(): (int(c), int(e)) for d, c, e in rows}
    daily = []
    for i in range(13, -1, -1):
        d = (now - timedelta(days=i)).date().isoformat()
        c, e = by_day.get(d, (0, 0))
        daily.append({"date": d, "requests": c, "errors": e})
    recent = db.scalars(select(L).where(L.api_key_id == k.id).order_by(L.id.desc()).limit(20)).all()
    return {
        "key": serialize(k),
        "requests": {"total": total, "errors_total": errors_total, "last_24h": count_since(timedelta(hours=24))[0],
                     "last_7d": count_since(timedelta(days=7))[0], "last_30d": count_since(timedelta(days=30))[0]},
        "rate_limit": {"per_minute": k.rate_limit_per_min, "used_this_minute": ratelimit.current_usage(k.id)},
        "quota": {"monthly": k.monthly_quota, "used": ratelimit.monthly_usage(db, k.id)},
        "daily": daily,
        "recent": [{"at": r.created_at, "method": r.method, "path": r.path, "status": r.status_code, "latency_ms": r.latency_ms} for r in recent],
    }
