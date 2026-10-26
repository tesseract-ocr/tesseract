"use client";
import { DragEvent, FormEvent, useRef, useState } from "react";
import Shell from "@/components/Shell";
import ResultView from "@/components/ResultView";
import { Err } from "@/components/ui";
import { api, LANGS } from "@/lib/api";

function Body() {
  const [file, setFile] = useState<File | null>(null);
  const [lang, setLang] = useState("jpn+eng");
  const [busy, setBusy] = useState(false);
  const [err, setErr] = useState<string | null>(null);
  const [job, setJob] = useState<any>(null);
  const [over, setOver] = useState(false);
  const input = useRef<HTMLInputElement>(null);

  const pick = (f?: File | null) => { if (f) { setFile(f); setErr(null); } };
  const drop = (e: DragEvent) => { e.preventDefault(); setOver(false); pick(e.dataTransfer.files?.[0]); };
  const submit = async (e: FormEvent) => {
    e.preventDefault(); if (!file) return;
    setBusy(true); setErr(null); setJob(null);
    try { const fd = new FormData(); fd.append("file", file); fd.append("lang", lang); setJob(await api("/api/web/jobs", { body: fd })); }
    catch (x: any) { setErr(x.message); } finally { setBusy(false); }
  };

  return (
    <>
      <h1 className="mb-4 text-xl font-bold">Extract text</h1>
      <form onSubmit={submit} className="card mb-6 p-5">
        <Err msg={err} />
        <div onDragOver={e => { e.preventDefault(); setOver(true); }} onDragLeave={() => setOver(false)} onDrop={drop} onClick={() => input.current?.click()}
          className={`cursor-pointer rounded-xl border-2 border-dashed p-8 text-center transition ${over ? "border-brand-600 bg-brand-50" : "border-slate-300 hover:border-brand-600"}`}>
          <input ref={input} type="file" hidden accept=".png,.jpg,.jpeg,.tif,.tiff,.bmp,.webp,.pdf" onChange={e => pick(e.target.files?.[0])} />
          <div className="text-sm"><b className="text-brand-600">Click to choose</b> or drag a file here</div>
          <div className="mt-1 text-xs text-slate-500">{file ? `${file.name} · ${(file.size / 1024).toFixed(0)} KB` : "PNG, JPG, TIFF, BMP, WEBP or PDF · up to 20 MB"}</div>
        </div>
        <div className="mt-4 flex flex-wrap items-center gap-3">
          <label className="text-sm text-slate-600">Language{" "}
            <select className="input ml-1 inline-block w-auto" value={lang} onChange={e => setLang(e.target.value)}>
              {LANGS.map(([c, l]) => <option key={c} value={c}>{l}</option>)}
            </select>
          </label>
          <button className="btn-primary" disabled={!file || busy}>{busy ? "Reading document…" : "Run OCR"}</button>
        </div>
      </form>
      {job && <ResultView key={job.id} job={job} />}
    </>
  );
}

export default function Page() {
  return <Shell><Body /></Shell>;
}
