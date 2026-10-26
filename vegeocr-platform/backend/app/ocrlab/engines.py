"""Recognition engines behind one interface. Add a new engine by writing a class with `name`, `available()` and `recognise()`.

  tesseract   printed text, all supported languages, and the only engine here that can be fine-tuned on your own data.
  trocr       English handwriting (print + cursive), line by line, with a per-line probability. Optional heavy dependency.
"""
from __future__ import annotations

import importlib.util
import os
import tempfile
import threading
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional

import cv2
import numpy as np

from ..services import ocr as tess
from . import linesplit

TROCR_MODEL = os.environ.get("TROCR_MODEL", "microsoft/trocr-base-handwritten")
TROCR_ALLOW_DOWNLOAD = os.environ.get("LAB_TROCR_ALLOW_DOWNLOAD", "0") == "1"  # downloading ~1.3 GB inside a request is never what you want


@dataclass
class Line:
    text: str
    confidence: float                       # 0-100
    bbox: List[int]                         # left, top, right, bottom in the recognised image
    words: List[dict] = field(default_factory=list)


class EngineError(Exception):
    """The engine could not read the image (as opposed to reading it and finding no text)."""


# ---- Tesseract --------------------------------------------------------------------------------------------------

def tsv_to_lines(tsv: str) -> List[Line]:
    rows = {}
    for row in tsv.splitlines()[1:]:
        c = row.split("\t")
        if len(c) < 12 or c[0] != "5":
            continue
        conf, text = float(c[10]), c[11].strip()
        if not text or conf < 0:
            continue
        l, t, w, h = int(c[6]), int(c[7]), int(c[8]), int(c[9])
        rows.setdefault(tuple(c[2:5]), []).append({"text": text, "confidence": round(conf, 1), "bbox": [l, t, l + w, t + h]})
    lines = []
    for key in sorted(rows, key=lambda k: tuple(int(x) for x in k)):
        ws = rows[key]
        text = tess.clean_text(" ".join(w["text"] for w in ws))
        if text:
            lines.append(Line(text, round(sum(w["confidence"] for w in ws) / len(ws), 1),
                              [min(w["bbox"][0] for w in ws), min(w["bbox"][1] for w in ws), max(w["bbox"][2] for w in ws), max(w["bbox"][3] for w in ws)], ws))
    return lines


class TesseractEngine:
    name = "tesseract"
    cheap = True    # fast enough to read an image twice (original vs enhanced) and keep the better

    def available(self) -> bool:
        from ..config import settings
        import shutil
        return bool(shutil.which(settings.tesseract_bin) or Path(settings.tesseract_bin).exists())

    def recognise(self, gray: np.ndarray, lang: str, psm: int = 3, tessdata: Optional[Path] = None, **_) -> List[Line]:
        with tempfile.NamedTemporaryFile(suffix=".png") as f:
            cv2.imwrite(f.name, gray)
            try:
                tsv = tess.run_tesseract(f.name, lang, psm=psm, tessdata=tessdata, config="tsv")
            except Exception as e:  # noqa: BLE001
                raise EngineError(f"Tesseract failed: {str(e)[:300]}") from e
        return tsv_to_lines(tsv)


# ---- TrOCR (handwriting) ----------------------------------------------------------------------------------------

class TrOCREngine:
    name = "trocr"
    cheap = False
    _lock = threading.Lock()
    _model = None
    _processor = None

    def available(self) -> bool:
        return self.unavailable_reason() is None

    def unavailable_reason(self) -> Optional[str]:
        if importlib.util.find_spec("torch") is None or importlib.util.find_spec("transformers") is None:
            return "TrOCR is not installed on this server (pip install -r requirements-handwriting.txt; needs about 2 GB of RAM)"
        if not TROCR_ALLOW_DOWNLOAD and not self._cached():
            return f"The TrOCR model '{TROCR_MODEL}' has not been downloaded (python scripts/download_trocr.py)"
        return None

    @staticmethod
    def _cached() -> bool:
        try:
            from huggingface_hub import try_to_load_from_cache
            return isinstance(try_to_load_from_cache(TROCR_MODEL, "config.json"), str)
        except Exception:  # noqa: BLE001
            return False

    @classmethod
    def _load(cls):
        with cls._lock:
            if cls._model is None:
                import torch
                from transformers import TrOCRProcessor, VisionEncoderDecoderModel
                local = not TROCR_ALLOW_DOWNLOAD
                cls._processor = TrOCRProcessor.from_pretrained(TROCR_MODEL, local_files_only=local)
                cls._model = VisionEncoderDecoderModel.from_pretrained(TROCR_MODEL, local_files_only=local).eval()
                torch.set_num_threads(max(1, min(4, os.cpu_count() or 1)))
        return cls._model, cls._processor

    def recognise(self, gray: np.ndarray, lang: str = "eng", batch: int = 4, **_) -> List[Line]:
        reason = self.unavailable_reason()
        if reason:
            raise EngineError(reason)
        import torch
        from PIL import Image
        model, proc = self._load()
        lines = linesplit.find_lines(gray) or [(0, 0, gray.shape[1], gray.shape[0])]  # no layout found: treat the whole image as one line
        chunks = [(i, c) for i, box in enumerate(lines) for c in linesplit.split_wide(gray, box)]
        texts, confs = [], []
        for s in range(0, len(chunks), batch):
            crops = [Image.fromarray(gray[t:b, l:r]).convert("RGB") for _, (l, t, r, b) in chunks[s:s + batch]]
            with torch.no_grad():
                pix = proc(images=crops, return_tensors="pt").pixel_values
                out = model.generate(pix, max_new_tokens=64, num_beams=1, output_scores=True, return_dict_in_generate=True)
                logp = model.compute_transition_scores(out.sequences, out.scores, normalize_logits=True)  # (B, T) log-prob of each chosen token
            seqs = out.sequences[:, 1:] if out.sequences.shape[1] > logp.shape[1] else out.sequences
            special = {proc.tokenizer.pad_token_id, proc.tokenizer.eos_token_id, proc.tokenizer.bos_token_id}
            for k, text in enumerate(proc.batch_decode(out.sequences, skip_special_tokens=True)):
                keep = [j for j in range(logp.shape[1]) if int(seqs[k, j]) not in special]
                texts.append(text.strip())
                confs.append(float(torch.exp(logp[k, keep].mean())) * 100 if keep else 0.0)  # geometric-mean token probability
        by_line: dict = {}
        for (i, box), text, conf in zip(chunks, texts, confs):
            by_line.setdefault(i, []).append((box, text, conf))
        result = []
        for i in sorted(by_line):
            parts = [p for p in by_line[i] if p[1]]
            if not parts:
                continue
            box = lines[i]
            result.append(Line(" ".join(p[1] for p in parts), round(min(p[2] for p in parts), 1), list(box),  # a line is only as sure as its weakest chunk
                               [{"text": p[1], "confidence": round(p[2], 1), "bbox": list(p[0])} for p in parts]))
        return result


ENGINES = {"tesseract": TesseractEngine(), "trocr": TrOCREngine()}
