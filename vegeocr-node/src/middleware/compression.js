import { isCompressibleType, negotiateEncoding } from "../lib/negotiate.js";

/**
 * Compresses text-like responses on the fly, off the event loop (zlib/brotli/zstd run on libuv's threadpool).
 *
 * Skipped: HEAD, 204/304, bodies under the threshold, non-text types (images, PDFs...), responses that already carry a
 * Content-Encoding (pre-compressed statics, cached variants) or `Cache-Control: no-transform`, and event streams.
 * If compression throws, the original bytes are sent unchanged. `Vary: Accept-Encoding` is always set.
 */
export function dynamicCompression({ codecs, cfg, metrics, logger }) {
  return function compression(req, res, next) {
    res.vary("Accept-Encoding");
    if (req.method === "HEAD") return next();
    const origEnd = res.end.bind(res);

    res.end = function end(chunk, encoding, cb) {
      if (typeof chunk === "function") { cb = chunk; chunk = undefined; encoding = undefined; }
      if (typeof encoding === "function") { cb = encoding; encoding = undefined; }
      const eligible =
        chunk && (typeof chunk === "string" || Buffer.isBuffer(chunk)) && !res.headersSent && !res.locals.skipCompression &&
        !res.getHeader("Content-Encoding") && ![204, 304].includes(res.statusCode) &&
        !/no-transform/i.test(String(res.getHeader("Cache-Control") ?? "")) && isCompressibleType(String(res.getHeader("Content-Type") ?? ""));
      if (!eligible) return origEnd(chunk, encoding, cb);

      const buf = Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk, encoding || "utf8");
      const enc = negotiateEncoding(req, codecs.available);
      res.end = origEnd; // never recurse
      if (buf.length < cfg.compression.threshold || enc === "identity") return origEnd(buf, cb);

      const t0 = performance.now();
      codecs.encode(enc, buf).then(
        (out) => {
          if (out.length >= buf.length) return origEnd(buf, cb); // incompressible: don't make it bigger
          metrics.observeCompression(enc, buf.length, out.length, (performance.now() - t0) / 1000);
          res.locals.originalBytes = buf.length;
          res.locals.compressedBytes = out.length;
          res.setHeader("Content-Encoding", enc);
          res.setHeader("Content-Length", out.length);
          const etag = res.getHeader("ETag"); // a different representation needs a different validator
          if (etag) res.setHeader("ETag", String(etag).replace(/"$/, `-${enc}"`));
          origEnd(out, cb);
        },
        (err) => {
          logger.warn({ err: err.message, enc }, "compression failed; sending uncompressed");
          origEnd(buf, cb);
        },
      );
      return res;
    };
    next();
  };
}
