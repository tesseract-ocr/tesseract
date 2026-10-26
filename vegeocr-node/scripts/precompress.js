// Build step: writes .zst / .br / .gz next to every compressible asset at MAXIMUM levels (zstd 19, brotli 11).
// This is slow (seconds per MB) and that's the point: it runs once at build time so requests cost zero CPU.
//   node scripts/precompress.js [dir=public]
import { readdir, readFile, rm, stat, writeFile } from "node:fs/promises";
import path from "node:path";
import { pathToFileURL } from "node:url";
import { promisify } from "node:util";
import zlib from "node:zlib";
import { createCodecs } from "../src/lib/codecs.js";

const COMPRESSIBLE = new Set([".js", ".mjs", ".css", ".html", ".svg", ".json", ".txt", ".xml", ".map", ".wasm", ".webmanifest", ".ico"]);
const MIN_BYTES = 256;
const SUFFIX = { zstd: ".zst", br: ".br", gzip: ".gz" };

async function* walk(dir) {
  for (const e of await readdir(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) yield* walk(p);
    else yield p;
  }
}

export async function precompress(dir, { levels = { zstd: 19, br: 11, gzip: 9 }, log = console.log } = {}) {
  const codecs = await createCodecs({ levels, zstdImpl: "auto" });
  const report = [];
  for await (const file of walk(dir)) {
    const ext = path.extname(file);
    if (!COMPRESSIBLE.has(ext)) continue;
    const { size } = await stat(file);
    if (size < MIN_BYTES) continue;
    const buf = await readFile(file);
    const row = { file: path.relative(dir, file), original: size };
    for (const enc of ["zstd", "br", "gzip"]) {
      const out = path.join(dir, row.file) + SUFFIX[enc];
      if (!codecs.available.includes(enc)) continue;
      const packed = await codecs.encode(enc, buf, levels[enc]);
      if (packed.length < size) { await writeFile(out, packed); row[enc] = packed.length; } // only keep it if it helps
      else await rm(out, { force: true });
    }
    report.push(row);
  }
  for (const r of report) log(`${r.file.padEnd(32)} ${String(r.original).padStart(8)}  zst ${String(r.zstd ?? "-").padStart(7)}  br ${String(r.br ?? "-").padStart(7)}  gz ${String(r.gzip ?? "-").padStart(7)}`);
  return report;
}

if (import.meta.url === pathToFileURL(process.argv[1]).href) {
  const dir = path.resolve(process.argv[2] ?? "public");
  console.log(`precompressing ${dir}`);
  await precompress(dir);
}
