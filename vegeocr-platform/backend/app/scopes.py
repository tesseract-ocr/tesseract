"""API-key scopes. A key can only do what its scopes allow, regardless of who owns it."""
from __future__ import annotations

from typing import Dict, Iterable, List, Optional

SCOPES: Dict[str, str] = {
    "read": "Read OCR jobs and this key's usage (GET /api/v1/jobs/{id}, /api/v1/usage)",
    "write": "Run OCR and submit training data (POST /api/v1/ocr, /api/v1/training-data)",
    "learning:read": "Read learning modules, lessons and your progress",
    "learning:write": "Update lesson progress and submit quiz answers",
    "admin": "Full access: implies every other scope. Only administrators can create these keys",
}
ALL_SCOPES: List[str] = list(SCOPES)
DEFAULT_SCOPES: List[str] = ["read", "write"]  # what keys created before scopes existed keep


def expand(granted: Optional[Iterable[str]]) -> set:
    """`admin` implies everything. Keys without a stored scope list (legacy rows) get the default."""
    s = set(granted) if granted else set(DEFAULT_SCOPES)
    return set(ALL_SCOPES) if "admin" in s else s


def missing(granted: Optional[Iterable[str]], needed: Iterable[str]) -> List[str]:
    have = expand(granted)
    return [n for n in needed if n not in have]
