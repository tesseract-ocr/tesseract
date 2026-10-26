"""Image quality analysis: is this picture good enough to read as-is, or should it be enhanced first?"""
from __future__ import annotations

import os
from dataclasses import asdict, dataclass, field
from typing import List

import cv2
import numpy as np

# Thresholds were calibrated on text images blurred / noised at known strengths (see tests/eval_enhancement.py).
BLUR_SOFT = float(os.environ.get("LAB_BLUR_SOFT", 0.38))      # sharpness below this: slightly soft (about a 1 px Gaussian blur)
BLUR_BLURRY = float(os.environ.get("LAB_BLUR_BLURRY", 0.24))  # below this: clearly blurry (about a 2 px blur or worse)
NOISE_HIGH = float(os.environ.get("LAB_NOISE_HIGH", 6.0))     # estimated noise sigma (grey levels)
CONTRAST_LOW = float(os.environ.get("LAB_CONTRAST_LOW", 38.0))
TEXT_HEIGHT_MIN = int(os.environ.get("LAB_TEXT_HEIGHT_MIN", 15))  # px (median glyph height ~ x-height): OCR engines read poorly below this
UNEVEN_LIGHT = float(os.environ.get("LAB_UNEVEN_LIGHT", 18.0))
ANALYSIS_WINDOW = 1600  # px: sharpness is measured at native resolution on at most this big a window


@dataclass
class QualityReport:
    width: int
    height: int
    sharpness: float          # edge sharpness 0..~0.6: higher = sharper
    blur_level: str           # sharp | soft | blurry
    noise_sigma: float
    contrast: float           # grey-level standard deviation
    brightness: float
    uneven_lighting: float    # spread of the estimated background (shadows / gradients)
    text_height: float        # median connected-component height in px (0 if unknown)
    needs_enhancement: bool = False
    reasons: List[str] = field(default_factory=list)

    def as_dict(self) -> dict:
        d = asdict(self)
        for k in ("sharpness", "noise_sigma", "contrast", "brightness", "uneven_lighting", "text_height"):
            d[k] = round(float(d[k]), 4 if k == "sharpness" else 1)
        return d


def to_gray(img: np.ndarray) -> np.ndarray:
    return img if img.ndim == 2 else cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)


def _densest_window(gray: np.ndarray) -> np.ndarray:
    """At most ANALYSIS_WINDOW square of the image, chosen where the most ink is (so a mostly-blank page isn't judged on its margin).
    Not resized: shrinking an image would hide its blur."""
    h, w = gray.shape
    win = ANALYSIS_WINDOW
    if h <= win and w <= win:
        return gray
    ink = (gray < gray.mean() - 20).astype(np.float32)
    best, pos = -1.0, (0, 0)
    for y in range(0, max(h - win, 0) + 1, max(win // 2, 1)):
        for x in range(0, max(w - win, 0) + 1, max(win // 2, 1)):
            v = float(ink[y:y + win, x:x + win].sum())
            if v > best:
                best, pos = v, (y, x)
    return gray[pos[0]:pos[0] + win, pos[1]:pos[1] + win]


def sharpness(gray: np.ndarray) -> float:
    """Edge sharpness: the steepest gradients (99.5th percentile) relative to the image's intensity range.
    A crisp edge goes from ink to paper within ~1 px (~0.5); blur spreads it over ~2.5σ px (σ=1: ~0.36, σ=2: ~0.22, σ=4: ~0.11).
    Unlike a Laplacian-variance score this barely depends on font size or page content (checked at 20/34/60 px text)."""
    g = _densest_window(gray).astype(np.float32)
    gx = cv2.Sobel(g, cv2.CV_32F, 1, 0, ksize=3) / 8.0
    gy = cv2.Sobel(g, cv2.CV_32F, 0, 1, ksize=3) / 8.0
    lo, hi = np.percentile(g, [1, 99])
    return float(np.percentile(np.hypot(gx, gy), 99.5) / max(float(hi - lo), 1.0))


def estimate_noise(gray: np.ndarray) -> float:
    """Immerkaer's fast noise-sigma estimate (robust to image structure)."""
    g = gray.astype(np.float32)
    h, w = g.shape
    kernel = np.array([[1, -2, 1], [-2, 4, -2], [1, -2, 1]], dtype=np.float32)
    s = np.abs(cv2.filter2D(g, -1, kernel)[1:-1, 1:-1]).sum()
    return float(s * np.sqrt(0.5 * np.pi) / (6.0 * (w - 2) * (h - 2)))


def background_unevenness(gray: np.ndarray) -> float:
    """Std-dev of a heavily smoothed copy: ~0 for a flat scan, large for shadows or gradient lighting."""
    small = cv2.resize(gray, (min(gray.shape[1], 256), min(gray.shape[0], 256)), interpolation=cv2.INTER_AREA)
    bg = cv2.dilate(small, np.ones((15, 15), np.uint8))  # text is dark: dilation removes it, leaving the paper
    return float(cv2.GaussianBlur(bg, (0, 0), 7).std())


def estimate_text_height(gray: np.ndarray) -> float:
    """Median height of dark connected components: a stand-in for character height."""
    g = gray
    if max(g.shape) > 1600:
        s = 1600 / max(g.shape)
        g = cv2.resize(g, (int(g.shape[1] * s), int(g.shape[0] * s)), interpolation=cv2.INTER_AREA)
    else:
        s = 1.0
    _, bw = cv2.threshold(g, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    n, _, stats, _ = cv2.connectedComponentsWithStats(bw, connectivity=8)
    heights = [stats[i, cv2.CC_STAT_HEIGHT] for i in range(1, n)
               if 4 <= stats[i, cv2.CC_STAT_HEIGHT] <= g.shape[0] * 0.3 and stats[i, cv2.CC_STAT_AREA] >= 6]
    return float(np.median(heights) / s) if len(heights) >= 5 else 0.0


def analyse(img: np.ndarray) -> QualityReport:
    gray = to_gray(img)
    h, w = gray.shape
    sharp = sharpness(gray)
    level = "blurry" if sharp < BLUR_BLURRY else "soft" if sharp < BLUR_SOFT else "sharp"
    noise, contrast = estimate_noise(gray), float(gray.std())
    uneven, th = background_unevenness(gray), estimate_text_height(gray)
    r = QualityReport(w, h, sharp, level, noise, contrast, float(gray.mean()), uneven, th)
    if level == "blurry":
        r.reasons.append(f"blurry (edge sharpness {sharp:.2f}, a sharp page is above {BLUR_SOFT})")
    elif level == "soft":
        r.reasons.append(f"slightly soft (edge sharpness {sharp:.2f}, a sharp page is above {BLUR_SOFT})")
    if noise > NOISE_HIGH:
        r.reasons.append(f"noisy (σ≈{noise:.1f})")
    if contrast < CONTRAST_LOW:
        r.reasons.append(f"low contrast ({contrast:.0f})")
    if uneven > UNEVEN_LIGHT:
        r.reasons.append("uneven lighting / shadows")
    if 0 < th < TEXT_HEIGHT_MIN:
        r.reasons.append(f"small text (~{th:.0f}px tall)")
    r.needs_enhancement = bool(r.reasons)
    return r
