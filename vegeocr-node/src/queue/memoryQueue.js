import { randomUUID } from "node:crypto";
import { EventEmitter } from "node:events";
import { publicError } from "./processor.js";

/**
 * In-process queue with the same interface as the BullMQ one. Used when REDIS_URL is not set (dev, tests, single
 * instance). Jobs are lost on restart, which is why production should use Redis.
 */
export function createMemoryQueue({ processor, concurrency, jobTtlSeconds }) {
  const jobs = new Map();
  const waiting = [];
  const inflight = new Map(); // cache key -> job id, so identical uploads share one OCR run
  const bus = new EventEmitter().setMaxListeners(0);
  let active = 0;
  let closed = false;

  const snapshot = (j) => ({
    id: j.id, state: j.state, progress: j.progress, key: j.key, error: j.error,
  });
  const emit = (j, type, extra = {}) => bus.emit(j.id, { type, ...extra });

  async function pump() {
    while (!closed && active < concurrency && waiting.length) {
      const job = waiting.shift();
      active++;
      job.state = "active";
      processor(job.data, { onProgress: (p) => { job.progress = { page: p.page, total: p.total }; emit(job, "progress", job.progress); } })
        .then((out) => { job.state = "completed"; job.key = out.key; emit(job, "completed", { key: out.key }); })
        .catch((err) => { job.state = "failed"; job.error = publicError(err); emit(job, "failed", { error: job.error }); })
        .finally(() => {
          active--;
          inflight.delete(job.data.key);
          job.finishedAt = Date.now();
          pump();
        });
    }
  }

  const sweep = setInterval(() => {
    const cutoff = Date.now() - jobTtlSeconds * 1000;
    for (const [id, j] of jobs) if (j.finishedAt && j.finishedAt < cutoff) jobs.delete(id);
  }, 60_000);
  sweep.unref();

  return {
    kind: "memory",
    /** Returns { id, deduped } - deduped means an identical job is already running and the caller's file can be dropped. */
    async enqueue(data) {
      const existing = inflight.get(data.key);
      if (existing && jobs.get(existing)?.state !== "failed") return { id: existing, deduped: true };
      const job = { id: randomUUID(), data, state: "waiting", progress: null, createdAt: Date.now() };
      jobs.set(job.id, job);
      inflight.set(data.key, job.id);
      waiting.push(job);
      queueMicrotask(pump);
      return { id: job.id, deduped: false };
    },
    async get(id) { const j = jobs.get(id); return j ? snapshot(j) : null; },
    subscribe(id, listener) { bus.on(id, listener); return () => bus.off(id, listener); },
    async counts() { return { waiting: waiting.length, active, completed: [...jobs.values()].filter((j) => j.state === "completed").length, failed: [...jobs.values()].filter((j) => j.state === "failed").length }; },
    async close() {
      closed = true;
      clearInterval(sweep);
      while (active > 0) await new Promise((r) => setTimeout(r, 50)); // let in-flight OCR finish
    },
  };
}
