// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gap_map.h"

#include "statistc.h"

namespace tesseract {

BOOL_VAR(gapmap_debug, false, "Say which blocks have tables");
BOOL_VAR(gapmap_use_ends, false, "Use large space at start and end of rows");
BOOL_VAR(gapmap_no_isolated_quanta, false, "Ensure gaps not less than 2quanta wide");
double_VAR(gapmap_big_gaps, 1.75, "xht multiplier");

/*************************************************************************
 * A block gap map is a quantised histogram of whitespace regions in the
 * block. It is a vertical projection of wide gaps WITHIN lines
 *
 * The map is held as an array of counts of rows which have a wide gap
 * covering that region of the row. Each bucket in the map represents a width
 * of about half an xheight - (The median of the xhts in the rows is used.)
 *
 * The block is considered RECTANGULAR - delimited by the left and right
 * extremes of the rows in the block. However, ONLY wide gaps WITHIN a row are
 * counted.
 *
 *************************************************************************/

GAPMAP::GAPMAP(     // Constructor
    TO_BLOCK *block // block
) {
  TO_ROW *row;         // current row
  BLOBNBOX_IT blob_it; // iterator
  TBOX blob_box;
  TBOX prev_blob_box;
  int16_t gap_width;
  int16_t start_of_row;
  int16_t end_of_row;
  STATS xht_stats(0, 127);
  int16_t min_quantum;
  int16_t max_quantum;
  int16_t i;

  /*
  Find left and right extremes and bucket size
*/
  map = nullptr;
  min_left = INT16_MAX;
  max_right = -INT16_MAX;
  total_rows = 0;
  any_tabs = false;

  // row iterator
  TO_ROW_IT row_it(block->get_rows());
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row = row_it.data();
    if (!row->blob_list()->empty()) {
      total_rows++;
      xht_stats.add(static_cast<int16_t>(floor(row->xheight + 0.5)), 1);
      blob_it.set_to_list(row->blob_list());
      start_of_row = blob_it.data()->bounding_box().left();
      end_of_row = blob_it.data_relative(-1)->bounding_box().right();
      if (min_left > start_of_row) {
        min_left = start_of_row;
      }
      if (max_right < end_of_row) {
        max_right = end_of_row;
      }
    }
  }
  if ((total_rows < 3) || (min_left >= max_right)) {
    bucket_size = 0;
    map_max = 0;
    total_rows = 0;
    min_left = max_right = 0;
    return;
  }
  bucket_size = static_cast<int16_t>(floor(xht_stats.median() + 0.5)) / 2;
  map_max = (max_right - min_left) / bucket_size;
  map = new int16_t[map_max + 1];
  for (i = 0; i <= map_max; i++) {
    map[i] = 0;
  }

  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row = row_it.data();
    if (!row->blob_list()->empty()) {
      blob_it.set_to_list(row->blob_list());
      blob_it.mark_cycle_pt();
      blob_box = box_next(&blob_it);
      prev_blob_box = blob_box;
      if (gapmap_use_ends) {
        /* Leading space */
        gap_width = blob_box.left() - min_left;
        if ((gap_width > gapmap_big_gaps * row->xheight) && gap_width > 2) {
          max_quantum = (blob_box.left() - min_left) / bucket_size;
          if (max_quantum > map_max) {
            max_quantum = map_max;
          }
          for (i = 0; i <= max_quantum; i++) {
            map[i]++;
          }
        }
      }
      while (!blob_it.cycled_list()) {
        blob_box = box_next(&blob_it);
        gap_width = blob_box.left() - prev_blob_box.right();
        if ((gap_width > gapmap_big_gaps * row->xheight) && gap_width > 2) {
          min_quantum = (prev_blob_box.right() - min_left) / bucket_size;
          max_quantum = (blob_box.left() - min_left) / bucket_size;
          if (max_quantum > map_max) {
            max_quantum = map_max;
          }
          for (i = min_quantum; i <= max_quantum; i++) {
            map[i]++;
          }
        }
        prev_blob_box = blob_box;
      }
      if (gapmap_use_ends) {
        /* Trailing space */
        gap_width = max_right - prev_blob_box.right();
        if ((gap_width > gapmap_big_gaps * row->xheight) && gap_width > 2) {
          min_quantum = (prev_blob_box.right() - min_left) / bucket_size;
          if (min_quantum < 0) {
            min_quantum = 0;
          }
          for (i = min_quantum; i <= map_max; i++) {
            map[i]++;
          }
        }
      }
    }
  }
  for (i = 0; i <= map_max; i++) {
    if (map[i] > total_rows / 2) {
      if (gapmap_no_isolated_quanta &&
          (((i == 0) && (map[i + 1] <= total_rows / 2)) ||
           ((i == map_max) && (map[i - 1] <= total_rows / 2)) ||
           ((i > 0) && (i < map_max) && (map[i - 1] <= total_rows / 2) &&
            (map[i + 1] <= total_rows / 2)))) {
        map[i] = 0; // prevent isolated quantum
      } else {
        any_tabs = true;
      }
    }
  }
  if (gapmap_debug && any_tabs) {
    tprintf("Table found\n");
  }
}

/*************************************************************************
 * GAPMAP::table_gap()
 * Is there a bucket in the specified range where more than half the rows in the
 * block have a wide gap?
 *************************************************************************/

bool GAPMAP::table_gap( // Is gap a table?
    int16_t left,       // From here
    int16_t right       // To here
) {
  int16_t min_quantum;
  int16_t max_quantum;
  int16_t i;
  bool tab_found = false;

  if (!any_tabs) {
    return false;
  }

  min_quantum = (left - min_left) / bucket_size;
  max_quantum = (right - min_left) / bucket_size;
  // Clip to the bounds of the array. In some circumstances (big blob followed
  // by small blob) max_quantum can exceed the map_max bounds, but we clip
  // here instead, as it provides better long-term safety.
  if (min_quantum < 0) {
    min_quantum = 0;
  }
  if (max_quantum > map_max) {
    max_quantum = map_max;
  }
  for (i = min_quantum; (!tab_found && (i <= max_quantum)); i++) {
    if (map[i] > total_rows / 2) {
      tab_found = true;
    }
  }
  return tab_found;
}

} // namespace tesseract
