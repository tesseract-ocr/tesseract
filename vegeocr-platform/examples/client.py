"""Minimal Python client for the VegeOCR public API.

    pip install requests
    export VEGEOCR_URL=http://localhost:8000 VEGEOCR_KEY=vocr_xxx
    python client.py ocr invoice.pdf            # extract text (polls automatically for big files)
    python client.py train line.png "請求書"      # submit a corrected line for model improvement
    python client.py usage
"""
import os
import sys
import time

import requests

BASE = os.environ.get("VEGEOCR_URL", "http://localhost:8000")
HEADERS = {"X-API-Key": os.environ["VEGEOCR_KEY"]}


class ApiError(Exception):
    pass


def _check(r):
    if r.status_code == 429:  # rate limited: honour Retry-After
        raise ApiError(f"rate limited, retry in {r.headers.get('Retry-After', '?')}s")
    if not r.ok:
        err = r.json().get("error", {})
        raise ApiError(f"{r.status_code} {err.get('code')}: {err.get('message')}")
    return r.json()


def ocr(path, lang="jpn+eng", wait=True, timeout=300):
    with open(path, "rb") as f:
        job = _check(requests.post(f"{BASE}/api/v1/ocr", headers=HEADERS, files={"file": f}, data={"lang": lang}))
    deadline = time.time() + timeout
    while wait and job["status"] in ("queued", "processing"):  # 202: large file, queued on a worker
        if time.time() > deadline:
            raise ApiError("timed out waiting for the job")
        time.sleep(2)
        job = _check(requests.get(f"{BASE}/api/v1/jobs/{job['id']}", headers=HEADERS))
    if job["status"] == "failed":
        raise ApiError(job.get("error", "OCR failed"))
    return job


def submit_training_line(image_path, text, lang="jpn"):
    """image_path must be a crop of ONE text line; text is exactly what it says."""
    with open(image_path, "rb") as f:
        return _check(requests.post(f"{BASE}/api/v1/training-data", headers=HEADERS, files={"image": f},
                                    data={"text": text, "lang": lang}))


def usage():
    return _check(requests.get(f"{BASE}/api/v1/usage", headers=HEADERS))


if __name__ == "__main__":
    cmd, *a = sys.argv[1:] or ["usage"]
    if cmd == "ocr":
        res = ocr(a[0], *(a[1:2]))
        print(f"confidence: {res['confidence']}\n{res['text']}")
    elif cmd == "train":
        print(submit_training_line(a[0], a[1]))
    else:
        print(usage())
