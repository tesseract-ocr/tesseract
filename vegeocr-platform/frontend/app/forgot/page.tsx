"use client";
import Link from "next/link";
import { FormEvent, useState } from "react";
import { api } from "@/lib/api";
import { AuthCard } from "@/components/Shell";
import { Err, Ok } from "@/components/ui";

export default function Forgot() {
  const [err, setErr] = useState<string | null>(null);
  const [msg, setMsg] = useState<string | null>(null);
  const [dev, setDev] = useState<string | null>(null);
  const submit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); setErr(null);
    try {
      const r = await api("/api/web/auth/forgot", { body: { email: new FormData(e.currentTarget).get("email") } });
      setMsg(r.message); setDev(r.dev_token || null);
    } catch (x: any) { setErr(x.message); }
  };
  return (
    <AuthCard title="Forgot password" subtitle="We'll email you a link to reset it."
      footer={<Link className="text-brand-600 hover:underline" href="/login">Back to sign in</Link>}>
      <form onSubmit={submit} className="space-y-4">
        <Err msg={err} /><Ok msg={msg} />
        {dev && <p className="rounded bg-amber-50 p-2 text-xs text-amber-800">Dev mode: <Link className="underline" href={`/reset?token=${dev}`}>open reset link</Link></p>}
        <label className="block text-sm">Email<input name="email" type="email" required className="input mt-1" /></label>
        <button className="btn-primary w-full">Send reset link</button>
      </form>
    </AuthCard>
  );
}
