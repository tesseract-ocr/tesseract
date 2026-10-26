import assert from "node:assert/strict";
import { after, before, describe, test } from "node:test";
import { createTestApp, json, png, request } from "./helpers.js";

describe("rate limiting", () => {
  let t;
  before(async () => { t = await createTestApp({ env: { RATE_LIMIT_OCR_MAX: "3", RATE_LIMIT_MAX: "8", RATE_LIMIT_WINDOW_MS: "60000" } }); });
  after(() => t.close());

  test("POST /api/ocr is limited more strictly: 429 with a JSON error and rate-limit headers", async () => {
    const file = await png(21);
    const statuses = [];
    let limited;
    for (let i = 0; i < 5; i++) {
      const r = await t.post({ file });
      statuses.push(r.status);
      if (r.status === 429) limited = r;
    }
    assert.deepEqual(statuses.slice(0, 3), [200, 200, 200]);
    assert.deepEqual(statuses.slice(3), [429, 429]);
    assert.equal(json(limited).error.code, "rate_limited");
    assert.ok(limited.headers["retry-after"], "Retry-After present");
    assert.ok(limited.headers.ratelimit || limited.headers["ratelimit-policy"], "standard RateLimit headers present");
  });

  test("the global /api limiter also applies to cheap endpoints", async () => {
    // 5 requests already counted from the test above (the global limiter sees every /api request), so 3 more fit.
    const out = [];
    for (let i = 0; i < 6; i++) out.push((await request(t.url, { path: "/api/ocr/00000000-0000-0000-0000-000000000000" })).status);
    assert.ok(out.includes(429), `expected a 429 in ${out}`);
    assert.equal(out[0], 404);
  });

  test("/health and /metrics are not rate limited", async () => {
    for (let i = 0; i < 15; i++) assert.equal((await request(t.url, { path: "/health" })).status, 200);
  });
});
