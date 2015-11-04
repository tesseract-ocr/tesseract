/**********************************************************************
 * tospace.cpp
 *
 * Compute fuzzy word spacing thresholds for each row.
 * I.e. set :   max_nonspace
 *              space_threshold
 *              min_space
 *              kern_size
 *              space_size
 * for each row.
 * ONLY FOR PROPORTIONAL BLOCKS - FIXED PITCH IS ASSUMED ALREADY DONE
 *
 * Note: functions in this file were originally not members of any
 * class or enclosed by any namespace. Now they are all static members
 * of the Textord class.
 *
 **********************************************************************/

#include "drawtord.h"
#include "ndminx.h"
#include "statistc.h"
#include "textord.h"
#include "tovars.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define MAXSPACING      128      /*max expected spacing in pix */

namespace tesseract {
void Textord::to_spacing(
    ICOORD page_tr,        //topright of page
    TO_BLOCK_LIST *blocks  //blocks on page
                         ) {
  TO_BLOCK_IT block_it;          //iterator
  TO_BLOCK *block;               //current block;
  TO_ROW_IT row_it;              //row iterator
  TO_ROW *row;                   //current row
  int block_index;               //block number
  int row_index;                 //row number
  //estimated width of real spaces for whole block
  inT16 block_space_gap_width;
  //estimated width of non space gaps for whole block
  inT16 block_non_space_gap_width;
  BOOL8 old_text_ord_proportional;//old fixed/prop result
  GAPMAP *gapmap = NULL;          //map of big vert gaps in blk

  block_it.set_to_list (blocks);
  block_index = 1;
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    block = block_it.data ();
    gapmap = new GAPMAP (block);
    block_spacing_stats(block,
                        gapmap,
                        old_text_ord_proportional,
                        block_space_gap_width,
                        block_non_space_gap_width);
    // Make sure relative values of block-level space and non-space gap
    // widths are reasonable. The ratio of 1:3 is also used in
    // block_spacing_stats, to corrrect the block_space_gap_width
    // Useful for arabic and hindi, when the non-space gap width is
    // often over-estimated and should not be trusted. A similar ratio
    // is found in block_spacing_stats.
    if (tosp_old_to_method && tosp_old_to_constrain_sp_kn &&
        (float) block_space_gap_width / block_non_space_gap_width < 3.0) {
      block_non_space_gap_width = (inT16) floor (block_space_gap_width / 3.0);
    }
    row_it.set_to_list (block->get_rows ());
    row_index = 1;
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      if ((row->pitch_decision == PITCH_DEF_PROP) ||
      (row->pitch_decision == PITCH_CORR_PROP)) {
        if ((tosp_debug_level > 0) && !old_text_ord_proportional)
          tprintf ("Block %d Row %d: Now Proportional\n",
            block_index, row_index);
        row_spacing_stats(row,
                          gapmap,
                          block_index,
                          row_index,
                          block_space_gap_width,
                          block_non_space_gap_width);
      }
      else {
        if ((tosp_debug_level > 0) && old_text_ord_proportional)
          tprintf
            ("Block %d Row %d: Now Fixed Pitch Decision:%d fp flag:%f\n",
            block_index, row_index, row->pitch_decision,
            row->fixed_pitch);
      }
#ifndef GRAPHICS_DISABLED
      if (textord_show_initial_words)
        plot_word_decisions (to_win, (inT16) row->fixed_pitch, row);
#endif
      row_index++;
    }
    delete gapmap;
    block_index++;
  }
}


/*************************************************************************
 * block_spacing_stats()
 *************************************************************************/

void Textord::block_spacing_stats(
    TO_BLOCK *block,
    GAPMAP *gapmap,
    BOOL8 &old_text_ord_proportional,
    inT16 &block_space_gap_width,     // resulting estimate
    inT16 &block_non_space_gap_width  // resulting estimate
                                  ) {
  TO_ROW_IT row_it;              // row iterator
  TO_ROW *row;                   // current row
  BLOBNBOX_IT blob_it;           // iterator

  STATS centre_to_centre_stats (0, MAXSPACING);
  // DEBUG USE ONLY
  STATS all_gap_stats (0, MAXSPACING);
  STATS space_gap_stats (0, MAXSPACING);
  inT16 minwidth = MAXSPACING;    // narrowest blob
  TBOX blob_box;
  TBOX prev_blob_box;
  inT16 centre_to_centre;
  inT16 gap_width;
  float real_space_threshold;
  float iqr_centre_to_centre;    // DEBUG USE ONLY
  float iqr_all_gap_stats;       // DEBUG USE ONLY
  inT32 end_of_row;
  inT32 row_length;

  row_it.set_to_list (block->get_rows ());
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    if (!row->blob_list ()->empty () &&
      (!tosp_only_use_prop_rows ||
      (row->pitch_decision == PITCH_DEF_PROP) ||
    (row->pitch_decision == PITCH_CORR_PROP))) {
      blob_it.set_to_list (row->blob_list ());
      blob_it.mark_cycle_pt ();
      end_of_row = blob_it.data_relative (-1)->bounding_box ().right ();
      if (tosp_use_pre_chopping)
        blob_box = box_next_pre_chopped (&blob_it);
      else if (tosp_stats_use_xht_gaps)
        blob_box = reduced_box_next (row, &blob_it);
      else
        blob_box = box_next (&blob_it);
      row_length = end_of_row - blob_box.left ();
      if (blob_box.width () < minwidth)
        minwidth = blob_box.width ();
      prev_blob_box = blob_box;
      while (!blob_it.cycled_list ()) {
        if (tosp_use_pre_chopping)
          blob_box = box_next_pre_chopped (&blob_it);
        else if (tosp_stats_use_xht_gaps)
          blob_box = reduced_box_next (row, &blob_it);
        else
          blob_box = box_next (&blob_it);
        if (blob_box.width () < minwidth)
          minwidth = blob_box.width ();
        gap_width = blob_box.left () - prev_blob_box.right ();
        if (!ignore_big_gap (row, row_length, gapmap,
                             prev_blob_box.right (), blob_box.left ())) {
          all_gap_stats.add (gap_width, 1);

          centre_to_centre = (blob_box.left () + blob_box.right () -
            (prev_blob_box.left () +
             prev_blob_box.right ())) / 2;
          //DEBUG
          centre_to_centre_stats.add (centre_to_centre, 1);
          // DEBUG
        }
        prev_blob_box = blob_box;
      }
    }
  }

                                 //Inadequate samples
  if (all_gap_stats.get_total () <= 1) {
    block_non_space_gap_width = minwidth;
    block_space_gap_width = -1;  //No est. space width
                                 //DEBUG
    old_text_ord_proportional = TRUE;
  }
  else {
    /* For debug only ..... */
    iqr_centre_to_centre = centre_to_centre_stats.ile (0.75) -
      centre_to_centre_stats.ile (0.25);
    iqr_all_gap_stats = all_gap_stats.ile (0.75) - all_gap_stats.ile (0.25);
    old_text_ord_proportional =
      iqr_centre_to_centre * 2 > iqr_all_gap_stats;
    /* .......For debug only */

    /*
    The median of the gaps is used as an estimate of the NON-SPACE gap width.
    This RELIES on the assumption that there are more gaps WITHIN words than
    BETWEEN words in a block

    Now try to estimate the width of a real space for all real spaces in the
    block. Do this by using a crude threshold to ignore "narrow" gaps, then
    find the median of the "wide" gaps and use this.
    */
    block_non_space_gap_width = (inT16) floor (all_gap_stats.median ());
    // median gap

    row_it.set_to_list (block->get_rows ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      if (!row->blob_list ()->empty () &&
        (!tosp_only_use_prop_rows ||
        (row->pitch_decision == PITCH_DEF_PROP) ||
      (row->pitch_decision == PITCH_CORR_PROP))) {
        real_space_threshold =
          MAX (tosp_init_guess_kn_mult * block_non_space_gap_width,
          tosp_init_guess_xht_mult * row->xheight);
        blob_it.set_to_list (row->blob_list ());
        blob_it.mark_cycle_pt ();
        end_of_row =
          blob_it.data_relative (-1)->bounding_box ().right ();
        if (tosp_use_pre_chopping)
          blob_box = box_next_pre_chopped (&blob_it);
        else if (tosp_stats_use_xht_gaps)
          blob_box = reduced_box_next (row, &blob_it);
        else
          blob_box = box_next (&blob_it);
        row_length = blob_box.left () - end_of_row;
        prev_blob_box = blob_box;
        while (!blob_it.cycled_list ()) {
          if (tosp_use_pre_chopping)
            blob_box = box_next_pre_chopped (&blob_it);
          else if (tosp_stats_use_xht_gaps)
            blob_box = reduced_box_next (row, &blob_it);
          else
            blob_box = box_next (&blob_it);
          gap_width = blob_box.left () - prev_blob_box.right ();
          if ((gap_width > real_space_threshold) &&
            !ignore_big_gap (row, row_length, gapmap,
            prev_blob_box.right (),
          blob_box.left ())) {
            /*
            If tosp_use_cert_spaces is enabled, the estimate of the space gap is
            restricted to obvious spaces - those wider than half the xht or those
            with wide blobs on both sides - i.e not things that are suspect 1's or
            punctuation that is sometimes widely spaced.
            */
            if (!tosp_block_use_cert_spaces ||
              (gap_width >
              tosp_fuzzy_space_factor2 * row->xheight)
              ||
              ((gap_width >
              tosp_fuzzy_space_factor1 * row->xheight)
              && (!tosp_narrow_blobs_not_cert
              || (!narrow_blob (row, prev_blob_box)
              && !narrow_blob (row, blob_box))))
              || (wide_blob (row, prev_blob_box)
              && wide_blob (row, blob_box)))
              space_gap_stats.add (gap_width, 1);
          }
          prev_blob_box = blob_box;
        }
      }
    }
                                 //Inadequate samples
    if (space_gap_stats.get_total () <= 2)
      block_space_gap_width = -1;//No est. space width
    else
      block_space_gap_width =
        MAX ((inT16) floor (space_gap_stats.median ()),
        3 * block_non_space_gap_width);
  }
}


