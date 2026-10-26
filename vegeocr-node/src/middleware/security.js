import cors from "cors";
import helmet from "helmet";

export function securityMiddleware(cfg) {
  const allowed = new Set(cfg.corsOrigins);
  return [
    helmet(),
    cors({
      // Same-origin and non-browser clients send no Origin header and are unaffected; browsers from other origins get no CORS headers.
      origin: (origin, cb) => cb(null, !origin || allowed.has(origin)),
      methods: ["GET", "HEAD", "POST", "OPTIONS"],
      exposedHeaders: ["ETag", "X-Request-Id", "X-Cache", "X-Cache-Variant", "Retry-After", "RateLimit", "RateLimit-Policy"],
      maxAge: 600,
    }),
  ];
}
