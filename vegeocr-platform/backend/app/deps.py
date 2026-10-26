from __future__ import annotations

from datetime import datetime, timezone

import jwt
from fastapi import Depends, Header, Request, Response
from sqlalchemy import select
from sqlalchemy.orm import Session

from .config import settings
from .db import get_db
from .errors import ApiError
from .scopes import missing
from .models import ApiKey, User
from .security import read_jwt, sha256
from .services import ratelimit


def current_user(authorization: str = Header(default=""), db: Session = Depends(get_db)) -> User:
    if not authorization.lower().startswith("bearer "):
        raise ApiError(401, "unauthorized", "Missing bearer token")
    try:
        claims = read_jwt(authorization[7:])
    except jwt.PyJWTError:
        raise ApiError(401, "invalid_token", "Invalid or expired token")
    user = db.get(User, int(claims["sub"]))
    if not user:
        raise ApiError(401, "invalid_token", "User no longer exists")
    if user.status != "active":
        raise ApiError(403, "suspended", "This account is suspended")
    if int(claims.get("pv", 0)) != (user.token_version or 0):  # the password changed after this token was issued
        raise ApiError(401, "session_expired", "Your password was changed. Please sign in again.")
    return user


def admin_user(user: User = Depends(current_user)) -> User:
    if user.role != "admin":
        raise ApiError(403, "forbidden", "Administrator access required")
    return user


def api_key_auth(request: Request, response: Response, x_api_key: str = Header(default=""),
                 db: Session = Depends(get_db)) -> ApiKey:
    if not x_api_key:
        raise ApiError(401, "missing_api_key", "Send your API key in the X-API-Key header")
    key = db.scalar(select(ApiKey).where(ApiKey.key_hash == sha256(x_api_key)))
    if not key:
        raise ApiError(401, "invalid_api_key", "API key not recognised")
    request.state.api_key_id, request.state.user_id = key.id, key.user_id
    if key.revoked_at:
        raise ApiError(401, "revoked_api_key", "This API key has been revoked")
    if not key.enabled:
        raise ApiError(401, "disabled_api_key", "This API key is disabled")
    if key.expires_at and key.expires_at <= datetime.now(timezone.utc):
        raise ApiError(401, "expired_api_key", "This API key has expired")
    user = db.get(User, key.user_id)
    if not user or user.status != "active":
        raise ApiError(403, "suspended", "The owner of this API key is suspended")
    ok, remaining, reset = ratelimit.check_rate(key.id, key.rate_limit_per_min)
    hdrs = {"X-RateLimit-Limit": str(key.rate_limit_per_min), "X-RateLimit-Remaining": str(remaining),
            "X-RateLimit-Reset": str(reset)}
    if not ok:
        raise ApiError(429, "rate_limited", f"Rate limit of {key.rate_limit_per_min}/min exceeded",
                       headers={**hdrs, "Retry-After": str(reset)})
    if request.method == "POST" and request.url.path in ratelimit.BILLABLE:
        used = ratelimit.monthly_usage(db, key.id)
        if used >= key.monthly_quota:
            raise ApiError(429, "quota_exceeded", f"Monthly quota of {key.monthly_quota} requests used up",
                           headers=hdrs)
    for k, v in hdrs.items():
        response.headers[k] = v
    key.last_used_at = datetime.now(timezone.utc)
    db.commit()
    request.state.owner = user
    return key


def require_scope(*needed: str):
    """Dependency factory: authenticate the key (401 / 403 / 429 as before), then require every listed scope (403).
    `admin` implies all scopes. Returns the ApiKey, so handlers use it exactly like api_key_auth."""

    def dependency(key: ApiKey = Depends(api_key_auth)) -> ApiKey:
        lacking = missing(key.scopes, needed)
        if lacking:
            raise ApiError(403, "insufficient_scope", f"This API key lacks the required scope: {', '.join(lacking)}",
                           headers={"X-Required-Scope": " ".join(needed)})
        return key

    return dependency
