import assert from "node:assert/strict";
import { randomUUID } from "node:crypto";
import http from "node:http";
import { after, before, describe, test } from "node:test";
import { Redis } from "ioredis";
import pino from "pino";
import { createBullQueue } from "../src/queue/bullQueue.js";
import { createTestApp, fakeOcr, json, pdfBytes, request, sleep, waitFor } from "./helpers.js";

const REDIS_URL = process.env.TEST_REDIS_URL ?? "redis://localhost:6379/15";
const redisUp = await new Promise((resolve) => {
  const r = new Redis(REDIS_URL, { lazyConnect: true, retryStrategy: () => null, maxRetriesPerRequest: 0 });
  r.on("error", () => {});
  r.connect().then(() => r.ping()).then(() => resolve(true), () => resolve(false)).finally(() => r.disconnect());
});

/** Reads an SSE stream until the server closes it; returns [{event, data}]. */
function sse(url, path) {
  return new Promise((resolve, reject) => {
    const events = [];
    http.get(new URL(path, url), { headers: { accept: "text/event-stream" } }, (res) => {
      let buf = "";
      res.setEncoding("utf8");
      res.on("data", (c) => {
        buf += c;
        let i;
        while ((i = buf.indexOf("\n\n")) >= 0) {
          const block = buf.slice(0, i); buf = buf.slice(i + 2);
          const ev = /^event: (.*)$/m.exec(block)?.[1]; const data = /^data: (.*)$/m.exec(block)?.[1];
          if (ev) events.push({ event: ev, data: JSON.parse(data) });
        }
      });
      res.on("end", () => resolve({ events, headers: res.headers }));
    }).on("error", reject);
    setTimeout(() => reject(new Error("sse timeout")), 8000).unref();
  });
}

