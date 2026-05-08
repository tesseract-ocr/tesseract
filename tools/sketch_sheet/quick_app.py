from __future__ import annotations

import subprocess
import sys
from datetime import date
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk


REPO_ROOT = Path(__file__).resolve().parents[2]
PYTHON_BIN = REPO_ROOT / ".venv" / "bin" / "python3"


def _run_cmd(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )


class SketchSheetApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Sketch Sheet")
        self.geometry("520x300")
        self.resizable(False, False)

        frame = ttk.Frame(self, padding=16)
        frame.pack(fill=tk.BOTH, expand=True)

        ttk.Label(frame, text="Sketch Sheet Quick App", font=("Helvetica", 15, "bold")).pack(
            anchor=tk.W
        )
        ttk.Label(frame, text=f"Repo: {REPO_ROOT}").pack(anchor=tk.W, pady=(2, 12))

        config = ttk.LabelFrame(frame, text="Sheet Settings", padding=10)
        config.pack(fill=tk.X)

        ttk.Label(config, text="Rows").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(config, text="Cols").grid(row=0, column=2, sticky=tk.W, padx=(16, 0))

        self.rows_var = tk.StringVar(value="3")
        self.cols_var = tk.StringVar(value="3")
        ttk.Entry(config, textvariable=self.rows_var, width=8).grid(row=0, column=1, sticky=tk.W)
        ttk.Entry(config, textvariable=self.cols_var, width=8).grid(row=0, column=3, sticky=tk.W)

        actions = ttk.Frame(frame, padding=(0, 12, 0, 0))
        actions.pack(fill=tk.X)
        ttk.Button(actions, text="Generate Sheet PDF", command=self.generate_pdf).pack(fill=tk.X)
        ttk.Button(actions, text="Ingest Image/PDF...", command=self.ingest_file).pack(
            fill=tk.X, pady=(8, 0)
        )

        self.status_var = tk.StringVar(value="Ready")
        ttk.Label(frame, textvariable=self.status_var).pack(anchor=tk.W, pady=(12, 0))

    def _rows_cols(self) -> tuple[int, int]:
        try:
            rows = int(self.rows_var.get().strip())
            cols = int(self.cols_var.get().strip())
            if rows < 1 or cols < 1:
                raise ValueError
            return rows, cols
        except ValueError:
            raise ValueError("Rows/Cols must be positive integers")

    def _check_env(self) -> bool:
        if not PYTHON_BIN.exists():
            messagebox.showerror(
                "Missing virtualenv",
                "Could not find .venv python.\nRun this once in Terminal:\n\n"
                "python3 -m venv .venv && source .venv/bin/activate && "
                "pip install -r tools/sketch_sheet/requirements.txt",
            )
            return False
        return True

    def generate_pdf(self) -> None:
        if not self._check_env():
            return
        try:
            rows, cols = self._rows_cols()
        except ValueError as err:
            messagebox.showerror("Invalid settings", str(err))
            return

        output = REPO_ROOT / "tmp" / f"sketch_sheet_{date.today().isoformat()}_{rows}x{cols}.pdf"
        output.parent.mkdir(parents=True, exist_ok=True)

        self.status_var.set("Generating PDF...")
        self.update_idletasks()
        cmd = [
            str(PYTHON_BIN),
            "-m",
            "tools.sketch_sheet.cli",
            "generate",
            "--rows",
            str(rows),
            "--cols",
            str(cols),
            "--page-size",
            "letter",
            "--orientation",
            "landscape",
            "--auto-date",
            "--output",
            str(output),
        ]
        result = _run_cmd(cmd)
        if result.returncode != 0:
            messagebox.showerror("Generate failed", result.stderr or result.stdout or "Unknown error")
            self.status_var.set("Generate failed")
            return

        messagebox.showinfo("Generated", f"PDF created:\n{output}")
        self.status_var.set(f"Generated: {output.name}")

    def ingest_file(self) -> None:
        if not self._check_env():
            return
        try:
            rows, cols = self._rows_cols()
        except ValueError as err:
            messagebox.showerror("Invalid settings", str(err))
            return

        source = filedialog.askopenfilename(
            title="Select sheet image or PDF",
            filetypes=[
                ("Image/PDF files", "*.png *.jpg *.jpeg *.pdf"),
                ("All files", "*.*"),
            ],
            initialdir=str(REPO_ROOT),
        )
        if not source:
            return

        self.status_var.set("Running ingest...")
        self.update_idletasks()
        cmd = [
            str(PYTHON_BIN),
            "-m",
            "tools.sketch_sheet.cli",
            "ingest",
            source,
            "--rows",
            str(rows),
            "--cols",
            str(cols),
            "--output-root",
            str(REPO_ROOT / "tmp"),
        ]
        result = _run_cmd(cmd)
        if result.returncode != 0:
            messagebox.showerror("Ingest failed", result.stderr or result.stdout or "Unknown error")
            self.status_var.set("Ingest failed")
            return

        line = (result.stdout or "").strip().splitlines()
        out_text = line[-1] if line else "Ingest finished."
        messagebox.showinfo("Ingest complete", out_text)
        self.status_var.set("Ingest complete")


def main() -> int:
    app = SketchSheetApp()
    app.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

