from __future__ import annotations

import json

import fitz

from ..models import OcrJob


def final_text(job: OcrJob) -> str:
    return job.edited_text if job.edited_text is not None else job.text


def to_json(job: OcrJob) -> str:
    return json.dumps({
        "id": job.id, "filename": job.filename, "lang": job.lang, "pages": job.page_count,
        "confidence": job.avg_confidence, "text": final_text(job),
        "lines": [{"page": p["page"], "text": ln["text"], "confidence": ln["conf"], "bbox": ln["bbox"]}
                  for p in (job.result or {}).get("pages", []) for ln in p["lines"]],
    }, ensure_ascii=False, indent=2)


def to_pdf(job: OcrJob) -> bytes:
    """A simple text PDF (with an embedded CJK-capable font) of the extracted text."""
    doc = fitz.open()
    font = fitz.Font("cjk")
    rect = fitz.Rect(54, 54, 541, 788)
    remaining = final_text(job) or " "
    while True:
        page = doc.new_page()
        page.insert_font(fontname="cjk", fontbuffer=font.buffer)
        spare = page.insert_textbox(rect, remaining, fontname="cjk", fontsize=11, lineheight=1.4)
        if spare >= 0:
            break
        # Too long for one page: binary-search how much fits, then continue on the next page.
        lo, hi = 0, len(remaining)
        while lo < hi:
            mid = (lo + hi + 1) // 2
            probe = doc.new_page()
            probe.insert_font(fontname="cjk", fontbuffer=font.buffer)
            ok = probe.insert_textbox(rect, remaining[:mid], fontname="cjk", fontsize=11, lineheight=1.4) >= 0
            doc.delete_page(-1)
            lo, hi = (mid, hi) if ok else (lo, mid - 1)
        doc.delete_page(-1)
        page = doc.new_page()
        page.insert_font(fontname="cjk", fontbuffer=font.buffer)
        page.insert_textbox(rect, remaining[:lo], fontname="cjk", fontsize=11, lineheight=1.4)
        remaining = remaining[lo:]
        if not remaining:
            break
    out = doc.tobytes()
    doc.close()
    return out
