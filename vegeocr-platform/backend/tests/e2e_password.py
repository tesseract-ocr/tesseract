"""Password change + session invalidation, against a RUNNING API.   python tests/e2e_password.py [base_url]"""
import os
import sys
import uuid

import httpx

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8000"
ADMIN = (os.environ.get("ADMIN_LOGIN", "admin"), os.environ.get("ADMIN_PASSWORD", "password123"))
c = httpx.Client(base_url=BASE, timeout=60)
fails = 0


def check(name, cond, extra=""):
    global fails
    print(("PASS " if cond else "FAIL ") + name + (f"   {extra}" if extra and not cond else ""))
    fails += 0 if cond else 1


bearer = lambda t: {"Authorization": f"Bearer {t}"}  # noqa: E731
code = lambda r: (r.json().get("error") or {}).get("code")  # noqa: E731
change = lambda tok, cur, new: c.post("/api/web/auth/change-password", json={"current_password": cur, "new_password": new}, headers=bearer(tok))  # noqa: E731

OLD, NEW = "Passw0rd123", "BetterPass456"


def new_user():
    email = f"pw{uuid.uuid4().hex[:8]}@example.com"
    t = c.post("/api/web/auth/signup", json={"email": email, "password": OLD}).json()["token"]
    return email, t


email, t1 = new_user()
t2 = c.post("/api/web/auth/login", json={"email": email, "password": OLD}).json()["token"]  # a second "device"
other_email, other = new_user()
key = c.post("/api/web/keys", json={"name": "survives", "scopes": ["read"]}, headers=bearer(t1)).json()["key"]

check("needs a login", c.post("/api/web/auth/change-password", json={"current_password": OLD, "new_password": NEW}).status_code == 401)
check("missing fields -> 422", c.post("/api/web/auth/change-password", json={}, headers=bearer(t1)).status_code == 422)
r = change(t1, "WrongPass999", NEW)
check("wrong current password -> 400 incorrect_password", r.status_code == 400 and code(r) == "incorrect_password", r.text)
check("...and the session is NOT logged out by that", c.get("/api/web/auth/me", headers=bearer(t1)).status_code == 200)
for weak, why in (("short1", "too short"), ("nodigitshere", "no digit"), ("12345678", "no letter")):
    r = change(t1, OLD, weak)
    check(f"weak new password rejected ({why})", r.status_code == 422 and code(r) == "weak_password", r.text)
r = change(t1, OLD, OLD)
check("same password rejected", r.status_code == 422 and code(r) == "same_password", r.text)
check("over-long password rejected", change(t1, OLD, "a1" * 100).status_code == 422)
check("password unchanged after all the failures", c.post("/api/web/auth/login", json={"email": email, "password": OLD}).status_code == 200)

r = change(t1, OLD, NEW)
check("change succeeds", r.status_code == 200 and r.json()["ok"] and r.json()["token"], r.text)
t3 = r.json()["token"]
check("this session continues with the fresh token", c.get("/api/web/auth/me", headers=bearer(t3)).status_code == 200)
x = c.get("/api/web/auth/me", headers=bearer(t1))
check("the token used for the change is signed out (401 session_expired)", x.status_code == 401 and code(x) == "session_expired", x.text)
check("another device's token is signed out too", c.get("/api/web/jobs", headers=bearer(t2)).status_code == 401)
check("old password no longer logs in", c.post("/api/web/auth/login", json={"email": email, "password": OLD}).status_code == 401)
check("new password logs in", c.post("/api/web/auth/login", json={"email": email, "password": NEW}).status_code == 200)
check("/me reports when the password was changed", c.get("/api/web/auth/me", headers=bearer(t3)).json()["password_changed_at"] is not None)
check("other users are unaffected", c.get("/api/web/auth/me", headers=bearer(other)).status_code == 200)
check("API keys keep working (they are not tied to the password)", c.get("/api/v1/usage", headers={"X-API-Key": key}).status_code == 200)

# changing it again works and again invalidates the previous token
t4 = change(t3, NEW, "Another789Pass").json()["token"]
check("a second change works and invalidates the previous token", c.get("/api/web/auth/me", headers=bearer(t3)).status_code == 401 and c.get("/api/web/auth/me", headers=bearer(t4)).status_code == 200)

# throttle: five wrong guesses per 15 minutes, per account
_, victim = new_user()
codes = [change(victim, "WrongGuess1", NEW).status_code for _ in range(5)]
r = change(victim, OLD, NEW)  # even the CORRECT current password is refused once throttled
check("5 wrong guesses are answered normally (400)", codes == [400] * 5, str(codes))
check("the 6th attempt is throttled (429), even with the right password", r.status_code == 429 and code(r) == "too_many_attempts", r.text)

# admin resets someone's password -> their sessions end
admin = c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]}).json()["token"]
uid = next(u["id"] for u in c.get("/api/admin/users?q=" + other_email, headers=bearer(admin)).json()["items"] if u["email"] == other_email)
check("admin sets a user's password", c.patch(f"/api/admin/users/{uid}", json={"password": "AdminSet123x"}, headers=bearer(admin)).status_code == 200)
check("...which signs that user's old session out", c.get("/api/web/auth/me", headers=bearer(other)).status_code == 401)
check("...and the new password works", c.post("/api/web/auth/login", json={"email": other_email, "password": "AdminSet123x"}).status_code == 200)
# the admin can change their own password too (round trip back to the original so the account is left untouched)
r = change(admin, ADMIN[1], "TempAdminPass1x")
check("admin can change their own password", r.status_code == 200, r.text)
back = change(r.json()["token"], "TempAdminPass1x", ADMIN[1])
check("...and change it back", back.status_code == 200 and c.post("/api/web/auth/admin-login", json={"email": ADMIN[0], "password": ADMIN[1]}).status_code == 200, back.text)

# reset flow (only when the server runs in dev mode and echoes the token)
rf = c.post("/api/web/auth/forgot", json={"email": other_email}).json()
if "dev_token" in rf:
    old_tok = c.post("/api/web/auth/login", json={"email": other_email, "password": "AdminSet123x"}).json()["token"]
    c.post("/api/web/auth/reset", json={"token": rf["dev_token"], "password": "ResetPass123x"})
    check("a password RESET also signs out existing sessions", c.get("/api/web/auth/me", headers=bearer(old_tok)).status_code == 401)
else:
    print("SKIP reset-invalidates-sessions (server is not in RETURN_RESET_TOKEN dev mode)")

print("\nFAILED: %d" % fails if fails else "\nALL PASSED")
sys.exit(1 if fails else 0)