/*************************************************************************
 * row_spacing_stats()
 * Set values for min_space, max_non_space based on row stats only
 * If failure - return 0 values.
 *************************************************************************/
void Textord::row_spacing_stats(
    TO_ROW *row,
    GAPMAP *gapmap,
    inT16 block_idx,
    inT16 row_idx,
    inT16 block_space_gap_width,    //estimate for block
    inT16 block_non_space_gap_width //estimate for block
                                ) {
  //iterator
  BLOBNBOX_IT blob_it = row->blob_list ();
  STATS all_gap_stats (0, MAXSPACING);
  STATS cert_space_gap_stats (0, MAXSPACING);
  STATS all_space_gap_stats (0, MAXSPACING);
  STATS small_gap_stats (0, MAXSPACING);
  TBOX blob_box;
  TBOX prev_blob_box;
  inT16 gap_width;
  inT16 real_space_threshold = 0;
  inT16 max = 0;
  inT16 index;
  inT16 large_gap_count = 0;
  BOOL8 suspected_table;
  inT32 max_max_nonspace;        //upper bound
  BOOL8 good_block_space_estimate = block_space_gap_width > 0;
  inT32 end_of_row;
  inT32 row_length = 0;
  float sane_space;
  inT32 sane_threshold;

  /* Collect first pass stats for row */

  if (!good_block_space_estimate)
    block_space_gap_width = inT16 (floor (row->xheight / 2));
  if (!row->blob_list ()->empty ()) {
    if (tosp_threshold_bias1 > 0)
      real_space_threshold =
        block_non_space_gap_width +
        inT16 (floor (0.5 +
        tosp_threshold_bias1 * (block_space_gap_width -
                                block_non_space_gap_width)));
    else
      real_space_threshold =     //Old TO method
        (block_space_gap_width + block_non_space_gap_width) / 2;
    blob_it.set_to_list (row->blob_list ());
    blob_it.mark_cycle_pt ();
    end_of_row = blob_it.data_relative (-1)->bounding_box ().right ();
    if (tosp_use_pre_chopping)
      blob_box = box_next_pre_chopped (&blob_it);
    else if (tosp_stats_use_xht_gaps)
      blob_box = reduced_box_next (row, &blob_it);
    else
      blob_box = box_next (&blob_it);
    row_length = end_of_row - blob_box.left ();
    prev_blob_box = blob_box;
    while (!blob_it.cycled_list ()) {
      if (tosp_use_pre_chopping)
        blob_box = box_next_pre_chopped (&blob_it);
      else if (tosp_stats_use_xht_gaps)
        blob_box = reduced_box_next (row, &blob_it);
      else
        blob_box = box_next (&blob_it);
      gap_width = blob_box.left () - prev_blob_box.right ();
      if (ignore_big_gap (row, row_length, gapmap,
        prev_blob_box.right (), blob_box.left ()))
        large_gap_count++;
      else {
        if (gap_width >= real_space_threshold) {
          if (!tosp_row_use_cert_spaces ||
            (gap_width > tosp_fuzzy_space_factor2 * row->xheight) ||
            ((gap_width > tosp_fuzzy_space_factor1 * row->xheight)
            && (!tosp_narrow_blobs_not_cert
            || (!narrow_blob (row, prev_blob_box)
            && !narrow_blob (row, blob_box))))
            || (wide_blob (row, prev_blob_box)
            && wide_blob (row, blob_box)))
            cert_space_gap_stats.add (gap_width, 1);
          all_space_gap_stats.add (gap_width, 1);
        }
        else
          small_gap_stats.add (gap_width, 1);
        all_gap_stats.add (gap_width, 1);
      }
      prev_blob_box = blob_box;
    }
  }
  suspected_table = (large_gap_count > 1) ||
      ((large_gap_count > 0) &&
       (all_gap_stats.get_total () <= tosp_few_samples));

  /* Now determine row kern size, space size and threshold */

  if ((cert_space_gap_stats.get_total () >=
    tosp_enough_space_samples_for_median) ||
    ((suspected_table ||
    all_gap_stats.get_total () <= tosp_short_row) &&
    cert_space_gap_stats.get_total () > 0)) {
    old_to_method(row,
                  &all_gap_stats,
                  &cert_space_gap_stats,
                  &small_gap_stats,
                  block_space_gap_width,
                  block_non_space_gap_width);
  } else {
    if (!tosp_recovery_isolated_row_stats ||
        !isolated_row_stats (row, gapmap, &all_gap_stats, suspected_table,
                             block_idx, row_idx)) {
      if (tosp_row_use_cert_spaces && (tosp_debug_level > 5))
        tprintf ("B:%d R:%d -- Inadequate certain spaces.\n",
          block_idx, row_idx);
      if (tosp_row_use_cert_spaces1 && good_block_space_estimate) {
                                 //Use block default
        row->space_size = block_space_gap_width;
        if (all_gap_stats.get_total () > tosp_redo_kern_limit)
          row->kern_size = all_gap_stats.median ();
        else
          row->kern_size = block_non_space_gap_width;
        row->space_threshold =
          inT32 (floor ((row->space_size + row->kern_size) /
                        tosp_old_sp_kn_th_factor));
      }
      else
        old_to_method(row,
                      &all_gap_stats,
                      &all_space_gap_stats,
                      &small_gap_stats,
                      block_space_gap_width,
                      block_non_space_gap_width);
    }
  }

  if (tosp_improve_thresh && !suspected_table)
    improve_row_threshold(row, &all_gap_stats);

  /* Now lets try to be careful not to do anything silly with tables when we
  are ignoring big gaps*/
  if (tosp_sanity_method == 0) {
    if (suspected_table &&
    (row->space_size < tosp_table_kn_sp_ratio * row->kern_size)) {
      if (tosp_debug_level > 5)
        tprintf ("B:%d R:%d -- DON'T BELIEVE SPACE %3.2f %d %3.2f.\n",
          block_idx, row_idx,
          row->kern_size, row->space_threshold, row->space_size);
      row->space_threshold =
        (inT32) (tosp_table_kn_sp_ratio * row->kern_size);
      row->space_size = MAX (row->space_threshold + 1, row->xheight);
    }
  }
  else if (tosp_sanity_method == 1) {
    sane_space = row->space_size;
    /* NEVER let space size get too close to kern size */
    if ((row->space_size < tosp_min_sane_kn_sp * MAX (row->kern_size, 2.5))
      || ((row->space_size - row->kern_size) <
    (tosp_silly_kn_sp_gap * row->xheight))) {
      if (good_block_space_estimate &&
        (block_space_gap_width >= tosp_min_sane_kn_sp * row->kern_size))
        sane_space = block_space_gap_width;
      else
        sane_space =
          MAX (tosp_min_sane_kn_sp * MAX (row->kern_size, 2.5),
          row->xheight / 2);
      if (tosp_debug_level > 5)
        tprintf
          ("B:%d R:%d -- DON'T BELIEVE SPACE %3.2f %d %3.2f -> %3.2f.\n",
          block_idx, row_idx, row->kern_size, row->space_threshold,
          row->space_size, sane_space);
      row->space_size = sane_space;
      row->space_threshold =
        inT32 (floor ((row->space_size + row->kern_size) /
                      tosp_old_sp_kn_th_factor));
    }
    /* NEVER let threshold get VERY far away from kern */
    sane_threshold = inT32 (floor (tosp_max_sane_kn_thresh *
      MAX (row->kern_size, 2.5)));
    if (row->space_threshold > sane_threshold) {
      if (tosp_debug_level > 5)
        tprintf ("B:%d R:%d -- DON'T BELIEVE THRESH %3.2f %d %3.2f->%d.\n",
          block_idx, row_idx,
          row->kern_size,
          row->space_threshold, row->space_size, sane_threshold);
      row->space_threshold = sane_threshold;
      if (row->space_size <= sane_threshold)
        row->space_size = row->space_threshold + 1.0f;
    }
    /* Beware of tables - there may be NO spaces */
    if (suspected_table) {
      sane_space = MAX (tosp_table_kn_sp_ratio * row->kern_size,
        tosp_table_xht_sp_ratio * row->xheight);
      sane_threshold = inT32 (floor ((sane_space + row->kern_size) / 2));

      if ((row->space_size < sane_space) ||
      (row->space_threshold < sane_threshold)) {
        if (tosp_debug_level > 5)
          tprintf ("B:%d R:%d -- SUSPECT NO SPACES %3.2f %d %3.2f.\n",
            block_idx, row_idx,
            row->kern_size,
            row->space_threshold, row->space_size);
                                 //the minimum sane value
        row->space_threshold = (inT32) sane_space;
        row->space_size = MAX (row->space_threshold + 1, row->xheight);
      }
    }
  }

  /* Now lets try to put some error limits on the threshold */

  if (tosp_old_to_method) {
    /* Old textord made a space if gap >= threshold */
                                 //NO FUZZY SPACES YET
    row->max_nonspace = row->space_threshold;
                                 //NO FUZZY SPACES       YET
    row->min_space = row->space_threshold + 1;
  }
  else {
    /* Any gap greater than 0.6 x-ht is bound to be a space (isn't it:-) */
    row->min_space =
      MIN (inT32 (ceil (tosp_fuzzy_space_factor * row->xheight)),
      inT32 (row->space_size));
    if (row->min_space <= row->space_threshold)
                                 //Don't be silly
      row->min_space = row->space_threshold + 1;
    /*
    Lets try to guess the max certain kern gap by looking at the cluster of
    kerns for the row. The row is proportional so the kerns should cluster
    tightly at the bottom of the distribution. We also expect most gaps to be
    kerns. Find the maximum of the kern piles between 0 and twice the kern
    estimate. Piles before the first one with less than 1/10 the maximum
    number of samples can be taken as certain kerns.

      Of course, there are some cases where the kern peak and space peaks merge,
      so we will put an UPPER limit on the max certain kern gap of some fraction
      below the threshold.
    */

    max_max_nonspace = inT32 ((row->space_threshold + row->kern_size) / 2);

                                 //default
    row->max_nonspace = max_max_nonspace;
    for (index = 0; index <= max_max_nonspace; index++) {
      if (all_gap_stats.pile_count (index) > max)
        max = all_gap_stats.pile_count (index);
      if ((index > row->kern_size) &&
      (all_gap_stats.pile_count (index) < 0.1 * max)) {
        row->max_nonspace = index;
        break;
      }
    }
  }

  /* Yet another algorithm - simpler this time - just choose a fraction of the
  threshold to space range */

  if ((tosp_fuzzy_sp_fraction > 0) &&
    (row->space_size > row->space_threshold))
    row->min_space = MAX (row->min_space,
      (inT32) ceil (row->space_threshold +
      tosp_fuzzy_sp_fraction *
      (row->space_size -
      row->space_threshold)));

  /* Ensure that ANY space less than some multiplier times the kern size is
  fuzzy.  In tables there is a risk of erroneously setting a small space size
  when there are no real spaces. Sometimes tables have text squashed into
  columns so that the kn->sp ratio is small anyway - this means that we can't
  use this to force a wider separation - hence we rely on context to join any
  dubious breaks. */

  if ((tosp_table_fuzzy_kn_sp_ratio > 0) &&
    (suspected_table || tosp_fuzzy_limit_all))
    row->min_space = MAX (row->min_space,
      (inT32) ceil (tosp_table_fuzzy_kn_sp_ratio *
      row->kern_size));

  if ((tosp_fuzzy_kn_fraction > 0) && (row->kern_size < row->space_threshold)) {
    row->max_nonspace = (inT32) floor (0.5 + row->kern_size +
      tosp_fuzzy_kn_fraction *
      (row->space_threshold -
      row->kern_size));
  }
  if (row->max_nonspace > row->space_threshold) {
                                 //Don't be silly
    row->max_nonspace = row->space_threshold;
  }

  if (tosp_debug_level > 5)
    tprintf
      ("B:%d R:%d L:%d-- Kn:%d Sp:%d Thr:%d -- Kn:%3.2f (%d) Thr:%d (%d) Sp:%3.2f\n",
      block_idx, row_idx, row_length, block_non_space_gap_width,
      block_space_gap_width, real_space_threshold, row->kern_size,
      row->max_nonspace, row->space_threshold, row->min_space,
      row->space_size);
  if (tosp_debug_level > 10)
    tprintf("row->kern_size = %3.2f, row->space_size = %3.2f, "
            "row->space_threshold = %d\n",
            row->kern_size, row->space_size, row->space_threshold);
}

