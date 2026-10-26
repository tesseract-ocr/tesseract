from __future__ import annotations

import uuid
from datetime import datetime, timezone
from typing import Optional

from sqlalchemy import (JSON, Boolean, DateTime, Float, ForeignKey, Index, Integer, String, Text, text)
from sqlalchemy.orm import Mapped, mapped_column, relationship

from .db import Base


def now():
    return datetime.now(timezone.utc)


def new_id():
    return uuid.uuid4().hex


class User(Base):
    __tablename__ = "users"
    id: Mapped[int] = mapped_column(primary_key=True)
    email: Mapped[str] = mapped_column(String(255), unique=True, index=True)
    username: Mapped[Optional[str]] = mapped_column(String(40), unique=True, index=True)  # optional alternative login name
    password_hash: Mapped[str] = mapped_column(String(100))
    full_name: Mapped[str] = mapped_column(String(120), default="")
    role: Mapped[str] = mapped_column(String(10), default="user")  # user | admin
    status: Mapped[str] = mapped_column(String(10), default="active")  # active | suspended
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    last_login_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    # Bumped whenever the password changes. Tokens carry the version they were issued under, so a change signs out every other session.
    token_version: Mapped[int] = mapped_column(Integer, default=0, server_default=text("0"))
    password_changed_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    api_keys = relationship("ApiKey", back_populates="user", cascade="all, delete-orphan")


class PasswordReset(Base):
    __tablename__ = "password_resets"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    token_hash: Mapped[str] = mapped_column(String(64), index=True)
    expires_at: Mapped[datetime] = mapped_column(DateTime(timezone=True))
    used_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))


class ApiKey(Base):
    __tablename__ = "api_keys"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    name: Mapped[str] = mapped_column(String(80))
    prefix: Mapped[str] = mapped_column(String(12))
    key_hash: Mapped[str] = mapped_column(String(64), unique=True, index=True)  # sha256 of the full key
    rate_limit_per_min: Mapped[int] = mapped_column(Integer, default=60)
    monthly_quota: Mapped[int] = mapped_column(Integer, default=1000)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    last_used_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    revoked_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))  # permanent (admin) revoke
    scopes: Mapped[Optional[list]] = mapped_column(JSON)  # NULL = legacy key => default scopes
    expires_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    enabled: Mapped[bool] = mapped_column(Boolean, default=True, server_default=text("true"))  # reversible on/off switch
    last4: Mapped[Optional[str]] = mapped_column(String(4))  # for the masked display; the secret itself is never stored
    rotated_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    user = relationship("User", back_populates="api_keys")


class ModelVersion(Base):
    __tablename__ = "model_versions"
    id: Mapped[int] = mapped_column(primary_key=True)
    lang: Mapped[str] = mapped_column(String(20), index=True)
    version: Mapped[int] = mapped_column(Integer)
    path: Mapped[str] = mapped_column(String(500))
    # active | candidate | rejected | archived
    status: Mapped[str] = mapped_column(String(12), default="candidate")
    trigger: Mapped[str] = mapped_column(String(12), default="manual")  # seed|manual|auto|schedule
    cer: Mapped[Optional[float]] = mapped_column(Float)  # character error rate on the test set
    baseline_cer: Mapped[Optional[float]] = mapped_column(Float)
    train_samples: Mapped[int] = mapped_column(Integer, default=0)
    test_samples: Mapped[int] = mapped_column(Integer, default=0)
    notes: Mapped[str] = mapped_column(Text, default="")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    activated_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    __table_args__ = (Index("ix_model_lang_version", "lang", "version", unique=True),)


