"""Order-form (発注書) reader: find the table grid, then read each cell with a recogniser suited to it.

Whole-page OCR garbles these forms because the layout is a dense grid of tiny printed cells plus handwriting. Reading cell by
cell lets us constrain each field: item codes are digits only, quantities are digits only, names are Japanese text. Anything
the engine is not sure about is flagged `review` instead of being guessed, so a person only checks the doubtful cells.
"""
from __future__ import annotations

import os
import tempfile
from dataclasses import dataclass, field, asdict
from typing import List, Optional

import cv2
import numpy as np

from ..services import ocr as tess
from . import digits
from .digits import ASSETS

CODE_MIN_CONF = 75
QTY_MIN_CONF = 90
INK_MIN = 0.012          # fraction of dark pixels in a quantity cell below which it counts as empty


@dataclass
class Cell:
    x: int
    y: int
    w: int
    h: int


@dataclass
class Row:
    table: int
    index: int
    code: str = ""
    name: str = ""
    quantity: str = ""
    unit: str = ""
    handwritten: bool = False
    confidence: float = 100.0
    review: bool = False
    note: str = ""


@dataclass
class FormResult:
    rows: List[Row] = field(default_factory=list)
    header: dict = field(default_factory=dict)
    cells: int = 0

    def as_dict(self):
        return {"header": self.header, "cells": self.cells, "rows": [asdict(r) for r in self.rows]}


def _binary(gray):
    return cv2.adaptiveThreshold(cv2.GaussianBlur(gray, (5, 5), 0), 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                 cv2.THRESH_BINARY_INV, 51, 15)


def paper_gray(img):
    """Greyscale with highlighter wiped out. Light, saturated strokes (green/yellow/pink markers) are painted over with the
    local paper tone so they are not mistaken for print; dark pen and black text are left alone."""
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    hue = hsv[..., 0]                                                  # OpenCV hue 0-179: 20-85 yellow..green, >150 / <8 pink; blue pen is ~95-130
    mark = ((hsv[..., 1] > 25) & (hsv[..., 2] > 90) & (((hue >= 20) & (hue <= 85)) | (hue >= 150) | (hue < 8))).astype(np.uint8) * 255
    mark = cv2.dilate(cv2.morphologyEx(mark, cv2.MORPH_OPEN, np.ones((3, 3), np.uint8)), np.ones((3, 3), np.uint8))
    if not mark.any():
        return gray
    paper = cv2.dilate(gray, np.ones((25, 25), np.uint8))              # brightest nearby pixel = paper
    out = gray.copy()
    out[mark > 0] = paper[mark > 0]
    return out


