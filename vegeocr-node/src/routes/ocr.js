import { Router } from "express";
import { HttpError } from "../errors.js";
import { removeFile, sha256File, sniffUpload } from "../lib/files.js";
import { sendCachedResult } from "../lib/respond.js";
import { publicError } from "../queue/processor.js";
import { createUpload, decompressRequestBody } from "../middleware/upload.js";

export const PREPROC_VERSION = 1; // bump when preprocessing/parsing changes so old cache entries are not served
const UUID = /^[0-9a-f-]{36}$/i;

export function ocrRouter({ cfg, cache, codecs, queue, runOcr, metrics, logger, limiter }) {
  const router = Router();
  const upload = createUpload(cfg.upload);
  const ctx = { cache, codecs, metrics, cfg, logger };

  const parseLang = (v) => {
    const lang = String(v || cfg.ocr.defaultLang).trim();
    if (!lang.split("+").every((l) => cfg.ocr.allowedLangs.includes(l))) {
      throw new HttpError(422, "invalid_lang", `lang must combine these codes with '+': ${cfg.ocr.allowedLangs.join(", ")}`);
    }
    return lang;
  };
  const parseMode = (v) => {
    const mode = String(v || "auto");
    if (!["auto", "sync", "async"].includes(mode)) throw new HttpError(422, "invalid_mode", "mode must be auto, sync or async");
    return mode;
  };

  /**
   * POST /api/ocr  (multipart: file, lang?, mode?)
   *   200 + result       small image, or any cache hit
   *   202 + job id       PDFs, big files, or mode=async  -> poll GET /api/ocr/:id or stream /api/ocr/:id/events
   */
  router.post("/", limiter, decompressRequestBody(cfg.upload.maxDecompressedBytes), upload, async (req, res) => {
    const file = req.file;
    if (!file) throw new HttpError(400, "missing_file", 'Send the file in a multipart field named "file"');
    let keepFile = false;
    try {
      const lang = parseLang(req.body?.lang);
      const mode = parseMode(req.query.mode ?? req.body?.mode);
      const mime = await sniffUpload(file.path); // magic bytes, not the Content-Type header
      const hash = await sha256File(file.path);
      const key = `${hash}:${lang}:p${PREPROC_VERSION}`;
      res.locals.fileHash = hash.slice(0, 12);

      if (await sendCachedResult(req, res, { ...ctx, key, cacheStatus: "HIT" })) {
        metrics.cacheRequests.inc({ result: "hit" });
        return;
      }
      metrics.cacheRequests.inc({ result: "miss" });

      const useQueue = mode === "async" || (mode === "auto" && (mime === "application/pdf" || file.size > cfg.ocr.syncMaxBytes));
      if (useQueue) {
        const { id, deduped } = await queue.enqueue({ path: file.path, key, mime, lang });
        keepFile = !deduped; // the processor deletes the file when the job ends; a deduped upload is just dropped
        return void res.status(202).set("Location", `/api/ocr/${id}`).json({
          id, status: "queued", deduplicated: deduped,
          links: { self: `/api/ocr/${id}`, events: `/api/ocr/${id}/events` },
        });
      }

      const started = performance.now();
      let result;
      try {
        result = await runOcr({ filePath: file.path, mime, lang });
      } catch (err) {
        metrics.ocrDuration.observe({ mode: "sync", status: "error" }, (performance.now() - started) / 1000);
        if (err.input) throw new HttpError(422, err.code, err.message);
        throw err;
      }
      res.locals.ocrMs = Math.round(performance.now() - started);
      metrics.ocrDuration.observe({ mode: "sync", status: "ok" }, res.locals.ocrMs / 1000);
      const body = Buffer.from(JSON.stringify(result));
      await cache.set(key, body);
      await sendCachedResult(req, res, { ...ctx, key, body, cacheStatus: "MISS" });
    } finally {
      if (!keepFile) removeFile(file.path);
    }
  });

  /** GET /api/ocr/:id -> status JSON while running; the full result (ETag / 304 / compressed variants) once done. */
  router.get("/:id", async (req, res) => {
    const info = UUID.test(req.params.id) ? await queue.get(req.params.id) : null;
    if (!info) throw new HttpError(404, "job_not_found", "No such job");
    if (info.state === "completed") {
      if (await sendCachedResult(req, res, { ...ctx, key: info.key, cacheStatus: "HIT" })) return;
      throw new HttpError(410, "result_expired", "The result has expired; upload the file again");
    }
    res.set("Cache-Control", "no-store");
    if (info.state === "failed") return void res.json({ id: info.id, status: "failed", error: info.error });
    if (info.state === "active") return void res.json({ id: info.id, status: "processing", progress: info.progress });
    res.set("Retry-After", "2").json({ id: info.id, status: "queued" });
  });

  /** GET /api/ocr/:id/events -> Server-Sent Events: status, progress (per page), then done | failed. */
  router.get("/:id/events", async (req, res) => {
    if (!UUID.test(req.params.id)) throw new HttpError(404, "job_not_found", "No such job");
    const id = req.params.id;
    const events = [];
    let live = false, finished = false;
    const send = (event, data) => !res.writableEnded && res.write(`event: ${event}\ndata: ${JSON.stringify(data)}\n\n`);
    const finish = () => { if (finished) return; finished = true; unsubscribe(); clearInterval(beat); metrics.activeSse.dec(); res.end(); };
    const handle = (e) => {
      if (e.type === "progress") send("progress", { page: e.page, total: e.total });
      else if (e.type === "completed") { send("done", { id, result: `/api/ocr/${id}` }); finish(); }
      else if (e.type === "failed") { send("failed", { id, error: e.error }); finish(); }
    };
    // Subscribe BEFORE reading state so an event that fires in between is buffered, not lost.
    const unsubscribe = queue.subscribe(id, (e) => (live ? handle(e) : events.push(e)));
    const info = await queue.get(id);
    if (!info) { unsubscribe(); throw new HttpError(404, "job_not_found", "No such job"); }

    res.writeHead(200, { "Content-Type": "text/event-stream", "Cache-Control": "no-cache, no-transform", Connection: "keep-alive", "X-Accel-Buffering": "no" });
    metrics.activeSse.inc();
    res.write("retry: 3000\n\n");
    const beat = setInterval(() => res.write(": keep-alive\n\n"), 15_000);
    req.on("close", finish);
    res.on("close", finish);
    res.locals.skipCompression = true;
    send("status", { id, status: info.state === "completed" ? "done" : info.state === "active" ? "processing" : info.state === "waiting" ? "queued" : "failed", progress: info.progress });
    if (info.state === "completed") { send("done", { id, result: `/api/ocr/${id}` }); return finish(); }
    if (info.state === "failed") { send("failed", { id, error: info.error }); return finish(); }
    live = true;
    for (const e of events) handle(e);
    res.locals.sse = { close: () => { send("shutdown", { reason: "server restarting" }); finish(); } };
    router.sseClosers?.add(res.locals.sse);
    res.on("close", () => router.sseClosers?.delete(res.locals.sse));
  });
  router.sseClosers = new Set();
  return router;
}
