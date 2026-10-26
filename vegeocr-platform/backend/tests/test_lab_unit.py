"""Unit tests for the OCR lab's image code (no database, no server).   .venv/bin/python -m pytest tests/test_lab_unit.py -q"""
import io
import os
import sys

import cv2
import numpy as np
import pytest
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from lab_samples import degrade, sample_lines, text_page  # noqa: E402

from app.ocrlab import datasets as D, enhance as E, linesplit as L, pipeline as P, quality as Q  # noqa: E402
from app.ocrlab.engines import Line  # noqa: E402

LINES = sample_lines(8)


@pytest.fixture(scope="module")
def crisp():
    return text_page(LINES, size=34)


# ---- quality ---------------------------------------------------------------------------------------------------

def test_crisp_page_is_not_flagged(crisp):
    r = Q.analyse(crisp)
    assert r.blur_level == "sharp" and not r.needs_enhancement and r.reasons == []


@pytest.mark.parametrize("size", [20, 34, 60])
def test_sharpness_does_not_depend_on_font_size(size):
    s = Q.analyse(text_page(LINES, size=size)).sharpness
    assert 0.45 < s < 0.62, s


@pytest.mark.parametrize("blur,level", [(0.6, "sharp"), (1.5, "soft"), (2.5, "blurry"), (4.0, "blurry")])
def test_blur_levels(crisp, blur, level):
    assert Q.analyse(degrade(crisp, blur=blur)).blur_level == level


def test_sharpness_decreases_monotonically_with_blur(crisp):
    s = [Q.analyse(degrade(crisp, blur=b)).sharpness for b in (0, 0.6, 1.0, 1.5, 2.0, 3.0, 4.0)]
    assert s == sorted(s, reverse=True)


def test_each_defect_gets_its_own_reason(crisp):
    assert any("noisy" in x for x in Q.analyse(degrade(crisp, noise=15)).reasons)
    assert any("shadow" in x or "lighting" in x for x in Q.analyse(degrade(crisp, shadow=0.5)).reasons)
    assert any("small text" in x for x in Q.analyse(text_page(LINES, size=18)).reasons)
    assert any("blur" in x or "soft" in x for x in Q.analyse(degrade(crisp, blur=2.5)).reasons)


def test_report_serialises(crisp):
    d = Q.analyse(crisp).as_dict()
    assert {"sharpness", "blur_level", "noise_sigma", "needs_enhancement", "reasons"} <= set(d)


def test_blank_image_does_not_crash():
    r = Q.analyse(np.full((100, 100), 255, np.uint8))
    assert r.text_height == 0.0


def test_colour_input_is_accepted(crisp):
    assert Q.analyse(cv2.cvtColor(crisp, cv2.COLOR_GRAY2BGR)).blur_level == "sharp"


# ---- enhance ---------------------------------------------------------------------------------------------------

def test_enhancement_sharpens_a_blurry_page(crisp):
    blurry = degrade(crisp, blur=2.5)
    res = E.enhance(blurry)
    assert Q.sharpness(res.image) > Q.sharpness(blurry) + 0.05
    assert any("sharpen" in s for s in res.steps)


def test_enhancement_denoises(crisp):
    noisy = degrade(crisp, noise=20)
    res = E.enhance(noisy)
    assert Q.estimate_noise(res.image) < Q.estimate_noise(noisy) * 0.6
    assert any("denoise" in s for s in res.steps)


def test_enhancement_flattens_shadows(crisp):
    shaded = degrade(crisp, shadow=0.6)
    res = E.enhance(shaded)
    assert Q.background_unevenness(res.image) < Q.background_unevenness(shaded) * 0.4
    assert "shading correction" in res.steps


def test_enhancement_upscales_small_text():
    small = text_page(LINES, size=16)
    res = E.enhance(small)
    assert res.scale > 1.3 and res.image.shape[0] > small.shape[0] * 1.3


def test_enhancement_output_is_single_channel_uint8(crisp):
    res = E.enhance(cv2.cvtColor(degrade(crisp, blur=2), cv2.COLOR_GRAY2BGR))
    assert res.image.ndim == 2 and res.image.dtype == np.uint8


def test_deskew_straightens_a_rotated_page(crisp):
    h, w = crisp.shape
    rotated = cv2.warpAffine(crisp, cv2.getRotationMatrix2D((w / 2, h / 2), 3.0, 1), (w, h), borderValue=255)
    res = E.enhance(rotated, opts=E.EnhanceOptions(denoise=False, upscale=False, sharpen=False, contrast=False, fix_lighting=False))
    assert 2.0 < abs(res.angle) < 4.0 and any("deskew" in s for s in res.steps)


def test_options_switch_steps_off(crisp):
    res = E.enhance(degrade(crisp, blur=2.5), opts=E.EnhanceOptions(sharpen=False, contrast=False, upscale=False, denoise=False, fix_lighting=False, deskew=False))
    assert res.steps == []


