import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { after, before, describe, test } from "node:test";
import zlib from "node:zlib";
import { createOcrPool } from "../src/workers/pool.js";
import { loadConfig } from "../src/config.js";
import { createTestApp, fakeOcr, json, multipart, pdfBytes, png, request } from "./helpers.js";

const code = (r) => json(r).error?.code;

describe("upload validation", () => {
  let t;
  before(async () => { t = await createTestApp({ env: { MAX_UPLOAD_MB: "1", MAX_DECOMPRESSED_MB: "2" } }); });
  after(() => t.close());

  test("valid PNG is accepted", async () => assert.equal((await t.post({ file: await png(31) })).status, 200));
  test("no file -> 400 missing_file", async () => {
    const r = await t.post({ file: null });
    assert.equal(r.status, 400);
    assert.equal(code(r), "missing_file");
  });
  test("wrong field name -> 400 unexpected_field", async () => {
    const r = await t.post({ file: await png(32), fieldName: "image" });
    assert.equal(r.status, 400);
    assert.equal(code(r), "unexpected_field");
  });
  test("text file renamed .png with an image Content-Type -> 415 (magic bytes win)", async () => {
    const r = await t.post({ file: Buffer.from("this is definitely not a png"), filename: "evil.png", contentType: "image/png" });
    assert.equal(r.status, 415);
    assert.equal(code(r), "unsupported_type");
  });
  test("a real PNG sent with a bogus Content-Type is still accepted", async () => {
    assert.equal((await t.post({ file: await png(33), contentType: "text/plain", filename: "x.txt" })).status, 200);
  });
  test("an executable is rejected", async () => {
    assert.equal((await t.post({ file: Buffer.concat([Buffer.from([0x7f, 0x45, 0x4c, 0x46]), Buffer.alloc(200)]), filename: "a.png" })).status, 415);
  });
  test("file over the size limit -> 413 file_too_large", async () => {
    const big = Buffer.concat([await png(34), Buffer.alloc(1.5 * 1024 * 1024)]);
    const r = await t.post({ file: big });
    assert.equal(r.status, 413);
    assert.equal(code(r), "file_too_large");
  });
  test("two files in one request -> 400 (one file per request)", async () => {
    const b = "----twofiles";
    const part = (buf) => Buffer.concat([Buffer.from(`--${b}\r\nContent-Disposition: form-data; name="file"; filename="a.png"\r\nContent-Type: image/png\r\n\r\n`), buf, Buffer.from("\r\n")]);
    const body = Buffer.concat([part(await png(61)), part(await png(62)), Buffer.from(`--${b}--\r\n`)]);
    const r = await request(t.url, { method: "POST", path: "/api/ocr", body, headers: { "content-type": `multipart/form-data; boundary=${b}` } });
    assert.equal(r.status, 400, r.body.toString());
  });
  test("unsupported language -> 422 invalid_lang; unknown mode -> 422", async () => {
    assert.equal(code(await t.post({ file: await png(36), fields: { lang: "klingon" } })), "invalid_lang");
    assert.equal(code(await t.post({ file: await png(37), fields: { lang: "eng+klingon" } })), "invalid_lang");
    assert.equal(code(await t.post({ file: await png(38), query: "?mode=turbo" })), "invalid_mode");
  });
  test("lang accepts allowed combinations", async () => assert.equal((await t.post({ file: await png(39), fields: { lang: "jpn+eng" } })).status, 200));
  test("malformed multipart -> 400", async () => {
    const r = await request(t.url, { method: "POST", path: "/api/ocr", body: Buffer.from("garbage"), headers: { "content-type": "multipart/form-data; boundary=zzz" } });
    assert.equal(r.status, 400);
  });
  test("no temp files are left behind after any of the above", async () => assert.deepEqual(t.leftoverUploads(), []));
});

