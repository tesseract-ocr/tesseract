"""なとりデリカ 発注書: a fixed list of items down the left, quantities handwritten in the 数量 and 追加 columns.

The table's ruled rows are doubled and broken by fax noise, so the generic cell finder is not used. Columns come from long vertical
rules, rows from the ink bands of the printed item names (which sit at a regular pitch)."""
from __future__ import annotations

from typing import List

import cv2
import numpy as np

from . import digits
from .forms import QTY_MIN_CONF, Row, FormResult, _binary, paper_gray, prepare, upright

ITEMS = [("11032", "キャベツ", "ケース"), ("11090", "ムキにんにく", "kg"), ("11043", "ピーマン", "kg"), ("11050", "茄子 5kg入り", "kg"),
         ("", "小松菜", "PC"), ("", "茄子5本入り", "PC"), ("", "ローズマリー", "PC"), ("", "りんご", "個"), ("", "(手書き)", "")]
COLS = ["code", "name", "spec", "unit", "qty", "extra", "origin", "note"]


def columns(g) -> List[int]:
    h, w = g.shape
    v = cv2.morphologyEx(_binary(g), cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (1, h // 6)))
    prof = (v > 0).sum(0)
    xs: List[int] = []
    for x in range(1, w - 1):
        if prof[x] >= prof[x - 1] and prof[x] > prof[x + 1] and prof[x] > h * 0.25 and (not xs or x - xs[-1] >= 12):
            xs.append(x)
    return xs


TEMPLATE = [0, 227, 535, 685, 800, 967, 1132, 1276, 1440]          # column rules as measured on a clean copy, from the left edge


def snap(xs: List[int]) -> List[int]:
    """Faint rules go missing on bad faxes: lay the known column layout over the outer rules and snap to any rule found nearby."""
    if len(xs) < 2:
        return xs
    left, right = xs[0], xs[-1]
    scale = (right - left) / TEMPLATE[-1]
    out = []
    for t in TEMPLATE:
        x = left + t * scale
        near = min(xs, key=lambda v: abs(v - x))
        out.append(near if abs(near - x) <= 15 else int(round(x)))
    return out


def item_bands(g, x0, x1, top=300, bottom=2000) -> List[tuple]:
    c = g[top:bottom, x0 + 8:x1 - 8]
    d = cv2.subtract(cv2.medianBlur(c, 31), c)
    bw = (d > 60).astype(np.uint8) * 255
    rules = cv2.morphologyEx(bw, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (max(c.shape[1] // 3, 5), 1)))
    bw = cv2.bitwise_and(bw, cv2.bitwise_not(cv2.dilate(rules, np.ones((5, 1), np.uint8))))
    on = (bw > 0).sum(1) > 6
    out, s = [], None
    for i, o in enumerate(on):
        if o and s is None:
            s = i
        if not o and s is not None:
            if i - s > 12:
                out.append((s + top, i + top))
            s = None
    return out


def read(img) -> FormResult:
    img = upright(img)
    g = paper_gray(img)
    xs = columns(g)
    res = FormResult()
    if len(xs) < 4:
        return res
    xs = snap(xs)
    bands = item_bands(g, xs[1], xs[2])[1:]                           # first band is the printed header
    for i, (y0, y1) in enumerate(bands[: len(ITEMS)]):
        code, name, unit = ITEMS[i]
        row = Row(table=0, index=i, code=code, name=name, unit=unit, confidence=100.0)
        mid, half = (y0 + y1) // 2, 50
        for key, k in (("quantity", 4), ("extra", 5)):
            cell = g[mid - half:mid + half, xs[k] + 8:xs[k + 1] - 6]
            p = prepare(cell, 80, strong=0.6, abs_dark=60, tight=False)
            got = digits.read_digits(255 - p) if p is not None and digits.available() else None
            if got is None:
                continue
            row.handwritten = True
            text, conf = got
            if key == "quantity":
                row.quantity = text
            else:
                row.note = f"追加 {text}"
            row.confidence = min(row.confidence, conf)
            if not text or conf < QTY_MIN_CONF:
                row.review, row.note = True, "quantity unclear"
        res.rows.append(row)
    return res
