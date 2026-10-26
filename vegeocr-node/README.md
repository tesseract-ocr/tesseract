# vegeocr-node

A high-throughput OCR API in Node.js (ESM, Express 5): dynamic zstd/brotli/gzip compression, content-addressed result
caching with ETags, OCR in a worker-thread pool, a BullMQ job queue with Server-Sent-Events progress, and the usual
production hardening (rate limits, timeouts, structured logs, Prometheus metrics, graceful shutdown).

OCR engine: the native **Tesseract** binary, fed through stdin from worker threads. PDFs are rasterised with `mupdf`
(WASM, no system dependency). This is a standalone service; it does not touch the Python platform in `../vegeocr-platform`.

```
vegeocr-node/
├─ src/
│  ├─ server.js              wiring, keep-alive tuning, graceful shutdown
│  ├─ app.js                 buildApp(deps): every dependency injectable (that is what makes it testable)
│  ├─ config.js              all settings from env (see .env.example)
│  ├─ logger.js  metrics.js  pino · prom-client
│  ├─ lib/                   codecs.js negotiate.js cache.js etag.js respond.js files.js
│  ├─ middleware/            compression · upload (+ request decompression) · rateLimit · security · timeout · staticPrecompressed · errors
│  ├─ routes/ocr.js          POST /api/ocr · GET /api/ocr/:id · GET /api/ocr/:id/events
│  ├─ queue/                 bullQueue.js (Redis) · memoryQueue.js (dev/tests) · processor.js
│  └─ workers/               pool.js (Piscina) · ocrWorker.js · preprocess.js (sharp) · tesseract.js
├─ scripts/                  build-public.js (hash filenames) · precompress.js (zstd 19 / brotli 11 / gzip 9)
├─ public-src/               tiny demo front-end (upload page with SSE progress)
├─ bench/                    benchmark.js · make-sample.js
├─ deploy/                   nginx.conf · Caddyfile (untested starting points)
├─ examples/curl.sh          runnable walkthrough of every feature
└─ test/                     122 tests (node:test)
```

## Setup

Requirements: **Node ≥ 20** (≥ 22.15 gives built-in zstd; otherwise the optional `@skhaz/zstd` package is used),
a **Tesseract 5** binary plus `.traineddata` files, and **Redis** (optional in dev, required in production).

```bash
npm install
cp .env.example .env     # set TESSERACT_BIN and TESSDATA_PREFIX at minimum; set REDIS_URL for the real queue/cache
npm run build:public     # hashes + pre-compresses the demo front-end into public/
npm start                # http://localhost:4000   (npm run dev for a pretty-printed, auto-reloading server)
```

Without `REDIS_URL` the service falls back to an in-memory cache, in-memory queue and in-memory rate-limit store. That is
fine for development and single-instance trials, but jobs are lost on restart and limits are per-process.

### Environment variables

See [`.env.example`](.env.example) for the full annotated list. The important ones:

| Variable | Default | Meaning |
|---|---|---|
| `REDIS_URL` | *(empty)* | Redis for cache, BullMQ and rate limits. Empty = in-memory fallbacks |
| `TESSERACT_BIN`, `TESSDATA_PREFIX` | `tesseract`, – | engine binary and model directory |
| `OCR_POOL_SIZE` | cores − 1 | worker threads (each runs one single-threaded tesseract at a time) |
| `OCR_SYNC_MAX_KB` | 2048 | in `mode=auto`, images larger than this are queued |
| `MAX_UPLOAD_MB` / `MAX_PDF_PAGES` | 20 / 50 | hard upload limits |
| `MAX_DECOMPRESSED_MB` | 64 | cap for `Content-Encoding`'d request bodies (decompression-bomb guard) |
| `ZSTD_LEVEL` / `BROTLI_LEVEL` / `GZIP_LEVEL` | 3 / 5 / 6 | request-time compression levels |
| `COMPRESS_THRESHOLD` | 1024 | don't compress bodies smaller than this (bytes) |
| `CACHE_TTL_SECONDS` / `CACHE_LRU_MAX_MB` | 86400 / 256 | result cache lifetime / in-memory fallback size |
| `RATE_LIMIT_MAX` / `RATE_LIMIT_OCR_MAX` | 300 / 30 | requests per window per IP: global / `POST /api/ocr` |
| `CORS_ORIGINS` | `http://localhost:3000` | comma-separated allowlist (`*` is refused in production) |
| `TRUST_PROXY` | – | set to `1` behind Nginx/Caddy so rate limits see real client IPs |
| `REQUEST_TIMEOUT_MS`, `OCR_TIMEOUT_MS` | 130000, 120000 | request ceiling / OCR ceiling |
| `KEEP_ALIVE_TIMEOUT_MS`, `HEADERS_TIMEOUT_MS` | 65000, 66000 | must exceed the proxy's idle timeout |
| `METRICS_TOKEN` | – | if set, `/metrics` requires `Authorization: Bearer <token>` |

