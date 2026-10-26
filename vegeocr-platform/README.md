# VegeOCR platform

OCR for images and PDFs (Japanese, Simplified/Traditional Chinese, English) with a user web app, an admin dashboard,
a public REST API and an auto-training pipeline built on Tesseract 5.

```
vegeocr-platform/
├─ backend/          FastAPI app (app/), Celery tasks (app/worker), training pipeline (app/services/training.py), tests/
├─ frontend/         Next.js + Tailwind: user app (/dashboard, /history, /api-keys) and admin (/admin/*)
├─ docs/             ARCHITECTURE.md, schema.sql
├─ examples/         client.py, client.mjs, curl.sh: how an external app calls the API
├─ docker-compose.yml
└─ .env.example
```

## Run with Docker (recommended)

Requires Docker with Compose v2. The first build compiles Tesseract with its training tools (~5–10 min).

```bash
cd vegeocr-platform
cp .env.example .env          # then edit: POSTGRES_PASSWORD, JWT_SECRET, ADMIN_EMAIL, ADMIN_PASSWORD
docker compose up -d --build
docker compose logs -f api    # first start downloads ~80 MB of language models
```

| What | Where |
|---|---|
| Web app | http://localhost:3000 (sign up, then upload) |
| Admin | http://localhost:3000/admin/login (the `ADMIN_EMAIL` / `ADMIN_PASSWORD` from `.env`) |
| API docs (Swagger) | http://localhost:8000/docs  ·  OpenAPI JSON: http://localhost:8000/openapi.json |

`PUBLIC_API_URL` is baked into the frontend at build time; change it (and rebuild) when deploying to a real domain.

## Run without Docker (macOS/Linux)

Prerequisites: Python 3.9+, Node 18+, PostgreSQL, Redis, and Tesseract **built with training tools**
(`cmake -S <repo root> -B build -DBUILD_TRAINING_TOOLS=ON && cmake --build build`; needs leptonica, pango, cairo, icu).

```bash
# 1. database + models
createdb vegeocr
mkdir -p backend/tessdata && cd backend/tessdata
for l in jpn jpn_vert chi_sim chi_tra eng osd; do curl -LO https://github.com/tesseract-ocr/tessdata_best/raw/main/$l.traineddata; done
ln -s <repo root>/tessdata/configs configs && cd ..

# 2. backend
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
cat > .env <<EOF
DATABASE_URL=postgresql+psycopg://USER:PASS@localhost:5432/vegeocr
JWT_SECRET=$(openssl rand -hex 32)
ADMIN_EMAIL=admin@example.com
ADMIN_PASSWORD=ChangeMe123
TESSERACT_BIN=<build>/bin/tesseract
LSTMTRAINING_BIN=<build>/bin/lstmtraining
COMBINE_TESSDATA_BIN=<build>/bin/combine_tessdata
RETURN_RESET_TOKEN=true      # dev only: shows the password-reset link instead of emailing it
EOF
.venv/bin/uvicorn app.main:app --port 8000                                              # terminal 1
.venv/bin/celery -A app.worker.celery_app.celery worker -B -Q ocr,training,celery -l info  # terminal 2 (needs Redis)

# 3. frontend (terminal 3)
cd ../frontend && npm install && npm run dev        # http://localhost:3000
```

## API keys, scopes and the Learning module

**Keys** (dashboard: *API keys*; code: `backend/app/routers/keys.py`, `backend/app/services/keys.py`)
- Create with a name, an expiration (never / 30 / 90 / 365 days / custom date) and scopes. The full key is shown **once** (with a Copy button), then only `vocr_****abcd`.
- Table: name, masked key, scopes, created, last used, expires, status (`active`, `disabled`, `expired`, `revoked`). Actions: **Usage**, **Rename**, **Rotate**, **Disable/Enable**, **Delete**. Rotate, disable and delete ask for confirmation.
- Rotate keeps the key's id, name, scopes and limits but swaps the secret; the old one stops working immediately.
- Usage: request counts (total / 24 h / 7 d / 30 d), rate limit and current-minute usage, monthly quota, a 14-day chart and the last 20 requests.
- Only a SHA-256 of each key is stored. A lost key can't be recovered; rotate it.

**Scopes** (`backend/app/scopes.py`, enforced by the `require_scope(...)` dependency in `backend/app/deps.py`)

| Scope | Endpoints |
|---|---|
| `read` | `GET /api/v1/jobs/{id}`, `GET /api/v1/usage` |
| `write` | `POST /api/v1/ocr`, `POST /api/v1/training-data` |
| `learning:read` | `GET /api/v1/modules`, `GET /api/v1/modules/{id}`, `GET /api/v1/progress` |
| `learning:write` | `POST /api/v1/progress`, `POST /api/v1/quizzes/{id}/submit` |
| `admin` | implies every scope; only administrators can create such keys |

