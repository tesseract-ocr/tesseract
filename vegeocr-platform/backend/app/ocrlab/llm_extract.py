"""LLM-based order-form extraction (発注書 → structured JSON).

Two modes:
  1. ``extract_with_llm(img_bytes)`` — sends the image to Claude (claude-opus-5-5) with the
     extraction spec as the system prompt and returns the parsed JSON dict.
     Requires ``ANTHROPIC_API_KEY`` to be set; raises ``LLMUnavailableError`` otherwise.

  2. ``local_to_extract(form_result, catalog)`` — converts a ``forms.FormResult`` or
     ``natori.FormResult`` (from the cell-grid reader) into the same JSON schema so callers
     get a consistent output regardless of the extraction path.

Both paths run the same deterministic post-checks:
  * weekday validation against the date
  * unit normalisation (ケ/ヶ → 個, with ``unit_raw`` and ``UNIT_OVERRIDDEN``)
  * ``MISSING_DELIVERY_DATE`` flag when delivery_date is absent on a line
"""
from __future__ import annotations

import base64
import json
import re
from datetime import date, datetime
from pathlib import Path
from typing import Any, Dict, List, Optional

_PROMPT_PATH = Path(__file__).parent / "extract_prompt.md"

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
WEEKDAYS_JP = ["月", "火", "水", "木", "金", "土", "日"]   # 0 = Mon

# Unit normalisation map (raw written form → normalised).
# Numeric strings are center-master unit codes.
UNIT_MAP = {
    # individual-piece variants
    "ケ": "個", "ヶ": "個", "个": "個", "コ": "個", "玉": "個",
    "01": "個",
    # count / length
    "02": "本",
    # pack / bag
    "P": "パック", "P/C": "パック", "PC": "パック", "Pc": "パック",
    "ﾊﾟｯｸ": "パック", "袋": "パック",   # 袋 maps to パック for center-master code 11 context
    "11": "パック",
    # set
    "セット": "セット", "ｾｯﾄ": "セット", "21": "セット",
    # weight
    "K": "kg", "Kg": "kg", "KG": "kg", "ｋｇ": "kg", "㎏": "kg", "キロ": "kg",
    "グラム": "g", "ｇ": "g",
    # case
    "ｹｰｽ": "ケース",
}

# Flags that 袋 should NOT be normalised to パック when it genuinely means 袋
# (preserved units per v3 spec)
_KEEP_AS_IS_UNITS = {"束", "把", "丁", "枚", "房", "株", "節", "粒", "瓶", "缶", "斤", "杯", "柵"}

_DOC_FLAGS = {
    "DUPLICATE", "PAGE_CROSSED_OUT", "LOW_IMAGE_QUALITY", "NEEDS_HUMAN_REVIEW",
    "ILLEGIBLE", "AMBIGUOUS_DATE", "MISSING_DELIVERY_DATE", "DATE_CORRECTED",
    "WEEKDAY_MISMATCH", "VERSION_CONFLICT", "DATE_CORRECTED_BY_NOTE",
    "UNMATCHED_CUSTOMER",
}
_LINE_FLAGS = {
    "DIGIT_AMBIGUOUS", "ROW_AMBIGUOUS", "UNIT_OVERRIDDEN", "QUANTITY_CORRECTED",
    "DATE_CORRECTED", "WEEKDAY_MISMATCH", "MISSING_DELIVERY_DATE", "AMBIGUOUS_DATE",
    "ILLEGIBLE",
}
_ORDER_LINE_FLAGS = {
    "DISCONTINUED_ITEM", "CODE_NAME_MISMATCH", "MULTIPLE_MATCHES",
    "UNMATCHED_ITEM", "UNMATCHED_CUSTOMER", "QTY_OUTLIER",
    "UNIT_CODE_UNKNOWN", "UNIT_MISMATCH", "NEEDS_HUMAN_REVIEW",
    "DIGIT_AMBIGUOUS", "ROW_AMBIGUOUS",
}

MODEL = "claude-opus-5-5"
MAX_TOKENS = 8192


# ---------------------------------------------------------------------------
# Exceptions
# ---------------------------------------------------------------------------
class LLMUnavailableError(RuntimeError):
    """Raised when the Anthropic API key is not configured."""


# ---------------------------------------------------------------------------
# Internal helpers
# ---------------------------------------------------------------------------
def _system_prompt() -> str:
    return _PROMPT_PATH.read_text(encoding="utf-8")


