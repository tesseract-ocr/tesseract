from __future__ import annotations

import json
import re
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
import string
from typing import Any, Optional
from uuid import uuid4

import cv2
import numpy as np
from PIL import Image

try:
    from pdf2image import convert_from_path
except ImportError:  # pragma: no cover - optional dependency for PDF ingest
    convert_from_path = None

try:
    import fitz  # PyMuPDF
except ImportError:  # pragma: no cover - optional dependency for PDF ingest fallback
    fitz = None

try:
    from rapidocr_onnxruntime import RapidOCR
except ImportError:  # pragma: no cover - optional dependency for OCR
    RapidOCR = None


@dataclass(frozen=True)
class IngestConfig:
    adaptive_block_size: int = 25
    adaptive_c: int = 10
    border_aspect_tolerance: float = 0.35
    min_border_area_ratio: float = 0.20


def _load_image(path: Path) -> np.ndarray:
    suffix = path.suffix.lower()
    if suffix == ".pdf":
        if convert_from_path is not None:
            try:
                pages = convert_from_path(str(path), first_page=1, last_page=1)
                if pages:
                    pil_image = pages[0].convert("RGB")
                    return cv2.cvtColor(np.array(pil_image), cv2.COLOR_RGB2BGR)
            except Exception:
                # Fall through to PyMuPDF when Poppler utilities are unavailable.
                pass

        if fitz is not None:
            doc = fitz.open(str(path))
            if len(doc) < 1:
                raise RuntimeError("No pages found in PDF input")
            page = doc.load_page(0)
            pix = page.get_pixmap(dpi=200, alpha=False)
            buf = np.frombuffer(pix.samples, dtype=np.uint8).reshape(pix.height, pix.width, pix.n)
            if pix.n == 4:
                buf = cv2.cvtColor(buf, cv2.COLOR_RGBA2RGB)
            return cv2.cvtColor(buf, cv2.COLOR_RGB2BGR)

        raise RuntimeError(
            "PDF input requires either poppler (for pdf2image) or PyMuPDF (fitz) installed"
        )

    img = cv2.imread(str(path))
    if img is None:
        raise RuntimeError(f"Could not read image: {path}")
    return img


def _order_points(pts: np.ndarray) -> np.ndarray:
    rect = np.zeros((4, 2), dtype=np.float32)
    s = pts.sum(axis=1)
    diff = np.diff(pts, axis=1)
    rect[0] = pts[np.argmin(s)]  # top-left
    rect[2] = pts[np.argmax(s)]  # bottom-right
    rect[1] = pts[np.argmin(diff)]  # top-right
    rect[3] = pts[np.argmax(diff)]  # bottom-left
    return rect


def _find_largest_quad(image: np.ndarray, config: IngestConfig) -> np.ndarray:
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    edges = cv2.Canny(blur, 50, 150)

    contours, _ = cv2.findContours(edges, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    contours = sorted(contours, key=cv2.contourArea, reverse=True)

    image_area = image.shape[0] * image.shape[1]
    best: Optional[np.ndarray] = None
    for cnt in contours[:50]:
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.02 * peri, True)
        if len(approx) != 4:
            continue
        area = cv2.contourArea(approx)
        if area < image_area * config.min_border_area_ratio:
            continue
        best = approx.reshape(4, 2).astype(np.float32)
        break

    if best is None:
        raise RuntimeError("Could not detect outer border quadrilateral")
    return _order_points(best)


def _warp_to_rect(image: np.ndarray, quad: np.ndarray) -> np.ndarray:
    (tl, tr, br, bl) = quad
    width_top = np.linalg.norm(tr - tl)
    width_bottom = np.linalg.norm(br - bl)
    max_w = int(max(width_top, width_bottom))

    height_left = np.linalg.norm(bl - tl)
    height_right = np.linalg.norm(br - tr)
    max_h = int(max(height_left, height_right))

    dst = np.array(
        [
            [0, 0],
            [max_w - 1, 0],
            [max_w - 1, max_h - 1],
            [0, max_h - 1],
        ],
        dtype=np.float32,
    )
    m = cv2.getPerspectiveTransform(quad, dst)
    return cv2.warpPerspective(image, m, (max_w, max_h))


