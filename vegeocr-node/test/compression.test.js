import assert from "node:assert/strict";
import { after, before, describe, test } from "node:test";
import express from "express";
import http from "node:http";
import zlib from "node:zlib";
import pino from "pino";
import { createCodecs } from "../src/lib/codecs.js";
import { negotiateEncoding, parseQ } from "../src/lib/negotiate.js";
import { dynamicCompression } from "../src/middleware/compression.js";
import { createMetrics } from "../src/metrics.js";
import { createTestApp, json, request } from "./helpers.js";

const h = (v) => ({ headers: v === undefined ? {} : { "accept-encoding": v } });
const ALL = ["zstd", "br", "gzip", "identity"];

describe("Accept-Encoding negotiation", () => {
  const cases = [
    [undefined, "identity", "no header -> identity"],
    ["", "identity", "empty header -> identity"],
    ["gzip, deflate, br, zstd", "zstd", "browser header: equal q -> OUR order wins (zstd), not the client's first (gzip)"],
    ["gzip, deflate, br", "br", "no zstd offered -> br"],
    ["gzip", "gzip", "gzip only"],
    ["zstd;q=0.2, br;q=0.9, gzip;q=0.5", "br", "q-values beat server order"],
    ["br;q=0.5, gzip;q=0.5", "br", "tie -> br before gzip"],
    ["gzip;q=1, br;q=0.9", "gzip", "client strongly prefers gzip"],
    ["*", "zstd", "wildcard -> best we have"],
    ["*;q=0.1, gzip", "gzip", "explicit beats wildcard"],
    ["br;q=0, gzip", "gzip", "q=0 excludes an encoding"],
    ["gzip;q=0, br;q=0, zstd;q=0", "identity", "everything refused -> identity"],
    ["identity", "identity", "identity only"],
    ["deflate, compress", "identity", "only encodings we don't support -> identity"],
    ["GZIP, BR", "br", "case-insensitive"],
  ];
  for (const [header, expected, why] of cases) test(`${JSON.stringify(header)} -> ${expected} (${why})`, () => assert.equal(negotiateEncoding(h(header), ALL), expected));

  test("an encoding the server lacks is never chosen", () => assert.equal(negotiateEncoding(h("zstd, gzip"), ["br", "gzip", "identity"]), "gzip"));
  test("parseQ reads weights", () => assert.deepEqual(parseQ("gzip;q=0.8, br, *;q=0.1"), { gzip: 0.8, br: 1, "*": 0.1 }));
});

