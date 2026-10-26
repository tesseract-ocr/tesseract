import zlib from "node:zlib";
import { promisify } from "node:util";

const gzipAsync = promisify(zlib.gzip);
const brotliAsync = promisify(zlib.brotliCompress);

/** Order of server preference when the client rates encodings equally. */
export const ENCODING_ORDER = ["zstd", "br", "gzip", "identity"];

async function loadZstd(impl) {
  const hasNative = typeof zlib.zstdCompress === "function"; // Node >= 22.15 / 23.8
  if ((impl === "auto" || impl === "native") && hasNative) {
    const zstdAsync = promisify(zlib.zstdCompress);
    return { name: "native", compress: (buf, level) => zstdAsync(buf, { params: { [zlib.constants.ZSTD_c_compressionLevel]: level } }) };
  }
  if (impl !== "native") {
    try {
      const mod = await import("@skhaz/zstd");
      return { name: "@skhaz/zstd", compress: async (buf, level) => Buffer.from(await mod.compress(buf, { level })) };
    } catch {
      /* not installed or no prebuilt binary for this platform */
    }
  }
  return null;
}

/**
 * Builds the compressor registry. `encode(name, buffer, level?)` is always async: it runs on libuv's threadpool
 * (zlib/brotli/zstd all do), never blocking the event loop.
 */
export async function createCodecs(compressionCfg) {
  const { levels, zstdImpl } = compressionCfg;
  const zstd = await loadZstd(zstdImpl);
  const impls = {
    gzip: (buf, level) => gzipAsync(buf, { level }),
    br: (buf, level) => brotliAsync(buf, { params: { [zlib.constants.BROTLI_PARAM_QUALITY]: level, [zlib.constants.BROTLI_PARAM_SIZE_HINT]: buf.length } }),
  };
  if (zstd) impls.zstd = zstd.compress;
  return {
    available: ENCODING_ORDER.filter((e) => e === "identity" || impls[e]),
    zstdImpl: zstd?.name ?? null,
    levels,
    encode(name, buf, level = levels[name]) {
      if (name === "identity") return Promise.resolve(buf);
      const fn = impls[name];
      if (!fn) return Promise.reject(new Error(`unsupported encoding ${name}`));
      return fn(buf, level);
    },
  };
}
