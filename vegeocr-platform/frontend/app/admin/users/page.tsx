"use client";
import { FormEvent, useCallback, useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, Err, fmtDate, Ok, Pager, Spinner, statusTone } from "@/components/ui";
import { api } from "@/lib/api";

const A = { scope: "admin" as const };

function Body() {
  const [data, setData] = useState<any>(null);
  const [q, setQ] = useState(""); const [page, setPage] = useState(1);
  const [err, setErr] = useState<string | null>(null); const [ok, setOk] = useState<string | null>(null);
  const [edit, setEdit] = useState<any>(null); const [creating, setCreating] = useState(false);
  const load = useCallback(() => api(`/api/admin/users?page=${page}&q=${encodeURIComponent(q)}`, A).then(setData).catch(x => setErr(x.message)), [page, q]);
  useEffect(() => { const t = setTimeout(load, 250); return () => clearTimeout(t); }, [load]);
  const act = async (fn: () => Promise<any>, msg: string) => { setErr(null); try { await fn(); setOk(msg); setEdit(null); setCreating(false); load(); } catch (x: any) { setErr(x.message); } };

  const create = (e: FormEvent<HTMLFormElement>) => { e.preventDefault(); const f = Object.fromEntries(new FormData(e.currentTarget)); act(() => api("/api/admin/users", { ...A, body: f }), "User created."); };
  const save = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); const f: any = Object.fromEntries(new FormData(e.currentTarget)); if (!f.password) delete f.password;
    act(() => api(`/api/admin/users/${edit.id}`, { ...A, method: "PATCH", body: f }), "User updated.");
  };
  return (
    <>
      <div className="mb-4 flex flex-wrap items-center gap-3">
        <h1 className="text-xl font-bold">Users</h1>
        <input className="input ml-auto max-w-xs" placeholder="Search email or name…" value={q} onChange={e => { setQ(e.target.value); setPage(1); }} />
        <button className="btn-primary" onClick={() => { setCreating(true); setEdit(null); }}>New user</button>
      </div>
      <Err msg={err} /><Ok msg={ok} />
      {(creating || edit) && (
        <form onSubmit={creating ? create : save} className="card mb-4 grid gap-3 p-4 sm:grid-cols-2 lg:grid-cols-5" key={edit?.id || "new"}>
          {creating && <label className="text-sm">Email<input name="email" type="email" required className="input mt-1" /></label>}
          <label className="text-sm">Name<input name="full_name" defaultValue={edit?.full_name} className="input mt-1" /></label>
          <label className="text-sm">Role<select name="role" defaultValue={edit?.role || "user"} className="input mt-1"><option>user</option><option>admin</option></select></label>
          {edit && <label className="text-sm">Status<select name="status" defaultValue={edit.status} className="input mt-1"><option>active</option><option>suspended</option></select></label>}
          <label className="text-sm">{creating ? "Password" : "New password"}<input name="password" type="password" required={creating} minLength={8} className="input mt-1" placeholder={edit ? "leave blank to keep" : ""} autoComplete="new-password" /></label>
          <div className="flex items-end gap-2"><button className="btn-primary">{creating ? "Create" : "Save"}</button><button type="button" className="btn-ghost" onClick={() => { setEdit(null); setCreating(false); }}>Cancel</button></div>
        </form>)}
      <div className="card overflow-x-auto">
        {!data ? <Spinner /> : (
          <table className="w-full"><thead className="border-b bg-slate-50"><tr><th className="th">User</th><th className="th">Role</th><th className="th">Status</th><th className="th">Jobs</th><th className="th">Last login</th><th className="th" /></tr></thead>
            <tbody className="divide-y divide-slate-100">{data.items.map((u: any) => (
              <tr key={u.id}><td className="td"><div className="font-medium">{u.email}</div><div className="text-xs text-slate-500">{u.full_name}</div></td>
                <td className="td"><Badge tone={u.role === "admin" ? "blue" : "slate"}>{u.role}</Badge></td>
                <td className="td"><Badge tone={statusTone(u.status)}>{u.status}</Badge></td>
                <td className="td">{u.jobs}</td><td className="td text-xs text-slate-500">{fmtDate(u.last_login_at)}</td>
                <td className="td space-x-1 whitespace-nowrap text-right">
                  <button className="btn-ghost btn-sm" onClick={() => { setEdit(u); setCreating(false); }}>Edit</button>
                  <button className="btn-ghost btn-sm" onClick={() => act(() => api(`/api/admin/users/${u.id}`, { ...A, method: "PATCH", body: { status: u.status === "active" ? "suspended" : "active" } }), u.status === "active" ? "User suspended." : "User reactivated.")}>{u.status === "active" ? "Suspend" : "Activate"}</button>
                  <button className="btn-danger btn-sm" onClick={() => confirm(`Delete ${u.email} and all their data?`) && act(() => api(`/api/admin/users/${u.id}`, { ...A, method: "DELETE" }), "User deleted.")}>Delete</button>
                </td></tr>))}
            </tbody></table>)}
        {data && <Pager page={page} total={data.total} per={25} onPage={setPage} />}
      </div>
    </>
  );
}

export default function Page() {
  return <Shell scope="admin"><Body /></Shell>;
}
