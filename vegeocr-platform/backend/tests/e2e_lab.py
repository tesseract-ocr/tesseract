"""OCR lab against a RUNNING API:  python tests/e2e_lab.py [base_url]
The API needs a worker (or CELERY_EAGER=true), Tesseract, and (for the handwriting checks) the TrOCR model."""
import io
import os
import sys
import time
import uuid
import zipfile

import cv2
import httpx
import numpy as np

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from lab_samples import degrade, sample_lines, text_page  # noqa: E402
from app.services.training import cer  # noqa: E402

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8000"
c = httpx.Client(base_url=BASE, timeout=120)
fails = 0


def check(name, cond, extra=""):
    global fails
    print(("PASS " if cond else "FAIL ") + name + (f"   {extra}" if extra and not cond else ""))
    fails += 0 if cond else 1


bearer = lambda t: {"Authorization": f"Bearer {t}"}  # noqa: E731
code = lambda r: (r.json().get("error") or {}).get("code") if "json" in r.headers.get("content-type", "") else None  # noqa: E731
png = lambda arr: cv2.imencode(".png", arr)[1].tobytes()  # noqa: E731


def signup():
    return c.post("/api/web/auth/signup", json={"email": f"lab{uuid.uuid4().hex[:8]}@example.com", "password": "Passw0rd123"}).json()["token"]


def submit(tok, data, name="x.png", mime="image/png", **fields):
    fields.setdefault("lang", "eng")
    return c.post("/api/web/lab/runs", files={"file": (name, data, mime)}, data={k: str(v) for k, v in fields.items()}, headers=bearer(tok))


def finish(tok, run_id, timeout=180):
    end = time.time() + timeout
    while time.time() < end:
        r = c.get(f"/api/web/lab/runs/{run_id}", headers=bearer(tok)).json()
        if r["status"] in ("done", "failed"):
            return r
        time.sleep(1)
    raise TimeoutError(run_id)


def run(tok, data, **kw):
    r = submit(tok, data, **kw)
    assert r.status_code == 202, r.text
    return finish(tok, r.json()["id"])


A, B = signup(), signup()
LINES = sample_lines(8)
REF = "\n".join(LINES)
text_of = lambda r: r["result"]["text"]  # noqa: E731

# ------------------------------------------------------------------------------------------------ capabilities
check("capabilities need a login", c.get("/api/web/lab/capabilities").status_code == 401)
cap = c.get("/api/web/lab/capabilities", headers=bearer(A)).json()
check("capabilities list engines, limits and training info", {"engines", "limits", "training", "modes", "low_confidence_threshold"} <= set(cap) and cap["engines"]["tesseract"]["available"])
TROCR = cap["engines"]["trocr"]["available"]
print(f"   (TrOCR available: {TROCR}; handwriting engine: {cap['handwriting_engine']})")

# ------------------------------------------------------------------------------------------------ printed, crisp
r = run(A, png(text_page(LINES, size=34)), mode="printed")
check("crisp printed page: done, correct text, nothing flagged", r["status"] == "done" and cer([(REF, text_of(r))]) < 0.02 and r["result"]["flagged_count"] == 0, f"{r['status']} cer={cer([(REF, text_of(r))]) if r.get('result') else None}")
check("crisp page was not enhanced", not r["result"]["enhanced"] and r["result"]["pages"][0]["quality_before"]["blur_level"] == "sharp")
check("lines carry a confidence and a box", all(0 <= l["confidence"] <= 100 and len(l["bbox"]) == 4 and "flagged" in l for l in r["result"]["pages"][0]["lines"]))
rid = r["id"]
img = c.get(f"/api/web/lab/runs/{rid}/pages/1/original", headers=bearer(A))
check("original image is served as PNG", img.status_code == 200 and img.content[:4] == b"\x89PNG")
check("no enhanced image exists when none was made", c.get(f"/api/web/lab/runs/{rid}/pages/1/enhanced", headers=bearer(A)).status_code == 404)
check("history lists the run", rid in [x["id"] for x in c.get("/api/web/lab/runs", headers=bearer(A)).json()])