def upright(img):
    """Rotate a sideways page (fax feeds often arrive turned 90 degrees) so table rules run horizontally. Page orientation is
    judged by which direction carries more ruled-line ink; 90 vs 270 cannot be told apart this way, so 90 clockwise is assumed."""
    h, w = img.shape[:2]
    bw = _binary(cv2.cvtColor(img, cv2.COLOR_BGR2GRAY))
    hor = cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (max(w // 12, 5), 1))).sum()
    ver = cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (1, max(h // 12, 5)))).sum()
    return cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE) if ver > 1.5 * hor else img


def detect_cells(img) -> List[Cell]:
    h, w = img.shape[:2]
    bw = _binary(cv2.cvtColor(img, cv2.COLOR_BGR2GRAY))
    horiz = cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (max(w // 25, 5), 1)))
    vert = cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (1, max(h // 40, 5))))
    grid = cv2.dilate(cv2.bitwise_or(horiz, vert), np.ones((5, 5), np.uint8))
    n, _, stats, _ = cv2.connectedComponentsWithStats(cv2.bitwise_not(grid), connectivity=4)
    return [Cell(int(stats[i, 0]), int(stats[i, 1]), int(stats[i, 2]), int(stats[i, 3])) for i in range(1, n)
            if 40 < stats[i, 2] < w * 0.5 and 25 < stats[i, 3] < h * 0.1]


def _clusters(values, tol):
    out = []
    for v in sorted(values):
        if out and v - out[-1][-1] <= tol:
            out[-1].append(v)
        else:
            out.append([v])
    return out


def group_tables(cells: List[Cell], page_w: int):
    """Return [(code_cells, name_cells, qty_cells)] per table, each list ordered top to bottom."""
    tol = page_w * 0.012
    cols = _clusters([c.x for c in cells], tol)
    centres = [float(np.median(c)) for c in cols]
    by_col = {i: [] for i in range(len(cols))}
    for c in cells:
        by_col[min(range(len(centres)), key=lambda i: abs(centres[i] - c.x))].append(c)
    ordered = [sorted(by_col[i], key=lambda c: c.y) for i in sorted(by_col, key=lambda i: centres[i]) if len(by_col[i]) >= 5]
    return [tuple(ordered[i:i + 3]) for i in range(0, len(ordered) - 2, 3)]


def _crop(img, c: Cell, pad=4, left=None):
    """Cell interior. `left` overrides the left padding (negative reaches into the border, for digits that touch it)."""
    lp = pad if left is None else left
    return img[c.y + pad:c.y + c.h - pad, max(c.x + lp, 0):c.x + c.w - pad]


def prepare(gray, height=48, strong=0.0, min_ink=0.0, abs_dark=0, tight=True):
    """Flatten the background, binarise with Otsu, strip ruled lines, crop to the ink and scale to a recogniser-friendly height.
    Returns None when the crop holds no ink. (Adaptive thresholds hollow out thick strokes, which wrecked large glyphs.)"""
    if gray.size == 0:
        return None
    g = cv2.GaussianBlur(gray, (3, 3), 0)
    bg = cv2.medianBlur(g, 51 if min(g.shape) > 60 else 21)
    d = cv2.subtract(bg, g)
    t, bw = cv2.threshold(d, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    if strong:                                                         # keep only the darkest strokes (pen, not marker)
        bw = ((d > max(t, strong * d.max(), abs_dark)) * 255).astype(np.uint8)
    if d.max() < 25:                                                   # nothing darker than the paper: empty cell
        return None
    h, w = bw.shape
    rules = cv2.bitwise_or(
        cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (max(w // 2, 5), 1))),
        cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (1, max(int(h * 0.8), 5)))))
    bw = cv2.bitwise_and(bw, cv2.bitwise_not(cv2.dilate(rules, np.ones((3, 3), np.uint8))))
    n, lab, st, _ = cv2.connectedComponentsWithStats((bw > 0).astype(np.uint8), connectivity=8)
    keep = np.zeros(n, bool)
    keep[1:] = (st[1:, 4] >= 25) & (st[1:, 3] >= 0.12 * h)                # drop fax speckle that would stretch the ink box
    bw = np.where(keep[lab], 255, 0).astype(np.uint8)
    ys, xs = np.where(bw > 0)
    if len(ys) < 60 or (ys.max() - ys.min()) < 0.25 * h or len(ys) < min_ink * bw.size:   # specks and shading, not writing
        return None
    out = 255 - (bw[max(ys.min() - 4, 0):ys.max() + 5, max(xs.min() - 4, 0):xs.max() + 5] if tight else bw)
    s = height / out.shape[0]
    return cv2.resize(out, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)


def _ink(gray) -> float:
    if gray.size == 0:
        return 0.0
    return float((_binary(gray) > 0).mean())


def _tess(gray, lang, psm, whitelist=None):
    """OCR one small crop; returns (text, confidence)."""
    h, w = gray.shape[:2]
    scale = max(1.0, 64 / max(h, 1))
    g = cv2.resize(gray, None, fx=scale, fy=scale, interpolation=cv2.INTER_CUBIC)
    g = cv2.copyMakeBorder(g, 12, 12, 12, 12, cv2.BORDER_CONSTANT, value=255)
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as f:
        path = f.name
    try:
        cv2.imwrite(path, g)
        cfg = ["tsv"]
        if whitelist:
            cfg = ["-c", f"tessedit_char_whitelist={whitelist}", "tsv"]
        tsv = _run(path, lang, psm, cfg)
    finally:
        os.unlink(path)
    words = []
    for row in tsv.splitlines()[1:]:
        c = row.split("\t")
        if len(c) >= 12 and c[0] == "5" and c[11].strip() and float(c[10]) >= 0:
            words.append((c[11].strip(), float(c[10])))
    if not words:
        return "", 0.0
    return "".join(t for t, _ in words), sum(c for _, c in words) / len(words)


def _run(path, lang, psm, cfg):
    import subprocess
    from ..config import settings
    from ..services.modelstore import RUNTIME
    cmd = [settings.tesseract_bin, path, "-", "-l", lang, "--psm", str(psm), "--tessdata-dir", str(RUNTIME)] + cfg
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    return r.stdout if r.returncode == 0 else ""


# ---- catalog: remember what each printed row looks like -----------------------------------------------------------
SIG_H, SIG_W = 36, 320
MATCH_MIN = 0.55          # below this a printed row is treated as unknown and OCR text is used (flagged for review)
UNIT_BOX = (0.6, 0.4)      # (x, y) fractions of a quantity cell; the printed unit lives below/right of this corner
VERIFY_BELOW = 75
SLOT_BONUS = 0.18
SLOT_FALLBACK = 0.25
MATCH_MARGIN = 0.05       # best must beat the runner-up by this much, otherwise the row is ambiguous


def signature(gray, width=SIG_W):
    """Fixed-size fingerprint of printed text. The same words in the same font give the same fingerprint regardless of
    photo scale, because the ink box is stretched to a fixed size."""
    p = prepare(gray, SIG_H, min_ink=0.02)
    if p is None:
        return None
    sig = cv2.resize(255 - p, (width, SIG_H), interpolation=cv2.INTER_AREA).astype(np.float32) / 255.0
    return cv2.GaussianBlur(sig, (0, 0), 1.2)


def _ncc(a, b):
    a, b = a - a.mean(), b - b.mean()
    d = float(np.sqrt((a * a).sum() * (b * b).sum()))
    return float((a * b).sum() / d) if d else 0.0


class Catalog:
    """code -> {name, unit} plus a fingerprint of the printed code cell and name cell for each item."""

    def __init__(self, items: List[dict], name_sigs: np.ndarray, code_sigs: np.ndarray, title: str = ""):
        self.items, self.name_sigs, self.code_sigs, self.title = items, name_sigs, code_sigs, title

    def match(self, name_sig, code_sig, slot=None):
        """Return (item_index, score, margin) of the closest catalog entry. The same store sends the same printed form, so
        the item that normally sits in this slot gets a head start; a degraded copy still has to resemble it to win."""
        scores = []
        for i in range(len(self.items)):
            n = _ncc(name_sig, self.name_sigs[i]) if name_sig is not None else 0.0
            c = _ncc(code_sig, self.code_sigs[i]) if code_sig is not None else n
            sc = 0.65 * n + 0.35 * c
            if slot is not None and self.items[i].get("slot") == list(slot):
                sc += SLOT_BONUS
            scores.append(sc)
        order = np.argsort(scores)[::-1]
        best = int(order[0])
        second = scores[int(order[1])] if len(order) > 1 else 0.0
        return best, scores[best], scores[best] - second

    def save(self, directory):
        import json
        from pathlib import Path
        d = Path(directory)
        d.mkdir(parents=True, exist_ok=True)
        (d / "catalog.json").write_text(json.dumps({"title": self.title, "items": self.items}, ensure_ascii=False, indent=1), encoding="utf-8")
        np.savez_compressed(d / "refs.npz", names=np.round(self.name_sigs * 255).astype(np.uint8), codes=np.round(self.code_sigs * 255).astype(np.uint8))

    @classmethod
    def load(cls, directory):
        import json
        from pathlib import Path
        d = Path(directory)
        meta = json.loads((d / "catalog.json").read_text(encoding="utf-8"))
        z = np.load(d / "refs.npz")
        return cls(meta["items"], z["names"].astype(np.float32) / 255, z["codes"].astype(np.float32) / 255, meta.get("title", ""))


def rows_of(tables_cells):
    """Yield (table, index, code_cell|None, name_cell, qty_cell|None). Rows are driven by the item-name column because it is the
    one column whose borders survive faxing; the code and quantity cells are looked up by vertical position."""
    for t, (codes, names, qtys) in enumerate(tables_cells):
        for i, nc in enumerate(names[1:]):
            mid = nc.y + nc.h / 2

            def near(cells):
                c = min(cells, key=lambda c: abs(c.y + c.h / 2 - mid), default=None)
                return c if c is not None and abs(c.y + c.h / 2 - mid) < 0.6 * nc.h else None
            yield t, i, near(codes[1:]), nc, near(qtys[1:])


def build_catalog(img, truth: dict, title: str = "") -> Catalog:
    """Learn from one clean form. `truth` maps (table, row_index) -> {code, name, unit} for the printed rows."""
    gray = paper_gray(img)
    items, ns, cs = [], [], []
    for t, i, cc, nc, _q in rows_of(group_tables(detect_cells(img), img.shape[1])):
        info = truth.get((t, i))
        if not info:
            continue
        n = signature(_crop(gray, nc))
        c = signature(_crop(gray, cc, left=0)) if cc is not None else None
        if n is None:
            continue
        items.append({**info, "slot": [t, i]})
        ns.append(n)
        cs.append(c if c is not None else n)
    return Catalog(items, np.stack(ns), np.stack(cs), title)


def read_form(img, catalog: Optional[Catalog] = None) -> FormResult:
    gray = paper_gray(img)
    cells = detect_cells(img)
    res = FormResult(cells=len(cells))
    for t, i, cc, nc, qc in rows_of(group_tables(cells, img.shape[1])):
        name_img = _crop(gray, nc)
        code_img = _crop(gray, cc, left=0) if cc is not None else None
        nsig = signature(name_img)
        if nsig is None:                                              # blank row on the printed form
            continue
        row = Row(table=t, index=i)
        if catalog is not None:
            code_sig = signature(code_img) if code_img is not None else None
            best, score, margin = catalog.match(nsig, code_sig, (t, i))
            slot_item = next((k for k, it in enumerate(catalog.items) if it.get("slot") == [t, i]), None)
            if score < MATCH_MIN and slot_item is not None and _ncc(nsig, catalog.name_sigs[slot_item]) >= SLOT_FALLBACK:
                best, score, margin = slot_item, MATCH_MIN, 0.0       # smudged or marked, but it is where this item normally sits
            if score >= MATCH_MIN:
                it = catalog.items[best]
                row.code, row.name, row.unit = it["code"], it["name"], it.get("unit", "")
                row.confidence = round(100 * min(1.0, score), 1)
                if margin < MATCH_MARGIN:
                    row.review, row.note = True, "ambiguous item" if score > MATCH_MIN else "check item (marked or smudged)"
        if not row.code:                                              # unknown item: fall back to plain OCR and flag it
            p = prepare(code_img) if code_img is not None else None
            row.code = _tess(p, "eng", 7, "0123456789")[0] if p is not None else ""
            pn = prepare(name_img)
            row.name, row.confidence = _tess(pn, "jpn", 7) if pn is not None else ("", 0.0)
            row.review, row.note = True, "item not in catalog"
        if qc is not None:
            hand = _crop(gray, qc)
            p = prepare(hand[:, 6: int(hand.shape[1] - 4)], 80, strong=0.6, abs_dark=60, tight=False)  # unit text is short; digits are tall
            if p is not None:
                got = digits.read_digits(255 - p, unit_box=UNIT_BOX) if digits.available() else None
                if got is not None:
                    row.handwritten = True
                    row.quantity, qcf = got
                    row.confidence = min(row.confidence, qcf) if row.confidence else qcf
                    if not row.quantity or qcf < QTY_MIN_CONF:
                        row.review, row.note = True, "quantity unclear"
                    elif row.confidence < VERIFY_BELOW:                # a marked or degraded cell: digits can look confident and still be wrong
                        row.review, row.note = True, "verify quantity (poor scan)"
        if row.note == "item not in catalog" and not row.quantity and row.confidence < 55:
            continue                                                  # shading or a smudge on an empty line, not an item
        res.rows.append(row)
    return res