void Textord::old_to_method(
    TO_ROW *row,
    STATS *all_gap_stats,
    STATS *space_gap_stats,
    STATS *small_gap_stats,
    inT16 block_space_gap_width,     //estimate for block
    inT16 block_non_space_gap_width  //estimate for block
                            ) {
  /* First, estimate row space size */
  /* Old to condition was > 2 */
  if (space_gap_stats->get_total () >= tosp_enough_space_samples_for_median) {
  //Adequate samples
    /* Set space size to median of spaces BUT limits it if it seems wildly out */
    row->space_size = space_gap_stats->median ();
    if (row->space_size > block_space_gap_width * 1.5) {
      if (tosp_old_to_bug_fix)
        row->space_size = block_space_gap_width * 1.5;
      else
                                 //BUG??? should be *1.5
        row->space_size = block_space_gap_width;
    }
    if (row->space_size < (block_non_space_gap_width * 2) + 1)
      row->space_size = (block_non_space_gap_width * 2) + 1;
  }
                                 //Only 1 or 2 samples
  else if (space_gap_stats->get_total () >= 1) {
                                 //hence mean not median
    row->space_size = space_gap_stats->mean ();
    if (row->space_size > block_space_gap_width * 1.5) {
      if (tosp_old_to_bug_fix)
        row->space_size = block_space_gap_width * 1.5;
      else
                                 //BUG??? should be *1.5
        row->space_size = block_space_gap_width;
    }
    if (row->space_size < (block_non_space_gap_width * 3) + 1)
      row->space_size = (block_non_space_gap_width * 3) + 1;
  }
  else {
                                 //Use block default
    row->space_size = block_space_gap_width;
  }

  /* Next, estimate row kern size */
  if ((tosp_only_small_gaps_for_kern) &&
    (small_gap_stats->get_total () > tosp_redo_kern_limit))
    row->kern_size = small_gap_stats->median ();
  else if (all_gap_stats->get_total () > tosp_redo_kern_limit)
    row->kern_size = all_gap_stats->median ();
  else                          //old TO -SAME FOR ALL ROWS
    row->kern_size = block_non_space_gap_width;

  /* Finally, estimate row space threshold */
  if (tosp_threshold_bias2 > 0) {
    row->space_threshold =
        inT32 (floor (0.5 + row->kern_size +
                      tosp_threshold_bias2 * (row->space_size -
                                              row->kern_size)));
  } else {
    /*
      NOTE old text ord uses (space_size + kern_size + 1)/2  as the threshold
    and holds this in a float. The use is with a >= test
    NEW textord uses an integer threshold and a > test
    It comes to the same thing.
      (Though there is a difference in that old textor has integer space_size
      and kern_size.)
    */
    row->space_threshold =
        inT32 (floor ((row->space_size + row->kern_size) / 2));
  }

  // Apply the same logic and ratios as in row_spacing_stats to
  // restrict relative values of the row's space_size, kern_size, and
  // space_threshold
  if (tosp_old_to_constrain_sp_kn && tosp_sanity_method == 1 &&
      ((row->space_size <
        tosp_min_sane_kn_sp * MAX (row->kern_size, 2.5)) ||
       ((row->space_size - row->kern_size) <
        tosp_silly_kn_sp_gap * row->xheight))) {
    if (row->kern_size > 2.5)
      row->kern_size = row->space_size / tosp_min_sane_kn_sp;
    row->space_threshold = inT32 (floor ((row->space_size + row->kern_size) /
                                         tosp_old_sp_kn_th_factor));
  }
}


/*************************************************************************
 * isolated_row_stats()
 * Set values for min_space, max_non_space based on row stats only
 *************************************************************************/
