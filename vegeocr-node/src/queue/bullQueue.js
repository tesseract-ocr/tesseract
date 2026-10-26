import { randomUUID } from "node:crypto";
import { EventEmitter } from "node:events";
import { Queue, QueueEvents, UnrecoverableError, Worker } from "bullmq";
import { publicError } from "./processor.js";

/**
 * BullMQ-backed queue (durable, shared across instances). Each BullMQ component needs its own Redis connection with
 * maxRetriesPerRequest=null, so we hand it `connection` options (derived from REDIS_URL) rather than a shared client.
 */
export function createBullQueue({ redisUrl, redis, processor, queueCfg, logger }) {
  const url = new URL(redisUrl);
  const connection = {
    host: url.hostname, port: Number(url.port || 6379), username: url.username || undefined,
    password: url.password ? decodeURIComponent(url.password) : undefined, db: url.pathname.length > 1 ? Number(url.pathname.slice(1)) : 0,
    tls: url.protocol === "rediss:" ? {} : undefined, maxRetriesPerRequest: null,
  };
  const name = queueCfg.name;
  const queue = new Queue(name, {
    connection,
    defaultJobOptions: {
      attempts: queueCfg.attempts, backoff: { type: "exponential", delay: 2000 },
      removeOnComplete: { age: queueCfg.jobTtlSeconds }, removeOnFail: { age: queueCfg.jobTtlSeconds },
    },
  });
  const events = new QueueEvents(name, { connection });
  const bus = new EventEmitter().setMaxListeners(0);
  const inflightKey = (k) => `ocrq:${name}:inflight:${k}`;

  const worker = new Worker(name, async (job) => {
    const finalAttempt = job.attemptsMade + 1 >= (job.opts.attempts ?? 1);
    try {
      return await processor(job.data, { onProgress: (p) => job.updateProgress({ page: p.page, total: p.total }), finalAttempt });
    } catch (err) {
      if (err.input) throw new UnrecoverableError(JSON.stringify(publicError(err))); // bad input: don't retry
      throw err;
    } finally {
      if (finalAttempt) await redis?.del(inflightKey(job.data.key)).catch(() => {});
    }
  }, { connection, concurrency: queueCfg.concurrency });
  worker.on("error", (err) => logger.error({ err: err.message }, "bullmq worker error"));

  events.on("progress", ({ jobId, data }) => bus.emit(jobId, { type: "progress", ...data }));
  events.on("completed", ({ jobId, returnvalue }) => bus.emit(jobId, { type: "completed", key: returnvalue?.key }));
  events.on("failed", ({ jobId, failedReason }) => bus.emit(jobId, { type: "failed", error: parseReason(failedReason) }));

  function parseReason(reason = "") {
    try { const p = JSON.parse(reason); if (p?.code) return p; } catch { /* plain message */ }
    return publicError(new Error(reason));
  }
  const stateOf = (s) => ({ waiting: "waiting", delayed: "waiting", prioritized: "waiting", "waiting-children": "waiting", active: "active", completed: "completed", failed: "failed" }[s] ?? "waiting");

  return {
    kind: "bullmq",
    async enqueue(data) {
      const id = randomUUID();
      if (redis) {
        // Coalesce identical uploads that are already queued/running (SET NX with a safety expiry).
        const ok = await redis.set(inflightKey(data.key), id, "EX", 600, "NX");
        if (!ok) {
          const existing = await redis.get(inflightKey(data.key));
          if (existing) return { id: existing, deduped: true };
        }
      }
      await queue.add("ocr", data, { jobId: id });
      return { id, deduped: false };
    },
    async get(id) {
      const job = await queue.getJob(id);
      if (!job) return null;
      const state = stateOf(await job.getState());
      return {
        id, state, progress: job.progress && typeof job.progress === "object" ? job.progress : null,
        key: job.returnvalue?.key, error: state === "failed" ? parseReason(job.failedReason) : undefined,
      };
    },
    subscribe(id, listener) { bus.on(id, listener); return () => bus.off(id, listener); },
    async counts() {
      const c = await queue.getJobCounts("waiting", "active", "delayed", "completed", "failed");
      return { waiting: c.waiting + c.delayed, active: c.active, completed: c.completed, failed: c.failed };
    },
    async ready() { await events.waitUntilReady(); await queue.waitUntilReady(); },
    async close() {
      await worker.close(); // waits for in-flight jobs to finish
      await events.close();
      await queue.close();
    },
  };
}
