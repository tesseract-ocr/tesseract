import { createBullQueue } from "./bullQueue.js";
import { createMemoryQueue } from "./memoryQueue.js";
import { createProcessor } from "./processor.js";

export async function createQueue({ cfg, redis, runOcr, cache, metrics, logger }) {
  const processor = createProcessor({ runOcr, cache, metrics, logger });
  if (cfg.redisUrl) {
    const q = createBullQueue({ redisUrl: cfg.redisUrl, redis, processor, queueCfg: cfg.queue, logger });
    await q.ready();
    return q;
  }
  logger.warn("REDIS_URL not set: using the in-memory queue (jobs are lost on restart)");
  return createMemoryQueue({ processor, concurrency: cfg.queue.concurrency, jobTtlSeconds: cfg.queue.jobTtlSeconds });
}
