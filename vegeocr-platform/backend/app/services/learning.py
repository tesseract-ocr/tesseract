"""Learning module logic, shared by the dashboard (JWT) routes and the public API (API-key) routes."""
from __future__ import annotations

from datetime import datetime, timezone
from typing import Dict, List, Optional

from sqlalchemy import select
from sqlalchemy.orm import Session, selectinload

from ..errors import ApiError
from ..models import LearningModule, Lesson, LessonProgress, Quiz, QuizAttempt, QuizQuestion


def _now():
    return datetime.now(timezone.utc)


# ---- shapes ---------------------------------------------------------------------------------------------------

def quiz_out(q: Quiz) -> dict:
    """What a learner may see. `correct` and `explanation` are deliberately absent until they submit."""
    return {"id": q.id, "title": q.title, "pass_percent": q.pass_percent,
            "questions": [{"id": x.id, "prompt": x.prompt, "kind": x.kind, "options": x.options} for x in q.questions]}


def lesson_out(l: Lesson, with_content: bool, with_quiz: bool) -> dict:
    out = {"id": l.id, "position": l.position, "title": l.title, "minutes": l.minutes,
           "has_quiz": l.quiz is not None, "quiz_id": l.quiz.id if l.quiz else None}
    if with_content:
        out["content"] = l.content
    if with_quiz and l.quiz:
        out["quiz"] = quiz_out(l.quiz)
    return out


def module_out(m: LearningModule, detail: bool = False, with_content: bool = False) -> dict:
    out = {"id": m.id, "slug": m.slug, "title": m.title, "topic": m.topic, "difficulty": m.difficulty, "summary": m.summary,
           "lesson_count": len(m.lessons), "estimated_minutes": sum(l.minutes for l in m.lessons)}
    if detail:
        out["lessons"] = [lesson_out(l, with_content, with_quiz=with_content) for l in m.lessons]
    return out


def _published_modules(db: Session) -> List[LearningModule]:
    return db.scalars(select(LearningModule).where(LearningModule.published.is_(True)).order_by(LearningModule.position, LearningModule.id)
                      .options(selectinload(LearningModule.lessons).selectinload(Lesson.quiz).selectinload(Quiz.questions))).all()


def get_module(db: Session, module_id: int) -> LearningModule:
    m = db.scalar(select(LearningModule).where(LearningModule.id == module_id, LearningModule.published.is_(True))
                  .options(selectinload(LearningModule.lessons).selectinload(Lesson.quiz).selectinload(Quiz.questions)))
    if not m:
        raise ApiError(404, "module_not_found", "No such module")
    return m


def list_modules(db: Session) -> List[dict]:
    return [module_out(m) for m in _published_modules(db)]


# ---- progress -------------------------------------------------------------------------------------------------

def _rows_by_lesson(db: Session, user_id: int) -> Dict[int, LessonProgress]:
    return {r.lesson_id: r for r in db.scalars(select(LessonProgress).where(LessonProgress.user_id == user_id))}


def _module_progress(m: LearningModule, rows: Dict[int, LessonProgress]) -> dict:
    lessons = m.lessons
    mine = [rows[l.id] for l in lessons if l.id in rows]
    done = [r for r in mine if r.completed_at]
    total = len(lessons)
    percent = (100 * len(done)) // total if total else 0
    if total and len(done) == total:
        status = "completed"
    elif mine:
        status = "in_progress"
    else:
        status = "not_started"

    def activity(r: LessonProgress):
        return max([t for t in (r.last_viewed_at, r.completed_at) if t], default=None)

    last = max(mine, key=lambda r: activity(r) or datetime.min.replace(tzinfo=timezone.utc), default=None)
    resume: Optional[int] = None
    if status != "completed":
        order = [l.id for l in lessons]
        if last and not last.completed_at:
            resume = last.lesson_id  # you opened it but didn't finish: go back to it
        else:
            after = order[order.index(last.lesson_id) + 1:] if last else order  # otherwise the next unfinished lesson
            resume = next((i for i in after if not (i in rows and rows[i].completed_at)), None) \
                or next((i for i in order if not (i in rows and rows[i].completed_at)), None)
    return {
        "module_id": m.id, "title": m.title, "status": status, "percent": percent, "lessons_completed": len(done), "lessons_total": total,
        "last_activity_at": activity(last) if last else None, "continue_lesson_id": resume,
        "lessons": [{"lesson_id": l.id, "completed": bool(rows.get(l.id) and rows[l.id].completed_at),
                     "completed_at": rows[l.id].completed_at if l.id in rows else None} for l in lessons],
    }


def user_progress(db: Session, user_id: int, module_id: Optional[int] = None) -> dict:
    rows = _rows_by_lesson(db, user_id)
    modules = _published_modules(db)
    if module_id is not None:
        modules = [m for m in modules if m.id == module_id]
        if not modules:
            raise ApiError(404, "module_not_found", "No such module")
    per = [_module_progress(m, rows) for m in modules]
    lesson_titles = {l.id: l.title for m in modules for l in m.lessons}
    active = [p for p in per if p["status"] == "in_progress" and p["last_activity_at"]]
    best = max(active, key=lambda p: p["last_activity_at"], default=None)
    return {
        "modules": per,
        "summary": {"modules_total": len(per), "completed": sum(p["status"] == "completed" for p in per),
                    "in_progress": sum(p["status"] == "in_progress" for p in per), "not_started": sum(p["status"] == "not_started" for p in per)},
        "continue": ({"module_id": best["module_id"], "module_title": best["title"], "lesson_id": best["continue_lesson_id"],
                      "lesson_title": lesson_titles.get(best["continue_lesson_id"])} if best and best["continue_lesson_id"] else None),
    }