Every request is authenticated and then scope-checked: missing, invalid, disabled, expired or revoked key **401**; suspended owner **403**;
valid key without the scope **403 `insufficient_scope`** (header `X-Required-Scope` names it); rate limit or quota **429**. Keys that existed before
scopes were introduced were migrated to `read` + `write`, so existing integrations keep working.

**Learning module** (dashboard: *Learning*; code: `routers/learning_web.py`, `routers/learning_api.py`, `services/learning.py`)
- Modules list (title, topic, difficulty, lesson count, progress), lesson view with markdown content and an optional quiz, per-user progress
  (`not_started` / `in_progress` / `completed`, % done) and **Continue where you left off**. Passing a quiz (70% by default) completes its lesson.
- Quiz answers never leave the server before a submission, and results return only correct/incorrect plus an explanation.
- The same service layer backs the dashboard (JWT) and the API (key), so progress made through either shows up in both.
- Sample curriculum (3 modules, 9 lessons, 11 questions) is seeded once from `backend/app/learning_content.py`; edit that file (or the tables) to change it.

**API docs** (dashboard: *API docs*): every endpoint, its scope, a curl example and an example response. All data comes from
`frontend/lib/api-docs.json`, and `backend/tests/verify_api_docs.py` checks each example against the live API.

Schema changes for all of this are applied automatically at startup (`ALTER TABLE ... IF NOT EXISTS` for `api_keys`, `create_all` for the learning tables).

```bash
cd backend
.venv/bin/python tests/e2e_keys_learning.py     # ~100 checks: key lifecycle, expiry, scopes, learning API + dashboard
.venv/bin/python tests/verify_api_docs.py       # documented examples == live responses, documented scopes == enforced scopes
```

## Deploying to a server (AWS EC2 / any Linux host)

Files: `docker-compose.prod.yml`, `.env.prod.example`, `deploy/server-setup.sh`, `deploy/deploy.sh`.

| | |
|---|---|
| Images | Built **on your machine** for `linux/amd64` (compiling Tesseract on a small server would fill its disk) and streamed over SSH |
| Exposed ports | Only **80** (and **443** with a domain). Postgres, Redis and the API are reachable only inside Docker. Caddy serves the web app and proxies `/api` to FastAPI, so there is a single public address |
| Secrets | Live only in `~/vegeocr/.env` on the server (`chmod 600`); nothing secret is baked into an image |
| Restarts | `restart: unless-stopped` + Docker enabled at boot; data is in the `pgdata` / `appdata` volumes |

```bash
# 1. once per server (Amazon Linux 2023): Docker, Compose plugin, 2 GB swap
ssh -i KEY ec2-user@HOST 'sudo bash -s' < deploy/server-setup.sh
# 2. once: create ~/vegeocr/.env from .env.prod.example (strong secrets; PUBLIC_*_URL = the address users will type)
# 3. every deploy / update (about 35 s when the build is cached)
deploy/deploy.sh ec2-user@HOST /path/to/KEY.pem
```
**Open the firewall:** in the EC2 *Security group* allow inbound **TCP 80** (and **443** for HTTPS) from the internet, and restrict **22** to your own IP.

**HTTPS:** over plain HTTP, passwords and tokens cross the network unencrypted. Point a domain at the server, set `SITE_ADDRESS=your.domain` and `PUBLIC_WEB_URL=https://your.domain` in the server's `.env`
(leave `PUBLIC_API_URL` empty: the web app calls the API on its own address, so no rebuild is needed for a new address), open 443, and restart
(`sudo docker compose up -d`). Caddy then obtains and renews the certificate itself.

**Can't open the firewall yet? Use an SSH tunnel** (SSH is already open): `ssh -i KEY -N -L 8080:localhost:80 ec2-user@HOST`, then browse http://localhost:8080.

**Small-host notes (tested on 2 vCPU / 913 MB RAM / 8 GB disk):** idle memory is about 500 MB, peaking near 680 MB during OCR (the 2 GB swapfile absorbs spikes);
a dense 70-line page OCRs in ~10 s and a 3-page scanned PDF in ~40 s; one OCR worker runs at a time. For regular use move to at least 2 GB RAM and a 20 GB disk.
Container logs are rotated (3 x 10 MB) so they cannot fill the disk.

```bash
ssh -i KEY ec2-user@HOST
cd ~/vegeocr
sudo docker compose ps                     # status
sudo docker compose logs -f api worker     # logs
sudo docker compose exec -T db pg_dump -U vegeocr vegeocr | gzip > ~/backup-$(date +%F).sql.gz   # database backup (nothing is scheduled automatically)
```
Rollback: re-run `deploy/deploy.sh` from the previous git revision. Database changes are applied automatically at start-up and are additive.