describe("compressed request bodies", () => {
  let t;
  before(async () => { t = await createTestApp({ env: { MAX_UPLOAD_MB: "10", MAX_DECOMPRESSED_MB: "1" } }); });
  after(() => t.close());
  const send = (body, enc, headers = {}) => request(t.url, { method: "POST", path: "/api/ocr", body, headers: { ...headers, "content-encoding": enc } });

  test("gzip / br / deflate bodies are decompressed and processed", async () => {
    const mp = multipart({ file: await png(41, 200) });
    for (const [enc, fn] of [["gzip", zlib.gzipSync], ["br", zlib.brotliCompressSync], ["deflate", zlib.deflateSync]]) {
      const r = await send(fn(mp.body), enc, mp.headers);
      assert.equal(r.status, 200, `${enc}: ${r.body}`);
    }
  });
  test("regression: a compressed body sent WITH Content-Length (curl, most HTTP libraries) is parsed too", async () => {
    const mp = multipart({ file: await png(45, 200) });
    const gz = zlib.gzipSync(mp.body);
    const r = await send(gz, "gzip", { ...mp.headers, "content-length": String(gz.length) });
    assert.equal(r.status, 200, r.body.toString());
  });
  test("zstd body is accepted when the runtime supports it", { skip: typeof zlib.zstdCompressSync !== "function" }, async () => {
    const mp = multipart({ file: await png(42, 200) });
    assert.equal((await send(zlib.zstdCompressSync(mp.body), "zstd", mp.headers)).status, 200);
  });
  test("decompression bomb: tiny gzip that inflates past the cap -> 413", async () => {
    const bomb = zlib.gzipSync(Buffer.alloc(8 * 1024 * 1024)); // 8 MB of zeros ~ 8 KB on the wire, cap is 1 MB
    assert.ok(bomb.length < 20_000);
    const mp = multipart({ file: Buffer.alloc(0) });
    const r = await send(bomb, "gzip", mp.headers);
    assert.equal(r.status, 413);
    assert.equal(code(r), "payload_too_large");
  });
  test("corrupt gzip -> 400 bad_content_encoding", async () => {
    const mp = multipart({ file: await png(43) });
    const r = await send(Buffer.from("not gzip at all"), "gzip", mp.headers);
    assert.equal(r.status, 400);
    assert.equal(code(r), "bad_content_encoding");
  });
  test("unsupported Content-Encoding -> 415", async () => {
    const mp = multipart({ file: await png(44) });
    assert.equal((await send(mp.body, "compress", mp.headers)).status, 415);
  });
  test("server still healthy afterwards", async () => assert.equal((await request(t.url, { path: "/health" })).status, 200));
});

describe("PDF page limit (real worker pool + mupdf)", () => {
  let t, pool;
  const threePagePdf = () => {
    // Minimal 3-page PDF; mupdf rebuilds the missing xref table.
    const objs = ["<</Type/Catalog/Pages 2 0 R>>", "<</Type/Pages/Kids[3 0 R 4 0 R 5 0 R]/Count 3>>", ...[0, 1, 2].map(() => "<</Type/Page/Parent 2 0 R/MediaBox[0 0 200 200]>>")];
    return Buffer.from(`%PDF-1.4\n${objs.map((o, i) => `${i + 1} 0 obj\n${o}\nendobj\n`).join("")}trailer\n<</Root 1 0 R/Size 6>>\n%%EOF\n`);
  };
  before(async () => {
    const cfg = loadConfig({ OCR_POOL_SIZE: "1", MAX_PDF_PAGES: "2", TESSERACT_BIN: "/nonexistent" });
    pool = createOcrPool(cfg.ocr, cfg.upload);
    t = await createTestApp({ runOcr: (task) => pool.run(task), env: { MAX_PDF_PAGES: "2" } });
  });
  after(async () => { await t.close(); await pool.destroy(); });

  test("sync mode: a PDF over the page limit is rejected with 422 too_many_pages", async () => {
    const r = await t.post({ file: threePagePdf(), filename: "x.pdf", contentType: "application/pdf", query: "?mode=sync" });
    assert.equal(r.status, 422, r.body.toString());
    assert.equal(code(r), "too_many_pages");
  });
  test("async mode: the job fails with the same code", async () => {
    const r = await t.post({ file: threePagePdf(), filename: "y.pdf", contentType: "application/pdf" });
    assert.equal(r.status, 202);
    const id = json(r).id;
    const done = await (async () => { for (let i = 0; i < 100; i++) { const s = json(await request(t.url, { path: `/api/ocr/${id}` })); if (s.status === "failed") return s; await new Promise((x) => setTimeout(x, 50)); } })();
    assert.equal(done.error.code, "too_many_pages");
  });
  test("a corrupt PDF is a 422, not a 500", async () => {
    const r = await t.post({ file: Buffer.from("%PDF-1.4\nthis is not really a pdf, just junk bytes\n"), filename: "z.pdf", contentType: "application/pdf", query: "?mode=sync" });
    assert.ok([422].includes(r.status), `${r.status} ${r.body}`);
  });
});
