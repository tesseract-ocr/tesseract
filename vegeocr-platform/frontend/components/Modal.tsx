"use client";
import { ReactNode, useEffect, useRef } from "react";

/** Accessible modal: role=dialog, closes on Esc / backdrop click, returns focus on close. */
export function Modal({ title, onClose, children, wide = false }: { title: string; onClose: () => void; children: ReactNode; wide?: boolean }) {
  const ref = useRef<HTMLDivElement>(null);
  useEffect(() => {
    const previous = document.activeElement as HTMLElement | null;
    ref.current?.focus();
    const onKey = (e: KeyboardEvent) => e.key === "Escape" && onClose();
    window.addEventListener("keydown", onKey);
    return () => { window.removeEventListener("keydown", onKey); previous?.focus?.(); };
  }, [onClose]);
  return (
    <div className="fixed inset-0 z-50 grid place-items-center bg-slate-900/40 p-4" onMouseDown={e => e.target === e.currentTarget && onClose()}>
      <div ref={ref} tabIndex={-1} role="dialog" aria-modal="true" aria-label={title}
        className={`max-h-[90vh] w-full overflow-y-auto rounded-xl bg-white p-5 shadow-xl outline-none ${wide ? "max-w-3xl" : "max-w-md"}`}>
        <div className="mb-3 flex items-start justify-between gap-4">
          <h2 className="text-lg font-semibold">{title}</h2>
          <button onClick={onClose} aria-label="Close" className="rounded px-2 text-xl leading-none text-slate-400 hover:text-slate-700">×</button>
        </div>
        {children}
      </div>
    </div>
  );
}

/** Confirmation for destructive actions. The confirm button is deliberately NOT focused by default. */
export function ConfirmDialog({ title, message, confirmLabel, danger = true, busy = false, onConfirm, onCancel }:
  { title: string; message: ReactNode; confirmLabel: string; danger?: boolean; busy?: boolean; onConfirm: () => void; onCancel: () => void }) {
  return (
    <Modal title={title} onClose={onCancel}>
      <div className="text-sm text-slate-600">{message}</div>
      <div className="mt-5 flex justify-end gap-2">
        <button className="btn-ghost" onClick={onCancel} disabled={busy}>Cancel</button>
        <button className={danger ? "btn bg-red-600 text-white hover:bg-red-700" : "btn-primary"} onClick={onConfirm} disabled={busy}>{busy ? "Working…" : confirmLabel}</button>
      </div>
    </Modal>
  );
}
