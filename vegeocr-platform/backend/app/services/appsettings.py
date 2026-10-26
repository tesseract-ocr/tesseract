"""Admin-editable runtime settings stored in the DB (falls back to env defaults)."""
from sqlalchemy.orm import Session

from ..config import settings
from ..models import Setting


def defaults():
    return {
        "retrain_threshold": settings.retrain_threshold,
        "retrain_min_test_samples": settings.retrain_min_test_samples,
        "retrain_max_iterations": settings.retrain_max_iterations,
        "retrain_schedule_hours": settings.retrain_schedule_hours,
        "auto_retrain_enabled": True,
        "min_improvement": 0.0,
    }


def get_all(db: Session) -> dict:
    out = defaults()
    row = db.get(Setting, "training")
    if row:
        out.update(row.value)
    return out


def update(db: Session, patch: dict) -> dict:
    cur = get_all(db)
    cur.update(patch)
    row = db.get(Setting, "training")
    if row:
        row.value = cur
    else:
        db.add(Setting(key="training", value=cur))
    db.commit()
    return cur
