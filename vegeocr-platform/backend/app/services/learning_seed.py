from __future__ import annotations

from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..learning_content import MODULES
from ..models import LearningModule, Lesson, Quiz, QuizQuestion


def seed_learning(db: Session) -> int:
    """Insert every curriculum module whose slug is not in the database yet (so new modules reach existing installs too).
    Existing modules are never modified, so edits made in the database are kept. Returns how many were created."""
    have = set(db.scalars(select(LearningModule.slug)))
    created = 0
    for m in MODULES:
        if m["slug"] in have:
            continue
        mod = LearningModule(slug=m["slug"], title=m["title"], topic=m["topic"], difficulty=m["difficulty"], summary=m["summary"], position=m["position"])
        for i, l in enumerate(m["lessons"], 1):
            lesson = Lesson(position=i, title=l["title"], content=l["content"], minutes=l.get("minutes", 5))
            if l.get("quiz"):
                q = l["quiz"]
                lesson.quiz = Quiz(title=q["title"], pass_percent=q.get("pass_percent", 70), questions=[
                    QuizQuestion(position=j, prompt=x["prompt"], kind=x["kind"], options=x["options"], correct=x["correct"], explanation=x.get("explanation", ""))
                    for j, x in enumerate(q["questions"], 1)])
            mod.lessons.append(lesson)
        db.add(mod)
        created += 1
    db.commit()
    return created
