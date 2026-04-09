/**********************************************************************
 * File:        underlin.cpp  (Formerly undrline.c)
 * Description: Code to chop blobs apart from underlines.
 * Author:      Ray Smith
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "underlin.h"

namespace tesseract {

double_VAR(textord_underline_offset, 0.1, "Fraction of x to ignore");
BOOL_VAR(textord_restore_underlines, true, "Chop underlines & put back");

/**********************************************************************
 * restore_underlined_blobs
 *
 * Find underlined blobs and put them back in the row.
 **********************************************************************/

void restore_underlined_blobs( // get chop points
    TO_BLOCK *block            // block to do
) {
  int16_t chop_coord;        // chop boundary
  TBOX blob_box;             // of underline
  BLOBNBOX *u_line;          // underline bit
  TO_ROW *row;               // best row for blob
  ICOORDELT_LIST chop_cells; // blobs to cut out
                             // real underlines
  BLOBNBOX_LIST residual_underlines;
  C_OUTLINE_LIST left_coutlines;
  C_OUTLINE_LIST right_coutlines;
  ICOORDELT_IT cell_it = &chop_cells;
  // under lines
  BLOBNBOX_IT under_it = &block->underlines;
  BLOBNBOX_IT ru_it = &residual_underlines;

  if (block->get_rows()->empty()) {
    return; // Don't crash if there are no rows.
  }
  for (under_it.mark_cycle_pt(); !under_it.cycled_list(); under_it.forward()) {
    u_line = under_it.extract();
    blob_box = u_line->bounding_box();
    row = most_overlapping_row(block->get_rows(), u_line);
    if (row == nullptr) {
      return; // Don't crash if there is no row.
    }
    find_underlined_blobs(u_line, &row->baseline, row->xheight,
                          row->xheight * textord_underline_offset, &chop_cells);
    cell_it.set_to_list(&chop_cells);
    for (cell_it.mark_cycle_pt(); !cell_it.cycled_list(); cell_it.forward()) {
      chop_coord = cell_it.data()->x();
      if (cell_it.data()->y() - chop_coord > textord_fp_chop_error + 1) {
        split_to_blob(u_line, chop_coord, textord_fp_chop_error + 0.5, &left_coutlines,
                      &right_coutlines);
        if (!left_coutlines.empty()) {
          ru_it.add_after_then_move(new BLOBNBOX(new C_BLOB(&left_coutlines)));
        }
        chop_coord = cell_it.data()->y();
        split_to_blob(nullptr, chop_coord, textord_fp_chop_error + 0.5, &left_coutlines,
                      &right_coutlines);
        if (!left_coutlines.empty()) {
          row->insert_blob(new BLOBNBOX(new C_BLOB(&left_coutlines)));
        }
        u_line = nullptr; // no more blobs to add
      }
      delete cell_it.extract();
    }
    if (!right_coutlines.empty()) {
      split_to_blob(nullptr, blob_box.right(), textord_fp_chop_error + 0.5, &left_coutlines,
                    &right_coutlines);
      if (!left_coutlines.empty()) {
        ru_it.add_after_then_move(new BLOBNBOX(new C_BLOB(&left_coutlines)));
      }
    }
    delete u_line;
  }
  if (!ru_it.empty()) {
    ru_it.move_to_first();
    for (ru_it.mark_cycle_pt(); !ru_it.cycled_list(); ru_it.forward()) {
      under_it.add_after_then_move(ru_it.extract());
    }
  }
}

/**********************************************************************
 * most_overlapping_row
 *
 * Return the row which most overlaps the blob.
 **********************************************************************/

TO_ROW *most_overlapping_row( // find best row
    TO_ROW_LIST *rows,        // list of rows
    BLOBNBOX *blob            // blob to place
) {
  int16_t x = (blob->bounding_box().left() + blob->bounding_box().right()) / 2;
  TO_ROW_IT row_it = rows; // row iterator
  TO_ROW *row;             // current row
  TO_ROW *best_row;        // output row
  float overlap;           // of blob & row
  float bestover;          // best overlap

  best_row = nullptr;
  bestover = static_cast<float>(-INT32_MAX);
  if (row_it.empty()) {
    return nullptr;
  }
  row = row_it.data();
  row_it.mark_cycle_pt();
  while (row->baseline.y(x) + row->descdrop > blob->bounding_box().top() && !row_it.cycled_list()) {
    best_row = row;
    bestover = blob->bounding_box().top() - row->baseline.y(x) + row->descdrop;
    row_it.forward();
    row = row_it.data();
  }
  while (row->baseline.y(x) + row->xheight + row->ascrise >= blob->bounding_box().bottom() &&
         !row_it.cycled_list()) {
    overlap = row->baseline.y(x) + row->xheight + row->ascrise;
    if (blob->bounding_box().top() < overlap) {
      overlap = blob->bounding_box().top();
    }
    if (blob->bounding_box().bottom() > row->baseline.y(x) + row->descdrop) {
      overlap -= blob->bounding_box().bottom();
    } else {
      overlap -= row->baseline.y(x) + row->descdrop;
    }
    if (overlap > bestover) {
      bestover = overlap;
      best_row = row;
    }
    row_it.forward();
    row = row_it.data();
  }
  if (bestover < 0 &&
      row->baseline.y(x) + row->xheight + row->ascrise - blob->bounding_box().bottom() > bestover) {
    best_row = row;
  }
  return best_row;
}

