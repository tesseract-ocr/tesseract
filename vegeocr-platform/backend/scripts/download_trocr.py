"""Download the TrOCR handwriting model once, so requests never block on a 1.3 GB download.
    python scripts/download_trocr.py [model_id]       (default: $TROCR_MODEL or microsoft/trocr-base-handwritten; trocr-small-handwritten is ~4x smaller)"""
import os
import sys

from huggingface_hub import snapshot_download

model = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("TROCR_MODEL", "microsoft/trocr-base-handwritten")
path = snapshot_download(model, allow_patterns=["*.json", "*.safetensors", "*.txt", "*.model"])  # skip duplicate .bin/.h5 weights
print("downloaded", model, "->", path)
