import { randomUUID } from "node:crypto";
import { mkdirSync } from "node:fs";
import path from "node:path";
import { createBrotliDecompress, createGunzip, createInflate } from "node:zlib";
import zlib from "node:zlib";
import { Transform } from "node:stream";
import multer from "multer";

/** Streams the multipart upload straight to disk (never buffered whole in memory) under hard limits. */
export function createUpload(uploadCfg) {
  mkdirSync(uploadCfg.dir, { recursive: true });
  return multer({
    storage: multer.diskStorage({
      destination: uploadCfg.dir,
      filename: (_req, _file, cb) => cb(null, `${randomUUID()}.upload`), // never trust the client's filename
    }),
    limits: { fileSize: uploadCfg.maxBytes, files: 1, fields: 8, fieldSize: 1024, parts: 10 },
  }).single("file");
}

const decoders = {
  gzip: () => createGunzip(),
  "x-gzip": () => createGunzip(),
  deflate: () => createInflate(),
  br: () => createBrotliDecompress(),
  ...(typeof zlib.createZstdDecompress === "function" ? { zstd: () => zlib.createZstdDecompress() } : {}),
};

/**
 * Accepts `Content-Encoding: gzip | deflate | br | zstd` request bodies (handy for big uploads from API clients).
 * The decompressed stream is capped: a tiny body that inflates to gigabytes is cut off with 413 (decompression bomb).
 * Only the upload route uses this: it works by replacing req.pipe(), which is how multer/busboy consume the body.
 */
export function decompressRequestBody(maxDecompressedBytes) {
  return function decompress(req, res, next) {
    const enc = String(req.headers["content-encoding"] ?? "").toLowerCase().trim();
    if (!enc || enc === "identity") return next();
    const make = decoders[enc];
    if (!make) return res.status(415).json({ error: { code: "unsupported_content_encoding", message: `Content-Encoding '${enc}' is not supported` } });

    let seen = 0;
    const fail = (status, code, message) => {
      if (res.headersSent) return;
      res.set("Connection", "close").status(status).json({ error: { code, message } });
      req.unpipe();
      req.destroy();
    };
    const decoder = make();
    const cap = new Transform({
      transform(chunk, _e, cb) {
        seen += chunk.length;
        if (seen > maxDecompressedBytes) return cb(Object.assign(new Error("too big"), { bomb: true }));
        cb(null, chunk);
      },
    });
    decoder.on("error", () => fail(400, "bad_content_encoding", `The request body is not valid ${enc} data`));
    cap.on("error", (e) => (e.bomb ? fail(413, "payload_too_large", `Decompressed body exceeds ${Math.floor(maxDecompressedBytes / 1048576)} MB`) : fail(400, "bad_content_encoding", "Invalid request body")));
    req.pipe(decoder).pipe(cap);
    req.pipe = (dest, opts) => cap.pipe(dest, opts); // consumers now read the decompressed bytes
    delete req.headers["content-encoding"];
    delete req.headers["content-length"]; // no longer true: the decompressed size is unknown up front
    // multer's body check (type-is) only believes a request has a body if Content-Length or Transfer-Encoding is present,
    // so without this, clients that send Content-Length (curl, most HTTP libraries) had their upload silently skipped.
    req.headers["transfer-encoding"] = "chunked";
    next();
  };
}