def _lesson(db: Session, lesson_id: int) -> Lesson:
    l = db.scalar(select(Lesson).join(LearningModule).where(Lesson.id == lesson_id, LearningModule.published.is_(True))
                  .options(selectinload(Lesson.quiz).selectinload(Quiz.questions), selectinload(Lesson.module).selectinload(LearningModule.lessons)))
    if not l:
        raise ApiError(404, "lesson_not_found", "No such lesson")
    return l


def _upsert(db: Session, user_id: int, lesson_id: int) -> LessonProgress:
    r = db.scalar(select(LessonProgress).where(LessonProgress.user_id == user_id, LessonProgress.lesson_id == lesson_id))
    if not r:
        r = LessonProgress(user_id=user_id, lesson_id=lesson_id)
        db.add(r)
    return r


def set_completion(db: Session, user_id: int, lesson_id: int, completed: bool) -> dict:
    lesson = _lesson(db, lesson_id)
    r = _upsert(db, user_id, lesson_id)
    now = _now()
    r.last_viewed_at = now
    if completed:
        r.completed_at = r.completed_at or now
    else:
        r.completed_at = None
    db.commit()
    progress = _module_progress(get_module(db, lesson.module_id), _rows_by_lesson(db, user_id))
    return {"lesson_id": lesson_id, "completed": completed, "module": progress}


def open_lesson(db: Session, user_id: int, lesson_id: int) -> dict:
    """Dashboard: returns the lesson (with quiz questions, without answers) and records it as the resume point."""
    lesson = _lesson(db, lesson_id)
    r = _upsert(db, user_id, lesson_id)
    r.last_viewed_at = _now()
    db.commit()
    siblings = lesson.module.lessons
    idx = [l.id for l in siblings].index(lesson.id)
    return {
        **lesson_out(lesson, with_content=True, with_quiz=True),
        "module": {"id": lesson.module.id, "title": lesson.module.title},
        "completed": bool(r.completed_at),
        "prev_lesson_id": siblings[idx - 1].id if idx > 0 else None,
        "next_lesson_id": siblings[idx + 1].id if idx + 1 < len(siblings) else None,
        "last_attempt": _last_attempt(db, user_id, lesson.quiz.id) if lesson.quiz else None,
    }


def _last_attempt(db: Session, user_id: int, quiz_id: int) -> Optional[dict]:
    a = db.scalar(select(QuizAttempt).where(QuizAttempt.user_id == user_id, QuizAttempt.quiz_id == quiz_id).order_by(QuizAttempt.id.desc()))
    return {"score": a.score, "passed": a.passed, "at": a.created_at} if a else None


# ---- quizzes --------------------------------------------------------------------------------------------------

def submit_quiz(db: Session, user_id: int, quiz_id: int, answers: List[dict], api_key_id: Optional[int] = None) -> dict:
    quiz = db.scalar(select(Quiz).where(Quiz.id == quiz_id).options(selectinload(Quiz.questions)))
    lesson = db.get(Lesson, quiz.lesson_id) if quiz else None
    if not quiz or not lesson or not db.get(LearningModule, lesson.module_id).published:
        raise ApiError(404, "quiz_not_found", "No such quiz")
    by_id = {q.id: q for q in quiz.questions}
    given: Dict[int, List[int]] = {}
    for a in answers:
        qid = a["question_id"]
        q = by_id.get(qid)
        if not q:
            raise ApiError(422, "invalid_answers", f"Question {qid} does not belong to this quiz")
        if qid in given:
            raise ApiError(422, "invalid_answers", f"Question {qid} was answered twice")
        sel = sorted(set(a["selected"]))
        if any(i < 0 or i >= len(q.options) for i in sel):
            raise ApiError(422, "invalid_answers", f"Question {qid}: option index out of range (0-{len(q.options) - 1})")
        if q.kind == "single" and len(sel) > 1:
            raise ApiError(422, "invalid_answers", f"Question {qid} accepts a single answer")
        given[qid] = sel

    results, correct = [], 0
    for q in quiz.questions:  # unanswered questions count as wrong
        ok = sorted(set(q.correct)) == given.get(q.id, [])
        correct += ok
        results.append({"question_id": q.id, "correct": ok, "explanation": q.explanation})
    total = len(quiz.questions)
    score = round(100 * correct / total) if total else 0
    passed = score >= quiz.pass_percent
    db.add(QuizAttempt(user_id=user_id, quiz_id=quiz.id, api_key_id=api_key_id, correct=correct, total=total, score=score, passed=passed,
                       answers=[{"question_id": k, "selected": v} for k, v in given.items()]))
    db.commit()
    if passed:  # passing the quiz completes the lesson
        set_completion(db, user_id, lesson.id, True)
    return {"quiz_id": quiz.id, "lesson_id": lesson.id, "score": score, "correct": correct, "total": total, "passed": passed,
            "pass_percent": quiz.pass_percent, "lesson_completed": passed, "results": results}
