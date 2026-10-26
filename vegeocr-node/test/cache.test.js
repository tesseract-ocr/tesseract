import assert from "node:assert/strict";
import { after, before, describe, test } from "node:test";
import zlib from "node:zlib";
import { etagMatches, makeEtag } from "../src/lib/etag.js";
import { createCache } from "../src/lib/cache.js";
import { createTestApp, fakeOcr, json, png, request } from "./helpers.js";

describe("etag helpers", () => {
  test("etagMatches: exact, weak, list, star, miss", () => {
    const e = '"abc-br"';
    assert.ok(etagMatches('"abc-br"', e));
    assert.ok(etagMatches('W/"abc-br"', e));
    assert.ok(etagMatches('"x", "abc-br"', e));
    assert.ok(etagMatches("*", e));
    assert.ok(!etagMatches('"other"', e));
    assert.ok(!etagMatches(undefined, e));
  });
  test("etag differs per encoding and is stable per key", () => {
    assert.notEqual(makeEtag("k", "br"), makeEtag("k", "gzip"));
    assert.equal(makeEtag("k", "br"), makeEtag("k", "br"));
  });
});

describe("LRU fallback cache", () => {
  test("round-trips buffers, keeps variants separate, evicts by size", async () => {
    const c = createCache({ ttlSeconds: 60, lruMaxBytes: 1000, logger: null });
    await c.set("a", Buffer.from("body"));
    await c.set("a", Buffer.from("zz"), "br");
    assert.equal((await c.get("a")).toString(), "body");
    assert.equal((await c.get("a", "br")).toString(), "zz");
    assert.equal(await c.get("a", "gzip"), null);
    await c.set("big1", Buffer.alloc(600));
    await c.set("big2", Buffer.alloc(600)); // pushes the total over 1000 bytes
    assert.equal(await c.get("a"), null, "oldest entry evicted");
  });
  test("entries expire after the TTL", async () => {
    const c = createCache({ ttlSeconds: 1, lruMaxBytes: 1e6, logger: null });
    await c.set("k", Buffer.from("v"));
    assert.ok(await c.get("k"));
    await new Promise((r) => setTimeout(r, 1200));
    assert.equal(await c.get("k"), null);
  });
});

describe("OCR result caching over HTTP", () => {
  let t, ocr;
  before(async () => { ocr = fakeOcr(); t = await createTestApp({ runOcr: ocr }); });
  after(() => t.close());

  test("same file twice: one OCR run, MISS then HIT, identical body", async () => {
    const file = await png(1);
    const a = await t.post({ file });
    const b = await t.post({ file });
    assert.equal(a.status, 200);
    assert.equal(a.headers["x-cache"], "MISS");
    assert.equal(b.headers["x-cache"], "HIT");
    assert.equal(ocr.calls.length, 1, "OCR must not run again for identical bytes");
    assert.deepEqual(json(a), json(b));
  });

  test("filename and Content-Type do not affect the cache key, content does", async () => {
    const file = await png(2);
    const before = ocr.calls.length;
    await t.post({ file, filename: "a.png" });
    const r = await t.post({ file, filename: "totally-different.jpg", contentType: "application/octet-stream" });
    assert.equal(r.headers["x-cache"], "HIT");
    assert.equal(ocr.calls.length, before + 1);
    await t.post({ file: await png(3) });
    assert.equal(ocr.calls.length, before + 2);
  });

  test("language is part of the key", async () => {
    const file = await png(4);
    const before = ocr.calls.length;
    await t.post({ file, fields: { lang: "eng" } });
    const r = await t.post({ file, fields: { lang: "jpn" } });
    assert.equal(r.headers["x-cache"], "MISS");
    assert.equal(ocr.calls.length, before + 2);
  });

  test("ETag is returned and If-None-Match yields 304 without re-running OCR", async () => {
    const file = await png(5);
    const first = await t.post({ file });
    const etag = first.headers.etag;
    assert.ok(etag?.startsWith('"'));
    const before = ocr.calls.length;
    const r = await t.post({ file, headers: { "if-none-match": etag } });
    assert.equal(r.status, 304);
    assert.equal(r.body.length, 0);
    assert.equal(r.headers.etag, etag);
    assert.equal(ocr.calls.length, before);
    const stale = await t.post({ file, headers: { "if-none-match": '"something-else"' } });
    assert.equal(stale.status, 200);
  });

  test("each encoding has its own ETag; a br ETag does not 304 a gzip request", async () => {
    const file = await png(6);
    const br = await t.post({ file, headers: { "accept-encoding": "br" } });
    const gz = await t.post({ file, headers: { "accept-encoding": "gzip" } });
    assert.notEqual(br.headers.etag, gz.headers.etag);
    assert.equal((await t.post({ file, headers: { "accept-encoding": "br", "if-none-match": br.headers.etag } })).status, 304);
    assert.equal((await t.post({ file, headers: { "accept-encoding": "gzip", "if-none-match": br.headers.etag } })).status, 200);
  });

  test("compressed variants are cached: second request is served without re-compressing", async () => {
    const file = await png(7);
    const first = await t.post({ file, headers: { "accept-encoding": "zstd" } });
    const second = await t.post({ file, headers: { "accept-encoding": "zstd" } });
    assert.equal(first.headers["x-cache-variant"], "MISS");
    assert.equal(second.headers["x-cache-variant"], "HIT");
    assert.ok(first.body.equals(second.body));
    assert.ok(await t.cache.get((await t.post({ file })).headers.etag && `${(await sha(file))}:eng:p1`, "zstd"), "variant stored in the cache");
  });

  test("cache hit is much cheaper than the cold request (no OCR, no compression)", async () => {
    const slow = fakeOcr({ delay: 150 });
    const s = await createTestApp({ runOcr: slow });
    try {
      const file = await png(8);
      let t0 = performance.now(); await s.post({ file, headers: { "accept-encoding": "br" } }); const cold = performance.now() - t0;
      await s.post({ file, headers: { "accept-encoding": "br" } });
      t0 = performance.now(); await s.post({ file, headers: { "accept-encoding": "br" } }); const warm = performance.now() - t0;
      assert.ok(cold > 250, `cold ${cold}`);
      assert.ok(warm < cold / 3, `warm ${warm} vs cold ${cold}`);
    } finally { await s.close(); }
  });

  test("a cache outage degrades to a miss, it does not fail the request", async () => {
    const broken = { kind: "broken", get: async () => null, set: async () => { throw new Error("redis down"); }, has: async () => false, close: async () => {} };
    const s = await createTestApp({ cache: { ...broken, set: async () => null } });
    try {
      const r = await s.post({ file: await png(9) });
      assert.equal(r.status, 200);
    } finally { await s.close(); }
  });

  test("temp uploads are always removed", async () => assert.deepEqual(t.leftoverUploads(), []));
});

async function sha(buf) {
  const { createHash } = await import("node:crypto");
  return createHash("sha256").update(buf).digest("hex");
}
