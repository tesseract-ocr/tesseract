"""image -> quality report -> (enhance) -> engine -> lines with confidence and a "needs review" flag.

Pure functions (no database, no files): the service layer stores the images and results. Every failure the user can cause is a LabError
with a stable code, so the API can answer 4xx instead of 500.
"""
from __future__ import annotations

import io
import os
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import cv2
import numpy as np
from PIL import Image, ImageOps

from . import enhance as E
from . import quality as Q
from .engines import ENGINES, EngineError, Line

LOW_CONFIDENCE = float(os.environ.get("LAB_LOW_CONFIDENCE", 70))   # lines below this (0-100) are flagged for review
MAX_PIXELS = int(os.environ.get("LAB_MAX_PIXELS", 40_000_000))
MAX_SIDE = int(os.environ.get("LAB_MAX_SIDE", 5000))
MAX_PAGES = int(os.environ.get("LAB_MAX_PAGES", 3))
PDF_DPI = 200
MODES = ("printed", "handwriting")
ENHANCE_MODES = ("auto", "on", "off")


class LabError(Exception):
    def __init__(self, status: int, code: str, message: str):
        super().__init__(message)
        self.status, self.code, self.message = status, code, message


# ---- loading --------------------------------------------------------------------------------------------------

def _to_bgr(im: Image.Image) -> np.ndarray:
    im = ImageOps.exif_transpose(im).convert("RGB")  # phone photos: honour the camera's rotation flag
    return cv2.cvtColor(np.array(im), cv2.COLOR_RGB2BGR)


