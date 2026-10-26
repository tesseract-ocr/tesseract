import { randomUUID } from "node:crypto";
import express from "express";
import pinoHttp from "pino-http";
import { dynamicCompression } from "./middleware/compression.js";
import { errorHandler, notFound } from "./middleware/errors.js";
import { createLimiters } from "./middleware/rateLimit.js";
import { securityMiddleware } from "./middleware/security.js";
import { staticPrecompressed } from "./middleware/staticPrecompressed.js";
import { requestTimeout } from "./middleware/timeout.js";
import { ocrRouter } from "./routes/ocr.js";
import { existsSync } from "node:fs";

/**
 * Builds the Express app from injected dependencies (config, cache, codecs, queue, OCR runner, ...), which is what
 * makes it testable without Redis, Tesseract or real worker threads.
 */
const finishFields = (res, val) => ({
  ...val,
  encoding: res.getHeader("content-encoding") || "identity",
  originalBytes: res.locals.originalBytes, compressedBytes: res.locals.compressedBytes,
  ocrMs: res.locals.ocrMs, cache: res.locals.cacheStatus, file: res.locals.fileHash,
});

export function buildApp({ cfg, logger, metrics, cache, codecs, queue, runOcr, redis = null, state = { draining: false }, pool = null }) {
  const app = express();
  app.disable("x-powered-by");
  app.set("etag", "weak");
  if (cfg.trustProxy) app.set("trust proxy", /^\d+$/.test(String(cfg.trustProxy)) ? Number(cfg.trustProxy) : cfg.trustProxy);

  // Request id + structured access log (encoding, original vs compressed size, OCR time, cache status).
  app.use(pinoHttp({
    logger,
    genReqId: (req, res) => {
      const given = req.headers["x-request-id"];
      const id = typeof given === "string" && /^[\w-]{8,64}$/.test(given) ? given : randomUUID();
      res.setHeader("X-Request-Id", id);
      return id;
    },
    autoLogging: { ignore: (req) => req.url === "/health" || req.url === "/metrics" },
    // Finish-time fields only (customProps would also run at request start, before the encoding is known).
    customSuccessObject: (req, res, val) => finishFields(res, val),
    customErrorObject: (req, res, err, val) => finishFields(res, val),
    serializers: { req: (r) => ({ id: r.id, method: r.method, url: r.url }), res: (r) => ({ statusCode: r.statusCode }) },
  }));
  app.use(...securityMiddleware(cfg));
  app.use((req, res, next) => {
    const end = metrics.httpDuration.startTimer();
    res.on("finish", () => end({ method: req.method, route: req.route ? `${req.baseUrl}${req.route.path}` : req.baseUrl || "unmatched", status: res.statusCode }));
    next();
  });
  app.use(dynamicCompression({ codecs, cfg, metrics, logger }));
  app.use(requestTimeout(cfg.timeouts.requestMs));

  const limiters = createLimiters({ cfg, redis });

  app.get("/health", async (_req, res) => {
    const body = { status: state.draining ? "draining" : "ok", uptime: Math.round(process.uptime()), cache: cache.kind, queue: queue.kind };
    try { body.jobs = await queue.counts(); } catch { body.status = "degraded"; }
    if (redis) body.redis = (await redis.ping().then(() => "up", () => "down"));
    if (body.redis === "down") body.status = "degraded";
    res.status(state.draining ? 503 : 200).set("Cache-Control", "no-store").json(body);
  });

  app.get("/metrics", async (req, res) => {
    if (cfg.metricsToken && req.headers.authorization !== `Bearer ${cfg.metricsToken}`) {
      return res.status(401).json({ error: { code: "unauthorized", message: "Bearer token required" } });
    }
    res.set("Content-Type", metrics.registry.contentType).set("Cache-Control", "no-store").send(await metrics.registry.metrics());
  });

  app.use("/api", limiters.global);
  const ocr = ocrRouter({ cfg, cache, codecs, queue, runOcr, metrics, logger, limiter: limiters.ocr });
  app.use("/api/ocr", ocr);
  app.closeSse = () => [...ocr.sseClosers].forEach((s) => s.close());

  if (existsSync(cfg.publicDir)) app.use(staticPrecompressed(cfg.publicDir, { available: codecs.available }));
  app.use(notFound);
  app.use(errorHandler(logger));
  return app;
}
