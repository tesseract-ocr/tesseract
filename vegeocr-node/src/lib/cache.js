import { LRUCache } from "lru-cache";

/**
 * OCR result cache. Stores the JSON body AND each compressed variant under separate keys, so a repeat request skips
 * both the OCR and the compression. Backed by Redis when available, otherwise a size-bounded in-memory LRU.
 * Cache failures never fail a request: they degrade to a miss.
 */
export function createCache({ redis, ttlSeconds, lruMaxBytes, logger }) {
  const prefix = "ocr:v1:";
  const k = (key, enc) => `${prefix}${key}${enc ? `:${enc}` : ""}`;

  if (redis) {
    const safe = async (fn, fallback) => {
      try { return await fn(); } catch (err) { logger?.warn({ err: err.message }, "cache redis error"); return fallback; }
    };
    return {
      kind: "redis",
      get: (key, enc) => safe(() => redis.getBuffer(k(key, enc)), null),
      set: (key, buf, enc) => safe(() => redis.set(k(key, enc), buf, "EX", ttlSeconds), null),
      has: (key) => safe(async () => (await redis.exists(k(key))) === 1, false),
      close: async () => {},
    };
  }

  const lru = new LRUCache({ maxSize: lruMaxBytes, sizeCalculation: (b) => b.length + 64, ttl: ttlSeconds * 1000, allowStale: false });
  return {
    kind: "memory",
    get: async (key, enc) => lru.get(k(key, enc)) ?? null,
    set: async (key, buf, enc) => { lru.set(k(key, enc), buf); },
    has: async (key) => lru.has(k(key)),
    stats: () => ({ entries: lru.size, bytes: lru.calculatedSize }),
    close: async () => lru.clear(),
  };
}
