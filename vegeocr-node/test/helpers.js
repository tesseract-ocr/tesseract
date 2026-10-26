import { randomUUID } from "node:crypto";
import fs from "node:fs";
import http from "node:http";
import os from "node:os";
import path from "node:path";
import pino from "pino";
import sharp from "sharp";
import { buildApp } from "../src/app.js";
import { loadConfig } from "../src/config.js";
import { createCache } from "../src/lib/cache.js";
import { createCodecs } from "../src/lib/codecs.js";
import { createMetrics } from "../src/metrics.js";
import { createMemoryQueue } from "../src/queue/memoryQueue.js";
import { createProcessor } from "../src/queue/processor.js";

export const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

/** Raw HTTP so tests see the bytes exactly as sent (fetch would transparently decode Content-Encoding). */
export function request(baseUrl, { method = "GET", path: p = "/", headers = {}, body } = {}) {
  return new Promise((resolve, reject) => {
    const u = new URL(p, baseUrl);
    const req = http.request(u, { method, headers: { "accept-encoding": "identity", ...headers } }, (res) => {
      const chunks = [];
      res.on("data", (c) => chunks.push(c));
      res.on("end", () => resolve({ status: res.statusCode, headers: res.headers, body: Buffer.concat(chunks) }));
    });
    req.on("error", reject);
    if (body) req.write(body);
    req.end();
  });
}
export const json = (res) => JSON.parse(res.body.toString("utf8"));

/** Builds a multipart/form-data body. `file` may be null to send fields only. */
export function multipart({ file, fieldName = "file", filename = "scan.png", contentType = "image/png", fields = {} }) {
  const boundary = `----t${randomUUID().replaceAll("-", "")}`;
  const parts = [];
  for (const [k, v] of Object.entries(fields)) parts.push(Buffer.from(`--${boundary}\r\nContent-Disposition: form-data; name="${k}"\r\n\r\n${v}\r\n`));
  if (file) parts.push(Buffer.from(`--${boundary}\r\nContent-Disposition: form-data; name="${fieldName}"; filename="${filename}"\r\nContent-Type: ${contentType}\r\n\r\n`), file, Buffer.from("\r\n"));
  parts.push(Buffer.from(`--${boundary}--\r\n`));
  return { body: Buffer.concat(parts), headers: { "content-type": `multipart/form-data; boundary=${boundary}` } };
}

/** A valid PNG that is unique per `seed` (so each gets its own content hash). */
export async function png(seed = 0, size = 64) {
  return sharp({ create: { width: size, height: size, channels: 3, background: { r: seed % 256, g: (seed * 7) % 256, b: 90 } } }).png().toBuffer();
}
export const pdfBytes = (seed = 0) => Buffer.from(`%PDF-1.4\n% test ${seed}\n1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n2 0 obj<</Type/Pages/Kids[]/Count 0>>endobj\ntrailer<</Root 1 0 R>>\n%%EOF\n`);

/** A realistic-size OCR result (word-level boxes) so compression has something to chew on. */
export function bigResult(lang = "eng", words = 1500) {
  const lines = [];
  for (let i = 0; i < words / 10; i++) {
    const ws = Array.from({ length: 10 }, (_, j) => ({ text: `word${i}_${j}`, confidence: 90 + ((i + j) % 10), bbox: [j * 90, i * 30, j * 90 + 80, i * 30 + 24] }));
    lines.push({ text: ws.map((w) => w.text).join(" "), confidence: 95, bbox: [0, i * 30, 900, i * 30 + 24], words: ws });
  }
  return { status: "done", schema: 1, lang, pageCount: 1, text: lines.map((l) => l.text).join("\n"), confidence: 95, pages: [{ page: 1, width: 1000, height: 3000, text: "", lines }], meta: { engine: "fake", ocrMs: 1 } };
}

/** OCR stand-in: records calls, reports progress, optionally slow or failing. */
export function fakeOcr({ delay = 0, words = 1500, fail = null } = {}) {
  const calls = [];
  const fn = async ({ filePath, mime, lang, onProgress }) => {
    calls.push({ filePath, mime, lang });
    onProgress?.({ page: 0, total: 2 });
    await sleep(delay);
    onProgress?.({ page: 1, total: 2 });
    await sleep(delay);
    onProgress?.({ page: 2, total: 2 });
    if (fail) throw fail;
    return bigResult(lang, words);
  };
  fn.calls = calls;
  return fn;
}

/** Spins up the full Express app on an ephemeral port with injectable dependencies. */
export async function createTestApp({ env = {}, runOcr = fakeOcr(), codecs, queue: queueFactory, cache: cacheOverride } = {}) {
  const uploadDir = fs.mkdtempSync(path.join(os.tmpdir(), "vegeocr-test-"));
  const cfg = loadConfig({ NODE_ENV: "test", LOG_LEVEL: "silent", UPLOAD_DIR: uploadDir, PUBLIC_DIR: path.join(uploadDir, "no-public"), RATE_LIMIT_OCR_MAX: "1000", RATE_LIMIT_MAX: "1000", OCR_POOL_SIZE: "2", ...env });
  const logger = pino({ level: "silent" });
  const metrics = createMetrics({ defaults: false });
  const realCodecs = codecs ?? (await createCodecs(cfg.compression));
  const cache = cacheOverride ?? createCache({ ttlSeconds: 60, lruMaxBytes: 64 * 1024 * 1024, logger });
  const processor = createProcessor({ runOcr, cache, metrics, logger });
  const queue = queueFactory ? await queueFactory({ processor, cfg }) : createMemoryQueue({ processor, concurrency: 2, jobTtlSeconds: 60 });
  const app = buildApp({ cfg, logger, metrics, cache, codecs: realCodecs, queue, runOcr });
  const server = http.createServer(app);
  await new Promise((r) => server.listen(0, "127.0.0.1", r));
  const url = `http://127.0.0.1:${server.address().port}`;
  return {
    url, cfg, cache, queue, metrics, runOcr, uploadDir, app,
    post: async ({ file, headers = {}, query = "", ...opts } = {}) => {
      const mp = multipart({ file, ...opts });
      return request(url, { method: "POST", path: `/api/ocr${query}`, body: mp.body, headers: { ...mp.headers, ...headers } });
    },
    leftoverUploads: () => fs.readdirSync(uploadDir).filter((f) => f.endsWith(".upload")),
    close: async () => {
      server.closeAllConnections();
      await new Promise((r) => server.close(r));
      await queue.close();
      await cache.close();
      fs.rmSync(uploadDir, { recursive: true, force: true });
    },
  };
}

export async function waitFor(fn, { timeout = 5000, interval = 25 } = {}) {
  const end = Date.now() + timeout;
  for (;;) {
    const v = await fn();
    if (v) return v;
    if (Date.now() > end) throw new Error("waitFor timed out");
    await sleep(interval);
  }
}
