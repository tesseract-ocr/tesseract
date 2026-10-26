"use client";
import { useMemo, useState } from "react";
import { api } from "@/lib/api";
import AuthImage from "./AuthImage";
import { Modal } from "./Modal";
import { Badge, Err, Ok } from "./ui";

type Line = { text: string; confidence: number; bbox: number[]; flagged: boolean };

const SHARP_TONE: Record<string, "green" | "amber" | "red"> = { sharp: "green", soft: "amber", blurry: "red" };

function QualityTable({ before, after }: { before: any; after: any | null }) {
  const rows: [string, (q: any) => string, boolean][] = [
    ["Edge sharpness", q => q.sharpness.toFixed(2), true], ["Noise (σ)", q => q.noise_sigma.toFixed(1), false],
    ["Contrast", q => String(Math.round(q.contrast)), true], ["Uneven lighting", q => q.uneven_lighting.toFixed(1), false], ["Text height (px)", q => q.text_height ? String(Math.round(q.text_height)) : "—", true],
  ];
  return (
    <table className="w-full text-sm">
      <thead><tr className="text-left text-xs uppercase text-slate-500"><th className="py-1 pr-3 font-semibold">Measure</th><th className="py-1 pr-3 font-semibold">Original</th>{after && <th className="py-1 font-semibold">Enhanced</th>}</tr></thead>
      <tbody>{rows.map(([label, f]) => <tr key={label} className="border-t border-slate-100"><td className="py-1 pr-3 text-slate-600">{label}</td><td className="py-1 pr-3 font-mono">{f(before)}</td>{after && <td className="py-1 font-mono">{f(after)}</td>}</tr>)}</tbody>
    </table>
  );
}

