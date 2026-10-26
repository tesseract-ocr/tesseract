import { spawn } from "node:child_process";

const CJK = "\\u3000-\\u303f\\u3040-\\u30ff\\u3400-\\u4dbf\\u4e00-\\u9fff\\uff00-\\uffef";
const CJK_GAP = new RegExp(`(?<=[${CJK}])[ \\t]+(?=[${CJK}])`, "gu");
const joinWords = (words) => words.join(" ").replace(CJK_GAP, "").trim();

/** Runs `tesseract stdin stdout tsv` on an image buffer. One OpenMP thread per process: the worker pool is the parallelism. */
export function runTesseract(buffer, { bin, lang, tessdataPrefix, timeoutMs }) {
  return new Promise((resolve, reject) => {
    const env = { ...process.env, OMP_THREAD_LIMIT: "1" };
    if (tessdataPrefix) env.TESSDATA_PREFIX = tessdataPrefix;
    const child = spawn(bin, ["stdin", "stdout", "-l", lang, "--psm", "3", "tsv"], { env, stdio: ["pipe", "pipe", "pipe"] });
    const out = [];
    let err = "";
    let timedOut = false;
    const timer = setTimeout(() => { timedOut = true; child.kill("SIGKILL"); }, timeoutMs);
    child.stdout.on("data", (c) => out.push(c));
    child.stderr.on("data", (c) => { if (err.length < 4000) err += c; });
    child.on("error", (e) => { clearTimeout(timer); reject(e); });
    child.on("close", (code) => {
      clearTimeout(timer);
      if (timedOut) return reject(new Error(`tesseract exceeded ${timeoutMs} ms`));
      if (code !== 0) return reject(new Error(`tesseract exited ${code}: ${err.trim().slice(0, 500)}`));
      resolve(Buffer.concat(out).toString("utf8"));
    });
    child.stdin.on("error", () => {}); // EPIPE if tesseract dies early; the close handler reports it
    child.stdin.end(buffer);
  });
}

/** TSV -> { text, confidence, lines:[{text, confidence, bbox, words:[{text, confidence, bbox}]}] } */
export function parseTsv(tsv) {
  const lines = new Map();
  for (const row of tsv.split("\n").slice(1)) {
    const c = row.split("\t");
    if (c.length < 12 || c[0] !== "5") continue;
    const conf = Number.parseFloat(c[10]);
    const text = c[11].trim();
    if (!text || conf < 0) continue;
    const left = +c[6], top = +c[7], width = +c[8], height = +c[9];
    const key = `${c[2]}.${c[3]}.${c[4]}`;
    if (!lines.has(key)) lines.set(key, []);
    lines.get(key).push({ text, confidence: Math.round(conf * 10) / 10, bbox: [left, top, left + width, top + height] });
  }
  const outLines = [...lines.values()].map((words) => ({
    text: joinWords(words.map((w) => w.text)),
    confidence: Math.round((words.reduce((s, w) => s + w.confidence, 0) / words.length) * 10) / 10,
    bbox: [Math.min(...words.map((w) => w.bbox[0])), Math.min(...words.map((w) => w.bbox[1])), Math.max(...words.map((w) => w.bbox[2])), Math.max(...words.map((w) => w.bbox[3]))],
    words,
  }));
  const all = outLines.flatMap((l) => l.words);
  return {
    text: outLines.map((l) => l.text).join("\n"),
    confidence: all.length ? Math.round((all.reduce((s, w) => s + w.confidence, 0) / all.length) * 10) / 10 : null,
    lines: outLines,
  };
}
