"use client";
import { FormEvent, useCallback, useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { ConfirmDialog, Modal } from "@/components/Modal";
import { Badge, Empty, Err, fmtDate, fmtDay, Ok, Spinner, Stat, statusTone } from "@/components/ui";
import { api } from "@/lib/api";
import { useApiBase } from "@/lib/useApiBase";

type Key = {
  id: number; name: string; masked: string; scopes: string[]; status: "active" | "disabled" | "expired" | "revoked";
  created_at: string; last_used_at: string | null; expires_at: string | null; rotated_at: string | null; request_count: number;
};
type Scope = { scope: string; description: string };
type Dialog =
  | { kind: "rename"; key: Key } | { kind: "rotate"; key: Key } | { kind: "disable"; key: Key } | { kind: "delete"; key: Key } | { kind: "usage"; key: Key };

const EXPIRY = [["never", "Never"], ["30", "30 days"], ["90", "90 days"], ["365", "365 days"], ["custom", "Custom date…"]] as const;

function CopyButton({ text, label = "Copy" }: { text: string; label?: string }) {
  const [done, setDone] = useState(false);
  const copy = async () => {
    try { await navigator.clipboard.writeText(text); }
    catch { const t = document.createElement("textarea"); t.value = text; document.body.appendChild(t); t.select(); document.execCommand("copy"); t.remove(); }
    setDone(true); setTimeout(() => setDone(false), 1500);
  };
  return <button type="button" className="btn-ghost btn-sm" onClick={copy}>{done ? "Copied ✓" : label}</button>;
}

function UsageDialog({ k, onClose }: { k: Key; onClose: () => void }) {
  const [u, setU] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  useEffect(() => { api(`/api/web/keys/${k.id}/usage`).then(setU).catch(x => setErr(x.message)); }, [k.id]);
  const max = Math.max(1, ...(u?.daily || []).map((d: any) => d.requests));
  return (
    <Modal title={`Usage: ${k.name}`} onClose={onClose} wide>
      <Err msg={err} />
      {!u ? !err && <Spinner /> : (
        <div className="space-y-5">
          <div className="grid grid-cols-2 gap-3 sm:grid-cols-4">
            <Stat label="Total requests" value={u.requests.total} sub={`${u.requests.errors_total} error${u.requests.errors_total === 1 ? "" : "s"}`} />
            <Stat label="Last 24 hours" value={u.requests.last_24h} />
            <Stat label="Last 7 days" value={u.requests.last_7d} />
            <Stat label="Last 30 days" value={u.requests.last_30d} />
          </div>
          <div className="grid gap-3 sm:grid-cols-2">
            <div className="card p-4 text-sm"><div className="text-xs font-semibold uppercase text-slate-500">Rate limit</div>
              <div className="mt-1 text-lg font-bold">{u.rate_limit.per_minute} / minute</div>
              <div className="text-slate-500">{u.rate_limit.used_this_minute} used in the current minute</div></div>
            <div className="card p-4 text-sm"><div className="text-xs font-semibold uppercase text-slate-500">Monthly quota (OCR + training data)</div>
              <div className="mt-1 text-lg font-bold">{u.quota.used} / {u.quota.monthly}</div>
              <div className="mt-1 h-2 rounded bg-slate-100"><div className="h-2 rounded bg-brand-500" style={{ width: `${Math.min(100, (100 * u.quota.used) / Math.max(1, u.quota.monthly))}%` }} /></div></div>
          </div>
          <div>
            <div className="mb-1 text-xs font-semibold uppercase text-slate-500">Requests per day (14 days)</div>
            <div className="flex h-24 items-end gap-1" role="img" aria-label="Requests per day">
              {u.daily.map((d: any) => (
                <div key={d.date} className="flex h-full flex-1 items-end" title={`${d.date}: ${d.requests} requests, ${d.errors} errors`}>
                  <div className="w-full rounded-t bg-brand-500" style={{ height: `${(100 * d.requests) / max}%`, minHeight: d.requests ? 3 : 1, opacity: d.requests ? 1 : 0.25 }} />
                </div>))}
            </div>
          </div>
          <div>
            <div className="mb-1 text-xs font-semibold uppercase text-slate-500">Recent activity</div>
            {u.recent.length === 0 ? <div className="rounded-lg border border-dashed p-4 text-center text-sm text-slate-500">No requests yet.</div> : (
              <div className="overflow-x-auto rounded-lg border border-slate-200">
                <table className="w-full text-xs"><thead className="bg-slate-50"><tr><th className="th">Time</th><th className="th">Request</th><th className="th">Status</th><th className="th">Latency</th></tr></thead>
                  <tbody className="divide-y divide-slate-100">{u.recent.map((r: any, i: number) => (
                    <tr key={i}><td className="td whitespace-nowrap text-slate-500">{fmtDate(r.at)}</td><td className="td font-mono">{r.method} {r.path}</td>
                      <td className="td"><Badge tone={r.status < 400 ? "green" : r.status < 500 ? "amber" : "red"}>{r.status}</Badge></td><td className="td">{r.latency_ms} ms</td></tr>))}</tbody></table>
              </div>)}
          </div>
        </div>)}
    </Modal>
  );
}

export default function ApiKeysPage() {
  return <Shell><Body /></Shell>;
}

function Body() {
  const API = useApiBase();
  const [keys, setKeys] = useState<Key[] | null>(null);
  const [scopes, setScopes] = useState<Scope[]>([]);
  const [fresh, setFresh] = useState<{ key: string; name: string; rotated?: boolean } | null>(null);
  const [dialog, setDialog] = useState<Dialog | null>(null);
  const [err, setErr] = useState<string | null>(null);
  const [ok, setOk] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);
  const [expiry, setExpiry] = useState<string>("never");
  const [picked, setPicked] = useState<string[]>(["read", "write"]);
  const [newName, setNewName] = useState("");

  const load = useCallback(() => api("/api/web/keys").then(setKeys).catch(x => setErr(x.message)), []);
  useEffect(() => { load(); api("/api/web/keys/scopes").then(setScopes).catch(() => {}); }, [load]);

  const create = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null); setOk(null);
    const f = new FormData(e.currentTarget);
    const body: any = { name: String(f.get("name") || ""), scopes: picked };
    if (expiry === "custom") {
      const d = String(f.get("custom_date") || "");
      if (!d) { setErr("Pick an expiration date"); return; }
      body.expires_at = new Date(`${d}T23:59:59`).toISOString(); // end of that day, local time
    } else if (expiry !== "never") body.expires_in_days = Number(expiry);
    if (!picked.length) { setErr("Choose at least one scope"); return; }
    try {
      const r = await api("/api/web/keys", { body });
      setFresh({ key: r.key, name: r.name }); setNewName(""); load();
    } catch (x: any) { setErr(x.message); }
  };

  const act = async (fn: () => Promise<any>, success: string) => {
    setBusy(true); setErr(null);
    try { const r = await fn(); setDialog(null); setOk(success); load(); return r; }
    catch (x: any) { setErr(x.message); setDialog(null); }
    finally { setBusy(false); }
  };
  const rename = (k: Key, name: string) => act(() => api(`/api/web/keys/${k.id}`, { method: "PATCH", body: { name } }), "Key renamed.");
  const rotate = async (k: Key) => { const r = await act(() => api(`/api/web/keys/${k.id}/rotate`, { method: "POST" }), "Key rotated. The old secret no longer works."); if (r) setFresh({ key: r.key, name: r.name, rotated: true }); };
  const disable = (k: Key) => act(() => api(`/api/web/keys/${k.id}/disable`, { method: "POST" }), "Key disabled.");
  const enable = (k: Key) => act(() => api(`/api/web/keys/${k.id}/enable`, { method: "POST" }), "Key enabled.");
  const remove = (k: Key) => act(() => api(`/api/web/keys/${k.id}`, { method: "DELETE" }), "Key deleted.");

  const toggleScope = (s: string) => setPicked(p => (p.includes(s) ? p.filter(x => x !== s) : [...p, s]));
  const minDate = new Date(Date.now() + 86400000).toISOString().slice(0, 10);

  return (
    <>
      <div className="mb-4 flex flex-wrap items-center gap-3"><h1 className="text-xl font-bold">API keys</h1>
        <a href="/api-docs" className="btn-ghost btn-sm ml-auto">API docs →</a></div>
      <Err msg={err} /><Ok msg={ok} />

      {fresh && (
        <div className="mb-4 rounded-xl border border-amber-300 bg-amber-50 p-4" role="alert">
          <div className="text-sm font-semibold text-amber-900">{fresh.rotated ? `New secret for “${fresh.name}”.` : `Key “${fresh.name}” created.`} Copy it now. It won&apos;t be shown again.</div>
          <div className="mt-2 flex flex-wrap items-center gap-2">
            <code data-testid="new-key" className="min-w-0 flex-1 break-all rounded bg-white p-2 text-sm">{fresh.key}</code>
            <CopyButton text={fresh.key} />
            <button className="btn-ghost btn-sm" onClick={() => setFresh(null)}>I&apos;ve copied it</button>
          </div>
          <div className="mt-2 text-xs text-amber-800">After you close this, only <code>{`vocr_****${fresh.key.slice(-4)}`}</code> is shown. We store just a hash, so a lost key can&apos;t be recovered: rotate it instead.</div>
        </div>)}

      <form onSubmit={create} className="card mb-6 space-y-4 p-4">
        <div className="grid gap-4 sm:grid-cols-2">
          <label className="text-sm">Key name
            <input name="name" required maxLength={80} className="input mt-1" placeholder="e.g. Warehouse app" value={newName} onChange={e => setNewName(e.target.value)} /></label>
          <div className="grid grid-cols-2 gap-2">
            <label className="text-sm">Expiration
              <select className="input mt-1" value={expiry} onChange={e => setExpiry(e.target.value)} aria-label="Expiration">{EXPIRY.map(([v, l]) => <option key={v} value={v}>{l}</option>)}</select></label>
            {expiry === "custom" && <label className="text-sm">Expires on
              <input name="custom_date" type="date" min={minDate} required className="input mt-1" /></label>}
          </div>
        </div>
        <fieldset>
          <legend className="text-sm font-medium">Scopes</legend>
          <div className="mt-2 grid gap-2 sm:grid-cols-2">
            {scopes.map(s => (
              <label key={s.scope} className={`flex cursor-pointer items-start gap-2 rounded-lg border p-2 text-sm ${picked.includes(s.scope) ? "border-brand-500 bg-brand-50" : "border-slate-200"}`}>
                <input type="checkbox" className="mt-1" checked={picked.includes(s.scope)} onChange={() => toggleScope(s.scope)} />
                <span><code className="font-semibold">{s.scope}</code><span className="block text-xs text-slate-500">{s.description}</span></span>
              </label>))}
          </div>
          <p className="mt-2 text-xs text-slate-500">Give each integration the fewest scopes it needs.</p>
        </fieldset>
        <button className="btn-primary">Generate key</button>
      </form>

      <div className="card mb-6 overflow-x-auto">
        {!keys ? <Spinner /> : keys.length === 0 ? <Empty>No API keys yet. Create one above.</Empty> : (
          <table className="w-full"><thead className="border-b bg-slate-50"><tr>
            <th className="th">Name</th><th className="th">Key</th><th className="th">Scopes</th><th className="th">Created</th><th className="th">Last used</th><th className="th">Expires</th><th className="th">Status</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{keys.map(k => (
              <tr key={k.id} data-testid={`key-${k.id}`}>
                <td className="td font-medium">{k.name}<div className="text-xs font-normal text-slate-500">{k.request_count} request{k.request_count === 1 ? "" : "s"}</div></td>
                <td className="td"><code className="text-xs">{k.masked}</code></td>
                <td className="td"><div className="flex max-w-[16rem] flex-wrap gap-1">{k.scopes.map(s => <Badge key={s} tone={s === "admin" ? "red" : s.startsWith("learning") ? "blue" : "slate"}>{s}</Badge>)}</div></td>
                <td className="td whitespace-nowrap text-xs text-slate-500">{fmtDay(k.created_at)}</td>
                <td className="td whitespace-nowrap text-xs text-slate-500">{k.last_used_at ? fmtDate(k.last_used_at) : "Never"}</td>
                <td className="td whitespace-nowrap text-xs text-slate-500">{k.expires_at ? fmtDay(k.expires_at) : "Never"}</td>
                <td className="td"><Badge tone={statusTone(k.status)}>{k.status}</Badge></td>
                <td className="td"><div className="flex flex-wrap justify-end gap-1">
                  <button className="btn-ghost btn-sm" onClick={() => setDialog({ kind: "usage", key: k })}>Usage</button>
                  <button className="btn-ghost btn-sm" onClick={() => setDialog({ kind: "rename", key: k })}>Rename</button>
                  {k.status !== "revoked" && <button className="btn-ghost btn-sm" onClick={() => setDialog({ kind: "rotate", key: k })}>Rotate</button>}
                  {k.status === "disabled" ? <button className="btn-ghost btn-sm" onClick={() => enable(k)}>Enable</button>
                    : k.status !== "revoked" && <button className="btn-ghost btn-sm" onClick={() => setDialog({ kind: "disable", key: k })}>Disable</button>}
                  <button className="btn-danger btn-sm" onClick={() => setDialog({ kind: "delete", key: k })}>Delete</button>
                </div></td>
              </tr>))}</tbody></table>)}
      </div>

      <div className="card p-5">
        <h2 className="font-semibold">Quick start</h2>
        <p className="mt-1 text-sm text-slate-600">Send your key in the <code>X-API-Key</code> header. Every endpoint, its required scope and example responses are in the <a className="text-brand-600 underline" href="/api-docs">API docs</a>.</p>
        <pre className="mt-3 overflow-x-auto rounded-lg bg-slate-900 p-4 text-xs text-slate-100">{`curl ${API}/api/v1/modules \\
  -H "X-API-Key: YOUR_KEY"        # needs the learning:read scope`}</pre>
      </div>

      {dialog?.kind === "usage" && <UsageDialog k={dialog.key} onClose={() => setDialog(null)} />}
      {dialog?.kind === "rename" && <RenameDialog k={dialog.key} busy={busy} onSave={n => rename(dialog.key, n)} onClose={() => setDialog(null)} />}
      {dialog?.kind === "rotate" && <ConfirmDialog title="Rotate this key?" confirmLabel="Rotate key" busy={busy} onCancel={() => setDialog(null)} onConfirm={() => rotate(dialog.key)}
        message={<>A new secret is generated for <b>{dialog.key.name}</b> and shown once. <b>The current secret stops working immediately</b>, so update every app that uses it.</>} />}
      {dialog?.kind === "disable" && <ConfirmDialog title="Disable this key?" confirmLabel="Disable key" busy={busy} onCancel={() => setDialog(null)} onConfirm={() => disable(dialog.key)}
        message={<>Requests using <b>{dialog.key.name}</b> will be rejected (401) until you enable it again.</>} />}
      {dialog?.kind === "delete" && <ConfirmDialog title="Delete this key?" confirmLabel="Delete permanently" busy={busy} onCancel={() => setDialog(null)} onConfirm={() => remove(dialog.key)}
        message={<>This permanently deletes <b>{dialog.key.name}</b> (<code>{dialog.key.masked}</code>). Apps using it will stop working and it cannot be restored.</>} />}
    </>
  );
}

function RenameDialog({ k, busy, onSave, onClose }: { k: Key; busy: boolean; onSave: (n: string) => void; onClose: () => void }) {
  const [name, setName] = useState(k.name);
  return (
    <Modal title="Rename key" onClose={onClose}>
      <form onSubmit={e => { e.preventDefault(); if (name.trim()) onSave(name.trim()); }}>
        <label className="text-sm">Name<input className="input mt-1" value={name} maxLength={80} autoFocus onChange={e => setName(e.target.value)} aria-label="Key name" /></label>
        <div className="mt-4 flex justify-end gap-2"><button type="button" className="btn-ghost" onClick={onClose}>Cancel</button>
          <button className="btn-primary" disabled={busy || !name.trim() || name.trim() === k.name}>Save</button></div>
      </form>
    </Modal>
  );
}
