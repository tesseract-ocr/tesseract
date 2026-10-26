"use client";
import { useEffect, useState } from "react";
import { api, blob, download } from "@/lib/api";
import { Badge, Err, Ok, statusTone } from "./ui";

type Line = { text: string; confidence: number; bbox: number[] };
type Page = { page: number; source: string; lines: Line[] };

/** Shows one OCR job: page image, editable lines (corrections feed the training queue), copy and downloads. */
export default function ResultView({ job: initial, onChange }: { job: any; onChange?: (j: any) => void }) {
  const [job, setJob] = useState<any>(initial);
  const [pages, setPages] = useState<Page[]>(initial.result || []);
  const [edits, setEdits] = useState<Record<string, string>>({});
  const [img, setImg] = useState<Record<number, string>>({});
  const [err, setErr] = useState<string | null>(null);
  const [ok, setOk] = useState<string | null>(null);
  const [mode, setMode] = useState<"lines" | "text">("lines");
  const [text, setText] = useState<string>(initial.text || "");

  // Poll while the job is still queued/processing (large files run on the worker).
  useEffect(() => {
    if (job.status !== "queued" && job.status !== "processing") return;
    const t = setInterval(async () => {
      try { const j = await api(`/api/web/jobs/${job.id}`); setJob(j); if (j.status === "done" || j.status === "failed") { setPages(j.result || []); setText(j.text || ""); onChange?.(j); } } catch {}
    }, 2000);
    return () => clearInterval(t);
  }, [job.status, job.id]);

  useEffect(() => {
    let urls: string[] = [];
    if (job.status === "done") {
      pages.filter(p => p.source === "ocr").forEach(async p => {
        try { const u = URL.createObjectURL(await blob(`/api/web/jobs/${job.id}/pages/${p.page}/image`)); urls.push(u); setImg(m => ({ ...m, [p.page]: u })); } catch {}
      });
    }
    return () => urls.forEach(URL.revokeObjectURL);
  }, [job.id, job.status]);

  const key = (p: number, i: number) => `${p}:${i}`;
  const dirty = Object.keys(edits).length;
  const flash = (m: string) => { setOk(m); setTimeout(() => setOk(null), 2500); };

  const saveLines = async () => {
    setErr(null);
    const list = Object.entries(edits).map(([k, t]) => { const [page, index] = k.split(":").map(Number); return { page, index, text: t }; });
    try {
      const r = await api(`/api/web/jobs/${job.id}/lines`, { method: "PATCH", body: { edits: list } });
      setPages(ps => ps.map(p => ({ ...p, lines: p.lines.map((l, i) => { const e = edits[key(p.page, i)]; return e !== undefined ? { ...l, text: e } : l; }) })));
      setText(r.text); setEdits({});
      flash(r.training_samples_created ? `Saved. ${r.training_samples_created} correction(s) sent to the training queue.` : "Saved.");
    } catch (x: any) { setErr(x.message); }
  };
  const saveText = async () => {
    setErr(null);
    try { await api(`/api/web/jobs/${job.id}/text`, { method: "PUT", body: { text } }); flash("Saved."); } catch (x: any) { setErr(x.message); }
  };
  const copy = async () => { await navigator.clipboard.writeText(mode === "text" ? text : pages.map(p => p.lines.map((l, i) => edits[key(p.page, i)] ?? l.text).join("\n")).join("\n\n")); flash("Copied to clipboard."); };
  const stem = (job.filename || "result").replace(/\.[^.]+$/, "");

  if (job.status === "queued" || job.status === "processing") return <div className="card p-8 text-center text-sm text-slate-600">⏳ Processing <b>{job.filename}</b>… this page updates automatically.</div>;
  if (job.status === "failed") return <div className="card p-6"><Err msg={`OCR failed: ${job.error || "unknown error"}`} /></div>;

  return (
    <div className="card">
      <div className="flex flex-wrap items-center gap-3 border-b border-slate-200 p-4">
        <div className="min-w-0">
          <div className="truncate font-semibold">{job.filename}</div>
          <div className="mt-0.5 flex flex-wrap items-center gap-2 text-xs text-slate-500">
            <Badge tone={statusTone(job.status)}>{job.status}</Badge><span>{job.lang}</span><span>{job.pages} page(s)</span>
            {job.confidence != null && <span>confidence {Number(job.confidence).toFixed(0)}%</span>}
          </div>
        </div>
        <div className="ml-auto flex flex-wrap gap-2">
          <button className="btn-ghost btn-sm" onClick={copy}>Copy</button>
          {(["txt", "json", "pdf"] as const).map(f => <button key={f} className="btn-ghost btn-sm" onClick={() => download(`/api/web/jobs/${job.id}/export?format=${f}`, `${stem}.${f}`).catch(x => setErr(x.message))}>.{f}</button>)}
        </div>
      </div>
      <div className="p-4">
        <Err msg={err} /><Ok msg={ok} />
        <div className="mb-3 inline-flex rounded-lg border border-slate-200 p-0.5 text-sm">
          {(["lines", "text"] as const).map(m => <button key={m} onClick={() => setMode(m)} className={`rounded-md px-3 py-1 ${mode === m ? "bg-brand-600 text-white" : "text-slate-600"}`}>{m === "lines" ? "Line corrections" : "Full text"}</button>)}
        </div>
        {mode === "text" ? (
          <>
            <textarea className="input h-80 font-mono" value={text} onChange={e => setText(e.target.value)} />
            <div className="mt-3 flex items-center gap-3"><button className="btn-primary" onClick={saveText}>Save text</button>
              <span className="text-xs text-slate-500">Free-form edits are saved but are not used for model training. Use line corrections for that.</span></div>
          </>
        ) : (
          <div className="space-y-6">
            {pages.map(p => (
              <div key={p.page} className="grid gap-4 md:grid-cols-2">
                <div>
                  <div className="mb-1 text-xs font-semibold uppercase text-slate-500">Page {p.page}{p.source === "text-layer" && " · digital text"}</div>
                  {img[p.page] ? <img src={img[p.page]} alt={`Page ${p.page}`} className="max-h-[28rem] w-full rounded-lg border border-slate-200 object-contain" />
                    : <div className="grid h-32 place-items-center rounded-lg border border-dashed border-slate-300 text-xs text-slate-400">{p.source === "text-layer" ? "Text extracted directly from the PDF" : "Loading image…"}</div>}
                </div>
                <div className="space-y-1.5">
                  {p.lines.length === 0 && <div className="text-sm text-slate-500">No text found on this page.</div>}
                  {p.lines.map((l, i) => {
                    const k = key(p.page, i), changed = edits[k] !== undefined;
                    return (
                      <div key={i} className="flex items-center gap-2">
                        <input className={`input ${changed ? "border-amber-400 bg-amber-50" : ""} ${l.confidence < 70 ? "border-red-300" : ""}`} value={edits[k] ?? l.text}
                          onChange={e => setEdits(s => e.target.value === l.text ? Object.fromEntries(Object.entries(s).filter(([x]) => x !== k)) : { ...s, [k]: e.target.value })} />
                        <span title="OCR confidence" className={`w-10 shrink-0 text-right text-xs ${l.confidence < 70 ? "text-red-600" : "text-slate-400"}`}>{Math.round(l.confidence)}%</span>
                      </div>
                    );
                  })}
                </div>
              </div>
            ))}
            <div className="flex items-center gap-3">
              <button className="btn-primary" disabled={!dirty} onClick={saveLines}>Save {dirty || ""} correction{dirty === 1 ? "" : "s"}</button>
              <span className="text-xs text-slate-500">Fix any mistakes. Corrected lines help improve the OCR model once an admin approves them. Low-confidence lines are outlined in red.</span>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
