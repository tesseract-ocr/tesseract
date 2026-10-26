"""Does enhancement actually improve OCR?  Character error rate (lower is better) on synthetic text pages, raw vs enhanced.
    .venv/bin/python tests/eval_enhancement.py [--seeds 3] [--size 34]
The degradations are synthetic (Gaussian blur, added noise, downscaling, a shading gradient): real photos are messier, so treat this as a sanity check."""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from lab_samples import degrade, sample_lines, text_page  # noqa: E402
from app.config import settings  # noqa: E402
from app.ocrlab import enhance as E, pipeline as P, quality as Q  # noqa: E402
from app.ocrlab.engines import ENGINES  # noqa: E402
from app.services.training import cer  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument("--seeds", type=int, default=3)
ap.add_argument("--size", type=int, default=34)
ap.add_argument("--force", action="store_true", help="enhance every case even when the detector says the image is fine")
args = ap.parse_args()

CASES = [
    ("crisp", {}), ("blur σ=1.0", {"blur": 1.0}), ("blur σ=1.5", {"blur": 1.5}), ("blur σ=2.0", {"blur": 2.0}), ("blur σ=2.5", {"blur": 2.5}), ("blur σ=3.0", {"blur": 3.0}), ("blur σ=3.5", {"blur": 3.5}),
    ("noise σ=10", {"noise": 10}), ("noise σ=20", {"noise": 20}), ("low-res ×0.5", {"scale": 0.5}), ("low-res ×0.35", {"scale": 0.35}), ("low-res ×0.25", {"scale": 0.25}),
    ("shadow 0.5", {"shadow": 0.5}), ("blur 1.5 + noise 10", {"blur": 1.5, "noise": 10}), ("low-res ×0.5 + blur 1", {"scale": 0.5, "blur": 1.0}),
]
tess = ENGINES["tesseract"]


def read(img):
    return "\n".join(l.text for l in tess.recognise(img, "eng", psm=3, tessdata=settings.base_tessdata))


print(f"font {args.size}px, {args.seeds} pages per case, engine tesseract/eng")
print(f"{'case':26} {'detector':>34} {'CER raw':>9} {'CER enh.':>9} {'Δ':>8}   steps")
tot_raw = tot_enh = 0.0
for name, kw in CASES:
    raw_e, enh_e, detected, steps = [], [], [], []
    for seed in range(args.seeds):
        lines = sample_lines(8, seed=seed + 1)
        ref = "\n".join(lines)
        img = degrade(text_page(lines, size=args.size), seed=seed, **kw)
        rep = Q.analyse(img)
        raw_e.append(cer([(ref, read(img))]))
        out = P.process_page(img, mode="printed", lang="eng", enhance_mode="on" if args.force else "auto", engine=tess, tessdata=settings.base_tessdata)
        enh_e.append(cer([(ref, "\n".join(l.text for l in out.lines))]))
        steps = out.steps if out.enhanced is not None else (["(kept original)"] if rep.needs_enhancement else [])
        detected.append(rep.needs_enhancement)
    r, e = sum(raw_e) / len(raw_e), sum(enh_e) / len(enh_e)
    tot_raw += r; tot_enh += e
    flag = "flagged" if all(detected) else "not flagged" if not any(detected) else "mixed"
    print(f"{name:26} {flag:>34} {r*100:8.1f}% {e*100:8.1f}% {100*(e-r):+7.1f}   {', '.join(steps) if steps else '-'}")
n = len(CASES)
print(f"{'AVERAGE':26} {'':>34} {tot_raw/n*100:8.1f}% {tot_enh/n*100:8.1f}% {100*(tot_enh-tot_raw)/n:+7.1f}")
