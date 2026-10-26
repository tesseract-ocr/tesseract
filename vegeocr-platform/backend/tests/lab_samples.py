"""Synthetic test images for the OCR lab: crisp text pages, and controlled degradations of them (blur, noise, low resolution, shadows)."""
import os
import random

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

FONTS = ["/System/Library/Fonts/Helvetica.ttc", "/System/Library/Fonts/Supplemental/Arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"]
WORDS = "invoice order total amount payment customer delivery warehouse tomato cabbage carrot weight price tax account number date reference quantity supplier receipt balance".split()


def font(size):
    for f in FONTS:
        if os.path.exists(f):
            return ImageFont.truetype(f, size)
    return ImageFont.load_default(size=size)


def sample_lines(n=8, seed=1, words_per_line=5):
    rnd = random.Random(seed)
    return [" ".join(rnd.choice(WORDS) for _ in range(words_per_line)) + f" {1000 + rnd.randint(0, 8999)}" for _ in range(n)]


def text_page(lines, size=34, margin=40, gap=1.7, handwriting_font=None):
    f = ImageFont.truetype(handwriting_font, size) if handwriting_font else font(size)
    w = int(max(f.getlength(l) for l in lines) + 2 * margin)
    h = int(len(lines) * size * gap + 2 * margin)
    im = Image.new("L", (w, h), 255)
    d = ImageDraw.Draw(im)
    for i, l in enumerate(lines):
        d.text((margin, margin + i * size * gap), l, fill=0, font=f)
    return np.array(im)


def degrade(gray, blur=0.0, noise=0.0, scale=1.0, shadow=0.0, seed=0):
    """blur = Gaussian sigma in px, noise = sigma in grey levels, scale<1 = low resolution (downscaled), shadow = gradient strength 0..1."""
    g = gray.astype(np.float32)
    if scale != 1.0:
        g = cv2.resize(g, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)
    if blur:
        g = cv2.GaussianBlur(g, (0, 0), blur)
    if shadow:
        h, w = g.shape
        ramp = np.tile(np.linspace(1.0, 1.0 - shadow, w, dtype=np.float32), (h, 1))
        g = 255 - (255 - g) * 1.0
        g = g * ramp
    if noise:
        g = g + np.random.RandomState(seed).normal(0, noise, g.shape).astype(np.float32)
    return np.clip(g, 0, 255).astype(np.uint8)