## API

| Endpoint | |
|---|---|
| `POST /api/ocr` | multipart `file` (+ optional `lang`, `mode=auto\|sync\|async`). **200** with the result, or **202** `{id, links}` for PDFs, large files or `mode=async` |
| `GET /api/ocr/:id` | job status JSON while running; the full result (ETag, 304, compressed) once done; `410` if the cached result expired |
| `GET /api/ocr/:id/events` | Server-Sent Events: `status`, `progress {page,total}`, then `done` or `failed` |
| `GET /health` | status, queue counts, Redis state (503 while draining) |
| `GET /metrics` | Prometheus |

Result shape: `{status:"done", lang, pageCount, text, confidence, pages:[{page, width, height, text, lines:[{text, confidence, bbox:[l,t,r,b], words:[{text, confidence, bbox}]}]}], meta}`.
Bounding boxes are in pixels of the *preprocessed* image (`pages[].width/height`), so scale them if you draw on the original.
Errors are always `{"error":{"code","message"}}`.

`mode=auto` decides for you: a small image runs inline in the worker pool (lowest latency); anything slow or large goes
through the queue so no HTTP connection is held open for tens of seconds.

## How each requirement is met, with curl

`sh examples/curl.sh [image.png]` runs all of these against a live server. Individually:

**1. Dynamic compression.** `Accept-Encoding` is negotiated in `lib/negotiate.js`; order zstd → br → gzip → identity; only
text-like types (json, `text/*`, xml, svg), never images/PDFs; skipped under 1 KB, on HEAD, for 204/304 and
`no-transform`; `Vary: Accept-Encoding` always; any compression error falls back to the uncompressed body.
```bash
curl -s -o /dev/null -D - -H 'Accept-Encoding: zstd, br, gzip' -F file=@page.png localhost:4000/api/ocr | grep -i content-encoding   # zstd
curl -s -o /dev/null -D - -H 'Accept-Encoding: br;q=0.4, gzip;q=0.9' -F file=@page.png localhost:4000/api/ocr | grep -i content-encoding   # gzip (q wins)
```
> **A pitfall this project hit and tests now guard:** the `accepts` package breaks ties between equally-rated encodings
> by the *client's* order, so a browser header `gzip, deflate, br, zstd` would always get gzip. We use `accepts` to decide
> what is acceptable, rank by q-value ourselves, and break ties by *server* preference.

zstd uses `node:zlib` when available (Node ≥ 22.15), else `@skhaz/zstd` (`ZSTD_IMPL=auto|native|skhaz`).

**2. Pre-compressed static assets.** `npm run build:public` copies `public-src/` → `public/`, content-hashes JS/CSS names
(`app.3f2a9c1b.js`), then writes `.zst/.br/.gz` at zstd 19 / brotli 11 / gzip 9. The handler serves the best file the
client accepts, `immutable` + one year for hashed names, `no-cache` for `index.html`, per-encoding ETags and 304s.
```bash
curl -s -D - -o /dev/null -H 'Accept-Encoding: br' localhost:4000/app.<hash>.js | grep -iE 'content-encoding|cache-control'
```