describe("dynamic compression middleware", () => {
  let server, url, codecs, failing = false;
  const big = JSON.stringify({ rows: Array.from({ length: 400 }, (_, i) => ({ i, name: `item-${i}` })) });
  before(async () => {
    const real = await createCodecs({ levels: { zstd: 3, br: 5, gzip: 6 }, zstdImpl: "auto" });
    codecs = { ...real, encode: (n, b, l) => (failing && n !== "identity" ? Promise.reject(new Error("boom")) : real.encode(n, b, l)) };
    const cfg = { compression: { threshold: 1024 } };
    const app = express();
    app.use(dynamicCompression({ codecs, cfg, metrics: createMetrics({ defaults: false }), logger: pino({ level: "silent" }) }));
    app.get("/json", (_q, r) => r.type("application/json").send(big));
    app.get("/small", (_q, r) => r.json({ ok: true }));
    app.get("/text", (_q, r) => r.type("text/plain").send("hello ".repeat(500)));
    app.get("/svg", (_q, r) => r.type("image/svg+xml").send("<svg>" + "<path d='M0 0'/>".repeat(200) + "</svg>"));
    app.get("/png", (_q, r) => r.type("image/png").send(Buffer.alloc(5000, 7)));
    app.get("/pdf", (_q, r) => r.type("application/pdf").send(Buffer.alloc(5000, 7)));
    app.get("/pre", (_q, r) => r.set({ "Content-Type": "application/json", "Content-Encoding": "gzip" }).send(zlib.gzipSync(big)));
    app.get("/notransform", (_q, r) => r.set("Cache-Control", "no-transform").type("application/json").send(big));
    app.get("/nocontent", (_q, r) => r.status(204).end());
    server = http.createServer(app);
    await new Promise((r) => server.listen(0, "127.0.0.1", r));
    url = `http://127.0.0.1:${server.address().port}`;
  });
  after(() => new Promise((r) => { server.closeAllConnections(); server.close(r); }));

  const get = (p, ae, method = "GET") => request(url, { method, path: p, headers: ae === undefined ? { "accept-encoding": "" } : { "accept-encoding": ae } });
  const decode = { zstd: (b) => zlib.zstdDecompressSync(b), br: zlib.brotliDecompressSync, gzip: zlib.gunzipSync };

  for (const enc of ["zstd", "br", "gzip"]) {
    test(`${enc}: negotiated, round-trips, smaller, sets Vary`, async () => {
      const r = await get("/json", enc);
      assert.equal(r.headers["content-encoding"], enc);
      assert.match(r.headers.vary, /Accept-Encoding/i);
      assert.equal(Number(r.headers["content-length"]), r.body.length);
      assert.equal(decode[enc](r.body).toString(), big);
      assert.ok(r.body.length < big.length / 3);
    });
  }
  test("equal q-values pick zstd over br over gzip", async () => assert.equal((await get("/json", "gzip, deflate, br, zstd")).headers["content-encoding"], "zstd"));
  test("client preference by q is honoured", async () => assert.equal((await get("/json", "zstd;q=0.1, gzip;q=0.9")).headers["content-encoding"], "gzip"));
  test("no Accept-Encoding -> identity, but Vary still set", async () => {
    const r = await get("/json");
    assert.equal(r.headers["content-encoding"], undefined);
    assert.match(r.headers.vary, /Accept-Encoding/i);
    assert.equal(r.body.toString(), big);
  });
  test("below the 1024-byte threshold -> not compressed (Vary still set)", async () => {
    const r = await get("/small", "br");
    assert.equal(r.headers["content-encoding"], undefined);
    assert.match(r.headers.vary, /Accept-Encoding/i);
  });
  test("text/* and svg are compressed", async () => {
    assert.equal((await get("/text", "br")).headers["content-encoding"], "br");
    assert.equal((await get("/svg", "br")).headers["content-encoding"], "br");
  });
  test("images and PDFs are never recompressed", async () => {
    for (const p of ["/png", "/pdf"]) {
      const r = await get(p, "zstd, br, gzip");
      assert.equal(r.headers["content-encoding"], undefined, p);
      assert.equal(r.body.length, 5000);
    }
  });
  test("HEAD is never compressed and has no body", async () => {
    const r = await get("/json", "br", "HEAD");
    assert.equal(r.headers["content-encoding"], undefined);
    assert.equal(r.body.length, 0);
  });
  test("already-encoded and no-transform responses pass through untouched", async () => {
    assert.equal((await get("/pre", "br")).headers["content-encoding"], "gzip");
    assert.equal((await get("/notransform", "br")).headers["content-encoding"], undefined);
  });
  test("204 stays empty", async () => assert.equal((await get("/nocontent", "br")).status, 204));
  test("compression failure falls back to the uncompressed body", async () => {
    failing = true;
    try {
      const r = await get("/json", "br");
      assert.equal(r.status, 200);
      assert.equal(r.headers["content-encoding"], undefined);
      assert.equal(r.body.toString(), big);
    } finally { failing = false; }
  });
  test("ETag differs per encoding", async () => {
    const a = await get("/json", "gzip"), b = await get("/json", "br");
    assert.notEqual(a.headers.etag, b.headers.etag);
  });
});

describe("compression of cached OCR results", () => {
  let t;
  before(async () => { t = await createTestApp(); });
  after(() => t.close());
  test("each encoding decodes to the identical JSON document", async () => {
    const { png } = await import("./helpers.js");
    const file = await png(11);
    const plain = json(await t.post({ file }));
    for (const enc of ["zstd", "br", "gzip"]) {
      const r = await t.post({ file, headers: { "accept-encoding": enc } });
      assert.equal(r.headers["content-encoding"], enc);
      assert.deepEqual(JSON.parse(({ zstd: zlib.zstdDecompressSync, br: zlib.brotliDecompressSync, gzip: zlib.gunzipSync })[enc](r.body)), plain);
    }
  });
});
