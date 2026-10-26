"""API-key management + scopes + learning module, against a RUNNING API.
    python tests/e2e_keys_learning.py [base_url]        (ADMIN_LOGIN / ADMIN_PASSWORD env override the admin credentials)"""
import os
import sys
import time
import uuid
from datetime import datetime, timedelta, timezone

import httpx

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from app.learning_content import MODULES  # noqa: E402  (the seed data, to know the right quiz answers)

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8000"
ADMIN = (os.environ.get("ADMIN_LOGIN", "admin"), os.environ.get("ADMIN_PASSWORD", "password123"))
c = httpx.Client(base_url=BASE, timeout=60)
fails = 0


def check(name, cond, extra=""):
    global fails
    print(("PASS " if cond else "FAIL ") + name + (f"   {extra}" if extra and not cond else ""))
    fails += 0 if cond else 1


bearer = lambda t: {"Authorization": f"Bearer {t}"}  # noqa: E731
code = lambda r: (r.json().get("error") or {}).get("code") if r.headers.get("content-type", "").startswith("application/json") else None  # noqa: E731
iso = lambda dt: dt.isoformat()  # noqa: E731


def signup():
    email = f"kl{uuid.uuid4().hex[:8]}@example.com"
    r = c.post("/api/web/auth/signup", json={"email": email, "password": "Passw0rd123"})
    return r.json()["token"]


A, B = signup(), signup()
admin_tok = c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]}).json()["token"]


def mk(tok, **body):
    body.setdefault("name", "k")
    return c.post("/api/web/keys", json=body, headers=bearer(tok))


def key_header(r):
    return {"X-API-Key": r.json()["key"]}


# ===================================================== key management =====================================================
r = c.get("/api/web/keys/scopes", headers=bearer(A)).json()
check("scope catalogue hides admin from normal users", {x["scope"] for x in r} == {"read", "write", "learning:read", "learning:write"})
check("scope catalogue shows admin to admins", "admin" in {x["scope"] for x in c.get("/api/web/keys/scopes", headers=bearer(admin_tok)).json()})

r = mk(A, name="default")
check("create key (defaults)", r.status_code == 201, r.text)
k = r.json()
check("secret shown once, with the vocr_ prefix", k["key"].startswith("vocr_") and "warning" in k)
check("default scopes are read + write", k["scopes"] == ["read", "write"])
check("masked as vocr_****abcd", k["masked"] == "vocr_****" + k["key"][-4:] and k["key"] not in k["masked"], k["masked"])
check("never expires by default", k["expires_at"] is None and k["status"] == "active")
listed = c.get("/api/web/keys", headers=bearer(A)).json()
check("list never contains the secret", all("key" not in x and k["key"] not in str(x) for x in listed))
check("list has name/scopes/created/last_used/status", {"name", "scopes", "created_at", "last_used_at", "status", "masked", "request_count"} <= set(listed[0]))

now = datetime.now(timezone.utc)
for days in (30, 90, 365):
    kk = mk(A, name=f"{days}d", expires_in_days=days).json()
    delta = datetime.fromisoformat(kk["expires_at"]) - now
    check(f"expires_in_days={days}", abs(delta - timedelta(days=days)) < timedelta(minutes=2), str(delta))
custom = mk(A, name="custom", expires_at=iso(now + timedelta(days=10)))
check("custom expiry date", custom.status_code == 201 and custom.json()["expires_at"] is not None)
check("invalid preset (45 days) -> 422", code(mk(A, expires_in_days=45)) == "invalid_expiry")
check("both expiry fields -> 422", code(mk(A, expires_in_days=30, expires_at=iso(now + timedelta(days=5)))) == "invalid_expiry")
check("expiry in the past -> 422", code(mk(A, expires_at=iso(now - timedelta(days=1)))) == "invalid_expiry")
check("expiry > 10 years -> 422", code(mk(A, expires_at=iso(now + timedelta(days=4000)))) == "invalid_expiry")
check("unknown scope -> 422", code(mk(A, scopes=["read", "root"])) == "invalid_scope")
check("empty scopes -> 422", code(mk(A, scopes=[])) == "no_scopes")
check("blank name -> 422", mk(A, name="").status_code == 422)
check("admin scope refused for normal users (403)", mk(A, scopes=["admin"]).status_code == 403)
adm = mk(admin_tok, name="admin key", scopes=["admin"])
check("admin scope allowed for administrators", adm.status_code == 201 and adm.json()["scopes"] == ["admin"], adm.text)

