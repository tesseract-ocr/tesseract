// Copies public-src/ -> public/, content-hashes JS/CSS filenames (app.js -> app.3f2a9c1b.js), rewrites references in the
// HTML, then pre-compresses everything. Hashed names are what make `Cache-Control: immutable` safe.
import { createHash } from "node:crypto";
import { cp, mkdir, readFile, readdir, rename, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { precompress } from "./precompress.js";

const src = path.resolve("public-src"), out = path.resolve(process.argv[2] ?? "public");
await rm(out, { recursive: true, force: true });
await mkdir(out, { recursive: true });
await cp(src, out, { recursive: true });

const renames = new Map();
for (const f of await readdir(out)) {
  if (!/\.(js|css)$/.test(f)) continue;
  const hash = createHash("sha256").update(await readFile(path.join(out, f))).digest("hex").slice(0, 8);
  const hashed = f.replace(/\.(js|css)$/, `.${hash}.$1`);
  await rename(path.join(out, f), path.join(out, hashed));
  renames.set(f, hashed);
}
for (const f of (await readdir(out)).filter((n) => n.endsWith(".html"))) {
  let html = await readFile(path.join(out, f), "utf8");
  for (const [from, to] of renames) html = html.replaceAll(from, to);
  await writeFile(path.join(out, f), html);
}
await precompress(out);
console.log("built", out, [...renames.values()].join(", "));
