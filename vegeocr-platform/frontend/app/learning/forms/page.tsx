"use client";
import { useEffect, useRef, useState } from "react";
import LearningTabs from "@/components/LearningTabs";
import Shell from "@/components/Shell";
import { Err, Spinner } from "@/components/ui";
import { api } from "@/lib/api";

type Row = { table: number; index: number; code: string; name: string; quantity: string; unit: string; handwritten: boolean; confidence: number; review: boolean; note: string };

export default function FormsPage() {
  return <Shell><Body /></Shell>;
}

function Body() {
  const [cats, setCats] = useState<{ id: string; title: string; items: number }[]>([]);
  const [catalog, setCatalog] = useState("");
  const [rows, setRows] = useState<Row[] | null>(null);
  const [busy, setBusy] = useState(false); const [err, setErr] = useState<string | null>(null);
  const [name, setName] = useState("");
  const fileRef = useRef<HTMLInputElement>(null);
  useEffect(() => { api("/api/web/lab/forms/catalogs").then(r => { setCats(r.catalogs); if (r.catalogs[0]) setCatalog(r.catalogs[0].id); }).catch(x => setErr(x.message)); }, []);

  const read = async (file?: File | null) => {
    if (!file) return; setBusy(true); setErr(null); setRows(null); setName(file.name);
    try { const fd = new FormData(); fd.append("file", file); fd.append("catalog", catalog); setRows((await api("/api/web/lab/forms/read", { body: fd })).rows); }
    catch (x: any) { setErr(x.message); } finally { setBusy(false); if (fileRef.current) fileRef.current.value = ""; }
  };
  const setQty = (i: number, v: string) => setRows(rs => rs && rs.map((r, k) => (k === i ? { ...r, quantity: v.replace(/\D/g, ""), review: false, note: "checked" } : r)));
  const ordered = rows?.filter(r => r.quantity) ?? [];
  const csv = () => {
    const esc = (v: string) => `"${v.replace(/"/g, '""')}"`;
    const body = ["商品コード,品目,発注数,単位", ...ordered.map(r => [r.code, esc(r.name), r.quantity, r.unit].join(","))].join("\r\n");
    const a = document.createElement("a"); a.href = URL.createObjectURL(new Blob(["﻿" + body], { type: "text/csv;charset=utf-8" }));
    a.download = `${name.replace(/\.[^.]+$/, "") || "order"}.csv`; a.click(); URL.revokeObjectURL(a.href);
  };

  return (
    <>
      <h1 className="mb-1 text-xl font-bold">Learning</h1>
      <LearningTabs />
      <p className="mb-4 max-w-2xl text-sm text-slate-600">Upload a photo or scan of an order form (発注書). Printed items are matched to the catalog, handwritten quantities are read, and anything the system is unsure about is highlighted for you to check — it never guesses silently.</p>
      <div className="mb-4 flex flex-wrap items-center gap-3">
        <label className="text-sm">Form <select className="input ml-1" value={catalog} onChange={e => setCatalog(e.target.value)} data-testid="form-catalog">
          {cats.length === 0 && <option value="">(no catalog installed)</option>}
          {cats.map(c => <option key={c.id} value={c.id}>{c.title} ({c.items} items)</option>)}</select></label>
        <input ref={fileRef} type="file" accept="image/*" hidden data-testid="form-file" onChange={e => read(e.target.files?.[0])} />
        <button className="btn-primary" disabled={busy} onClick={() => fileRef.current?.click()}>{busy ? "Reading…" : "Upload form image"}</button>
        {ordered.length > 0 && <button className="btn-ghost" onClick={csv}>Download CSV ({ordered.length} lines)</button>}
      </div>
      <Err msg={err} />
      {busy && <Spinner />}
      {rows && (
        <div className="card overflow-x-auto" data-testid="form-result">
          <p className="mb-2 text-sm text-slate-600">{rows.length} printed lines, {ordered.length} with a quantity, <b className={rows.some(r => r.review) ? "text-amber-700" : "text-emerald-700"}>{rows.filter(r => r.review).length} to check</b>.</p>
          <table className="w-full text-sm">
            <thead><tr className="text-left text-slate-500"><th className="py-1 pr-3">コード</th><th className="pr-3">品目</th><th className="pr-3">発注数</th><th className="pr-3">単位</th><th>確認</th></tr></thead>
            <tbody>{rows.map((r, i) => (
              <tr key={i} className={r.review ? "bg-amber-50" : r.quantity ? "bg-emerald-50" : ""} data-review={r.review ? "1" : "0"}>
                <td className="py-1 pr-3 font-mono">{r.code}</td><td className="pr-3">{r.name}</td>
                <td className="pr-3"><input className="input w-20 py-0.5" inputMode="numeric" value={r.quantity} aria-label={`Quantity for ${r.name}`} onChange={e => setQty(i, e.target.value)} /></td>
                <td className="pr-3">{r.unit}</td><td className="text-amber-700">{r.review ? r.note : ""}</td></tr>))}</tbody>
          </table>
        </div>)}
    </>
  );
}