# --- rename / ownership
check("rename", c.patch(f"/api/web/keys/{k['id']}", json={"name": "renamed"}, headers=bearer(A)).json()["name"] == "renamed")
check("rename someone else's key -> 404", c.patch(f"/api/web/keys/{k['id']}", json={"name": "x"}, headers=bearer(B)).status_code == 404)
check("rotate/disable/delete/usage of someone else's key -> 404", all(c.request(m, f"/api/web/keys/{k['id']}{p}", headers=bearer(B)).status_code == 404
                                                                    for m, p in (("POST", "/rotate"), ("POST", "/disable"), ("DELETE", ""), ("GET", "/usage"))))

# --- disable / enable
H = key_header(r)
check("active key works", c.get("/api/v1/usage", headers=H).status_code == 200)
check("disable", c.post(f"/api/web/keys/{k['id']}/disable", headers=bearer(A)).json()["status"] == "disabled")
x = c.get("/api/v1/usage", headers=H)
check("disabled key -> 401 disabled_api_key", x.status_code == 401 and code(x) == "disabled_api_key", x.text)
check("enable", c.post(f"/api/web/keys/{k['id']}/enable", headers=bearer(A)).json()["status"] == "active")
check("re-enabled key works again", c.get("/api/v1/usage", headers=H).status_code == 200)

# --- rotate
old = H
rot = c.post(f"/api/web/keys/{k['id']}/rotate", headers=bearer(A))
check("rotate returns a new secret once", rot.status_code == 200 and rot.json()["key"] != k["key"] and "warning" in rot.json())
check("rotate keeps id/name/scopes, sets rotated_at, new mask", rot.json()["id"] == k["id"] and rot.json()["name"] == "renamed" and rot.json()["scopes"] == k["scopes"]
      and rot.json()["rotated_at"] and rot.json()["masked"].endswith(rot.json()["key"][-4:]))
x = c.get("/api/v1/usage", headers=old)
check("old secret stops working immediately (401)", x.status_code == 401, x.text)
check("new secret works", c.get("/api/v1/usage", headers=key_header(rot)).status_code == 200)
H = key_header(rot)

# --- expiry enforced at request time
short = mk(A, name="short-lived", scopes=["read"], expires_at=iso(datetime.now(timezone.utc) + timedelta(seconds=3)))
check("short-lived key works before it expires", c.get("/api/v1/usage", headers=key_header(short)).status_code == 200)
time.sleep(4)
x = c.get("/api/v1/usage", headers=key_header(short))
check("expired key -> 401 expired_api_key", x.status_code == 401 and code(x) == "expired_api_key", x.text)
check("list reports it as expired", next(i for i in c.get("/api/web/keys", headers=bearer(A)).json() if i["id"] == short.json()["id"])["status"] == "expired")

# --- usage
for _ in range(3):
    c.get("/api/v1/usage", headers=H)
c.get("/api/v1/modules", headers=H)  # this key has read+write only -> 403, which is still recorded
u = c.get(f"/api/web/keys/{k['id']}/usage", headers=bearer(A)).json()
check("usage: request counts", u["requests"]["total"] >= 5 and u["requests"]["last_24h"] >= 5 and u["requests"]["errors_total"] >= 1, str(u["requests"]))
check("usage: rate limit info", u["rate_limit"]["per_minute"] == 60 and u["rate_limit"]["used_this_minute"] >= 1, str(u["rate_limit"]))
check("usage: 14-day series and recent activity", len(u["daily"]) == 14 and len(u["recent"]) >= 5 and {"at", "method", "path", "status", "latency_ms"} <= set(u["recent"][0]))
check("usage: recent activity includes the 403", any(x["path"] == "/api/v1/modules" and x["status"] == 403 for x in u["recent"]))
check("list shows last_used_at and request_count", next(i for i in c.get("/api/web/keys", headers=bearer(A)).json() if i["id"] == k["id"])["last_used_at"] is not None)

# --- delete
dk = mk(A, name="to-delete")
check("delete", c.delete(f"/api/web/keys/{dk.json()['id']}", headers=bearer(A)).status_code == 204)
check("deleted key -> 401", c.get("/api/v1/usage", headers=key_header(dk)).status_code == 401)
check("deleted key gone from list", dk.json()["id"] not in [i["id"] for i in c.get("/api/web/keys", headers=bearer(A)).json()])