class OcrJob(Base):
    __tablename__ = "ocr_jobs"
    id: Mapped[str] = mapped_column(String(32), primary_key=True, default=new_id)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    api_key_id: Mapped[Optional[int]] = mapped_column(ForeignKey("api_keys.id", ondelete="SET NULL"))
    source: Mapped[str] = mapped_column(String(8), default="web")  # web | api
    filename: Mapped[str] = mapped_column(String(255))
    mime: Mapped[str] = mapped_column(String(60))
    file_path: Mapped[str] = mapped_column(String(500))
    lang: Mapped[str] = mapped_column(String(40))
    status: Mapped[str] = mapped_column(String(12), default="queued")  # queued|processing|done|failed
    error: Mapped[Optional[str]] = mapped_column(Text)
    page_count: Mapped[int] = mapped_column(Integer, default=0)
    text: Mapped[str] = mapped_column(Text, default="")
    edited_text: Mapped[Optional[str]] = mapped_column(Text)
    avg_confidence: Mapped[Optional[float]] = mapped_column(Float)
    result: Mapped[Optional[dict]] = mapped_column(JSON)  # {"pages":[{"page":1,"image":..,"lines":[..]}]}
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now, index=True)
    finished_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))


class TrainingSample(Base):
    __tablename__ = "training_samples"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[Optional[int]] = mapped_column(ForeignKey("users.id", ondelete="SET NULL"))
    api_key_id: Mapped[Optional[int]] = mapped_column(ForeignKey("api_keys.id", ondelete="SET NULL"))
    job_id: Mapped[Optional[str]] = mapped_column(String(32))
    source: Mapped[str] = mapped_column(String(12))  # correction | api
    lang: Mapped[str] = mapped_column(String(20), index=True)
    image_path: Mapped[str] = mapped_column(String(500))
    text: Mapped[str] = mapped_column(Text)
    original_text: Mapped[Optional[str]] = mapped_column(Text)
    status: Mapped[str] = mapped_column(String(10), default="pending", index=True)  # pending|approved|rejected
    is_test: Mapped[bool] = mapped_column(Boolean, default=False)
    used_in_version_id: Mapped[Optional[int]] = mapped_column(ForeignKey("model_versions.id", ondelete="SET NULL"))
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    reviewed_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))


class TrainingRun(Base):
    __tablename__ = "training_runs"
    id: Mapped[int] = mapped_column(primary_key=True)
    lang: Mapped[str] = mapped_column(String(20))
    status: Mapped[str] = mapped_column(String(10), default="queued")  # queued|running|deployed|rejected|failed
    trigger: Mapped[str] = mapped_column(String(12), default="manual")
    requested_by: Mapped[Optional[int]] = mapped_column(ForeignKey("users.id", ondelete="SET NULL"))
    started_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    finished_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    version_id: Mapped[Optional[int]] = mapped_column(ForeignKey("model_versions.id", ondelete="SET NULL"))
    message: Mapped[str] = mapped_column(Text, default="")
    # At most one queued/running run per language, enforced by the database (no check-then-insert race).
    __table_args__ = (Index("uq_one_active_run_per_lang", "lang", unique=True,
                            postgresql_where=text("status in ('queued','running')")),)


class ApiRequestLog(Base):
    __tablename__ = "api_request_logs"
    id: Mapped[int] = mapped_column(primary_key=True)
    api_key_id: Mapped[Optional[int]] = mapped_column(ForeignKey("api_keys.id", ondelete="SET NULL"), index=True)
    user_id: Mapped[Optional[int]] = mapped_column(ForeignKey("users.id", ondelete="SET NULL"), index=True)
    method: Mapped[str] = mapped_column(String(8))
    path: Mapped[str] = mapped_column(String(200))
    status_code: Mapped[int] = mapped_column(Integer)
    latency_ms: Mapped[int] = mapped_column(Integer, default=0)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now, index=True)


class Setting(Base):
    __tablename__ = "settings"
    key: Mapped[str] = mapped_column(String(60), primary_key=True)
    value: Mapped[dict] = mapped_column(JSON)


# ---- learning module ------------------------------------------------------------------------------------------

