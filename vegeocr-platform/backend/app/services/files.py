from __future__ import annotations

import io

import fitz
from PIL import Image

from ..config import settings
from ..errors import ApiError

IMAGE_MAGIC = [
    (b"\x89PNG\r\n\x1a\n", "image/png", ".png"),
    (b"\xff\xd8\xff", "image/jpeg", ".jpg"),
    (b"II*\x00", "image/tiff", ".tif"),
    (b"MM\x00*", "image/tiff", ".tif"),
    (b"BM", "image/bmp", ".bmp"),
]
MAX_PIXELS = 100_000_000


def sniff(data: bytes):
    """Identify the file by its content, not by the client-supplied name or content type."""
    if data[:5] == b"%PDF-":
        return "application/pdf", ".pdf"
    if data[:4] == b"RIFF" and data[8:12] == b"WEBP":
        return "image/webp", ".webp"
    for magic, mime, ext in IMAGE_MAGIC:
        if data.startswith(magic):
            return mime, ext
    return None


def validate_upload(data: bytes):
    """Returns (mime, ext, page_count). Raises ApiError for anything unsupported or oversized."""
    if len(data) > settings.max_upload_mb * 1024 * 1024:
        raise ApiError(413, "file_too_large", f"File exceeds {settings.max_upload_mb} MB")
    if not data:
        raise ApiError(400, "empty_file", "The uploaded file is empty")
    kind = sniff(data)
    if not kind:
        raise ApiError(415, "unsupported_type", "Only PNG, JPEG, TIFF, BMP, WEBP images and PDF files are accepted")
    mime, ext = kind
    try:
        if mime == "application/pdf":
            with fitz.open(stream=data, filetype="pdf") as doc:
                if doc.needs_pass:
                    raise ApiError(400, "encrypted_pdf", "Password-protected PDFs are not supported")
                pages = doc.page_count
            if pages > settings.max_pdf_pages:
                raise ApiError(413, "too_many_pages", f"PDF has {pages} pages; the limit is {settings.max_pdf_pages}")
            return mime, ext, pages
        with Image.open(io.BytesIO(data)) as im:
            if im.width * im.height > MAX_PIXELS:
                raise ApiError(413, "image_too_large", "Image dimensions are too large")
            im.verify()
        return mime, ext, 1
    except ApiError:
        raise
    except Exception:
        raise ApiError(400, "corrupt_file", "The file could not be read; it may be corrupt")