**3. Result caching.** Key = SHA-256 of the file bytes + language + preprocessing version, so filename and Content-Type
are irrelevant. The JSON body *and each compressed variant* are cached (Redis, or a byte-bounded LRU), so repeats skip OCR
and compression. ETag derives from the key + encoding; `If-None-Match` → `304`.
```bash
curl -s -D - -o /dev/null -H 'Accept-Encoding: br' -F file=@page.png localhost:4000/api/ocr | grep -iE 'x-cache|etag'
curl -s -o /dev/null -w '%{http_code}\n' -H 'Accept-Encoding: br' -H 'If-None-Match: "<etag>"' -F file=@page.png localhost:4000/api/ocr   # 304
```
Response headers: `X-Cache: HIT|MISS`, `X-Cache-Variant: HIT|MISS` (was the compressed variant already cached?).

**4. OCR off the event loop + queue + SSE.** All CPU work (PDF rasterising, sharp, tesseract spawning) runs in a Piscina pool;
the main thread only parses requests and moves bytes. Identical uploads that are *in flight* share one job.
```bash
curl -s -F file=@scan.pdf localhost:4000/api/ocr          # 202 {"id":"…","links":{…}}
curl -s localhost:4000/api/ocr/<id>                        # {"status":"processing","progress":{"page":2,"total":3}}
curl -sN localhost:4000/api/ocr/<id>/events                # event: progress … event: done
```

**5. Image preprocessing** (`workers/preprocess.js`): EXIF auto-rotate, single-channel grayscale, scale toward ~300 DPI,
cap the long side (`OCR_MAX_DIMENSION`), strip metadata; PDF pages are rasterised at 300 DPI. Scaling rule: a declared DPI
below 300 is scaled up (max 4×); with no DPI metadata only small images (< 1200 px) are doubled. Upload type is checked by
magic bytes (`file-type`), never by `Content-Type`.

**6. Upload handling.** `multer` streams to disk under limits (size, one file, field counts); the temp file is always
removed. Request bodies with `Content-Encoding: gzip|deflate|br|zstd` are accepted on the upload route, capped *after*
decompression:
```bash
{ printf -- "--B\r\nContent-Disposition: form-data; name=\"file\"; filename=\"p.png\"\r\nContent-Type: image/png\r\n\r\n"; cat page.png; printf -- "\r\n--B--\r\n"; } | gzip -c > body.gz
curl -s -H 'Content-Type: multipart/form-data; boundary=B' -H 'Content-Encoding: gzip' --data-binary @body.gz localhost:4000/api/ocr   # 200
head -c 200000000 /dev/zero | gzip -9 > bomb.gz   # ~194 KB
curl -s -H 'Content-Type: multipart/form-data; boundary=B' -H 'Content-Encoding: gzip' --data-binary @bomb.gz localhost:4000/api/ocr    # 413 payload_too_large
```

**7. Abuse protection.** `helmet`; `express-rate-limit` (Redis store when `REDIS_URL` is set) with a stricter limit on
`POST /api/ocr`; CORS allowlist (other origins get no CORS headers); a request timeout (503) and an OCR timeout.
```bash
RATE_LIMIT_OCR_MAX=3 npm start      # then 4 quick uploads: the 4th is 429 {"error":{"code":"rate_limited"}} with Retry-After
curl -s -o /dev/null -D - -H 'Origin: https://evil.example' localhost:4000/health | grep -i access-control   # (nothing)
```

