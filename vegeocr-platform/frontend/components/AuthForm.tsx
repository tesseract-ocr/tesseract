"use client";
import Link from "next/link";
import { useRouter } from "next/navigation";
import { FormEvent, useEffect, useState } from "react";
import { api, getSession, Scope, setSession } from "@/lib/api";
import { AuthCard } from "./Shell";
import { Err } from "./ui";

export default function AuthForm({ mode, scope = "user" }: { mode: "login" | "signup"; scope?: Scope }) {
  const router = useRouter();
  const [err, setErr] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);
  const [show, setShow] = useState(false);
  // Already signed in? Skip the form.
  useEffect(() => {
    const s = getSession(scope);
    if (s && (scope === "user" || s.user.role === "admin")) router.replace(scope === "admin" ? "/admin" : "/dashboard");
  }, [scope, router]);
  const submit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null); setBusy(true);
    const f = new FormData(e.currentTarget);
    try {
      const path = mode === "signup" ? "/api/web/auth/signup" : scope === "admin" ? "/api/web/auth/admin-login" : "/api/web/auth/login";
      const body: any = { email: f.get("email"), password: f.get("password") };
      if (mode === "signup") body.full_name = f.get("full_name");
      const s = await api(path, { body, scope });
      setSession(scope, s);
      router.replace(scope === "admin" ? "/admin" : "/dashboard");
    } catch (x: any) { setErr(x.message); setBusy(false); }
  };
  const admin = scope === "admin";
  return (
    <AuthCard
      title={admin ? "Admin sign in" : mode === "signup" ? "Create your account" : "Sign in"}
      subtitle={admin ? "Restricted area. Administrators only." : mode === "signup" ? "Start extracting text from images and PDFs." : "Welcome back. Sign in to continue."}
      footer={admin ? <Link className="text-brand-600 hover:underline" href="/login">User sign in</Link> : mode === "signup"
        ? <>Already have an account? <Link className="text-brand-600 hover:underline" href="/login">Sign in</Link></>
        : <>New here? <Link className="text-brand-600 hover:underline" href="/signup">Create an account</Link></>}>
      <form onSubmit={submit} className="space-y-4">
        <Err msg={err} />
        {mode === "signup" && <label className="block text-sm">Name<input name="full_name" className="input mt-1" autoComplete="name" /></label>}
        <label className="block text-sm">{mode === "login" ? "Email or username" : "Email"}
          <input name="email" type={mode === "login" ? "text" : "email"} required autoFocus className="input mt-1" autoComplete={mode === "login" ? "username" : "email"} />
        </label>
        <label className="block text-sm">Password
          <div className="relative mt-1">
            <input name="password" type={show ? "text" : "password"} required minLength={mode === "signup" ? 8 : 1} className="input pr-16"
                   autoComplete={mode === "signup" ? "new-password" : "current-password"} />
            <button type="button" onClick={() => setShow(v => !v)} className="absolute inset-y-0 right-2 my-auto h-7 rounded px-2 text-xs font-medium text-slate-500 hover:text-brand-600"
                    aria-label={show ? "Hide password" : "Show password"}>{show ? "Hide" : "Show"}</button>
          </div>
          {mode === "signup" && <span className="mt-1 block text-xs text-slate-500">At least 8 characters with a letter and a number.</span>}
        </label>
        <button className="btn-primary w-full" disabled={busy}>{busy ? "Please wait…" : mode === "signup" ? "Sign up" : "Sign in"}</button>
        {mode === "login" && !admin && <div className="text-center text-sm"><Link className="text-slate-500 hover:text-brand-600" href="/forgot">Forgot password?</Link></div>}
      </form>
    </AuthCard>
  );
}
