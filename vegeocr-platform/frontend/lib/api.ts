// NEXT_PUBLIC_API_URL unset  -> http://localhost:8000 (local development).
// NEXT_PUBLIC_API_URL=""     -> same origin: the app and /api are served from one address (the production setup behind Caddy),
//                               so the site works from any address (IP, domain, SSH tunnel) with no rebuild.
const configured = process.env.NEXT_PUBLIC_API_URL;
export const API = configured === undefined ? "http://localhost:8000" : configured.replace(/\/+$/, "");

export type Scope = "user" | "admin";
const KEY = (s: Scope) => (s === "admin" ? "vocr_admin" : "vocr_user");

export type Session = { token: string; user: { id: number; email: string; full_name: string; role: string } };

export function getSession(scope: Scope = "user"): Session | null {
  if (typeof window === "undefined") return null;
  try { return JSON.parse(localStorage.getItem(KEY(scope)) || "null"); } catch { return null; }
}
export function setSession(scope: Scope, s: Session | null) {
  if (s) localStorage.setItem(KEY(scope), JSON.stringify(s)); else localStorage.removeItem(KEY(scope));
}

export class ApiFail extends Error {
  status: number; code: string;
  constructor(status: number, code: string, message: string) { super(message); this.status = status; this.code = code; }
}

async function raw(path: string, scope: Scope, init: RequestInit = {}) {
  const s = getSession(scope);
  const headers: Record<string, string> = { ...(init.headers as any) };
  if (s) headers.Authorization = `Bearer ${s.token}`;
  if (init.body && !(init.body instanceof FormData)) headers["Content-Type"] = "application/json";
  let res: Response;
  try { res = await fetch(API + path, { ...init, headers }); }
  catch { throw new ApiFail(0, "network", "Cannot reach the server. Is the API running?"); }
  if (res.status === 401 && s && !path.includes("/auth/")) {
    setSession(scope, null);
    if (typeof window !== "undefined") window.location.href = scope === "admin" ? "/admin/login" : "/login";
  }
  if (!res.ok) {
    let code = "error", msg = `Request failed (${res.status})`;
    try { const j = await res.json(); code = j.error?.code || code; msg = j.error?.message || msg; } catch {}
    throw new ApiFail(res.status, code, msg);
  }
  return res;
}

export async function api<T = any>(path: string, opts: { method?: string; body?: any; scope?: Scope } = {}): Promise<T> {
  const res = await raw(path, opts.scope || "user", {
    method: opts.method || (opts.body ? "POST" : "GET"),
    body: opts.body instanceof FormData ? opts.body : opts.body ? JSON.stringify(opts.body) : undefined,
  });
  return res.status === 204 ? (null as T) : res.json();
}

export async function blob(path: string, scope: Scope = "user") { return (await raw(path, scope)).blob(); }

export async function download(path: string, filename: string, scope: Scope = "user") {
  const url = URL.createObjectURL(await blob(path, scope));
  const a = document.createElement("a"); a.href = url; a.download = filename; a.click();
  setTimeout(() => URL.revokeObjectURL(url), 5000);
}

export const LANGS: [string, string][] = [
  ["jpn", "日本語 Japanese"], ["jpn_vert", "日本語 Japanese (vertical)"],
  ["chi_sim", "简体中文 Chinese (Simplified)"], ["chi_tra", "繁體中文 Chinese (Traditional)"],
  ["jpn+chi_sim", "日本語 + 简体中文"], ["jpn+chi_tra", "日本語 + 繁體中文"],
];
