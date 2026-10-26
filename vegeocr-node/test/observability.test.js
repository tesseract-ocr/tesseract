import assert from "node:assert/strict";
import { after, before, describe, test } from "node:test";
import { createTestApp, json, png, request } from "./helpers.js";

describe("health and metrics", () => {
  let t;
  before(async () => { t = await createTestApp(); });
  after(() => t.close());

  test("/health reports status and queue counts", async () => {
    const r = await request(t.url, { path: "/health" });
    assert.equal(r.status, 200);
    assert.equal(json(r).status, "ok");
    assert.ok("jobs" in json(r));
  });
  test("X-Request-Id is generated, and a well-formed client id is echoed", async () => {
    assert.match((await request(t.url, { path: "/health" })).headers["x-request-id"], /^[0-9a-f-]{36}$/);
    assert.equal((await request(t.url, { path: "/health", headers: { "x-request-id": "client-abc-12345" } })).headers["x-request-id"], "client-abc-12345");
    assert.notEqual((await request(t.url, { path: "/health", headers: { "x-request-id": "bad id with spaces!" } })).headers["x-request-id"], "bad id with spaces!");
  });
  test("security headers are present (helmet) and x-powered-by is gone", async () => {
    const h = (await request(t.url, { path: "/health" })).headers;
    assert.ok(h["x-content-type-options"] && h["strict-transport-security"] && h["x-frame-options"] !== undefined);
    assert.equal(h["x-powered-by"], undefined);
  });
  test("CORS: allowed origin is echoed, other origins get no CORS headers", async () => {
    const ok = await request(t.url, { path: "/health", headers: { origin: "http://localhost:3000" } });
    assert.equal(ok.headers["access-control-allow-origin"], "http://localhost:3000");
    const bad = await request(t.url, { path: "/health", headers: { origin: "https://evil.example" } });
    assert.equal(bad.headers["access-control-allow-origin"], undefined);
    const pre = await request(t.url, { method: "OPTIONS", path: "/api/ocr", headers: { origin: "https://evil.example", "access-control-request-method": "POST" } });
    assert.equal(pre.headers["access-control-allow-origin"], undefined);
  });
  test("/metrics exposes latency, cache, compression and queue metrics", async () => {
    const file = await png(81);
    await t.post({ file, headers: { "accept-encoding": "br" } });
    await t.post({ file, headers: { "accept-encoding": "br" } });
    const text = (await request(t.url, { path: "/metrics" })).body.toString();
    assert.match(text, /http_request_duration_seconds_bucket\{[^}]*route="\/api\/ocr[^}]*\}/);
    assert.match(text, /ocr_cache_requests_total\{result="hit"\} 1/);
    assert.match(text, /ocr_cache_requests_total\{result="miss"\} 1/);
    assert.match(text, /http_compression_ratio_count\{encoding="br"\}/);
    assert.match(text, /ocr_job_duration_seconds_bucket/);
    assert.match(text, /ocr_queue_depth|# TYPE ocr_queue_depth gauge/);
  });
  test("unknown routes and errors are JSON, never HTML or stack traces", async () => {
    const r = await request(t.url, { path: "/nope" });
    assert.equal(r.status, 404);
    assert.equal(json(r).error.code, "not_found");
  });
});

describe("metrics endpoint protection", () => {
  let t;
  before(async () => { t = await createTestApp({ env: { METRICS_TOKEN: "s3cret" } }); });
  after(() => t.close());
  test("requires the bearer token when METRICS_TOKEN is set", async () => {
    assert.equal((await request(t.url, { path: "/metrics" })).status, 401);
    assert.equal((await request(t.url, { path: "/metrics", headers: { authorization: "Bearer wrong" } })).status, 401);
    assert.equal((await request(t.url, { path: "/metrics", headers: { authorization: "Bearer s3cret" } })).status, 200);
  });
});
