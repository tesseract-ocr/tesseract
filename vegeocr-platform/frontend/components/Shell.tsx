"use client";
import Link from "next/link";
import { usePathname, useRouter } from "next/navigation";
import { ReactNode, useEffect, useState } from "react";
import { api, getSession, Scope, setSession } from "@/lib/api";

const USER_NAV = [["/dashboard", "New OCR"], ["/history", "History"], ["/learning", "Learning"], ["/api-keys", "API keys"], ["/api-docs", "API docs"], ["/account", "Account"]];
const ADMIN_NAV = [["/admin", "Overview"], ["/admin/users", "Users"], ["/admin/keys", "API keys"], ["/admin/usage", "Usage"],
                   ["/admin/models", "Models"], ["/admin/training-data", "Training data"], ["/admin/account", "Account"]];

/** Protects a page and renders the nav. scope="admin" requires an admin session. */
export default function Shell({ scope = "user", children }: { scope?: Scope; children: ReactNode }) {
  const router = useRouter(), path = usePathname();
  const [ready, setReady] = useState(false);
  const [name, setName] = useState("");
  useEffect(() => {
    const s = getSession(scope);
    if (!s || (scope === "admin" && s.user.role !== "admin")) { router.replace(scope === "admin" ? "/admin/login" : "/login"); return; }
    setName(s.user.full_name || s.user.email);
    setReady(true);
  }, [scope, router]);
  if (!ready) return null;
  const nav = scope === "admin" ? ADMIN_NAV : USER_NAV;
  const logout = async () => {
    try { await api("/api/web/auth/logout", { method: "POST", scope }); } catch {}
    setSession(scope, null);
    router.replace(scope === "admin" ? "/admin/login" : "/login");
  };
  return (
    <div className="min-h-screen">
      <header className="border-b border-slate-200 bg-white">
        <div className="mx-auto flex max-w-6xl flex-wrap items-center gap-x-6 gap-y-2 px-4 py-3">
          <Link href={scope === "admin" ? "/admin" : "/dashboard"} className="flex items-center gap-2 font-bold">
            <span className="grid h-8 w-8 place-items-center rounded-lg bg-brand-600 text-white">V</span>
            VegeOCR{scope === "admin" && <span className="rounded bg-slate-800 px-1.5 py-0.5 text-[10px] font-semibold text-white">ADMIN</span>}
          </Link>
          <nav className="flex flex-wrap gap-1">
            {nav.map(([href, label]) => {
              const on = href === "/admin" ? path === href : path.startsWith(href);
              return <Link key={href} href={href} className={`rounded-md px-3 py-1.5 text-sm font-medium ${on ? "bg-brand-50 text-brand-700" : "text-slate-600 hover:bg-slate-100"}`}>{label}</Link>;
            })}
          </nav>
          <div className="ml-auto flex items-center gap-3 text-sm text-slate-600">
            <Link href={scope === "admin" ? "/admin/account" : "/account"} className="hidden hover:text-brand-600 sm:inline" title="Account settings">{name}</Link>
            <button onClick={logout} className="btn-ghost btn-sm">Log out</button>
          </div>
        </div>
      </header>
      <main className="mx-auto max-w-6xl px-4 py-6">{children}</main>
    </div>
  );
}

export function AuthCard({ title, subtitle, children, footer }: { title: string; subtitle?: string; children: ReactNode; footer?: ReactNode }) {
  return (
    <div className="grid min-h-screen place-items-center px-4 py-10">
      <div className="w-full max-w-sm">
        <div className="mb-6 flex items-center justify-center gap-2 text-xl font-bold">
          <span className="grid h-9 w-9 place-items-center rounded-lg bg-brand-600 text-white">V</span> VegeOCR
        </div>
        <div className="card p-6">
          <h1 className="text-lg font-semibold">{title}</h1>
          {subtitle && <p className="mt-1 text-sm text-slate-500">{subtitle}</p>}
          <div className="mt-5">{children}</div>
        </div>
        {footer && <div className="mt-4 text-center text-sm text-slate-600">{footer}</div>}
      </div>
    </div>
  );
}
