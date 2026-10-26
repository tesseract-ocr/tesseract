"use client";
import Link from "next/link";
import { useParams, useRouter } from "next/navigation";
import { FormEvent, useCallback, useEffect, useRef, useState } from "react";
import AuthImage from "@/components/AuthImage";
import LearningTabs from "@/components/LearningTabs";
import { ConfirmDialog, Modal } from "@/components/Modal";
import Shell from "@/components/Shell";
import { Badge, Err, fmtDate, Ok, Pager, Spinner, statusTone } from "@/components/ui";
import { api, download } from "@/lib/api";

export default function DatasetPage() {
  return <Shell><Body /></Shell>;
}

const pct = (v: number | null | undefined) => (v == null ? "—" : `${(v * 100).toFixed(1)}%`);

function SampleRow({ ds, s, onChanged, onDelete }: { ds: number; s: any; onChanged: () => void; onDelete: (s: any) => void }) {
  const [text, setText] = useState(s.text);
  const [state, setState] = useState<"" | "saving" | "saved" | "error">("");
  useEffect(() => setText(s.text), [s.text]);
  const save = async () => {
    if (text.trim() === s.text) return;
    setState("saving");
    try { await api(`/api/web/lab/datasets/${ds}/samples/${s.id}`, { method: "PATCH", body: { text } }); setState("saved"); onChanged(); setTimeout(() => setState(""), 1500); }
    catch { setState("error"); }
  };
  return (
    <tr data-testid={`sample-${s.id}`}>
      <td className="td w-56"><AuthImage path={`/api/web/lab/datasets/${ds}/samples/${s.id}/image`} alt={`Sample ${s.id}`} className="h-12 max-w-[13rem] rounded border border-slate-200 bg-white object-contain" /></td>
      <td className="td"><input aria-label={`Text of sample ${s.id}`} className={`input ${state === "error" ? "border-red-400" : ""}`} value={text} maxLength={300} onChange={e => setText(e.target.value)} onBlur={save} onKeyDown={e => e.key === "Enter" && (e.target as HTMLInputElement).blur()} />
        {s.original_text && s.original_text !== s.text && <div className="mt-1 text-xs text-slate-400">engine read: {s.original_text}</div>}</td>
      <td className="td whitespace-nowrap"><Badge tone={s.source === "correction" ? "blue" : "slate"}>{s.source}</Badge><div className="mt-1 text-xs text-slate-500" aria-live="polite">{state === "saving" ? "saving…" : state === "saved" ? "✓ saved" : state === "error" ? "could not save" : ""}</div></td>
      <td className="td text-right"><button className="btn-danger btn-sm" onClick={() => onDelete(s)}>Delete</button></td>
    </tr>
  );
}

