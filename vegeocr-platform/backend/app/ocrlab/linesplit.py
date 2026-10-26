"""Find text lines (and word-group chunks) in an image. Line-level recognisers such as TrOCR read ONE line at a time,
and they squash the crop to a square, so very wide lines must be cut into chunks first."""
from __future__ import annotations

from typing import List, Tuple

import cv2
import numpy as np

Box = Tuple[int, int, int, int]  # left, top, right, bottom


def binarise(gray: np.ndarray) -> np.ndarray:
    """Ink = 255. Otsu, with a local fallback for uneven lighting."""
    blur = cv2.GaussianBlur(gray, (3, 3), 0)
    _, bw = cv2.threshold(blur, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    if bw.mean() > 90:  # more than ~35% "ink": the page is shaded; use a local threshold instead
        bw = cv2.adaptiveThreshold(blur, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 35, 15)
    # drop specks
    n, labels, stats, _ = cv2.connectedComponentsWithStats(bw, connectivity=8)
    small = np.isin(labels, [i for i in range(1, n) if stats[i, cv2.CC_STAT_AREA] < 8])
    bw[small] = 0
    return bw


def glyph_height(bw: np.ndarray) -> float:
    n, _, stats, _ = cv2.connectedComponentsWithStats(bw, connectivity=8)
    hs = [stats[i, cv2.CC_STAT_HEIGHT] for i in range(1, n) if stats[i, cv2.CC_STAT_AREA] >= 15]
    return float(np.median(hs)) if hs else 0.0


def _runs(mask: np.ndarray) -> List[Tuple[int, int]]:
    out, start = [], None
    for i, v in enumerate(mask):
        if v and start is None:
            start = i
        elif not v and start is not None:
            out.append((start, i)); start = None
    if start is not None:
        out.append((start, len(mask)))
    return out


def find_lines(gray: np.ndarray, min_ink: float = 0.004) -> List[Box]:
    """Horizontal projection profile: rows with ink form bands; bands separated by a small gap (ascenders / descenders) are merged.
    Works for single-column pages with roughly level lines; deskew first (enhance.py does) when the page is rotated."""
    bw = binarise(gray)
    h, w = bw.shape
    if not bw.any():
        return []
    gh = max(glyph_height(bw), 8.0)
    prof = cv2.GaussianBlur((bw > 0).sum(axis=1).astype(np.float32).reshape(-1, 1), (1, 0), sigmaX=0, sigmaY=max(gh * 0.12, 1.0)).ravel()
    bands = _runs(prof > max(w * min_ink, 1.5))
    merged: List[List[int]] = []
    for a, b in bands:
        if merged and a - merged[-1][1] < gh * 0.45:
            merged[-1][1] = b
        else:
            merged.append([a, b])
    boxes: List[Box] = []
    for a, b in merged:
        if b - a < gh * 0.45:  # a stray mark, not a line
            continue
        cols = np.where(bw[a:b].any(axis=0))[0]
        if len(cols) < gh * 0.6:
            continue
        padx, pady = int(gh * 0.5), int(gh * 0.3)
        boxes.append((int(max(cols[0] - padx, 0)), int(max(a - pady, 0)), int(min(cols[-1] + padx, w)), int(min(b + pady, h))))  # plain ints: numpy ints are not JSON-serialisable
    return boxes


def split_wide(gray: np.ndarray, box: Box, max_aspect: float = 9.0) -> List[Box]:
    """Cut an over-wide line at its widest blank gaps so each chunk is at most `max_aspect` times wider than tall."""
    l, t, r, b = box
    hgt, wid = b - t, r - l
    if wid <= max_aspect * hgt:
        return [box]
    ink = binarise(gray[t:b, l:r]).any(axis=0)
    gaps = [(s, e) for s, e in _runs(~ink) if e - s >= max(hgt * 0.3, 4) and s > 0 and e < len(ink)]
    limit = int(max_aspect * hgt)
    cuts, start = [], 0
    while len(ink) - start > limit:
        usable = [g for g in gaps if start + hgt < g[0] <= start + limit]
        if not usable:  # no gap: hard cut (rare)
            cut = start + limit
        else:
            g = max(usable, key=lambda g: g[0])  # the last gap that still fits
            cut = (g[0] + g[1]) // 2
        cuts.append(cut); start = cut
    edges = [0] + cuts + [len(ink)]
    return [(l + edges[i], t, l + edges[i + 1], b) for i in range(len(edges) - 1)]