/** One finished extraction: warnings, quality report, side-by-side preview, editable lines with review flags, and the feedback loop into a dataset. */
export default function LabResult({ run, datasets, onDatasetsChanged }: { run: any; datasets: any[]; onDatasetsChanged: () => void }) {
  const result = run.result;
  const [pageNo, setPageNo] = useState(1);
  const [edits, setEdits] = useState<Record<string, string>>({});
  const [saved, setSaved] = useState<Record<string, boolean>>({});
  const [onlyFlagged, setOnlyFlagged] = useState(false);
  const [target, setTarget] = useState<string>("");
  const [newName, setNewName] = useState(""); const [newCat, setNewCat] = useState("handwriting");
  const [zoom, setZoom] = useState<"original" | "enhanced" | null>(null);
  const [msg, setMsg] = useState<string | null>(null); const [err, setErr] = useState<string | null>(null); const [busy, setBusy] = useState(false);
  const page = result.pages.find((p: any) => p.page === pageNo) || result.pages[0];
  const key = (i: number) => `${page.page}:${i}`;
  const edited = Object.keys(edits).filter(k => !saved[k]);
  const thr = result.low_confidence_threshold;
  const lines: Line[] = page.lines;
  const shown = useMemo(() => lines.map((l, i) => ({ l, i })).filter(({ l, i }) => !onlyFlagged || l.flagged || edits[key(i)] !== undefined), [lines, onlyFlagged, edits]); // eslint-disable-line react-hooks/exhaustive-deps
  const fullText = result.pages.map((p: any) => p.lines.map((l: Line, i: number) => edits[`${p.page}:${i}`] ?? l.text).join("\n")).join("\n\n");
  const imgPath = (which: string) => `/api/web/lab/runs/${run.id}/pages/${page.page}/${which}`;

  const download = () => {
    const a = document.createElement("a");
    a.href = URL.createObjectURL(new Blob([fullText], { type: "text/plain;charset=utf-8" }));
    a.download = `${(run.filename || "extraction").replace(/\.[^.]+$/, "")}.txt`; a.click();
  };

  const saveCorrections = async () => {
    setBusy(true); setErr(null); setMsg(null);
    try {
      let ds = target;
      if (ds === "new") {
        const d = await api("/api/web/lab/datasets", { body: { name: newName.trim(), category: newCat, lang: run.lang.split("+")[0] === "eng" || !["jpn", "chi_sim", "chi_tra"].includes(run.lang.split("+")[0]) ? "eng" : run.lang.split("+")[0] } });
        ds = String(d.id); setTarget(ds); onDatasetsChanged();
      }
      if (!ds) throw new Error("Choose a dataset to save to");
      let n = 0;
      for (const k of edited) {
        const [p, i] = k.split(":").map(Number);
        await api(`/api/web/lab/runs/${run.id}/corrections`, { body: { page: p, index: i, text: edits[k], dataset_id: Number(ds) } });
        setSaved(s => ({ ...s, [k]: true })); n++;
      }
      setMsg(`${n} correction${n === 1 ? "" : "s"} saved as training data. Open "My training data" to retrain.`); onDatasetsChanged();
    } catch (x: any) { setErr(x.message); } finally { setBusy(false); }
  };

  const before = page.quality_before, after = page.quality_after;
  return (
    <div className="card" data-testid="lab-result">
      <div className="flex flex-wrap items-center gap-2 border-b border-slate-200 p-4">
        <div className="min-w-0"><div className="truncate font-semibold">{run.filename}</div>
          <div className="mt-1 flex flex-wrap items-center gap-2 text-xs text-slate-500">
            <Badge tone="blue">{result.engine}</Badge><span>{result.pages.length} page(s)</span>
            {result.avg_confidence != null && <span>average confidence {Math.round(result.avg_confidence)}%</span>}
            {result.enhanced && <Badge tone="green">enhanced</Badge>}
            {result.used_dataset_model && <Badge tone="green">custom model</Badge>}</div></div>
        <div className="ml-auto flex gap-2"><button className="btn-ghost btn-sm" onClick={() => navigator.clipboard.writeText(fullText)}>Copy text</button><button className="btn-ghost btn-sm" onClick={download}>.txt</button></div>
      </div>
      <div className="space-y-5 p-4">
        {result.warnings.map((w: string, i: number) => <div key={i} role="alert" className="rounded-lg border border-amber-200 bg-amber-50 px-3 py-2 text-sm text-amber-900">⚠ {w}</div>)}
        {result.flagged_count > 0
          ? <div data-testid="review-banner" className="rounded-lg border border-red-200 bg-red-50 px-3 py-2 text-sm text-red-800"><b>{result.flagged_count} line{result.flagged_count === 1 ? "" : "s"} below {thr}% confidence</b> may be wrong. They are outlined in red below: please review them.</div>
          : lines.length > 0 && <div className="rounded-lg border border-green-200 bg-green-50 px-3 py-2 text-sm text-green-800">All lines are at or above {thr}% confidence. Still worth a quick skim: confidence is not a guarantee.</div>}

        {result.pages.length > 1 && <div className="flex gap-1" role="tablist" aria-label="Pages">{result.pages.map((p: any) => <button key={p.page} role="tab" aria-selected={p.page === page.page} onClick={() => setPageNo(p.page)} className={`rounded-md px-3 py-1 text-sm ${p.page === page.page ? "bg-brand-600 text-white" : "bg-slate-100 text-slate-600"}`}>Page {p.page}</button>)}</div>}

        <section aria-label="Image quality" className="grid gap-4 lg:grid-cols-3">
          <div className="lg:col-span-2">
            <div className="mb-2 flex flex-wrap items-center gap-2 text-sm"><b>{page.enhanced ? "Original vs enhanced" : "Image"}</b>
              <Badge tone={SHARP_TONE[before.blur_level]}>{before.blur_level}</Badge>
              {page.enhanced ? <span className="text-slate-500">Click an image to enlarge</span> : <span className="text-slate-500">No enhancement was needed or kept</span>}</div>
            <div className={`grid gap-3 ${page.enhanced ? "sm:grid-cols-2" : ""}`}>
              <figure><AuthImage path={imgPath("original")} alt={`Original, page ${page.page}`} className="max-h-80 w-full rounded-lg border border-slate-200 bg-white object-contain" onClick={() => setZoom("original")} /><figcaption className="mt-1 text-center text-xs font-semibold uppercase text-slate-500">Original</figcaption></figure>
              {page.enhanced && <figure><AuthImage path={imgPath("enhanced")} alt={`Enhanced, page ${page.page}`} className="max-h-80 w-full rounded-lg border border-slate-200 bg-white object-contain" onClick={() => setZoom("enhanced")} /><figcaption className="mt-1 text-center text-xs font-semibold uppercase text-brand-700">Enhanced</figcaption></figure>}
            </div>
          </div>
          <div className="space-y-3 text-sm">
            {before.reasons.length > 0 ? <div><div className="text-xs font-semibold uppercase text-slate-500">Problems detected</div><ul className="mt-1 list-disc pl-5 text-slate-700">{before.reasons.map((r: string) => <li key={r}>{r}</li>)}</ul></div>
              : <div className="text-slate-600">No quality problems detected.</div>}
            {page.enhance_steps.length > 0 && <div><div className="text-xs font-semibold uppercase text-slate-500">Applied</div><div className="mt-1 flex flex-wrap gap-1">{page.enhance_steps.map((s: string) => <Badge key={s} tone="green">{s}</Badge>)}</div></div>}
            {page.enhance_check && <p className="rounded bg-slate-50 p-2 text-xs text-slate-600">Both versions were read. Confidence: original {page.enhance_check.original_confidence}%, enhanced {page.enhance_check.enhanced_confidence}%. Kept the <b>{page.enhance_check.kept}</b>.</p>}
            <QualityTable before={before} after={after} />
            <p className="text-xs text-slate-500">Sharpness is measured in pixels, so an upscaled copy can score lower even though it reads better.</p>
          </div>
        </section>

        <section aria-label="Extracted text">
          <div className="mb-2 flex flex-wrap items-center gap-3"><h3 className="font-semibold">Extracted text</h3>
            <label className="flex items-center gap-1.5 text-sm text-slate-600"><input type="checkbox" checked={onlyFlagged} onChange={e => setOnlyFlagged(e.target.checked)} /> Only lines needing review</label></div>
          {lines.length === 0 ? <div className="rounded-lg border border-dashed p-6 text-center text-sm text-slate-500">No text was found on this page.</div> : (
            <div className="space-y-1.5">
              {shown.length === 0 && <div className="text-sm text-slate-500">Nothing needs review on this page.</div>}
              {shown.map(({ l, i }) => {
                const k = key(i), changed = edits[k] !== undefined;
                return (
                  <div key={i} className="flex items-center gap-2" data-testid={`line-${i}`}>
                    <input aria-label={`Line ${i + 1}`} className={`input ${l.flagged && !changed ? "border-red-400 bg-red-50" : ""} ${changed ? "border-amber-400 bg-amber-50" : ""}`} value={edits[k] ?? l.text}
                      onChange={e => { setSaved(s => ({ ...s, [k]: false })); setEdits(s => e.target.value === l.text ? Object.fromEntries(Object.entries(s).filter(([x]) => x !== k)) : { ...s, [k]: e.target.value }); }} />
                    <span title="Confidence" className={`w-24 shrink-0 text-right text-xs ${l.flagged ? "font-semibold text-red-600" : "text-slate-400"}`}>{saved[k] ? "✓ saved" : l.flagged ? `review · ${Math.round(l.confidence)}%` : `${Math.round(l.confidence)}%`}</span>
                  </div>);
              })}
            </div>)}
        </section>

        {lines.length > 0 && (
          <section aria-label="Save corrections" className="rounded-lg border border-slate-200 bg-slate-50 p-4">
            <h3 className="font-semibold">Teach the model: save your corrections</h3>
            <p className="mt-1 text-sm text-slate-600">Each line you fix is saved with its image and the right text as a training sample in one of your datasets. Train on them under <i>My training data</i>.</p>
            <Err msg={err} /><Ok msg={msg} />
            <div className="mt-3 flex flex-wrap items-end gap-3">
              <label className="text-sm">Dataset<select className="input mt-1 w-56" value={target} onChange={e => setTarget(e.target.value)} aria-label="Dataset to save corrections to">
                <option value="">Choose…</option>{datasets.map(d => <option key={d.id} value={d.id}>{d.name} ({d.category})</option>)}<option value="new">+ New dataset…</option></select></label>
              {target === "new" && <><label className="text-sm">Name<input className="input mt-1 w-44" value={newName} onChange={e => setNewName(e.target.value)} placeholder="e.g. My handwriting" aria-label="New dataset name" /></label>
                <label className="text-sm">Category<input className="input mt-1 w-36" list="cats" value={newCat} onChange={e => setNewCat(e.target.value)} /><datalist id="cats">{["handwriting", "printed", "blurry", "receipts", "forms"].map(c => <option key={c} value={c} />)}</datalist></label></>}
              <button className="btn-primary" disabled={busy || edited.length === 0 || !target || (target === "new" && !newName.trim())} onClick={saveCorrections}>{busy ? "Saving…" : `Save ${edited.length} correction${edited.length === 1 ? "" : "s"}`}</button>
            </div>
          </section>)}
      </div>
      {zoom && <Modal title={zoom === "original" ? "Original" : "Enhanced"} onClose={() => setZoom(null)} wide><AuthImage path={imgPath(zoom)} alt={zoom} className="mx-auto max-h-[75vh] w-full object-contain" /></Modal>}
    </div>
  );
}
