from __future__ import annotations

import re
from datetime import datetime, timedelta, timezone

import redis
from fastapi import APIRouter, Depends, Request
from pydantic import BaseModel, EmailStr, Field
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..db import get_db
from ..deps import current_user
from ..errors import ApiError
from ..models import PasswordReset, User
from ..security import hash_password, make_jwt, new_reset_token, sha256, verify_password

router = APIRouter(prefix="/api/web/auth", tags=["web: auth"])
_r = redis.Redis.from_url(settings.redis_url, socket_timeout=1)


def strong(pw: str):
    if len(pw) < 8 or not re.search(r"[A-Za-z]", pw) or not re.search(r"\d", pw):
        raise ApiError(422, "weak_password", "Password must be at least 8 characters and contain a letter and a number")


def _limit(key: str, limit: int, message: str):
    """Fixed 15-minute window counter in Redis (fails open if Redis is down)."""
    try:
        n = _r.incr(key)
        if n == 1:
            _r.expire(key, 900)
        if n > limit:
            raise ApiError(429, "too_many_attempts", message)
    except redis.RedisError:
        pass


def throttle(request: Request, email: str):
    """Max 10 login attempts per 15 minutes per (ip, email)."""
    _limit(f"login:{request.client.host if request.client else '-'}:{email.lower()}", 10, "Too many login attempts. Try again in 15 minutes.")


def user_out(u: User):
    return {"id": u.id, "email": u.email, "username": u.username, "full_name": u.full_name, "role": u.role, "status": u.status}


class SignUp(BaseModel):
    email: EmailStr
    password: str = Field(max_length=128)
    full_name: str = Field(default="", max_length=120)


class Login(BaseModel):
    email: str = Field(max_length=255, description="Email address or username")
    password: str = Field(max_length=128)


class Forgot(BaseModel):
    email: EmailStr


class ChangePassword(BaseModel):
    current_password: str = Field(max_length=128)
    new_password: str = Field(max_length=128)


class Reset(BaseModel):
    token: str
    password: str = Field(max_length=128)


@router.post("/signup", status_code=201)
def signup(body: SignUp, db: Session = Depends(get_db)):
    strong(body.password)
    if db.scalar(select(User).where(User.email == body.email.lower())):
        raise ApiError(409, "email_taken", "An account with this email already exists")
    u = User(email=body.email.lower(), password_hash=hash_password(body.password), full_name=body.full_name.strip())
    db.add(u)
    db.commit()
    return {"token": make_jwt(u.id, u.role, u.token_version or 0), "user": user_out(u)}


def _login(body: Login, request: Request, db: Session, admin_only: bool):
    throttle(request, body.email)
    ident = body.email.strip().lower()
    u = db.scalar(select(User).where((User.email == ident) | (func.lower(User.username) == ident)))
    if not u or not verify_password(body.password, u.password_hash):
        raise ApiError(401, "bad_credentials", "Incorrect email or password")
    if u.status != "active":
        raise ApiError(403, "suspended", "This account is suspended")
    if admin_only and u.role != "admin":
        raise ApiError(403, "forbidden", "Administrator access required")
    u.last_login_at = datetime.now(timezone.utc)
    db.commit()
    return {"token": make_jwt(u.id, u.role, u.token_version or 0), "user": user_out(u)}


@router.post("/login")
def login(body: Login, request: Request, db: Session = Depends(get_db)):
    return _login(body, request, db, admin_only=False)


@router.post("/admin-login")
def admin_login(body: Login, request: Request, db: Session = Depends(get_db)):
    return _login(body, request, db, admin_only=True)


@router.post("/logout")
def logout(_: User = Depends(current_user)):
    return {"ok": True}  # JWTs are stateless: the client discards the token


@router.get("/me")
def me(u: User = Depends(current_user)):
    return {**user_out(u), "created_at": u.created_at, "last_login_at": u.last_login_at, "password_changed_at": u.password_changed_at}


@router.post("/change-password")
def change_password(body: ChangePassword, db: Session = Depends(get_db), u: User = Depends(current_user)):
    """Change your own password. Every OTHER session (other browsers, stolen tokens) is signed out; this one continues with a fresh token."""
    # A stolen token must not be able to guess the current password. Only WRONG guesses count: typos in the new password don't.
    guard = f"pwchange:{u.id}"
    try:
        if int(_r.get(guard) or 0) >= 5:
            raise ApiError(429, "too_many_attempts", "Too many incorrect attempts. Try again in 15 minutes.")
    except redis.RedisError:
        pass
    if not verify_password(body.current_password, u.password_hash):
        try:
            if _r.incr(guard) == 1:
                _r.expire(guard, 900)
        except redis.RedisError:
            pass
        raise ApiError(400, "incorrect_password", "Your current password is incorrect")
    strong(body.new_password)
    if body.new_password == body.current_password:
        raise ApiError(422, "same_password", "Choose a password different from your current one")
    u.password_hash = hash_password(body.new_password)
    u.token_version = (u.token_version or 0) + 1
    u.password_changed_at = datetime.now(timezone.utc)
    db.commit()
    try:
        _r.delete(guard)
    except redis.RedisError:
        pass
    return {"ok": True, "token": make_jwt(u.id, u.role, u.token_version), "message": "Password changed. Other devices have been signed out."}


@router.post("/forgot")
def forgot(body: Forgot, db: Session = Depends(get_db)):
    """Always answers the same way so the endpoint can't be used to discover which emails are registered."""
    u = db.scalar(select(User).where(User.email == body.email.lower()))
    out = {"ok": True, "message": "If that email exists, a reset link has been sent."}
    if u:
        token, h = new_reset_token()
        db.add(PasswordReset(user_id=u.id, token_hash=h, expires_at=datetime.now(timezone.utc) + timedelta(hours=1)))
        db.commit()
        # TODO(deploy): send `token` by email via your SMTP/provider instead of logging it.
        print(f"[password-reset] {u.email}: /reset?token={token}", flush=True)
        if settings.return_reset_token:
            out["dev_token"] = token
    return out


@router.post("/reset")
def reset(body: Reset, db: Session = Depends(get_db)):
    strong(body.password)
    pr = db.scalar(select(PasswordReset).where(PasswordReset.token_hash == sha256(body.token)))
    if not pr or pr.used_at or pr.expires_at < datetime.now(timezone.utc):
        raise ApiError(400, "invalid_token", "This reset link is invalid or has expired")
    u = db.get(User, pr.user_id)
    u.password_hash = hash_password(body.password)
    u.token_version = (u.token_version or 0) + 1  # a reset signs out every existing session
    u.password_changed_at = datetime.now(timezone.utc)
    pr.used_at = datetime.now(timezone.utc)
    db.commit()
    return {"ok": True}