# ---- line splitting ----------------------------------------------------------------------------------------------

def test_finds_every_text_line(crisp):
    assert len(L.find_lines(crisp)) == 8


def test_line_boxes_are_ordered_top_to_bottom_and_inside_the_image(crisp):
    boxes = L.find_lines(crisp)
    assert [b[1] for b in boxes] == sorted(b[1] for b in boxes)
    assert all(0 <= l < r <= crisp.shape[1] and 0 <= t < b <= crisp.shape[0] for l, t, r, b in boxes)


def test_blank_page_has_no_lines():
    assert L.find_lines(np.full((200, 300), 255, np.uint8)) == []


def test_handles_a_page_of_pure_noise_without_crashing():
    L.find_lines(np.random.RandomState(0).randint(0, 255, (200, 300)).astype(np.uint8))


def test_wide_lines_are_cut_into_chunks_at_gaps():
    page = text_page(["alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu nu xi omicron"], size=34)
    box = L.find_lines(page)[0]
    chunks = L.split_wide(page, box, max_aspect=6)
    assert len(chunks) >= 2
    assert all((r - l) <= 6.5 * (b - t) for l, t, r, b in chunks)
    assert chunks[0][0] == box[0] and chunks[-1][2] == box[2]  # nothing lost at the ends


def test_short_lines_are_not_split(crisp):
    box = L.find_lines(crisp)[0]
    assert L.split_wide(crisp, box, max_aspect=40) == [box]


# ---- loading ---------------------------------------------------------------------------------------------------

def png_bytes(arr, fmt="PNG", **kw):
    b = io.BytesIO()
    Image.fromarray(arr).save(b, fmt, **kw)
    return b.getvalue()


def test_loads_png_and_jpeg(crisp):
    for fmt, mime in (("PNG", "image/png"), ("JPEG", "image/jpeg")):
        pages, warns = P.load_pages(png_bytes(crisp, fmt), mime)
        assert len(pages) == 1 and pages[0].ndim == 3 and warns == []


def test_corrupt_image_is_a_clean_error():
    with pytest.raises(P.LabError) as e:
        P.load_pages(b"\x89PNG\r\n\x1a\n" + b"garbage" * 50, "image/png")
    assert e.value.status == 422 and e.value.code == "unreadable_image"


