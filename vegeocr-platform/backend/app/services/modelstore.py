"""Model version registry. One shared runtime tessdata dir is assembled from the base models plus the
currently-active fine-tuned version of each language; activating/rolling back just repoints a symlink."""
from __future__ import annotations

import os
import shutil
from datetime import datetime, timezone
from pathlib import Path

from sqlalchemy import select
from sqlalchemy.orm import Session

from ..config import settings
from ..models import ModelVersion

RUNTIME = settings.storage_dir / "runtime_tessdata"


def _link(src: Path, dst: Path):
    tmp = dst.with_name(dst.name + ".tmp")
    if tmp.exists() or tmp.is_symlink():
        tmp.unlink()
    os.symlink(src.resolve(), tmp)
    os.replace(tmp, dst)  # atomic swap so running OCR never sees a missing file


def sync_runtime(db: Session):
    RUNTIME.mkdir(parents=True, exist_ok=True)
    for f in settings.base_tessdata.glob("*.traineddata"):
        _link(f, RUNTIME / f.name)
    cfg = settings.base_tessdata / "configs"
    if cfg.exists() and not (RUNTIME / "configs").exists():
        os.symlink(cfg.resolve(), RUNTIME / "configs")
    for mv in db.scalars(select(ModelVersion).where(ModelVersion.status == "active")):
        _link(Path(mv.path), RUNTIME / f"{mv.lang}.traineddata")


def ensure_seed(db: Session):
    """Register the base models as version 0 so there is always something to roll back to."""
    for lang in settings.trainable_list:
        base = settings.base_tessdata / f"{lang}.traineddata"
        if not base.exists():
            continue
        if db.scalar(select(ModelVersion).where(ModelVersion.lang == lang)):
            continue
        db.add(ModelVersion(lang=lang, version=0, path=str(base.resolve()), status="active", trigger="seed",
                            notes="Base model", activated_at=datetime.now(timezone.utc)))
    db.commit()
    sync_runtime(db)


def active_version(db: Session, lang: str):
    return db.scalar(select(ModelVersion).where(ModelVersion.lang == lang, ModelVersion.status == "active"))


def activate(db: Session, mv: ModelVersion):
    for other in db.scalars(select(ModelVersion).where(ModelVersion.lang == mv.lang, ModelVersion.status == "active")):
        other.status = "archived"
    mv.status = "active"
    mv.activated_at = datetime.now(timezone.utc)
    db.commit()
    sync_runtime(db)


def next_version_number(db: Session, lang: str) -> int:
    last = db.scalar(select(ModelVersion.version).where(ModelVersion.lang == lang).order_by(ModelVersion.version.desc()))
    return (last if last is not None else -1) + 1


def version_dir(lang: str, version: int) -> Path:
    d = settings.storage_dir / "models" / lang / f"v{version}"
    d.mkdir(parents=True, exist_ok=True)
    return d
