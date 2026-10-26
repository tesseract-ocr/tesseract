"""User-owned labelled datasets: create / edit / delete samples, import and export as a zip, and save corrections from an extraction.

Sample = ONE line of text: a cropped image (8-400 px high) plus exactly what it says. That is what the fine-tuning step trains on.
Zip format (import AND export, so a dataset round-trips):
    labels.tsv      one sample per line:  images/sample_1.png <TAB> the exact text      (labels.csv with a comma also works)
    images/...      the image files
    dataset.json    optional: {"name", "category", "lang", "description"}
"""
from __future__ import annotations

import csv
import io
import json
import posixpath
import shutil
import zipfile
from pathlib import Path
from typing import List, Optional, Tuple

import cv2
import numpy as np
from PIL import Image
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import Dataset, DatasetSample, User
from ..services.samples import clean_label

MAX_DATASETS = 20
MAX_SAMPLES = 500
MAX_IMAGE_BYTES = 2 * 1024 * 1024
MAX_ZIP_ENTRIES = 1500
MAX_ZIP_UNCOMPRESSED = 100 * 1024 * 1024
LINE_MAX_HEIGHT = 100  # corrections cropped from upscaled images are shrunk to this height
CATEGORY_HINTS = ["handwriting", "printed", "blurry", "receipts", "forms", "general"]


def root(dataset_id: int) -> Path:
    d = settings.storage_dir / "datasets" / str(dataset_id)
    d.mkdir(parents=True, exist_ok=True)
    return d


def owned(db: Session, dataset_id: int, user: User) -> Dataset:
    ds = db.get(Dataset, dataset_id)
    if not ds or ds.user_id != user.id:  # a foreign dataset looks exactly like a missing one
        raise ApiError(404, "dataset_not_found", "Dataset not found")
    return ds


def out(ds: Dataset, counts: Optional[dict] = None) -> dict:
    active = next((m for m in ds.models if m.active), None)
    return {"id": ds.id, "name": ds.name, "category": ds.category, "description": ds.description, "lang": ds.lang,
            "sample_count": (counts or {}).get(ds.id, len(ds.samples)), "created_at": ds.created_at, "updated_at": ds.updated_at,
            "active_model": model_out(active) if active else None}


def model_out(m) -> dict:
    return {"id": m.id, "version": m.version, "status": m.status, "active": m.active, "cer_before": m.cer_before, "cer_after": m.cer_after,
            "train_samples": m.train_samples, "test_samples": m.test_samples, "iterations": m.iterations, "message": m.message,
            "created_at": m.created_at, "finished_at": m.finished_at}


def sample_out(s: DatasetSample) -> dict:
    return {"id": s.id, "text": s.text, "original_text": s.original_text, "source": s.source, "created_at": s.created_at, "updated_at": s.updated_at}


def create(db: Session, user: User, name: str, category: str, lang: str, description: str = "") -> Dataset:
    name, category = name.strip(), (category or "general").strip().lower()[:40] or "general"
    if lang not in settings.trainable_list:
        raise ApiError(422, "invalid_lang", f"lang must be one of: {', '.join(settings.trainable_list)}")
    if db.scalar(select(func.count()).select_from(Dataset).where(Dataset.user_id == user.id)) >= MAX_DATASETS:
        raise ApiError(409, "too_many_datasets", f"You can have at most {MAX_DATASETS} datasets")
    if db.scalar(select(Dataset).where(Dataset.user_id == user.id, Dataset.name == name)):
        raise ApiError(409, "dataset_exists", f"You already have a dataset called '{name}'")
    ds = Dataset(user_id=user.id, name=name, category=category, lang=lang, description=description.strip()[:500])
    db.add(ds)
    db.commit()
    return ds


