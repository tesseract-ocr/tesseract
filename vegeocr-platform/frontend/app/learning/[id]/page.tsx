"use client";
import Link from "next/link";
import { useParams } from "next/navigation";
import { useEffect, useState } from "react";
import Shell from "@/components/Shell";
import { Badge, DIFFICULTY_TONE, Err, ProgressBar, Spinner, STATUS_LABEL, statusTone } from "@/components/ui";
import { api } from "@/lib/api";

export default function ModulePage() {
  return <Shell><Body /></Shell>;
}

function Body() {
  const { id } = useParams<{ id: string }>();
  const [m, setM] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  useEffect(() => { api(`/api/web/learning/modules/${id}`).then(setM).catch(x => setErr(x.message)); }, [id]);
  if (err) return <><Link href="/learning" className="text-sm text-slate-500 hover:text-brand-600">← Learning</Link><div className="mt-3"><Err msg={err} /></div></>;
  if (!m) return <Spinner />;
  const p = m.progress, done = new Map<number, boolean>(p.lessons.map((l: any) => [l.lesson_id, l.completed]));
  const resume = p.continue_lesson_id;

  return (
    <>
      <Link href="/learning" className="text-sm text-slate-500 hover:text-brand-600">← Learning</Link>
      <div className="card mt-3 p-5">
        <div className="flex flex-wrap items-center gap-2"><Badge>{m.topic}</Badge><Badge tone={DIFFICULTY_TONE[m.difficulty]}>{m.difficulty}</Badge><Badge tone={statusTone(p.status)}>{STATUS_LABEL[p.status]}</Badge></div>
        <h1 className="mt-2 text-2xl font-bold">{m.title}</h1>
        <p className="mt-1 text-slate-600">{m.summary}</p>
        <div className="mt-4 max-w-md"><ProgressBar percent={p.percent} label="Module progress" />
          <div className="mt-1 text-xs text-slate-500">{p.percent}% complete · {p.lessons_completed} of {p.lessons_total} lessons · about {m.estimated_minutes} min</div></div>
        {resume && <Link className="btn-primary mt-4" href={`/learning/${m.id}/lessons/${resume}`}>{p.status === "not_started" ? "Start module" : "Continue where you left off"} →</Link>}
        {p.status === "completed" && <div className="mt-4 text-sm font-medium text-green-700">🎉 You&apos;ve completed this module.</div>}
      </div>

      <ol className="card mt-4 divide-y divide-slate-100">
        {m.lessons.map((l: any) => (
          <li key={l.id}>
            <Link href={`/learning/${m.id}/lessons/${l.id}`} className="flex items-center gap-3 p-4 hover:bg-slate-50" data-testid={`lesson-${l.id}`}>
              <span className={`grid h-7 w-7 shrink-0 place-items-center rounded-full text-xs font-bold ${done.get(l.id) ? "bg-green-100 text-green-700" : "bg-slate-100 text-slate-500"}`} aria-label={done.get(l.id) ? "Completed" : "Not completed"}>
                {done.get(l.id) ? "✓" : l.position}</span>
              <div className="min-w-0 flex-1"><div className="font-medium">{l.title}</div><div className="text-xs text-slate-500">{l.minutes} min{l.has_quiz && " · includes a quiz"}</div></div>
              {l.id === resume && <Badge tone="blue">{p.status === "not_started" ? "Start here" : "Resume"}</Badge>}
              <span className="text-slate-400">›</span>
            </Link>
          </li>))}
      </ol>
    </>
  );
}
