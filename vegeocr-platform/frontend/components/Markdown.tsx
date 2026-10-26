import { Fragment, ReactNode } from "react";

/**
 * A small, safe markdown renderer for lesson content: # headings, paragraphs, - / 1. lists, tables, ``` code, **bold**, `code`,
 * [links](https://...). It builds React elements (never raw HTML), so content cannot inject markup.
 */
function inline(text: string): ReactNode[] {
  const out: ReactNode[] = [];
  const re = /(\*\*[^*]+\*\*|`[^`]+`|\[[^\]]+\]\(https?:\/\/[^)\s]+\))/g;
  let last = 0, m: RegExpExecArray | null, i = 0;
  while ((m = re.exec(text))) {
    if (m.index > last) out.push(text.slice(last, m.index));
    const t = m[0];
    if (t.startsWith("**")) out.push(<strong key={i++}>{t.slice(2, -2)}</strong>);
    else if (t.startsWith("`")) out.push(<code key={i++} className="rounded bg-slate-100 px-1 py-0.5 text-[0.85em]">{t.slice(1, -1)}</code>);
    else { const [, label, url] = /\[([^\]]+)\]\(([^)]+)\)/.exec(t)!; out.push(<a key={i++} href={url} target="_blank" rel="noopener noreferrer" className="text-brand-600 underline">{label}</a>); }
    last = m.index + t.length;
  }
  if (last < text.length) out.push(text.slice(last));
  return out;
}

export default function Markdown({ source }: { source: string }) {
  const lines = source.replace(/\r\n/g, "\n").split("\n");
  const blocks: ReactNode[] = [];
  let i = 0, k = 0;
  while (i < lines.length) {
    const line = lines[i];
    if (!line.trim()) { i++; continue; }
    if (line.startsWith("```")) {
      const code: string[] = []; i++;
      while (i < lines.length && !lines[i].startsWith("```")) code.push(lines[i++]);
      i++;
      blocks.push(<pre key={k++} className="my-3 overflow-x-auto rounded-lg bg-slate-900 p-3 text-xs text-slate-100">{code.join("\n")}</pre>);
    } else if (/^#{1,3}\s/.test(line)) {
      const level = /^#+/.exec(line)![0].length, text = line.replace(/^#+\s*/, "");
      const cls = ["", "mt-1 mb-3 text-2xl font-bold", "mt-6 mb-2 text-lg font-semibold", "mt-4 mb-1 font-semibold"][level];
      blocks.push(<div key={k++} role="heading" aria-level={level} className={cls}>{inline(text)}</div>);
      i++;
    } else if (/^\s*[-*]\s/.test(line) || /^\s*\d+\.\s/.test(line)) {
      const ordered = /^\s*\d+\./.test(line), items: string[] = [];
      while (i < lines.length && (ordered ? /^\s*\d+\.\s/.test(lines[i]) : /^\s*[-*]\s/.test(lines[i]))) items.push(lines[i++].replace(/^\s*([-*]|\d+\.)\s+/, ""));
      const Tag = ordered ? "ol" : "ul";
      blocks.push(<Tag key={k++} className={`my-2 ml-5 space-y-1 ${ordered ? "list-decimal" : "list-disc"}`}>{items.map((t, j) => <li key={j}>{inline(t)}</li>)}</Tag>);
    } else if (line.trim().startsWith("|")) {
      const rows: string[][] = [];
      while (i < lines.length && lines[i].trim().startsWith("|")) { rows.push(lines[i].trim().replace(/^\||\|$/g, "").split("|").map(c => c.trim())); i++; }
      const [head, , ...body] = rows;
      blocks.push(
        <div key={k++} className="my-3 overflow-x-auto"><table className="w-full border border-slate-200 text-sm">
          <thead className="bg-slate-50"><tr>{head.map((h, j) => <th key={j} className="border-b border-slate-200 px-3 py-2 text-left font-semibold">{inline(h)}</th>)}</tr></thead>
          <tbody>{body.map((r, j) => <tr key={j} className="border-b border-slate-100 last:border-0">{r.map((c, n) => <td key={n} className="px-3 py-2">{inline(c)}</td>)}</tr>)}</tbody>
        </table></div>);
    } else {
      const para: string[] = [];
      while (i < lines.length && lines[i].trim() && !/^(#{1,3}\s|```|\s*[-*]\s|\s*\d+\.\s|\|)/.test(lines[i])) para.push(lines[i++]);
      blocks.push(<p key={k++} className="my-2 leading-relaxed">{para.map((t, j) => <Fragment key={j}>{j > 0 && " "}{inline(t)}</Fragment>)}</p>);
    }
  }
  return <div className="text-[15px] text-slate-700">{blocks}</div>;
}
