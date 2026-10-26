// Generates a dense text page (many words => a large JSON result) for demos and the benchmark.
//   node bench/make-sample.js [out.png] [lines]
import { mkdir } from "node:fs/promises";
import path from "node:path";
import sharp from "sharp";

const out = process.argv[2] ?? "bench/sample-page.png";
const lines = Number(process.argv[3] ?? 70);
const words = "invoice order total amount payment customer delivery warehouse tomato cabbage carrot weight price tax account number date reference quantity supplier receipt balance".split(" ");
let seed = 42;
const rnd = () => (seed = (seed * 1664525 + 1013904223) % 4294967296) / 4294967296;
const row = (i) => Array.from({ length: 9 }, () => words[Math.floor(rnd() * words.length)]).join(" ") + ` ${1000 + i * 7}`;
const svg = `<svg xmlns="http://www.w3.org/2000/svg" width="1700" height="${60 + lines * 36}"><rect width="100%" height="100%" fill="white"/>
${Array.from({ length: lines }, (_, i) => `<text x="40" y="${60 + i * 36}" font-family="Helvetica, Arial, sans-serif" font-size="26" fill="black">${row(i)}</text>`).join("\n")}</svg>`;
await mkdir(path.dirname(out), { recursive: true });
await sharp(Buffer.from(svg)).png().toFile(out);
console.log(`wrote ${out} (${lines} lines)`);
