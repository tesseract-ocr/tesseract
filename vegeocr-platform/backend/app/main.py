from __future__ import annotations

import time
from contextlib import asynccontextmanager

from fastapi import FastAPI, HTTPException, Request
from fastapi.exceptions import RequestValidationError
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy import select, text

from .config import settings
from .db import Base, SessionLocal, engine
from .middleware.compression import CompressionMiddleware
from .errors import http_error_handler, validation_error_handler
from .models import ApiRequestLog, User
from .routers import admin, auth, keys, lab, learning_api, learning_web, public_api, web
from .security import hash_password
from .services import modelstore
from .services.learning_seed import seed_learning


@asynccontextmanager
async def lifespan(app: FastAPI):
    Base.metadata.create_all(engine)  # swap for Alembic migrations once the schema starts changing in production
    with engine.begin() as conn:  # lightweight upgrade for databases created before `username` existed
        conn.execute(text("ALTER TABLE users ADD COLUMN IF NOT EXISTS username VARCHAR(40)"))
        conn.execute(text("CREATE UNIQUE INDEX IF NOT EXISTS ix_users_username ON users (username)"))
        conn.execute(text("ALTER TABLE users ADD COLUMN IF NOT EXISTS token_version INTEGER NOT NULL DEFAULT 0"))
        conn.execute(text("ALTER TABLE users ADD COLUMN IF NOT EXISTS password_changed_at TIMESTAMPTZ"))
        # API-key management columns. Keys created before scopes existed keep working with the default scopes.
        for ddl in ("scopes JSON", "expires_at TIMESTAMPTZ", "enabled BOOLEAN NOT NULL DEFAULT TRUE", "last4 VARCHAR(4)", "rotated_at TIMESTAMPTZ"):
            conn.execute(text(f"ALTER TABLE api_keys ADD COLUMN IF NOT EXISTS {ddl}"))
        conn.execute(text("""UPDATE api_keys SET scopes = '["read","write"]'::json WHERE scopes IS NULL"""))
    with SessionLocal() as db:
        admin = db.scalar(select(User).where(User.email == settings.admin_email.lower()))
        if not admin:
            db.add(User(email=settings.admin_email.lower(), username=settings.admin_username.lower(),
                        password_hash=hash_password(settings.admin_password), full_name="Administrator", role="admin"))
            db.commit()
        elif not admin.username:
            admin.username = settings.admin_username.lower()
            db.commit()
        modelstore.ensure_seed(db)
        seed_learning(db)
    yield


app = FastAPI(title="VegeOCR API", version="1.0.0", lifespan=lifespan,
              description="OCR for images and PDFs (Japanese, Chinese, English). Authenticate with `X-API-Key`.")
app.add_middleware(CORSMiddleware, allow_origins=settings.cors_list, allow_methods=["*"], allow_headers=["*"],
                   expose_headers=["Content-Disposition", "X-RateLimit-Remaining"])
# Added last so it is outermost: it sees the final response (CORS headers included) of every route.
app.add_middleware(CompressionMiddleware)
app.add_exception_handler(HTTPException, http_error_handler)
app.add_exception_handler(RequestValidationError, validation_error_handler)


@app.middleware("http")
async def log_api_requests(request: Request, call_next):
    """Log every public API call (including rejected ones) for usage charts and quotas."""
    start = time.time()
    status = 500
    try:
        response = await call_next(request)
        status = response.status_code
        return response
    finally:
        if request.url.path.startswith("/api/v1/"):
            try:
                with SessionLocal() as db:
                    db.add(ApiRequestLog(api_key_id=getattr(request.state, "api_key_id", None),
                                         user_id=getattr(request.state, "user_id", None), method=request.method,
                                         path=request.url.path[:200], status_code=status,
                                         latency_ms=int((time.time() - start) * 1000)))
                    db.commit()
            except Exception:  # noqa: BLE001 - logging must never break a request
                pass


for r in (auth.router, web.router, keys.router, lab.router, learning_web.router, admin.router, public_api.router, learning_api.router):
    app.include_router(r)


@app.get("/health", tags=["meta"])
def health():
    return {"status": "ok"}
