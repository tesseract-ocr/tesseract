"use client";
import { DragEvent, FormEvent, useCallback, useEffect, useRef, useState } from "react";
import LabResult from "@/components/LabResult";
import LearningTabs from "@/components/LearningTabs";
import Shell from "@/components/Shell";
import { Badge, Err, fmtDate, Spinner, statusTone } from "@/components/ui";
import { api, LANGS } from "@/lib/api";

export default function LabPage() {
  return <Shell><Body /></Shell>;
}

const MODE_HELP: Record<string, string> = {
  printed: “Typed or printed text: documents, receipts, forms. Supports Japanese and Chinese.”,
  handwriting: “Handwritten notes, print or cursive. Neat block letters often work better as “Printed”.”,
};

function Body() {
  const [cap, setCap] = useState<any>(null);
  const [datasets, setDatasets] = useState<any[]>([]);
  const [history, setHistory] = useState<any[]>([]);
  const [file, setFile] = useState<File | null>(null);
  const [mode, setMode] = useState("printed"); const [lang, setLang] = useState("jpn"); const [enhance, setEnhance] = useState("auto"); const [dataset, setDataset] = useState("");
  const [run, setRun] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null); const [busy, setBusy] = useState(false); const [over, setOver] = useState(false);
  const input = useRef<HTMLInputElement>(null);

  const loadDatasets = useCallback(() => api("/api/web/lab/datasets").then(d => setDatasets(d.items)).catch(() => {}), []);
  const loadHistory = useCallback(() => api("/api/web/lab/runs").then(setHistory).catch(() => {}), []);
  useEffect(() => { api("/api/web/lab/capabilities").then(setCap).catch(x => setErr(x.message)); loadDatasets(); loadHistory(); }, [loadDatasets, loadHistory]);

  // Poll the run until it finishes (extraction runs in a background worker).
  useEffect(() => {
    if (!run || (run.status !== "queued" && run.status !== "processing")) return;
    const t = setInterval(async () => {
      try { const r = await api(`/api/web/lab/runs/${run.id}`); setRun(r); if (r.status === "done" || r.status === "failed") { clearInterval(t); setBusy(false); loadHistory(); } } catch (x: any) { setErr(x.message); clearInterval(t); setBusy(false); }
    }, 1200);
    return () => clearInterval(t);
  }, [run, loadHistory]);

  const submit = async (e: FormEvent) => {
    e.preventDefault(); if (!file) return;
    setBusy(true); setErr(null); setRun(null);
    try {
      const fd = new FormData(); fd.append("file", file); fd.append("mode", mode); fd.append("lang", lang); fd.append("enhance", enhance); fd.append("engine", "auto");
      if (dataset) fd.append("dataset_id", dataset);
      setRun(await api("/api/web/lab/runs", { body: fd }));
    } catch (x: any) { setErr(x.message); setBusy(false); }
  };
  const pick = (f?: File | null) => { if (f) { setFile(f); setErr(null); } };
  const drop = (e: DragEvent) => { e.preventDefault(); setOver(false); pick(e.dataTransfer.files?.[0]); };
  const open = async (id: string) => { setErr(null); try { setRun(await api(`/api/web/lab/runs/${id}`)); window.scrollTo({ top: 0, behavior: "smooth" }); } catch (x: any) { setErr(x.message); } };

  const hwNote = mode === "handwriting" && cap && cap.handwriting_engine !== "trocr";
  const withModel = datasets.filter(d => d.active_model);

  return (
    <>
      <h1 className="mb-1 text-xl font-bold">Learning</h1>
      <LearningTabs />
      <p className="mb-4 max-w-3xl text-sm text-slate-600">Read handwriting, rescue blurry or low-quality images, then correct what the engine got wrong. Corrections become your own training data.</p>
      <Err msg={err} />
      <form onSubmit={submit} className="card mb-6 space-y-4 p-5" aria-label="New extraction">
        <div onDragOver={e => { e.preventDefault(); setOver(true); }} onDragLeave={() => setOver(false)} onDrop={drop} onClick={() => input.current?.click()}
          className={`cursor-pointer rounded-xl border-2 border-dashed p-6 text-center transition ${over ? "border-brand-600 bg-brand-50" : "border-slate-300 hover:border-brand-600"}`}>
          <input ref={input} type="file" hidden data-testid="lab-file" accept=".png,.jpg,.jpeg,.tif,.tiff,.bmp,.webp,.pdf" onChange={e => pick(e.target.files?.[0])} />
          <div className="text-sm"><b className="text-brand-600">Click to choose</b> or drag an image or PDF here</div>
          <div className="mt-1 text-xs text-slate-500">{file ? `${file.name} · ${(file.size / 1024).toFixed(0)} KB` : `PNG, JPG, TIFF, BMP, WEBP or PDF · up to ${cap?.limits.max_upload_mb ?? 15} MB · PDFs: first ${cap?.limits.max_pages ?? 3} pages`}</div>
        </div>
        <div className="grid gap-4 md:grid-cols-4">
          <fieldset className="md:col-span-2"><legend className="text-sm font-medium">What is in the image?</legend>
            <div className="mt-1 inline-flex rounded-lg border border-slate-200 p-0.5 text-sm" role="radiogroup" aria-label="Mode">
              {(["printed", "handwriting"] as const).map(m => <label key={m} className={`cursor-pointer rounded-md px-4 py-1.5 ${mode === m ? "bg-brand-600 text-white" : "text-slate-600"}`}><input type="radio" name="mode" value={m} className="sr-only" checked={mode === m} onChange={() => setMode(m)} />{m === "printed" ? "Printed text" : "Handwriting"}</label>)}
            </div><p className="mt-1.5 text-xs text-slate-500">{MODE_HELP[mode]}</p></fieldset>
          <label className="text-sm font-medium">Language<select className="input mt-1" value={lang} onChange={e => setLang(e.target.value)} aria-label="Language">{LANGS.map(([c, l]) => <option key={c} value={c}>{l}</option>)}</select></label>
          <label className="text-sm font-medium">Image enhancement<select className="input mt-1" value={enhance} onChange={e => setEnhance(e.target.value)} aria-label="Enhancement">
            <option value="auto">Auto: only if the image needs it</option><option value="on">Always enhance</option><option value="off">Never</option></select></label>
        </div>
        {hwNote && <div role=”alert” className=”rounded-lg border border-amber-200 bg-amber-50 px-3 py-2 text-sm text-amber-900”>⚠ The handwriting engine (TrOCR) is not installed on this server, so Tesseract will be used. It is much less accurate on handwriting, especially cursive. {cap.engines.trocr.reason}</div>}
        <div className="flex flex-wrap items-end gap-3">
          {withModel.length > 0 && <label className="text-sm font-medium">Model<select className="input mt-1 w-64" value={dataset} onChange={e => setDataset(e.target.value)} aria-label="Model"><option value="">Standard model</option>{withModel.map(d => <option key={d.id} value={d.id}>My model: {d.name}</option>)}</select></label>}
          <button className="btn-primary" disabled={!file || busy}>{busy ? "Working…" : "Extract text"}</button>
          {busy && <span className="text-sm text-slate-500" role="status">Reading the image{enhance !== "off" ? ", enhancing if needed" : ""}…</span>}
        </div>
      </form>

      {run && (run.status === "queued" || run.status === "processing") && <div className="card mb-6 p-8 text-center text-sm text-slate-600" role="status">⏳ Processing <b>{run.filename}</b>… the result appears here automatically.</div>}
      {run?.status === "failed" && <div className="mb-6"><Err msg={`Extraction failed: ${run.error?.message || "unknown error"}`} /></div>}
      {run?.status === "done" && run.result && <div className="mb-6"><LabResult key={run.id} run={run} datasets={datasets} onDatasetsChanged={loadDatasets} /></div>}

      <section aria-label="Recent extractions" className="card">
        <h2 className="border-b border-slate-200 p-4 font-semibold">Recent extractions</h2>
        {!history ? <Spinner /> : history.length === 0 ? <div className="p-6 text-center text-sm text-slate-500">Nothing yet.</div> : (
          <ul className="divide-y divide-slate-100">{history.slice(0, 8).map(h => (
            <li key={h.id}><button onClick={() => open(h.id)} className="flex w-full flex-wrap items-center gap-3 p-3 text-left hover:bg-slate-50">
              <span className="min-w-0 flex-1 truncate font-medium">{h.filename}</span><Badge>{h.mode}</Badge><Badge tone={statusTone(h.status)}>{h.status}</Badge>
              {h.summary && <span className="text-xs text-slate-500">{h.summary.engine}{h.summary.flagged_count ? ` · ${h.summary.flagged_count} to review` : ""}{h.summary.enhanced ? " · enhanced" : ""}</span>}
              <span className="text-xs text-slate-400">{fmtDate(h.created_at)}</span></button></li>))}</ul>)}
      </section>
    </>
  );
}
