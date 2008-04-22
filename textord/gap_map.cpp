#include "mfcpch.h"
#include          "statistc.h"
#include          "gap_map.h"

#define EXTERN
EXTERN BOOL_VAR (gapmap_debug, FALSE, "Say which blocks have tables");
EXTERN BOOL_VAR (gapmap_use_ends, FALSE,
"Use large space at start and end of rows");
EXTERN BOOL_VAR (gapmap_no_isolated_quanta, FALSE,
"Ensure gaps not less than 2quanta wide");
EXTERN double_VAR (gapmap_big_gaps, 1.75, "xht multiplier");

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

GAPMAP::GAPMAP(                 //Constructor
               TO_BLOCK *block  //block
              ) {
  TO_ROW_IT row_it;              //row iterator
  TO_ROW *row;                   //current row
  BLOBNBOX_IT blob_it;           //iterator
  TBOX blob_box;
  TBOX prev_blob_box;
  inT16 gap_width;
  inT16 start_of_row;
  inT16 end_of_row;
  STATS xht_stats (0, 128);
  inT16 min_quantum;
  inT16 max_quantum;
  inT16 i;

  row_it.set_to_list (block->get_rows ());
  /*
    Find left and right extremes and bucket size
  */
  map = NULL;
  min_left = MAX_INT16;
  max_right = -MAX_INT16;
  total_rows = 0;
  any_tabs = FALSE;
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    if (!row->blob_list ()->empty ()) {
      total_rows++;
      xht_stats.add ((inT16) floor (row->xheight + 0.5), 1);
      blob_it.set_to_list (row->blob_list ());
      start_of_row = blob_it.data ()->bounding_box ().left ();
      end_of_row = blob_it.data_relative (-1)->bounding_box ().right ();
      if (min_left > start_of_row)
        min_left = start_of_row;
      if (max_right < end_of_row)
        max_right = end_of_row;
    }
  }
  if ((total_rows < 3) || (min_left >= max_right)) {
    total_rows = 0;
    min_left = max_right = 0;
    return;
  }
  bucket_size = (inT16) floor (xht_stats.median () + 0.5) / 2;
  map_max = (max_right - min_left) / bucket_size;
  map = (inT16 *) alloc_mem ((map_max + 1) * sizeof (inT16));
  for (i = 0; i <= map_max; i++)
    map[i] = 0;

  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    if (!row->blob_list ()->empty ()) {
      blob_it.set_to_list (row->blob_list ());
      blob_it.mark_cycle_pt ();
      blob_box = box_next (&blob_it);
      prev_blob_box = blob_box;
      if (gapmap_use_ends) {
        /* Leading space */
        gap_width = blob_box.left () - min_left;
        if ((gap_width > gapmap_big_gaps * row->xheight)
        && gap_width > 2) {
          max_quantum = (blob_box.left () - min_left) / bucket_size;
          for (i = 0; i <= max_quantum; i++)
            map[i]++;
        }
      }
      while (!blob_it.cycled_list ()) {
        blob_box = box_next (&blob_it);
        gap_width = blob_box.left () - prev_blob_box.right ();
        if ((gap_width > gapmap_big_gaps * row->xheight)
        && gap_width > 2) {
          min_quantum =
            (prev_blob_box.right () - min_left) / bucket_size;
          max_quantum = (blob_box.left () - min_left) / bucket_size;
          for (i = min_quantum; i <= max_quantum; i++)
            map[i]++;
        }
        prev_blob_box = blob_box;
      }
      if (gapmap_use_ends) {
        /* Trailing space */
        gap_width = max_right - prev_blob_box.right ();
        if ((gap_width > gapmap_big_gaps * row->xheight)
        && gap_width > 2) {
          min_quantum =
            (prev_blob_box.right () - min_left) / bucket_size;
          for (i = min_quantum; i <= map_max; i++)
            map[i]++;
        }
      }
    }
  }
  for (i = 0; i <= map_max; i++) {
    if (map[i] > total_rows / 2) {
      if (gapmap_no_isolated_quanta &&
        (((i == 0) &&
        (map[i + 1] <= total_rows / 2)) ||
        ((i == map_max) &&
        (map[i - 1] <= total_rows / 2)) ||
        ((i > 0) &&
        (i < map_max) &&
        (map[i - 1] <= total_rows / 2) &&
      (map[i + 1] <= total_rows / 2)))) {
        map[i] = 0;              //prevent isolated quantum
      }
      else
        any_tabs = TRUE;
    }
  }
  if (gapmap_debug && any_tabs)
    tprintf ("Table found\n");
}


/*************************************************************************
 * GAPMAP::table_gap()
 * Is there a bucket in the specified range where more than half the rows in the
 * block have a wide gap?
 *************************************************************************/

BOOL8 GAPMAP::table_gap(             //Is gap a table?
                        inT16 left,  //From here
                        inT16 right  //To here
                       ) {
  inT16 min_quantum;
  inT16 max_quantum;
  inT16 i;
  BOOL8 tab_found = FALSE;

  if (!any_tabs)
    return FALSE;

  min_quantum = (left - min_left) / bucket_size;
  max_quantum = (right - min_left) / bucket_size;
  for (i = min_quantum; (!tab_found && (i <= max_quantum)); i++)
    if (map[i] > total_rows / 2)
      tab_found = TRUE;
  return tab_found;
}
