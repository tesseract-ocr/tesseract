import pino from "pino";

export function createLogger(cfg) {
  return pino({
    level: cfg.logLevel,
    base: undefined,
    redact: ["req.headers.authorization", "req.headers.cookie"],
    ...(cfg.pretty ? { transport: { target: "pino-pretty" } } : {}),
  });
}
