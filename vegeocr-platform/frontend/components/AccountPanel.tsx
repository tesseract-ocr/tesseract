"use client";
import { FormEvent, useEffect, useState } from "react";
import { api, getSession, Scope, setSession } from "@/lib/api";
import { Badge, Err, fmtDate, Ok } from "./ui";

function PasswordField({ label, value, onChange, autoComplete, id }: { label: string; value: string; onChange: (v: string) => void; autoComplete: string; id: string }) {
  const [show, setShow] = useState(false);
  return (
    <label className="block text-sm" htmlFor={id}>{label}
      <div className="relative mt-1">
        <input id={id} name={id} type={show ? "text" : "password"} value={value} onChange={e => onChange(e.target.value)} autoComplete={autoComplete} maxLength={128} required className="input pr-16" />
        <button type="button" onClick={() => setShow(v => !v)} aria-label={`${show ? "Hide" : "Show"} ${label.toLowerCase()}`}
          className="absolute inset-y-0 right-2 my-auto h-7 rounded px-2 text-xs font-medium text-slate-500 hover:text-brand-600">{show ? "Hide" : "Show"}</button>
      </div>
    </label>
  );
}

function Rule({ ok, children }: { ok: boolean; children: string }) {
  return <li className={`flex items-center gap-2 ${ok ? "text-green-700" : "text-slate-500"}`}><span aria-hidden className="w-4 text-center">{ok ? "✓" : "○"}</span><span>{children}<span className="sr-only">{ok ? " (met)" : " (not met)"}</span></span></li>;
}

/** Profile summary + change-password form. `scope` picks which session (user app or admin app) the token belongs to. */
export default function AccountPanel({ scope = "user" }: { scope?: Scope }) {
  const [me, setMe] = useState<any>(null);
  const [current, setCurrent] = useState(""); const [next, setNext] = useState(""); const [confirm, setConfirm] = useState("");
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null); const [busy, setBusy] = useState(false);
  const loadMe = () => api("/api/web/auth/me", { scope }).then(setMe).catch(x => setErr(x.message));
  useEffect(() => { loadMe(); }, []); // eslint-disable-line react-hooks/exhaustive-deps

  const rules = {
    length: next.length >= 8,
    letter: /[A-Za-z]/.test(next),
    number: /\d/.test(next),
    different: next.length > 0 && next !== current,
    match: next.length > 0 && next === confirm,
  };
  const valid = current.length > 0 && Object.values(rules).every(Boolean);

  const submit = async (e: FormEvent) => {
    e.preventDefault(); if (!valid) return;
    setBusy(true); setErr(null); setOk(null);
    try {
      const r = await api("/api/web/auth/change-password", { scope, body: { current_password: current, new_password: next } });
      const s = getSession(scope);
      if (s) setSession(scope, { ...s, token: r.token }); // this session continues; every other one is signed out
      setCurrent(""); setNext(""); setConfirm("");
      setOk(r.message || "Password changed."); loadMe();
    } catch (x: any) { setErr(x.message); } finally { setBusy(false); }
  };

  return (
    <div className="grid gap-6 lg:grid-cols-5">
      <section className="card p-5 lg:col-span-2" aria-label="Profile">
        <h2 className="font-semibold">Your account</h2>
        {!me ? <p className="mt-3 text-sm text-slate-500">Loading…</p> : (
          <dl className="mt-3 space-y-3 text-sm">
            <div><dt className="text-xs font-semibold uppercase text-slate-500">Name</dt><dd>{me.full_name || "—"}</dd></div>
            <div><dt className="text-xs font-semibold uppercase text-slate-500">Email</dt><dd className="break-all">{me.email}</dd></div>
            {me.username && <div><dt className="text-xs font-semibold uppercase text-slate-500">Username</dt><dd>{me.username}</dd></div>}
            <div><dt className="text-xs font-semibold uppercase text-slate-500">Role</dt><dd><Badge tone={me.role === "admin" ? "blue" : "slate"}>{me.role}</Badge></dd></div>
            <div><dt className="text-xs font-semibold uppercase text-slate-500">Last sign-in</dt><dd>{fmtDate(me.last_login_at)}</dd></div>
            <div><dt className="text-xs font-semibold uppercase text-slate-500">Password last changed</dt><dd data-testid="pw-changed">{me.password_changed_at ? fmtDate(me.password_changed_at) : "Never changed"}</dd></div>
          </dl>)}
      </section>

      <form onSubmit={submit} className="card space-y-4 p-5 lg:col-span-3" aria-label="Change password">
        <div><h2 className="font-semibold">Change password</h2>
          <p className="mt-1 text-sm text-slate-500">After you change it, every other device and browser is signed out. API keys keep working.</p></div>
        <Err msg={err} /><Ok msg={ok} />
        <PasswordField id="current-password" label="Current password" value={current} onChange={setCurrent} autoComplete="current-password" />
        <PasswordField id="new-password" label="New password" value={next} onChange={setNext} autoComplete="new-password" />
        <PasswordField id="confirm-password" label="Confirm new password" value={confirm} onChange={setConfirm} autoComplete="new-password" />
        <ul className="grid gap-1 text-sm sm:grid-cols-2" aria-label="Password requirements">
          <Rule ok={rules.length}>At least 8 characters</Rule><Rule ok={rules.letter}>Contains a letter</Rule>
          <Rule ok={rules.number}>Contains a number</Rule><Rule ok={rules.different}>Different from the current one</Rule>
          <Rule ok={rules.match}>Both new passwords match</Rule>
        </ul>
        <div className="flex flex-wrap items-center gap-3">
          <button className="btn-primary" disabled={!valid || busy}>{busy ? "Changing…" : "Change password"}</button>
          <span className="text-xs text-slate-500">Forgot your current password? Sign out and use “Forgot password” on the sign-in page.</span>
        </div>
      </form>
    </div>
  );
}
