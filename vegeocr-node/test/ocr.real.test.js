import assert from "node:assert/strict";
import { accessSync, constants } from "node:fs";
import { after, before, describe, test } from "node:test";
import sharp from "sharp";
import { loadConfig } from "../src/config.js";
import { preprocess } from "../src/workers/preprocess.js";
import { parseTsv } from "../src/workers/tesseract.js";
import { createOcrPool } from "../src/workers/pool.js";
import { createTestApp, json } from "./helpers.js";

const bin = process.env.TESSERACT_BIN || "tesseract";
const haveTesseract = (() => { try { accessSync(bin, constants.X_OK); return true; } catch { return false; } })();
const textImage = (text, { width = 900, height = 160, size = 70 } = {}) =>
  sharp(Buffer.from(`<svg xmlns="http://www.w3.org/2000/svg" width="${width}" height="${height}"><rect width="100%" height="100%" fill="white"/><text x="30" y="${height * 0.65}" font-family="Helvetica, Arial, sans-serif" font-size="${size}" fill="black">${text}</text></svg>`)).png().toBuffer();

describe("image preprocessing (sharp)", () => {
  const opts = { maxDimension: 6000, targetDpi: 300 };
  const raw = (w, h, extra = {}) => sharp({ create: { width: w, height: h, channels: 3, background: "#ccc" } }).withMetadata(extra).png().toBuffer();

  test("EXIF orientation is applied (portrait stored as landscape ends up upright)", async () => {
    const img = await sharp({ create: { width: 100, height: 50, channels: 3, background: "#fff" } }).jpeg().withMetadata({ orientation: 6 }).toBuffer();
    const r = await preprocess(img, opts);
    assert.ok(r.height > r.width, `${r.width}x${r.height}`);
  });
  test("output is grayscale and carries no metadata", async () => {
    const img = await sharp({ create: { width: 300, height: 300, channels: 3, background: "#f00" } }).jpeg().withExif({ IFD0: { Copyright: "secret" } }).toBuffer();
    assert.ok((await sharp(img).metadata()).exif, "fixture has EXIF");
    const out = await sharp((await preprocess(img, opts)).buffer).metadata();
    assert.equal(out.space, "b-w");
    assert.equal(out.exif, undefined);
    assert.equal(out.icc, undefined);
  });
  test("low declared DPI is scaled up toward 300 DPI (max 4x)", async () => {
    const r = await preprocess(await raw(1000, 500, { density: 150 }), opts);
    assert.equal(r.scale, 2);
    assert.equal(r.width, 2000);
    assert.equal((await preprocess(await raw(400, 400, { density: 72 }), opts)).scale, 4);
  });
  test("300 DPI and above are left alone", async () => assert.equal((await preprocess(await raw(1500, 900, { density: 300 }), opts)).scale, 1));
  test("no DPI metadata: small images are doubled, large ones are not", async () => {
    assert.equal((await preprocess(await raw(600, 400), opts)).scale, 2);
    assert.equal((await preprocess(await raw(2400, 1600), opts)).scale, 1);
  });
  test("the longest side is capped at maxDimension, keeping the aspect ratio", async () => {
    const r = await preprocess(await raw(3000, 1500, { density: 72 }), { maxDimension: 2000, targetDpi: 300 });
    assert.equal(Math.max(r.width, r.height), 2000);
    assert.ok(Math.abs(r.width / r.height - 2) < 0.01);
  });
  test("pages rasterised from PDFs at target DPI are not rescaled", async () => assert.equal((await preprocess(await raw(500, 700), { ...opts, rendered: true })).scale, 1));
  test("garbage input is rejected", async () => assert.rejects(preprocess(Buffer.from("nope"), opts)));
});

describe("TSV parsing", () => {
  const tsv = ["level\tpage_num\tblock_num\tpar_num\tline_num\tword_num\tleft\ttop\twidth\theight\tconf\ttext",
    "4\t1\t1\t1\t1\t0\t10\t10\t200\t30\t-1\t", "5\t1\t1\t1\t1\t1\t10\t10\t90\t30\t96.5\tHello", "5\t1\t1\t1\t1\t2\t110\t10\t100\t30\t90\tworld",
    "5\t1\t1\t1\t2\t1\t10\t50\t80\t30\t-1\t ", "5\t1\t1\t1\t2\t2\t10\t50\t80\t30\t80\t日本", "5\t1\t1\t1\t2\t3\t95\t50\t80\t30\t82\t語"].join("\n");
  test("groups words into lines with boxes and confidence; joins CJK without spaces", () => {
    const r = parseTsv(tsv);
    assert.equal(r.lines.length, 2);
    assert.equal(r.lines[0].text, "Hello world");
    assert.deepEqual(r.lines[0].bbox, [10, 10, 210, 40]);
    assert.equal(r.lines[0].confidence, 93.3);
    assert.equal(r.lines[1].text, "日本語");
    assert.equal(r.lines[1].words.length, 2, "blank / negative-confidence rows are dropped");
    assert.equal(r.text, "Hello world\n日本語");
  });
});

describe("real OCR through the worker pool", { skip: haveTesseract ? false : `tesseract not found (set TESSERACT_BIN)` }, () => {
  let pool, t;
  before(async () => {
    const cfg = loadConfig({ ...process.env, OCR_POOL_SIZE: "2" });
    pool = createOcrPool(cfg.ocr, cfg.upload);
    t = await createTestApp({ runOcr: (task) => pool.run(task), env: { TESSERACT_BIN: bin, TESSDATA_PREFIX: process.env.TESSDATA_PREFIX ?? "" } });
  });
  after(async () => { await t?.close(); await pool?.destroy(); });

  test("sync request returns text, per-word boxes and confidences", async () => {
    const r = await t.post({ file: await textImage("HELLO WORLD 12345"), fields: { lang: "eng" } });
    assert.equal(r.status, 200, r.body.toString());
    const body = json(r);
    assert.match(body.text.replace(/\s+/g, " "), /HELLO WORLD 12345/i);
    const word = body.pages[0].lines[0].words[0];
    assert.equal(word.bbox.length, 4);
    assert.ok(word.confidence > 50);
    assert.ok(body.meta.ocrMs > 0 && body.pages[0].width > 0);
  });
  test("a rotated JPEG (EXIF orientation 6) is read upright", async () => {
    const upright = await textImage("ROTATED PAGE TEXT", { width: 1000, height: 160 });
    const stored = await sharp(upright).rotate(270).jpeg().withMetadata({ orientation: 6 }).toBuffer(); // pixels stored rotated 90° CCW; EXIF orientation 6 says "rotate 90° CW to display"
    const r = await t.post({ file: stored, filename: "r.jpg", contentType: "image/jpeg", fields: { lang: "eng" } });
    assert.equal(r.status, 200);
    assert.match(json(r).text.toUpperCase(), /ROTATED/);
  });
  test("a blank image yields an empty result, not an error", async () => {
    const blank = await sharp({ create: { width: 400, height: 200, channels: 3, background: "#fff" } }).png().toBuffer();
    const r = await t.post({ file: blank });
    assert.equal(r.status, 200);
    assert.equal(json(r).text, "");
  });
  test("a truncated PNG is a 422, not a 500", async () => {
    const full = await textImage("TRUNCATED");
    const r = await t.post({ file: full.subarray(0, Math.floor(full.length / 2)), query: "?mode=sync" });
    assert.ok([415, 422].includes(r.status), `${r.status} ${r.body}`);
  });
});
