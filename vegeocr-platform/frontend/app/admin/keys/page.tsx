"use client";
import { useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, Err, fmtDate, Ok, Spinner } from "@/components/ui";
import { api } from "@/lib/api";

const A = { scope: "admin" as const };

function Row({ k, reload, setErr, setOk }: any) {
  const [rl, setRl] = useState(k.rate_limit_per_min), [mq, setMq] = useState(k.monthly_quota);
  const dirty = rl !== k.rate_limit_per_min || mq !== k.monthly_quota;
  const pct = k.monthly_quota ? Math.min(100, Math.round((100 * k.used_this_month) / k.monthly_quota)) : 100;
  const save = async () => { try { await api(`/api/admin/keys/${k.id}`, { ...A, method: "PATCH", body: { rate_limit_per_min: +rl, monthly_quota: +mq } }); setOk("Limits saved."); reload(); } catch (x: any) { setErr(x.message); } };
  const revoke = async () => { if (confirm(`Revoke ${k.prefix}…?`)) { try { await api(`/api/admin/keys/${k.id}/revoke`, { ...A, method: "POST" }); reload(); } catch (x: any) { setErr(x.message); } } };
  return (
    <tr>
      <td className="td"><div className="font-medium">{k.name} {k.status !== "active" && <Badge tone={k.status === "revoked" ? "red" : "slate"}>{k.status}</Badge>}</div><code className="text-xs text-slate-500">{k.masked}</code><div className="mt-1 flex flex-wrap gap-1">{(k.scopes || []).map((sc: string) => <Badge key={sc}>{sc}</Badge>)}</div></td>
      <td className="td text-sm">{k.owner}</td>
      <td className="td"><input type="number" min={1} className="input w-24" value={rl} disabled={k.revoked} onChange={e => setRl(+e.target.value)} /></td>
      <td className="td"><input type="number" min={0} className="input w-28" value={mq} disabled={k.revoked} onChange={e => setMq(+e.target.value)} /></td>
      <td className="td w-40"><div className="h-2 rounded bg-slate-100"><div className={`h-2 rounded ${pct >= 90 ? "bg-red-500" : "bg-brand-500"}`} style={{ width: `${pct}%` }} /></div><div className="mt-1 text-xs text-slate-500">{k.used_this_month} / {k.monthly_quota}</div></td>
      <td className="td text-xs text-slate-500">{fmtDate(k.last_used_at)}</td>
      <td className="td space-x-1 whitespace-nowrap text-right">{dirty && <button className="btn-primary btn-sm" onClick={save}>Save</button>}{!k.revoked && <button className="btn-danger btn-sm" onClick={revoke}>Revoke</button>}</td>
    </tr>
  );
}

function Body() {
  const [keys, setKeys] = useState<any[] | null>(null);
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null);
  const load = () => api("/api/admin/keys", A).then(setKeys).catch(x => setErr(x.message));
  useEffect(() => { load(); }, []);
  return (
    <>
      <h1 className="mb-4 text-xl font-bold">API keys</h1>
      <Err msg={err} /><Ok msg={ok} />
      <div className="card overflow-x-auto">
        {!keys ? <Spinner /> : (
          <table className="w-full"><thead className="border-b bg-slate-50"><tr><th className="th">Key</th><th className="th">Owner</th><th className="th">Rate / min</th><th className="th">Monthly quota</th><th className="th">Used this month</th><th className="th">Last used</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{keys.map(k => <Row key={`${k.id}-${k.rate_limit_per_min}-${k.monthly_quota}`} k={k} reload={load} setErr={setErr} setOk={setOk} />)}</tbody></table>)}
      </div>
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