# ===================================================== scope enforcement =====================================================
def scoped(tok, *scopes):
    return key_header(mk(tok, name="+".join(scopes), scopes=list(scopes)))


bad_file = {"file": ("x.png", b"not an image", "image/png")}
ro, wo, lr, lw, ad = scoped(A, "read"), scoped(A, "write"), scoped(A, "learning:read"), scoped(A, "learning:write"), key_header(adm)
x = c.post("/api/v1/ocr", files=bad_file, headers=ro)
check("read key cannot POST /ocr -> 403 insufficient_scope", x.status_code == 403 and code(x) == "insufficient_scope" and x.headers.get("x-required-scope") == "write", x.text)
check("read key can GET /usage", c.get("/api/v1/usage", headers=ro).status_code == 200)
check("read key cannot read learning -> 403", c.get("/api/v1/modules", headers=ro).status_code == 403)
x = c.post("/api/v1/ocr", files=bad_file, headers=wo)
check("write key passes the scope check on /ocr (then fails on the bad file: 415)", x.status_code == 415, x.text)
check("write key cannot GET /usage -> 403", c.get("/api/v1/usage", headers=wo).status_code == 403)
check("learning:read key lists modules", c.get("/api/v1/modules", headers=lr).status_code == 200)
check("learning:read key reads progress", c.get("/api/v1/progress", headers=lr).status_code == 200)
check("learning:read key cannot POST progress -> 403", c.post("/api/v1/progress", json={"lesson_id": 1}, headers=lr).status_code == 403)
check("learning:read key cannot submit a quiz -> 403", c.post("/api/v1/quizzes/1/submit", json={"answers": []}, headers=lr).status_code == 403)
check("learning:write key can POST progress", c.post("/api/v1/progress", json={"lesson_id": 1, "completed": False}, headers=lw).status_code == 200)
check("learning:write key cannot read modules -> 403", c.get("/api/v1/modules", headers=lw).status_code == 403)
check("admin-scope key passes everywhere", all(x.status_code != 403 for x in (
    c.get("/api/v1/usage", headers=ad), c.get("/api/v1/modules", headers=ad), c.get("/api/v1/progress", headers=ad),
    c.post("/api/v1/progress", json={"lesson_id": 1, "completed": False}, headers=ad), c.post("/api/v1/ocr", files=bad_file, headers=ad))))
x = c.get("/api/v1/modules")
check("no key -> 401 missing_api_key", x.status_code == 401 and code(x) == "missing_api_key")
check("wrong key -> 401 invalid_api_key", code(c.get("/api/v1/modules", headers={"X-API-Key": "vocr_nope"})) == "invalid_api_key")
check("401 and 403 are different failures (valid key, wrong scope is 403)", c.get("/api/v1/modules", headers=ro).status_code == 403)

# ===================================================== learning API =====================================================
fresh = scoped(B, "learning:read", "learning:write")
mods = c.get("/api/v1/modules", headers=fresh)
check("GET /modules", mods.status_code == 200 and len(mods.json()["modules"]) == len(MODULES), mods.text[:200])
m0 = mods.json()["modules"][0]
check("module has title/topic/difficulty/lesson_count", {"id", "title", "topic", "difficulty", "lesson_count", "summary"} <= set(m0) and m0["lesson_count"] == 3 and "lessons" not in m0)
check("difficulties are valid", {m["difficulty"] for m in mods.json()["modules"]} <= {"beginner", "intermediate", "advanced"})
det = c.get(f"/api/v1/modules/{m0['id']}", headers=fresh)
check("GET /modules/:id returns lessons with content", det.status_code == 200 and len(det.json()["lessons"]) == 3 and all(l["content"] for l in det.json()["lessons"]), det.text[:200])
quizzes = [l["quiz"] for m in mods.json()["modules"] for l in c.get(f"/api/v1/modules/{m['id']}", headers=fresh).json()["lessons"] if l.get("quiz")]
leak = any(k in q_ for q in quizzes for q_ in q["questions"] for k in ("correct", "explanation"))
check("quiz payloads never leak the answers", quizzes and not leak)
check("unknown module -> 404", code(c.get("/api/v1/modules/999999", headers=fresh)) == "module_not_found")
check("non-numeric module id -> 422", c.get("/api/v1/modules/abc", headers=fresh).status_code == 422)

