"""Checks that every example response in frontend/lib/api-docs.json has the same shape as the real API's response.
    python tests/verify_api_docs.py [base_url]       (needs a running API; ADMIN_LOGIN / ADMIN_PASSWORD env as usual)"""
import json
import os
import sys
import time
import uuid

import httpx

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8000"
ADMIN = (os.environ.get("ADMIN_LOGIN", "admin"), os.environ.get("ADMIN_PASSWORD", "password123"))
HERE = os.path.dirname(__file__)
docs = json.load(open(os.path.join(HERE, "..", "..", "frontend", "lib", "api-docs.json"), encoding="utf-8"))
by_key = {f"{e['method']} {e['path']}": e for e in docs["endpoints"]}
c = httpx.Client(base_url=BASE, timeout=120)
fails = 0


def shape_diff(doc, live, path="$"):
    """Differences in structure (keys and JSON types), ignoring values. null in either side matches anything."""
    if doc is None or live is None:
        return []
    if isinstance(doc, dict) and isinstance(live, dict):
        out = [f"{path}: documented but not returned: {sorted(set(doc) - set(live))}" for _ in [0] if set(doc) - set(live)]
        out += [f"{path}: returned but not documented: {sorted(set(live) - set(doc))}" for _ in [0] if set(live) - set(doc)]
        for k in set(doc) & set(live):
            out += shape_diff(doc[k], live[k], f"{path}.{k}")
        return out
    if isinstance(doc, list) and isinstance(live, list):
        return shape_diff(doc[0], live[0], f"{path}[0]") if doc and live else []
    kinds = lambda v: "number" if isinstance(v, (int, float)) and not isinstance(v, bool) else type(v).__name__  # noqa: E731
    return [] if kinds(doc) == kinds(live) else [f"{path}: documented {kinds(doc)}, returned {kinds(live)}"]


def compare(name, endpoint, response, status_expected):
    global fails
    doc = next(r for r in endpoint["responses"] if r["status"] == status_expected and r["label"] == name)
    diffs = shape_diff(doc["body"], response.json())
    ok = response.status_code == status_expected and not diffs
    fails += 0 if ok else 1
    print(("PASS " if ok else "FAIL ") + f"{endpoint['method']} {endpoint['path']} ({status_expected} {name})" + ("" if ok else f"  status={response.status_code} {diffs}"))


tok = c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]}).json()["token"]
AUTH = {"Authorization": f"Bearer {tok}"}
# Keys are capped per user, so remove what earlier runs of the test suites left behind and clean up after ourselves.
for old in c.get("/api/web/keys", headers=AUTH).json():
    if old["name"] in ("docs-check", "ro", "x", "admin key"):
        c.delete(f"/api/web/keys/{old['id']}", headers=AUTH)


def temp_key(scopes):
    r = c.post("/api/web/keys", json={"name": "x", "scopes": scopes}, headers=AUTH)
    return r.json()["id"], {"X-API-Key": r.json()["key"]}
key = c.post("/api/web/keys", json={"name": "docs-check", "scopes": ["admin"]}, headers=AUTH).json()
H = {"X-API-Key": key["key"]}
sample = open(os.path.join(HERE, "sample.png"), "rb").read()
unique = lambda: sample + uuid.uuid4().bytes  # noqa: E731

# --- OCR
e = by_key["POST /api/v1/ocr"]
r = c.post("/api/v1/ocr", files={"file": ("a.png", unique(), "image/png")}, data={"lang": "eng"}, headers=H)
compare("Finished", e, r, 200)
job_id = r.json()["id"]
r = c.post("/api/v1/ocr", files={"file": ("a.png", unique(), "image/png")}, data={"lang": "eng", "async": "true"}, headers=H)
compare("Queued (large file or async=true)", e, r, 202)
queued_id = r.json()["id"]
e = by_key["GET /api/v1/jobs/{id}"]
compare("Done", e, c.get(f"/api/v1/jobs/{job_id}", headers=H), 200)
live = c.get(f"/api/v1/jobs/{queued_id}", headers=H)
if live.json()["status"] == "queued":
    compare("Still queued", e, live, 200)
else:
    print("SKIP GET /api/v1/jobs/{id} (Still queued): the worker already picked it up")
compare("Accepted for review", by_key["POST /api/v1/training-data"], c.post("/api/v1/training-data", files={"image": ("l.png", unique(), "image/png")}, data={"text": "test", "lang": "jpn"}, headers=H), 201)
compare("OK", by_key["GET /api/v1/usage"], c.get("/api/v1/usage", headers=H), 200)

# --- Learning
compare("OK", by_key["GET /api/v1/modules"], c.get("/api/v1/modules", headers=H), 200)
compare("OK", by_key["GET /api/v1/modules/{id}"], c.get("/api/v1/modules/1", headers=H), 200)
compare("OK", by_key["GET /api/v1/progress"], c.get("/api/v1/progress?module_id=1", headers=H), 200)
compare("OK", by_key["POST /api/v1/progress"], c.post("/api/v1/progress", json={"lesson_id": 1, "completed": True}, headers=H), 200)
q = c.get("/api/v1/modules/1", headers=H).json()["lessons"][0]["quiz"]
compare("OK", by_key["POST /api/v1/quizzes/{id}/submit"], c.post(f"/api/v1/quizzes/{q['id']}/submit", json={"answers": [{"question_id": x["id"], "selected": [0]} for x in q["questions"]]}, headers=H), 200)

# --- the documented error example matches a real 403
low_id, low_h = temp_key(["learning:read"])
r = c.post("/api/v1/progress", json={"lesson_id": 1}, headers=low_h)
c.delete(f"/api/web/keys/{low_id}", headers=AUTH)
diffs = shape_diff(docs["errorExample"], r.json())
ok = r.status_code == 403 and not diffs and r.json()["error"]["code"] == docs["errorExample"]["error"]["code"]
fails += 0 if ok else 1
print(("PASS " if ok else "FAIL ") + "documented error example matches a real 403" + ("" if ok else f" {r.status_code} {r.text}"))

# --- every documented endpoint really requires the scope it documents
for k, e in by_key.items():
    if e["scope"] == "admin":
        continue
    # a key holding every scope EXCEPT the documented one must be refused with 403
    others = [s for s in ("read", "write", "learning:read", "learning:write") if s != e["scope"]]
    kid, h = temp_key(others)
    path = e["path"].replace("{id}", "1")
    r = c.request(e["method"], path, headers=h, json={} if e["method"] == "POST" and "ocr" not in path and "training" not in path else None)
    c.delete(f"/api/web/keys/{kid}", headers=AUTH)
    ok = r.status_code == 403
    fails += 0 if ok else 1
    print(("PASS " if ok else "FAIL ") + f"{k}: a key without '{e['scope']}' gets 403" + ("" if ok else f" (got {r.status_code})"))

c.delete(f"/api/web/keys/{key['id']}", headers=AUTH)
print("\nFAILED: %d" % fails if fails else "\nALL DOCUMENTED EXAMPLES MATCH THE LIVE API")
sys.exit(1 if fails else 0)
