"""End-to-end check against a RUNNING stack:  python tests/e2e.py [base_url] [sample_image.png]
Needs the API + worker running and the admin account from .env."""
import os
import sys
import time
import uuid

import httpx

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8000"
IMG = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), "sample.png")
ADMIN = (os.environ.get("ADMIN_LOGIN", "admin"), os.environ.get("ADMIN_PASSWORD", "password123"))
c = httpx.Client(base_url=BASE, timeout=120)
fails = 0


def check(name, cond, extra=""):
    global fails
    print(("PASS " if cond else "FAIL ") + name + (f"  {extra}" if extra and not cond else ""))
    fails += 0 if cond else 1


def auth(tok):
    return {"Authorization": f"Bearer {tok}"}


email = f"user{uuid.uuid4().hex[:6]}@example.com"
r = c.post("/api/web/auth/signup", json={"email": email, "password": "weak"})
check("weak password rejected", r.status_code == 422, r.text)
r = c.post("/api/web/auth/signup", json={"email": email, "password": "Passw0rd123", "full_name": "Test"})
check("signup", r.status_code == 201, r.text)
r = c.post("/api/web/auth/login", json={"email": email, "password": "wrong-pass1"})
check("bad password -> 401", r.status_code == 401)
tok = c.post("/api/web/auth/login", json={"email": email, "password": "Passw0rd123"}).json()["token"]
check("me", c.get("/api/web/auth/me", headers=auth(tok)).json()["email"] == email)
check("no token -> 401", c.get("/api/web/jobs").status_code == 401)

# forgot / reset (the token is only echoed when the server runs with RETURN_RESET_TOKEN=true, i.e. dev)
r = c.post("/api/web/auth/forgot", json={"email": email}).json()
check("forgot answers generically", r.get("ok") is True)
check("forgot unknown email same shape", c.post("/api/web/auth/forgot", json={"email": "nobody@example.com"}).json().get("ok"))
check("reset bad token", c.post("/api/web/auth/reset", json={"token": "x", "password": "NewPassw0rd1"}).status_code == 400)
password = "Passw0rd123"
if "dev_token" in r:
    check("reset ok", c.post("/api/web/auth/reset", json={"token": r["dev_token"], "password": "NewPassw0rd1"}).status_code == 200)
    check("reset token single-use", c.post("/api/web/auth/reset", json={"token": r["dev_token"], "password": "NewPassw0rd2"}).status_code == 400)
    password = "NewPassw0rd1"
else:
    print("SKIP reset happy-path (server is not in RETURN_RESET_TOKEN dev mode)")
tok = c.post("/api/web/auth/login", json={"email": email, "password": password}).json()["token"]

# upload + OCR
files = {"file": ("sample.png", open(IMG, "rb"), "image/png")}
r = c.post("/api/web/jobs", files=files, data={"lang": "jpn+eng"}, headers=auth(tok))
check("upload image", r.status_code == 201 and r.json()["status"] == "done", r.text)
job = r.json()
check("has text + confidence", bool(job.get("text")) and job.get("confidence") is not None, str(job)[:200])
print("   OCR text:", repr(job.get("text")))
check("bad file type rejected", c.post("/api/web/jobs", files={"file": ("a.png", b"not an image at all", "image/png")},
                                      headers=auth(tok)).status_code == 415)
check("bad lang rejected", c.post("/api/web/jobs", files={"file": ("a.png", open(IMG, "rb"), "image/png")},
                                  data={"lang": "klingon"}, headers=auth(tok)).status_code == 422)
hist = c.get("/api/web/jobs", headers=auth(tok)).json()
check("history lists job", hist["total"] == 1)
for fmt, magic in (("txt", None), ("json", b"{"), ("pdf", b"%PDF")):
    r = c.get(f"/api/web/jobs/{job['id']}/export?format={fmt}", headers=auth(tok))
    check(f"export {fmt}", r.status_code == 200 and (magic is None or r.content.startswith(magic)), r.text[:100])
check("page image", c.get(f"/api/web/jobs/{job['id']}/pages/1/image", headers=auth(tok)).status_code == 200)

# corrections -> training samples
line0 = job["result"][0]["lines"][0]["text"]
r = c.patch(f"/api/web/jobs/{job['id']}/lines", json={"edits": [{"page": 1, "index": 0, "text": line0 + "X"}]}, headers=auth(tok))
check("line correction creates sample", r.status_code == 200 and r.json()["training_samples_created"] == 1, r.text)
check("edited text saved", c.get(f"/api/web/jobs/{job['id']}", headers=auth(tok)).json()["text"].startswith(line0 + "X"))

# isolation between users
email2 = f"user{uuid.uuid4().hex[:6]}@example.com"
tok2 = c.post("/api/web/auth/signup", json={"email": email2, "password": "Passw0rd123"}).json()["token"]
check("other user cannot read job", c.get(f"/api/web/jobs/{job['id']}", headers=auth(tok2)).status_code == 404)
check("other user cannot delete job", c.delete(f"/api/web/jobs/{job['id']}", headers=auth(tok2)).status_code == 404)
check("user blocked from admin", c.get("/api/admin/users", headers=auth(tok)).status_code == 403)
check("user blocked from admin login", c.post("/api/web/auth/admin-login", json={"email": email, "password": password}).status_code == 403)

