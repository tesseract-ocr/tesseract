"use client";
import Link from "next/link";
import { useParams } from "next/navigation";
import { useCallback, useEffect, useState } from "react";
import Markdown from "@/components/Markdown";
import Shell from "@/components/Shell";
import { Badge, Err, ProgressBar, Spinner } from "@/components/ui";
import { api } from "@/lib/api";

export default function LessonPage() {
  return <Shell><Body /></Shell>;
}

function Quiz({ quiz, onResult }: { quiz: any; onResult: () => void }) {
  const [sel, setSel] = useState<Record<number, number[]>>({});
  const [res, setRes] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);
  const pick = (q: any, i: number) => setSel(s => {
    const cur = s[q.id] || [];
    return { ...s, [q.id]: q.kind === "multiple" ? (cur.includes(i) ? cur.filter(x => x !== i) : [...cur, i]) : [i] };
  });
  const submit = async () => {
    setBusy(true); setErr(null);
    try {
      setRes(await api(`/api/web/learning/quizzes/${quiz.id}/submit`, { body: { answers: quiz.questions.filter((q: any) => sel[q.id]?.length).map((q: any) => ({ question_id: q.id, selected: sel[q.id] })) } }));
      onResult();
    } catch (x: any) { setErr(x.message); } finally { setBusy(false); }
  };
  const result = (qid: number) => res?.results.find((r: any) => r.question_id === qid);
  return (
    <section className="card mt-6 p-5" aria-label="Quiz" data-testid="quiz">
      <h2 className="text-lg font-semibold">{quiz.title}</h2>
      <p className="text-sm text-slate-500">Score {quiz.pass_percent}% or more to pass. Passing completes the lesson.</p>
      <Err msg={err} />
      <div className="mt-4 space-y-5">
        {quiz.questions.map((q: any, n: number) => {
          const r = result(q.id);
          return (
            <fieldset key={q.id} className={`rounded-lg border p-3 ${r ? (r.correct ? "border-green-300 bg-green-50" : "border-red-300 bg-red-50") : "border-slate-200"}`}>
              <legend className="px-1 text-sm font-medium">{n + 1}. {q.prompt} {q.kind === "multiple" && <span className="text-xs font-normal text-slate-500">(select all that apply)</span>}</legend>
              <div className="mt-1 space-y-1">
                {q.options.map((o: string, i: number) => (
                  <label key={i} className="flex cursor-pointer items-center gap-2 text-sm">
                    <input type={q.kind === "multiple" ? "checkbox" : "radio"} name={`q${q.id}`} checked={(sel[q.id] || []).includes(i)} onChange={() => pick(q, i)} disabled={busy} />{o}
                  </label>))}
              </div>
              {r && <div className="mt-2 text-sm"><b className={r.correct ? "text-green-700" : "text-red-700"}>{r.correct ? "Correct." : "Not quite."}</b> <span className="text-slate-600">{r.explanation}</span></div>}
            </fieldset>);
        })}
      </div>
      <div className="mt-4 flex flex-wrap items-center gap-3">
        <button className="btn-primary" onClick={submit} disabled={busy}>{res ? "Submit again" : "Submit answers"}</button>
        {res && (
          <div role="status" data-testid="quiz-result" className={`text-sm font-medium ${res.passed ? "text-green-700" : "text-red-700"}`}>
            Score {res.score}% ({res.correct}/{res.total}): {res.passed ? "passed 🎉" : `not passed yet (need ${res.pass_percent}%). Review the explanations and try again.`}
          </div>)}
      </div>
    </section>
  );
}

function Body() {
  const { id, lessonId } = useParams<{ id: string; lessonId: string }>();
  const [lesson, setLesson] = useState<any>(null);
  const [mod, setMod] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  const loadModule = useCallback(() => api(`/api/web/learning/modules/${id}`).then(setMod).catch(() => {}), [id]);
  const loadLesson = useCallback(() => api(`/api/web/learning/lessons/${lessonId}`).then(setLesson).catch(x => setErr(x.message)), [lessonId]);
  useEffect(() => { setLesson(null); setErr(null); loadLesson(); loadModule(); }, [loadLesson, loadModule]);

  const setDone = async (completed: boolean) => {
    setBusy(true); setErr(null);
    try { await api(`/api/web/learning/lessons/${lessonId}/complete?completed=${completed}`, { method: "POST" }); await Promise.all([loadLesson(), loadModule()]); }
    catch (x: any) { setErr(x.message); } finally { setBusy(false); }
  };

  if (err && !lesson) return <><Link href={`/learning/${id}`} className="text-sm text-slate-500 hover:text-brand-600">← Module</Link><div className="mt-3"><Err msg={err} /></div></>;
  if (!lesson) return <Spinner />;
  const p = mod?.progress;

  return (
    <>
      <div className="flex flex-wrap items-center gap-3 text-sm">
        <Link href="/learning" className="text-slate-500 hover:text-brand-600">Learning</Link><span className="text-slate-300">/</span>
        <Link href={`/learning/${id}`} className="text-slate-500 hover:text-brand-600">{lesson.module.title}</Link>
        {p && <div className="ml-auto w-40"><ProgressBar percent={p.percent} label="Module progress" /><div className="text-right text-xs text-slate-500">{p.percent}% of module</div></div>}
      </div>

      <article className="card mt-3 p-6">
        <div className="mb-2 flex items-center gap-2 text-xs text-slate-500">{lesson.minutes} min read{lesson.completed && <Badge tone="green">Completed</Badge>}</div>
        <h1 className="sr-only">{lesson.title}</h1>
        <Markdown source={lesson.content} />
      </article>

      {lesson.quiz && <Quiz quiz={lesson.quiz} onResult={() => { loadLesson(); loadModule(); }} />}
      <Err msg={err} />

      <div className="mt-5 flex flex-wrap items-center gap-3">
        {lesson.prev_lesson_id ? <Link className="btn-ghost" href={`/learning/${id}/lessons/${lesson.prev_lesson_id}`}>← Previous</Link> : <span />}
        <div className="ml-auto flex items-center gap-3">
          {lesson.completed
            ? <button className="btn-ghost" disabled={busy} onClick={() => setDone(false)}>Completed ✓ (mark as not done)</button>
            : <button className="btn-primary" disabled={busy} onClick={() => setDone(true)}>Mark as complete</button>}
          {lesson.next_lesson_id
            ? <Link className={lesson.completed ? "btn-primary" : "btn-ghost"} href={`/learning/${id}/lessons/${lesson.next_lesson_id}`}>Next lesson →</Link>
            : p?.status === "completed" ? <Link className="btn-primary" href="/learning">Finish: back to Learning</Link> : null}
        </div>
      </div>
    </>
  );
}
