// Node 18+ example:  VEGEOCR_URL=http://localhost:8000 VEGEOCR_KEY=vocr_xxx node client.mjs invoice.pdf
import { readFile } from "node:fs/promises";
import { basename } from "node:path";

const BASE = process.env.VEGEOCR_URL ?? "http://localhost:8000";
const KEY = process.env.VEGEOCR_KEY;

async function call(path, init = {}) {
  const res = await fetch(BASE + path, { ...init, headers: { "X-API-Key": KEY, ...init.headers } });
  const body = await res.json();
  if (!res.ok) throw new Error(`${res.status} ${body.error?.code}: ${body.error?.message}`);
  return body;
}

export async function ocr(file, lang = "jpn+eng") {
  const form = new FormData();
  form.append("file", new Blob([await readFile(file)]), basename(file));
  form.append("lang", lang);
  let job = await call("/api/v1/ocr", { method: "POST", body: form });
  while (job.status === "queued" || job.status === "processing") {
    await new Promise((r) => setTimeout(r, 2000));
    job = await call(`/api/v1/jobs/${job.id}`);
  }
  if (job.status === "failed") throw new Error(job.error);
  return job;
}

if (process.argv[2]) {
  const job = await ocr(process.argv[2]);
  console.log(`confidence: ${job.confidence}\n${job.text}`);
}
