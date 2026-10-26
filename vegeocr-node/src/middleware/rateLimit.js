import { rateLimit } from "express-rate-limit";
import { RedisStore } from "rate-limit-redis";

/** Global limiter plus a stricter one for the expensive OCR upload. Redis store when available so limits hold across instances. */
export function createLimiters({ cfg, redis }) {
  const make = (name, limit) =>
    rateLimit({
      windowMs: cfg.rateLimit.windowMs,
      limit,
      standardHeaders: "draft-7",
      legacyHeaders: false,
      ...(redis ? { store: new RedisStore({ sendCommand: (...args) => redis.call(...args), prefix: `rl:${name}:` }) } : {}),
      handler: (req, res) => {
        res.status(429).json({ error: { code: "rate_limited", message: `Too many requests. Limit is ${limit} per ${Math.round(cfg.rateLimit.windowMs / 1000)}s.` } });
      },
    });
  return { global: make("global", cfg.rateLimit.max), ocr: make("ocr", cfg.rateLimit.ocrMax) };
}
