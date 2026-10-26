# OCR lab: handwriting, blurry images and your own training data

In the dashboard: **Learning → Extraction lab** (read an image) and **Learning → My training data** (datasets, corrections, training).
The **Courses** tab also has a course that teaches these features.

## 1. How the three features connect

```
 upload (image or PDF, up to 15 MB, first 3 pages)
   │  checked by magic bytes, size, pixel count          unsupported / corrupt / huge  → clear 4xx error
   ▼
 LabRun "queued"  ──►  Celery worker (queue "ocr")
   │
   ▼
 ┌─────────────────────────────── pipeline.py ────────────────────────────────┐
 │ 2. BLURRY / LOW QUALITY                                                     │
 │   quality.analyse  → sharpness, noise, contrast, lighting, text size        │
 │        │ needs enhancement?  (Auto: only if a problem was found)            │
 │        ▼ yes                                                                │
 │   enhance.enhance  → denoise · flatten shadows · deskew · upscale ·         │
 │                      sharpen · (contrast)        → ENHANCED image           │
 │        │   printed text: read BOTH, keep the one the engine is surer about  │
 │        ▼                                                                    │
 │ 1. HANDWRITING            engines.py                                        │
 │   handwriting + English → TrOCR  (line by line: linesplit.py)               │
 │   printed, or other languages → Tesseract (base model, or YOUR model ──┐    │
 │        ▼                                                              │    │
 │   lines + confidence + "flagged" when below the review threshold      │    │
 └──────────────────────────────────────┬────────────────────────────────┼────┘
                                        ▼                                │
   UI: original | enhanced side by side, editable lines, red = review     │
                                        │ you fix a line                  │
                                        ▼                                │
 3. YOUR TRAINING DATA                                                   │
   crop the line image  ──►  sample in one of YOUR datasets  ◄── add by hand / import a zip
                                        │
                                        ▼  "Train"  (Celery queue "training")
   fine-tune Tesseract on 4/5 of the samples, score the held-out 1/5:
   error rate BEFORE vs AFTER  ──►  model version; if it improved it is switched on ─┘
```

Feature 3 closes the loop: a model trained on your corrections is what feature 1/2 can use on the next upload (**Model → My model: …**).

## 2. Libraries and models, and why

| Job | Chosen | Why | Considered, not chosen |
|---|---|---|---|
| Image enhancement | **OpenCV** (`opencv-python-headless`) | Mature, CPU-only, no model download, deterministic, ~50 MB. Every step is explainable to the user ("sharpened because edge sharpness was 0.22") | Real-ESRGAN / SwinIR super-resolution: better on photos but need GBs of weights and a GPU to be practical |
| Printed text + **fine-tuning** | **Tesseract 5** (already in the platform) | The only engine here we can fine-tune on a user's lines (`lstmtraining`), supports Japanese / Chinese / English | PaddleOCR: strong multilingual printed OCR, but heavy dependencies (PaddlePaddle) and no simple fine-tune path |
| **Handwriting** (print + cursive) | **TrOCR** `microsoft/trocr-base-handwritten` | Best open model for English handwriting; a clean line-in / text-out interface; we get a real per-line probability from it | Tesseract: poor on cursive (see numbers). PaddleOCR handwriting is weaker. EasyOCR: similar to PaddleOCR |
| Line finding for TrOCR | OpenCV projection profile (`linesplit.py`) | TrOCR reads one line; long lines are cut at blank gaps because the model squashes its input to a square | Layout-analysis models: overkill for single-column notes |

**TrOCR limits you should know:** English only; needs about 2 GB of RAM and ~1.3 GB of disk; ~0.6 s per line on a laptop CPU; it is **not** fine-tuned by the custom-training feature (that fine-tunes Tesseract). For Japanese/Chinese handwriting there is no good small open model wired in; the lab falls back to Tesseract and says so on screen.

## 3. Feature 1: handwriting recognition

