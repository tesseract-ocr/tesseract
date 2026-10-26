"""Handwritten digit strings (order quantities). A tiny CNN trained by scripts/train_digits.py, run here in plain numpy so the
server needs no torch. Tesseract's digit mode was too unstable on slanted handwriting ("10" -> "0", "11" -> "")."""
from __future__ import annotations

from pathlib import Path
from typing import List, Optional, Tuple

import cv2
import numpy as np
from numpy.lib.stride_tricks import sliding_window_view

from ..config import settings

_W = None


ASSETS = Path(__file__).resolve().parent / "assets"


def model_path() -> Path:
    """A model retrained into storage wins over the one shipped with the app."""
    own = settings.storage_dir / "models" / "digits.npz"
    return own if own.exists() else ASSETS / "digits.npz"


def available() -> bool:
    return model_path().exists()


def _weights():
    global _W
    if _W is None:
        _W = dict(np.load(model_path()))
    return _W


def _conv(x, w, b):                 # x: C,H,W   w: O,C,3,3
    win = sliding_window_view(x, (3, 3), axis=(1, 2))                  # C,H-2,W-2,3,3
    return np.einsum("chwij,ocij->ohw", win, w) + b[:, None, None]


def _pool(x):
    c, h, w = x.shape
    return x[:, : h // 2 * 2, : w // 2 * 2].reshape(c, h // 2, 2, w // 2, 2).max(axis=(2, 4))


def classify(img28: np.ndarray) -> Tuple[int, float]:
    """img28: 28x28 float32, ink = 1.0 on 0.0 paper. Returns (digit, probability)."""
    w = _weights()
    with np.errstate(all="ignore"):          # Accelerate-backed numpy on macOS raises spurious matmul warnings
        return _forward(w, img28)


def _forward(w, img28):
    x = _pool(np.maximum(_conv(img28[None], w["c1.weight"], w["c1.bias"]), 0))
    x = _pool(np.maximum(_conv(x, w["c2.weight"], w["c2.bias"]), 0))
    h = np.maximum(w["f1.weight"] @ x.reshape(-1) + w["f1.bias"], 0)
    z = w["f2.weight"] @ h + w["f2.bias"]
    p = np.exp(z - z.max())
    p /= p.sum()
    return int(p.argmax()), float(p.max())


def _to_mnist(ink: np.ndarray) -> np.ndarray:
    """Scale the glyph to 20px on its longer side and centre it by centre of mass in a 28x28 frame, like MNIST."""
    ys, xs = np.where(ink > 0)
    g = ink[ys.min():ys.max() + 1, xs.min():xs.max() + 1].astype(np.float32)
    s = 20.0 / max(g.shape)
    g = cv2.resize(g, (max(int(round(g.shape[1] * s)), 1), max(int(round(g.shape[0] * s)), 1)), interpolation=cv2.INTER_AREA)
    canvas = np.zeros((28, 28), np.float32)
    canvas[(28 - g.shape[0]) // 2:(28 - g.shape[0]) // 2 + g.shape[0], (28 - g.shape[1]) // 2:(28 - g.shape[1]) // 2 + g.shape[1]] = g
    m = canvas.sum()
    if m > 0:
        cy, cx = (np.indices(canvas.shape) * canvas).sum(axis=(1, 2)) / m
        canvas = cv2.warpAffine(canvas, np.float32([[1, 0, 13.5 - cx], [0, 1, 13.5 - cy]]), (28, 28))
    return np.clip(canvas / max(canvas.max(), 1e-6), 0, 1)


def split_glyphs(ink: np.ndarray, unit_box=None) -> List[np.ndarray]:
    """Ink mask (255 = ink) -> one mask per digit, left to right. Specks and stray dots are dropped; pieces that overlap
    horizontally (a '5' and its top bar) are merged."""
    n, lab, st, _ = cv2.connectedComponentsWithStats((ink > 0).astype(np.uint8), connectivity=8)
    comps = [(st[i, 0], st[i, 0] + st[i, 2], i) for i in range(1, n)]
    if not comps:
        return []
    H, W = ink.shape[:2]
    # a thin tall stroke hugging the left/right edge is the cell border, not a "1"
    comps = [c for c in comps if not (st[c[2], 2] < 0.2 * st[c[2], 3] and (st[c[2], 0] <= 2 or st[c[2], 0] + st[c[2], 2] >= W - 2))]
    if not comps:
        return []
    tallest = max(st[i, 3] for _, _, i in comps)
    comps = [c for c in comps if st[c[2], 3] >= tallest * 0.3 and st[c[2], 4] >= 12]
    if unit_box:
        # the printed unit (本, pc, kg) sits in the cell's bottom-right corner: ignore anything that lies wholly inside it
        ux, uy = unit_box[0] * W, unit_box[1] * H
        comps = [c for c in comps if not (st[c[2], 0] >= ux and st[c[2], 1] >= uy)]
    comps.sort()
    groups: List[list] = []
    for x0, x1, i in comps:
        if groups and x0 < groups[-1][1] - 0.4 * min(x1 - x0, groups[-1][1] - groups[-1][0]):
            groups[-1][1] = max(groups[-1][1], x1)
            groups[-1][2].append(i)
        else:
            groups.append([x0, x1, [i]])
    return [np.isin(lab, g[2]).astype(np.uint8) * 255 for g in groups]


def read_digits(ink: np.ndarray, unit_box=None) -> Optional[Tuple[str, float]]:
    """Read a handwritten number from an ink mask. Returns None when there is no writing at all (an empty cell), otherwise
    (text, confidence 0-100) where text is '' if the strokes could not be read as digits."""
    glyphs = split_glyphs(ink, unit_box)
    if not glyphs:
        return None
    if len(glyphs) > 6 or not available():
        return "", 0.0
    out, conf = [], 1.0
    for g in glyphs:
        d, p = classify(_to_mnist(g))
        out.append(str(d))
        conf = min(conf, p)
    return "".join(out), round(conf * 100, 1)