p = c.get("/api/v1/progress", headers=fresh).json()
check("progress starts empty (not_started, 0%)", all(m["status"] == "not_started" and m["percent"] == 0 for m in p["modules"]) and p["continue"] is None and p["summary"]["not_started"] == len(MODULES))
m1 = det.json(); l1, l2, l3 = (l["id"] for l in m1["lessons"])
r1 = c.post("/api/v1/progress", json={"lesson_id": l1}, headers=fresh).json()["module"]
check("completing lesson 1 -> in_progress, 33%", r1["status"] == "in_progress" and r1["percent"] == 33 and r1["lessons_completed"] == 1, str(r1))
check("continue points at the next lesson", r1["continue_lesson_id"] == l2)
check("progress summary + continue", (lambda q: q["summary"]["in_progress"] == 1 and q["continue"]["lesson_id"] == l2)(c.get("/api/v1/progress", headers=fresh).json()))
check("un-completing works", c.post("/api/v1/progress", json={"lesson_id": l1, "completed": False}, headers=fresh).json()["module"]["percent"] == 0)
for lid in (l1, l2, l3):
    r3 = c.post("/api/v1/progress", json={"lesson_id": lid, "completed": True}, headers=fresh).json()["module"]
check("all lessons complete -> completed, 100%, nothing to continue", r3["status"] == "completed" and r3["percent"] == 100 and r3["continue_lesson_id"] is None)
check("progress filter by module", len(c.get(f"/api/v1/progress?module_id={m0['id']}", headers=fresh).json()["modules"]) == 1)
check("progress for unknown module -> 404", c.get("/api/v1/progress?module_id=999999", headers=fresh).status_code == 404)
check("progress for unknown lesson -> 404", code(c.post("/api/v1/progress", json={"lesson_id": 999999}, headers=fresh)) == "lesson_not_found")
check("bad progress body -> 422", c.post("/api/v1/progress", json={"lesson_id": "x"}, headers=fresh).status_code == 422)
C = signup()  # a brand-new user: nothing of B's (or A's) progress may be visible to them
check("progress is per user", all(m["status"] == "not_started" for m in c.get("/api/v1/progress", headers=scoped(C, "learning:read")).json()["modules"]))