def load_pages(data: bytes, mime: str) -> Tuple[List[np.ndarray], List[str]]:
    """Decode an upload (already type-checked by magic bytes) into BGR pages. Returns (pages, warnings)."""
    warnings: List[str] = []
    try:
        if mime == "application/pdf":
            import fitz
            with fitz.open(stream=data, filetype="pdf") as doc:
                if doc.needs_pass:
                    raise LabError(422, "encrypted_pdf", "Password-protected PDFs are not supported")
                total = doc.page_count
                if total == 0:
                    raise LabError(422, "empty_pdf", "The PDF has no pages")
                if total > MAX_PAGES:
                    warnings.append(f"Only the first {MAX_PAGES} of {total} pages were processed")
                pages = []
                for page in list(doc)[:MAX_PAGES]:
                    pm = page.get_pixmap(dpi=PDF_DPI, colorspace=fitz.csRGB)
                    pages.append(cv2.cvtColor(np.frombuffer(pm.samples, np.uint8).reshape(pm.height, pm.width, 3), cv2.COLOR_RGB2BGR))
        else:
            with Image.open(io.BytesIO(data)) as im:
                if im.width * im.height > MAX_PIXELS:
                    raise LabError(413, "image_too_large", f"Image is larger than {MAX_PIXELS // 1_000_000} megapixels")
                pages = [_to_bgr(im)]
    except LabError:
        raise
    except Exception as e:  # noqa: BLE001 - any decoder failure means the file isn't a readable image
        raise LabError(422, "unreadable_image", f"The file could not be read as an image or PDF ({type(e).__name__})") from e
    out = []
    for p in pages:
        if max(p.shape[:2]) > MAX_SIDE:
            s = MAX_SIDE / max(p.shape[:2])
            p = cv2.resize(p, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
            warnings.append(f"A large page was reduced to {MAX_SIDE}px on its longest side")
        out.append(p)
    return out, warnings


# ---- engine choice --------------------------------------------------------------------------------------------

def choose_engine(mode: str, lang: str, requested: str = "auto"):
    """Returns (engine, warnings). Handwriting prefers TrOCR (English) and degrades loudly to Tesseract when it can't."""
    if requested not in ("auto", *ENGINES):
        raise LabError(422, "invalid_engine", f"engine must be auto, {', '.join(ENGINES)}")
    tesseract, trocr = ENGINES["tesseract"], ENGINES["trocr"]
    if requested == "trocr":
        reason = trocr.unavailable_reason()
        if reason:
            raise LabError(422, "engine_unavailable", reason)
        if set(lang.split("+")) != {"eng"}:
            raise LabError(422, "engine_language", "The TrOCR handwriting model reads English only; pick language 'eng' or use the Tesseract engine")
        return trocr, []
    if requested == "tesseract" or mode == "printed":
        return tesseract, []
    # handwriting + auto
    reason = trocr.unavailable_reason()
    if reason is None and set(lang.split("+")) == {"eng"}:
        return trocr, []
    why = reason or "the TrOCR handwriting model reads English only"
    return tesseract, [f"Handwriting engine not used ({why}). Falling back to Tesseract, which is much less accurate on handwriting."]


# ---- one page -------------------------------------------------------------------------------------------------

def weighted_confidence(lines: List[Line]) -> float:
    """Mean word confidence weighted by word length (long words say more about the reading than punctuation does)."""
    ws = [w for l in lines for w in (l.words or [{"text": l.text, "confidence": l.confidence}])]
    total = sum(len(w["text"]) for w in ws)
    return sum(w["confidence"] * len(w["text"]) for w in ws) / total if total else 0.0


@dataclass
class PageOutcome:
    lines: List[Line]
    before: Q.QualityReport
    after: Optional[Q.QualityReport]
    enhanced: Optional[np.ndarray]          # grey image the engine read when enhancement ran
    used: np.ndarray                        # the grey image the engine actually read (bboxes refer to this one)
    steps: List[str] = field(default_factory=list)
    reasons: List[str] = field(default_factory=list)
    scale: float = 1.0
    check: Optional[dict] = None            # original-vs-enhanced comparison, when it was run


def process_page(bgr: np.ndarray, *, mode: str, lang: str, enhance_mode: str, engine, tessdata: Optional[Path] = None) -> PageOutcome:
    gray = Q.to_gray(bgr)
    before = Q.analyse(gray)
    do_enhance = enhance_mode == "on" or (enhance_mode == "auto" and before.needs_enhancement)
    enhanced = after = None
    steps: List[str] = []
    scale = 1.0
    used = gray
    if do_enhance:
        res = E.enhance(gray, before)
        if res.steps:
            enhanced, used, steps, scale = res.image, res.image, res.steps, res.scale
            after = Q.analyse(enhanced)
    psm = 6 if mode == "handwriting" else 3

    def read(img):
        try:
            return engine.recognise(img, lang, psm=psm, tessdata=tessdata)
        except EngineError as e:
            raise LabError(502, "extraction_failed", str(e)) from e

    lines, check = read(used), None
    if enhanced is not None and enhance_mode == "auto" and getattr(engine, "cheap", False):
        # Enhancement helps most images a lot, but a fixed recipe can make a readable image worse. Reading both is cheap with Tesseract,
        # so keep whichever the engine is more confident about, and say so.
        original_lines = read(gray)
        c_enh, c_org = weighted_confidence(lines), weighted_confidence(original_lines)
        check = {"original_confidence": round(c_org, 1), "enhanced_confidence": round(c_enh, 1), "kept": "enhanced" if c_enh >= c_org else "original"}
        if c_org > c_enh:
            lines, used, enhanced, after, steps, scale = original_lines, gray, None, None, [], 1.0
    return PageOutcome(lines, before, after, enhanced, used, steps, before.reasons if do_enhance else [], scale, check)


def page_result(n: int, out: PageOutcome, threshold: float = LOW_CONFIDENCE) -> dict:
    flagged = [l for l in out.lines if l.confidence < threshold]
    confs = [l.confidence for l in out.lines]
    return {
        "page": n, "width": int(out.used.shape[1]), "height": int(out.used.shape[0]),
        "enhanced": out.enhanced is not None, "enhance_check": out.check, "enhance_steps": out.steps, "enhance_reasons": out.reasons, "scale": round(out.scale, 2),
        "quality_before": out.before.as_dict(), "quality_after": out.after.as_dict() if out.after else None,
        "lines": [{"text": l.text, "confidence": l.confidence, "bbox": l.bbox, "flagged": l.confidence < threshold, "words": l.words} for l in out.lines],
        "text": "\n".join(l.text for l in out.lines),
        "avg_confidence": round(sum(confs) / len(confs), 1) if confs else None,
        "flagged_count": len(flagged),
    }
