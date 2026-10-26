"""OCR lab: handwriting recognition, blurry-image enhancement and user-provided training data.

Layout (each file can be changed independently):
    quality.py   measure an image (blur, noise, contrast, text size) and decide whether it needs enhancement
    enhance.py   OpenCV enhancement pipeline (denoise, shading correction, upscale, deskew, sharpen, contrast)
    engines.py   recognition engines behind one interface: Tesseract (printed, fine-tunable) and TrOCR (handwriting, optional)
    linesplit.py find text lines in an image (used to feed line-level engines such as TrOCR)
    pipeline.py  image -> quality report -> (enhance) -> engine -> lines with confidence + flags
    datasets.py  user-owned labelled datasets: CRUD, zip import/export, saving corrections
    train.py     on-demand fine-tuning of a dataset model, with accuracy before vs after
See docs/HANDWRITING.md.
"""
