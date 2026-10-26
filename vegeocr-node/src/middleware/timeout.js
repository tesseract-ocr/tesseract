/** Answers 503 if a handler hasn't finished in `ms`, so a stuck OCR call can't hold a connection forever. SSE streams are exempt. */
export function requestTimeout(ms) {
  return function timeout(req, res, next) {
    if (req.path.endsWith("/events")) return next();
    const timer = setTimeout(() => {
      if (!res.headersSent) res.status(503).set("Retry-After", "5").json({ error: { code: "request_timeout", message: "The request took too long" } });
    }, ms);
    timer.unref();
    res.on("close", () => clearTimeout(timer));
    next();
  };
}
