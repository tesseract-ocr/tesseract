"""Learning API, authenticated by API key. Every route declares the scope it needs."""
from __future__ import annotations

from typing import Optional

from fastapi import APIRouter, Depends, Query
from sqlalchemy.orm import Session

from ..db import get_db
from ..deps import require_scope
from ..models import ApiKey
from ..schemas_learning import ProgressUpdate, QuizSubmit
from ..services import learning as svc

router = APIRouter(prefix="/api/v1", tags=["public API: learning"])
ERR = {401: {"description": "Missing / invalid / revoked / disabled / expired API key"}, 403: {"description": "The key lacks the required scope"},
       429: {"description": "Rate limit or quota exceeded"}}


@router.get("/modules", responses=ERR, summary="List learning modules (scope: learning:read)")
def list_modules(key: ApiKey = Depends(require_scope("learning:read")), db: Session = Depends(get_db)):
    return {"modules": svc.list_modules(db)}


@router.get("/modules/{module_id}", responses=ERR, summary="A module with its lessons and quizzes (scope: learning:read)")
def get_module(module_id: int, key: ApiKey = Depends(require_scope("learning:read")), db: Session = Depends(get_db)):
    """Lessons include their content and quiz questions. Correct answers are never included."""
    return svc.module_out(svc.get_module(db, module_id), detail=True, with_content=True)


@router.get("/progress", responses=ERR, summary="The key owner's learning progress (scope: learning:read)")
def get_progress(module_id: Optional[int] = Query(default=None, description="Limit to one module"),
                 key: ApiKey = Depends(require_scope("learning:read")), db: Session = Depends(get_db)):
    return svc.user_progress(db, key.user_id, module_id)


@router.post("/progress", responses=ERR, summary="Mark a lesson complete or incomplete (scope: learning:write)")
def update_progress(body: ProgressUpdate, key: ApiKey = Depends(require_scope("learning:write")), db: Session = Depends(get_db)):
    return svc.set_completion(db, key.user_id, body.lesson_id, body.completed)


@router.post("/quizzes/{quiz_id}/submit", responses=ERR, summary="Submit quiz answers and get a score (scope: learning:write)")
def submit_quiz(quiz_id: int, body: QuizSubmit, key: ApiKey = Depends(require_scope("learning:write")), db: Session = Depends(get_db)):
    """Unanswered questions count as wrong. Passing marks the lesson complete."""
    return svc.submit_quiz(db, key.user_id, quiz_id, body.as_dicts(), api_key_id=key.id)