def _clean_flags(flags: Any, allowed: set) -> List[str]:
    if not isinstance(flags, list):
        return []
    return [f for f in flags if isinstance(f, str) and f in allowed]


def _parse_date(s: Any) -> Optional[str]:
    if not isinstance(s, str) or not s.strip():
        return None
    s = s.strip()
    if re.fullmatch(r"\d{4}-\d{2}-\d{2}", s):
        return s
    return None


def _weekday_matches(d_str: str, text: str) -> bool:
    """Return False if a Japanese weekday abbreviation in *text* contradicts *d_str*."""
    m = re.search(r"([月火水木金土日])曜", text or "")
    if not m:
        return True
    try:
        d = date.fromisoformat(d_str)
    except ValueError:
        return True
    expected = WEEKDAYS_JP.index(m.group(1))
    return d.weekday() == expected


def _normalise_unit(raw: Any) -> tuple:
    """Return (normalised_unit, unit_raw_if_changed, overridden).

    Units in _KEEP_AS_IS_UNITS are never remapped.  袋 that arrives as a
    center-master numeric code "11" is remapped to パック; the written word 袋
    is kept as-is unless it is a numeric code context (handled by UNIT_MAP entry).
    """
    if not isinstance(raw, str) or not raw.strip():
        return raw, None, False
    stripped = raw.strip()
    if stripped in _KEEP_AS_IS_UNITS:
        return stripped, None, False
    # 袋 as a plain word is kept; it only maps to パック via "11" code
    if stripped == "袋":
        return stripped, None, False
    n = UNIT_MAP.get(stripped, stripped)
    if n != stripped:
        return n, stripped, True
    return stripped, None, False


def _post_check(doc: dict) -> dict:
    """Apply deterministic rules to a parsed extraction dict and return it."""
    # Unit normalisation and weekday check on each line
    doc_delivery = _parse_date(doc.get("delivery_date"))
    doc_flags: List[str] = doc.get("flags") or []

    for item in doc.get("items") or []:
        flags: List[str] = item.get("flags") or []

        # Unit normalisation
        unit, unit_raw, overridden = _normalise_unit(item.get("unit"))
        item["unit"] = unit
        if overridden:
            item["unit_raw"] = unit_raw or item.get("unit_raw")
            if "UNIT_OVERRIDDEN" not in flags:
                flags.append("UNIT_OVERRIDDEN")

        # Per-line delivery date
        ld = _parse_date(item.get("delivery_date"))
        item["delivery_date"] = ld
        effective_date = ld or doc_delivery
        if effective_date is None and "MISSING_DELIVERY_DATE" not in flags:
            flags.append("MISSING_DELIVERY_DATE")

        # use_date check
        if item.get("use_date"):
            item["use_date"] = _parse_date(item.get("use_date"))
        else:
            item["use_date"] = None

        # Status & addition defaults
        if item.get("status") not in ("active", "cancelled"):
            item["status"] = "active"
        item["is_addition"] = bool(item.get("is_addition", False))
        item.setdefault("item_name_normalized", None)
        item.setdefault("cost_center", None)

        # Weekday check on the effective date (line_note may carry the weekday)
        if effective_date:
            note = item.get("line_note") or ""
            if not _weekday_matches(effective_date, note):
                if "WEEKDAY_MISMATCH" not in flags:
                    flags.append("WEEKDAY_MISMATCH")
                if "WEEKDAY_MISMATCH" not in doc_flags:
                    doc_flags.append("WEEKDAY_MISMATCH")

        item["flags"] = _clean_flags(flags, _LINE_FLAGS)

    # Clean doc-level date fields
    doc["delivery_date"] = _parse_date(doc.get("delivery_date"))
    doc["order_date"] = _parse_date(doc.get("order_date"))
    doc["billing_date"] = _parse_date(doc.get("billing_date"))
    doc["flags"] = _clean_flags(doc_flags, _DOC_FLAGS)

    # Defaults for new document-level fields if not set
    doc.setdefault("document_type", "order")
    doc.setdefault("no_additions_confirmed", None)
    doc.setdefault("delivery_time_slot", None)
    doc.setdefault("billing_entity", None)
    doc.setdefault("branch_fax", None)
    doc.setdefault("sourcing_request", None)
    if "internal_annotations" not in doc or not isinstance(doc["internal_annotations"], list):
        doc["internal_annotations"] = []
    if "corrections" not in doc or not isinstance(doc["corrections"], list):
        doc["corrections"] = []
    if "merged_orders" not in doc or not isinstance(doc["merged_orders"], list):
        doc["merged_orders"] = []

    # form_template_id must be 8 digits
    tid = doc.get("form_template_id")
    if isinstance(tid, str) and not re.fullmatch(r"\d{8}", tid):
        doc["form_template_id"] = None

    # Validate order_lines if present (v3 master-matching output)
    if "order_lines" in doc:
        _validate_order_lines(doc)

    return doc


