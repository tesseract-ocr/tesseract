from __future__ import annotations

import hashlib
import secrets
from datetime import datetime, timedelta, timezone

import bcrypt
import jwt

from .config import settings

API_KEY_PREFIX = "vocr_"


def hash_password(pw: str) -> str:
    return bcrypt.hashpw(pw.encode()[:72], bcrypt.gensalt()).decode()


def verify_password(pw: str, hashed: str) -> bool:
    try:
        return bcrypt.checkpw(pw.encode()[:72], hashed.encode())
    except ValueError:
        return False


def make_jwt(user_id: int, role: str, version: int = 0) -> str:
    exp = datetime.now(timezone.utc) + timedelta(minutes=settings.jwt_expire_minutes)
    return jwt.encode({"sub": str(user_id), "role": role, "pv": version, "exp": exp}, settings.jwt_secret, algorithm="HS256")


def read_jwt(token: str) -> dict:
    return jwt.decode(token, settings.jwt_secret, algorithms=["HS256"])


def sha256(s: str) -> str:
    return hashlib.sha256(s.encode()).hexdigest()


def generate_api_key():
    """Returns (plaintext, prefix, sha256). The plaintext is shown to the user once and never stored."""
    key = API_KEY_PREFIX + secrets.token_urlsafe(32)
    return key, key[:12], sha256(key)


def new_reset_token():
    t = secrets.token_urlsafe(32)
    return t, sha256(t)
