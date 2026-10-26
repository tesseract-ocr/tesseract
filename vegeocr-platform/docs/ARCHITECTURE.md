# VegeOCR architecture

```
 Browser ── Next.js + Tailwind (user app + /admin) ──┐
 External apps ── X-API-Key ─────────────────────────┤
                                                      ▼
                        FastAPI  (JWT for web users, hashed API keys for /api/v1)
                        ├─ auth · OCR jobs · history · corrections · export
                        ├─ API keys · rate limit (Redis) · monthly quota (DB) · request log
                        └─ admin: users, keys, usage, models, training data, retrain / rollback
                           │                │                 │
                      PostgreSQL        Redis (Celery       Storage volume
                      (all state)       broker + rate       uploads/ training/ models/
                                        counters)           runtime_tessdata/ train_work/
                                            │
                                  Celery workers (Tesseract inside)
                                  ├─ queue "ocr":      large / async OCR jobs
                                  └─ queue "training": fine-tune → evaluate → deploy-if-better
                                  Celery beat (embedded): auto-retrain check every 10 min
```

## Request flows

**OCR.** `POST /api/v1/ocr` (or `/api/web/jobs`) validates the upload by *content* (magic bytes, page count, size,
pixel count), stores it, creates an `ocr_jobs` row, then either runs OCR inline (small file) or queues it on Celery
(`async=true`, PDF > 2 pages, file > 5 MB) and returns `202` + a job id. PDFs with a text layer are read directly;
scanned pages are rendered at 300 dpi and OCR'd. Output is per-line text, confidence and bounding box.

**Corrections → training data.** In the UI a user edits individual OCR lines. Each changed line is cropped from
the page image using its bounding box and stored as a `training_samples` row (`pending`). External apps can submit
`(line image, text)` pairs via `POST /api/v1/training-data`. Nothing is trained on until an admin approves it.

**Auto-training** (`app/services/training.py`):
1. Approved samples are split train / test (~10 % hold-out, deterministic; identical texts always share a side so
   the test set is never a copy of the training set).
2. Each training sample becomes an `.lstmf` (Tesseract `lstm.train`).
3. `lstmtraining --continue_from` fine-tunes the **base** model with all approved training samples.
4. The candidate and the live model are both scored on the held-out lines (character error rate, whitespace ignored).
5. The candidate is **deployed only if its CER is lower** (by at least `min_improvement`). Otherwise it is stored as
   `rejected` and the live model is untouched.
6. Triggers: admin "Retrain now"; ≥ `retrain_threshold` new approved samples; or `retrain_schedule_hours` elapsed with
   any new sample. A DB unique index guarantees one active run per language.

**Model versions & rollback.** Every run writes `models/<lang>/v<N>/<lang>.traineddata` and a `model_versions` row
(status `active | candidate | rejected | archived`). All OCR reads one shared `runtime_tessdata/` directory whose
`<lang>.traineddata` symlink points at the active version. Activating any version, including an older one (rollback),
swaps that symlink atomically; no restart is needed.

## Security model
* Passwords: bcrypt. API keys: 256-bit random, only the SHA-256 is stored, shown once. Reset tokens: hashed, 1 h, single use.
* Web users get a JWT; every route re-checks the user is still active (suspending takes effect immediately).
* Role checks are server-side (`/api/admin/*` requires `role == admin`). Users only ever see their own jobs/keys
  (a foreign id answers `404`, same as a missing one).
* Login throttling (10 / 15 min per IP + email), per-key rate limit, per-key monthly quota.
* Uploads: type sniffed from content, size / page / pixel limits, no user-controlled paths.

## Database
See [`schema.sql`](schema.sql). Tables: `users`, `password_resets`, `api_keys`, `ocr_jobs`, `training_samples`,
`model_versions`, `training_runs`, `api_request_logs`, `settings`.