BOOL8 Textord::isolated_row_stats(TO_ROW *row,
                                  GAPMAP *gapmap,
                                  STATS *all_gap_stats,
                                  BOOL8 suspected_table,
                                  inT16 block_idx,
                                  inT16 row_idx) {
  float kern_estimate;
  float crude_threshold_estimate;
  inT16 small_gaps_count;
  inT16 total;
  //iterator
  BLOBNBOX_IT blob_it = row->blob_list ();
  STATS cert_space_gap_stats (0, MAXSPACING);
  STATS all_space_gap_stats (0, MAXSPACING);
  STATS small_gap_stats (0, MAXSPACING);
  TBOX blob_box;
  TBOX prev_blob_box;
  inT16 gap_width;
  inT32 end_of_row;
  inT32 row_length;

  kern_estimate = all_gap_stats->median ();
  crude_threshold_estimate = MAX (tosp_init_guess_kn_mult * kern_estimate,
    tosp_init_guess_xht_mult * row->xheight);
  small_gaps_count = stats_count_under (all_gap_stats,
    (inT16)
    ceil (crude_threshold_estimate));
  total = all_gap_stats->get_total ();

  if ((total <= tosp_redo_kern_limit) ||
    ((small_gaps_count / (float) total) < tosp_enough_small_gaps) ||
  (total - small_gaps_count < 1)) {
    if (tosp_debug_level > 5)
      tprintf ("B:%d R:%d -- Can't do isolated row stats.\n",
        block_idx, row_idx);
    return FALSE;
  }
  blob_it.set_to_list (row->blob_list ());
  blob_it.mark_cycle_pt ();
  end_of_row = blob_it.data_relative (-1)->bounding_box ().right ();
  if (tosp_use_pre_chopping)
    blob_box = box_next_pre_chopped (&blob_it);
  else if (tosp_stats_use_xht_gaps)
    blob_box = reduced_box_next (row, &blob_it);
  else
    blob_box = box_next (&blob_it);
  row_length = end_of_row - blob_box.left ();
  prev_blob_box = blob_box;
  while (!blob_it.cycled_list ()) {
    if (tosp_use_pre_chopping)
      blob_box = box_next_pre_chopped (&blob_it);
    else if (tosp_stats_use_xht_gaps)
      blob_box = reduced_box_next (row, &blob_it);
    else
      blob_box = box_next (&blob_it);
    gap_width = blob_box.left () - prev_blob_box.right ();
    if (!ignore_big_gap (row, row_length, gapmap,
      prev_blob_box.right (), blob_box.left ()) &&
    (gap_width > crude_threshold_estimate)) {
      if ((gap_width > tosp_fuzzy_space_factor2 * row->xheight) ||
        ((gap_width > tosp_fuzzy_space_factor1 * row->xheight) &&
        (!tosp_narrow_blobs_not_cert ||
        (!narrow_blob (row, prev_blob_box) &&
        !narrow_blob (row, blob_box)))) ||
        (wide_blob (row, prev_blob_box) && wide_blob (row, blob_box)))
        cert_space_gap_stats.add (gap_width, 1);
      all_space_gap_stats.add (gap_width, 1);
    }
    if (gap_width < crude_threshold_estimate)
      small_gap_stats.add (gap_width, 1);

    prev_blob_box = blob_box;
  }
  if (cert_space_gap_stats.get_total () >=
    tosp_enough_space_samples_for_median)
                                 //median
    row->space_size = cert_space_gap_stats.median ();
  else if (suspected_table && (cert_space_gap_stats.get_total () > 0))
                                 //to avoid spaced
    row->space_size = cert_space_gap_stats.mean ();
  //      1's in tables
  else if (all_space_gap_stats.get_total () >=
    tosp_enough_space_samples_for_median)
                                 //median
    row->space_size = all_space_gap_stats.median ();
  else
    row->space_size = all_space_gap_stats.mean ();

  if (tosp_only_small_gaps_for_kern)
    row->kern_size = small_gap_stats.median ();
  else
    row->kern_size = all_gap_stats->median ();
  row->space_threshold =
    inT32 (floor ((row->space_size + row->kern_size) / 2));
  /* Sanity check */
  if ((row->kern_size >= row->space_threshold) ||
    (row->space_threshold >= row->space_size) ||
  (row->space_threshold <= 0)) {
    if (tosp_debug_level > 5)
      tprintf ("B:%d R:%d -- Isolated row stats SANITY FAILURE: %f %d %f\n",
        block_idx, row_idx,
        row->kern_size, row->space_threshold, row->space_size);
    row->kern_size = 0.0f;
    row->space_threshold = 0;
    row->space_size = 0.0f;
    return FALSE;
  }

  if (tosp_debug_level > 5)
    tprintf ("B:%d R:%d -- Isolated row stats: %f %d %f\n",
      block_idx, row_idx,
      row->kern_size, row->space_threshold, row->space_size);
  return TRUE;
}

inT16 Textord::stats_count_under(STATS *stats, inT16 threshold) {
  inT16 index;
  inT16 total = 0;

  for (index = 0; index < threshold; index++)
    total += stats->pile_count (index);
  return total;
}


/*************************************************************************
 * improve_row_threshold()
 *    Try to recognise a "normal line" -
 *           > 25 gaps
 *     &&    space > 3 * kn  && space > 10
 *              (I.e. reasonably large space and kn:sp ratio)
 *     &&    > 3/4 # gaps < kn + (sp - kn)/3
 *              (I.e. most gaps are well away from space estimate)
 *     &&    a gap of max( 3, (sp - kn)/3 ) empty histogram positions is found
 *           somewhere in the histogram between kn and sp
 *     THEN set the threshold and fuzzy limits to this gap - ie NO fuzzies
 *          NO!!!!! the bristol line has "11" with a gap of 12 between the 1's!!!
 *          try moving the default threshold to within this band but leave the
 *          fuzzy limit calculation as at present.
 *************************************************************************/
void Textord::improve_row_threshold(TO_ROW *row, STATS *all_gap_stats) {
  float sp = row->space_size;
  float kn = row->kern_size;
  inT16 reqd_zero_width = 0;
  inT16 zero_width = 0;
  inT16 zero_start = 0;
  inT16 index = 0;

  if (tosp_debug_level > 10)
    tprintf ("Improve row threshold 0");
  if ((all_gap_stats->get_total () <= 25) ||
    (sp <= 10) ||
    (sp <= 3 * kn) ||
    (stats_count_under (all_gap_stats,
    (inT16) ceil (kn + (sp - kn) / 3 + 0.5)) <
    (0.75 * all_gap_stats->get_total ())))
    return;
  if (tosp_debug_level > 10)
    tprintf (" 1");
  /*
  Look for the first region of all 0's in the histogram which is wider than
  max( 3, (sp - kn)/3 ) and starts between kn and sp. If found, and current
  threshold is not within it, move the threshold so that is is just inside it.
  */
  reqd_zero_width = (inT16) floor ((sp - kn) / 3 + 0.5);
  if (reqd_zero_width < 3)
    reqd_zero_width = 3;

  for (index = inT16 (ceil (kn)); index < inT16 (floor (sp)); index++) {
    if (all_gap_stats->pile_count (index) == 0) {
      if (zero_width == 0)
        zero_start = index;
      zero_width++;
    }
    else {
      if (zero_width >= reqd_zero_width)
        break;
      else {
        zero_width = 0;
      }
    }
  }
  index--;
  if (tosp_debug_level > 10)
    tprintf (" reqd_z_width: %d found %d 0's, starting %d; thresh: %d/n",
      reqd_zero_width, zero_width, zero_start, row->space_threshold);
  if ((zero_width < reqd_zero_width) ||
    ((row->space_threshold >= zero_start) &&
    (row->space_threshold <= index)))
    return;
  if (tosp_debug_level > 10)
    tprintf (" 2");
  if (row->space_threshold < zero_start) {
    if (tosp_debug_level > 5)
      tprintf
        ("Improve row kn:%5.2f sp:%5.2f 0's: %d -> %d  thresh:%d -> %d\n",
        kn, sp, zero_start, index, row->space_threshold, zero_start);
    row->space_threshold = zero_start;
  }
  if (row->space_threshold > index) {
    if (tosp_debug_level > 5)
      tprintf
        ("Improve row kn:%5.2f sp:%5.2f 0's: %d -> %d  thresh:%d -> %d\n",
        kn, sp, zero_start, index, row->space_threshold, index);
    row->space_threshold = index;
  }
}


/**********************************************************************
 * make_prop_words
 *
 * Convert a TO_BLOCK to a BLOCK.
 **********************************************************************/
