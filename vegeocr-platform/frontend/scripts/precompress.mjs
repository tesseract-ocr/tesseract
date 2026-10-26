// Build step: write .zst / .br / .gz next to every compressible file at MAXIMUM levels (zstd 19, brotli 11, gzip 9).
// Slow on purpose: it runs once while the image is built, so requests cost no CPU.
//   node scripts/precompress.mjs <dir> [<dir> ...]        (e.g. .next/static)
// zstd needs Node >= 22.15 (built into node:zlib). On older Node the .zst files are skipped with a warning.
import { readdir, readFile, rm, stat, writeFile } from "node:fs/promises";
import path from "node:path";
import { promisify } from "node:util";
import zlib from "node:zlib";

const LEVELS = { zstd: 19, br: 11, gzip: 9 };
const MIN_BYTES = 256;
const COMPRESSIBLE = new Set([".js", ".mjs", ".css", ".html", ".svg", ".json", ".txt", ".xml", ".map", ".webmanifest", ".wasm"]);
// Fonts, images, video, audio, archives and PDFs are already compressed (or don't benefit): never touched.

const gzip = promisify(zlib.gzip);
const brotli = promisify(zlib.brotliCompress);
const zstd = typeof zlib.zstdCompress === "function" ? promisify(zlib.zstdCompress) : null;
if (!zstd) console.warn(`! Node ${process.version} has no built-in zstd (needs >= 22.15): skipping .zst files`);

const encoders = [
  ["gzip", ".gz", (b) => gzip(b, { level: LEVELS.gzip })],
  ["br", ".br", (b) => brotli(b, { params: { [zlib.constants.BROTLI_PARAM_QUALITY]: LEVELS.br, [zlib.constants.BROTLI_PARAM_SIZE_HINT]: b.length } })],
  ...(zstd ? [["zstd", ".zst", (b) => zstd(b, { params: { [zlib.constants.ZSTD_c_compressionLevel]: LEVELS.zstd } })]] : []),
];

async function* walk(dir) {
  for (const e of await readdir(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) yield* walk(p);
    else yield p;
  }
}

let files = 0, original = 0;
const packed = { gzip: 0, br: 0, zstd: 0 };
for (const dir of process.argv.slice(2)) {
  for await (const file of walk(dir)) {
    if (!COMPRESSIBLE.has(path.extname(file))) continue;
    const { size } = await stat(file);
    if (size < MIN_BYTES) continue;
    const buf = await readFile(file);
    files++; original += size;
    for (const [name, ext, encode] of encoders) {
      const out = await encode(buf);
      if (out.length < size) { await writeFile(file + ext, out); packed[name] += out.length; }
      else { await rm(file + ext, { force: true }); packed[name] += size; } // keep nothing that doesn't help
    }
  }
}
const pct = (n) => `${((100 * n) / original).toFixed(1)}%`;
console.log(`precompressed ${files} files, ${(original / 1024).toFixed(0)} KB -> gzip-9 ${pct(packed.gzip)}, br-11 ${pct(packed.br)}${zstd ? `, zstd-19 ${pct(packed.zstd)}` : ""}`);
