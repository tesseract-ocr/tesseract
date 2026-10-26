import { createReadStream } from "node:fs";
import { stat } from "node:fs/promises";
import path from "node:path";
import { pipeline } from "node:stream/promises";
import mime from "mime-types";
import { ENCODING_ORDER } from "../lib/codecs.js";
import { negotiateEncoding } from "../lib/negotiate.js";
import { etagMatches } from "../lib/etag.js";

const EXT = { zstd: ".zst", br: ".br", gzip: ".gz" };
const HASHED = /[.-][0-9a-f]{8,}\.[a-z0-9]+$/i; // app.3f2a9c1b.js

/**
 * Serves build output from `root`, preferring the pre-compressed sibling (.zst / .br / .gz) the client accepts best.
 * Hashed filenames get `immutable` one-year caching; everything else (index.html) must revalidate.
 * File lookups are memoised so steady-state requests do no stat() calls beyond the first.
 */
export function staticPrecompressed(root, { available = ENCODING_ORDER } = {}) {
  const resolvedRoot = path.resolve(root);
  const memo = new Map();
  const statFile = async (file) => {
    if (memo.has(file)) return memo.get(file);
    const info = await stat(file).then((s) => (s.isFile() ? { size: s.size, mtimeMs: s.mtimeMs } : null), () => null);
    memo.set(file, info);
    return info;
  };

  return async function serve(req, res, next) {
    if (req.method !== "GET" && req.method !== "HEAD") return next();
    try {
      let rel = decodeURIComponent(req.path);
      if (rel.endsWith("/")) rel += "index.html";
      const file = path.join(resolvedRoot, rel);
      if (!file.startsWith(resolvedRoot + path.sep)) return next(); // path traversal

      const plain = await statFile(file);
      if (!plain) return next();
      const type = mime.lookup(file) || "application/octet-stream";
      const wanted = negotiateEncoding(req, available.filter((e) => e === "identity" || EXT[e]));
      let servedFile = file, info = plain, enc = "identity";
      if (wanted !== "identity") {
        const pre = await statFile(file + EXT[wanted]);
        if (pre) { servedFile = file + EXT[wanted]; info = pre; enc = wanted; }
      }

      const etag = `"${info.size.toString(16)}-${Math.floor(info.mtimeMs).toString(16)}-${enc}"`;
      res.vary("Accept-Encoding");
      res.set({
        "Content-Type": /^(text|application\/(javascript|json))/.test(type) ? `${type}; charset=utf-8` : type,
        "Cache-Control": HASHED.test(file) ? "public, max-age=31536000, immutable" : "no-cache",
        ETag: etag,
        "Last-Modified": new Date(plain.mtimeMs).toUTCString(),
        "X-Content-Type-Options": "nosniff",
      });
      if (enc !== "identity") res.set("Content-Encoding", enc);
      if (etagMatches(req.headers["if-none-match"], etag)) return res.status(304).end();
      res.set("Content-Length", info.size);
      if (req.method === "HEAD") return res.end();
      await pipeline(createReadStream(servedFile), res);
    } catch (err) {
      if (err?.code === "ERR_STREAM_PREMATURE_CLOSE") return; // client went away
      next(err);
    }
  };
}
