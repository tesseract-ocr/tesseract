from __future__ import annotations

from datetime import datetime
from typing import List, Optional

from fastapi import APIRouter, Depends
from fastapi.responses import Response
from pydantic import BaseModel, Field
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..db import get_db
from ..deps import current_user
from ..models import ApiKey, ApiRequestLog, User
from ..scopes import ALL_SCOPES, SCOPES
from ..errors import ApiError
from ..services import keys as svc

router = APIRouter(prefix="/api/web/keys", tags=["web: API keys"])


class KeyCreate(BaseModel):
    name: str = Field(min_length=1, max_length=80)
    scopes: Optional[List[str]] = Field(default=None, description="Defaults to read + write")
    expires_in_days: Optional[int] = Field(default=None, description="30, 90 or 365. Omit (and omit expires_at) for a key that never expires")
    expires_at: Optional[datetime] = Field(default=None, description="Custom expiration (ISO 8601)")


class KeyRename(BaseModel):
    name: str = Field(min_length=1, max_length=80)


@router.get("/scopes")
def scope_catalogue(user: User = Depends(current_user)):
    """Scopes the UI offers (admin is only offered to administrators)."""
    return [{"scope": s, "description": SCOPES[s]} for s in ALL_SCOPES if s != "admin" or user.role == "admin"]


@router.get("")
def list_keys(db: Session = Depends(get_db), user: User = Depends(current_user)):
    keys = db.scalars(select(ApiKey).where(ApiKey.user_id == user.id).order_by(ApiKey.id.desc())).all()
    counts = dict(db.execute(select(ApiRequestLog.api_key_id, func.count()).where(ApiRequestLog.api_key_id.in_([k.id for k in keys]))
                             .group_by(ApiRequestLog.api_key_id)).all()) if keys else {}
    return [svc.serialize(k, counts.get(k.id, 0)) for k in keys]


@router.post("", status_code=201)
def create_key(body: KeyCreate, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.create_key(db, user, body.name, body.scopes, body.expires_in_days, body.expires_at)


@router.patch("/{key_id}")
def rename_key(key_id: int, body: KeyRename, db: Session = Depends(get_db), user: User = Depends(current_user)):
    k = svc.own_key(db, key_id, user)
    k.name = body.name.strip()
    db.commit()
    return svc.serialize(k)


@router.post("/{key_id}/rotate")
def rotate_key(key_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.rotate_key(db, svc.own_key(db, key_id, user))


@router.post("/{key_id}/disable")
def disable_key(key_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    k = svc.own_key(db, key_id, user)
    k.enabled = False
    db.commit()
    return svc.serialize(k)


@router.post("/{key_id}/enable")
def enable_key(key_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    k = svc.own_key(db, key_id, user)
    if k.revoked_at:
        raise ApiError(409, "key_revoked", "A revoked key cannot be re-enabled")
    k.enabled = True
    db.commit()
    return svc.serialize(k)


@router.delete("/{key_id}", status_code=204)
def delete_key(key_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Permanent. Request history stays (it is kept without the key reference)."""
    db.delete(svc.own_key(db, key_id, user))
    db.commit()
    return Response(status_code=204)


@router.get("/{key_id}/usage")
def key_usage(key_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.usage(db, svc.own_key(db, key_id, user))