def test_truncated_image_is_a_clean_error(crisp):
    data = png_bytes(crisp)
    with pytest.raises(P.LabError) as e:
        P.load_pages(data[: len(data) // 3], "image/png")
    assert e.value.code == "unreadable_image"


def test_huge_images_are_refused(crisp, monkeypatch):
    monkeypatch.setattr(P, "MAX_PIXELS", 1000)
    with pytest.raises(P.LabError) as e:
        P.load_pages(png_bytes(crisp), "image/png")
    assert e.value.status == 413 and e.value.code == "image_too_large"


def test_oversized_pages_are_reduced(crisp, monkeypatch):
    monkeypatch.setattr(P, "MAX_SIDE", 300)
    pages, warns = P.load_pages(png_bytes(crisp), "image/png")
    assert max(pages[0].shape[:2]) == 300 and any("reduced" in w for w in warns)


def test_exif_rotation_is_applied(crisp):
    im = Image.fromarray(crisp).convert("RGB")
    exif = im.getexif()
    exif[0x0112] = 6  # "rotate 90 CW to display"
    b = io.BytesIO()
    im.save(b, "JPEG", exif=exif)
    pages, _ = P.load_pages(b.getvalue(), "image/jpeg")
    assert pages[0].shape[:2] == (crisp.shape[1], crisp.shape[0])  # width and height swapped


def test_pdf_pages_are_rendered_and_capped(crisp, monkeypatch):
    import fitz
    doc = fitz.open()
    for _ in range(5):
        p = doc.new_page()
        p.insert_text((72, 100), "Hello page", fontsize=20)
    data = doc.tobytes()
    pages, warns = P.load_pages(data, "application/pdf")
    assert len(pages) == P.MAX_PAGES and any("first" in w for w in warns)


def test_empty_pdf_is_a_clean_error():
    import fitz
    doc = fitz.open()
    doc.new_page()
    doc.delete_page(0) if doc.page_count else None
    with pytest.raises(P.LabError):
        P.load_pages(b"%PDF-1.4\n%junk", "application/pdf")


# ---- engine choice -----------------------------------------------------------------------------------------------

class FakeTrOCR:
    name, cheap = "trocr", False
    def __init__(self, reason=None): self.reason = reason
    def unavailable_reason(self): return self.reason
    def available(self): return self.reason is None


@pytest.fixture
def engines(monkeypatch):
    def setup(trocr_reason=None):
        monkeypatch.setitem(P.ENGINES, "trocr", FakeTrOCR(trocr_reason))
    return setup


def test_printed_always_uses_tesseract(engines):
    engines(None)
    assert P.choose_engine("printed", "eng")[0].name == "tesseract"


def test_handwriting_prefers_trocr_for_english(engines):
    engines(None)
    eng, warns = P.choose_engine("handwriting", "eng")
    assert eng.name == "trocr" and warns == []


def test_handwriting_falls_back_loudly_when_trocr_is_missing(engines):
    engines("not installed")
    eng, warns = P.choose_engine("handwriting", "eng")
    assert eng.name == "tesseract" and "not installed" in warns[0] and "less accurate" in warns[0]


def test_handwriting_in_japanese_falls_back_with_a_reason(engines):
    engines(None)
    eng, warns = P.choose_engine("handwriting", "jpn+eng")
    assert eng.name == "tesseract" and "English only" in warns[0]


def test_forcing_an_unavailable_engine_is_an_error(engines):
    engines("not installed")
    with pytest.raises(P.LabError) as e:
        P.choose_engine("handwriting", "eng", "trocr")
    assert e.value.code == "engine_unavailable"


def test_trocr_with_a_non_english_language_is_an_error(engines):
    engines(None)
    with pytest.raises(P.LabError) as e:
        P.choose_engine("handwriting", "jpn", "trocr")
    assert e.value.code == "engine_language"


def test_unknown_engine_is_an_error():
    with pytest.raises(P.LabError) as e:
        P.choose_engine("printed", "eng", "magic")
    assert e.value.status == 422


# ---- process_page: flags, enhancement decision, original-vs-enhanced check ----------------------------------------------

class FakeEngine:
    """Returns canned lines; the confidence it reports depends on whether it was handed the enhanced (larger/sharper) image."""
    name, cheap = "fake", True
    def __init__(self, conf_original, conf_enhanced, original):
        self.co, self.ce, self.original = conf_original, conf_enhanced, original
    def recognise(self, gray, lang, **kw):
        conf = self.co if np.array_equal(gray, self.original) else self.ce  # compare pixels, not shapes (not every enhancement resizes)
        return [Line("hello world", conf, [0, 0, 10, 10], [{"text": "hello", "confidence": conf, "bbox": [0, 0, 5, 5]}, {"text": "world", "confidence": conf, "bbox": [5, 0, 10, 5]}])]


def test_a_sharp_page_is_not_enhanced_in_auto_mode(crisp):
    out = P.process_page(cv2.cvtColor(crisp, cv2.COLOR_GRAY2BGR), mode="printed", lang="eng", enhance_mode="auto", engine=FakeEngine(90, 90, crisp))
    assert out.enhanced is None and out.after is None and out.steps == []


def test_a_blurry_page_is_enhanced_and_reported(crisp):
    blurry = degrade(crisp, blur=2.5)
    out = P.process_page(blurry, mode="printed", lang="eng", enhance_mode="auto", engine=FakeEngine(30, 90, blurry))
    assert out.enhanced is not None and out.after.sharpness > out.before.sharpness
    assert out.check == {"original_confidence": 30.0, "enhanced_confidence": 90.0, "kept": "enhanced"}


def test_the_original_is_kept_when_enhancing_made_it_less_confident(crisp):
    blurry = degrade(crisp, blur=2.5)
    out = P.process_page(blurry, mode="printed", lang="eng", enhance_mode="auto", engine=FakeEngine(85, 40, blurry))
    assert out.enhanced is None and out.check["kept"] == "original" and out.steps == []


def test_enhance_off_never_enhances(crisp):
    out = P.process_page(degrade(crisp, blur=3), mode="printed", lang="eng", enhance_mode="off", engine=FakeEngine(50, 90, crisp))
    assert out.enhanced is None


def test_enhance_on_forces_it_even_for_a_sharp_page(crisp):
    out = P.process_page(crisp, mode="printed", lang="eng", enhance_mode="on", engine=FakeEngine(90, 90, crisp))
    assert out.check is None  # the original-vs-enhanced comparison only runs in auto mode


def test_low_confidence_lines_are_flagged(crisp):
    out = P.process_page(crisp, mode="printed", lang="eng", enhance_mode="off", engine=FakeEngine(50, 50, crisp))
    r = P.page_result(1, out, threshold=70)
    assert r["flagged_count"] == 1 and r["lines"][0]["flagged"] and r["avg_confidence"] == 50.0
    assert P.page_result(1, out, threshold=40)["flagged_count"] == 0


def test_a_failing_engine_becomes_a_lab_error(crisp):
    from app.ocrlab.engines import EngineError
    class Boom:
        name, cheap = "boom", False
        def recognise(self, *a, **k): raise EngineError("kaput")
    with pytest.raises(P.LabError) as e:
        P.process_page(crisp, mode="printed", lang="eng", enhance_mode="off", engine=Boom())
    assert e.value.status == 502 and e.value.code == "extraction_failed"


def test_an_image_with_no_text_gives_no_lines_not_an_error():
    class Empty:
        name, cheap = "empty", False
        def recognise(self, *a, **k): return []
    out = P.process_page(np.full((200, 300, 3), 255, np.uint8), mode="printed", lang="eng", enhance_mode="off", engine=Empty())
    r = P.page_result(1, out)
    assert r["lines"] == [] and r["avg_confidence"] is None and r["text"] == ""


# ---- dataset label parsing and zip safety ------------------------------------------------------------------------------

def test_label_parsing_formats():
    assert D._parse_labels("a.png\thello world\nb.png\tx, y\n") == [("a.png", "hello world"), ("b.png", "x, y")]
    assert D._parse_labels("a.png,hello\n# comment\n\nb.png,\"x, y\"\n") == [("a.png", "hello"), ("b.png", "x, y")]
    assert D._parse_labels("a.png,one,two\n") == [("a.png", "one,two")]
    assert D._parse_labels("justonecolumn\n") == []


def zip_of(files):
    import zipfile
    b = io.BytesIO()
    with zipfile.ZipFile(b, "w") as z:
        for n, d in files.items():
            z.writestr(n, d)
    return b.getvalue()


def test_zip_without_labels_is_rejected():
    from app.errors import ApiError
    with pytest.raises(ApiError) as e:
        D.read_zip(zip_of({"a.png": b"x"}))
    assert e.value.detail["code"] == "no_labels"


def test_not_a_zip_is_rejected():
    from app.errors import ApiError
    with pytest.raises(ApiError) as e:
        D.read_zip(b"this is not a zip")
    assert e.value.detail["code"] == "bad_zip"


def test_zip_bomb_is_refused_before_reading(monkeypatch):
    from app.errors import ApiError
    monkeypatch.setattr(D, "MAX_ZIP_UNCOMPRESSED", 1000)
    with pytest.raises(ApiError) as e:
        D.read_zip(zip_of({"labels.tsv": b"a.png\tx", "big.bin": b"0" * 5000}))
    assert e.value.status_code == 413


def test_header_row_is_skipped_and_paths_resolved():
    z, rows, meta = D.read_zip(zip_of({"data/labels.csv": b"filename,text\nimg/a.png,hello\n", "data/img/a.png": b"x", "data/dataset.json": b'{"name": "N", "lang": "eng"}'}))
    assert rows == [("data/img/a.png", "hello")] and meta["name"] == "N"


# ---- regressions ------------------------------------------------------------------------------------------------------

def test_line_boxes_are_plain_ints_so_results_can_be_stored_as_json(crisp):
    import json
    boxes = L.find_lines(crisp)
    assert boxes and all(type(v) is int for b in boxes for v in b)
    json.dumps(boxes)


def test_page_results_are_json_serialisable(crisp):
    import json
    out = P.process_page(degrade(crisp, blur=2.5), mode="printed", lang="eng", enhance_mode="on", engine=FakeEngine(80, 80, crisp))
    json.dumps(P.page_result(1, out))


# ---- security: decompression bombs -----------------------------------------------------------------------------------------

def tiny_huge_png(width, height):
    """A valid grey PNG that is a few hundred KB on disk but declares width x height pixels (all black)."""
    import struct
    import zlib

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    comp, row, idat = zlib.compressobj(9), b"\x00" * (width + 1), []
    for _ in range(height):
        idat.append(comp.compress(row))
    idat.append(comp.flush())
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0)) + chunk(b"IDAT", b"".join(idat)) + chunk(b"IEND", b"")


def test_a_png_bomb_is_rejected_from_its_header_without_being_decoded():
    import time
    from app.errors import ApiError
    bomb = tiny_huge_png(20000, 20000)  # 400 million pixels declared
    assert len(bomb) < D.MAX_IMAGE_BYTES, "the test PNG must be small enough to get past the byte limit"
    t = time.time()
    with pytest.raises(ApiError) as e:
        D._decode_line(bomb)
    assert e.value.detail["code"] == "invalid_image" and time.time() - t < 2


def test_a_mid_size_png_bomb_is_rejected_too():
    from app.errors import ApiError
    bomb = tiny_huge_png(10000, 5000)  # 50 megapixels: under Pillow's own limit, so our dimension check must catch it
    with pytest.raises(ApiError) as e:
        D._decode_line(bomb)
    assert e.value.detail["code"] == "invalid_image"


def test_a_normal_line_image_is_accepted():
    arr = D._decode_line(png_bytes(text_page(["hello line"], size=30, margin=14)))
    assert arr.ndim == 2 and 8 <= arr.shape[0] <= 400