ROW *Textord::make_prop_words(
    TO_ROW *row,     // row to make
    FCOORD rotation  // for drawing
                              ) {
  BOOL8 bol;                     // start of line
  /* prev_ values are for start of word being built. non prev_ values are for
  the gap between the word being built and the next one. */
  BOOL8 prev_fuzzy_sp;           // probably space
  BOOL8 prev_fuzzy_non;          // probably not
  uinT8 prev_blanks;             // in front of word
  BOOL8 fuzzy_sp = false;        // probably space
  BOOL8 fuzzy_non = false;       // probably not
  uinT8 blanks = 0;              // in front of word
  BOOL8 prev_gap_was_a_space = FALSE;
  BOOL8 break_at_next_gap = FALSE;
  ROW *real_row;                 // output row
  C_OUTLINE_IT cout_it;
  C_BLOB_LIST cblobs;
  C_BLOB_IT cblob_it = &cblobs;
  WERD_LIST words;
  WERD_IT word_it;               // new words
  WERD *word;                    // new word
  WERD_IT rep_char_it;           // repeated char words
  inT32 next_rep_char_word_right = MAX_INT32;
  float repetition_spacing;      // gap between repetitions
  inT32 xstarts[2];              // row ends
  inT32 prev_x;                  // end of prev blob
  BLOBNBOX *bblob;               // current blob
  TBOX blob_box;                 // bounding box
  BLOBNBOX_IT box_it;            // iterator
  TBOX prev_blob_box;
  TBOX next_blob_box;
  inT16 prev_gap = MAX_INT16;
  inT16 current_gap = MAX_INT16;
  inT16 next_gap = MAX_INT16;
  inT16 prev_within_xht_gap = MAX_INT16;
  inT16 current_within_xht_gap = MAX_INT16;
  inT16 next_within_xht_gap = MAX_INT16;
  inT16 word_count = 0;

  rep_char_it.set_to_list (&(row->rep_words));
  if (!rep_char_it.empty ()) {
    next_rep_char_word_right =
      rep_char_it.data ()->bounding_box ().right ();
  }

  prev_x = -MAX_INT16;
  cblob_it.set_to_list (&cblobs);
  box_it.set_to_list (row->blob_list ());
  word_it.set_to_list (&words);
  bol = TRUE;
  prev_blanks = 0;
  prev_fuzzy_sp = FALSE;
  prev_fuzzy_non = FALSE;
  if (!box_it.empty ()) {
    xstarts[0] = box_it.data ()->bounding_box ().left ();
    if (xstarts[0] > next_rep_char_word_right) {
      /* We need to insert a repeated char word at the start of the row */
      word = rep_char_it.extract ();
      word_it.add_after_then_move (word);
      /* Set spaces before repeated char word */
      word->set_flag (W_BOL, TRUE);
      bol = FALSE;
      word->set_blanks (0);
                                 //NO uncertainty
      word->set_flag (W_FUZZY_SP, FALSE);
      word->set_flag (W_FUZZY_NON, FALSE);
      xstarts[0] = word->bounding_box ().left ();
      /* Set spaces after repeated char word (and leave current word set) */
      repetition_spacing = find_mean_blob_spacing (word);
      current_gap = box_it.data ()->bounding_box ().left () -
        next_rep_char_word_right;
      current_within_xht_gap = current_gap;
      if (current_gap > tosp_rep_space * repetition_spacing) {
        prev_blanks = (uinT8) floor (current_gap / row->space_size);
        if (prev_blanks < 1)
          prev_blanks = 1;
      }
      else
        prev_blanks = 0;
      if (tosp_debug_level > 5)
        tprintf ("Repch wd at BOL(%d, %d). rep spacing %5.2f;  Rgap:%d  ",
          box_it.data ()->bounding_box ().left (),
          box_it.data ()->bounding_box ().bottom (),
          repetition_spacing, current_gap);
      prev_fuzzy_sp = FALSE;
      prev_fuzzy_non = FALSE;
      if (rep_char_it.empty ()) {
        next_rep_char_word_right = MAX_INT32;
      }
      else {
        rep_char_it.forward ();
        next_rep_char_word_right =
          rep_char_it.data ()->bounding_box ().right ();
      }
    }

    peek_at_next_gap(row,
                     box_it,
                     next_blob_box,
                     next_gap,
                     next_within_xht_gap);
    do {
      bblob = box_it.data ();
      blob_box = bblob->bounding_box ();
      if (bblob->joined_to_prev ()) {
        if (bblob->cblob () != NULL) {
          cout_it.set_to_list (cblob_it.data ()->out_list ());
          cout_it.move_to_last ();
          cout_it.add_list_after (bblob->cblob ()->out_list ());
          delete bblob->cblob ();
        }
      } else {
        if (bblob->cblob() != NULL)
          cblob_it.add_after_then_move (bblob->cblob ());
        prev_x = blob_box.right ();
      }
      box_it.forward ();         //next one
      bblob = box_it.data ();
      blob_box = bblob->bounding_box ();

      if (!bblob->joined_to_prev() && bblob->cblob() != NULL) {
        /* Real Blob - not multiple outlines or pre-chopped */
        prev_gap = current_gap;
        prev_within_xht_gap = current_within_xht_gap;
        prev_blob_box = next_blob_box;
        current_gap = next_gap;
        current_within_xht_gap = next_within_xht_gap;
        peek_at_next_gap(row,
                         box_it,
                         next_blob_box,
                         next_gap,
                         next_within_xht_gap);

        inT16 prev_gap_arg = prev_gap;
        inT16 next_gap_arg = next_gap;
        if (tosp_only_use_xht_gaps) {
          prev_gap_arg = prev_within_xht_gap;
          next_gap_arg = next_within_xht_gap;
        }
        // Decide if a word-break should be inserted
        if (blob_box.left () > next_rep_char_word_right ||
            make_a_word_break(row, blob_box, prev_gap_arg, prev_blob_box,
                              current_gap, current_within_xht_gap,
                              next_blob_box, next_gap_arg,
                              blanks, fuzzy_sp, fuzzy_non,
                              prev_gap_was_a_space,
                              break_at_next_gap) ||
            box_it.at_first()) {
          /* Form a new word out of the blobs collected */
          word = new WERD (&cblobs, prev_blanks, NULL);
          word_count++;
          word_it.add_after_then_move (word);
          if (bol) {
            word->set_flag (W_BOL, TRUE);
            bol = FALSE;
          }
          if (prev_fuzzy_sp)
                                 //probably space
            word->set_flag (W_FUZZY_SP, TRUE);
          else if (prev_fuzzy_non)
            word->set_flag (W_FUZZY_NON, TRUE);
          //probably not

          if (blob_box.left () > next_rep_char_word_right) {
            /* We need to insert a repeated char word */
            word = rep_char_it.extract ();
            word_it.add_after_then_move (word);

            /* Set spaces before repeated char word */
            repetition_spacing = find_mean_blob_spacing (word);
            current_gap = word->bounding_box ().left () - prev_x;
            current_within_xht_gap = current_gap;
            if (current_gap > tosp_rep_space * repetition_spacing) {
              blanks =
                (uinT8) floor (current_gap / row->space_size);
              if (blanks < 1)
                blanks = 1;
            }
            else
              blanks = 0;
            if (tosp_debug_level > 5)
              tprintf
                ("Repch wd (%d,%d) rep gap %5.2f;  Lgap:%d (%d blanks);",
                word->bounding_box ().left (),
                word->bounding_box ().bottom (),
                repetition_spacing, current_gap, blanks);
            word->set_blanks (blanks);
                                 //NO uncertainty
            word->set_flag (W_FUZZY_SP, FALSE);
            word->set_flag (W_FUZZY_NON, FALSE);

            /* Set spaces after repeated char word (and leave current word set) */
            current_gap =
              blob_box.left () - next_rep_char_word_right;
            if (current_gap > tosp_rep_space * repetition_spacing) {
              blanks = (uinT8) (current_gap / row->space_size);
              if (blanks < 1)
                blanks = 1;
            }
            else
              blanks = 0;
            if (tosp_debug_level > 5)
              tprintf (" Rgap:%d (%d blanks)\n",
                current_gap, blanks);
            fuzzy_sp = FALSE;
            fuzzy_non = FALSE;

            if (rep_char_it.empty ()) {
              next_rep_char_word_right = MAX_INT32;
            }
            else {
              rep_char_it.forward ();
              next_rep_char_word_right =
                rep_char_it.data ()->bounding_box ().right ();
            }
          }

          if (box_it.at_first () && rep_char_it.empty ()) {
                                 //at end of line
            word->set_flag (W_EOL, TRUE);
            xstarts[1] = prev_x;
          }
          else {
            prev_blanks = blanks;
            prev_fuzzy_sp = fuzzy_sp;
            prev_fuzzy_non = fuzzy_non;
          }
        }
      }
    }
    while (!box_it.at_first ()); //until back at start

    /* Insert any further repeated char words */
    while (!rep_char_it.empty ()) {
      word = rep_char_it.extract ();
      word_it.add_after_then_move (word);

      /* Set spaces before repeated char word */
      repetition_spacing = find_mean_blob_spacing (word);
      current_gap = word->bounding_box ().left () - prev_x;
      if (current_gap > tosp_rep_space * repetition_spacing) {
        blanks = (uinT8) floor (current_gap / row->space_size);
        if (blanks < 1)
          blanks = 1;
      }
      else
        blanks = 0;
      if (tosp_debug_level > 5)
        tprintf
          ("Repch wd at EOL (%d,%d). rep spacing %d; Lgap:%d (%d blanks)\n",
          word->bounding_box ().left (), word->bounding_box ().bottom (),
          repetition_spacing, current_gap, blanks);
      word->set_blanks (blanks);
                                 //NO uncertainty
      word->set_flag (W_FUZZY_SP, FALSE);
      word->set_flag (W_FUZZY_NON, FALSE);
      prev_x = word->bounding_box ().right ();
      if (rep_char_it.empty ()) {
                                 //at end of line
        word->set_flag (W_EOL, TRUE);
        xstarts[1] = prev_x;
      }
      else {
        rep_char_it.forward ();
      }
    }
    real_row = new ROW (row,
      (inT16) row->kern_size, (inT16) row->space_size);
    word_it.set_to_list (real_row->word_list ());
                                 //put words in row
    word_it.add_list_after (&words);
    real_row->recalc_bounding_box ();

    if (tosp_debug_level > 4) {
      tprintf ("Row: Made %d words in row ((%d,%d)(%d,%d))\n",
        word_count,
        real_row->bounding_box ().left (),
        real_row->bounding_box ().bottom (),
        real_row->bounding_box ().right (),
        real_row->bounding_box ().top ());
    }
    return real_row;
  }
  return NULL;
}

