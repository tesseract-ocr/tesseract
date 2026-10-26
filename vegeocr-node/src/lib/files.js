import { createHash } from "node:crypto";
import { createReadStream } from "node:fs";
import { unlink } from "node:fs/promises";
import { fileTypeFromFile } from "file-type";
import { HttpError } from "../errors.js";

export const ALLOWED_TYPES = new Set(["image/png", "image/jpeg", "image/tiff", "image/webp", "image/gif", "application/pdf"]);

/** Streaming SHA-256 so a 20 MB upload is never held in memory just to be hashed. */
export function sha256File(path) {
  return new Promise((resolve, reject) => {
    const h = createHash("sha256");
    createReadStream(path).on("data", (c) => h.update(c)).on("error", reject).on("end", () => resolve(h.digest("hex")));
  });
}

/** Identifies the upload by its first bytes, ignoring the client-supplied filename and Content-Type. */
export async function sniffUpload(path) {
  const t = await fileTypeFromFile(path);
  if (!t || !ALLOWED_TYPES.has(t.mime)) {
    throw new HttpError(415, "unsupported_type", "Only PNG, JPEG, TIFF, WEBP, GIF images and PDF files are accepted");
  }
  return t.mime;
}

export const removeFile = (path) => unlink(path).catch(() => {});
