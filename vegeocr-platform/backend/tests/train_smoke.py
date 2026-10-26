"""Training pipeline smoke test:  python tests/train_smoke.py LINES_DIR
Submits labelled line images through the public API, approves them as admin, triggers a retrain and waits."""
import os
import sys
import time

import httpx

BASE = os.environ.get("BASE", "http://localhost:8000")
d = sys.argv[1]
ADMIN = (os.environ.get("ADMIN_LOGIN", "admin"), os.environ.get("ADMIN_PASSWORD", "password123"))
c = httpx.Client(base_url=BASE, timeout=600)
tok = c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]}).json()["token"]
A = {"Authorization": f"Bearer {tok}"}
key = c.post("/api/web/keys", json={"name": "train-smoke"}, headers=A).json()
c.patch(f"/api/admin/keys/{key['id']}", json={"rate_limit_per_min": 1000, "monthly_quota": 100000}, headers=A)
H = {"X-API-Key": key["key"]}
rows = [l.rstrip("\n").split("\t") for l in open(f"{d}/labels.tsv", encoding="utf-8") if l.strip()]
ids = []
for name, text in rows:
    r = c.post("/api/v1/training-data", files={"image": (name, open(f"{d}/{name}", "rb"))}, data={"text": text, "lang": "jpn"}, headers=H)
    r.raise_for_status()
    ids.append(r.json()["id"])
print("submitted", len(ids))
print("approve:", c.post("/api/admin/training-data/review", json={"ids": ids, "action": "approve"}, headers=A).json())
c.put("/api/admin/training-settings", json={"retrain_max_iterations": int(os.environ.get("ITERS", 300)), "auto_retrain_enabled": False}, headers=A)
run = c.post("/api/admin/models/retrain", json={"lang": "jpn"}, headers=A).json()
print("run", run)
for _ in range(600):
    r = next(x for x in c.get("/api/admin/training-runs", headers=A).json() if x["id"] == run["run_id"])
    if r["status"] not in ("queued", "running"):
        break
    time.sleep(3)
print(r)
for m in c.get("/api/admin/models", headers=A).json():
    if m["lang"] == "jpn":
        print(m["version"], m["status"], m["cer"], m["baseline_cer"], m["train_samples"], m["test_samples"], m["notes"])