def delete(db: Session, ds: Dataset) -> None:
    shutil.rmtree(settings.storage_dir / "datasets" / str(ds.id), ignore_errors=True)
    shutil.rmtree(settings.storage_dir / "dataset_models" / str(ds.id), ignore_errors=True)
    db.delete(ds)
    db.commit()


# ---- samples --------------------------------------------------------------------------------------------------

def _decode_line(data: bytes) -> np.ndarray:
    if len(data) > MAX_IMAGE_BYTES:
        raise ApiError(413, "image_too_large", f"Sample images can be at most {MAX_IMAGE_BYTES // 1024 // 1024} MB")
    # Read the DECLARED size from the header first. A 100 KB PNG can claim 20000x20000 pixels and use gigabytes of RAM when decoded.
    try:
        with Image.open(io.BytesIO(data)) as probe:
            w, h = probe.size
    except Image.DecompressionBombError:  # Pillow's own guard for absurd sizes (> ~178 megapixels)
        raise ApiError(422, "invalid_image", "A sample must be a single line of text: at most 400 px high and 4000 px wide")
    except Exception:  # noqa: BLE001
        raise ApiError(422, "unreadable_image", "The sample image could not be read (PNG, JPEG, BMP, TIFF or WEBP)")
    if h > 400 or w > 4000:
        raise ApiError(422, "invalid_image", "A sample must be a single line of text: at most 400 px high and 4000 px wide")
    arr = cv2.imdecode(np.frombuffer(data, np.uint8), cv2.IMREAD_GRAYSCALE)
    if arr is None:
        raise ApiError(422, "unreadable_image", "The sample image could not be read (PNG, JPEG, BMP, TIFF or WEBP)")
    h, w = arr.shape
    if h < 8 or w < 8:
        raise ApiError(422, "invalid_image", "The sample image is too small (at least 8x8 px)")
    if h > 400 or w > 4000:
        raise ApiError(422, "invalid_image", "A sample must be a single line of text: at most 400 px high and 4000 px wide")
    return arr


def add_sample(db: Session, ds: Dataset, image: "bytes | np.ndarray", text: str, source: str = "upload", original: Optional[str] = None) -> DatasetSample:
    if db.scalar(select(func.count()).select_from(DatasetSample).where(DatasetSample.dataset_id == ds.id)) >= MAX_SAMPLES:
        raise ApiError(409, "dataset_full", f"A dataset can hold at most {MAX_SAMPLES} samples")
    label = clean_label(text)
    gray = _decode_line(image) if isinstance(image, (bytes, bytearray)) else image
    s = DatasetSample(dataset_id=ds.id, image_path="", text=label, source=source, original_text=original)
    db.add(s)
    db.flush()
    path = root(ds.id) / f"{s.id}.png"
    cv2.imwrite(str(path), gray)
    s.image_path = str(path)
    ds.updated_at = s.created_at
    db.commit()
    return s