# --- quizzes: module 2, lesson 1 has a 2-question quiz
m2 = c.get(f"/api/v1/modules/{mods.json()['modules'][1]['id']}", headers=fresh).json()
les = m2["lessons"][0]; quiz = les["quiz"]
truth = {q["prompt"]: q["correct"] for q in next(l for l in MODULES[1]["lessons"] if l["title"] == les["title"])["quiz"]["questions"]}
right = lambda q: truth[next(x for x in quiz["questions"] if x["id"] == q["id"])["prompt"]]  # noqa: E731
wrong = lambda q: [i for i in range(len(q["options"])) if i not in right(q)][:1]  # noqa: E731
qs = quiz["questions"]
res = c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": q["id"], "selected": wrong(q)} for q in qs]}, headers=fresh).json()
check("all wrong -> 0, not passed, lesson not completed", res["score"] == 0 and not res["passed"] and not res["lesson_completed"] and all(not x["correct"] for x in res["results"]), str(res))
check("results carry explanations but not the answers", all(x["explanation"] for x in res["results"]) and "correct_options" not in res)
res = c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": qs[0]["id"], "selected": right(qs[0])}]}, headers=fresh).json()
check("unanswered counts as wrong (1 of 2 = 50%, below the 70% pass mark)", res["score"] == 50 and res["correct"] == 1 and res["total"] == 2 and not res["passed"], str(res))
prog = [l for l in c.get(f"/api/v1/progress?module_id={m2['id']}", headers=fresh).json()["modules"][0]["lessons"] if l["lesson_id"] == les["id"]][0]
check("failing a quiz does not complete the lesson", not prog["completed"])
res = c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": q["id"], "selected": right(q)} for q in qs]}, headers=fresh).json()
check("all right -> 100%, passed, lesson completed", res["score"] == 100 and res["passed"] and res["lesson_completed"], str(res))
prog = [l for l in c.get(f"/api/v1/progress?module_id={m2['id']}", headers=fresh).json()["modules"][0]["lessons"] if l["lesson_id"] == les["id"]][0]
check("passing a quiz completes the lesson in the progress record", prog["completed"])
check("question from another quiz -> 422", code(c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": 999999, "selected": [0]}]}, headers=fresh)) == "invalid_answers")
check("duplicate answer -> 422", c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": qs[0]["id"], "selected": [0]}, {"question_id": qs[0]["id"], "selected": [1]}]}, headers=fresh).status_code == 422)
check("option index out of range -> 422", c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": qs[0]["id"], "selected": [99]}]}, headers=fresh).status_code == 422)
single = next(q for q in qs if q["kind"] == "single")
check("two options on a single-choice question -> 422", c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={"answers": [{"question_id": single["id"], "selected": [0, 1]}]}, headers=fresh).status_code == 422)
check("missing answers field -> 422", c.post(f"/api/v1/quizzes/{quiz['id']}/submit", json={}, headers=fresh).status_code == 422)
check("unknown quiz -> 404", code(c.post("/api/v1/quizzes/999999/submit", json={"answers": []}, headers=fresh)) == "quiz_not_found")

# ===================================================== learning dashboard (JWT) =====================================================
check("dashboard endpoints need a login", c.get("/api/web/learning/modules").status_code == 401)
dm = c.get("/api/web/learning/modules", headers=bearer(C)).json()
check("dashboard module list: title, topic, difficulty, lesson count, progress", len(dm) == len(MODULES) and {"title", "topic", "difficulty", "lesson_count", "progress"} <= set(dm[0]) and dm[0]["progress"]["status"] == "not_started")
dd = c.get(f"/api/web/learning/modules/{dm[0]['id']}", headers=bearer(C)).json()
check("dashboard module detail lists lessons + progress", len(dd["lessons"]) == 3 and dd["progress"]["lessons_total"] == 3 and "content" not in dd["lessons"][0])
ll = c.get(f"/api/web/learning/lessons/{dd['lessons'][1]['id']}", headers=bearer(C)).json()
check("lesson view: content, prev/next, module", ll["content"] and ll["prev_lesson_id"] == dd["lessons"][0]["id"] and ll["next_lesson_id"] == dd["lessons"][2]["id"] and ll["module"]["id"] == dd["id"])
check("opening a lesson records the resume point", c.get("/api/web/learning/progress", headers=bearer(C)).json()["continue"]["lesson_id"] == dd["lessons"][1]["id"])
qlesson = next(l for l in c.get(f"/api/web/learning/modules/{dm[0]['id']}", headers=bearer(C)).json()["lessons"] if l["has_quiz"])
ql = c.get(f"/api/web/learning/lessons/{qlesson['id']}", headers=bearer(C)).json()
check("lesson view: quiz has questions and no answers", ql["quiz"]["questions"] and not any("correct" in q or "explanation" in q for q in ql["quiz"]["questions"]))
check("dashboard quiz submit", c.post(f"/api/web/learning/quizzes/{ql['quiz']['id']}/submit", json={"answers": []}, headers=bearer(C)).json()["score"] == 0)
check("dashboard mark complete", c.post(f"/api/web/learning/lessons/{dd['lessons'][0]['id']}/complete", headers=bearer(C)).json()["module"]["lessons_completed"] == 1)
check("dashboard progress after completing", c.get("/api/web/learning/progress", headers=bearer(C)).json()["summary"]["in_progress"] == 1)
lk = mk(B, name="logged", scopes=["learning:read", "learning:write"]).json()
c.get("/api/v1/modules", headers={"X-API-Key": lk["key"]}); c.get("/api/v1/progress", headers={"X-API-Key": lk["key"]})
c.post("/api/v1/progress", json={"lesson_id": 1}, headers={"X-API-Key": lk["key"]})
paths = [x["path"] for x in c.get(f"/api/web/keys/{lk['id']}/usage", headers=bearer(B)).json()["recent"]]
check("learning API calls show up in that key's recent activity", {"/api/v1/modules", "/api/v1/progress"} <= set(paths), str(paths))

c.delete(f"/api/web/keys/{adm.json()['id']}", headers=bearer(admin_tok))  # keep the admin account under the per-user key cap
print("\nFAILED: %d" % fails if fails else "\nALL PASSED")
sys.exit(1 if fails else 0)
