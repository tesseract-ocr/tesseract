"use client";
import Link from "next/link";
import { useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Err, Spinner, Stat } from "@/components/ui";
import { api } from "@/lib/api";

function Body() {
  const [o, setO] = useState<any>(null); const [u, setU] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  useEffect(() => {
    api("/api/admin/overview", { scope: "admin" }).then(setO).catch(x => setErr(x.message));
    api("/api/admin/usage?days=7", { scope: "admin" }).then(setU).catch(() => {});
  }, []);
  const jobs = o?.jobs || {}, s = o?.samples || {};
  return (
    <>
      <h1 className="mb-4 text-xl font-bold">Overview</h1>
      <Err msg={err} />
      {!o ? <Spinner /> : (
        <>
          <div className="grid grid-cols-2 gap-3 lg:grid-cols-4">
            <Stat label="Users" value={o.users} />
            <Stat label="Active API keys" value={o.active_keys} />
            <Stat label="OCR jobs" value={Object.values(jobs).reduce((a: number, b: any) => a + b, 0) as number} sub={`${jobs.failed || 0} failed`} />
            <Stat label="API success (7d)" value={u?.success_rate != null ? `${u.success_rate}%` : "—"} sub={u ? `${u.total} requests` : ""} />
          </div>
          <div className="mt-6 grid gap-4 md:grid-cols-2">
            <div className="card p-5">
              <h2 className="font-semibold">Training data</h2>
              <div className="mt-3 flex gap-6 text-sm">
                <div><div className="text-2xl font-bold text-amber-600">{s.pending || 0}</div>pending review</div>
                <div><div className="text-2xl font-bold text-green-700">{s.approved || 0}</div>approved</div>
                <div><div className="text-2xl font-bold text-slate-400">{s.rejected || 0}</div>rejected</div>
              </div>
              <Link href="/admin/training-data" className="btn-ghost btn-sm mt-4">Review samples</Link>
            </div>
            <div className="card p-5">
              <h2 className="font-semibold">Active models</h2>
              <ul className="mt-3 space-y-1 text-sm">
                {o.models.map((m: any) => <li key={m.lang} className="flex justify-between"><span className="font-mono">{m.lang}</span>
                  <span>v{m.version}{m.cer != null && <span className="ml-2 text-slate-500">CER {(m.cer * 100).toFixed(1)}%</span>}</span></li>)}
              </ul>
              <Link href="/admin/models" className="btn-ghost btn-sm mt-4">Manage models</Link>
            </div>
          </div>
        </>)}
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
