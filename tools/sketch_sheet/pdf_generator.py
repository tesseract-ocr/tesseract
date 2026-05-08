from __future__ import annotations

import io
from dataclasses import dataclass
from datetime import date
from typing import Optional

from reportlab.lib.colors import black, Color
from reportlab.lib.pagesizes import A4, LETTER, landscape, portrait
from reportlab.pdfgen import canvas


PAGE_SIZES = {
    "letter": LETTER,
    "a4": A4,
}


@dataclass(frozen=True)
class SheetLayout:
    page_margin_pt: float = 28.8  # 0.4"
    outer_border_pt: float = 1.0
    grid_line_pt: float = 0.5
    grid_line_gray: float = 0.25  # 25% black
    title_block_height_pt: float = 86.4  # 1.2"
    label_col_width_pt: float = 50.4  # 0.7"
    label_font_size_pt: float = 10.0
    value_font_size_pt: float = 10.0
    cell_number_font_size_pt: float = 8.0
    cell_number_gray: float = 0.4
    cell_number_inset_pt: float = 6.0
    field_padding_pt: float = 6.0


def _page_size(size: str, orientation: str) -> tuple[float, float]:
    normalized_size = size.strip().lower()
    normalized_orientation = orientation.strip().lower()
    if normalized_size not in PAGE_SIZES:
        raise ValueError(f"Unsupported page size: {size}")
    if normalized_orientation not in {"portrait", "landscape"}:
        raise ValueError(f"Unsupported orientation: {orientation}")

    base = PAGE_SIZES[normalized_size]
    return landscape(base) if normalized_orientation == "landscape" else portrait(base)


def _sheet_id() -> str:
    # Keep IDs short but sortable enough for prototypes.
    return date.today().strftime("%Y%m%d")


def generate_sheet(
    rows: int,
    cols: int,
    orientation: str = "landscape",
    page_size: str = "letter",
    title: Optional[str] = None,
    ref: Optional[str] = None,
    sheet_date: Optional[str] = None,
    show_cell_numbers: bool = True,
) -> bytes:
    if rows < 1 or cols < 1:
        raise ValueError("rows and cols must both be >= 1")

    w_pt, h_pt = _page_size(page_size, orientation)
    layout = SheetLayout()
    out = io.BytesIO()
    c = canvas.Canvas(out, pagesize=(w_pt, h_pt))

    c.setTitle("Sketch Sheet")
    c.setAuthor("Vizcom Prototype")
    c.setSubject("Sketch Sheet prototype PDF")
    c.setProducer("sketch_sheet_v0_1")
    c.setKeywords(f"grid:{rows}x{cols};template_version:v0.1;sheet_id:{_sheet_id()}")

    left = layout.page_margin_pt
    right = w_pt - layout.page_margin_pt
    bottom = layout.page_margin_pt
    top = h_pt - layout.page_margin_pt

    # Outer border (registration boundary)
    c.setStrokeColor(black)
    c.setLineWidth(layout.outer_border_pt)
    c.rect(left, bottom, right - left, top - bottom, stroke=1, fill=0)

    content_left = left
    content_right = right
    content_bottom = bottom
    content_top = top

    title_h = layout.title_block_height_pt
    title_top = content_bottom + title_h
    grid_bottom = title_top
    grid_top = content_top
    grid_h = grid_top - grid_bottom
    grid_w = content_right - content_left

    if grid_h <= 0:
        raise ValueError("Title block + margins leave no space for grid")

    cell_w = grid_w / cols
    cell_h = grid_h / rows

    # Grid boundary
    c.setLineWidth(layout.outer_border_pt)
    c.setStrokeColor(black)
    c.rect(content_left, grid_bottom, grid_w, grid_h, stroke=1, fill=0)

    # Internal grid lines
    c.setLineWidth(layout.grid_line_pt)
    c.setStrokeColor(Color(layout.grid_line_gray, layout.grid_line_gray, layout.grid_line_gray))
    for i in range(1, cols):
        x = content_left + i * cell_w
        c.line(x, grid_bottom, x, grid_top)
    for j in range(1, rows):
        y = grid_bottom + j * cell_h
        c.line(content_left, y, content_right, y)

    # Cell numbers
    if show_cell_numbers:
        c.setFont("Helvetica", layout.cell_number_font_size_pt)
        c.setFillColor(
            Color(layout.cell_number_gray, layout.cell_number_gray, layout.cell_number_gray)
        )
        n = 1
        for r in range(rows):
            y_top = grid_top - r * cell_h
            for cl in range(cols):
                x_left = content_left + cl * cell_w
                c.drawString(
                    x_left + layout.cell_number_inset_pt,
                    y_top - layout.cell_number_inset_pt - layout.cell_number_font_size_pt,
                    str(n),
                )
                n += 1
        c.setFillColor(black)

    # Title block outer border.
    c.setStrokeColor(black)
    c.setLineWidth(layout.outer_border_pt)
    c.rect(content_left, content_bottom, content_right - content_left, title_h, stroke=1, fill=0)

    # Split title block into two rows.
    row_mid = content_bottom + title_h / 2.0
    c.line(content_left, row_mid, content_right, row_mid)

    # Top row: Title label + value
    title_label_right = content_left + layout.label_col_width_pt
    c.line(title_label_right, row_mid, title_label_right, content_bottom + title_h)

    # Bottom row: Ref label+value, Date label+value
    ref_label_right = content_left + layout.label_col_width_pt
    date_label_left = content_left + (content_right - content_left) * 0.7
    date_label_right = date_label_left + layout.label_col_width_pt
    c.line(ref_label_right, content_bottom, ref_label_right, row_mid)
    c.line(date_label_left, content_bottom, date_label_left, row_mid)
    c.line(date_label_right, content_bottom, date_label_right, row_mid)

    # Labels
    c.setFont("Helvetica-Bold", layout.label_font_size_pt)
    c.drawString(content_left + layout.field_padding_pt, row_mid + layout.field_padding_pt, "Title")
    c.drawString(content_left + layout.field_padding_pt, content_bottom + layout.field_padding_pt, "Ref.")
    c.drawString(date_label_left + layout.field_padding_pt, content_bottom + layout.field_padding_pt, "Date")

    # Values / placeholders
    c.setFont("Helvetica", layout.value_font_size_pt)
    title_text = title or ""
    ref_text = ref if ref else "@"
    date_text = sheet_date or ""

    c.drawString(title_label_right + layout.field_padding_pt, row_mid + layout.field_padding_pt, title_text)
    c.drawString(ref_label_right + layout.field_padding_pt, content_bottom + layout.field_padding_pt, ref_text)
    c.drawString(
        date_label_right + layout.field_padding_pt,
        content_bottom + layout.field_padding_pt,
        date_text,
    )

    c.showPage()
    c.save()
    return out.getvalue()