class LearningModule(Base):
    __tablename__ = "learning_modules"
    id: Mapped[int] = mapped_column(primary_key=True)
    slug: Mapped[str] = mapped_column(String(80), unique=True)
    title: Mapped[str] = mapped_column(String(160))
    topic: Mapped[str] = mapped_column(String(60), index=True)
    difficulty: Mapped[str] = mapped_column(String(14))  # beginner | intermediate | advanced
    summary: Mapped[str] = mapped_column(Text, default="")
    position: Mapped[int] = mapped_column(Integer, default=0)
    published: Mapped[bool] = mapped_column(Boolean, default=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    lessons = relationship("Lesson", back_populates="module", order_by="Lesson.position", cascade="all, delete-orphan")


class Lesson(Base):
    __tablename__ = "lessons"
    id: Mapped[int] = mapped_column(primary_key=True)
    module_id: Mapped[int] = mapped_column(ForeignKey("learning_modules.id", ondelete="CASCADE"), index=True)
    position: Mapped[int] = mapped_column(Integer)
    title: Mapped[str] = mapped_column(String(160))
    content: Mapped[str] = mapped_column(Text, default="")  # light markdown
    minutes: Mapped[int] = mapped_column(Integer, default=5)
    module = relationship("LearningModule", back_populates="lessons")
    quiz = relationship("Quiz", back_populates="lesson", uselist=False, cascade="all, delete-orphan")


class Quiz(Base):
    __tablename__ = "quizzes"
    id: Mapped[int] = mapped_column(primary_key=True)
    lesson_id: Mapped[int] = mapped_column(ForeignKey("lessons.id", ondelete="CASCADE"), unique=True)
    title: Mapped[str] = mapped_column(String(160), default="Check your understanding")
    pass_percent: Mapped[int] = mapped_column(Integer, default=70)
    lesson = relationship("Lesson", back_populates="quiz")
    questions = relationship("QuizQuestion", back_populates="quiz", order_by="QuizQuestion.position", cascade="all, delete-orphan")


class QuizQuestion(Base):
    __tablename__ = "quiz_questions"
    id: Mapped[int] = mapped_column(primary_key=True)
    quiz_id: Mapped[int] = mapped_column(ForeignKey("quizzes.id", ondelete="CASCADE"), index=True)
    position: Mapped[int] = mapped_column(Integer)
    prompt: Mapped[str] = mapped_column(Text)
    kind: Mapped[str] = mapped_column(String(10), default="single")  # single | multiple
    options: Mapped[list] = mapped_column(JSON)  # list[str]
    correct: Mapped[list] = mapped_column(JSON)  # list[int]: NEVER sent to clients
    explanation: Mapped[str] = mapped_column(Text, default="")
    quiz = relationship("Quiz", back_populates="questions")


class QuizAttempt(Base):
    __tablename__ = "quiz_attempts"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    quiz_id: Mapped[int] = mapped_column(ForeignKey("quizzes.id", ondelete="CASCADE"), index=True)
    api_key_id: Mapped[Optional[int]] = mapped_column(ForeignKey("api_keys.id", ondelete="SET NULL"))
    correct: Mapped[int] = mapped_column(Integer)
    total: Mapped[int] = mapped_column(Integer)
    score: Mapped[int] = mapped_column(Integer)  # percent
    passed: Mapped[bool] = mapped_column(Boolean)
    answers: Mapped[list] = mapped_column(JSON)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)


class LessonProgress(Base):
    __tablename__ = "lesson_progress"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    lesson_id: Mapped[int] = mapped_column(ForeignKey("lessons.id", ondelete="CASCADE"), index=True)
    last_viewed_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    completed_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    __table_args__ = (Index("uq_progress_user_lesson", "user_id", "lesson_id", unique=True),)


# ---- OCR lab: extraction runs, user datasets, per-dataset models ----------------------------------------------------

