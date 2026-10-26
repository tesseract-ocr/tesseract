"use client";
import Link from "next/link";
import { useEffect, useMemo, useState } from "react";
import LearningTabs from "@/components/LearningTabs";
import Shell from "@/components/Shell";
import { Badge, DIFFICULTY_TONE, Empty, Err, ProgressBar, Spinner, STATUS_LABEL, statusTone } from "@/components/ui";
import { api } from "@/lib/api";

export default function LearningPage() {
  return <Shell><Body /></Shell>;
}

function Body() {
  const [modules, setModules] = useState<any[] | null>(null);
  const [progress, setProgress] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  const [topic, setTopic] = useState(""); const [difficulty, setDifficulty] = useState("");
  useEffect(() => {
    api("/api/web/learning/modules").then(setModules).catch(x => setErr(x.message));
    api("/api/web/learning/progress").then(setProgress).catch(() => {});
  }, []);
  const topics = useMemo(() => Array.from(new Set((modules || []).map(m => m.topic))), [modules]);
  const shown = (modules || []).filter(m => (!topic || m.topic === topic) && (!difficulty || m.difficulty === difficulty));
  const cont = progress?.continue;

  return (
    <>
      <div className="mb-4 flex flex-wrap items-center gap-3">
        <h1 className="text-xl font-bold">Learning</h1>
        {progress && <span className="text-sm text-slate-500">{progress.summary.completed} of {progress.summary.modules_total} modules completed</span>}
      </div>
      <LearningTabs />
      <Err msg={err} />

      {cont && (
        <div className="mb-5 flex flex-wrap items-center gap-3 rounded-xl border border-brand-100 bg-brand-50 p-4" data-testid="continue-card">
          <div className="min-w-0 flex-1">
            <div className="text-xs font-semibold uppercase tracking-wide text-brand-700">Continue where you left off</div>
            <div className="truncate font-semibold">{cont.lesson_title}</div>
            <div className="truncate text-sm text-slate-600">{cont.module_title}</div>
          </div>
          <Link className="btn-primary" href={`/learning/${cont.module_id}/lessons/${cont.lesson_id}`}>Continue →</Link>
        </div>)}

      <div className="mb-4 flex flex-wrap gap-2">
        <select className="input w-auto" value={topic} onChange={e => setTopic(e.target.value)} aria-label="Filter by topic"><option value="">All topics</option>{topics.map(t => <option key={t}>{t}</option>)}</select>
        <select className="input w-auto" value={difficulty} onChange={e => setDifficulty(e.target.value)} aria-label="Filter by difficulty">
          <option value="">All levels</option><option value="beginner">Beginner</option><option value="intermediate">Intermediate</option><option value="advanced">Advanced</option></select>
      </div>

      {!modules ? <Spinner /> : shown.length === 0 ? <div className="card"><Empty>No modules match these filters.</Empty></div> : (
        <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
          {shown.map(m => {
            const p = m.progress;
            return (
              <Link key={m.id} href={`/learning/${m.id}`} data-testid={`module-${m.id}`} className="card flex flex-col p-4 transition hover:border-brand-500">
                <div className="flex flex-wrap items-center gap-2">
                  <Badge>{m.topic}</Badge><Badge tone={DIFFICULTY_TONE[m.difficulty]}>{m.difficulty}</Badge>
                </div>
                <h2 className="mt-2 font-semibold">{m.title}</h2>
                <p className="mt-1 line-clamp-3 flex-1 text-sm text-slate-600">{m.summary}</p>
                <div className="mt-3 text-xs text-slate-500">{m.lesson_count} lessons · about {m.estimated_minutes} min</div>
                <div className="mt-3"><ProgressBar percent={p.percent} label={`${m.title} progress`} /></div>
                <div className="mt-1.5 flex items-center justify-between text-xs">
                  <Badge tone={statusTone(p.status)}>{STATUS_LABEL[p.status]}</Badge>
                  <span className="text-slate-500">{p.percent}% · {p.lessons_completed}/{p.lessons_total}</span>
                </div>
              </Link>);
          })}
        </div>)}
    </>
  );
}
