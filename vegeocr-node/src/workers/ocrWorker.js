// Runs inside a Piscina worker thread: all CPU-heavy work (PDF rasterising, image preprocessing) stays off the event loop.
import { readFile } from "node:fs/promises";
import * as mupdf from "mupdf";
import { OcrInputError } from "../errors.js";
import { preprocess } from "./preprocess.js";
import { parseTsv, runTesseract } from "./tesseract.js";

const SCHEMA_VERSION = 1;

export default async function runOcr({ filePath, mime, lang, port, opts }) {
  const started = performance.now();
  const progress = (page, total) => port?.postMessage({ type: "progress", page, total });
  const pages = [];
  const ocrPage = async (input, rendered) => {
    let pre;
    try {
      pre = await preprocess(input, { maxDimension: opts.maxDimension, targetDpi: opts.targetDpi, rendered });
    } catch {
      throw new Error("OCR_INPUT:corrupt_image:The image could not be decoded"); // any decode failure is the caller's file, not a server fault
    }
    const tsv = await runTesseract(pre.buffer, { bin: opts.tesseractBin, lang, tessdataPrefix: opts.tessdataPrefix, timeoutMs: opts.pageTimeoutMs });
    return { width: pre.width, height: pre.height, ...parseTsv(tsv) };
  };

  try {
    const data = await readFile(filePath);
    if (mime === "application/pdf") {
      let doc;
      try { doc = mupdf.Document.openDocument(data, "application/pdf"); } catch { throw new OcrInputError("corrupt_pdf", "The PDF could not be read"); }
      try {
        if (doc.needsPassword()) throw new OcrInputError("encrypted_pdf", "Password-protected PDFs are not supported");
        const total = doc.countPages();
        if (total > opts.maxPdfPages) throw new OcrInputError("too_many_pages", `PDF has ${total} pages; the limit is ${opts.maxPdfPages}`);
        const scale = opts.targetDpi / 72;
        progress(0, total);
        for (let i = 0; i < total; i++) {
          const page = doc.loadPage(i);
          const pixmap = page.toPixmap(mupdf.Matrix.scale(scale, scale), mupdf.ColorSpace.DeviceGray, false, true);
          const png = Buffer.from(pixmap.asPNG());
          pixmap.destroy?.();
          pages.push({ page: i + 1, ...(await ocrPage(png, true)) });
          progress(i + 1, total);
        }
      } finally {
        doc.destroy?.();
      }
    } else {
      progress(0, 1);
      pages.push({ page: 1, ...(await ocrPage(data, false)) });
      progress(1, 1);
    }
  } catch (err) {
    // Errors cross the thread boundary by structured clone, which keeps only name/message/stack and drops custom
    // properties like `code`. So input errors are tagged in the message and re-hydrated by workers/pool.js.
    if (err instanceof OcrInputError) throw new Error(`OCR_INPUT:${err.code}:${err.message}`);
    if (/Input buffer|unsupported image format|corrupt|VipsJpeg|bad seek|Invalid|pngload|premature end/i.test(err.message)) {
      throw new Error("OCR_INPUT:corrupt_image:The image could not be decoded");
    }
    throw err;
  }

  const all = pages.flatMap((p) => p.lines.flatMap((l) => l.words));
  return {
    status: "done",
    schema: SCHEMA_VERSION,
    lang,
    pageCount: pages.length,
    text: pages.map((p) => p.text).join("\n\n").trim(),
    confidence: all.length ? Math.round((all.reduce((s, w) => s + w.confidence, 0) / all.length) * 10) / 10 : null,
    pages,
    meta: { engine: "tesseract", ocrMs: Math.round(performance.now() - started) },
  };
}
