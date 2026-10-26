import { Redis } from "ioredis";
import { buildApp } from "./app.js";
import { loadConfig } from "./config.js";
import { createCache } from "./lib/cache.js";
import { createCodecs } from "./lib/codecs.js";
import { createLogger } from "./logger.js";
import { createMetrics } from "./metrics.js";
import { createQueue } from "./queue/index.js";
import { createOcrPool } from "./workers/pool.js";

const cfg = loadConfig();
const logger = createLogger(cfg);
const metrics = createMetrics();
const state = { draining: false };

const redis = cfg.redisUrl ? new Redis(cfg.redisUrl, { maxRetriesPerRequest: 2, enableReadyCheck: true }) : null;
redis?.on("error", (err) => logger.error({ err: err.message }, "redis error"));

const codecs = await createCodecs(cfg.compression);
const cache = createCache({ redis, ttlSeconds: cfg.cache.ttlSeconds, lruMaxBytes: cfg.cache.lruMaxBytes, logger });
const pool = createOcrPool(cfg.ocr, cfg.upload);
const runOcr = (task) => pool.run(task);
const queue = await createQueue({ cfg, redis, runOcr, cache, metrics, logger });
metrics.setQueueSource(async () => ({ ...(await queue.counts()), pool_waiting: pool.queued() }));

const app = buildApp({ cfg, logger, metrics, cache, codecs, queue, runOcr, redis, state, pool });
const server = app.listen(cfg.port, cfg.host, () =>
  logger.info({ port: cfg.port, cache: cache.kind, queue: queue.kind, zstd: codecs.zstdImpl, encodings: codecs.available, poolSize: pool.size }, "listening"));

// Must exceed the reverse proxy's idle timeout (Nginx keepalive_timeout 65s, ALB 60s) or the proxy
// will occasionally reuse a socket Node has just closed and return 502.
server.keepAliveTimeout = cfg.timeouts.keepAliveMs;
server.headersTimeout = cfg.timeouts.headersMs; // must be > keepAliveTimeout
server.requestTimeout = cfg.timeouts.requestMs;

let shuttingDown = false;
async function shutdown(signal) {
  if (shuttingDown) return;
  shuttingDown = true;
  logger.info({ signal }, "shutting down: no new requests, finishing in-flight OCR");
  state.draining = true; // /health now answers 503 so the load balancer stops routing here
  const force = setTimeout(() => { logger.error("shutdown timed out; exiting"); process.exit(1); }, cfg.timeouts.shutdownMs);
  force.unref();
  try {
    app.closeSse();
    await new Promise((resolve) => { server.close(resolve); server.closeIdleConnections?.(); }); // stop accepting; wait for in-flight requests
    await queue.close(); // waits for active jobs
    await pool.close(); // waits for running OCR tasks
    await cache.close();
    await redis?.quit();
    logger.info("clean shutdown");
    process.exit(0);
  } catch (err) {
    logger.error({ err: err.message }, "error during shutdown");
    process.exit(1);
  }
}
for (const s of ["SIGTERM", "SIGINT"]) process.on(s, () => shutdown(s));
process.on("unhandledRejection", (err) => logger.error({ err: String(err) }, "unhandled rejection"));
