"use client";
import { useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, Err, fmtDate, Ok, Spinner, statusTone } from "@/components/ui";
import { api } from "@/lib/api";

const A = { scope: "admin" as const };
const pct = (v: number | null) => (v == null ? "—" : `${(v * 100).toFixed(1)}%`);

function Body() {
  const [models, setModels] = useState<any[] | null>(null);
  const [runs, setRuns] = useState<any[]>([]);
  const [cfg, setCfg] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null);
  const load = () => {
    api("/api/admin/models", A).then(setModels).catch(x => setErr(x.message));
    api("/api/admin/training-runs", A).then(setRuns).catch(() => {});
    api("/api/admin/training-settings", A).then(setCfg).catch(() => {});
  };
  useEffect(() => { load(); }, []);
  useEffect(() => { if (!runs.some(r => r.status === "queued" || r.status === "running")) return; const t = setInterval(load, 4000); return () => clearInterval(t); }, [runs]);

  const retrain = async (lang: string) => { setErr(null); try { await api("/api/admin/models/retrain", { ...A, body: { lang } }); setOk(`Training started for ${lang}.`); load(); } catch (x: any) { setErr(x.message); } };
  const activate = async (m: any) => { if (!confirm(`Make ${m.lang} v${m.version} the live model?`)) return; try { await api(`/api/admin/models/${m.id}/activate`, { ...A, method: "POST" }); setOk(`${m.lang} v${m.version} is now live.`); load(); } catch (x: any) { setErr(x.message); } };
  const saveCfg = async () => { try { setCfg(await api("/api/admin/training-settings", { ...A, method: "PUT", body: cfg })); setOk("Settings saved."); } catch (x: any) { setErr(x.message); } };
  const langs = Array.from(new Set((models || []).map(m => m.lang)));
  const busy = runs.some(r => r.status === "queued" || r.status === "running");
  const num = (k: string, label: string, step = 1) => <label className="text-sm">{label}<input type="number" step={step} className="input mt-1" value={cfg[k]} onChange={e => setCfg({ ...cfg, [k]: +e.target.value })} /></label>;

  return (
    <>
      <h1 className="mb-4 text-xl font-bold">OCR models</h1>
      <Err msg={err} /><Ok msg={ok} />
      {busy && <div className="mb-4 rounded-lg border border-amber-200 bg-amber-50 px-4 py-2 text-sm text-amber-800">⏳ A training run is in progress. A new model goes live only if its error rate beats the current one.</div>}
      {!models ? <Spinner /> : langs.map(lang => (
        <div key={lang} className="card mb-4 overflow-x-auto">
          <div className="flex items-center justify-between p-4"><h2 className="font-semibold"><span className="font-mono">{lang}</span></h2>
            <button className="btn-primary btn-sm" disabled={busy} onClick={() => retrain(lang)}>Retrain now</button></div>
          <table className="w-full"><thead className="border-y bg-slate-50"><tr><th className="th">Version</th><th className="th">Status</th><th className="th">Trigger</th><th className="th">CER (test)</th><th className="th">vs. previous</th><th className="th">Samples</th><th className="th">Created</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{models.filter(m => m.lang === lang).map(m => (
              <tr key={m.id}><td className="td font-mono">v{m.version}</td><td className="td"><Badge tone={statusTone(m.status)}>{m.status}</Badge></td>
                <td className="td text-xs">{m.trigger}</td><td className="td">{pct(m.cer)}</td><td className="td text-xs text-slate-500">{m.baseline_cer != null ? `was ${pct(m.baseline_cer)}` : "—"}</td>
                <td className="td text-xs">{m.version === 0 ? "base model" : `${m.train_samples} train / ${m.test_samples} test`}</td>
                <td className="td text-xs text-slate-500">{fmtDate(m.created_at)}</td>
                <td className="td text-right">{m.status !== "active" && <button className="btn-ghost btn-sm" onClick={() => activate(m)}>{m.status === "archived" ? "Roll back" : "Activate"}</button>}</td></tr>))}
            </tbody></table>
        </div>))}
      {cfg && (
        <div className="card mb-4 p-5"><h2 className="font-semibold">Auto-training</h2>
          <p className="mt-1 text-xs text-slate-500">Lower CER (character error rate) is better. A candidate is deployed only if its CER on held-out approved samples improves on the live model.</p>
          <div className="mt-3 grid gap-3 sm:grid-cols-2 lg:grid-cols-5">
            {num("retrain_threshold", "Retrain after N new samples")}{num("retrain_schedule_hours", "…or every N hours")}
            {num("retrain_min_test_samples", "Min. test samples")}{num("retrain_max_iterations", "Training iterations")}{num("min_improvement", "Min. CER improvement", 0.001)}
          </div>
          <div className="mt-3 flex items-center gap-4"><label className="flex items-center gap-2 text-sm"><input type="checkbox" checked={cfg.auto_retrain_enabled} onChange={e => setCfg({ ...cfg, auto_retrain_enabled: e.target.checked })} /> Enable automatic retraining</label>
            <button className="btn-primary btn-sm" onClick={saveCfg}>Save settings</button></div>
        </div>)}
      <div className="card overflow-x-auto"><h2 className="p-4 pb-2 font-semibold">Training runs</h2>
        <table className="w-full"><thead className="border-y bg-slate-50"><tr><th className="th">#</th><th className="th">Lang</th><th className="th">Status</th><th className="th">Trigger</th><th className="th">Started</th><th className="th">Result</th></tr></thead>
          <tbody className="divide-y divide-slate-100">{runs.length === 0 ? <tr><td className="td text-slate-500" colSpan={6}>No runs yet.</td></tr> : runs.map(r => (
            <tr key={r.id}><td className="td">{r.id}</td><td className="td font-mono">{r.lang}</td><td className="td"><Badge tone={statusTone(r.status)}>{r.status}</Badge></td><td className="td text-xs">{r.trigger}</td>
              <td className="td text-xs text-slate-500">{fmtDate(r.started_at)}</td><td className="td max-w-md text-xs text-slate-600">{r.message}</td></tr>))}</tbody></table></div>
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