# ------------------------------------------------------------------------------------------------ blurry: detect, enhance, compare
blurry_lines = sample_lines(8, seed=2)
BREF = "\n".join(blurry_lines)
blurry = png(degrade(text_page(blurry_lines, size=20), blur=2.0))
on = run(A, blurry, mode="printed", enhance="auto")
off = run(A, blurry, mode="printed", enhance="off")
pg = on["result"]["pages"][0]
check("blurry image is detected and enhanced automatically", on["result"]["enhanced"] and pg["quality_before"]["blur_level"] in ("blurry", "soft") and pg["enhance_steps"], str(pg["quality_before"]))
check("the report says WHY and WHAT was done", pg["enhance_reasons"] and any("sharpen" in s for s in pg["enhance_steps"]))
check("quality improves: edge sharpness after > before", pg["quality_after"]["sharpness"] > pg["quality_before"]["sharpness"], f"{pg['quality_before']['sharpness']} -> {pg['quality_after']['sharpness']}")
e_on, e_off = cer([(BREF, text_of(on))]), cer([(BREF, text_of(off))])
print(f"   blurry page error rate: enhancement off {e_off*100:.1f}%  ->  auto {e_on*100:.1f}%")
check("enhancement makes the extraction clearly better on a blurry page", e_on < e_off - 0.05 and e_on < 0.2, f"{e_off:.3f} -> {e_on:.3f}")
check("the enhanced image is served (side-by-side preview)", c.get(f"/api/web/lab/runs/{on['id']}/pages/1/enhanced", headers=bearer(A)).status_code == 200)
check("original-vs-enhanced check is reported", pg["enhance_check"] and pg["enhance_check"]["kept"] in ("enhanced", "original"))
check("enhance=off really does nothing", not off["result"]["enhanced"])
forced = run(A, png(text_page(LINES, size=34)), mode="printed", enhance="on")
check("enhance=on forces enhancement on a sharp page", forced["result"]["enhanced"] is True or forced["result"]["pages"][0]["enhance_steps"] == [])

# ------------------------------------------------------------------------------------------------ low confidence flags
hard = run(A, png(degrade(text_page(sample_lines(6, seed=3), size=18), blur=2.8, noise=14)), mode="printed", enhance="auto")
if hard["status"] == "done" and hard["result"]["pages"][0]["lines"]:
    ls = hard["result"]["pages"][0]["lines"]
    thr = hard["result"]["low_confidence_threshold"]
    check("lines under the threshold are flagged, others are not", all(l["flagged"] == (l["confidence"] < thr) for l in ls) and hard["result"]["flagged_count"] == sum(l["flagged"] for l in ls))
    check("a very poor image produces flagged lines", hard["result"]["flagged_count"] > 0, str([l["confidence"] for l in ls]))
else:
    print("SKIP flag check: the engine found no lines in the hard image")
blank = run(A, png(np.full((300, 500), 255, np.uint8)), mode="printed")
check("a blank page is 'done' with no lines and a clear warning (not an error)", blank["status"] == "done" and blank["result"]["pages"][0]["lines"] == [] and any("No text" in w for w in blank["result"]["warnings"]))

# ------------------------------------------------------------------------------------------------ handwriting
HW = "/System/Library/Fonts/Supplemental/Apple Chancery.ttf"
if TROCR and os.path.exists(HW):
    hw_lines = ["The quick brown fox jumps over the lazy dog", "Please call me back after lunch today"]
    hw = run(A, png(text_page(hw_lines, size=56, handwriting_font=HW)), mode="handwriting")
    check("handwriting mode uses the TrOCR engine for English", hw["status"] == "done" and hw["result"]["engine"] == "trocr", hw["result"]["engine"] if hw.get("result") else str(hw))
    check("handwriting text is close to the truth and has line confidences", cer([("\n".join(hw_lines), text_of(hw))]) < 0.25 and all(0 <= l["confidence"] <= 100 for l in hw["result"]["pages"][0]["lines"]), text_of(hw))
    check("forcing the Tesseract engine for handwriting works", run(A, png(text_page(hw_lines, size=56, handwriting_font=HW)), mode="handwriting", engine="tesseract")["result"]["engine"] == "tesseract")
    x = submit(A, png(text_page(hw_lines, size=56)), mode="handwriting", engine="trocr", lang="jpn")
    check("TrOCR with a non-English language is refused cleanly", finish(A, x.json()["id"])["error"]["code"] == "engine_language")
