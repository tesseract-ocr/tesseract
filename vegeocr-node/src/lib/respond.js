import { etagMatches, makeEtag } from "./etag.js";
import { negotiateEncoding } from "./negotiate.js";

/**
 * Sends a cached OCR result: negotiates the encoding, answers If-None-Match with 304, and serves the compressed
 * variant straight from the cache (compressing and storing it on first use). Returns false if the entry is gone.
 * `body` may be passed when the caller just produced the result (so a cache outage doesn't lose it).
 */
export async function sendCachedResult(req, res, { cache, key, codecs, metrics, cfg, logger, cacheStatus, body }) {
  body = body ?? (await cache.get(key));
  if (!body) return false;

  let enc = body.length >= cfg.compression.threshold ? negotiateEncoding(req, codecs.available) : "identity";
  res.vary("Accept-Encoding");
  res.set({ "Cache-Control": "private, no-cache", "X-Cache": cacheStatus });
  res.locals.cacheStatus = cacheStatus;

  let etag = makeEtag(key, enc);
  if (etagMatches(req.headers["if-none-match"], etag)) {
    res.set("ETag", etag).status(304).end();
    return true;
  }

  let payload = body;
  if (enc !== "identity") {
    const cached = await cache.get(key, enc);
    if (cached) {
      payload = cached;
      res.set("X-Cache-Variant", "HIT");
    } else {
      try {
        const t0 = performance.now();
        payload = await codecs.encode(enc, body);
        metrics.observeCompression(enc, body.length, payload.length, (performance.now() - t0) / 1000);
        await cache.set(key, payload, enc);
        res.set("X-Cache-Variant", "MISS");
      } catch (err) {
        logger.warn({ err: err.message, enc }, "compression failed; sending uncompressed");
        enc = "identity";
        payload = body;
      }
    }
  }
  etag = makeEtag(key, enc);
  res.locals.skipCompression = true; // already handled here; don't let the generic middleware touch it
  res.locals.originalBytes = body.length;
  res.locals.compressedBytes = payload.length;
  res.set({ "Content-Type": "application/json; charset=utf-8", ETag: etag, "Content-Length": payload.length });
  if (enc !== "identity") res.set("Content-Encoding", enc);
  res.status(200).end(req.method === "HEAD" ? undefined : payload);
  return true;
}
