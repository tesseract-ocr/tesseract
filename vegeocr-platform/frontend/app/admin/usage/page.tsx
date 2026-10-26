"use client";
import { useEffect, useState } from "react";
import { Bar, BarChart, CartesianGrid, Legend, ResponsiveContainer, Tooltip, XAxis, YAxis } from "recharts";
import Shell from "@/components/Shell";
import { Err, Spinner, Stat } from "@/components/ui";
import { api } from "@/lib/api";

function Body() {
  const [days, setDays] = useState(14);
  const [d, setD] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  useEffect(() => { setD(null); api(`/api/admin/usage?days=${days}`, { scope: "admin" }).then(setD).catch(x => setErr(x.message)); }, [days]);
  const rate = (r: any) => (r.requests ? `${Math.round((100 * (r.requests - r.errors)) / r.requests)}%` : "—");
  const chart = d?.daily.map((x: any) => ({ date: x.date.slice(5), Success: x.requests - x.errors, Errors: x.errors })) || [];
  return (
    <>
      <div className="mb-4 flex items-center gap-3"><h1 className="text-xl font-bold">Usage</h1>
        <select className="input ml-auto w-auto" value={days} onChange={e => setDays(+e.target.value)}>{[7, 14, 30, 90].map(n => <option key={n} value={n}>Last {n} days</option>)}</select></div>
      <Err msg={err} />
      {!d ? <Spinner /> : (
        <>
          <div className="grid grid-cols-3 gap-3"><Stat label="Requests" value={d.total} /><Stat label="Errors" value={d.errors} /><Stat label="Success rate" value={d.success_rate != null ? `${d.success_rate}%` : "—"} /></div>
          <div className="card mt-4 p-4">
            <h2 className="mb-2 font-semibold">Requests per day</h2>
            {chart.length === 0 ? <div className="p-8 text-center text-sm text-slate-500">No API traffic in this period.</div> : (
              <div className="h-64"><ResponsiveContainer><BarChart data={chart}><CartesianGrid strokeDasharray="3 3" vertical={false} /><XAxis dataKey="date" fontSize={12} /><YAxis allowDecimals={false} fontSize={12} /><Tooltip /><Legend />
                <Bar dataKey="Success" stackId="a" fill="#43a047" /><Bar dataKey="Errors" stackId="a" fill="#e53935" /></BarChart></ResponsiveContainer></div>)}
          </div>
          <div className="mt-4 grid gap-4 lg:grid-cols-2">
            <div className="card overflow-x-auto"><h2 className="p-4 pb-0 font-semibold">By API key</h2>
              <table className="mt-2 w-full"><thead className="border-b bg-slate-50"><tr><th className="th">Key</th><th className="th">Requests</th><th className="th">Errors</th><th className="th">Success</th></tr></thead>
                <tbody className="divide-y divide-slate-100">{d.by_key.map((r: any, i: number) => <tr key={i}><td className="td">{r.name || "—"}<div className="text-xs text-slate-500">{r.owner}</div></td><td className="td">{r.requests}</td><td className="td">{r.errors}</td><td className="td">{rate(r)}</td></tr>)}</tbody></table></div>
            <div className="card overflow-x-auto"><h2 className="p-4 pb-0 font-semibold">By user</h2>
              <table className="mt-2 w-full"><thead className="border-b bg-slate-50"><tr><th className="th">User</th><th className="th">Requests</th><th className="th">Errors</th><th className="th">Success</th></tr></thead>
                <tbody className="divide-y divide-slate-100">{d.by_user.map((r: any, i: number) => <tr key={i}><td className="td">{r.email || "—"}</td><td className="td">{r.requests}</td><td className="td">{r.errors}</td><td className="td">{rate(r)}</td></tr>)}</tbody></table></div>
          </div>
        </>)}
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