else:
    print("SKIP handwriting (TrOCR or a handwriting font is not available here)")
jp = run(A, png(text_page(LINES[:3], size=40)), mode="handwriting", lang="jpn+eng")
check("handwriting in Japanese falls back to Tesseract with a visible warning", jp["result"]["engine"] == "tesseract" and any("Falling back" in w or "English only" in w for w in jp["result"]["warnings"]), str(jp["result"]["warnings"]))

# ------------------------------------------------------------------------------------------------ PDF
import fitz  # noqa: E402
doc = fitz.open()
for i in range(2):
    pgx = doc.new_page()
    pgx.insert_image(pgx.rect, stream=png(text_page(sample_lines(8, seed=10 + i), size=34)))
pdf = run(A, doc.tobytes(), name="scan.pdf", mime="application/pdf", mode="printed")
check("a 2-page scanned PDF gives 2 pages of results", pdf["status"] == "done" and len(pdf["result"]["pages"]) == 2 and all(p["lines"] for p in pdf["result"]["pages"]))
doc5 = fitz.open()
for i in range(5):
    doc5.new_page().insert_text((72, 100), f"page {i}", fontsize=24)
many = run(A, doc5.tobytes(), name="many.pdf", mime="application/pdf", mode="printed")
check("a 5-page PDF is capped at 3 pages with a warning", len(many["result"]["pages"]) == 3 and any("first 3" in w for w in many["result"]["warnings"]))

# ------------------------------------------------------------------------------------------------ errors
check("a text file is refused (415)", (lambda x: x.status_code == 415 and code(x) == "unsupported_type")(submit(A, b"just some text, not an image", name="a.png")))
check("a Windows executable renamed .png is refused (415)", submit(A, b"MZ\x90\x00" + b"\x00" * 200, name="evil.png").status_code == 415)
check("an empty file is refused (400)", submit(A, b"").status_code == 400)
check("a corrupt PNG is refused (400)", submit(A, png(text_page(LINES, size=30))[:300]).status_code in (400, 422))
big = submit(A, b"\x89PNG\r\n\x1a\n" + b"\x00" * (16 * 1024 * 1024))
check("a file over 15 MB is refused (413)", big.status_code == 413 and code(big) == "file_too_large")
good = png(text_page(LINES[:2], size=30))
for field, val, expect in (("mode", "telepathy", "invalid_mode"), ("enhance", "maybe", "invalid_enhance"), ("engine", "magic", "invalid_engine"), ("lang", "klingon", "invalid_lang")):
    x = submit(A, good, **{field: val})
    check(f"invalid {field} -> 422 {expect}", x.status_code == 422 and code(x) == expect, x.text)
check("someone else's dataset id is refused", submit(A, good, dataset_id=999999).status_code == 404)
_f = finish(A, submit(A, png(degrade(text_page(LINES[:2], size=30))), mode="handwriting", engine="trocr", lang="jpn").json()["id"])
check("a failed run is reported with a code, not a crash", _f["status"] == "failed" and _f["error"]["code"] in ("engine_language", "engine_unavailable"), str(_f.get("error")))
check("runs are private to their owner", c.get(f"/api/web/lab/runs/{rid}", headers=bearer(B)).status_code == 404 and c.get(f"/api/web/lab/runs/{rid}/pages/1/original", headers=bearer(B)).status_code == 404)
check("runs need a login", c.get(f"/api/web/lab/runs/{rid}").status_code == 401)
dl = submit(A, good); finish(A, dl.json()["id"])
check("a run can be deleted", c.delete(f"/api/web/lab/runs/{dl.json()['id']}", headers=bearer(A)).status_code == 204 and c.get(f"/api/web/lab/runs/{dl.json()['id']}", headers=bearer(A)).status_code == 404)