/**********************************************************************
 * find_underlined_blobs
 *
 * Find the start and end coords of blobs in the underline.
 **********************************************************************/

void find_underlined_blobs(    // get chop points
    BLOBNBOX *u_line,          // underlined unit
    QSPLINE *baseline,         // actual baseline
    float xheight,             // height of line
    float baseline_offset,     // amount to shrinke it
    ICOORDELT_LIST *chop_cells // places to chop
) {
  ICOORD blob_chop; // sides of blob
  TBOX blob_box = u_line->bounding_box();
  // cell iterator
  ICOORDELT_IT cell_it = chop_cells;
  STATS upper_proj(blob_box.left(), blob_box.right());
  STATS middle_proj(blob_box.left(), blob_box.right());
  STATS lower_proj(blob_box.left(), blob_box.right());
  C_OUTLINE_IT out_it; // outlines of blob

  ASSERT_HOST(u_line->cblob() != nullptr);

  out_it.set_to_list(u_line->cblob()->out_list());
  for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
    vertical_cunderline_projection(out_it.data(), baseline, xheight, baseline_offset, &lower_proj,
                                   &middle_proj, &upper_proj);
  }

  for (auto x = blob_box.left(); x < blob_box.right(); x++) {
    if (middle_proj.pile_count(x) > 0) {
      auto y = x + 1;
      for (; y < blob_box.right() && middle_proj.pile_count(y) > 0; y++) {
        ;
      }
      blob_chop = ICOORD(x, y);
      cell_it.add_after_then_move(new ICOORDELT(blob_chop));
      x = y;
    }
  }
}

/**********************************************************************
 * vertical_cunderline_projection
 *
 * Compute the vertical projection of an outline from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_cunderline_projection( // project outlines
    C_OUTLINE *outline,              // outline to project
    QSPLINE *baseline,               // actual baseline
    float xheight,                   // height of line
    float baseline_offset,           // amount to shrinke it
    STATS *lower_proj,               // below baseline
    STATS *middle_proj,              // centre region
    STATS *upper_proj                // top region
) {
  ICOORD pos;               // current point
  ICOORD step;              // edge step
  int16_t lower_y, upper_y; // region limits
  C_OUTLINE_IT out_it = outline->child();

  pos = outline->start_pos();
  int16_t length = outline->pathlength();
  for (int16_t stepindex = 0; stepindex < length; stepindex++) {
    step = outline->step(stepindex);
    if (step.x() > 0) {
      lower_y = static_cast<int16_t>(floor(baseline->y(pos.x()) + baseline_offset + 0.5));
      upper_y = static_cast<int16_t>(floor(baseline->y(pos.x()) + baseline_offset + xheight + 0.5));
      if (pos.y() >= lower_y) {
        lower_proj->add(pos.x(), -lower_y);
        if (pos.y() >= upper_y) {
          middle_proj->add(pos.x(), lower_y - upper_y);
          upper_proj->add(pos.x(), upper_y - pos.y());
        } else {
          middle_proj->add(pos.x(), lower_y - pos.y());
        }
      } else {
        lower_proj->add(pos.x(), -pos.y());
      }
    } else if (step.x() < 0) {
      lower_y = static_cast<int16_t>(floor(baseline->y(pos.x() - 1) + baseline_offset + 0.5));
      upper_y =
          static_cast<int16_t>(floor(baseline->y(pos.x() - 1) + baseline_offset + xheight + 0.5));
      if (pos.y() >= lower_y) {
        lower_proj->add(pos.x() - 1, lower_y);
        if (pos.y() >= upper_y) {
          middle_proj->add(pos.x() - 1, upper_y - lower_y);
          upper_proj->add(pos.x() - 1, pos.y() - upper_y);
        } else {
          middle_proj->add(pos.x() - 1, pos.y() - lower_y);
        }
      } else {
        lower_proj->add(pos.x() - 1, pos.y());
      }
    }
    pos += step;
  }

  for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
    vertical_cunderline_projection(out_it.data(), baseline, xheight, baseline_offset, lower_proj,
                                   middle_proj, upper_proj);
  }
}

} // namespace tesseract
