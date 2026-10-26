from __future__ import annotations

import io
import uuid
from typing import Optional

from PIL import Image
from sqlalchemy.orm import Session

from ..config import settings
from ..errors import ApiError
from ..models import TrainingSample
from .jobs import ALLOWED_LANGS


def train_lang_for(job_lang: str) -> Optional[str]:
    """jpn+eng -> jpn: the first language in the combination that we can fine-tune."""
    for p in job_lang.split("+"):
        if p in settings.trainable_list:
            return p
    return None


def save_line_image(img: Image.Image) -> str:
    path = settings.storage_dir / "training" / f"{uuid.uuid4().hex}.png"
    img.convert("L").save(path)
    return str(path)


def crop_line(page_image: str, bbox, pad: int = 6) -> Image.Image:
    with Image.open(page_image) as im:
        l, t, r, b = bbox
        return im.crop((max(l - pad, 0), max(t - pad, 0), min(r + pad, im.width), min(b + pad, im.height))).copy()


def add_sample(db: Session, *, source: str, lang: str, image_path: str, text: str, original: Optional[str] = None,
               user_id=None, api_key_id=None, job_id=None) -> TrainingSample:
    s = TrainingSample(source=source, lang=lang, image_path=image_path, text=text, original_text=original,
                       user_id=user_id, api_key_id=api_key_id, job_id=job_id)
    db.add(s)
    db.commit()
    return s


def clean_label(text: str) -> str:
    t = " ".join((text or "").split())  # single line, no tabs/newlines
    if not t or len(t) > 300:
        raise ApiError(422, "invalid_text", "text must be 1-300 characters on a single line")
    return t


def check_line_image(data: bytes) -> Image.Image:
    try:
        im = Image.open(io.BytesIO(data))
        im.load()
    except Exception:
        raise ApiError(400, "corrupt_file", "Could not read the image")
    if im.height > 400 or im.width > 4000 or im.height < 8 or im.width < 8:
        raise ApiError(422, "invalid_image", "Training images must be a single text line (8-400 px high, up to 4000 px wide)")
    return im
