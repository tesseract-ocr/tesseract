"use client";
import { useState } from "react";
import Shell from "@/components/Shell";
import { Badge } from "@/components/ui";
import { useApiBase } from "@/lib/useApiBase";
import docs from "@/lib/api-docs.json";

const METHOD_TONE: Record<string, "green" | "blue" | "amber" | "red"> = { GET: "green", POST: "blue", PATCH: "amber", DELETE: "red" };
const SCOPES = [
  ["read", "Read OCR jobs and this key's usage"], ["write", "Run OCR and submit training data"],
  ["learning:read", "Read modules, lessons and progress"], ["learning:write", "Update progress and submit quizzes"], ["admin", "Everything (administrators only)"],
];

function Code({ text, label }: { text: string; label: string }) {
  const [done, setDone] = useState(false);
  const copy = async () => {
    try { await navigator.clipboard.writeText(text); } catch { /* clipboard blocked: the text is still selectable */ }
    setDone(true); setTimeout(() => setDone(false), 1500);
  };
  return (
    <div className="relative">
      <button onClick={copy} className="absolute right-2 top-2 rounded bg-white/10 px-2 py-0.5 text-xs text-slate-200 hover:bg-white/20" aria-label={`Copy ${label}`}>{done ? "Copied ✓" : "Copy"}</button>
      <pre className="overflow-x-auto rounded-lg bg-slate-900 p-4 pr-16 text-xs leading-relaxed text-slate-100" aria-label={label}>{text}</pre>
    </div>
  );
}

export default function ApiDocsPage() {
  return <Shell><Body /></Shell>;
}

function Body() {
  const API = useApiBase();
  const groups = Array.from(new Set(docs.endpoints.map(e => e.group)));
  const [open, setOpen] = useState<Record<string, number>>({});
  return (
    <>
      <h1 className="text-xl font-bold">API documentation</h1>
      <p className="mt-1 text-sm text-slate-600">Base URL <code className="rounded bg-slate-100 px-1">{API}</code>. Interactive reference (Swagger): <a className="text-brand-600 underline" href={`${API}/docs`} target="_blank" rel="noreferrer">{API}/docs</a>.</p>

      <section className="card mt-4 p-5">
        <h2 className="font-semibold">Authentication</h2>
        <p className="mt-1 text-sm text-slate-600">Send your key in the <code className="rounded bg-slate-100 px-1">{docs.auth.header}</code> header. {docs.auth.note} Manage keys on the <a className="text-brand-600 underline" href="/api-keys">API keys</a> page.</p>
        <div className="mt-3 overflow-x-auto"><table className="w-full text-sm"><thead className="bg-slate-50"><tr><th className="th">Scope</th><th className="th">Allows</th></tr></thead>
          <tbody className="divide-y divide-slate-100">{SCOPES.map(([s, d]) => <tr key={s}><td className="td"><code className="font-semibold">{s}</code></td><td className="td text-slate-600">{d}</td></tr>)}</tbody></table></div>
        <p className="mt-2 text-xs text-slate-500">A key whose scopes don&apos;t cover an endpoint gets <b>403 insufficient_scope</b>. An unknown, disabled or expired key gets <b>401</b>.</p>
      </section>

      <nav className="mt-4 flex flex-wrap gap-2" aria-label="Endpoints">
        {docs.endpoints.map((e, i) => <a key={i} href={`#ep-${i}`} className="rounded-md border border-slate-200 bg-white px-2 py-1 text-xs hover:border-brand-500"><b className="mr-1">{e.method}</b>{e.path}</a>)}
      </nav>

      {groups.map(g => (
        <section key={g} className="mt-6">
          <h2 className="mb-3 text-lg font-semibold">{g}</h2>
          <div className="space-y-4">
            {docs.endpoints.map((e, i) => e.group !== g ? null : (
              <article key={i} id={`ep-${i}`} className="card scroll-mt-4 p-5" data-testid={`endpoint-${e.method}-${e.path}`}>
                <div className="flex flex-wrap items-center gap-2">
                  <Badge tone={METHOD_TONE[e.method]}>{e.method}</Badge>
                  <code className="text-sm font-semibold">{e.path}</code>
                  <span className="ml-auto text-xs text-slate-500">Required scope <code className="ml-1 rounded bg-brand-50 px-1.5 py-0.5 font-semibold text-brand-700">{e.scope}</code></span>
                </div>
                <h3 className="mt-2 font-medium">{e.summary}</h3>
                <p className="text-sm text-slate-600">{e.description}</p>
                {e.params.length > 0 && (
                  <table className="mt-3 w-full text-sm"><thead className="bg-slate-50"><tr><th className="th">Parameter</th><th className="th">In</th><th className="th">Description</th></tr></thead>
                    <tbody className="divide-y divide-slate-100">{e.params.map(p => <tr key={p.name}><td className="td"><code>{p.name}</code></td><td className="td text-slate-500">{p.in}</td><td className="td text-slate-600">{p.desc}</td></tr>)}</tbody></table>)}
                <div className="mt-3 text-xs font-semibold uppercase text-slate-500">Example request</div>
                <Code label={`curl example for ${e.method} ${e.path}`} text={e.curl.replaceAll("$API", API)} />
                <div className="mt-3 flex flex-wrap items-center gap-2 text-xs font-semibold uppercase text-slate-500">Example response
                  {e.responses.map((r, n) => (
                    <button key={n} onClick={() => setOpen(o => ({ ...o, [i]: n }))} className={`rounded px-2 py-0.5 normal-case ${(open[i] ?? 0) === n ? "bg-brand-600 text-white" : "bg-slate-100 text-slate-600"}`}>{r.status} {r.label}</button>))}
                </div>
                <Code label={`JSON response for ${e.method} ${e.path}`} text={JSON.stringify(e.responses[open[i] ?? 0].body, null, 2)} />
              </article>))}
          </div>
        </section>))}

      <section className="card mt-6 p-5">
        <h2 className="font-semibold">Errors</h2>
        <p className="mt-1 text-sm text-slate-600">Every error has the same shape:</p>
        <div className="mt-2"><Code label="error response" text={JSON.stringify(docs.errorExample, null, 2)} /></div>
        <div className="mt-3 overflow-x-auto"><table className="w-full text-sm"><thead className="bg-slate-50"><tr><th className="th">Status</th><th className="th">Code</th><th className="th">Meaning</th></tr></thead>
          <tbody className="divide-y divide-slate-100">{docs.errors.map((x, i) => <tr key={i}><td className="td"><Badge tone={x.status < 500 ? "amber" : "red"}>{x.status}</Badge></td><td className="td"><code className="text-xs">{x.code}</code></td><td className="td text-slate-600">{x.meaning}</td></tr>)}</tbody></table></div>
      </section>
    </>
  );
}
