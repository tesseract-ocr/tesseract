// Benchmark against a RUNNING server.
//   node bench/make-sample.js                       # once: dense text page (~700 words -> ~65 KB JSON)
//   node bench/benchmark.js [--url http://localhost:4000] [--file bench/sample-page.png] [--runs 30] [--lang eng]
//
// 1. Wire size + latency per Accept-Encoding on the cached-result path (variant already cached: what steady state looks like)
// 2. Compression CPU cost per codec/level, measured in-process on the real result JSON (the trade-off behind the defaults)
// 3. Cold (OCR) vs cached request latency
// 4. Event-loop responsiveness: /health latency while a cold OCR is running (proves OCR is off the main thread)
import { randomBytes } from "node:crypto";
import { readFile } from "node:fs/promises";
import http from "node:http";
import zlib from "node:zlib";
import { createCodecs } from "../src/lib/codecs.js";

const args = Object.fromEntries(process.argv.slice(2).reduce((a, v, i, all) => (v.startsWith("--") ? [...a, [v.slice(2), all[i + 1]]] : a), []));
const URL_ = args.url ?? "http://localhost:4000";
const FILE = args.file ?? "bench/sample-page.png";
const RUNS = Number(args.runs ?? 30);
const LANG = args.lang ?? "eng";

const median = (xs) => [...xs].sort((a, b) => a - b)[Math.floor(xs.length / 2)];
const pct = (xs, p) => [...xs].sort((a, b) => a - b)[Math.min(xs.length - 1, Math.floor((p / 100) * xs.length))];
const ms = (n) => `${n.toFixed(1)} ms`;
const kb = (n) => `${(n / 1024).toFixed(1)} KB`;
const table = (rows, head) => {
  const all = [head, ...rows].map((r) => r.map(String));
  const w = head.map((_, i) => Math.max(...all.map((r) => r[i].length)));
  const line = (r) => "| " + r.map((c, i) => (i === 0 ? c.padEnd(w[i]) : c.padStart(w[i]))).join(" | ") + " |";
  console.log([line(all[0]), "|" + w.map((x, i) => (i === 0 ? ":" : "") + "-".repeat(x + (i === 0 ? 1 : 1)) + (i === 0 ? "" : ":")).join("|") + "|", ...all.slice(1).map(line)].join("\n") + "\n");
};

function post(buf, encoding) {
  const b = "----bench" + randomBytes(8).toString("hex");
  const body = Buffer.concat([
    Buffer.from(`--${b}\r\nContent-Disposition: form-data; name="lang"\r\n\r\n${LANG}\r\n--${b}\r\nContent-Disposition: form-data; name="file"; filename="p.png"\r\nContent-Type: image/png\r\n\r\n`),
    buf, Buffer.from(`\r\n--${b}--\r\n`)]);
  return new Promise((resolve, reject) => {
    const t0 = performance.now();
    const req = http.request(new globalThis.URL("/api/ocr?mode=sync", URL_), { method: "POST", headers: { "content-type": `multipart/form-data; boundary=${b}`, "accept-encoding": encoding } }, (res) => {
      const chunks = [];
      res.on("data", (c) => chunks.push(c));
      res.on("end", () => resolve({ status: res.statusCode, wire: Buffer.concat(chunks), headers: res.headers, ms: performance.now() - t0 }));
    });
    req.on("error", reject);
    req.end(body);
  });
}
const get = (path) => new Promise((resolve, reject) => {
  const t0 = performance.now();
  http.get(new globalThis.URL(path, URL_), (res) => { res.resume(); res.on("end", () => resolve(performance.now() - t0)); }).on("error", reject);
});
const decode = { identity: (b) => b, gzip: zlib.gunzipSync, br: zlib.brotliDecompressSync, zstd: (b) => zlib.zstdDecompressSync(b) };
// A different file each time (trailing bytes after PNG's IEND are ignored by decoders) => different SHA-256 => guaranteed cache miss.
const unique = (png) => Buffer.concat([png, randomBytes(16)]);

const png = await readFile(FILE);
const health = await get("/health").catch(() => null);
if (health === null) { console.error(`No server at ${URL_}. Start it first: npm start`); process.exit(1); }
console.log(`Server ${URL_} · file ${FILE} (${kb(png.length)}) · ${RUNS} runs\n`);

