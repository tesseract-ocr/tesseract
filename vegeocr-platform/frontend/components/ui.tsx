"use client";
import { ReactNode } from "react";

export function Badge({ children, tone = "slate" }: { children: ReactNode; tone?: "slate" | "green" | "red" | "amber" | "blue" }) {
  const t = { slate: "bg-slate-100 text-slate-700", green: "bg-green-100 text-green-800", red: "bg-red-100 text-red-700",
              amber: "bg-amber-100 text-amber-800", blue: "bg-blue-100 text-blue-800" }[tone];
  return <span className={`inline-block rounded-full px-2 py-0.5 text-xs font-semibold ${t}`}>{children}</span>;
}

export function statusTone(s: string) {
  return ({ done: "green", active: "green", approved: "green", deployed: "green", completed: "green", queued: "amber", processing: "amber",
            running: "amber", pending: "amber", expired: "amber", candidate: "blue", in_progress: "blue", failed: "red", rejected: "red",
            suspended: "red", revoked: "red", disabled: "slate", archived: "slate", not_started: "slate" } as any)[s] || "slate";
}

export function Err({ msg }: { msg?: string | null }) {
  return msg ? <div role="alert" className="mb-4 rounded-lg border border-red-200 bg-red-50 px-4 py-2 text-sm text-red-700">{msg}</div> : null;
}
export function Ok({ msg }: { msg?: string | null }) {
  return msg ? <div role="status" className="mb-4 rounded-lg border border-green-200 bg-green-50 px-4 py-2 text-sm text-green-800">{msg}</div> : null;
}
export function Spinner() { return <div className="p-10 text-center text-sm text-slate-500">Loading…</div>; }
export function Empty({ children }: { children: ReactNode }) { return <div className="p-10 text-center text-sm text-slate-500">{children}</div>; }

export function Stat({ label, value, sub }: { label: string; value: ReactNode; sub?: string }) {
  return (
    <div className="card p-4">
      <div className="text-xs font-semibold uppercase tracking-wide text-slate-500">{label}</div>
      <div className="mt-1 text-2xl font-bold">{value}</div>
      {sub && <div className="text-xs text-slate-500">{sub}</div>}
    </div>
  );
}

export function Pager({ page, total, per, onPage }: { page: number; total: number; per: number; onPage: (p: number) => void }) {
  const pages = Math.max(1, Math.ceil(total / per));
  if (pages <= 1) return null;
  return (
    <div className="flex items-center justify-between border-t border-slate-200 px-4 py-3 text-sm text-slate-600">
      <span>{total} total</span>
      <div className="flex items-center gap-2">
        <button className="btn-ghost btn-sm" disabled={page <= 1} onClick={() => onPage(page - 1)}>Prev</button>
        <span>Page {page} / {pages}</span>
        <button className="btn-ghost btn-sm" disabled={page >= pages} onClick={() => onPage(page + 1)}>Next</button>
      </div>
    </div>
  );
}

export const fmtDay = (s?: string | null) => (s ? new Date(s).toLocaleDateString() : "—");
export const fmtDate = (s?: string | null) => (s ? new Date(s).toLocaleString() : "—");

export const STATUS_LABEL: Record<string, string> = { not_started: "Not started", in_progress: "In progress", completed: "Completed" };

export function ProgressBar({ percent, label }: { percent: number; label?: string }) {
  return (
    <div role="progressbar" aria-valuenow={percent} aria-valuemin={0} aria-valuemax={100} aria-label={label || "Progress"}>
      <div className="h-2 overflow-hidden rounded-full bg-slate-100"><div className="h-2 rounded-full bg-brand-500 transition-all" style={{ width: `${percent}%` }} /></div>
    </div>
  );
}

export const DIFFICULTY_TONE: Record<string, "green" | "amber" | "red"> = { beginner: "green", intermediate: "amber", advanced: "red" };