/**********************************************************************
 * make_blob_words
 *
 * Converts words into blobs so that each blob is a single character.
 *  Used for chopper test.
 **********************************************************************/
ROW *Textord::make_blob_words(
    TO_ROW *row,     // row to make
    FCOORD rotation  // for drawing
                              ) {
  bool bol;                      // start of line
  ROW *real_row;                 // output row
  C_OUTLINE_IT cout_it;
  C_BLOB_LIST cblobs;
  C_BLOB_IT cblob_it = &cblobs;
  WERD_LIST words;
  WERD_IT word_it;               // new words
  WERD *word;                    // new word
  BLOBNBOX *bblob;               // current blob
  TBOX blob_box;                 // bounding box
  BLOBNBOX_IT box_it;            // iterator
  inT16 word_count = 0;

  cblob_it.set_to_list(&cblobs);
  box_it.set_to_list(row->blob_list());
  word_it.set_to_list(&words);
  bol = TRUE;
  if (!box_it.empty()) {

    do {
      bblob = box_it.data();
      blob_box = bblob->bounding_box();
      if (bblob->joined_to_prev()) {
        if (bblob->cblob() != NULL) {
          cout_it.set_to_list(cblob_it.data()->out_list());
          cout_it.move_to_last();
          cout_it.add_list_after(bblob->cblob()->out_list());
          delete bblob->cblob();
        }
      } else {
        if (bblob->cblob() != NULL)
          cblob_it.add_after_then_move(bblob->cblob());
      }
      box_it.forward();         // next one
      bblob = box_it.data();
      blob_box = bblob->bounding_box();

      if (!bblob->joined_to_prev() && !cblobs.empty()) {
        word = new WERD(&cblobs, 1, NULL);
        word_count++;
        word_it.add_after_then_move(word);
        if (bol) {
          word->set_flag(W_BOL, TRUE);
          bol = FALSE;
        }
        if (box_it.at_first()) { // at end of line
          word->set_flag(W_EOL, TRUE);
        }
      }
    }
    while (!box_it.at_first()); // until back at start
    /* Setup the row with created words. */
    real_row = new ROW(row, (inT16) row->kern_size, (inT16) row->space_size);
    word_it.set_to_list(real_row->word_list());
                                 //put words in row
    word_it.add_list_after(&words);
    real_row->recalc_bounding_box();
    if (tosp_debug_level > 4) {
      tprintf ("Row:Made %d words in row ((%d,%d)(%d,%d))\n",
        word_count,
        real_row->bounding_box().left(),
        real_row->bounding_box().bottom(),
        real_row->bounding_box().right(),
        real_row->bounding_box().top());
    }
    return real_row;
  }
  return NULL;
}

