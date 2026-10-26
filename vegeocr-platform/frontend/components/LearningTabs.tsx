"use client";
import Link from "next/link";
import { usePathname } from "next/navigation";

const TABS = [["/learning", "Courses"], ["/learning/lab", "Extraction lab"], ["/learning/datasets", "My training data"]] as const;

/** The three areas of the Learning section: lessons, the handwriting/enhancement lab, and your own training data. */
export default function LearningTabs() {
  const path = usePathname();
  const active = (href: string) => (href === "/learning" ? path === "/learning" : path.startsWith(href));
  return (
    <nav aria-label="Learning sections" className="mb-5 flex gap-1 overflow-x-auto border-b border-slate-200">
      {TABS.map(([href, label]) => (
        <Link key={href} href={href} aria-current={active(href) ? "page" : undefined}
          className={`-mb-px whitespace-nowrap border-b-2 px-4 py-2 text-sm font-medium ${active(href) ? "border-brand-600 text-brand-700" : "border-transparent text-slate-500 hover:text-slate-800"}`}>{label}</Link>
      ))}
    </nav>
  );
}
