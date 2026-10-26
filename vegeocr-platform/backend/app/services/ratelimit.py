from __future__ import annotations

import logging
import time
from datetime import datetime, timezone

import redis
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..models import ApiRequestLog

log = logging.getLogger("ratelimit")
_r = redis.Redis.from_url(settings.redis_url, socket_timeout=1)
BILLABLE = ("/api/v1/ocr", "/api/v1/training-data")


def check_rate(key_id: int, limit: int):
    """Fixed-window per-minute counter. Returns (allowed, remaining, reset_seconds). Fails open if Redis is down."""
    window = int(time.time() // 60)
    k = f"rl:{key_id}:{window}"
    try:
        pipe = _r.pipeline()
        pipe.incr(k)
        pipe.expire(k, 70)
        count = pipe.execute()[0]
    except redis.RedisError:
        log.warning("redis unavailable; rate limiting disabled for this request")
        return True, limit, 60
    return count <= limit, max(limit - count, 0), 60 - int(time.time() % 60)


def month_start():
    n = datetime.now(timezone.utc)
    return n.replace(day=1, hour=0, minute=0, second=0, microsecond=0)


def monthly_usage(db: Session, key_id: int) -> int:
    return db.scalar(select(func.count()).select_from(ApiRequestLog).where(
        ApiRequestLog.api_key_id == key_id, ApiRequestLog.created_at >= month_start(),
        ApiRequestLog.method == "POST", ApiRequestLog.path.in_(BILLABLE),
        ApiRequestLog.status_code < 400)) or 0


def current_usage(key_id: int) -> int:
    """Requests this key has made in the current rate-limit minute (0 if Redis is unavailable)."""
    try:
        return int(_r.get(f"rl:{key_id}:{int(time.time() // 60)}") or 0)
    except redis.RedisError:
        return 0