* Choose **Handwriting**. If TrOCR is installed and the language is English it is used; otherwise Tesseract is used and a warning explains why. Capabilities are visible at `GET /api/web/lab/capabilities`.
* Each line comes back with a **confidence** (0-100; for TrOCR it is the geometric-mean probability of the line's tokens). Lines under the threshold (default 70, `LAB_LOW_CONFIDENCE`) are **flagged** and outlined in red.
* The text is editable, copyable, downloadable as `.txt`.
* Neat block capitals are often read as well or better in **Printed** mode.

## 4. Feature 2: blurry image enhancement

**Detection** (`quality.py`) measures: edge sharpness (steepest gradients relative to the intensity range; barely depends on font size), noise (Immerkaer estimate), contrast, uneven lighting, and median glyph height. Thresholds were calibrated on pages blurred at known strengths. In **Auto** mode the image is enhanced only if a problem is found. You can also force it **On** or turn it **Off**.

**Enhancement** (`enhance.py`), only the steps that are needed: denoise (non-local means) → shading correction → deskew (≤ 12°) → upscale small text (up to ×3) → unsharp-mask sharpening (strength follows the blur level) → CLAHE contrast for very faint pages.

**Safety net:** for Tesseract the original is read as well, and the version the engine is more confident about is kept; the screen says which and why. A fixed recipe can make a readable image worse, and this catches most (not all) of those cases.

**Measured** (`tests/eval_enhancement.py`, Tesseract/English, 3 synthetic pages per case, 14 degradation types: Gaussian blur σ 1-3.5, noise, low resolution, shadow, combinations). Character error rate, lower is better:

| Text size | No enhancement | Auto pipeline |
|---|---|---|
| 14 px | 56.9% | **43.5%** |
| 20 px | 35.8% | **19.8%** |
| 34 px | 4.9% | **0.2%** |

Biggest wins (same runs): 20 px text with σ=2 blur 28% → 2.5%; 20 px low-resolution ×0.35 90% → 15%; 14 px with a shadow gradient 23% → 0%; 14 px low-resolution ×0.5 95% → 20%. **Remaining regressions:** a few cases still get worse (14 px blur + noise went 39% → 56%, and 14 px σ=1 blur 1% → 4%); and some are hopeless either way (14 px with σ ≥ 3 blur stays above 94% error) because the confidence check is not a perfect judge. Severe blur (σ ≥ 3) is not recoverable.
These are *synthetic* degradations; real photos are messier. Re-run the script on your own kind of images before relying on it.

## 5. Feature 3: custom manual training

**Dataset** = a named, categorised set of samples. **Sample** = a picture of ONE line (8-400 px high, up to 4000 wide) + exactly what it says.

Getting samples in:
1. **By hand:** *Add sample* (image + text).
2. **Corrections (feedback loop):** in the Extraction lab, edit a line and *Save N corrections*; the line is cropped from the image the engine read and saved with the right text.
3. **Import a zip.** Layout (the export uses the same layout, so a dataset round-trips):
   ```
   labels.tsv        images/line_1.png<TAB>the exact text      (labels.csv with a comma also works; a header row is skipped)
   images/line_1.png
   dataset.json      optional: {"name": "...", "category": "handwriting", "lang": "eng", "description": "..."}
   ```
   Unsafe paths, missing images, bad images and unreadable labels are skipped and reported. Zips are size-checked before anything is unpacked.

Managing: edit a label inline, delete a sample (confirmation), search, filter datasets by category, export as zip, delete a dataset (confirmation).

**Training** (needs ≥ 10 samples): *Train* fine-tunes the dataset language's Tesseract model on 4 of every 5 samples (by id). The other 1 in 5 is never trained on; it measures the **error rate before** (the model OCR uses today) **and after**. Result per version: before → after, samples used, iterations. If it improved, the model is switched on automatically; if not, it is kept off and you are told. Switch any version on or off yourself. One training runs per dataset at a time and at most two per user.

**Example** (60 poor-quality line images, 12 held out): 35.3% → 32.9% on macOS, 35.3% → 29.8% inside the Linux container. Small hold-out sets are noisy; more and more varied samples give a more trustworthy number.

### Add your own data and retrain: the short version
1. **Learning → My training data → New dataset** (name, category, language).
2. Add 10+ samples: *Add sample*, *Import zip…*, or correct lines in the Extraction lab.
3. Open the dataset → **Train model**. Wait for the row to show *before → after*.
4. **Extraction lab → Model → My model: …** to use it. Add more samples and **Retrain** any time.

## 6. API (all `JWT`, prefix `/api/web/lab`)

| Endpoint | |
|---|---|
| `GET /capabilities` | engines available (and why not), limits, threshold |
| `POST /runs` (multipart `file`, `mode`, `lang`, `enhance`, `engine`, `dataset_id`) | start an extraction → `202`; poll `GET /runs/{id}` |
| `GET /runs`, `GET /runs/{id}`, `DELETE /runs/{id}` | history / result / delete |
| `GET /runs/{id}/pages/{n}/{original\|enhanced}` | the preview images |
| `POST /runs/{id}/corrections` | save a corrected line into a dataset |
| `GET/POST /datasets`, `GET/PATCH/DELETE /datasets/{id}` | datasets (`?category=`) |
| `POST /datasets/import`, `POST /datasets/{id}/import`, `GET /datasets/{id}/export` | zip import (new / into) and export |
| `GET/POST /datasets/{id}/samples`, `PATCH/DELETE …/samples/{sid}`, `GET …/samples/{sid}/image` | samples |
| `POST /datasets/{id}/train`, `GET /datasets/{id}/models`, `POST …/models/{mid}/activate\|deactivate` | training |

Everything is private to its owner (another user's id answers `404`).

## 7. Errors that are handled

| Situation | Result |
|---|---|
| Not an image or PDF (checked by content, not file name) | `415 unsupported_type` |
| Empty / over 15 MB / absurd pixel count | `400 empty_file` / `413 file_too_large` / `413 image_too_large` |
| Corrupt or truncated image, encrypted or empty PDF | `400` or run `failed` with `unreadable_image` / `encrypted_pdf` / `empty_pdf` |
| Bad `mode` / `lang` / `enhance` / `engine` | `422 invalid_*` |
| TrOCR requested but missing, or with a non-English language | run `failed`: `engine_unavailable` / `engine_language` |
| Engine crash | run `failed`: `extraction_failed` (message, no stack trace) |
| Image with no text | `done`, no lines, and a warning (not an error) |
| Worker died mid-run | after 15 min the run is marked `failed: timed_out` so the UI never spins forever |
| Sample is a whole page / huge / a PNG "bomb" (tiny file declaring 20000×20000 px) | `422 invalid_image` (the declared size is checked before decoding) |
| Bad zip, no labels, zip bomb, path traversal | `422 bad_zip` / `422 no_labels` / `413 zip_too_large` / entry skipped |
| Too few samples, training already running, too many trainings | `422 not_enough_samples` / `409 training_in_progress` / `429 too_many_trainings` |

## 8. Configuration (environment)

`LAB_LOW_CONFIDENCE` (70), `LAB_MIN_SAMPLES` (10), `LAB_MAX_PAGES` (3), `LAB_MAX_SIDE` (5000), `LAB_MAX_PIXELS` (40 000 000), `TROCR_MODEL` (`microsoft/trocr-base-handwritten`; `…-small-handwritten` is ~4× smaller), `LAB_TROCR_ALLOW_DOWNLOAD` (0), detector thresholds `LAB_BLUR_SOFT` / `LAB_BLUR_BLURRY` / `LAB_NOISE_HIGH` / `LAB_CONTRAST_LOW` / `LAB_TEXT_HEIGHT_MIN` / `LAB_UNEVEN_LIGHT`.

**Installing the handwriting engine**
```bash
# local venv
pip install -r requirements-handwriting.txt      # on Linux first: pip install torch --index-url https://download.pytorch.org/whl/cpu
python scripts/download_trocr.py                  # one-time, ~1.3 GB. Requests never download models.
# Docker
INSTALL_HANDWRITING=1 ENABLE_HANDWRITING=1 docker compose up -d --build     # image +~1 GB, model fetched on first start into the shared volume
```
The production compose file does **not** install it: the 1 GB server cannot run TrOCR. There, handwriting mode falls back to Tesseract with a visible warning, and the image-size limits are lowered.

## 9. Honest limits

* **Handwriting accuracy on real people's writing is not measured here.** I had no real handwriting samples. The check used computer fonts that imitate handwriting (cursive Snell Roundhand: TrOCR 1.6% error vs Tesseract 27.6%; on tidy print-style fonts Tesseract was as good or better). Real handwriting is messier. Judge it on your own pages.
* Confidence is not a guarantee: Tesseract can be confidently wrong (a blurry "date" read as "cate" at 90%). Flags catch many errors, not all.
* Single-column layouts only for handwriting (a column split is not attempted).
* Custom training improves **Tesseract**; the handwriting engine is used as-is.
* Training data is per user and is never shared with the global model.

## 10. Tests

```
backend/tests/test_lab_unit.py       59 unit tests: quality, enhancement, line finding, loading, engine choice, flags, zip safety, image bombs
backend/tests/e2e_lab.py             ~90 API checks against a running stack (extraction, errors, datasets, corrections, import/export, training before/after, caps)
backend/tests/eval_enhancement.py    the measurement behind the table above
backend/tests/lab_samples.py         synthetic pages and degradations used by all of the above
```