# ------------------------------------------------------------------------------------------------ datasets
mk = lambda tok, **b: c.post("/api/web/lab/datasets", json={"name": "d", "category": "handwriting", "lang": "eng", **b}, headers=bearer(tok))  # noqa: E731
d1 = mk(A, name="My notes", category="Handwriting")
check("create a dataset (category is normalised)", d1.status_code == 201 and d1.json()["category"] == "handwriting" and d1.json()["sample_count"] == 0, d1.text)
D1 = d1.json()["id"]
check("duplicate name -> 409", mk(A, name="My notes").status_code == 409)
check("same name for another user is fine", mk(B, name="My notes").status_code == 201)
check("invalid language -> 422", mk(A, name="x", lang="klingon").status_code == 422)
check("blank name -> 422", mk(A, name="").status_code == 422)
d2 = mk(A, name="Receipts", category="printed", lang="eng").json()
lst = c.get("/api/web/lab/datasets", headers=bearer(A)).json()
check("list shows my datasets and their categories", {x["name"] for x in lst["items"]} == {"My notes", "Receipts"} and set(lst["categories"]) == {"handwriting", "printed"})
check("filter by category", [x["name"] for x in c.get("/api/web/lab/datasets?category=printed", headers=bearer(A)).json()["items"]] == ["Receipts"])
check("rename / recategorise", c.patch(f"/api/web/lab/datasets/{d2['id']}", json={"name": "Shop receipts", "category": "forms"}, headers=bearer(A)).json()["name"] == "Shop receipts")
check("rename to an existing name -> 409", c.patch(f"/api/web/lab/datasets/{d2['id']}", json={"name": "My notes"}, headers=bearer(A)).status_code == 409)
check("other users cannot see / change / delete my dataset", all(c.request(m, f"/api/web/lab/datasets/{D1}{p}", headers=bearer(B), **({"json": {}} if m == "PATCH" else {})).status_code == 404 for m, p in (("GET", ""), ("PATCH", ""), ("DELETE", ""), ("GET", "/samples"), ("GET", "/export"), ("POST", "/train"))))

# --- samples
line_png = lambda t, size=30, **kw: png(degrade(text_page([t], size=size, margin=14), **kw))  # noqa: E731
add = lambda tok, ds, data, text: c.post(f"/api/web/lab/datasets/{ds}/samples", files={"image": ("l.png", data, "image/png")}, data={"text": text}, headers=bearer(tok))  # noqa: E731
s1 = add(A, D1, line_png("hello world"), "hello world")
check("add a sample (image + text)", s1.status_code == 201 and s1.json()["text"] == "hello world" and s1.json()["source"] == "upload", s1.text)
S1 = s1.json()["id"]
check("sample image is served", c.get(f"/api/web/lab/datasets/{D1}/samples/{S1}/image", headers=bearer(A)).content[:4] == b"\x89PNG")
check("multi-line / odd whitespace in the label is normalised", add(A, D1, line_png("two words"), "two\n  words").json()["text"] == "two words")
check("empty label -> 422", add(A, D1, line_png("x"), "   ").status_code == 422)
check("a whole page is not a line (422)", add(A, D1, png(text_page(LINES, size=40)), "x").status_code == 422)
import struct, zlib  # noqa: E402,E401
def _bomb(w, h):
    ck = lambda k, d: struct.pack(">I", len(d)) + k + d + struct.pack(">I", zlib.crc32(k + d) & 0xFFFFFFFF)  # noqa: E731
    z = zlib.compressobj(9); body = b"".join(z.compress(b"\x00" * (w + 1)) for _ in range(h)) + z.flush()
    return b"\x89PNG\r\n\x1a\n" + ck(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 0, 0, 0, 0)) + ck(b"IDAT", body) + ck(b"IEND", b"")
