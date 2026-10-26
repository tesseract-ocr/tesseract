"use client";
import Link from "next/link";
import { useParams } from "next/navigation";
import { useEffect, useState } from "react";
import Shell from "@/components/Shell";
import ResultView from "@/components/ResultView";
import { Err, Spinner } from "@/components/ui";
import { api } from "@/lib/api";

function Body() {
  const { id } = useParams<{ id: string }>();
  const [job, setJob] = useState<any>(null);
  const [err, setErr] = useState<string | null>(null);
  useEffect(() => { api(`/api/web/jobs/${id}`).then(setJob).catch(x => setErr(x.message)); }, [id]);
  return (
    <>
      <Link href="/history" className="mb-3 inline-block text-sm text-slate-500 hover:text-brand-600">← History</Link>
      <Err msg={err} />
      {!job && !err ? <Spinner /> : job && <ResultView job={job} />}
    </>
  );
}

export default function Page() {
  return <Shell><Body /></Shell>;
}
