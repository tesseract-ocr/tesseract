# Sketch Sheet Prototype (v0.1)

This folder contains a standalone Python prototype for:

- generating sketch-sheet PDFs (`generate`)
- ingesting a photographed/scanned sheet into segmented cell images (`ingest`)

## Install

Use Python 3.10+.

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r tools/sketch_sheet/requirements.txt
```

## Generate a PDF

```bash
python3 -m tools.sketch_sheet.cli generate \
  --rows 3 --cols 3 \
  --page-size letter \
  --orientation landscape \
  --auto-date
```

Options:

- `--title "..."` pre-fills title
- `--ref "@SS26Palette @MidsoleCollection"` pre-fills ref field
- `--date YYYY-MM-DD` explicitly set date
- `--no-cell-numbers` hides numeric labels
- `--output path/to/file.pdf` sets output path

## Ingest an image/PDF

```bash
python3 -m tools.sketch_sheet.cli ingest \
  path/to/photo.jpg \
  --rows 3 --cols 3 \
  --output-root .
```

Output is a folder named:

`sketch_sheet_<YYYY-MM-DD>_<short-id>/`

With:

- `cells/01.png ...`
- `metadata.json`
- `original.<ext>`
- `debug/warped_page.png`, `debug/grid_region.png`, `debug/title_region.png`

## Notes

- OCR for title/ref/date is intentionally stubbed in `ingest.py` (`_placeholder_ocr_title_block`).
- Wire your preferred vision API there to extract `title`, `refs`, and `date` with confidence.
- Current border detection expects the full outer border to be visible (v0 constraint).