**8. Transport.** See [Reverse proxy](#reverse-proxy-and-where-compression-should-happen). Node's `keepAliveTimeout` (65 s) and
`headersTimeout` (66 s) are set above the proxy's 60 s upstream idle timeout.

**9. Observability.** `pino` JSON logs with request id (`X-Request-Id` is accepted if well-formed, echoed, and logged),
chosen encoding, original/compressed bytes, OCR ms, cache status. Prometheus metrics: `http_request_duration_seconds`,
`ocr_job_duration_seconds`, `ocr_queue_depth{state}`, `ocr_cache_requests_total{result}`, `http_compression_ratio{encoding}`,
`http_compression_seconds{encoding}`, `sse_connections`, plus Node defaults. Graceful shutdown on SIGTERM/SIGINT: `/health`
turns 503, new connections stop, SSE clients get a `shutdown` event, in-flight requests and *running* OCR jobs finish (30 s
cap), then Redis and the queue close. Jobs that were still *waiting* stay in Redis and are processed after the next start
(checked: SIGTERM mid-job, restart, the waiting jobs completed); the in-memory queue has no such durability and drops them.
```bash
{"level":30,"req":{"id":"fc62aca6-…","method":"POST","url":"/api/ocr?mode=sync"},"res":{"statusCode":200},"responseTime":5985,
 "encoding":"br","originalBytes":64942,"compressedBytes":11863,"ocrMs":5981,"cache":"MISS","file":"3be738228782","msg":"request completed"}
curl -s localhost:4000/health ; curl -s localhost:4000/metrics | grep ocr_
```

## Reverse proxy, and where compression should happen

Run Node behind **Nginx or Caddy** ([`deploy/nginx.conf`](deploy/nginx.conf), [`deploy/Caddyfile`](deploy/Caddyfile); both are
starting points I have not run): TLS, HTTP/2 and HTTP/3, request buffering for slow uploaders, and static files belong there.

| What | Compress where | Why |
|---|---|---|
| Front-end JS/CSS/HTML | **At build time**, served by the proxy from disk (`gzip_static`/`brotli_static`/`zstd_static`, Caddy `precompressed`) | Max levels cost nothing at request time and Node isn't involved at all |
| OCR JSON | **In Node**, proxy compression **off** for `/api/` | Results are content-addressed and cached: Node compresses *once* and replays the bytes. A proxy would re-compress on every request and cannot see the cache. Nginx and Caddy skip responses that already have `Content-Encoding`, so nothing is compressed twice |
| SSE | Nowhere | Streams must not be buffered or compressed (`proxy_buffering off`; Node also sends `X-Accel-Buffering: no`) |
| TLS, HTTP/2/3, keep-alive | Proxy | Node serves plain HTTP/1.1 to the proxy |

Keep-alive rule: the **upstream** side must close idle sockets *first*. Node's `keepAliveTimeout` (65 s) > proxy idle (60 s),
and `headersTimeout` (66 s) > `keepAliveTimeout`. Reverse it and you get sporadic 502s under load. Set `TRUST_PROXY=1`.

## Tests

```bash
npm test      # 122 tests, ~13 s
```
Uses `node:test`; no mocks of the system under test: compression negotiation (32 cases), caching / ETag / 304 / variant
caching, rate limiting, upload validation (magic bytes, limits, gzip/br/zstd bodies, decompression bomb, PDF page limit
through the real worker pool), the queue flow (in-memory **and** real BullMQ/Redis, SSE, in-flight de-duplication,
failures), static serving, CORS/helmet/metrics, and, when `TESSERACT_BIN` is available, real OCR through the pool plus a
real `SIGTERM` graceful-shutdown test. Redis tests use `redis://localhost:6379/15` (`TEST_REDIS_URL` to change) and skip
themselves if Redis is down.

## Benchmark

```bash
npm run bench:sample                     # generate a dense text page
node bench/benchmark.js --runs 30        # against a running server
```
Measured on an Apple-silicon laptop, 70-line / ~700-word page, 63 KB JSON result, Redis cache, loopback. Your numbers will differ; the *shape* will not.

**Size on the wire** (cached result, variant already cached)

| encoding | bytes | of original | median latency | client decode |
|:--|--:|--:|--:|--:|
| identity | 63.4 KB | 100% | 2.1 ms | – |
| gzip | 11.5 KB | 18.1% | 2.1 ms | 0.1 ms |
| br | 11.6 KB | 18.3% | 2.0 ms | 0.1 ms |
| zstd | 12.0 KB | 18.9% | 2.0 ms | 0.1 ms |

**Compression cost per level** (same JSON, in-process)

| level | size | compress | throughput |
|:--|--:|--:|--:|
| gzip 1 / 6 / 9 | 14.7 / 11.5 / 11.3 KB | 0.2 / 0.6 / 1.1 ms | 261 / 96 / 56 MB/s |
| br 4 / 5 / 9 / **11** | 12.0 / 11.6 / 10.4 / **8.6** KB | 0.4 / 0.6 / 0.9 / **43.4 ms** | 160 / 101 / 67 / **1** MB/s |
| zstd 1 / 3 / 9 / **19** | 12.3 / 12.0 / 10.7 / **9.4** KB | 0.1 / 0.1 / 0.5 / **14.2 ms** | 515 / 488 / 125 / **4** MB/s |

**Cold vs cached**

| request | median | vs cold |
|:--|--:|--:|
| cold: OCR + compress + store | 5925 ms | 1× |
| cached body, new encoding (compress once) | 4.1 ms | ~1400× faster |
| cached body + cached variant | 1.8 ms | ~3200× faster |

**Event loop while OCR runs:** `/health` median 0.4 ms idle vs 0.6 ms during a 6-second OCR (p99 1.1 → 1.4 ms).

## Trade-offs

* **Compression level = CPU vs bytes.** From the table: brotli 11 saves 26% over brotli 5 but costs **~70× the CPU**
  (43 ms vs 0.6 ms); zstd 19 is ~140× slower than zstd 3 for 22% fewer bytes. That is why the maximum levels are used *only*
  at build time for static files and the request-time defaults are zstd 3 / br 5 / gzip 6. Notice zstd 3 is the fastest
  by a wide margin but, on this payload, **~4% bigger than gzip 6 / br 5**: its win is CPU, not size. If your payloads are
  mostly cache hits (variants are stored), you pay compression once, so raising `BROTLI_LEVEL` to 9 or `ZSTD_LEVEL` to 9
  is cheap and worth it. If your hit rate is low, keep the defaults.
* **Cache memory.** Each distinct document stores its JSON plus up to three compressed variants, here ~63 + 3×12 ≈ 100 KB.
  10 000 documents ≈ 1 GB of Redis. Set a TTL (default 24 h) and `maxmemory-policy allkeys-lru` in Redis. The in-memory
  fallback is hard-capped by bytes (`CACHE_LRU_MAX_MB`), not by entry count, so one huge result can't blow the heap.
* **Queue vs synchronous.** Synchronous is simplest and fastest for one small image, but it ties up a connection (and a
  worker thread) for the whole OCR and hits proxy timeouts on big documents. The queue adds a round trip (poll or SSE) and
  Redis, in exchange for backpressure, retries, durability across restarts, and no held connections. `mode=auto` uses sync
  for small single images and the queue for PDFs and large files; use `mode=async` to force the queue for everything.
* **Pool size.** `cores − 1` threads, each running tesseract with `OMP_THREAD_LIMIT=1`: parallelism across requests, not
  inside one. For a single huge page you would get lower latency from tesseract's own multithreading, but worse throughput.
* **ETag per encoding.** The same JSON in br and identity are different representations, so they get different ETags
  (`…-br`, `…-identity`). A client that switches `Accept-Encoding` re-downloads once; correctness over hit rate.
* **Where limits live.** The rate limiter keys on IP. Behind a proxy that means `TRUST_PROXY` must be right, or everyone
  shares one bucket. Add API-key limits if you authenticate clients.

## Known limitations

* **No authentication.** The service is open apart from rate limits. Put it behind your gateway, or add API-key middleware.
* Digital PDFs are rasterised and OCR'd like scans; using their embedded text layer would be faster and exact (not done here).
* Request-body decompression is wired into the upload route only (it works by replacing `req.pipe`, which multer/busboy use).
* Aborting a running Piscina task terminates its thread; a tesseract child it spawned finishes on its own (it also has its own
  per-page timeout, `OCR_TIMEOUT_MS × 0.8`).
* The in-memory queue loses jobs on restart; use Redis for anything real.
* `deploy/nginx.conf` and `deploy/Caddyfile` are unverified templates. Zstd in Nginx needs a third-party module.