def _split_regions(warped: np.ndarray, title_block_ratio: float = 1.2 / 8.5) -> tuple[np.ndarray, np.ndarray]:
    h, w = warped.shape[:2]
    title_h = int(h * title_block_ratio)
    title_h = max(1, min(h - 1, title_h))
    title_region = warped[h - title_h : h, 0:w]
    grid_region = warped[0 : h - title_h, 0:w]
    return grid_region, title_region


def _threshold_cell(cell_bgr: np.ndarray, config: IngestConfig) -> np.ndarray:
    gray = cv2.cvtColor(cell_bgr, cv2.COLOR_BGR2GRAY)
    return cv2.adaptiveThreshold(
        gray,
        255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY,
        config.adaptive_block_size,
        config.adaptive_c,
    )


def _parse_refs(text: str) -> list[str]:
    return re.findall(r"@[A-Za-z0-9_\-]+", text)


def _clean_text(text: str) -> str:
    normalized = " ".join((text or "").replace("\n", " ").split())
    return normalized.strip()


def _sanitize_date_text(text: str) -> str:
    cleaned = _clean_text(text)
    cleaned = re.sub(r"^[^A-Za-z0-9]+", "", cleaned)
    cleaned = re.sub(r"\bDate\b[:\s]*", "", cleaned, flags=re.IGNORECASE)
    return cleaned.strip(" " + string.punctuation)


def _sanitize_title_text(text: str) -> str:
    cleaned = _clean_text(text)
    cleaned = re.sub(r"^\s*Title\b[:\s]*", "", cleaned, flags=re.IGNORECASE)
    return cleaned.strip(" " + string.punctuation)


def _sanitize_ref_text(text: str) -> str:
    cleaned = _clean_text(text)
    cleaned = re.sub(r"^\s*Ref\.?\b[:\s]*", "", cleaned, flags=re.IGNORECASE)
    return cleaned


def _ocr_crop(engine: Any, crop: np.ndarray) -> tuple[str, float]:
    gray = cv2.cvtColor(crop, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (3, 3), 0)
    th = cv2.adaptiveThreshold(
        gray,
        255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY,
        25,
        10,
    )

    variants = [
        cv2.resize(crop, None, fx=2.0, fy=2.0, interpolation=cv2.INTER_CUBIC),
        cv2.resize(gray, None, fx=2.0, fy=2.0, interpolation=cv2.INTER_CUBIC),
        cv2.resize(th, None, fx=2.0, fy=2.0, interpolation=cv2.INTER_CUBIC),
        cv2.resize(255 - th, None, fx=2.0, fy=2.0, interpolation=cv2.INTER_CUBIC),
    ]

    best_text = ""
    best_conf = 0.0
    for v in variants:
        result, _elapsed = engine(v)
        if not result:
            continue
        texts = [item[1] for item in result if len(item) >= 3]
        scores = [float(item[2]) for item in result if len(item) >= 3]
        text = _clean_text(" ".join(texts))
        conf = (sum(scores) / len(scores)) if scores else 0.0
        if conf > best_conf and text:
            best_text = text
            best_conf = conf

    return best_text, best_conf


