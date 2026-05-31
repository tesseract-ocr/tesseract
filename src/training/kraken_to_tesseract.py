#!/usr/bin/env python3
"""Best-effort converter from kraken .mlmodel to Tesseract-compatible assets.

The script extracts model metadata/state from a kraken model, maps the VGSL
network string to Tesseract-supported layers where possible, and exports a
conversion bundle for later assembly into a Tesseract model.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import numpy as np

try:
    import torch
except ImportError as exc:  # pragma: no cover - runtime dependency
    raise SystemExit("Missing dependency 'torch'. Install: pip install torch") from exc


UNSUPPORTED_KRAKEN_PREFIXES = {
    "Do": "Dropout layers are inference no-ops and are removed by default.",
    "Bn": "Batch normalization layers are not supported by Tesseract VGSL.",
    "Gr": "GroupNorm layers are not supported by Tesseract VGSL.",
    "A": "Kraken standalone activation layers are not supported by Tesseract VGSL.",
    "Lpa": "Kraken attention LSTM variants are not supported by Tesseract VGSL.",
}

SUPPORTED_TESSERACT_LAYER_PREFIXES = {
    "1", "S", "P", "A", "L", "R", "M", "F", "O", "C", "T"
}


@dataclass
class UnsupportedLayer:
    token: str
    reason: str


class ConversionError(RuntimeError):
    """Raised when conversion cannot continue."""


def _tokenize_vgsl(spec: str) -> list[str]:
    raw = spec.strip()
    if raw.startswith("[") and raw.endswith("]"):
        raw = raw[1:-1]
    return [part for part in raw.split() if part]


def _find_first_key(payload: Any, keys: set[str]) -> Any:
    if isinstance(payload, dict):
        for key, value in payload.items():
            if str(key).lower() in keys:
                return value
            nested = _find_first_key(value, keys)
            if nested is not None:
                return nested
    elif isinstance(payload, (list, tuple)):
        for item in payload:
            nested = _find_first_key(item, keys)
            if nested is not None:
                return nested
    return None


def _find_vgsl_spec(payload: Any) -> str:
    spec = _find_first_key(payload, {"vgsl", "vgsl_spec", "network", "network_spec", "spec"})
    if not isinstance(spec, str) or "[" not in spec:
        raise ConversionError("Could not locate VGSL specification in kraken model metadata")
    return spec


def _find_state_dict(payload: Any) -> dict[str, Any]:
    state_dict = _find_first_key(payload, {"state_dict", "model_state_dict", "weights"})
    if isinstance(state_dict, dict) and state_dict:
        return state_dict
    if isinstance(payload, dict) and payload and all(hasattr(v, "detach") for v in payload.values()):
        return payload
    raise ConversionError("Could not locate state_dict in kraken model")


def _map_vgsl_layers(tokens: list[str], keep_dropout: bool) -> tuple[list[str], list[str], list[UnsupportedLayer]]:
    mapped: list[str] = []
    notes: list[str] = []
    unsupported: list[UnsupportedLayer] = []

    for token in tokens:
        if token.startswith("Do"):
            if keep_dropout:
                unsupported.append(UnsupportedLayer(token, UNSUPPORTED_KRAKEN_PREFIXES["Do"]))
            else:
                notes.append(f"Removed dropout layer '{token}'")
            continue
        if token.startswith("Lpa"):
            unsupported.append(UnsupportedLayer(token, UNSUPPORTED_KRAKEN_PREFIXES["Lpa"]))
            continue
        if token.startswith("Bn"):
            unsupported.append(UnsupportedLayer(token, UNSUPPORTED_KRAKEN_PREFIXES["Bn"]))
            continue
        if token.startswith("Gr"):
            unsupported.append(UnsupportedLayer(token, UNSUPPORTED_KRAKEN_PREFIXES["Gr"]))
            continue
        if token.startswith("A") and (len(token) == 1 or not token[1].isdigit()):
            unsupported.append(UnsupportedLayer(token, UNSUPPORTED_KRAKEN_PREFIXES["A"]))
            continue

        if token[0] not in SUPPORTED_TESSERACT_LAYER_PREFIXES:
            unsupported.append(UnsupportedLayer(token, "Unknown/unsupported layer prefix for Tesseract VGSL"))
            continue

        mapped.append(token)

    return mapped, notes, unsupported


def _to_numpy_state_dict(state_dict: dict[str, Any]) -> dict[str, np.ndarray]:
    converted: dict[str, np.ndarray] = {}
    for key, value in state_dict.items():
        if not hasattr(value, "detach"):
            continue
        tensor = value.detach().cpu().numpy()
        converted[str(key)] = tensor.astype(np.float32, copy=False)
    if not converted:
        raise ConversionError("State dict does not contain tensor weights")
    return converted


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path, help="Input kraken .mlmodel path")
    parser.add_argument(
        "--output_prefix",
        required=True,
        type=Path,
        help="Output prefix for generated files (without extension)",
    )
    parser.add_argument(
        "--keep_dropout",
        action="store_true",
        help="Keep dropout layers as unsupported errors instead of dropping them",
    )
    parser.add_argument(
        "--allow_unsupported",
        action="store_true",
        help="Write conversion bundle even if unsupported layers are present",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if not args.input.is_file():
        raise SystemExit(f"Input model does not exist: {args.input}")

    model_obj = torch.load(args.input, map_location="cpu")
    original_spec = _find_vgsl_spec(model_obj)
    state_dict = _find_state_dict(model_obj)
    mapped_layers, mapping_notes, unsupported = _map_vgsl_layers(
        _tokenize_vgsl(original_spec),
        keep_dropout=args.keep_dropout,
    )

    if unsupported and not args.allow_unsupported:
        details = "\n".join(f"  - {item.token}: {item.reason}" for item in unsupported)
        raise SystemExit(
            "Unsupported kraken VGSL layers found. Use --allow_unsupported to emit a partial conversion bundle.\n"
            f"{details}"
        )

    output_prefix = args.output_prefix
    output_prefix.parent.mkdir(parents=True, exist_ok=True)

    mapped_spec = "[" + " ".join(mapped_layers) + "]"
    (output_prefix.with_suffix(".network_spec")).write_text(mapped_spec + "\n", encoding="utf-8")

    np.savez_compressed(output_prefix.with_suffix(".weights.npz"), **_to_numpy_state_dict(state_dict))

    unsupported_payload = [{"token": item.token, "reason": item.reason} for item in unsupported]
    summary = {
        "input_model": str(args.input),
        "original_vgsl": original_spec,
        "mapped_vgsl": mapped_spec,
        "mapping_notes": mapping_notes,
        "unsupported_layers": unsupported_payload,
        "tesseract_binary_serialization": (
            "not generated by this script yet; use exported VGSL + weights bundle as input for a"
            " serializer bridge to produce a .lstm component"
        ),
    }
    (output_prefix.with_suffix(".conversion.json")).write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    print(f"Wrote {output_prefix.with_suffix('.network_spec')}")
    print(f"Wrote {output_prefix.with_suffix('.weights.npz')}")
    print(f"Wrote {output_prefix.with_suffix('.conversion.json')}")

    if unsupported:
        print("Completed partial conversion with unsupported layers:")
        for item in unsupported:
            print(f"  - {item.token}: {item.reason}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