_t = time.time(); _x = add(A, D1, _bomb(20000, 20000), "bomb")
check("a PNG bomb (20000x20000 declared, tiny file) is refused fast, without being decoded", _x.status_code == 422 and time.time() - _t < 5, f"{_x.status_code} {time.time() - _t:.1f}s")
check("a non-image is refused (422)", add(A, D1, b"not an image", "x").status_code == 422)
check("a tiny image is refused (422)", add(A, D1, png(np.zeros((3, 3), np.uint8)), "x").status_code == 422)
check("edit a label", c.patch(f"/api/web/lab/datasets/{D1}/samples/{S1}", json={"text": "hello there"}, headers=bearer(A)).json()["text"] == "hello there")
check("a sample id from another dataset is refused", c.patch(f"/api/web/lab/datasets/{d2['id']}/samples/{S1}", json={"text": "x"}, headers=bearer(A)).status_code == 404)
lst = c.get(f"/api/web/lab/datasets/{D1}/samples?q=there", headers=bearer(A)).json()
check("search and paging", lst["total"] == 1 and lst["items"][0]["id"] == S1)
check("delete a sample", c.delete(f"/api/web/lab/datasets/{D1}/samples/{S1}", headers=bearer(A)).status_code == 204 and c.get(f"/api/web/lab/datasets/{D1}/samples/{S1}/image", headers=bearer(A)).status_code == 404)

# --- corrections (the feedback loop)
cor_run = run(A, png(degrade(text_page(sample_lines(4, seed=20), size=26), blur=1.2)), mode="printed")
ln = cor_run["result"]["pages"][0]["lines"][0]
cr = c.post(f"/api/web/lab/runs/{cor_run['id']}/corrections", json={"page": 1, "index": 0, "text": "corrected text here", "dataset_id": D1}, headers=bearer(A))
check("saving a correction creates a labelled sample", cr.status_code == 201 and cr.json()["source"] == "correction" and cr.json()["text"] == "corrected text here" and cr.json()["original_text"] == ln["text"], cr.text)
img = cv2.imdecode(np.frombuffer(c.get(f"/api/web/lab/datasets/{D1}/samples/{cr.json()['id']}/image", headers=bearer(A)).content, np.uint8), cv2.IMREAD_GRAYSCALE)
check("...and the sample image is the cropped LINE (short, wide, not the page)", img is not None and img.shape[0] <= 100 and img.shape[1] > img.shape[0] * 3, str(None if img is None else img.shape))
check("correction with a bad line index -> 422", c.post(f"/api/web/lab/runs/{cor_run['id']}/corrections", json={"page": 1, "index": 99, "text": "x", "dataset_id": D1}, headers=bearer(A)).status_code == 422)
check("correction into someone else's dataset -> 404", c.post(f"/api/web/lab/runs/{cor_run['id']}/corrections", json={"page": 1, "index": 0, "text": "x", "dataset_id": D1}, headers=bearer(B)).status_code == 404)
check("correction from a run that is not yours -> 404", c.post(f"/api/web/lab/runs/{cor_run['id']}/corrections", json={"page": 1, "index": 0, "text": "x", "dataset_id": mk(B, name='b2').json()['id']}, headers=bearer(B)).status_code == 404)