# ---------------------------------------------------------------------------
# Order-line validation (v3)
# ---------------------------------------------------------------------------
_REVIEW_TRIGGERS = {"UNMATCHED_ITEM", "UNMATCHED_CUSTOMER", "UNIT_MISMATCH", "QTY_OUTLIER"}


def _validate_order_lines(doc: dict) -> dict:
    """Validate and normalise the order_lines array (v3 master-matching output)."""
    lines = doc.get("order_lines")
    if not isinstance(lines, list):
        return doc
    for i, ol in enumerate(lines):
        if not isinstance(ol, dict):
            continue

        # 行番号: always 2-digit zero-padded text
        ol["行番号"] = str(i + 1).zfill(2)

        # Codes must remain strings
        for code_field in ("商品コード", "得意先コード"):
            v = ol.get(code_field)
            if isinstance(v, (int, float)):
                ol[code_field] = str(int(v))

        # match_confidence default
        if not isinstance(ol.get("match_confidence"), (int, float)):
            ol["match_confidence"] = 0.0
        ol.setdefault("match_method", "none")
        ol.setdefault("match_candidates", [])

        # Clean flags
        flags = _clean_flags(ol.get("flags") or [], _ORDER_LINE_FLAGS)

        # Auto-set NEEDS_HUMAN_REVIEW
        needs_review = (
            any(f in flags for f in _REVIEW_TRIGGERS)
            or float(ol.get("match_confidence") or 0) < 0.8
        )
        if needs_review and "NEEDS_HUMAN_REVIEW" not in flags:
            flags.append("NEEDS_HUMAN_REVIEW")

        ol["flags"] = flags

        # 園着日 defaults to 納品日付 when absent
        if not ol.get("園着日"):
            ol["園着日"] = ol.get("納品日付")

        # 締日 and 伝票番号 are set by the business system, never by OCR
        ol["締日"] = None
        ol["伝票番号"] = None

    doc["order_lines"] = lines
    return doc


# ---------------------------------------------------------------------------
# Master-data block builder
# ---------------------------------------------------------------------------
def _build_master_blocks(master_data: dict) -> str:
    """Serialise runtime master tables into XML-like blocks for the user message."""
    parts: List[str] = []
    for tag, key in [
        ("product_master", "product_master"),
        ("center_master", "center_master"),
        ("customer_master", "customer_master"),
        ("order_history", "order_history"),
    ]:
        data = master_data.get(key)
        if data is None:
            continue
        body = json.dumps(data, ensure_ascii=False, indent=None) if not isinstance(data, str) else data
        parts.append(f"<{tag}>\n{body}\n</{tag}>")
    return "\n\n".join(parts)


# ---------------------------------------------------------------------------
# LLM extraction path
# ---------------------------------------------------------------------------
def extract_with_llm(
    img_bytes: bytes,
    *,
    media_type: str = "image/jpeg",
    master_data: Optional[dict] = None,
) -> dict:
    """Send *img_bytes* to Claude and return the parsed extraction dict.

    Raises ``LLMUnavailableError`` when ``ANTHROPIC_API_KEY`` is not set.
    Raises ``json.JSONDecodeError`` if the model response is not valid JSON.
    """
    from ..config import settings
    key = settings.anthropic_api_key
    if not key:
        raise LLMUnavailableError(
            "ANTHROPIC_API_KEY is not set. Set it in the environment or .env to enable "
            "Claude-vision extraction."
        )

    import anthropic  # imported late so the module loads without anthropic installed
    client = anthropic.Anthropic(api_key=key)

    b64 = base64.standard_b64encode(img_bytes).decode()

    # Build user text: optional master-data blocks followed by the instruction
    master_blocks = _build_master_blocks(master_data) if master_data else ""
    instruction = (
        "この発注書から注文情報をすべて抽出し、指定のJSONスキーマで返してください。"
        "JSONのみ出力してください。"
    )
    if master_blocks:
        instruction = master_blocks + "\n\n" + instruction

    response = client.messages.create(
        model=MODEL,
        max_tokens=MAX_TOKENS,
        system=_system_prompt(),
        messages=[
            {
                "role": "user",
                "content": [
                    {
                        "type": "image",
                        "source": {
                            "type": "base64",
                            "media_type": media_type,
                            "data": b64,
                        },
                    },
                    {"type": "text", "text": instruction},
                ],
            }
        ],
    )

    text = next((b.text for b in response.content if b.type == "text"), "")
    # Strip any accidental markdown fences
    text = re.sub(r"^```(?:json)?\s*", "", text.strip())
    text = re.sub(r"\s*```$", "", text)

    doc = json.loads(text)
    return _post_check(doc)