def crop_line(used_gray: np.ndarray, bbox: List[int]) -> np.ndarray:
    """A training sample from a line box of an extraction: a little padding, shrunk if the (enhanced) image was upscaled."""
    l, t, r, b = bbox
    pad = max(4, int((b - t) * 0.15))
    crop = used_gray[max(t - pad, 0):min(b + pad, used_gray.shape[0]), max(l - pad, 0):min(r + pad, used_gray.shape[1])]
    if crop.size == 0:
        raise ApiError(422, "bad_line", "That line has no image area")
    if crop.shape[0] > LINE_MAX_HEIGHT:
        s = LINE_MAX_HEIGHT / crop.shape[0]
        crop = cv2.resize(crop, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
    return crop


def remove_sample(db: Session, s: DatasetSample) -> None:
    Path(s.image_path).unlink(missing_ok=True)
    db.delete(s)
    db.commit()


# ---- zip import / export ----------------------------------------------------------------------------------------

def _parse_labels(raw: str) -> List[Tuple[str, str]]:
    """`path<TAB>text` or CSV `path,text` (commas inside the text are fine when quoted, or when they are all after the first comma)."""
    rows = []
    for line in raw.splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        if "\t" in line:
            path, _, text = line.partition("\t")
        else:
            row = next(csv.reader([line]), [])
            if len(row) < 2:
                continue
            path, text = row[0], ",".join(row[1:])
        rows.append((path.strip(), text.strip()))
    return rows


def read_zip(data: bytes) -> Tuple[zipfile.ZipFile, List[Tuple[str, str]], dict]:
    """Opens an uploaded zip safely and returns (zip, [(path, text)], dataset.json)."""
    try:
        z = zipfile.ZipFile(io.BytesIO(data))
        infos = z.infolist()
    except zipfile.BadZipFile:
        raise ApiError(422, "bad_zip", "That file is not a valid zip archive")
    if len(infos) > MAX_ZIP_ENTRIES:
        raise ApiError(413, "zip_too_many_files", f"The zip has more than {MAX_ZIP_ENTRIES} entries")
    if sum(i.file_size for i in infos) > MAX_ZIP_UNCOMPRESSED:  # declared sizes, checked BEFORE reading anything (zip-bomb guard)
        raise ApiError(413, "zip_too_large", "The zip would unpack to more than 100 MB")
    names = {posixpath.normpath(i.filename): i for i in infos if not i.is_dir()}
    label_file = next((n for n in names if posixpath.basename(n).lower() in ("labels.tsv", "labels.csv", "labels.txt")), None)
    if not label_file:
        raise ApiError(422, "no_labels", "The zip needs a labels.tsv (or labels.csv): one line per sample: image path, then the text")
    rows = _parse_labels(z.read(label_file).decode("utf-8-sig", errors="replace"))
    if rows and rows[0][0] not in names and posixpath.join(posixpath.dirname(label_file), rows[0][0]) not in names:
        rows = rows[1:]  # a header row such as "filename,text"
    meta = {}
    meta_file = next((n for n in names if posixpath.basename(n).lower() == "dataset.json"), None)
    if meta_file:
        try:
            meta = json.loads(z.read(meta_file))
        except Exception:  # noqa: BLE001
            meta = {}
    base = posixpath.dirname(label_file)
    return z, [(posixpath.normpath(posixpath.join(base, p)), t) for p, t in rows], meta


def import_zip(db: Session, ds: Dataset, data: bytes) -> dict:
    z, rows, _ = read_zip(data)
    names = {posixpath.normpath(i.filename) for i in z.infolist()}
    added, skipped = 0, []
    for path, text in rows:
        try:
            if path.startswith("/") or path.startswith("..") or ".." in path.split("/"):
                raise ApiError(422, "unsafe_path", "unsafe path")
            if path not in names:
                raise ApiError(422, "missing_image", "image not in the zip")
            add_sample(db, ds, z.read(path), text, source="import")
            added += 1
        except ApiError as e:
            skipped.append({"file": path, "reason": e.detail["message"] if isinstance(e.detail, dict) else str(e.detail)})
            if e.status_code == 409:  # dataset full: stop, the rest would all fail the same way
                skipped.extend({"file": p, "reason": "dataset is full"} for p, _ in rows[rows.index((path, text)) + 1:])
                break
    return {"added": added, "skipped": skipped[:50], "skipped_count": len(skipped)}


def export_zip(ds: Dataset) -> bytes:
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as z:
        labels = []
        for s in ds.samples:
            p = Path(s.image_path)
            if not p.exists():
                continue
            arc = f"images/sample_{s.id}.png"
            z.write(p, arc)
            labels.append(f"{arc}\t{s.text}")
        z.writestr("labels.tsv", "\n".join(labels) + ("\n" if labels else ""))
        z.writestr("dataset.json", json.dumps({"name": ds.name, "category": ds.category, "lang": ds.lang, "description": ds.description}, ensure_ascii=False, indent=2))
    return buf.getvalue()
