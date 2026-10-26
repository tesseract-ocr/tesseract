import client from "prom-client";

/** Prometheus metrics. Pass `queue` to getQueueDepth lazily so the gauge reads the live value at scrape time. */
export function createMetrics({ defaults = true } = {}) {
  const registry = new client.Registry();
  if (defaults) client.collectDefaultMetrics({ register: registry });
  const m = {
    registry,
    httpDuration: new client.Histogram({
      name: "http_request_duration_seconds", help: "HTTP request latency", labelNames: ["method", "route", "status"],
      buckets: [0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 2.5, 5, 10, 30, 60, 120], registers: [registry],
    }),
    ocrDuration: new client.Histogram({
      name: "ocr_job_duration_seconds", help: "OCR execution time", labelNames: ["mode", "status"],
      buckets: [0.1, 0.25, 0.5, 1, 2, 5, 10, 20, 30, 60, 120], registers: [registry],
    }),
    cacheRequests: new client.Counter({ name: "ocr_cache_requests_total", help: "OCR cache lookups", labelNames: ["result"], registers: [registry] }),
    compressionRatio: new client.Histogram({
      name: "http_compression_ratio", help: "compressed/original size per encoding (lower is better)", labelNames: ["encoding"],
      buckets: [0.02, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 0.75, 1], registers: [registry],
    }),
    compressionSeconds: new client.Histogram({
      name: "http_compression_seconds", help: "time spent compressing a response", labelNames: ["encoding"],
      buckets: [0.0005, 0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1], registers: [registry],
    }),
    responseBytes: new client.Counter({ name: "http_response_bytes_total", help: "bytes sent, by stage", labelNames: ["stage"], registers: [registry] }),
    queueDepth: new client.Gauge({ name: "ocr_queue_depth", help: "OCR jobs by state", labelNames: ["state"], registers: [registry] }),
    activeSse: new client.Gauge({ name: "sse_connections", help: "open SSE streams", registers: [registry] }),
    observeCompression(encoding, original, compressed, seconds) {
      m.compressionRatio.observe({ encoding }, compressed / original);
      m.compressionSeconds.observe({ encoding }, seconds);
      m.responseBytes.inc({ stage: "original" }, original);
      m.responseBytes.inc({ stage: "sent" }, compressed);
    },
    setQueueSource(fn) {
      m.queueDepth.collect = async function collect() {
        try {
          const c = await fn();
          for (const [state, n] of Object.entries(c)) this.set({ state }, n);
        } catch { /* queue unavailable: leave last value */ }
      };
    },
  };
  return m;
}
