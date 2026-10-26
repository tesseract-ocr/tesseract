"""Enhancement pipeline for blurry / noisy / low-resolution images. Every step is optional and reported, so the UI can say exactly what was done.

Order matters: denoise first (sharpening amplifies noise), then fix lighting, then upscale, then sharpen at the final resolution.
"""
from __future__ import annotations

import math
from dataclasses import dataclass, field
from typing import List, Tuple

import cv2
import numpy as np

from . import quality as Q

TARGET_TEXT_HEIGHT = 24      # px: comfortable median glyph height (about the x-height) for OCR engines
MAX_UPSCALE = 3.0
MAX_SIDE = 5000


@dataclass
class EnhanceOptions:
    denoise: bool = True
    fix_lighting: bool = True
    upscale: bool = True
    deskew: bool = True
    sharpen: bool = True
    contrast: bool = True
    # Tuned by a parameter sweep over blur / noise / low-res / shadow pages (tests/eval_enhancement.py): a stronger, wider unsharp mask beat the
    # gentle one, and CLAHE (now only for really faint pages) mostly amplified noise. Synthetic degradations: re-check on your own images.
    sharpen_gain: float = 1.6      # multiplies the unsharp-mask strength
    sharpen_radius: float = 1.5    # multiplies the unsharp-mask radius
    max_upscale: float = MAX_UPSCALE


@dataclass
class EnhanceResult:
    image: np.ndarray                       # single-channel uint8
    steps: List[str] = field(default_factory=list)
    scale: float = 1.0
    angle: float = 0.0


def _denoise(gray: np.ndarray, sigma: float) -> np.ndarray:
    # Non-local means keeps stroke edges; strength follows the measured noise.
    return cv2.fastNlMeansDenoising(gray, None, h=float(np.clip(sigma * 1.1, 4, 14)), templateWindowSize=7, searchWindowSize=21)


def _fix_lighting(gray: np.ndarray) -> np.ndarray:
    """Shading correction: divide by the estimated paper colour so shadows and gradients become flat white."""
    k = max(15, (min(gray.shape) // 20) | 1)
    bg = cv2.medianBlur(cv2.dilate(gray, np.ones((k, k), np.uint8)), k)
    return cv2.divide(gray, bg, scale=255)


def _text_mask_angle(gray: np.ndarray) -> float:
    """Skew angle in degrees (+ = text runs downhill), from the minimum-area rectangle of the ink pixels."""
    _, bw = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    pts = cv2.findNonZero(bw)
    if pts is None or len(pts) < 200:
        return 0.0
    angle = cv2.minAreaRect(pts)[-1]
    angle = angle - 90 if angle > 45 else angle  # OpenCV >= 4.5 reports (0, 90]
    return float(angle)


def _rotate(gray: np.ndarray, angle: float) -> np.ndarray:
    h, w = gray.shape
    m = cv2.getRotationMatrix2D((w / 2, h / 2), angle, 1.0)
    return cv2.warpAffine(gray, m, (w, h), flags=cv2.INTER_CUBIC, borderMode=cv2.BORDER_REPLICATE)


def _unsharp(gray: np.ndarray, level: str, scale: float, gain: float = 1.0, radius: float = 1.0) -> np.ndarray:
    """Unsharp mask whose radius and strength follow how blurry the image was (and how much we upscaled it)."""
    sigma = {"soft": 1.0, "blurry": 1.8}.get(level, 0.8) * max(1.0, math.sqrt(scale)) * radius
    amount = {"soft": 0.9, "blurry": 1.6}.get(level, 0.6) * gain
    blurred = cv2.GaussianBlur(gray, (0, 0), sigma)
    return cv2.addWeighted(gray, 1 + amount, blurred, -amount, 0)


def enhance(img: np.ndarray, report: Q.QualityReport | None = None, opts: EnhanceOptions | None = None) -> EnhanceResult:
    opts = opts or EnhanceOptions()
    gray = Q.to_gray(img).copy()
    rep = report or Q.analyse(gray)
    res = EnhanceResult(gray)

    if opts.denoise and rep.noise_sigma > Q.NOISE_HIGH * 0.6:
        gray = _denoise(gray, rep.noise_sigma)
        res.steps.append(f"denoise (σ≈{rep.noise_sigma:.1f})")
    if opts.fix_lighting and rep.uneven_lighting > Q.UNEVEN_LIGHT * 0.7:
        gray = _fix_lighting(gray)
        res.steps.append("shading correction")
    if opts.deskew:
        angle = _text_mask_angle(gray)
        if 0.4 <= abs(angle) <= 12:
            gray = _rotate(gray, angle)
            res.angle = angle
            res.steps.append(f"deskew ({angle:+.1f}°)")
    if opts.upscale and rep.text_height > 0 and rep.text_height < TARGET_TEXT_HEIGHT:
        f = min(opts.max_upscale, TARGET_TEXT_HEIGHT / rep.text_height, MAX_SIDE / max(gray.shape))
        if f >= 1.15:
            gray = cv2.resize(gray, None, fx=f, fy=f, interpolation=cv2.INTER_CUBIC)
            res.scale = f
            res.steps.append(f"upscale ×{f:.1f}")
    if opts.sharpen and rep.blur_level != "sharp":
        gray = _unsharp(gray, rep.blur_level, res.scale, opts.sharpen_gain, opts.sharpen_radius)
        res.steps.append(f"sharpen ({rep.blur_level})")
    if opts.contrast and rep.contrast < Q.CONTRAST_LOW * 0.8:  # only genuinely faint pages (std-dev under ~30)
        gray = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8)).apply(gray)
        res.steps.append("contrast (CLAHE)")
    res.image = np.clip(gray, 0, 255).astype(np.uint8)
    return res
