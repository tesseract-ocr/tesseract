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


# ---------------------------------------------------------------------------
# llm_extract: post-checks and local adapter
# ---------------------------------------------------------------------------

def test_post_check_unit_normalisation():
    from app.ocrlab import llm_extract
    doc = {
        "supplier": None, "customer_name": None, "customer_code": None,
        "contact_person": None, "order_date": None, "delivery_date": "2025-09-10",
        "delivery_location": None, "form_template_id": None, "notes": None,
        "fax_received_at": None, "change_type": None, "billing_date": None,
        "flags": [],
        "items": [
            {"item_name": "キャベツ", "spec": None, "quantity": 3, "additional_quantity": None,
             "unit": "ケ", "unit_raw": None, "delivery_date": None, "line_note": None,
             "unit_price": None, "confidence": 90, "flags": []},
        ],
    }
    out = llm_extract._post_check(doc)
    item = out["items"][0]
    assert item["unit"] == "個"
    assert item["unit_raw"] == "ケ"
    assert "UNIT_OVERRIDDEN" in item["flags"]


def test_post_check_weekday_mismatch():
    from app.ocrlab import llm_extract
    doc = {
        "supplier": None, "customer_name": None, "customer_code": None,
        "contact_person": None, "order_date": None, "delivery_date": "2025-09-10",  # 2025-09-10 is a Wednesday (水)
        "delivery_location": None, "form_template_id": None, "notes": None,
        "fax_received_at": None, "change_type": None, "billing_date": None,
        "flags": [],
        "items": [
            {"item_name": "大根", "spec": None, "quantity": 2, "additional_quantity": None,
             "unit": "本", "unit_raw": None, "delivery_date": None,
             "line_note": "月曜日",  # says Monday — mismatch
             "unit_price": None, "confidence": 88, "flags": []},
        ],
    }
    out = llm_extract._post_check(doc)
    assert "WEEKDAY_MISMATCH" in out["items"][0]["flags"]
    assert "WEEKDAY_MISMATCH" in out["flags"]


def test_post_check_form_template_id_rejected_if_not_8_digits():
    from app.ocrlab import llm_extract
    doc = {
        "supplier": None, "customer_name": None, "customer_code": None,
        "contact_person": None, "order_date": None, "delivery_date": None,
        "delivery_location": None, "form_template_id": "2025",  # 4-digit year — not allowed
        "notes": None, "fax_received_at": None, "change_type": None, "billing_date": None,
        "flags": [], "items": [],
    }
    out = llm_extract._post_check(doc)
    assert out["form_template_id"] is None


def test_post_check_valid_8_digit_template_id_kept():
    from app.ocrlab import llm_extract
    doc = {
        "supplier": None, "customer_name": None, "customer_code": None,
        "contact_person": None, "order_date": None, "delivery_date": None,
        "delivery_location": None, "form_template_id": "20240501",
        "notes": None, "fax_received_at": None, "change_type": None, "billing_date": None,
        "flags": [], "items": [],
    }
    out = llm_extract._post_check(doc)
    assert out["form_template_id"] == "20240501"


def test_missing_delivery_date_flag():
    from app.ocrlab import llm_extract
    doc = {
        "supplier": None, "customer_name": None, "customer_code": None,
        "contact_person": None, "order_date": None, "delivery_date": None,
        "delivery_location": None, "form_template_id": None, "notes": None,
        "fax_received_at": None, "change_type": None, "billing_date": None,
        "flags": [],
        "items": [
            {"item_name": "ほうれん草", "spec": None, "quantity": 5, "additional_quantity": None,
             "unit": "袋", "unit_raw": None, "delivery_date": None, "line_note": None,
             "unit_price": None, "confidence": 80, "flags": []},
        ],
    }
    out = llm_extract._post_check(doc)
    assert "MISSING_DELIVERY_DATE" in out["items"][0]["flags"]


def test_local_to_extract_from_form_result():
    """local_to_extract converts a FormResult into the extraction schema."""
    from app.ocrlab import llm_extract
    from app.ocrlab.forms import FormResult, Row

    rows = [
        Row(table=0, index=1, code="11032", name="キャベツ", quantity="3", unit="ケース",
            handwritten=True, confidence=95.0, review=False, note=""),
        Row(table=0, index=2, code="11090", name="ムキにんにく", quantity="1", unit="kg",
            handwritten=True, confidence=88.0, review=True, note="追加 2"),
    ]
    res = FormResult(rows=rows, cells=30)
    out = llm_extract.local_to_extract(res, catalog="natori", delivery_date="2025-09-10")

    assert out["_source"] == "local"
    assert len(out["items"]) == 2

    first = out["items"][0]
    assert first["item_name"] == "キャベツ"
    assert first["quantity"] == 3
    assert first["delivery_date"] == "2025-09-10"

    second = out["items"][1]
    assert second["additional_quantity"] == 2
    assert second["line_note"] is None   # "追加 N" is parsed out of note
    assert "DIGIT_AMBIGUOUS" in second["flags"]   # review=True + handwritten=True


def test_llm_unavailable_error_without_key(monkeypatch):
    from app.ocrlab import llm_extract
    monkeypatch.setattr("app.ocrlab.llm_extract.llm_available", lambda: False)
    # Patch the config so anthropic_api_key is empty
    import app.config as cfg
    orig = cfg.settings.anthropic_api_key
    cfg.settings.anthropic_api_key = ""
    try:
        with pytest.raises(llm_extract.LLMUnavailableError):
            llm_extract.extract_with_llm(b"fake")
    finally:
        cfg.settings.anthropic_api_key = orig


def test_extract_with_llm_mocked(monkeypatch):
    """extract_with_llm parses the Claude response and runs post-checks."""
    import app.config as cfg
    from app.ocrlab import llm_extract

    cfg.settings.anthropic_api_key = "sk-ant-test-key"

    llm_response_json = """{
  "supplier": "なとりデリカ",
  "customer_name": null,
  "customer_code": null,
  "contact_person": null,
  "order_date": null,
  "delivery_date": "2025-09-10",
  "delivery_location": null,
  "form_template_id": null,
  "notes": null,
  "fax_received_at": null,
  "change_type": null,
  "billing_date": null,
  "flags": [],
  "items": [
    {
      "item_name": "キャベツ",
      "spec": null,
      "quantity": 3,
      "additional_quantity": 1,
      "unit": "ヶ",
      "unit_raw": null,
      "delivery_date": null,
      "line_note": null,
      "unit_price": null,
      "confidence": 92,
      "flags": []
    }
  ]
}"""

    class _FakeBlock:
        type = "text"
        text = llm_response_json

    class _FakeResponse:
        content = [_FakeBlock()]

    class _FakeMessages:
        def create(self, **kw):
            return _FakeResponse()

    class _FakeClient:
        messages = _FakeMessages()

    import anthropic
    monkeypatch.setattr(anthropic, "Anthropic", lambda **kw: _FakeClient())

    result = llm_extract.extract_with_llm(b"\xff\xd8\xff\xe0fake-jpeg-bytes")
    item = result["items"][0]
    assert item["unit"] == "個"          # normalised from ヶ
    assert item["unit_raw"] == "ヶ"
    assert "UNIT_OVERRIDDEN" in item["flags"]
    assert result["supplier"] == "なとりデリカ"
    assert result["items"][0]["additional_quantity"] == 1
