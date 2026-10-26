from __future__ import annotations

import re
import subprocess
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path

import fitz
from PIL import Image

from ..config import settings
from .modelstore import RUNTIME

_CJK = r"　-〿぀-ヿ㐀-䶿一-鿿＀-￯"
_CJK_GAP = re.compile(rf"(?<=[{_CJK}])[ \t]+(?=[{_CJK}])")
MIN_TEXT_LAYER = 20


def clean_text(t: str) -> str:
    return _CJK_GAP.sub("", t).strip()


def run_tesseract(image_path: str, lang: str, psm: int = 3, tessdata: Path = None, config: str = None):
    cmd = [settings.tesseract_bin, image_path, "-", "-l", lang, "--psm", str(psm),
           "--tessdata-dir", str(tessdata or RUNTIME)]
    if config:
        cmd.append(config)
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
    if r.returncode != 0:
        raise RuntimeError(r.stderr.strip() or "tesseract failed")
    return r.stdout


def ocr_lines(image_path: str, lang: str, psm: int = 3, tessdata: Path = None):
    """OCR one image and return [{text, conf, bbox:[l,t,r,b]}], one entry per detected text line."""
    tsv = run_tesseract(image_path, lang, psm, tessdata, config="tsv")
    words = defaultdict(list)
    boxes = {}
    for row in tsv.splitlines()[1:]:
        c = row.split("\t")
        if len(c) < 12:
            continue
        level, key = int(c[0]), tuple(c[1:5])
        left, top, w, h, conf, text = int(c[6]), int(c[7]), int(c[8]), int(c[9]), float(c[10]), c[11]
        if level == 4:
            boxes[key] = [left, top, left + w, top + h]
        elif level == 5 and text.strip() and conf >= 0:
            words[key].append((text, conf))
    lines = []
    for key in sorted(words, key=lambda k: tuple(int(x) for x in k)):
        txt = clean_text(" ".join(w for w, _ in words[key]))
        if txt:
            confs = [c for _, c in words[key]]
            lines.append({"text": txt, "conf": round(sum(confs) / len(confs), 1), "bbox": boxes.get(key, [0, 0, 0, 0])})
    return lines


def process_file(file_path: str, mime: str, lang: str, out_dir: Path):
    """Run OCR over an image or PDF. Returns (result_json, full_text, avg_confidence, page_count)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    pages = []
    if mime == "application/pdf":
        with fitz.open(file_path) as doc:
            for i, page in enumerate(doc, 1):
                text = page.get_text().strip()
                if len(text) >= MIN_TEXT_LAYER:  # digital PDF: trust the text layer, no OCR needed
                    lines = []
                    for blk in page.get_text("dict")["blocks"]:
                        for ln in blk.get("lines", []):
                            t = "".join(s["text"] for s in ln["spans"]).strip()
                            if t:
                                lines.append({"text": t, "conf": 100.0, "bbox": [round(v) for v in ln["bbox"]]})
                    pages.append({"page": i, "image": None, "source": "text-layer", "lines": lines})
                else:
                    img = out_dir / f"page_{i}.png"
                    page.get_pixmap(dpi=settings.pdf_dpi).save(str(img))
                    pages.append({"page": i, "image": img.name, "source": "ocr", "lines": ocr_lines(str(img), lang)})
    else:
        with Image.open(file_path) as im:
            png = out_dir / "page_1.png"
            im.convert("RGB").save(png)  # normalised copy used for line crops
        pages.append({"page": 1, "image": png.name, "source": "ocr", "lines": ocr_lines(str(png), lang)})
    confs = [ln["conf"] for p in pages for ln in p["lines"]]
    text = "\n\n".join("\n".join(ln["text"] for ln in p["lines"]) for p in pages).strip()
    return {"pages": pages}, text, (round(sum(confs) / len(confs), 1) if confs else None), len(pages)


def run_job(db, job):
    """Execute an OcrJob (shared by the inline path and the Celery task)."""
    job.status = "processing"
    db.commit()
    try:
        out = settings.storage_dir / "uploads" / job.id
        result, text, conf, n = process_file(job.file_path, job.mime, job.lang, out)
        job.result, job.text, job.avg_confidence, job.page_count = result, text, conf, n
        job.status, job.error = "done", None
    except Exception as e:  # noqa: BLE001 - record any failure on the job
        job.status, job.error = "failed", str(e)[:1000]
    job.finished_at = datetime.now(timezone.utc)
    db.commit()
    return job