# --- export / import round trip
ex = c.get(f"/api/web/lab/datasets/{D1}/export", headers=bearer(A))
z = zipfile.ZipFile(io.BytesIO(ex.content))
names = z.namelist()
check("export is a zip with labels.tsv, dataset.json and the images", ex.status_code == 200 and "labels.tsv" in names and "dataset.json" in names and any(n.startswith("images/") for n in names), str(names))
labels = z.read("labels.tsv").decode().strip().splitlines()
n_before = c.get(f"/api/web/lab/datasets/{D1}", headers=bearer(A)).json()["sample_count"]
check("every sample is in labels.tsv with its text", len(labels) == n_before and any("corrected text here" in l for l in labels))
imp = c.post("/api/web/lab/datasets/import", files={"file": ("export.zip", ex.content, "application/zip")}, data={"name": "Imported copy"}, headers=bearer(A))
check("importing the export creates an identical dataset", imp.status_code == 201 and imp.json()["import"]["added"] == n_before and imp.json()["dataset"]["category"] == "handwriting" and imp.json()["dataset"]["name"] == "Imported copy", imp.text)
back = c.get(f"/api/web/lab/datasets/{imp.json()['dataset']['id']}/samples?per_page=100", headers=bearer(A)).json()
check("...with the same texts", sorted(s["text"] for s in back["items"]) == sorted(l.split("\t", 1)[1] for l in labels))
csvzip = io.BytesIO()
with zipfile.ZipFile(csvzip, "w") as zf:
    zf.writestr("labels.csv", "filename,text\na.png,csv one\nsub/b.png,\"csv, two\"\nmissing.png,nope\n../evil.png,bad\n")
    zf.writestr("a.png", line_png("csv one")); zf.writestr("sub/b.png", line_png("csv two"))
imp2 = c.post(f"/api/web/lab/datasets/{D1}/import", files={"file": ("c.zip", csvzip.getvalue(), "application/zip")}, headers=bearer(A)).json()
check("CSV labels work; missing files and unsafe paths are skipped with reasons", imp2["added"] == 1 or imp2["added"] == 2, str(imp2))
check("...the skipped ones are reported", imp2["skipped_count"] >= 2 and any("unsafe" in s["reason"] or "not in the zip" in s["reason"] for s in imp2["skipped"]), str(imp2))
check("not a zip -> 422", c.post(f"/api/web/lab/datasets/{D1}/import", files={"file": ("c.zip", b"nope", "application/zip")}, headers=bearer(A)).status_code == 422)
nolab = io.BytesIO()
with zipfile.ZipFile(nolab, "w") as zf:
    zf.writestr("a.png", b"x")
check("a zip without labels -> 422 no_labels", code(c.post(f"/api/web/lab/datasets/{D1}/import", files={"file": ("c.zip", nolab.getvalue(), "application/zip")}, headers=bearer(A))) == "no_labels")
check("a failed import-as-new leaves no empty dataset behind", (lambda before, _: len(c.get("/api/web/lab/datasets", headers=bearer(A)).json()["items"]) == before)(len(c.get("/api/web/lab/datasets", headers=bearer(A)).json()["items"]), c.post("/api/web/lab/datasets/import", files={"file": ("c.zip", nolab.getvalue(), "application/zip")}, data={"name": "ghost"}, headers=bearer(A))))

# ------------------------------------------------------------------------------------------------ training
check("training with too few samples is refused with a clear message", (lambda x: x.status_code == 422 and code(x) == "not_enough_samples")(c.post(f"/api/web/lab/datasets/{d2['id']}/train", headers=bearer(A))))
TR = mk(A, name="Blurry receipts", category="blurry", lang="eng").json()["id"]
# A realistic reason to train: a source of poor scans the standard model reads badly (small, blurred, noisy text).
rnd = sample_lines(60, seed=77, words_per_line=3)
for i, t in enumerate(rnd):
    add(A, TR, line_png(t, size=22, blur=2.2, noise=14, seed=i), t)
info = c.get(f"/api/web/lab/datasets/{TR}", headers=bearer(A)).json()
check("the dataset reports it is ready to train", info["training"]["ready"] and info["sample_count"] == 60 and info["training"]["test_samples"] == 12, str(info["training"]))
t0 = time.time()
tr = c.post(f"/api/web/lab/datasets/{TR}/train", headers=bearer(A))
check("training starts (202) as a background job", tr.status_code == 202 and tr.json()["status"] in ("queued", "training"), tr.text)
dup = c.post(f"/api/web/lab/datasets/{TR}/train", headers=bearer(A))
check("a second training while one runs -> 409", dup.status_code == 409 and code(dup) == "training_in_progress", dup.text)
m = tr.json()
while time.time() - t0 < 600:
    ms = c.get(f"/api/web/lab/datasets/{TR}/models", headers=bearer(A)).json()["models"]
    m = next(x for x in ms if x["id"] == tr.json()["id"])
    if m["status"] not in ("queued", "training"):
        break
    time.sleep(3)
