"""Learning module for the dashboard (JWT-authenticated users)."""
from __future__ import annotations

from typing import Optional

from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from ..db import get_db
from ..deps import current_user
from ..models import User
from ..schemas_learning import ProgressUpdate, QuizSubmit
from ..services import learning as svc

router = APIRouter(prefix="/api/web/learning", tags=["web: learning"])


@router.get("/modules")
def modules(db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Every module with this user's progress merged in."""
    progress = {p["module_id"]: p for p in svc.user_progress(db, user.id)["modules"]}
    return [{**m, "progress": {k: progress[m["id"]][k] for k in ("status", "percent", "lessons_completed", "lessons_total", "continue_lesson_id")}}
            for m in svc.list_modules(db)]


@router.get("/modules/{module_id}")
def module(module_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    out = svc.module_out(svc.get_module(db, module_id), detail=True)
    out["progress"] = svc.user_progress(db, user.id, module_id)["modules"][0]
    return out


@router.get("/lessons/{lesson_id}")
def lesson(lesson_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Opening a lesson records it as your resume point."""
    return svc.open_lesson(db, user.id, lesson_id)


@router.post("/lessons/{lesson_id}/complete")
def complete(lesson_id: int, completed: bool = True, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.set_completion(db, user.id, lesson_id, completed)


@router.post("/quizzes/{quiz_id}/submit")
def submit(quiz_id: int, body: QuizSubmit, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.submit_quiz(db, user.id, quiz_id, body.as_dicts())


@router.get("/progress")
def progress(db: Session = Depends(get_db), user: User = Depends(current_user)):
    return svc.user_progress(db, user.id)
