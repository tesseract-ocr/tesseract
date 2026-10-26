"use client";
import { useCallback, useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, Empty, Err, Ok, Pager, Spinner, statusTone } from "@/components/ui";
import { api, blob } from "@/lib/api";

const A = { scope: "admin" as const };

function Thumb({ id }: { id: number }) {
  const [u, setU] = useState<string>();
  useEffect(() => { let url = ""; blob(`/api/admin/training-data/${id}/image`, "admin").then(b => { url = URL.createObjectURL(b); setU(url); }).catch(() => {}); return () => { if (url) URL.revokeObjectURL(url); }; }, [id]);
  return u ? <img src={u} alt="line" className="max-h-14 max-w-xs rounded border border-slate-200 bg-white" /> : <div className="h-10 w-32 rounded bg-slate-100" />;
}

function Body() {
  const [status, setStatus] = useState("pending"); const [source, setSource] = useState(""); const [page, setPage] = useState(1);
  const [data, setData] = useState<any>(null); const [sel, setSel] = useState<number[]>([]);
  const [text, setText] = useState<Record<number, string>>({});
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null);
  const load = useCallback(() => api(`/api/admin/training-data?status=${status}&source=${source}&page=${page}&per_page=20`, A).then(d => { setData(d); setSel([]); }).catch(x => setErr(x.message)), [status, source, page]);
  useEffect(() => { load(); }, [load]);
  const review = async (ids: number[], action: "approve" | "reject") => {
    setErr(null);
    try { const body: any = { ids, action }; if (ids.length === 1 && text[ids[0]] !== undefined) body.text = text[ids[0]];
      await api("/api/admin/training-data/review", { ...A, body }); setOk(`${ids.length} sample(s) ${action}d.`); load(); } catch (x: any) { setErr(x.message); }
  };
  const toggle = (id: number) => setSel(s => s.includes(id) ? s.filter(x => x !== id) : [...s, id]);
  return (
    <>
      <div className="mb-4 flex flex-wrap items-center gap-3"><h1 className="text-xl font-bold">Training data</h1>
        <div className="ml-auto flex gap-2">
          <select className="input w-auto" value={status} onChange={e => { setStatus(e.target.value); setPage(1); }}><option value="pending">Pending</option><option value="approved">Approved</option><option value="rejected">Rejected</option><option value="">All</option></select>
          <select className="input w-auto" value={source} onChange={e => { setSource(e.target.value); setPage(1); }}><option value="">All sources</option><option value="correction">User corrections</option><option value="api">API submissions</option></select>
        </div></div>
      <Err msg={err} /><Ok msg={ok} />
      {sel.length > 0 && <div className="mb-3 flex items-center gap-2 text-sm"><b>{sel.length} selected</b><button className="btn-primary btn-sm" onClick={() => review(sel, "approve")}>Approve</button><button className="btn-danger btn-sm" onClick={() => review(sel, "reject")}>Reject</button></div>}
      <div className="card overflow-x-auto">
        {!data ? <Spinner /> : data.items.length === 0 ? <Empty>Nothing here.</Empty> : (
          <table className="w-full"><thead className="border-b bg-slate-50"><tr><th className="th w-8"><input type="checkbox" aria-label="Select all" checked={sel.length === data.items.length} onChange={e => setSel(e.target.checked ? data.items.map((x: any) => x.id) : [])} /></th><th className="th">Image</th><th className="th">Label</th><th className="th">Source</th><th className="th">Status</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{data.items.map((s: any) => (
              <tr key={s.id}><td className="td"><input type="checkbox" checked={sel.includes(s.id)} onChange={() => toggle(s.id)} aria-label={`Select ${s.id}`} /></td>
                <td className="td"><Thumb id={s.id} /></td>
                <td className="td min-w-[16rem]">{s.status === "pending" ? <input className="input" value={text[s.id] ?? s.text} onChange={e => setText({ ...text, [s.id]: e.target.value })} /> : <span>{s.text}</span>}
                  {s.original_text && s.original_text !== s.text && <div className="mt-1 text-xs text-slate-400">OCR said: {s.original_text}</div>}</td>
                <td className="td text-xs"><Badge tone={s.source === "api" ? "blue" : "slate"}>{s.source}</Badge> <span className="font-mono">{s.lang}</span></td>
                <td className="td"><Badge tone={statusTone(s.status)}>{s.status}</Badge>{s.status === "approved" && <div className="mt-1 text-xs text-slate-500">{s.is_test ? "test set" : "training"}</div>}</td>
                <td className="td space-x-1 whitespace-nowrap text-right">{s.status !== "approved" && <button className="btn-primary btn-sm" onClick={() => review([s.id], "approve")}>Approve</button>}{s.status !== "rejected" && <button className="btn-danger btn-sm" onClick={() => review([s.id], "reject")}>Reject</button>}</td></tr>))}
            </tbody></table>)}
        {data && <Pager page={page} total={data.total} per={20} onPage={setPage} />}
      </div>
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