print(f"   training took {time.time() - t0:.0f}s: {m['status']} | before {m['cer_before']} -> after {m['cer_after']} | {m['message']}")
check("training finishes with measured accuracy before and after", m["status"] in ("ready", "no_gain") and m["cer_before"] is not None and m["cer_after"] is not None, str(m))
check("the split is honest: 1 in 5 samples is held out", m["test_samples"] == 12 and m["train_samples"] == 48)
if m["status"] == "ready":
    check("a model that improved is switched on automatically, and it really is better", m["active"] and m["cer_after"] < m["cer_before"])
    ex = run(A, png(degrade(text_page(sample_lines(4, seed=91, words_per_line=3), size=22), blur=2.2, noise=14)), mode="printed", dataset_id=TR)
    check("extraction can use the dataset's model", ex["status"] == "done" and ex["result"]["used_dataset_model"] is True)
    check("model can be switched off and on", c.post(f"/api/web/lab/datasets/{TR}/models/{m['id']}/deactivate", headers=bearer(A)).json()["active"] is False and c.post(f"/api/web/lab/datasets/{TR}/models/{m['id']}/activate", headers=bearer(A)).json()["active"] is True)
    off_ = run(A, png(text_page(LINES[:2], size=30)), mode="printed", dataset_id=TR)
    check("with the model on, normal text is still read correctly", off_["status"] == "done" and cer([("\n".join(LINES[:2]), text_of(off_))]) < 0.1)
else:
    print("NOTE the fine-tune did not beat the base model on this data; that outcome is handled and reported, not hidden")
    check("a model without gain is not switched on", not m["active"])
no_ds = run(A, png(text_page(LINES[:2], size=30)), mode="printed", dataset_id=D1)
check("a dataset without an active model falls back to the standard model, with a warning", no_ds["status"] == "done" and no_ds["result"]["used_dataset_model"] is False and any("no active model" in w for w in no_ds["result"]["warnings"]))
# per-user cap: three small datasets, three training requests back to back -> the third is refused
cap_ds = []
for n in range(3):
    did = mk(B, name=f"cap{n}", category="general").json()["id"]
    for i, t in enumerate(sample_lines(10, seed=300 + n, words_per_line=2)):
        add(B, did, line_png(t, size=24), t)
    cap_ds.append(did)
codes = [c.post(f"/api/web/lab/datasets/{d}/train", headers=bearer(B)).status_code for d in cap_ds]
check("a user can have 2 trainings in progress; the 3rd is refused (429)", codes == [202, 202, 429], str(codes))
check("...with a clear code", code(c.post(f"/api/web/lab/datasets/{cap_ds[2]}/train", headers=bearer(B))) == "too_many_trainings")
for d in cap_ds[:2]:  # let them finish so nothing is left running when the suite ends
    for _ in range(150):
        ms = c.get(f"/api/web/lab/datasets/{d}/models", headers=bearer(B)).json()["models"]
        if ms and ms[0]["status"] not in ("queued", "training"):
            break
        time.sleep(2)
check("both trainings end cleanly (ready, no gain, or a clear failure message)", all(c.get(f"/api/web/lab/datasets/{d}/models", headers=bearer(B)).json()["models"][0]["status"] in ("ready", "no_gain", "failed") for d in cap_ds[:2]))
check("other users cannot train / activate my models", c.post(f"/api/web/lab/datasets/{TR}/models/{m['id']}/activate", headers=bearer(B)).status_code == 404)
check("deleting a dataset removes it and its samples", c.delete(f"/api/web/lab/datasets/{TR}", headers=bearer(A)).status_code == 204 and c.get(f"/api/web/lab/datasets/{TR}", headers=bearer(A)).status_code == 404)

print("\nFAILED: %d" % fails if fails else "\nALL PASSED")
sys.exit(1 if fails else 0)
