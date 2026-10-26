import assert from "node:assert/strict";
import fs from "node:fs";
import http from "node:http";
import os from "node:os";
import path from "node:path";
import { after, before, describe, test } from "node:test";
import zlib from "node:zlib";
import express from "express";
import { precompress } from "../scripts/precompress.js";
import { ENCODING_ORDER } from "../src/lib/codecs.js";
import { staticPrecompressed } from "../src/middleware/staticPrecompressed.js";
import { request } from "./helpers.js";

describe("pre-compressed static assets", () => {
  let dir, server, url;
  const js = "export const answer = 42;\n".repeat(400);
  before(async () => {
    dir = fs.mkdtempSync(path.join(os.tmpdir(), "vegeocr-static-"));
    fs.writeFileSync(path.join(dir, "app.3f2a9c1b.js"), js);
    fs.writeFileSync(path.join(dir, "index.html"), "<!doctype html><title>t</title>" + "<p>hello</p>".repeat(100));
    fs.writeFileSync(path.join(dir, "logo.png"), Buffer.alloc(3000, 1));
    fs.writeFileSync(path.join(dir, "tiny.css"), "a{}"); // too small to bother compressing
    fs.writeFileSync(path.join(dir, "..secret"), "x");
    await precompress(dir, { log: () => {} });
    const app = express();
    app.use(staticPrecompressed(dir, { available: ENCODING_ORDER }));
    server = http.createServer(app);
    await new Promise((r) => server.listen(0, "127.0.0.1", r));
    url = `http://127.0.0.1:${server.address().port}`;
  });
  after(() => { server.closeAllConnections(); server.close(); fs.rmSync(dir, { recursive: true, force: true }); });
  const get = (p, ae, extra = {}) => request(url, { path: p, headers: { "accept-encoding": ae, ...extra } });

  test("build step wrote .zst/.br/.gz for text assets, none for images or tiny files", () => {
    for (const ext of [".zst", ".br", ".gz"]) assert.ok(fs.existsSync(path.join(dir, `app.3f2a9c1b.js${ext}`)), ext);
    assert.ok(!fs.existsSync(path.join(dir, "logo.png.br")));
    assert.ok(!fs.existsSync(path.join(dir, "tiny.css.br")));
  });
  test("each pre-compressed file decompresses to the original", () => {
    const f = path.join(dir, "app.3f2a9c1b.js");
    assert.equal(zlib.gunzipSync(fs.readFileSync(f + ".gz")).toString(), js);
    assert.equal(zlib.brotliDecompressSync(fs.readFileSync(f + ".br")).toString(), js);
    if (typeof zlib.zstdDecompressSync === "function") assert.equal(zlib.zstdDecompressSync(fs.readFileSync(f + ".zst")).toString(), js);
  });
  test("build-time max levels beat the request-time defaults on realistic text", async () => {
    const { createCodecs } = await import("../src/lib/codecs.js");
    const c = await createCodecs({ levels: { zstd: 3, br: 5, gzip: 6 }, zstdImpl: "auto" });
    let seed = 7;
    const rnd = () => (seed = (seed * 1103515245 + 12345) % 2147483648) / 2147483648;
    const words = Array.from({ length: 300 }, () => Math.random().toString(36).slice(2, 2 + 3 + Math.floor(rnd() * 8)));
    const text = Buffer.from(Array.from({ length: 6000 }, () => words[Math.floor(rnd() * words.length)]).join(" "));
    for (const [enc, fast, max] of [["br", 5, 11], ["gzip", 6, 9], ["zstd", 3, 19]]) {
      if (!c.available.includes(enc)) continue;
      const a = (await c.encode(enc, text, fast)).length, b = (await c.encode(enc, text, max)).length;
      assert.ok(b <= a, `${enc}: level ${max} (${b}) should not be larger than level ${fast} (${a})`);
    }
  });
  test("serves the best encoding the client accepts, with correct headers", async () => {
    for (const [ae, want] of [["zstd, br, gzip", "zstd"], ["br, gzip", "br"], ["gzip", "gzip"]]) {
      const r = await get("/app.3f2a9c1b.js", ae);
      assert.equal(r.headers["content-encoding"], want, ae);
      assert.match(r.headers["content-type"], /javascript/);
      assert.match(r.headers.vary, /Accept-Encoding/i);
      assert.equal(Number(r.headers["content-length"]), r.body.length);
    }
    const plain = await get("/app.3f2a9c1b.js", "identity");
    assert.equal(plain.headers["content-encoding"], undefined);
    assert.equal(plain.body.toString(), js);
  });
  test("hashed filename -> immutable one-year cache; index.html -> must revalidate", async () => {
    assert.equal((await get("/app.3f2a9c1b.js", "br")).headers["cache-control"], "public, max-age=31536000, immutable");
    assert.equal((await get("/index.html", "br")).headers["cache-control"], "no-cache");
    assert.equal((await get("/", "br")).status, 200, "/ serves index.html");
  });
  test("conditional request -> 304 (per-encoding ETag)", async () => {
    const r = await get("/app.3f2a9c1b.js", "br");
    assert.equal((await get("/app.3f2a9c1b.js", "br", { "if-none-match": r.headers.etag })).status, 304);
    assert.equal((await get("/app.3f2a9c1b.js", "gzip", { "if-none-match": r.headers.etag })).status, 200);
  });
  test("images are served as-is", async () => {
    const r = await get("/logo.png", "br");
    assert.equal(r.headers["content-encoding"], undefined);
    assert.equal(r.headers["content-type"], "image/png");
    assert.equal(r.body.length, 3000);
  });
  test("HEAD has headers but no body", async () => {
    const r = await request(url, { method: "HEAD", path: "/app.3f2a9c1b.js", headers: { "accept-encoding": "br" } });
    assert.equal(r.status, 200);
    assert.equal(r.body.length, 0);
    assert.equal(r.headers["content-encoding"], "br");
  });
  test("path traversal and missing files do not escape the root", async () => {
    for (const p of ["/../etc/passwd", "/%2e%2e/etc/passwd", "/..%2f..%2fetc/passwd", "/nope.js"]) {
      const r = await request(url, { path: p });
      assert.ok([404, 400].includes(r.status), `${p} -> ${r.status}`);
    }
  });
});
