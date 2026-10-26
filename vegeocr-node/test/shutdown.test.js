import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { accessSync, constants, mkdtempSync, rmSync } from "node:fs";
import net from "node:net";
import os from "node:os";
import path from "node:path";
import { test } from "node:test";
import sharp from "sharp";
import { multipart, request, sleep } from "./helpers.js";

const bin = process.env.TESSERACT_BIN || "tesseract";
const have = (() => { try { accessSync(bin, constants.X_OK); return true; } catch { return false; } })();
const freePort = () => new Promise((res) => { const s = net.createServer().listen(0, () => { const p = s.address().port; s.close(() => res(p)); }); });

test("graceful shutdown: SIGTERM lets the in-flight OCR finish, then exits 0", { skip: have ? false : "tesseract not found" }, async () => {
  const port = await freePort();
  const dir = mkdtempSync(path.join(os.tmpdir(), "vegeocr-sd-"));
  const child = spawn(process.execPath, ["src/server.js"], {
    env: { ...process.env, PORT: String(port), HOST: "127.0.0.1", REDIS_URL: "", UPLOAD_DIR: dir, PUBLIC_DIR: path.join(dir, "none"), OCR_POOL_SIZE: "1", LOG_LEVEL: "info" },
    stdio: ["ignore", "pipe", "pipe"],
  });
  let log = "";
  child.stdout.on("data", (d) => (log += d));
  child.stderr.on("data", (d) => (log += d));
  const exited = new Promise((r) => child.on("exit", (code, sig) => r({ code, sig })));
  try {
    for (let i = 0; i < 100 && !log.includes("listening"); i++) await sleep(50);
    assert.ok(log.includes("listening"), log);
    const lines = Array.from({ length: 18 }, (_, i) => `<text x="30" y="${50 + i * 40}" font-size="30" font-family="Helvetica">line ${i} some words to read here</text>`).join("");
    const img = await sharp(Buffer.from(`<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="800"><rect width="100%" height="100%" fill="white"/>${lines}</svg>`)).png().toBuffer();
    const mp = multipart({ file: img, fields: { lang: "eng" } });
    const inflight = request(`http://127.0.0.1:${port}`, { method: "POST", path: "/api/ocr?mode=sync", body: mp.body, headers: mp.headers });
    await sleep(400); // the request is now inside OCR
    child.kill("SIGTERM");
    const res = await inflight;
    assert.equal(res.status, 200, "in-flight request must complete");
    assert.match(JSON.parse(res.body).text, /line/);
    const { code } = await exited;
    assert.equal(code, 0, log);
    assert.match(log, /clean shutdown/);
    await assert.rejects(request(`http://127.0.0.1:${port}`, { path: "/health" }), "port is closed after shutdown");
  } finally {
    child.kill("SIGKILL");
    rmSync(dir, { recursive: true, force: true });
  }
});
