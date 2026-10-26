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
UNIT_MAP = {"ケ": "個", "ヶ": "個", "个": "個"}            # raw → normalised

_DOC_FLAGS = {
    "DUPLICATE", "PAGE_CROSSED_OUT", "LOW_IMAGE_QUALITY", "NEEDS_HUMAN_REVIEW",
    "ILLEGIBLE", "AMBIGUOUS_DATE", "MISSING_DELIVERY_DATE", "DATE_CORRECTED",
    "WEEKDAY_MISMATCH",
}
_LINE_FLAGS = {
    "DIGIT_AMBIGUOUS", "ROW_AMBIGUOUS", "UNIT_OVERRIDDEN", "QUANTITY_CORRECTED",
    "DATE_CORRECTED", "WEEKDAY_MISMATCH", "MISSING_DELIVERY_DATE", "AMBIGUOUS_DATE",
    "ILLEGIBLE",
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
    """Return (normalised_unit, unit_raw_if_changed, overridden)."""
    if not isinstance(raw, str) or not raw.strip():
        return raw, None, False
    n = UNIT_MAP.get(raw.strip(), raw.strip())
    if n != raw.strip():
        return n, raw.strip(), True
    return raw.strip(), None, False


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
            item["unit_raw"] = unit_raw
            if "UNIT_OVERRIDDEN" not in flags:
                flags.append("UNIT_OVERRIDDEN")

        # Per-line delivery date
        ld = _parse_date(item.get("delivery_date"))
        item["delivery_date"] = ld
        effective_date = ld or doc_delivery
        if effective_date is None and "MISSING_DELIVERY_DATE" not in flags:
            flags.append("MISSING_DELIVERY_DATE")

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

    # form_template_id must be 8 digits
    tid = doc.get("form_template_id")
    if isinstance(tid, str) and not re.fullmatch(r"\d{8}", tid):
        doc["form_template_id"] = None

    return doc


# ---------------------------------------------------------------------------
# LLM extraction path
# ---------------------------------------------------------------------------
def extract_with_llm(img_bytes: bytes, *, media_type: str = "image/jpeg") -> dict:
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
                    {
                        "type": "text",
                        "text": (
                            "この発注書から注文情報をすべて抽出し、指定のJSONスキーマで返してください。"
                            "JSONのみ出力してください。"
                        ),
                    },
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

        items.append({
            "item_name": row.name or "",
            "spec": None,
            "quantity": qty,
            "additional_quantity": add_qty,
            "unit": unit_norm,
            "unit_raw": unit_raw,
            "delivery_date": eff_delivery,
            "line_note": note_text or None,
            "unit_price": None,
            "confidence": int(row.confidence),
            "flags": line_flags,
        })

    doc = {
        "supplier": None,
        "customer_name": None,
        "customer_code": None,
        "contact_person": None,
        "order_date": None,
        "delivery_date": _parse_date(delivery_date),
        "delivery_location": None,
        "form_template_id": None,
        "notes": None,
        "fax_received_at": None,
        "change_type": None,
        "billing_date": None,
        "flags": doc_flags,
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