def _ocr_title_block(title_region: np.ndarray) -> dict[str, Any]:
    if RapidOCR is None:
        return {
            "title": "",
            "refs": [],
            "date": "",
            "raw_text": "",
            "confidence": {"title": 0.0, "refs": 0.0, "date": 0.0},
            "warnings": ["rapidocr-onnxruntime not installed"],
        }

    engine = RapidOCR()
    h, w = title_region.shape[:2]

    # Match PDF geometry from generator.
    mid = int(h * 0.5)
    label_col = int(w * (0.7 / 10.2))  # 0.7" over approx 10.2" usable width.
    date_label_left = int(w * 0.7)
    date_label_right = min(w, date_label_left + label_col)

    # Slight padding to avoid border lines.
    pad = max(2, int(min(h, w) * 0.01))

    title_crop = title_region[pad : mid - pad, label_col + pad : w - pad]
    ref_crop = title_region[mid + pad : h - pad, label_col + pad : date_label_left - pad]
    date_crop = title_region[mid + pad : h - pad, date_label_right + pad : w - pad]

    title_text, title_conf = _ocr_crop(engine, title_crop)
    ref_text, ref_conf = _ocr_crop(engine, ref_crop)
    date_text, date_conf = _ocr_crop(engine, date_crop)

    refs = _parse_refs(ref_text)
    if not refs:
        # Fallback: OCR often misses '@' and reads it as Q/O.
        guessed = []
        for token in re.findall(r"[A-Za-z][A-Za-z0-9_\-]{2,}", ref_text):
            t = token.strip()
            if len(t) >= 3:
                guessed.append(f"@{t}")
        refs = guessed[:5]

    combined_raw = " | ".join([title_text, ref_text, date_text]).strip(" |")
    return {
        "title": _sanitize_title_text(title_text),
        "refs": refs,
        "date": _sanitize_date_text(date_text),
        "raw_text": combined_raw,
        "confidence": {
            "title": round(float(title_conf), 3),
            "refs": round(float(ref_conf), 3),
            "date": round(float(date_conf), 3),
        },
    }


def ingest_sheet(
    image_path: str,
    rows: int = 3,
    cols: int = 3,
    output_root: str = ".",
    archive_original: bool = True,
    write_debug: bool = True,
) -> Path:
    config = IngestConfig()
    src = Path(image_path).expanduser().resolve()
    image = _load_image(src)

    quad = _find_largest_quad(image, config)
    warped = _warp_to_rect(image, quad)
    grid_region, title_region = _split_regions(warped)

    ts = datetime.now().strftime("%Y-%m-%d")
    short_id = uuid4().hex[:6]
    out_dir = Path(output_root).expanduser().resolve() / f"sketch_sheet_{ts}_{short_id}"
    cells_dir = out_dir / "cells"
    debug_dir = out_dir / "debug"
    cells_dir.mkdir(parents=True, exist_ok=True)
    if write_debug:
        debug_dir.mkdir(parents=True, exist_ok=True)

    gh, gw = grid_region.shape[:2]
    cell_w = gw // cols
    cell_h = gh // rows

    cell_paths: list[str] = []
    idx = 1
    for r in range(rows):
        for c in range(cols):
            x0 = c * cell_w
            y0 = r * cell_h
            x1 = gw if c == cols - 1 else (c + 1) * cell_w
            y1 = gh if r == rows - 1 else (r + 1) * cell_h
            cell = grid_region[y0:y1, x0:x1]
            th = _threshold_cell(cell, config)

            cell_name = f"{idx:02d}.png"
            cv2.imwrite(str(cells_dir / cell_name), th)
            cell_paths.append(f"cells/{cell_name}")
            idx += 1

    ocr_data = _ocr_title_block(title_region)
    refs_raw = ocr_data.get("refs") or _parse_refs(ocr_data.get("raw_text", ""))
    refs = sorted(set(refs_raw))

    metadata = {
        "template_version": "v0.1",
        "title": ocr_data.get("title", ""),
        "refs": refs,
        "date": ocr_data.get("date", ""),
        "grid": f"{rows}x{cols}",
        "confidence": ocr_data.get("confidence", {}),
        "cells": cell_paths,
        "source_file": src.name,
    }

    if archive_original:
        image_ext = src.suffix.lower() if src.suffix else ".jpg"
        archive_path = out_dir / f"original{image_ext}"
        if src.suffix.lower() == ".pdf":
            Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB)).save(archive_path.with_suffix(".jpg"))
            metadata["source_file_archived"] = "original.jpg"
        else:
            cv2.imwrite(str(archive_path), image)
            metadata["source_file_archived"] = archive_path.name

    (out_dir / "metadata.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")

    if write_debug:
        cv2.imwrite(str(debug_dir / "warped_page.png"), warped)
        cv2.imwrite(str(debug_dir / "grid_region.png"), grid_region)
        cv2.imwrite(str(debug_dir / "title_region.png"), title_region)

    return out_dir

