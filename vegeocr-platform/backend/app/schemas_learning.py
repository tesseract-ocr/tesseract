from __future__ import annotations

from typing import List

from pydantic import BaseModel, Field


class ProgressUpdate(BaseModel):
    lesson_id: int = Field(ge=1)
    completed: bool = Field(default=True, description="true marks the lesson complete, false clears it")


class AnswerIn(BaseModel):
    question_id: int = Field(ge=1)
    selected: List[int] = Field(max_length=20, description="Indexes of the chosen options (0-based)")


class QuizSubmit(BaseModel):
    answers: List[AnswerIn] = Field(max_length=100)

    def as_dicts(self):
        return [a.model_dump() for a in self.answers]