BOOL8 Textord::make_a_word_break(
    TO_ROW *row,   // row being made
    TBOX blob_box, // for next_blob // how many blanks?
    inT16 prev_gap,
    TBOX prev_blob_box,
    inT16 real_current_gap,
    inT16 within_xht_current_gap,
    TBOX next_blob_box,
    inT16 next_gap,
    uinT8 &blanks,
    BOOL8 &fuzzy_sp,
    BOOL8 &fuzzy_non,
    BOOL8& prev_gap_was_a_space,
    BOOL8& break_at_next_gap) {
  BOOL8 space;
  inT16 current_gap;
  float fuzzy_sp_to_kn_limit;

  if (break_at_next_gap) {
    break_at_next_gap = FALSE;
    return TRUE;
  }
  /* Inhibit using the reduced gap if
    The kerning is large - chars are not kerned and reducing "f"s can cause
    erroneous blanks
  OR  The real gap is less than 0
  OR  The real gap is less than the kerning estimate
  */
  if ((row->kern_size > tosp_large_kerning * row->xheight) ||
      ((tosp_dont_fool_with_small_kerns >= 0) &&
       (real_current_gap < tosp_dont_fool_with_small_kerns * row->kern_size)))
                                 //Ignore the difference
    within_xht_current_gap = real_current_gap;

  if (tosp_use_xht_gaps && tosp_only_use_xht_gaps)
    current_gap = within_xht_current_gap;
  else
    current_gap = real_current_gap;

  if (tosp_old_to_method) {
                                 //Boring old method
    space = current_gap > row->max_nonspace;
    if (space && (current_gap < MAX_INT16)) {
      if (current_gap < row->min_space) {
        if (current_gap > row->space_threshold) {
          blanks = 1;
          fuzzy_sp = TRUE;
          fuzzy_non = FALSE;
        }
        else {
          blanks = 0;
          fuzzy_sp = FALSE;
          fuzzy_non = TRUE;
        }
      }
      else {
        blanks = (uinT8) (current_gap / row->space_size);
        if (blanks < 1)
          blanks = 1;
        fuzzy_sp = FALSE;
        fuzzy_non = FALSE;
      }
    }
    return space;
  }
  else {
  /* New exciting heuristic method */
    if (prev_blob_box.null_box ())  // Beginning of row
      prev_gap_was_a_space = TRUE;

                                 //Default as old TO
    space = current_gap > row->space_threshold;

    /* Set defaults for the word break incase we find one.  Currently there are
    no fuzzy spaces. Depending on the reliability of the different heuristics
    we may need to set PARTICULAR spaces to fuzzy or not. The values will ONLY
    be used if the function returns TRUE - ie the word is to be broken.
    */
    blanks = (uinT8) (current_gap / row->space_size);
    if (blanks < 1)
      blanks = 1;
    fuzzy_sp = FALSE;
    fuzzy_non = FALSE;
    /*
    If xht measure causes gap to flip one of the 3 thresholds act accordingly -
    despite any other heuristics - the MINIMUM action is to pass a fuzzy kern to
    context.
    */
    if (tosp_use_xht_gaps &&
      (real_current_gap <= row->max_nonspace) &&
    (within_xht_current_gap > row->max_nonspace)) {
      space = TRUE;
      fuzzy_non = TRUE;
#ifndef GRAPHICS_DISABLED
      mark_gap (blob_box, 20,
        prev_gap, prev_blob_box.width (),
        current_gap, next_blob_box.width (), next_gap);
#endif
    }
    else if (tosp_use_xht_gaps &&
      (real_current_gap <= row->space_threshold) &&
    (within_xht_current_gap > row->space_threshold)) {
      space = TRUE;
      if (tosp_flip_fuzz_kn_to_sp)
        fuzzy_sp = TRUE;
      else
        fuzzy_non = TRUE;
#ifndef GRAPHICS_DISABLED
      mark_gap (blob_box, 21,
        prev_gap, prev_blob_box.width (),
        current_gap, next_blob_box.width (), next_gap);
#endif
    }
    else if (tosp_use_xht_gaps &&
      (real_current_gap < row->min_space) &&
    (within_xht_current_gap >= row->min_space)) {
      space = TRUE;
#ifndef GRAPHICS_DISABLED
      mark_gap (blob_box, 22,
        prev_gap, prev_blob_box.width (),
        current_gap, next_blob_box.width (), next_gap);
#endif
    }
    else if (tosp_force_wordbreak_on_punct &&
             !suspected_punct_blob(row, prev_blob_box) &&
             suspected_punct_blob(row, blob_box)) {
      break_at_next_gap = TRUE;
    }
    /* Now continue with normal heuristics */
    else if ((current_gap < row->min_space) &&
    (current_gap > row->space_threshold)) {
      /* Heuristics to turn dubious spaces to kerns */
      if (tosp_pass_wide_fuzz_sp_to_context > 0)
        fuzzy_sp_to_kn_limit = row->kern_size +
          tosp_pass_wide_fuzz_sp_to_context *
          (row->space_size - row->kern_size);
      else
        fuzzy_sp_to_kn_limit = 99999.0f;

      /* If current gap is significantly smaller than the previous space the other
      side of a narrow blob then this gap is a kern. */
      if ((prev_blob_box.width () > 0) &&
        narrow_blob (row, prev_blob_box) &&
        prev_gap_was_a_space &&
      (current_gap <= tosp_gap_factor * prev_gap)) {
        if ((tosp_all_flips_fuzzy) ||
        (current_gap > fuzzy_sp_to_kn_limit)) {
          if (tosp_flip_fuzz_sp_to_kn)
            fuzzy_non = TRUE;
          else
            fuzzy_sp = TRUE;
        }
        else
          space = FALSE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 1,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
      /* If current gap not much bigger than the previous kern the other side of a
      narrow blob then this gap is a kern as well */
      else if ((prev_blob_box.width () > 0) &&
        narrow_blob (row, prev_blob_box) &&
        !prev_gap_was_a_space &&
      (current_gap * tosp_gap_factor <= prev_gap)) {
        if ((tosp_all_flips_fuzzy) ||
        (current_gap > fuzzy_sp_to_kn_limit)) {
          if (tosp_flip_fuzz_sp_to_kn)
            fuzzy_non = TRUE;
          else
            fuzzy_sp = TRUE;
        }
        else
          space = FALSE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 2,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
      else if ((next_blob_box.width () > 0) &&
        narrow_blob (row, next_blob_box) &&
        (next_gap > row->space_threshold) &&
      (current_gap <= tosp_gap_factor * next_gap)) {
        if ((tosp_all_flips_fuzzy) ||
        (current_gap > fuzzy_sp_to_kn_limit)) {
          if (tosp_flip_fuzz_sp_to_kn)
            fuzzy_non = TRUE;
          else
            fuzzy_sp = TRUE;
        }
        else
          space = FALSE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 3,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
      else if ((next_blob_box.width () > 0) &&
        narrow_blob (row, next_blob_box) &&
        (next_gap <= row->space_threshold) &&
      (current_gap * tosp_gap_factor <= next_gap)) {
        if ((tosp_all_flips_fuzzy) ||
        (current_gap > fuzzy_sp_to_kn_limit)) {
          if (tosp_flip_fuzz_sp_to_kn)
            fuzzy_non = TRUE;
          else
            fuzzy_sp = TRUE;
        }
        else
          space = FALSE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 4,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
      else if ((((next_blob_box.width () > 0) &&
        narrow_blob (row, next_blob_box)) ||
        ((prev_blob_box.width () > 0) &&
      narrow_blob (row, prev_blob_box)))) {
        fuzzy_sp = TRUE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 6,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
    }
    else if ((current_gap > row->max_nonspace) &&
             (current_gap <= row->space_threshold)) {

      /* Heuristics to turn dubious kerns to spaces */
      /* TRIED THIS BUT IT MADE THINGS WORSE
          if ( prev_gap == MAX_INT16 )
            prev_gap = 0;  // start of row
          if ( next_gap == MAX_INT16 )
            next_gap = 0;  // end of row
      */
      if ((prev_blob_box.width () > 0) &&
        (next_blob_box.width () > 0) &&
        (current_gap >=
        tosp_kern_gap_factor1 * MAX (prev_gap, next_gap)) &&
        wide_blob (row, prev_blob_box) &&
      wide_blob (row, next_blob_box)) {

        space = TRUE;
        /*
        tosp_flip_caution is an attempt to stop the default changing in cases
        where there is a large difference between the kern and space estimates.
          See problem in 'chiefs' where "have" gets split in the quotation.
        */
        if ((tosp_flip_fuzz_kn_to_sp) &&
          ((tosp_flip_caution <= 0) ||
          (tosp_flip_caution * row->kern_size > row->space_size)))
          fuzzy_sp = TRUE;
        else
          fuzzy_non = TRUE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 7,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      } else if (prev_blob_box.width() > 0 &&
                 next_blob_box.width() > 0 &&
                 current_gap > 5 &&  // Rule 9 handles small gap, big ratio.
                 current_gap >=
                   tosp_kern_gap_factor2 * MAX(prev_gap, next_gap) &&
                 !(narrow_blob(row, prev_blob_box) ||
                   suspected_punct_blob(row, prev_blob_box)) &&
                 !(narrow_blob(row, next_blob_box) ||
                   suspected_punct_blob(row, next_blob_box))) {
        space = TRUE;
        fuzzy_non = TRUE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 8,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
      else if ((tosp_kern_gap_factor3 > 0) &&
               (prev_blob_box.width () > 0) &&
               (next_blob_box.width () > 0) &&
               (current_gap >= tosp_kern_gap_factor3 * MAX (prev_gap, next_gap)) &&
               (!tosp_rule_9_test_punct ||
                (!suspected_punct_blob (row, prev_blob_box) &&
                 !suspected_punct_blob (row, next_blob_box)))) {
        space = TRUE;
        fuzzy_non = TRUE;
#ifndef GRAPHICS_DISABLED
        mark_gap (blob_box, 9,
          prev_gap, prev_blob_box.width (),
          current_gap, next_blob_box.width (), next_gap);
#endif
      }
    }
    if (tosp_debug_level > 10)
      tprintf("word break = %d current_gap = %d, prev_gap = %d, "
              "next_gap = %d\n", space ? 1 : 0, current_gap,
              prev_gap, next_gap);
    prev_gap_was_a_space = space && !(fuzzy_non);
    return space;
  }
}

BOOL8 Textord::narrow_blob(TO_ROW *row, TBOX blob_box) {
  BOOL8 result;
  result = ((blob_box.width () <= tosp_narrow_fraction * row->xheight) ||
    (((float) blob_box.width () / blob_box.height ()) <=
    tosp_narrow_aspect_ratio));
  return result;
}

BOOL8 Textord::wide_blob(TO_ROW *row, TBOX blob_box) {
  BOOL8 result;
  if (tosp_wide_fraction > 0) {
    if (tosp_wide_aspect_ratio > 0)
      result = ((blob_box.width () >= tosp_wide_fraction * row->xheight) &&
        (((float) blob_box.width () / blob_box.height ()) >
        tosp_wide_aspect_ratio));
    else
      result = (blob_box.width () >= tosp_wide_fraction * row->xheight);
  }
  else
    result = !narrow_blob (row, blob_box);
  return result;
}

BOOL8 Textord::suspected_punct_blob(TO_ROW *row, TBOX box) {
  BOOL8 result;
  float baseline;
  float blob_x_centre;
  /* Find baseline of centre of blob */
  blob_x_centre = (box.right () + box.left ()) / 2.0;
  baseline = row->baseline.y (blob_x_centre);

  result = (box.height () <= 0.66 * row->xheight) ||
           (box.top () < baseline + row->xheight / 2.0) ||
           (box.bottom () > baseline + row->xheight / 2.0);
  return result;
}


void Textord::peek_at_next_gap(TO_ROW *row,
                               BLOBNBOX_IT box_it,
                               TBOX &next_blob_box,
                               inT16 &next_gap,
                               inT16 &next_within_xht_gap) {
  TBOX next_reduced_blob_box;
  TBOX bit_beyond;
  BLOBNBOX_IT reduced_box_it = box_it;

  next_blob_box = box_next (&box_it);
  next_reduced_blob_box = reduced_box_next (row, &reduced_box_it);
  if (box_it.at_first ()) {
    next_gap = MAX_INT16;
    next_within_xht_gap = MAX_INT16;
  }
  else {
    bit_beyond = box_it.data ()->bounding_box ();
    next_gap = bit_beyond.left () - next_blob_box.right ();
    bit_beyond = reduced_box_next (row, &reduced_box_it);
    next_within_xht_gap =
      bit_beyond.left () - next_reduced_blob_box.right ();
  }
}


#ifndef GRAPHICS_DISABLED
void Textord::mark_gap(
    TBOX blob,   // blob following gap
    inT16 rule,  // heuristic id
    inT16 prev_gap,
    inT16 prev_blob_width,
    inT16 current_gap,
    inT16 next_blob_width,
    inT16 next_gap) {
  ScrollView::Color col;                    //of ellipse marking flipped gap

  switch (rule) {
    case 1:
      col = ScrollView::RED;
      break;
    case 2:
      col = ScrollView::CYAN;
      break;
    case 3:
      col = ScrollView::GREEN;
      break;
    case 4:
      col = ScrollView::BLACK;
      break;
    case 5:
      col = ScrollView::MAGENTA;
      break;
    case 6:
      col = ScrollView::BLUE;
      break;

    case 7:
      col = ScrollView::WHITE;
      break;
    case 8:
      col = ScrollView::YELLOW;
      break;
    case 9:
      col = ScrollView::BLACK;
      break;

    case 20:
      col = ScrollView::CYAN;
      break;
    case 21:
      col = ScrollView::GREEN;
      break;
    case 22:
      col = ScrollView::MAGENTA;
      break;
    default:
      col = ScrollView::BLACK;
  }
  if (textord_show_initial_words) {
    to_win->Pen(col);
  /*  if (rule < 20)
      //interior_style(to_win, INT_SOLID, FALSE);
    else
      //interior_style(to_win, INT_HOLLOW, TRUE);*/
                                 //x radius
    to_win->Ellipse (current_gap / 2.0f,
      blob.height () / 2.0f,     //y radius
                                 //x centre
      blob.left () - current_gap / 2.0f,
                                 //y centre
      blob.bottom () + blob.height () / 2.0f);
 }
  if (tosp_debug_level > 5)
    tprintf ("  (%d,%d) Sp<->Kn Rule %d %d %d %d %d\n",
      blob.left () - current_gap / 2, blob.bottom (), rule,
      prev_gap, prev_blob_width, current_gap,
      next_blob_width, next_gap);
}
#endif

float Textord::find_mean_blob_spacing(WERD *word) {
  C_BLOB_IT cblob_it;
  TBOX blob_box;
  inT32 gap_sum = 0;
  inT16 gap_count = 0;
  inT16 prev_right;

  cblob_it.set_to_list (word->cblob_list ());
  if (!cblob_it.empty ()) {
    cblob_it.mark_cycle_pt ();
    prev_right = cblob_it.data ()->bounding_box ().right ();
    //first blob
    cblob_it.forward ();
    for (; !cblob_it.cycled_list (); cblob_it.forward ()) {
      blob_box = cblob_it.data ()->bounding_box ();
      gap_sum += blob_box.left () - prev_right;
      gap_count++;
      prev_right = blob_box.right ();
    }
  }
  if (gap_count > 0)
    return (gap_sum / (float) gap_count);
  else
    return 0.0f;
}


BOOL8 Textord::ignore_big_gap(TO_ROW *row,
                              inT32 row_length,
                              GAPMAP *gapmap,
                              inT16 left,
                              inT16 right) {
  inT16 gap = right - left + 1;

  if (tosp_ignore_big_gaps > 999)
    return FALSE;                //Don't ignore
  if (tosp_ignore_big_gaps > 0)
    return (gap > tosp_ignore_big_gaps * row->xheight);
  if (gap > tosp_ignore_very_big_gaps * row->xheight)
    return TRUE;
  if (tosp_ignore_big_gaps == 0) {
    if ((gap > 2.1 * row->xheight) && (row_length > 20 * row->xheight))
      return TRUE;
    if ((gap > 1.75 * row->xheight) &&
      ((row_length > 35 * row->xheight) ||
      gapmap->table_gap (left, right)))
      return TRUE;
  }
  else {
  /* ONLY time gaps < 3.0 * xht are ignored is when they are part of a table */
    if ((gap > gapmap_big_gaps * row->xheight) &&
      gapmap->table_gap (left, right))
      return TRUE;
  }
  return FALSE;
}


/**********************************************************************
 * reduced_box_next
 *
 * Compute the bounding box of this blob with merging of x overlaps
 * but no pre-chopping.
 * Then move the iterator on to the start of the next blob.
 * DON'T reduce the box for small things - eg punctuation.
 **********************************************************************/
TBOX Textord::reduced_box_next(
    TO_ROW *row,     // current row
    BLOBNBOX_IT *it  // iterator to blobds
                               ) {
  BLOBNBOX *blob;                //current blob
  BLOBNBOX *head_blob;           //place to store box
  TBOX full_box;                  //full blob boundg box
  TBOX reduced_box;               //box of significant part
  inT16 left_above_xht;          //ABOVE xht left limit
  inT16 new_left_above_xht;      //ABOVE xht left limit

  blob = it->data ();
  if (blob->red_box_set ()) {
    reduced_box = blob->reduced_box ();
    do {
      it->forward();
      blob = it->data();
    }
    while (blob->cblob() == NULL || blob->joined_to_prev());
    return reduced_box;
  }
  head_blob = blob;
  full_box = blob->bounding_box ();
  reduced_box = reduced_box_for_blob (blob, row, &left_above_xht);
  do {
    it->forward ();
    blob = it->data ();
    if (blob->cblob() == NULL)
                                 //was pre-chopped
      full_box += blob->bounding_box ();
    else if (blob->joined_to_prev ()) {
      reduced_box +=
        reduced_box_for_blob(blob, row, &new_left_above_xht);
      left_above_xht = MIN (left_above_xht, new_left_above_xht);
    }
  }
                                 //until next real blob
  while (blob->cblob() == NULL || blob->joined_to_prev());

  if ((reduced_box.width () > 0) &&
    ((reduced_box.left () + tosp_near_lh_edge * reduced_box.width ())
  < left_above_xht) && (reduced_box.height () > 0.7 * row->xheight)) {
#ifndef GRAPHICS_DISABLED
    if (textord_show_initial_words)
      reduced_box.plot (to_win, ScrollView::YELLOW, ScrollView::YELLOW);
#endif
  }
  else
    reduced_box = full_box;
  head_blob->set_reduced_box (reduced_box);
  return reduced_box;
}


/*************************************************************************
 * reduced_box_for_blob()
 * Find box for blob which is the same height and y position as the whole blob,
 * but whose left limit is the left most position of the blob ABOVE the
 * baseline and whose right limit is the right most position of the blob BELOW
 * the xheight.
 *
 *
 * !!!!!!! WONT WORK WITH LARGE UPPER CASE CHARS - T F V W - look at examples on
 *         "home".  Perhaps we need something which say if the width ABOVE the
 *         xht alone includes the whole of the reduced width, then use the full
 *         blob box - Might still fail on italic F
 *
 *         Alternatively we could be a little less severe and only reduce the
 *         left and right edges by half the difference between the full box and
 *         the reduced box.
 *
 * NOTE that we need to rotate all the coordinates as
 * find_blob_limits finds the y min and max within a specified x band
 *************************************************************************/
TBOX Textord::reduced_box_for_blob(
    BLOBNBOX *blob,
    TO_ROW *row,
    inT16 *left_above_xht) {
  float baseline;
  float blob_x_centre;
  float left_limit;
  float right_limit;
  float junk;
  TBOX blob_box;

  /* Find baseline of centre of blob */

  blob_box = blob->bounding_box ();
  blob_x_centre = (blob_box.left () + blob_box.right ()) / 2.0;
  baseline = row->baseline.y (blob_x_centre);

  /*
  Find LH limit of blob ABOVE the xht. This is so that we can detect certain
  caps ht chars which should NOT have their box reduced: T, Y, V, W etc
  */
  left_limit = (float) MAX_INT32;
  junk = (float) -MAX_INT32;
  find_cblob_hlimits(blob->cblob(), (baseline + 1.1 * row->xheight),
                     static_cast<float>(MAX_INT16), left_limit, junk);
  if (left_limit > junk)
    *left_above_xht = MAX_INT16; //No area above xht
  else
    *left_above_xht = (inT16) floor (left_limit);
  /*
  Find reduced LH limit of blob - the left extent of the region ABOVE the
  baseline.
  */
  left_limit = (float) MAX_INT32;
  junk = (float) -MAX_INT32;
  find_cblob_hlimits(blob->cblob(), baseline, static_cast<float>(MAX_INT16),
                     left_limit, junk);

  if (left_limit > junk)
    return TBOX ();               //no area within xht so return empty box
  /*
  Find reduced RH limit of blob - the right extent of the region BELOW the xht.
  */
  junk = (float) MAX_INT32;
  right_limit = (float) -MAX_INT32;
  find_cblob_hlimits(blob->cblob(), static_cast<float>(-MAX_INT16),
                     (baseline + row->xheight), junk, right_limit);
  if (junk > right_limit)
    return TBOX ();               //no area within xht so return empty box

  return TBOX (ICOORD ((inT16) floor (left_limit), blob_box.bottom ()),
    ICOORD ((inT16) ceil (right_limit), blob_box.top ()));
}
}  // namespace tesseract
