from __future__ import annotations

import argparse
from datetime import date
from pathlib import Path

from .ingest import ingest_sheet
from .pdf_generator import generate_sheet


def _build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Sketch Sheet PDF + ingest prototype")
    sub = p.add_subparsers(dest="cmd", required=True)

    gen = sub.add_parser("generate", help="Generate a sketch sheet PDF")
    gen.add_argument("--rows", type=int, default=3)
    gen.add_argument("--cols", type=int, default=3)
    gen.add_argument("--page-size", choices=["letter", "a4"], default="letter")
    gen.add_argument("--orientation", choices=["portrait", "landscape"], default="landscape")
    gen.add_argument("--title", default="")
    gen.add_argument("--ref", default="")
    gen.add_argument("--date", dest="sheet_date", default="")
    gen.add_argument("--auto-date", action="store_true", help="Fill date with today's date")
    gen.add_argument("--no-cell-numbers", action="store_true")
    gen.add_argument("--output", default="", help="Output PDF path")

    ing = sub.add_parser("ingest", help="Ingest a sketched sheet image/PDF")
    ing.add_argument("input_path", help="Path to image or PDF")
    ing.add_argument("--rows", type=int, default=3)
    ing.add_argument("--cols", type=int, default=3)
    ing.add_argument("--output-root", default=".")
    ing.add_argument("--no-debug", action="store_true")

    return p


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    if args.cmd == "generate":
        sheet_date = args.sheet_date
        if args.auto_date and not sheet_date:
            sheet_date = date.today().isoformat()

        pdf_bytes = generate_sheet(
            rows=args.rows,
            cols=args.cols,
            orientation=args.orientation,
            page_size=args.page_size,
            title=args.title or None,
            ref=args.ref or None,
            sheet_date=sheet_date or None,
            show_cell_numbers=not args.no_cell_numbers,
        )

        out = Path(args.output) if args.output else Path(
            f"sketch_sheet_{date.today().isoformat()}_{args.rows}x{args.cols}.pdf"
        )
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(pdf_bytes)
        print(f"Wrote {out}")
        return 0

    if args.cmd == "ingest":
        out_dir = ingest_sheet(
            image_path=args.input_path,
            rows=args.rows,
            cols=args.cols,
            output_root=args.output_root,
            write_debug=not args.no_debug,
        )
        print(f"Wrote ingest output to {out_dir}")
        return 0

    parser.error("Unknown command")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())