class LabRun(Base):
    """One extraction (handwriting / printed, optionally enhanced). Images live in storage/lab/<id>/."""
    __tablename__ = "lab_runs"
    id: Mapped[str] = mapped_column(String(32), primary_key=True, default=new_id)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    filename: Mapped[str] = mapped_column(String(255))
    mime: Mapped[str] = mapped_column(String(60))
    mode: Mapped[str] = mapped_column(String(12))            # printed | handwriting
    lang: Mapped[str] = mapped_column(String(40))
    enhance: Mapped[str] = mapped_column(String(6), default="auto")  # auto | on | off
    engine: Mapped[str] = mapped_column(String(12), default="auto")  # auto | tesseract | trocr
    dataset_id: Mapped[Optional[int]] = mapped_column(ForeignKey("datasets.id", ondelete="SET NULL"))  # use this dataset's active model
    status: Mapped[str] = mapped_column(String(12), default="queued")  # queued | processing | done | failed
    error_code: Mapped[Optional[str]] = mapped_column(String(40))
    error: Mapped[Optional[str]] = mapped_column(Text)
    result: Mapped[Optional[dict]] = mapped_column(JSON)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now, index=True)
    finished_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))


class Dataset(Base):
    """A user's own labelled samples (line image + exact text), grouped by name and category."""
    __tablename__ = "datasets"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), index=True)
    name: Mapped[str] = mapped_column(String(80))
    category: Mapped[str] = mapped_column(String(40), default="general", index=True)
    description: Mapped[str] = mapped_column(Text, default="")
    lang: Mapped[str] = mapped_column(String(20), default="eng")      # the Tesseract language this dataset fine-tunes
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    updated_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now, onupdate=now)
    samples = relationship("DatasetSample", back_populates="dataset", cascade="all, delete-orphan", order_by="DatasetSample.id")
    models = relationship("DatasetModel", back_populates="dataset", cascade="all, delete-orphan", order_by="DatasetModel.version.desc()")
    __table_args__ = (Index("uq_dataset_user_name", "user_id", "name", unique=True),)


class DatasetSample(Base):
    __tablename__ = "dataset_samples"
    id: Mapped[int] = mapped_column(primary_key=True)
    dataset_id: Mapped[int] = mapped_column(ForeignKey("datasets.id", ondelete="CASCADE"), index=True)
    image_path: Mapped[str] = mapped_column(String(500))
    text: Mapped[str] = mapped_column(Text)
    original_text: Mapped[Optional[str]] = mapped_column(Text)       # what the engine read, for corrections
    source: Mapped[str] = mapped_column(String(12), default="upload")  # upload | import | correction
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    updated_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now, onupdate=now)
    dataset = relationship("Dataset", back_populates="samples")


class DatasetModel(Base):
    """A fine-tuned model version for one dataset, with its measured accuracy before and after."""
    __tablename__ = "dataset_models"
    id: Mapped[int] = mapped_column(primary_key=True)
    dataset_id: Mapped[int] = mapped_column(ForeignKey("datasets.id", ondelete="CASCADE"), index=True)
    version: Mapped[int] = mapped_column(Integer)
    status: Mapped[str] = mapped_column(String(10), default="queued")  # queued | training | ready | no_gain | failed
    path: Mapped[Optional[str]] = mapped_column(String(500))
    cer_before: Mapped[Optional[float]] = mapped_column(Float)         # character error rate on held-out samples, before training
    cer_after: Mapped[Optional[float]] = mapped_column(Float)
    train_samples: Mapped[int] = mapped_column(Integer, default=0)
    test_samples: Mapped[int] = mapped_column(Integer, default=0)
    iterations: Mapped[int] = mapped_column(Integer, default=0)
    active: Mapped[bool] = mapped_column(Boolean, default=False)
    message: Mapped[str] = mapped_column(Text, default="")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=now)
    finished_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True))
    dataset = relationship("Dataset", back_populates="models")
    # one model in flight per dataset, enforced by the database
    __table_args__ = (Index("uq_dataset_model_version", "dataset_id", "version", unique=True),
                      Index("uq_one_training_per_dataset", "dataset_id", unique=True, postgresql_where=text("status in ('queued','training')")))
