import { removeFile } from "../lib/files.js";

/**
 * The work behind one queued job, shared by both queue implementations:
 * run OCR in the pool, store the result in the cache, always remove the temp upload when we are done with it.
 */
export function createProcessor({ runOcr, cache, metrics, logger }) {
  return async function process(data, { onProgress, signal, finalAttempt = true } = {}) {
    const { path, key, mime, lang } = data;
    const started = performance.now();
    try {
      const result = await runOcr({ filePath: path, mime, lang, onProgress, signal });
      await cache.set(key, Buffer.from(JSON.stringify(result)));
      metrics.ocrDuration.observe({ mode: "queue", status: "ok" }, (performance.now() - started) / 1000);
      logger.info({ key, ocrMs: Math.round(performance.now() - started), pages: result.pageCount }, "ocr job done");
      await removeFile(path);
      return { key };
    } catch (err) {
      metrics.ocrDuration.observe({ mode: "queue", status: "error" }, (performance.now() - started) / 1000);
      logger.warn({ key, err: err.message, code: err.code }, "ocr job failed");
      if (finalAttempt || err.input) await removeFile(path); // keep the file only if another attempt will need it
      throw err;
    }
  };
}

/** Maps any thrown error to the {code, message} shape clients see (no stack traces, no internal paths). */
export function publicError(err) {
  if (err?.input) return { code: err.code, message: err.message };
  if (err?.name === "AbortError" || /abort|timed? ?out/i.test(err?.message ?? "")) return { code: "ocr_timeout", message: "OCR took too long and was cancelled" };
  return { code: "ocr_failed", message: "OCR failed" };
}