# ---------------------------------------------------------------------------
# Local adapter: FormResult → extraction schema
# ---------------------------------------------------------------------------
def local_to_extract(form_result, *, catalog: str = "", delivery_date: Optional[str] = None) -> dict:
    """Convert a ``forms.FormResult`` or ``natori.FormResult`` to the extraction JSON schema.

    This allows callers to get a consistent output shape even when the LLM is unavailable.
    Confidence and flags are mapped from the cell-grid reader's ``Row`` fields.
    """
    doc_flags: List[str] = []
    if not form_result.rows:
        doc_flags.append("NEEDS_HUMAN_REVIEW")

    items: List[dict] = []
    for row in form_result.rows:
        qty_str = (row.quantity or "").strip()
        try:
            qty = float(qty_str) if qty_str else None
            if qty is not None and qty == int(qty):
                qty = int(qty)
        except ValueError:
            qty = None

        note_text = row.note or ""
        add_qty = None
        if note_text.startswith("追加 "):
            try:
                add_qty = int(note_text.split()[1])
                note_text = ""
            except (IndexError, ValueError):
                pass

        line_flags: List[str] = []
        if row.review:
            line_flags.append("DIGIT_AMBIGUOUS" if row.handwritten else "ROW_AMBIGUOUS")

        unit_norm, unit_raw, overridden = _normalise_unit(row.unit)
        if overridden:
            line_flags.append("UNIT_OVERRIDDEN")

        eff_delivery = _parse_date(delivery_date)
        if eff_delivery is None:
            line_flags.append("MISSING_DELIVERY_DATE")

        is_cancelled = bool(note_text and ("キャンセル" in note_text or "取消" in note_text))
        items.append({
            "item_name": row.name or "",
            "item_name_normalized": None,
            "spec": None,
            "quantity": qty,
            "additional_quantity": add_qty,
            "unit": unit_norm,
            "unit_raw": unit_raw,
            "status": "cancelled" if is_cancelled else "active",
            "is_addition": bool(add_qty and not qty),
            "delivery_date": eff_delivery,
            "use_date": None,
            "cost_center": None,
            "line_note": note_text or None,
            "unit_price": None,
            "confidence": int(row.confidence),
            "flags": line_flags,
        })

    doc = {
        "document_type": "order",
        "supplier": None,
        "customer_name": None,
        "customer_code": None,
        "contact_person": None,
        "order_date": None,
        "delivery_date": _parse_date(delivery_date),
        "delivery_time_slot": None,
        "delivery_location": None,
        "billing_entity": None,
        "branch_fax": None,
        "sourcing_request": None,
        "no_additions_confirmed": None,
        "form_template_id": None,
        "notes": None,
        "internal_annotations": [],
        "corrections": [],
        "fax_received_at": None,
        "change_type": None,
        "billing_date": None,
        "flags": doc_flags,
        "merged_orders": [],
        "items": items,
        "_source": "local",
        "_catalog": catalog or None,
    }
    return _post_check(doc)


# ---------------------------------------------------------------------------
# Availability check
# ---------------------------------------------------------------------------
def llm_available() -> bool:
    """True when ANTHROPIC_API_KEY is set and anthropic package is importable."""
    try:
        from ..config import settings  # noqa: F401
        import anthropic  # noqa: F401
        return bool(settings.anthropic_api_key)
    except Exception:
        return False
