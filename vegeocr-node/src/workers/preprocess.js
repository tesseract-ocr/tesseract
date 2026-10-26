import sharp from "sharp";

/**
 * Normalises an image for OCR: EXIF auto-rotate, grayscale, scale so text is ~targetDpi, cap the largest side,
 * and drop all metadata (sharp strips EXIF/ICC unless asked to keep it).
 *
 * Scale rule: if the file declares its resolution (DPI) and it is below target, scale up by target/dpi (max 4x).
 * If it declares nothing, only small images (< 1200 px on the long side) are doubled; larger ones are assumed to
 * already have usable text size. `rendered: true` (PDF pages we rasterised at target DPI ourselves) skips scaling.
 */
export async function preprocess(input, { maxDimension, targetDpi, rendered = false }) {
  const meta = await sharp(input, { limitInputPixels: 268_402_689 }).metadata();
  const swap = meta.orientation && meta.orientation >= 5; // EXIF orientations 5-8 are rotated by 90°
  const w0 = swap ? meta.height : meta.width;
  const h0 = swap ? meta.width : meta.height;

  let scale = 1;
  if (!rendered) {
    if (meta.density && meta.density > 1) scale = meta.density < targetDpi ? Math.min(targetDpi / meta.density, 4) : 1;
    else if (Math.max(w0, h0) < 1200) scale = 2;
  }
  let w = Math.round(w0 * scale);
  let h = Math.round(h0 * scale);
  const over = Math.max(w, h) / maxDimension;
  if (over > 1) { w = Math.round(w / over); h = Math.round(h / over); }

  const { data, info } = await sharp(input, { limitInputPixels: 268_402_689 })
    .rotate() // applies EXIF orientation, then clears the tag
    .grayscale()
    .toColourspace("b-w") // a real single-channel image: a third of the bytes of a gray-looking RGB PNG
    .resize(w, h, { fit: "fill", kernel: scale >= 1 ? "lanczos3" : "lanczos2" })
    .png({ compressionLevel: 1 }) // the buffer only goes to tesseract's stdin: favour speed over size
    .toBuffer({ resolveWithObject: true });
  return { buffer: data, width: info.width, height: info.height, scale };
}
