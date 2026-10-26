"use client";
import Link from "next/link";
import { useCallback, useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, Empty, Err, fmtDate, Pager, Spinner, statusTone } from "@/components/ui";
import { api } from "@/lib/api";

function Body() {
  const [data, setData] = useState<any>(null);
  const [q, setQ] = useState(""); const [page, setPage] = useState(1);
  const [err, setErr] = useState<string | null>(null);
  const load = useCallback(() => api(`/api/web/jobs?page=${page}&per_page=15&q=${encodeURIComponent(q)}`).then(setData).catch(x => setErr(x.message)), [page, q]);
  useEffect(() => { const t = setTimeout(load, 250); return () => clearTimeout(t); }, [load]);
  const del = async (id: string) => { if (confirm("Delete this result?")) { await api(`/api/web/jobs/${id}`, { method: "DELETE" }); load(); } };
  return (
    <>
      <div className="mb-4 flex flex-wrap items-center gap-3">
        <h1 className="text-xl font-bold">History</h1>
        <input className="input ml-auto max-w-xs" placeholder="Search file name or text…" value={q} onChange={e => { setQ(e.target.value); setPage(1); }} />
      </div>
      <Err msg={err} />
      <div className="card overflow-x-auto">
        {!data ? <Spinner /> : data.items.length === 0 ? <Empty>No results yet. <Link className="text-brand-600 underline" href="/dashboard">Run your first OCR</Link>.</Empty> : (
          <table className="w-full">
            <thead className="border-b border-slate-200 bg-slate-50"><tr><th className="th">Document</th><th className="th">Text</th><th className="th">Date</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">
              {data.items.map((j: any) => (
                <tr key={j.id} className="hover:bg-slate-50">
                  <td className="td"><Link href={`/history/${j.id}`} className="font-medium hover:text-brand-600">{j.filename}</Link>
                    <div className="mt-1 flex gap-2 text-xs text-slate-500"><Badge tone={statusTone(j.status)}>{j.status}</Badge>{j.lang} · {j.pages}p</div></td>
                  <td className="td max-w-md"><div className="line-clamp-2 text-slate-600">{j.preview || "—"}</div></td>
                  <td className="td whitespace-nowrap text-xs text-slate-500">{fmtDate(j.created_at)}</td>
                  <td className="td whitespace-nowrap text-right"><Link className="btn-ghost btn-sm" href={`/history/${j.id}`}>Open</Link>{" "}<button className="btn-danger btn-sm" onClick={() => del(j.id)}>Delete</button></td>
                </tr>))}
            </tbody>
          </table>)}
        {data && <Pager page={page} total={data.total} per={15} onPage={setPage} />}
      </div>
    </>
  );
}

export default function Page() {
  return <Shell><Body /></Shell>;
}