function suite(name, makeQueue) {
  describe(`job queue flow (${name})`, () => {
    let t, ocr;
    const pdf = (seed) => pdfBytes(seed);
    const post = (seed, extra = {}) => t.post({ file: pdf(seed), filename: "d.pdf", contentType: "application/pdf", ...extra });

    before(async () => { ocr = fakeOcr({ delay: 80 }); t = await createTestApp({ runOcr: ocr, queue: makeQueue }); });
    after(() => t.close());

    test("PDF -> 202 + job id + Location; polling reaches the finished result", async () => {
      const r = await post(1);
      assert.equal(r.status, 202);
      const { id, status, links } = json(r);
      assert.equal(status, "queued");
      assert.equal(r.headers.location, `/api/ocr/${id}`);
      assert.equal(links.events, `/api/ocr/${id}/events`);
      const done = await waitFor(async () => { const g = await request(t.url, { path: `/api/ocr/${id}` }); return g.status === 200 && json(g).status === "done" ? g : null; });
      assert.equal(json(done).pageCount, 1);
      assert.ok(json(done).pages[0].lines.length > 0);
    });

    test("in-progress job reports status, then the result carries an ETag and supports 304", async () => {
      const id = json(await post(2)).id;
      const early = json(await request(t.url, { path: `/api/ocr/${id}` }));
      assert.ok(["queued", "processing"].includes(early.status), early.status);
      const done = await waitFor(async () => { const g = await request(t.url, { path: `/api/ocr/${id}` }); return g.headers.etag ? g : null; });
      const again = await request(t.url, { path: `/api/ocr/${id}`, headers: { "if-none-match": done.headers.etag } });
      assert.equal(again.status, 304);
    });

    test("?mode=async forces the queue for a small image; ?mode=sync forces inline", async () => {
      const { png } = await import("./helpers.js");
      assert.equal((await t.post({ file: await png(71), query: "?mode=async" })).status, 202);
      assert.equal((await t.post({ file: await png(72), query: "?mode=sync" })).status, 200);
    });

    test("SSE streams status, per-page progress, then done, and ends the stream", async () => {
      const id = json(await post(3)).id;
      const { events, headers } = await sse(t.url, `/api/ocr/${id}/events`);
      assert.match(headers["content-type"], /text\/event-stream/);
      assert.equal(headers["content-encoding"], undefined, "event streams are never compressed");
      const names = events.map((e) => e.event);
      assert.equal(names[0], "status");
      assert.ok(names.includes("progress"), names.join(","));
      assert.equal(names.at(-1), "done");
      assert.equal(events.at(-1).data.result, `/api/ocr/${id}`);
    });

    test("shutdown: open SSE streams get a `shutdown` event and are closed cleanly", async () => {
      const slow = await createTestApp({ runOcr: fakeOcr({ delay: 300 }), queue: makeQueue });
      try {
        const id = json(await slow.post({ file: pdf(77), filename: "s.pdf", contentType: "application/pdf" })).id;
        const stream = sse(slow.url, `/api/ocr/${id}/events`);
        await sleep(150);
        slow.app.closeSse();
        const names = (await stream).events.map((e) => e.event);
        assert.equal(names.at(-1), "shutdown", names.join());
      } finally { await slow.close(); }
    });

    test("SSE on an already-finished job answers immediately with done", async () => {
      const id = json(await post(4)).id;
      await waitFor(async () => json(await request(t.url, { path: `/api/ocr/${id}` })).status === "done");
      const { events } = await sse(t.url, `/api/ocr/${id}/events`);
      assert.deepEqual(events.map((e) => e.event), ["status", "done"]);
    });

    test("identical uploads in flight share one job and one OCR run", async () => {
      const before = ocr.calls.length;
      const [a, b, c] = await Promise.all([post(5), post(5), post(5)]);
      const ids = [a, b, c].map((r) => json(r).id);
      assert.equal(new Set(ids).size, 1, ids.join());
      assert.ok([a, b, c].some((r) => json(r).deduplicated === true));
      await waitFor(async () => json(await request(t.url, { path: `/api/ocr/${ids[0]}` })).status === "done");
      assert.equal(ocr.calls.length, before + 1);
      assert.deepEqual(t.leftoverUploads(), [], "deduplicated uploads are cleaned up");
    });

    test("unknown / malformed job ids -> 404", async () => {
      assert.equal((await request(t.url, { path: `/api/ocr/${randomUUID()}` })).status, 404);
      assert.equal((await request(t.url, { path: "/api/ocr/not-a-uuid" })).status, 404);
      assert.equal((await request(t.url, { path: `/api/ocr/${randomUUID()}/events` })).status, 404);
    });

    test("expired result -> 410", async () => {
      const id = json(await post(6)).id;
      await waitFor(async () => json(await request(t.url, { path: `/api/ocr/${id}` })).status === "done");
      const info = await t.queue.get(id);
      await t.cache.close(); // memory cache: wipes entries
      const r = await request(t.url, { path: `/api/ocr/${id}` });
      assert.equal(r.status, 410);
      assert.equal(json(r).error.code, "result_expired");
      assert.ok(info.key);
    });

    test("queue depth is observable", async () => {
      const c = await t.queue.counts();
      assert.ok("waiting" in c && "active" in c);
    });
  });

  describe(`job failures (${name})`, () => {
    let t;
    before(async () => { t = await createTestApp({ runOcr: fakeOcr({ fail: Object.assign(new Error("bad input"), { code: "too_many_pages", input: true }) }), queue: makeQueue }); });
    after(() => t.close());
    test("a failed job reports status=failed with a clean error and the upload is removed", async () => {
      const id = json(await t.post({ file: pdfBytes(90), filename: "f.pdf", contentType: "application/pdf" })).id;
      const s = await waitFor(async () => { const j = json(await request(t.url, { path: `/api/ocr/${id}` })); return j.status === "failed" ? j : null; });
      assert.equal(s.error.code, "too_many_pages");
      assert.equal(JSON.stringify(s).includes("/"), false, "no file paths leak");
      await waitFor(() => t.leftoverUploads().length === 0);
    });
  });
}

suite("in-memory", undefined);

if (redisUp) {
  const name = `test-${randomUUID().slice(0, 8)}`;
  suite("BullMQ + Redis", async ({ processor, cfg }) => {
    const redis = new Redis(REDIS_URL, { maxRetriesPerRequest: 2 });
    const q = createBullQueue({ redisUrl: REDIS_URL, redis, processor, queueCfg: { ...cfg.queue, name: `${name}-${randomUUID().slice(0, 4)}`, concurrency: 2, attempts: 1 }, logger: pino({ level: "silent" }) });
    await q.ready();
    const close = q.close.bind(q);
    q.close = async () => { await close(); await redis.quit(); };
    return q;
  });
} else {
  test("BullMQ + Redis queue flow", { skip: `Redis not reachable at ${REDIS_URL} (set TEST_REDIS_URL)` }, () => {});
}
