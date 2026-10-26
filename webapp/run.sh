#!/bin/sh
# Local dev launcher. Set PGPASSWORD in your shell first (do not commit it).
cd "$(dirname "$0")"
export TESSERACT_BIN="${TESSERACT_BIN:-/private/tmp/claude-501/-Volumes-work-OCR-System-VegeOCR/984abfeb-f1d3-4735-8371-3cafdd2caec8/scratchpad/build/bin/tesseract}"
export TESSDATA_PREFIX="${TESSDATA_PREFIX:-$PWD/tessdata}"
exec .venv/bin/python app.py
