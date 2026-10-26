import { MessageChannel } from "node:worker_threads";
import { Piscina } from "piscina";

/**
 * Wraps a Piscina pool. `run({filePath, mime, lang, onProgress, signal})` resolves with the OCR result.
 * A MessagePort is handed to the worker so it can report page progress while the task is running.
 */
export function createOcrPool(ocrCfg, uploadCfg) {
  const pool = new Piscina({
    filename: new URL("./ocrWorker.js", import.meta.url).href,
    minThreads: 1,
    maxThreads: ocrCfg.poolSize,
    idleTimeout: 60_000,
    // Bounded queue: if every thread is busy and 4x more tasks are waiting, fail fast instead of growing memory.
    maxQueue: ocrCfg.poolSize * 4,
  });
  const opts = {
    tesseractBin: ocrCfg.tesseractBin, tessdataPrefix: ocrCfg.tessdataPrefix, maxDimension: ocrCfg.maxDimension,
    targetDpi: ocrCfg.targetDpi, maxPdfPages: uploadCfg.maxPdfPages,
    pageTimeoutMs: Math.max(5_000, Math.floor(ocrCfg.timeoutMs * 0.8)),
  };
  return {
    size: ocrCfg.poolSize,
    stats: () => ({ queued: pool.queueSize, threads: pool.threads.length, completed: pool.completed }),
    queued: () => pool.queueSize,
    async run({ filePath, mime, lang, onProgress, signal }) {
      const { port1, port2 } = new MessageChannel();
      port1.on("message", (m) => m?.type === "progress" && onProgress?.(m));
      port1.unref();
      try {
        return await pool.run({ filePath, mime, lang, port: port2, opts }, { transferList: [port2], signal: signal ?? AbortSignal.timeout(ocrCfg.timeoutMs) });
      } catch (err) {
        const m = /^OCR_INPUT:([a-z_]+):([\s\S]*)$/.exec(err?.message ?? "");
        if (m) throw Object.assign(new Error(m[2]), { code: m[1], input: true }); // caller's fault: 422, not 500
        throw err;
      } finally {
        port1.close();
      }
    },
    close: () => pool.close(),
    destroy: () => pool.destroy(),
  };
}