# API keys
r = c.post("/api/web/keys", json={"name": "test"}, headers=auth(tok))
check("create key", r.status_code == 201 and r.json()["key"].startswith("vocr_"), r.text)
key = r.json()
check("key list hides secret", "key" not in c.get("/api/web/keys", headers=auth(tok)).json()[0])
H = {"X-API-Key": key["key"]}
check("api no key -> 401", c.post("/api/v1/ocr", files={"file": ("a.png", open(IMG, "rb"))}).status_code == 401)
check("api bad key -> 401", c.post("/api/v1/ocr", files={"file": ("a.png", open(IMG, "rb"))}, headers={"X-API-Key": "nope"}).status_code == 401)
r = c.post("/api/v1/ocr", files={"file": ("a.png", open(IMG, "rb"))}, data={"lang": "jpn+eng"}, headers=H)
check("api ocr sync", r.status_code == 200 and r.json()["status"] == "done" and r.json()["text"], r.text[:200])
check("rate-limit headers", "x-ratelimit-remaining" in r.headers)
r = c.post("/api/v1/ocr", files={"file": ("a.png", open(IMG, "rb"))}, data={"async": "true"}, headers=H)
check("api ocr async -> 202", r.status_code == 202 and r.json()["status"] == "queued", r.text[:200])
jid = r.json()["id"]
for _ in range(60):
    st = c.get(f"/api/v1/jobs/{jid}", headers=H).json()
    if st["status"] in ("done", "failed"):
        break
    time.sleep(1)
check("async job completes via worker", st["status"] == "done", str(st)[:200])
check("job of another key's owner hidden", c.get(f"/api/v1/jobs/{jid}", headers={"X-API-Key": c.post("/api/web/keys", json={"name": "k2"}, headers=auth(tok2)).json()["key"]}).status_code == 404)
r = c.post("/api/v1/training-data", files={"image": ("l.png", open(IMG, "rb"))}, data={"text": "test", "lang": "jpn"}, headers=H)
check("training-data accepted", r.status_code == 201, r.text)
r = c.post("/api/v1/training-data", files={"image": ("l.png", open(IMG, "rb"))}, data={"text": "", "lang": "jpn"}, headers=H)
check("training-data empty text rejected", r.status_code == 422, r.text)
u = c.get("/api/v1/usage", headers=H).json()
check("usage", u["total_requests"] >= 3 and u["quota"]["used"] >= 3, str(u))

# admin
a = c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]})
check("admin login", a.status_code == 200, a.text)
AT = auth(a.json()["token"])
users = c.get("/api/admin/users", headers=AT).json()
check("admin lists users", users["total"] >= 3)
uid = next(x["id"] for x in c.get(f"/api/admin/users?q={email}", headers=AT).json()["items"] if x["email"] == email)  # search: the list is paged and test users pile up
check("admin suspends user", c.patch(f"/api/admin/users/{uid}", json={"status": "suspended"}, headers=AT).status_code == 200)
check("suspended cannot login", c.post("/api/web/auth/login", json={"email": email, "password": password}).status_code == 403)
check("suspended token rejected", c.get("/api/web/auth/me", headers=auth(tok)).status_code == 403)
check("suspended key rejected", c.get("/api/v1/usage", headers=H).status_code == 403)
c.patch(f"/api/admin/users/{uid}", json={"status": "active"}, headers=AT)
check("admin sets key limits", c.patch(f"/api/admin/keys/{key['id']}", json={"rate_limit_per_min": 3, "monthly_quota": 1000}, headers=AT).status_code == 200)
codes = [c.get("/api/v1/usage", headers=H).status_code for _ in range(6)]
check("rate limit kicks in (429)", 429 in codes, str(codes))
r = c.get("/api/v1/usage", headers=H)
check("429 body shape", r.json().get("error", {}).get("code") == "rate_limited" and "retry-after" in r.headers, r.text)
c.patch(f"/api/admin/keys/{key['id']}", json={"rate_limit_per_min": 100, "monthly_quota": 2}, headers=AT)
time.sleep(0.2)
r = c.post("/api/v1/ocr", files={"file": ("a.png", open(IMG, "rb"))}, headers=H)
check("quota exceeded (429)", r.status_code == 429 and r.json()["error"]["code"] == "quota_exceeded", r.text[:150])
check("admin revokes key", c.post(f"/api/admin/keys/{key['id']}/revoke", headers=AT).status_code == 200)
check("revoked key -> 401", c.get("/api/v1/usage", headers=H).status_code == 401)
check("admin usage", c.get("/api/admin/usage", headers=AT).json()["total"] > 0)
check("admin overview", "jobs" in c.get("/api/admin/overview", headers=AT).json())
check("admin models list", len(c.get("/api/admin/models", headers=AT).json()) >= 1)
pending = c.get("/api/admin/training-data?status=pending", headers=AT).json()
check("admin sees pending samples", pending["total"] >= 2)
check("sample image served", c.get(f"/api/admin/training-data/{pending['items'][0]['id']}/image", headers=AT).status_code == 200)
check("admin cannot delete self", c.delete("/api/admin/users/1", headers=AT).status_code in (409, 404))
check("admin deletes user2", c.delete(f"/api/admin/users/{next(x['id'] for x in c.get(f'/api/admin/users?q={email2}', headers=AT).json()['items'] if x['email'] == email2)}", headers=AT).status_code == 204)
print("\nFAILED: %d" % fails if fails else "\nALL PASSED")
sys.exit(1 if fails else 0)