## Account and password change

Dashboard: **Account** (user app, and admin app under `/admin/account`), reachable from the nav and by clicking your name. It shows your profile
(email, username, role, last sign-in, when the password last changed) and a **Change password** form with live rule feedback.

- API: `POST /api/web/auth/change-password` with `{current_password, new_password}` (needs a login). Rules: at least 8 characters with a letter and a number,
  different from the current password. Wrong current password: `400 incorrect_password`; after 5 wrong guesses in 15 minutes: `429` (typos in the *new* password don't count).
- **Other sessions are signed out.** Each user has a `token_version` that is stored in every login token; changing the password bumps it, so tokens from other
  devices (or a stolen one) stop working with `401 session_expired`, while the response returns a fresh token so you stay signed in. A password **reset**
  and an **admin setting someone's password** do the same. API keys are not tied to the password and keep working.
- Test: `cd backend && .venv/bin/python tests/e2e_password.py`.

## OCR lab: handwriting, blurry images, your own training data

**Learning → Extraction lab** and **Learning → My training data** (details, architecture, measurements and limits: [docs/HANDWRITING.md](docs/HANDWRITING.md)).

- **Handwriting** (print and cursive, English) via TrOCR, with a per-line confidence; other languages fall back to Tesseract with a warning. TrOCR is optional (it needs ~2 GB of RAM):
  `pip install -r backend/requirements-handwriting.txt && python backend/scripts/download_trocr.py`, or build the Docker image with `INSTALL_HANDWRITING=1` and run with `ENABLE_HANDWRITING=1`. The production compose file does not install it.
- **Blurry / low-quality images** are detected and enhanced with OpenCV (denoise, shadows, deskew, upscale, sharpen), shown **side by side** with the original; lines under the confidence threshold are outlined for review.
- **Your own training data**: datasets of (line image, exact text) samples, grouped by category, with add / edit / delete, zip import and export, and one-click saving of corrections from an extraction. **Train** on demand and see the error rate **before vs after** on held-out samples.
- Tests: `backend/tests/test_lab_unit.py` (59), `backend/tests/e2e_lab.py` (~90 checks against a running stack), `backend/tests/eval_enhancement.py` (the enhancement measurements).

## Response compression

| What | Where | How |
|---|---|---|
| API JSON/text (FastAPI) | `backend/app/middleware/compression.py`, registered in `backend/app/main.py` | On the fly, off the event loop. zstd 3 / brotli 5 / gzip 6 |
| Next.js static assets (`/_next/static/*`) | `frontend/scripts/precompress.mjs` (run in `frontend/Dockerfile`), served by `frontend/Caddyfile` | Pre-compressed at image build: zstd 19 / brotli 11 / gzip 9; served from disk, `immutable` |
| Next.js HTML | `frontend/Caddyfile` (`encode`) | On the fly, zstd or gzip (stock Caddy has no dynamic brotli encoder) |

Negotiation: best of **zstd > br > gzip > identity** that the client accepts. q-values are honoured (`q=0` = refused); equal
q-values (every browser sends them) are broken by *our* order. Only text-like types are compressed (JSON, `text/*`, XML, SVG,
JavaScript), so images, video, audio, fonts, ZIP and PDF pass through untouched. Bodies under 1 KB, HEAD, 204/206/304,
Range requests, `no-transform` and already-encoded responses are skipped. `Vary: Accept-Encoding` is merged into any existing
`Vary`, and a changed representation gets a per-encoding ETag suffix. Without `zstandard`/`brotli` installed it falls back to
brotli/gzip/identity. A compression error sends the original bytes. Settings (env): `COMPRESS_MIN_BYTES` (1024),
`COMPRESS_ZSTD_LEVEL` (3), `COMPRESS_BROTLI_QUALITY` (5), `COMPRESS_GZIP_LEVEL` (6), `COMPRESS_MAX_BUFFER_MB` (16),
`COMPRESS_DISABLE` (e.g. `zstd`, to test fallbacks).

**Verify which encoding you get**

```bash
# API: -s -o /dev/null -D - prints only the headers; the raw size is what crossed the wire
curl -s -o /dev/null -D - -H 'Accept-Encoding: gzip, deflate, br, zstd' http://localhost:8000/openapi.json | grep -iE 'content-encoding|content-length|vary'
curl -s -o /dev/null -w '%{size_download} bytes\n' -H 'Accept-Encoding: br'   http://localhost:8000/openapi.json
curl -s -o /dev/null -w '%{size_download} bytes\n' -H 'Accept-Encoding: identity' http://localhost:8000/openapi.json
curl -s -o /dev/null -D - -H 'Accept-Encoding: br;q=0, gzip' http://localhost:8000/openapi.json | grep -i content-encoding   # gzip (q=0 refuses br)

# Static asset through the edge (pre-compressed on disk) and HTML (compressed on the fly)
CHUNK=$(curl -s localhost:3000/login | grep -oE '/_next/static/chunks/[^"]+\.js' | head -1)
curl -s -o /dev/null -D - -H 'Accept-Encoding: zstd, br, gzip' "localhost:3000$CHUNK" | grep -iE 'content-encoding|cache-control'
curl -s -o /dev/null -D - -H 'Accept-Encoding: zstd, br, gzip' localhost:3000/login | grep -iE 'content-encoding|vary'

# Round-trip check: decoded bytes must equal the uncompressed ones
curl -s -H 'Accept-Encoding: identity' http://localhost:8000/openapi.json | shasum; curl -s --compressed -H 'Accept-Encoding: gzip' http://localhost:8000/openapi.json | shasum
```
(`curl --compressed` only decodes gzip/deflate/br/zstd that your curl build supports. The `-w '%{size_download}'` form always shows the wire size.)

Tests: `cd backend && .venv/bin/python -m pytest tests/test_compression.py -q` (72 cases).

## Tests

With the API and worker running:

```bash
cd backend
.venv/bin/python tests/e2e.py                                  # 56 checks: auth, OCR, corrections, keys, limits, admin, isolation
swift tests/make_lines.swift /tmp/lines                        # (macOS) synthetic line images
.venv/bin/python tests/train_smoke.py /tmp/lines               # submit → approve → retrain → evaluate → deploy/reject
```

## Public API

Authenticate with `X-API-Key: vocr_…` (create keys in the web app). Errors always look like
`{"error": {"code": "...", "message": "..."}}`.

| Endpoint | Purpose |
|---|---|
| `POST /api/v1/ocr` | multipart `file`, optional `lang` (default `jpn+eng`), optional `async=true`. `200` with text + per-line confidence, or `202` + job id |
| `GET /api/v1/jobs/{id}` | status / result of a job |
| `POST /api/v1/training-data` | multipart `image` (ONE text line), `text`, `lang`: queued for admin review |
| `GET /api/v1/usage` | this key's quota, limits, last-7-day requests and errors |

Every response carries `X-RateLimit-Limit/Remaining/Reset`; `429` adds `Retry-After`. Codes: `missing_api_key`,
`invalid_api_key`, `revoked_api_key` (401), `rate_limited`, `quota_exceeded` (429), `unsupported_type` (415),
`file_too_large` (413), `invalid_lang` / `invalid_text` (422). See [`examples/`](examples) for Python, Node and curl.

## Configuration (backend env vars)

`DATABASE_URL`, `REDIS_URL`, `JWT_SECRET`, `ADMIN_EMAIL`, `ADMIN_PASSWORD`, `CORS_ORIGINS`, `MAX_UPLOAD_MB` (20),
`MAX_PDF_PAGES` (50), `SYNC_MAX_PAGES` (2), `DEFAULT_RATE_LIMIT_PER_MIN` (60), `DEFAULT_MONTHLY_QUOTA` (1000),
`RETRAIN_THRESHOLD` (50), `RETRAIN_MIN_TEST_SAMPLES` (5), `RETRAIN_MAX_ITERATIONS` (400), `RETRAIN_SCHEDULE_HOURS` (24).
Training settings can also be changed live in **Admin → Models**.

## Known limitations / before going to production

* **Password-reset email is not sent.** The link is printed in the API log (`[password-reset] …`). Wire your SMTP
  provider into `routers/auth.py::forgot`.
* **Serve over HTTPS** (put Caddy/nginx/a load balancer in front) and set `CORS_ORIGINS`, `PUBLIC_API_URL` to the real domains.
* The schema is created with `create_all` at startup. Add Alembic migrations before the schema changes in production.
  (An existing DB created before the `uq_one_active_run_per_lang` index was added needs that index created by hand.)
* The JWT lives in `localStorage` (simple, XSS-sensitive). For stricter setups move it to an httpOnly cookie.
* Run exactly one `worker` container (it embeds the scheduler); scale OCR throughput with extra workers started
  *without* `-B`.
* Fine-tuning needs a few hundred reviewed lines per language to move accuracy meaningfully; with few samples the
  evaluation gate will (correctly) reject candidates. Fine-tuning requires the float `tessdata_best` models.
* Only images and PDFs are accepted by the platform (the earlier single-file `webapp/` also reads DOCX/XLSX).