// ---- 1. wire size & latency per encoding (cache warm) -----------------------------------------------------------------
console.log("## 1. Response size and latency per Accept-Encoding (cached result, compressed variant already cached)\n");
const warm = unique(png);
const first = await post(warm, "identity");
if (first.status !== 200) { console.error("OCR failed:", first.status, first.wire.toString()); process.exit(1); }
const originalBytes = first.wire.length;
const rows = [];
for (const enc of ["identity", "gzip", "br", "zstd"]) {
  await post(warm, enc); // populate the variant cache; not measured
  const wire = [], total = [], dec = [];
  for (let i = 0; i < RUNS; i++) {
    const r = await post(warm, enc);
    const t = performance.now(); decode[r.headers["content-encoding"] ?? "identity"](r.wire); const d = performance.now() - t;
    wire.push(r.wire.length); total.push(r.ms); dec.push(d);
  }
  rows.push([enc, kb(median(wire)), `${((100 * median(wire)) / originalBytes).toFixed(1)}%`, ms(median(total)), ms(pct(total, 95)), ms(median(dec))]);
}
table(rows, ["encoding", "bytes on wire", "of original", "median latency", "p95 latency", "client decode"]);
console.log(`(original JSON: ${kb(originalBytes)}; latency here is loopback, so it mostly shows server work, not network time. On a real link, fewer bytes dominate.)\n`);

// ---- 2. CPU cost per codec and level -----------------------------------------------------------------------------------
console.log("## 2. Compression cost per level (in-process, on the real result JSON)\n");
const body = first.wire;
const codecs = await createCodecs({ levels: { zstd: 3, br: 5, gzip: 6 }, zstdImpl: "auto" });
const plan = [["gzip", [1, 6, 9]], ["br", [4, 5, 9, 11]], ["zstd", [1, 3, 9, 19]]];
const costRows = [];
for (const [enc, levels] of plan) {
  if (!codecs.available.includes(enc)) continue;
  for (const level of levels) {
    const times = []; let out;
    for (let i = 0; i < 7; i++) { const t = performance.now(); out = await codecs.encode(enc, body, level); times.push(performance.now() - t); }
    const dt = []; for (let i = 0; i < 7; i++) { const t = performance.now(); decode[enc](out); dt.push(performance.now() - t); }
    costRows.push([`${enc} ${level}`, kb(out.length), `${((100 * out.length) / body.length).toFixed(1)}%`, ms(median(times)), ms(median(dt)), (body.length / 1048576 / (median(times) / 1000)).toFixed(0) + " MB/s"]);
  }
}
table(costRows, ["codec level", "size", "of original", "compress", "decompress", "throughput"]);

// ---- 3. cold vs cached OCR ----------------------------------------------------------------------------------------------
console.log("## 3. Cold OCR vs cached\n");
const N = Math.min(RUNS, 5);
const cold = [], variantMiss = [], variantHit = [];
for (let i = 0; i < N; i++) {
  const f = unique(png);
  cold.push((await post(f, "br")).ms);
  variantMiss.push((await post(f, "zstd")).ms);
  await post(f, "zstd");
  variantHit.push((await post(f, "zstd")).ms);
}
table([
  ["cold: OCR + compress + store", ms(median(cold)), "1x"],
  ["cached body, new encoding (compress once)", ms(median(variantMiss)), `${(median(cold) / median(variantMiss)).toFixed(0)}x faster`],
  ["cached body + cached variant", ms(median(variantHit)), `${(median(cold) / median(variantHit)).toFixed(0)}x faster`],
], ["request", `median of ${N}`, "vs cold"]);

// ---- 4. event-loop responsiveness while OCR runs ---------------------------------------------------------------------------
console.log("## 4. /health latency while a cold OCR is running (event loop stays free?)\n");
const idle = []; for (let i = 0; i < 20; i++) idle.push(await get("/health"));
const during = []; let done = false;
const ocr = post(unique(png), "br").then(() => { done = true; });
while (!done) { during.push(await get("/health")); await new Promise((r) => setTimeout(r, 20)); }
await ocr;
table([
  ["idle", idle.length, ms(median(idle)), ms(pct(idle, 99))],
  ["during OCR", during.length, ms(median(during)), ms(pct(during, 99))],
], ["/health", "samples", "median", "p99"]);
