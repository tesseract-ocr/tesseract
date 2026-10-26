#!/bin/sh
# Fetch the base OCR models once (tessdata_best = float models, required for fine-tuning) into the shared volume.
set -e
T="${BASE_TESSDATA:-/data/tessdata}"
mkdir -p "$T" "${STORAGE_DIR:-/data/storage}"
for l in jpn jpn_vert chi_sim chi_tra eng osd; do
  if [ ! -s "$T/$l.traineddata" ]; then
    echo "[entrypoint] downloading $l.traineddata"
    curl -fsSL -o "$T/$l.traineddata.tmp" "https://github.com/tesseract-ocr/tessdata_best/raw/main/$l.traineddata"
    mv "$T/$l.traineddata.tmp" "$T/$l.traineddata"
  fi
done
[ -e "$T/configs" ] || ln -s /opt/tesseract/share/tessdata/configs "$T/configs"
# Optional handwriting model (only when the image was built with INSTALL_HANDWRITING=1). Cached in the shared volume, so this runs once.
if [ "${ENABLE_HANDWRITING:-0}" = "1" ]; then
  python /app/scripts/download_trocr.py || echo "[entrypoint] TrOCR download failed: handwriting will fall back to Tesseract"
fi
exec "$@"
