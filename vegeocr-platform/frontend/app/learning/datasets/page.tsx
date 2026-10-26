"use client";
import Link from "next/link";
import { FormEvent, useCallback, useEffect, useRef, useState } from "react";
import LearningTabs from "@/components/LearningTabs";
import { ConfirmDialog, Modal } from "@/components/Modal";
import Shell from "@/components/Shell";
import { Badge, Empty, Err, fmtDay, Ok, Spinner } from "@/components/ui";
import { api, download } from "@/lib/api";

export default function DatasetsPage() {
  return <Shell><Body /></Shell>;
}

const pct = (v: number | null | undefined) => (v == null ? "—" : `${(v * 100).toFixed(1)}%`);

function Body() {
  const [data, setData] = useState<{ items: any[]; categories: string[] } | null>(null);
  const [cap, setCap] = useState<any>(null);
  const [cat, setCat] = useState("");
  const [creating, setCreating] = useState(false); const [del, setDel] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null); const [busy, setBusy] = useState(false);
  const fileRef = useRef<HTMLInputElement>(null);
  const load = useCallback(() => api(`/api/web/lab/datasets${cat ? `?category=${encodeURIComponent(cat)}` : ""}`).then(setData).catch(x => setErr(x.message)), [cat]);
  useEffect(() => { load(); }, [load]);
  useEffect(() => { api("/api/web/lab/capabilities").then(setCap).catch(() => {}); }, []);

  const create = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null); const f = new FormData(e.currentTarget);
    try { await api("/api/web/lab/datasets", { body: { name: f.get("name"), category: f.get("category"), lang: f.get("lang"), description: f.get("description") } }); setCreating(false); setOk("Dataset created."); load(); }
    catch (x: any) { setErr(x.message); }
  };
  const importZip = async (file?: File | null) => {
    if (!file) return; setBusy(true); setErr(null); setOk(null);
    try {
      const fd = new FormData(); fd.append("file", file);
      const r = await api("/api/web/lab/datasets/import", { body: fd });
      setOk(`Imported “${r.dataset.name}”: ${r.import.added} sample${r.import.added === 1 ? "" : "s"} added${r.import.skipped_count ? `, ${r.import.skipped_count} skipped (${r.import.skipped[0]?.reason})` : ""}.`); load();
    } catch (x: any) { setErr(x.message); } finally { setBusy(false); if (fileRef.current) fileRef.current.value = ""; }
  };
  const remove = async () => { setBusy(true); try { await api(`/api/web/lab/datasets/${del.id}`, { method: "DELETE" }); setDel(null); setOk("Dataset deleted."); load(); } catch (x: any) { setErr(x.message); setDel(null); } finally { setBusy(false); } };

  return (
    <>
      <h1 className="mb-1 text-xl font-bold">Learning</h1>
      <LearningTabs />
      <div className="mb-4 flex flex-wrap items-center gap-3">
        <p className="max-w-2xl flex-1 text-sm text-slate-600">Your own labelled samples (a picture of ONE line of text, plus exactly what it says), grouped by dataset and category. Train a model on a dataset to improve recognition for your kind of documents.</p>
        <input ref={fileRef} type="file" accept=".zip" hidden data-testid="import-zip" onChange={e => importZip(e.target.files?.[0])} />
        <button className="btn-ghost" disabled={busy} onClick={() => fileRef.current?.click()}>Import zip…</button>
        <button className="btn-primary" onClick={() => setCreating(true)}>New dataset</button>
      </div>
      <Err msg={err} /><Ok msg={ok} />
      {data && data.categories.length > 0 && (
        <div className="mb-4 flex flex-wrap gap-2" role="group" aria-label="Filter by category">
          <button onClick={() => setCat("")} className={`rounded-full px-3 py-1 text-sm ${cat === "" ? "bg-brand-600 text-white" : "bg-slate-100 text-slate-600"}`}>All</button>
          {data.categories.map(c => <button key={c} onClick={() => setCat(c)} className={`rounded-full px-3 py-1 text-sm ${cat === c ? "bg-brand-600 text-white" : "bg-slate-100 text-slate-600"}`}>{c}</button>)}
        </div>)}

      {!data ? <Spinner /> : data.items.length === 0 ? (
        <div className="card"><Empty>No datasets{cat ? ` in “${cat}”` : " yet"}. Create one, import a zip, or correct a line in the <Link className="text-brand-600 underline" href="/learning/lab">Extraction lab</Link> to start.</Empty></div>
      ) : (
        <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
          {data.items.map(d => (
            <article key={d.id} className="card flex flex-col p-4" data-testid={`dataset-${d.id}`}>
              <div className="flex flex-wrap items-center gap-2"><Badge>{d.category}</Badge><Badge tone="slate">{d.lang}</Badge></div>
              <Link href={`/learning/datasets/${d.id}`} className="mt-2 font-semibold hover:text-brand-600">{d.name}</Link>
              {d.description && <p className="mt-1 line-clamp-2 text-sm text-slate-600">{d.description}</p>}
              <div className="mt-3 text-sm text-slate-600">{d.sample_count} sample{d.sample_count === 1 ? "" : "s"} · created {fmtDay(d.created_at)}</div>
              <div className="mt-2 flex-1 text-sm">{d.active_model
                ? <span className="text-green-700">✓ Model v{d.active_model.version} on: error rate {pct(d.active_model.cer_before)} → <b>{pct(d.active_model.cer_after)}</b></span>
                : <span className="text-slate-500">No custom model yet{cap && d.sample_count < cap.training.min_samples ? ` (needs ${cap.training.min_samples} samples)` : ""}</span>}</div>
              <div className="mt-3 flex flex-wrap gap-2"><Link className="btn-primary btn-sm" href={`/learning/datasets/${d.id}`}>Open</Link>
                <button className="btn-ghost btn-sm" onClick={() => download(`/api/web/lab/datasets/${d.id}/export`, `${d.name}.zip`).catch(x => setErr(x.message))}>Export zip</button>
                <button className="btn-danger btn-sm" onClick={() => setDel(d)}>Delete</button></div>
            </article>))}
        </div>)}

      {creating && (
        <Modal title="New dataset" onClose={() => setCreating(false)}>
          <form onSubmit={create} className="space-y-3">
            <label className="block text-sm">Name<input name="name" required maxLength={80} className="input mt-1" placeholder="e.g. My handwriting" autoFocus /></label>
            <label className="block text-sm">Category<input name="category" list="category-hints" defaultValue="general" maxLength={40} className="input mt-1" />
              <datalist id="category-hints">{(cap?.training.categories || ["handwriting", "printed", "blurry", "general"]).map((c: string) => <option key={c} value={c} />)}</datalist></label>
            <label className="block text-sm">Language the samples are in<select name="lang" className="input mt-1" defaultValue="jpn">{(cap?.training.languages || ["jpn", "chi_sim", "chi_tra"]).map((l: string) => <option key={l}>{l}</option>)}</select></label>
            <label className="block text-sm">Description (optional)<input name="description" maxLength={500} className="input mt-1" /></label>
            <div className="flex justify-end gap-2"><button type="button" className="btn-ghost" onClick={() => setCreating(false)}>Cancel</button><button className="btn-primary">Create</button></div>
          </form>
        </Modal>)}
      {del && <ConfirmDialog title="Delete this dataset?" confirmLabel="Delete dataset" busy={busy} onCancel={() => setDel(null)} onConfirm={remove}
        message={<>This permanently deletes <b>{del.name}</b>, its {del.sample_count} sample(s) and every model trained on it. Export it first if you may need it again.</>} />}
    </>
  );
}
