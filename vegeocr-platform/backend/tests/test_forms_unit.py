"""Order-form reader: grid detection, catalog matching, handwritten digits, marker handling. Synthetic forms only (no customer data)."""
import cv2
import numpy as np
import pytest

from app.ocrlab import digits, forms

ITEMS = ["BT 13/15", "Squid IQF", "Tuna 500g", "Seaweed", "Shrimp M", "Octopus", "Salmon cut", "Yuba 5pc"]


def make_form(qty=None, marker=False, noise=0, w=1500, h=1300):
    """Three tables are overkill; one table of code | name | qty cells is the same geometry the reader uses per table."""
    img = np.full((h, w, 3), 235, np.uint8)
    cols = [(60, 200), (200, 900), (900, 1100)]            # code, name, quantity
    top, rh = 100, 100
    for x0, x1 in cols:
        cv2.rectangle(img, (x0, top), (x1, top + rh * (len(ITEMS) + 3)), (0, 0, 0), 3)
    for r in range(len(ITEMS) + 4):
        cv2.line(img, (60, top + rh * r), (1100, top + rh * r), (0, 0, 0), 3)
    for x0, x1 in cols:
        cv2.line(img, (x0, top), (x0, top + rh * (len(ITEMS) + 3)), (0, 0, 0), 3)
    cv2.line(img, (1100, top), (1100, top + rh * (len(ITEMS) + 3)), (0, 0, 0), 3)
    for i, name in enumerate(ITEMS):                       # row 0 is the header, row 1 stays blank
        y = top + rh * (i + 2)
        cv2.putText(img, f"{40000 + i * 37}", (70, y + 65), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 0, 0), 2)
        cv2.putText(img, name, (220, y + 65), cv2.FONT_HERSHEY_DUPLEX, 1.5, (0, 0, 0), 3)
        cv2.putText(img, "pc", (1045, y + 90), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 0), 2)
    cv2.putText(img, "Item", (220, top + 65), cv2.FONT_HERSHEY_DUPLEX, 1.5, (0, 0, 0), 3)
    for i, q in (qty or {}).items():
        y = top + rh * (i + 2)
        cv2.putText(img, str(q), (920, y + 70), cv2.FONT_HERSHEY_SCRIPT_SIMPLEX, 1.9, (20, 20, 20), 3)
    if marker:
        overlay = img.copy()
        cv2.line(overlay, (200, top + 330), (1000, top + 345), (120, 220, 130), 38)   # light green highlighter across rows
        img = cv2.addWeighted(overlay, 0.55, img, 0.45, 0)
    if noise:
        img = np.clip(img.astype(np.int16) + np.random.RandomState(1).normal(0, noise, img.shape), 0, 255).astype(np.uint8)
    return img


@pytest.fixture(scope="module")
def catalog():
    truth = {(0, i): {"code": str(40000 + i * 37), "name": n, "unit": "pc"} for i, n in enumerate(ITEMS)}
    # the blank row (index 0) shifts items down by one
    truth = {(0, i + 1): v for (_, i), v in truth.items()}
    return forms.build_catalog(make_form(), truth, "test")


def test_grid_is_found():
    cells = forms.detect_cells(make_form())
    assert len(cells) >= 3 * (len(ITEMS) + 3)


def test_rows_are_read_and_blank_rows_skipped(catalog):
    res = forms.read_form(make_form(), catalog)
    assert [r.code for r in res.rows] == [str(40000 + i * 37) for i in range(len(ITEMS))]
    assert all(not r.review for r in res.rows)


def test_degraded_copy_still_matches(catalog):
    res = forms.read_form(cv2.GaussianBlur(make_form(noise=12), (0, 0), 1.3), catalog)
    assert [r.code for r in res.rows] == [str(40000 + i * 37) for i in range(len(ITEMS))]


def test_highlighter_does_not_change_the_item(catalog):
    res = forms.read_form(make_form(marker=True), catalog)
    assert [r.code for r in res.rows if not r.review] == [str(40000 + i * 37) for i in range(len(ITEMS)) if not any(x.review and x.code == str(40000 + i * 37) for x in res.rows)]
    assert len(res.rows) == len(ITEMS)


@pytest.mark.skipif(not digits.available(), reason="digit model not installed")
def test_handwritten_quantities_are_read(catalog):
    want = {2: 12, 4: 5, 6: 30}
    res = forms.read_form(make_form(qty=want), catalog)
    got = {r.index: r.quantity for r in res.rows if r.quantity}
    assert {k + 1: str(v) for k, v in want.items()} == got      # +1: the blank row


def test_empty_quantity_cells_stay_empty(catalog):
    res = forms.read_form(make_form(), catalog)
    assert not any(r.handwritten or r.quantity for r in res.rows)


def test_unknown_item_is_flagged_not_guessed():
    other = forms.build_catalog(make_form(), {(0, 1): {"code": "40000", "name": "BT 13/15", "unit": "pc"}}, "tiny")
    res = forms.read_form(make_form(), other)
    unknown = [r for r in res.rows if r.note == "item not in catalog"]
    assert unknown and all(r.review for r in unknown)


@pytest.mark.skipif(not digits.available(), reason="digit model not installed")
def test_digit_classifier_on_script_digits():
    ok = 0
    for d in range(10):
        canvas = np.zeros((90, 70), np.uint8)
        cv2.putText(canvas, str(d), (8, 70), cv2.FONT_HERSHEY_SCRIPT_SIMPLEX, 2.4, 255, 4)
        text, conf = digits.read_digits(canvas)
        ok += text == str(d)
    assert ok >= 8


def test_catalog_roundtrip(tmp_path, catalog):
    catalog.save(tmp_path)
    again = forms.Catalog.load(tmp_path)
    assert again.items == catalog.items
    assert np.abs(again.name_sigs - catalog.name_sigs).max() < 0.01


def test_sideways_page_is_turned_upright():
    page = make_form()
    sideways = cv2.rotate(page, cv2.ROTATE_90_COUNTERCLOCKWISE)
    fixed = forms.upright(sideways)
    assert fixed.shape == page.shape
    assert forms.upright(page) is page


def test_vocabulary_snaps_close_readings_and_refuses_far_ones():
    from app.ocrlab.vocab import Vocabulary
    v = Vocabulary()
    assert v.match("キャベツ")[0] == "キャベツ" and not v.match("キャベツ")[2]
    assert v.match("きゃべつ")[0] == "キャベツ"            # kana script does not matter
    assert v.match("ビーマン")[0] == "ピーマン"            # one wrong mark
    assert v.match("ぴゃぴゃぴゃ")[0] is None              # garbage is not forced onto an item
    v.add("カリフラワー")
    assert v.match("カリフラワ")[0] == "カリフラワー"