function Body() {
  const { id } = useParams<{ id: string }>(); const router = useRouter();
  const dsId = Number(id);
  const [ds, setDs] = useState<any>(null);
  const [samples, setSamples] = useState<any>(null);
  const [page, setPage] = useState(1); const [q, setQ] = useState("");
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null); const [busy, setBusy] = useState(false);
  const [adding, setAdding] = useState(false); const [editing, setEditing] = useState(false); const [delSample, setDelSample] = useState<any>(null); const [delDs, setDelDs] = useState(false);
  const zipRef = useRef<HTMLInputElement>(null);

  const loadDs = useCallback(() => api(`/api/web/lab/datasets/${dsId}`).then(setDs).catch(x => setErr(x.message)), [dsId]);
  const loadSamples = useCallback(() => api(`/api/web/lab/datasets/${dsId}/samples?page=${page}&per_page=10&q=${encodeURIComponent(q)}`).then(setSamples).catch(x => setErr(x.message)), [dsId, page, q]);
  useEffect(() => { loadDs(); }, [loadDs]);
  useEffect(() => { const t = setTimeout(loadSamples, 200); return () => clearTimeout(t); }, [loadSamples]);
  const training = ds?.models?.some((m: any) => m.status === "queued" || m.status === "training");
  useEffect(() => { if (!training) return; const t = setInterval(loadDs, 2500); return () => clearInterval(t); }, [training, loadDs]);

  const refresh = () => { loadDs(); loadSamples(); };
  const addSample = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null); setOk(null);
    const f = new FormData(e.currentTarget);
    try { await api(`/api/web/lab/datasets/${dsId}/samples`, { body: f }); setOk("Sample added."); (e.target as HTMLFormElement).reset(); refresh(); }
    catch (x: any) { setErr(x.message); }
  };
  const importZip = async (file?: File | null) => {
    if (!file) return; setBusy(true); setErr(null); setOk(null);
    try { const fd = new FormData(); fd.append("file", file); const r = await api(`/api/web/lab/datasets/${dsId}/import`, { body: fd });
      setOk(`${r.added} sample${r.added === 1 ? "" : "s"} imported${r.skipped_count ? `, ${r.skipped_count} skipped: ${r.skipped.slice(0, 3).map((s: any) => `${s.file} (${s.reason})`).join("; ")}` : ""}.`); refresh(); }
    catch (x: any) { setErr(x.message); } finally { setBusy(false); if (zipRef.current) zipRef.current.value = ""; }
  };
  const removeSample = async () => { setBusy(true); try { await api(`/api/web/lab/datasets/${dsId}/samples/${delSample.id}`, { method: "DELETE" }); setDelSample(null); refresh(); } catch (x: any) { setErr(x.message); setDelSample(null); } finally { setBusy(false); } };
  const removeDs = async () => { setBusy(true); try { await api(`/api/web/lab/datasets/${dsId}`, { method: "DELETE" }); router.replace("/learning/datasets"); } catch (x: any) { setErr(x.message); setDelDs(false); setBusy(false); } };
  const train = async () => { setErr(null); setOk(null); try { await api(`/api/web/lab/datasets/${dsId}/train`, { method: "POST" }); setOk("Training started. This takes a minute or more; the page updates by itself."); loadDs(); } catch (x: any) { setErr(x.message); } };
  const toggle = async (m: any) => { try { await api(`/api/web/lab/datasets/${dsId}/models/${m.id}/${m.active ? "deactivate" : "activate"}`, { method: "POST" }); loadDs(); } catch (x: any) { setErr(x.message); } };
  const saveEdit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); const f = new FormData(e.currentTarget);
    try { await api(`/api/web/lab/datasets/${dsId}`, { method: "PATCH", body: { name: f.get("name"), category: f.get("category"), description: f.get("description") } }); setEditing(false); loadDs(); }
    catch (x: any) { setErr(x.message); }
  };

  if (!ds) return <>{err ? <Err msg={err} /> : <Spinner />}</>;
  const req = ds.training; const latest = ds.models[0];

  return (
    <>
      <h1 className="mb-1 text-xl font-bold">Learning</h1>
      <LearningTabs />
      <Link href="/learning/datasets" className="text-sm text-slate-500 hover:text-brand-600">← All datasets</Link>
      <div className="card mt-3 p-5">
        <div className="flex flex-wrap items-start gap-3">
          <div className="min-w-0 flex-1"><div className="flex flex-wrap items-center gap-2"><Badge>{ds.category}</Badge><Badge tone="slate">{ds.lang}</Badge></div>
            <h2 className="mt-2 text-2xl font-bold" data-testid="dataset-name">{ds.name}</h2>{ds.description && <p className="mt-1 text-slate-600">{ds.description}</p>}
            <p className="mt-1 text-sm text-slate-500">{ds.sample_count} sample{ds.sample_count === 1 ? "" : "s"}</p></div>
          <div className="flex flex-wrap gap-2"><button className="btn-ghost btn-sm" onClick={() => setEditing(true)}>Edit</button>
            <button className="btn-ghost btn-sm" onClick={() => download(`/api/web/lab/datasets/${dsId}/export`, `${ds.name}.zip`).catch(x => setErr(x.message))}>Export zip</button>
            <input ref={zipRef} type="file" accept=".zip" hidden data-testid="import-into" onChange={e => importZip(e.target.files?.[0])} />
            <button className="btn-ghost btn-sm" disabled={busy} onClick={() => zipRef.current?.click()}>Import zip…</button>
            <button className="btn-danger btn-sm" onClick={() => setDelDs(true)}>Delete dataset</button></div>
        </div>
      </div>
      <div className="mt-4"><Err msg={err} /><Ok msg={ok} /></div>

      <section className="card mt-2 p-5" aria-label="Training">
        <div className="flex flex-wrap items-center gap-3"><h2 className="font-semibold">Train a model on this data</h2>
          <button className="btn-primary ml-auto" onClick={train} disabled={!req.ready || training} data-testid="train-btn">{training ? "Training…" : latest ? "Retrain" : "Train model"}</button></div>
        <p className="mt-1 text-sm text-slate-600">Fine-tunes the <b>{ds.lang}</b> Tesseract model on your samples. 1 in 5 samples is held back and never trained on; it is used to measure the error rate <i>before</i> and <i>after</i> (lower is better).</p>
        {!req.ready && <p className="mt-2 rounded bg-amber-50 p-2 text-sm text-amber-900">You have {req.samples} sample{req.samples === 1 ? "" : "s"}; at least {req.minimum} are needed before training makes sense.</p>}
        {req.ready && <p className="mt-2 text-xs text-slate-500">{req.train_samples} to train on · {req.test_samples} held out to measure accuracy. Small hold-out sets are noisy: more samples give a more trustworthy score.</p>}
        {ds.models.length > 0 && (
          <div className="mt-4 overflow-x-auto"><table className="w-full text-sm" aria-label="Model versions">
            <thead className="border-b bg-slate-50"><tr><th className="th">Version</th><th className="th">Status</th><th className="th">Error rate: before → after</th><th className="th">Data</th><th className="th">Trained</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{ds.models.map((m: any) => {
              const done = m.cer_before != null && m.cer_after != null; const better = done && m.cer_after < m.cer_before;
              return (
                <tr key={m.id} data-testid={`model-${m.version}`}>
                  <td className="td font-mono">v{m.version}</td>
                  <td className="td"><Badge tone={m.status === "no_gain" ? "amber" : statusTone(m.status)}>{m.status === "no_gain" ? "no improvement" : m.status}</Badge>{m.active && <Badge tone="green"> in use</Badge>}</td>
                  <td className="td">{done ? <div className="min-w-[14rem]"><div className="font-mono"><span className="text-slate-500">{pct(m.cer_before)}</span> → <b className={better ? "text-green-700" : "text-amber-700"}>{pct(m.cer_after)}</b> <span className="text-xs text-slate-500">({better ? "−" : "+"}{Math.abs((m.cer_after - m.cer_before) * 100).toFixed(1)} pts)</span></div>
                    <div className="mt-1 space-y-0.5" aria-hidden><div className="h-1.5 rounded bg-slate-300" style={{ width: `${Math.min(100, m.cer_before * 100)}%` }} /><div className={`h-1.5 rounded ${better ? "bg-green-500" : "bg-amber-500"}`} style={{ width: `${Math.min(100, m.cer_after * 100)}%` }} /></div></div>
                    : <span className="text-slate-500">{m.status === "failed" ? "—" : "measuring…"}</span>}
                    {m.message && m.status !== "ready" && <div className="mt-1 max-w-md text-xs text-slate-500">{m.message}</div>}</td>
                  <td className="td text-xs text-slate-500">{m.train_samples} train / {m.test_samples} test{m.iterations ? <><br />{m.iterations} iterations</> : null}</td>
                  <td className="td text-xs text-slate-500">{fmtDate(m.finished_at || m.created_at)}</td>
                  <td className="td text-right">{(m.status === "ready" || m.status === "no_gain") && <button className="btn-ghost btn-sm" onClick={() => toggle(m)}>{m.active ? "Stop using" : "Use this model"}</button>}</td>
                </tr>);
            })}</tbody></table></div>)}
        {ds.models.some((m: any) => m.active) && <p className="mt-3 text-sm text-slate-600">To use it, pick <b>My model: {ds.name}</b> in the <Link className="text-brand-600 underline" href="/learning/lab">Extraction lab</Link>.</p>}
      </section>

      <section className="card mt-4" aria-label="Samples">
        <div className="flex flex-wrap items-center gap-3 border-b border-slate-200 p-4"><h2 className="font-semibold">Samples</h2>
          <input className="input ml-auto max-w-xs" placeholder="Search text…" value={q} onChange={e => { setQ(e.target.value); setPage(1); }} aria-label="Search samples" />
          <button className="btn-primary btn-sm" onClick={() => setAdding(a => !a)}>{adding ? "Close" : "Add sample"}</button></div>
        {adding && (
          <form onSubmit={addSample} className="grid gap-3 border-b border-slate-200 bg-slate-50 p-4 sm:grid-cols-[1fr_2fr_auto]" aria-label="Add sample">
            <label className="text-sm">Image of ONE line<input name="image" type="file" accept="image/*" required className="input mt-1 bg-white" data-testid="sample-file" /></label>
            <label className="text-sm">Exactly what it says<input name="text" required maxLength={300} className="input mt-1" placeholder="Type the text in the image" /></label>
            <div className="flex items-end"><button className="btn-primary">Add</button></div>
            <p className="text-xs text-slate-500 sm:col-span-3">One line per image, 8 to 400 px high. Crop tightly; type what is written, including mistakes and punctuation. Tip: correcting lines in the Extraction lab saves them here automatically.</p>
          </form>)}
        {!samples ? <Spinner /> : samples.items.length === 0 ? <div className="p-8 text-center text-sm text-slate-500">{q ? "No samples match." : "No samples yet. Add one, import a zip, or save corrections from the Extraction lab."}</div> : (
          <div className="overflow-x-auto"><table className="w-full"><thead className="border-b bg-slate-50"><tr><th className="th">Image</th><th className="th">Text (click to edit)</th><th className="th">Source</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{samples.items.map((s: any) => <SampleRow key={s.id} ds={dsId} s={s} onChanged={loadDs} onDelete={setDelSample} />)}</tbody></table></div>)}
        {samples && <Pager page={page} total={samples.total} per={10} onPage={setPage} />}
      </section>

      {editing && <Modal title="Edit dataset" onClose={() => setEditing(false)}><form onSubmit={saveEdit} className="space-y-3">
        <label className="block text-sm">Name<input name="name" defaultValue={ds.name} required maxLength={80} className="input mt-1" /></label>
        <label className="block text-sm">Category<input name="category" defaultValue={ds.category} maxLength={40} className="input mt-1" /></label>
        <label className="block text-sm">Description<input name="description" defaultValue={ds.description} maxLength={500} className="input mt-1" /></label>
        <div className="flex justify-end gap-2"><button type="button" className="btn-ghost" onClick={() => setEditing(false)}>Cancel</button><button className="btn-primary">Save</button></div></form></Modal>}
      {delSample && <ConfirmDialog title="Delete this sample?" confirmLabel="Delete sample" busy={busy} onCancel={() => setDelSample(null)} onConfirm={removeSample} message={<>Delete the sample “{delSample.text}”? Models already trained keep what they learned; the next training will not include it.</>} />}
      {delDs && <ConfirmDialog title="Delete this dataset?" confirmLabel="Delete dataset" busy={busy} onCancel={() => setDelDs(false)} onConfirm={removeDs} message={<>This permanently deletes <b>{ds.name}</b>, its {ds.sample_count} sample(s) and all of its models.</>} />}
    </>
  );
}
