"use client";
import Link from "next/link";
import { useRouter, useSearchParams } from "next/navigation";
import { FormEvent, Suspense, useState } from "react";
import { api } from "@/lib/api";
import { AuthCard } from "@/components/Shell";
import { Err } from "@/components/ui";

function Form() {
  const token = useSearchParams().get("token") || "";
  const router = useRouter();
  const [err, setErr] = useState<string | null>(null);
  const submit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null);
    const f = new FormData(e.currentTarget);
    if (f.get("password") !== f.get("confirm")) { setErr("Passwords do not match"); return; }
    try { await api("/api/web/auth/reset", { body: { token, password: f.get("password") } }); router.replace("/login"); }
    catch (x: any) { setErr(x.message); }
  };
  return (
    <AuthCard title="Choose a new password" footer={<Link className="text-brand-600 hover:underline" href="/login">Back to sign in</Link>}>
      <form onSubmit={submit} className="space-y-4">
        <Err msg={err} />
        <label className="block text-sm">New password<input name="password" type="password" required minLength={8} className="input mt-1" autoComplete="new-password" /></label>
        <label className="block text-sm">Confirm<input name="confirm" type="password" required className="input mt-1" autoComplete="new-password" /></label>
        <button className="btn-primary w-full">Reset password</button>
      </form>
    </AuthCard>
  );
}
export default function Reset() { return <Suspense><Form /></Suspense>; }
