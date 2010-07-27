/*****************************************************************************
 *
 * File:        blkocc.cpp  (Formerly blockocc.c)
 * Description:  Block Occupancy routines
 * Author:       Chris Newton
 * Created:      Fri Nov 8
 * Modified:
 * Language:     C++
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1991, Hewlett-Packard Company.
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
 ******************************************************************************/

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include "mfcpch.h"
#include          <ctype.h>
#include          <math.h>
#include          "errcode.h"
#include          "drawtord.h"
#include          "blkocc.h"
#include          "notdll.h"

const ERRCODE BLOCKOCC = "BlockOcc";

ELISTIZE (REGION_OCC)
#define EXTERN
EXTERN BOOL_VAR (blockocc_show_result, FALSE,
"Show intermediate results");

/* The values given here should reflect the values of bln_x_height and
 * bln_baseline_offset. These are defined as part of the word class
 * definition                                                             */

EXTERN INT_VAR (blockocc_desc_height, 0,
"Descender height after normalisation");
EXTERN INT_VAR (blockocc_asc_height, 255,
"Ascender height after normalisation");

EXTERN INT_VAR (blockocc_band_count, 4, "Number of bands used");

EXTERN double_VAR (textord_underline_threshold, 0.5,
"Fraction of width occupied");

// Forward declarations of static functions

//project outlines
static void horizontal_cblob_projection(C_BLOB *blob,  //blob to project
                                 STATS *stats   //output
                                );
static void horizontal_coutline_projection(                     //project outlines
                                    C_OUTLINE *outline,  //outline to project
                                    STATS *stats         //output
                                   );
static void set_bands(                 //init from varibles
               float baseline,  //top of bottom band
               float xheight    //height of split band
              );
                                 //blob to do
static void find_transitions(PBLOB *blob, REGION_OCC_LIST *region_occ_list);
static void record_region(  //add region on list
                   inT16 band,
                   float new_min,
                   float new_max,
                   inT16 region_type,
                   REGION_OCC_LIST *region_occ_list);
static inT16 find_containing_maximal_band(  //find range's band
                                   float y1,
                                   float y2,
                                   BOOL8 *doubly_contained);
static void find_significant_line(POLYPT_IT it, inT16 *band);
static inT16 find_overlapping_minimal_band(  //find range's band
                                    float y1,
                                    float y2);
static inT16 find_region_type(inT16 entry_band,
                       inT16 current_band,
                       inT16 exit_band,
                       float entry_x,
                       float exit_x);
static void find_trans_point(POLYPT_IT *pt_it,
                      inT16 current_band,
                      inT16 next_band,
                      FCOORD *transition_pt);
static void next_region(POLYPT_IT *start_pt_it,
                 inT16 start_band,
                 inT16 *to_band,
                 float *min_x,
                 float *max_x,
                 inT16 *increment,
                 FCOORD *exit_pt);
static inT16 find_band(  // find POINT's band
                float y);
static void compress_region_list(  // join open regions
                          REGION_OCC_LIST *region_occ_list);
static void find_fbox(OUTLINE_IT *out_it,
               float *min_x,
               float *min_y,
               float *max_x,
               float *max_y);
static void maintain_limits(float *min_x, float *max_x, float x);


/**
A note on transitions.

We want to record occupancy in various bands. In general we need to consider
7 situations:

@verbatim
(1)     (2)  (3)             (4)
 \       /   \           /   \           /
__\_____/_____\_________/_____\_________/______ Upper Limit
   \   /       \       /       \       /
   /   \        \-->--/         \--<--/     /-----\
  v     ^                                  /       \(7) 
  \      \                                 \       /
   \      \      /--<--\      /-->--\       \-----/
____\______\____/_______\____/_______\_________ Lower Limit
     \      \  /         \  /         \
             (5)          (6)
@endverbatim

We know that following "next" pointers around an outline keeps the black area
on the LEFT. We only need be concerned with situations 1,2,3,5 and 7.
4 and 6 can be ignored as they represent small incursions into a large black
region which will be recorded elsewhere.  Situations 3 and 5 define encloseed
areas bounded by the upper and lower limits respectively.  Situation 1 is open
to the right, awaiting a closure by a situation 2 which is open to the right.
Situation 7 is entirely enclosed within the band.

The situations are refered to as region types and are determined by
find_region_type.

An empty region type is used to denote entry to an adjacent band and return
to the original band at the same x location.
***********************************************************************/

#define REGION_TYPE_EMPTY 0
#define REGION_TYPE_OPEN_RIGHT 1
#define REGION_TYPE_OPEN_LEFT 2
#define REGION_TYPE_UPPER_BOUND 3
#define REGION_TYPE_UPPER_UNBOUND 4
#define REGION_TYPE_LOWER_BOUND 5
#define REGION_TYPE_LOWER_UNBOUND 6
#define REGION_TYPE_ENCLOSED 7

BAND bands[MAX_NUM_BANDS + 1];   // band defns

/**
 * test_underline
 *
 * Check to see if the blob is an underline.
 * Return TRUE if it is.
 */

BOOL8 test_underline(                   //look for underlines
                     BOOL8 testing_on,  //< drawing blob
                     PBLOB *blob,       //< blob to test
                     float baseline,    //< coords of baseline
                     float xheight      //< height of line
                    ) {
  inT16 occ;
  inT16 blob_width;              //width of blob
  TBOX blob_box;                  //bounding box
  float occs[MAX_NUM_BANDS + 1]; //total occupancy

  blob_box = blob->bounding_box ();
  set_bands(baseline, xheight);  //setup block occ
  blob_width = blob->bounding_box ().width ();
  if (testing_on) {
    //              blob->plot(to_win,GOLDENROD,GOLDENROD);
    //              line_color_index(to_win,GOLDENROD);
    //              move2d(to_win,blob_box.left(),baseline);
    //              draw2d(to_win,blob_box.right(),baseline);
    //              move2d(to_win,blob_box.left(),baseline+xheight);
    //              draw2d(to_win,blob_box.right(),baseline+xheight);
    tprintf
      ("Testing underline on blob at (%d,%d)->(%d,%d), base=%g\nOccs:",
      blob->bounding_box ().left (), blob->bounding_box ().bottom (),
      blob->bounding_box ().right (), blob->bounding_box ().top (),
      baseline);
  }
  block_occ(blob, occs);
  if (testing_on) {
    for (occ = 0; occ <= MAX_NUM_BANDS; occ++)
      tprintf ("%g ", occs[occ]);
    tprintf ("\n");
  }
  if (occs[1] > occs[2] + occs[2] && occs[1] > occs[3] + occs[3]
    && occs[1] > blob_width * textord_underline_threshold)
    return TRUE;                 //real underline
  if (occs[4] > occs[2] + occs[2]
    && occs[4] > blob_width * textord_underline_threshold)
    return TRUE;                 //overline
  return FALSE;                  //neither
}


/**
 * test_underline
 *
 * Check to see if the blob is an underline.
 * Return TRUE if it is.
 */

BOOL8 test_underline(                   //look for underlines
                     BOOL8 testing_on,  //< drawing blob
                     C_BLOB *blob,      //< blob to test
                     inT16 baseline,    //< coords of baseline
                     inT16 xheight      //< height of line
                    ) {
  inT16 occ;
  inT16 blob_width;              //width of blob
  TBOX blob_box;                  //bounding box
  inT32 desc_occ;
  inT32 x_occ;
  inT32 asc_occ;
  STATS projection;

  blob_box = blob->bounding_box ();
  blob_width = blob->bounding_box ().width ();
  projection.set_range (blob_box.bottom (), blob_box.top () + 1);
  if (testing_on) {
    //              blob->plot(to_win,GOLDENROD,GOLDENROD);
    //              line_color_index(to_win,GOLDENROD);
    //              move2d(to_win,blob_box.left(),baseline);
    //              draw2d(to_win,blob_box.right(),baseline);
    //              move2d(to_win,blob_box.left(),baseline+xheight);
    //              draw2d(to_win,blob_box.right(),baseline+xheight);
    tprintf
      ("Testing underline on blob at (%d,%d)->(%d,%d), base=%d\nOccs:",
      blob->bounding_box ().left (), blob->bounding_box ().bottom (),
      blob->bounding_box ().right (), blob->bounding_box ().top (),
      baseline);
  }
  horizontal_cblob_projection(blob, &projection);
  desc_occ = 0;
  for (occ = blob_box.bottom (); occ < baseline; occ++)
    if (occ <= blob_box.top () && projection.pile_count (occ) > desc_occ)
                                 //max in region
      desc_occ = projection.pile_count (occ);
  x_occ = 0;
  for (occ = baseline; occ <= baseline + xheight; occ++)
    if (occ >= blob_box.bottom () && occ <= blob_box.top ()
    && projection.pile_count (occ) > x_occ)
                                 //max in region
      x_occ = projection.pile_count (occ);
  asc_occ = 0;
  for (occ = baseline + xheight + 1; occ <= blob_box.top (); occ++)
    if (occ >= blob_box.bottom () && projection.pile_count (occ) > asc_occ)
      asc_occ = projection.pile_count (occ);
  if (testing_on) {
    tprintf ("%d %d %d\n", desc_occ, x_occ, asc_occ);
  }
  if (desc_occ == 0 && x_occ == 0 && asc_occ == 0) {
    tprintf ("Bottom=%d, top=%d, base=%d, x=%d\n",
      blob_box.bottom (), blob_box.top (), baseline, xheight);
    projection.print (stdout, TRUE);
  }
  if (desc_occ > x_occ + x_occ
    && desc_occ > blob_width * textord_underline_threshold)
    return TRUE;                 //real underline
  if (asc_occ > x_occ + x_occ
    && asc_occ > blob_width * textord_underline_threshold)
    return TRUE;                 //overline
  return FALSE;                  //neither
}


/**
 * horizontal_cblob_projection
 *
 * Compute the horizontal projection of a cblob from its outlines
 * and add to the given STATS.
 */

static void horizontal_cblob_projection(               //project outlines
                                 C_BLOB *blob,  //< blob to project
                                 STATS *stats   //< output
                                ) {
                                 //outlines of blob
  C_OUTLINE_IT out_it = blob->out_list ();

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    horizontal_coutline_projection (out_it.data (), stats);
  }
}


/**
 * horizontal_coutline_projection
 *
 * Compute the horizontal projection of a outline from its outlines
 * and add to the given STATS.
 */

static void horizontal_coutline_projection(                     //project outlines
                                    C_OUTLINE *outline,  //< outline to project
                                    STATS *stats         //< output
                                   ) {
  ICOORD pos;                    //current point
  ICOORD step;                   //edge step
  inT32 length;                  //of outline
  inT16 stepindex;               //current step
  C_OUTLINE_IT out_it = outline->child ();

  pos = outline->start_pos ();
  length = outline->pathlength ();
  for (stepindex = 0; stepindex < length; stepindex++) {
    step = outline->step (stepindex);
    if (step.y () > 0) {
      stats->add (pos.y (), pos.x ());
    }
    else if (step.y () < 0) {
      stats->add (pos.y () - 1, -pos.x ());
    }
    pos += step;
  }

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    horizontal_coutline_projection (out_it.data (), stats);
  }
}


static void set_bands(                 //init from varibles
               float baseline,  //top of bottom band
               float xheight    //height of split band
              ) {
  inT16 int_bl, int_xh;          //for band.set

  bands[DOT_BAND].set (0, 0, 0, 0, 0, 0);

  int_bl = (inT16) baseline;
  int_xh = (inT16) xheight;
  bands[1].set (int_bl, int_bl, int_bl,
    NO_LOWER_LIMIT, NO_LOWER_LIMIT, NO_LOWER_LIMIT);

  bands[2].set (int_bl + int_xh / 2, int_bl + int_xh / 2, int_bl + int_xh / 2,
    int_bl, int_bl, int_bl);

  bands[3].set (int_bl + int_xh, int_bl + int_xh, int_bl + int_xh,
    int_bl + int_xh / 2, int_bl + int_xh / 2,
    int_bl + int_xh / 2);

  bands[4].set (NO_UPPER_LIMIT, NO_UPPER_LIMIT, NO_UPPER_LIMIT,
    int_bl + int_xh, int_bl + int_xh, int_bl + int_xh);
}


void
block_occ (PBLOB * blob,         //blob to do
float occs[]                     //output histogram
) {
  int band_index;                //current band
  REGION_OCC *region;            //current segment
  REGION_OCC_LIST region_occ_list[MAX_NUM_BANDS + 1];
  REGION_OCC_IT region_it;       //region iterator

  find_transitions(blob, region_occ_list);
  compress_region_list(region_occ_list);
  for (band_index = 0; band_index <= MAX_NUM_BANDS; band_index++) {
    occs[band_index] = 0.0f;
    region_it.set_to_list (&region_occ_list[band_index]);
    for (region_it.mark_cycle_pt (); !region_it.cycled_list ();
    region_it.forward ()) {
      region = region_it.data ();
      occs[band_index] += region->max_x - region->min_x;
    }
  }
}


void find_transitions(PBLOB *blob,  //blob to do
                      REGION_OCC_LIST *region_occ_list) {
  OUTLINE_IT outline_it;
  TBOX box;
  POLYPT_IT pt_it;
  FCOORD point1;
  FCOORD point2;
  FCOORD *entry_pt = &point1;
  FCOORD *exit_pt = &point2;
  FCOORD *temp_pt;
  inT16 increment;
  inT16 prev_band;
  inT16 band;
  inT16 next_band;
  float min_x;
  float max_x;
  float min_y;
  float max_y;
  BOOL8 doubly_contained;

  outline_it = blob->out_list ();
  for (outline_it.mark_cycle_pt (); !outline_it.cycled_list ();
  outline_it.forward ()) {
    find_fbox(&outline_it, &min_x, &min_y, &max_x, &max_y);

    if (bands[DOT_BAND].range_in_nominal (max_y, min_y)) {
      record_region(DOT_BAND,
                    min_x,
                    max_x,
                    REGION_TYPE_ENCLOSED,
                    region_occ_list);
    }
    else {
      band = find_containing_maximal_band (max_y, min_y,
        &doubly_contained);
      if (band != UNDEFINED_BAND) {
                                 //No transitions
        if (!doubly_contained)
          record_region(band,
                        min_x,
                        max_x,
                        REGION_TYPE_ENCLOSED,
                        region_occ_list);
        else {
          //                                      if (wordocc_debug_on && blockocc_show_result)
          //                                      {
          //                                              fprintf( db_win,
          //                                                "Ignoring doubly contained outline (%d, %d) (%d, %d)\n",
          //                                                box.left(), box.top(),
          //                                                box.right(), box.bottom());
          //                                              fprintf( db_win, "\tContained in bands %d and %d\n",
          //                                                                      band, band + 1 );
          //                                      }
        }
      }
      else {
                                 //There are transitns
        /*
        Determining a good start point for recognising transitions between bands
        is complicated by error limits on bands.  We need to find a line which
        significantly occupies a band.

        Having found such a point, we need to find a significant transition out of
        its band and start the walk around the outline from there.

        Note that we are relying on having recognised and dealt with elsewhere,
        outlines which do not significantly occupy more than one region. A
        particularly nasty case of this are outlines which do not significantly
        occupy ANY band. I.e. they lie entirely within the error limits.
        Given this condition, all remaining outlines must contain at least one
        significant line.  */

        pt_it = outline_it.data ()->polypts ();

        find_significant_line(pt_it, &band);
        *entry_pt = pt_it.data ()->pos;
        next_region(&pt_it,
                    band,
                    &next_band,
                    &min_x,
                    &max_x,
                    &increment,
                    exit_pt);
        pt_it.mark_cycle_pt ();

        // Found the first real transition, so start walking the outline from here.

        do {
          prev_band = band;
          band = band + increment;

          while (band != next_band) {
            temp_pt = entry_pt;
            entry_pt = exit_pt;
            exit_pt = temp_pt;
            min_x = max_x = entry_pt->x ();

            find_trans_point (&pt_it, band, band + increment,
              exit_pt);
            maintain_limits (&min_x, &max_x, exit_pt->x ());

            record_region (band,
              min_x,
              max_x,
              find_region_type (prev_band,
              band,
              band + increment,
              entry_pt->x (),
              exit_pt->x ()),
              region_occ_list);
            prev_band = band;
            band = band + increment;
          }

          temp_pt = entry_pt;
          entry_pt = exit_pt;
          exit_pt = temp_pt;
          min_x = max_x = entry_pt->x ();
          next_region(&pt_it,
                      band,
                      &next_band,
                      &min_x,
                      &max_x,
                      &increment,
                      exit_pt);

          record_region (band,
            min_x,
            max_x,
            find_region_type (prev_band,
            band,
            band + increment,
            entry_pt->x (),
            exit_pt->x ()),
            region_occ_list);
        }
        while (!pt_it.cycled_list ());
      }
    }
  }
}


static void record_region(  //add region on list
                   inT16 band,
                   float new_min,
                   float new_max,
                   inT16 region_type,
                   REGION_OCC_LIST *region_occ_list) {
  REGION_OCC_IT it (&(region_occ_list[band]));

  //   if (wordocc_debug_on && blockocc_show_result)
  //         fprintf( db_win, "\nBand %d, region type %d, from %f to %f",
  //                                 band, region_type, new_min, new_max );

  if ((region_type == REGION_TYPE_UPPER_UNBOUND) ||
    (region_type == REGION_TYPE_LOWER_UNBOUND) ||
    (region_type == REGION_TYPE_EMPTY))
    return;

  if (it.empty ()) {
    it.add_after_stay_put (new REGION_OCC (new_min, new_max, region_type));
  }
  else {

    /* Insert in sorted order of average limit */

    while ((new_min + new_max > it.data ()->min_x + it.data ()->max_x) &&
      (!it.at_last ()))
      it.forward ();

    if ((it.at_last ()) &&       //at the end
      (new_min + new_max > it.data ()->min_x + it.data ()->max_x))
      //new range > current
      it.add_after_stay_put (new REGION_OCC (new_min,
        new_max, region_type));
    else {
      it.add_before_stay_put (new REGION_OCC (new_min,
        new_max, region_type));
    }
  }
}


static inT16 find_containing_maximal_band(  //find range's band
                                   float y1,
                                   float y2,
                                   BOOL8 *doubly_contained) {
  inT16 band;

  *doubly_contained = FALSE;

  for (band = 1; band <= blockocc_band_count; band++) {
    if (bands[band].range_in_maximal (y1, y2)) {
      if ((band < blockocc_band_count) &&
        (bands[band + 1].range_in_maximal (y1, y2)))
        *doubly_contained = TRUE;
      return band;
    }
  }
  return UNDEFINED_BAND;
}


static void find_significant_line(POLYPT_IT it, inT16 *band) {

  /* Look for a line which significantly occupies at least one band. I.e. part
  of the line is in the non-margin part of the band. */

  *band = find_overlapping_minimal_band (it.data ()->pos.y (),
    it.data ()->pos.y () +
    it.data ()->vec.y ());

  while (*band == UNDEFINED_BAND) {
    it.forward ();
    *band = find_overlapping_minimal_band (it.data ()->pos.y (),
      it.data ()->pos.y () +
      it.data ()->vec.y ());
  }
}


static inT16 find_overlapping_minimal_band(  //find range's band
                                    float y1,
                                    float y2) {
  inT16 band;

  for (band = 1; band <= blockocc_band_count; band++) {
    if (bands[band].range_overlaps_minimal (y1, y2))
      return band;
  }
  return UNDEFINED_BAND;
}


static inT16 find_region_type(inT16 entry_band,
                       inT16 current_band,
                       inT16 exit_band,
                       float entry_x,
                       float exit_x) {
  if (entry_band > exit_band)
    return REGION_TYPE_OPEN_RIGHT;
  if (entry_band < exit_band)
    return REGION_TYPE_OPEN_LEFT;
  if (entry_x == exit_x)
    return REGION_TYPE_EMPTY;
  if (entry_band > current_band) {
    if (entry_x < exit_x)
      return REGION_TYPE_UPPER_BOUND;
    else
      return REGION_TYPE_UPPER_UNBOUND;
  }
  else {
    if (entry_x > exit_x)
      return REGION_TYPE_LOWER_BOUND;
    else
      return REGION_TYPE_LOWER_UNBOUND;
  }
}


static void find_trans_point(POLYPT_IT *pt_it,
                      inT16 current_band,
                      inT16 next_band,
                      FCOORD *transition_pt) {
  float x1, x2, y1, y2;          // points of edge
  float gradient;                // m in y = mx + c
  float offset;                  // c in y = mx + c

  if (current_band < next_band)
    transition_pt->set_y (bands[current_band].max);
  //going up
  else
    transition_pt->set_y (bands[current_band].min);
  //going down

  x1 = pt_it->data ()->pos.x ();
  x2 = x1 + pt_it->data ()->vec.x ();
  y1 = pt_it->data ()->pos.y ();
  y2 = y1 + pt_it->data ()->vec.y ();

  if (x1 == x2)
    transition_pt->set_x (x1);   //avoid div by 0
  else {
    if (y1 == y2)                //avoid div by 0
      transition_pt->set_x ((x1 + x2) / 2.0);
    else {
      gradient = (y1 - y2) / (float) (x1 - x2);
      offset = y1 - x1 * gradient;
      transition_pt->set_x ((transition_pt->y () - offset) / gradient);
    }
  }
}


static void next_region(POLYPT_IT *start_pt_it,
                 inT16 start_band,
                 inT16 *to_band,
                 float *min_x,
                 float *max_x,
                 inT16 *increment,
                 FCOORD *exit_pt) {
  /*
  Given an edge and a band which the edge significantly occupies, find the
  significant end of the region containing the band. I.e. find an edge which
  points to another band such that the outline subsequetly moves significantly
  out of the starting band.

  Note that we can assume that we are significantly inside the current band to
  start with because the edges passed will be from previous calls to this
  routine apart from the first - the result of which is only used to establish
  the start of the first region.
  */

  inT16 band;                    //band of current edge
  inT16 prev_band = start_band;  //band of prev edge
                                 //edge crossing out
  POLYPT_IT last_transition_out_it;
                                 //band it pts to
  inT16 last_trans_out_to_band = 0;
  float ext_min_x = 0.0f;
  float ext_max_x = 0.0f;

  start_pt_it->forward ();
  band = find_band (start_pt_it->data ()->pos.y ());

  while ((band == start_band) ||
  bands[start_band].in_maximal (start_pt_it->data ()->pos.y ())) {
    if (band == start_band) {
                                 //Return to start band
      if (prev_band != start_band) {
        *min_x = ext_min_x;
        *max_x = ext_max_x;
      }
      maintain_limits (min_x, max_x, start_pt_it->data ()->pos.x ());
    }
    else {
      if (prev_band == start_band) {
                                 //Exit from start band
                                 //so remember edge
        last_transition_out_it = *start_pt_it;
                                 //before we left
        last_transition_out_it.backward ();
                                 //and band it pts to
        last_trans_out_to_band = band;
        ext_min_x = *min_x;
        ext_max_x = *max_x;
      }
      maintain_limits (&ext_min_x, &ext_max_x,
        start_pt_it->data ()->pos.x ());
    }
    prev_band = band;
    start_pt_it->forward ();
    band = find_band (start_pt_it->data ()->pos.y ());
  }

  if (prev_band == start_band) { //exit from start band
    *to_band = band;
                                 //so remember edge
    last_transition_out_it = *start_pt_it;
                                 //before we left
    last_transition_out_it.backward ();
  }
  else {
    *to_band = last_trans_out_to_band;
  }

  if (*to_band > start_band)
    *increment = 1;
  else
    *increment = -1;

  find_trans_point (&last_transition_out_it, start_band,
    start_band + *increment, exit_pt);
  maintain_limits (min_x, max_x, exit_pt->x ());
  *start_pt_it = last_transition_out_it;
}


static inT16 find_band(  // find POINT's band
                float y) {
  inT16 band;

  for (band = 1; band <= blockocc_band_count; band++) {
    if (bands[band].in_nominal (y))
      return band;
  }
  BLOCKOCC.error ("find_band", ABORT, "Cant find band for %d", y);
  return 0;
}


static void compress_region_list(  // join open regions
                          REGION_OCC_LIST *region_occ_list) {
  REGION_OCC_IT it (&(region_occ_list[0]));
  REGION_OCC *open_right = NULL;

  inT16 i = 0;

  for (i = 0; i <= blockocc_band_count; i++) {
    it.set_to_list (&(region_occ_list[i]));
    if (!it.empty ()) {
      /* First check for left right pairs. Merge them into the open right and delete
      the open left. */
      open_right = NULL;
      for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
        switch (it.data ()->region_type) {
          case REGION_TYPE_OPEN_RIGHT:
          {
            if (open_right != NULL)
              BLOCKOCC.error ("compress_region_list", ABORT,
                "unmatched right");
            else
              open_right = it.data ();
            break;
          }
          case REGION_TYPE_OPEN_LEFT:
          {
            if (open_right == NULL)
              BLOCKOCC.error ("compress_region_list", ABORT,
                "unmatched left");
            else {
              open_right->max_x = it.data ()->max_x;
              open_right = NULL;
              delete it.extract ();
            }
            break;
          }
          default:
            break;
        }
      }
      if (open_right != NULL)
        BLOCKOCC.error ("compress_region_list", ABORT,
          "unmatched right remaining");

      /* Now cycle the list again, merging and deleting any redundant regions */
      it.move_to_first ();
      open_right = it.data ();
      while (!it.at_last ()) {
        it.forward ();
        if (it.data ()->min_x <= open_right->max_x) {
        // Overlaps
          if (it.data ()->max_x > open_right->max_x)
            open_right->max_x = it.data ()->max_x;
          // Extend
          delete it.extract ();
        }
        else
          open_right = it.data ();
      }
    }
  }
}


static void find_fbox(OUTLINE_IT *out_it,
               float *min_x,
               float *min_y,
               float *max_x,
               float *max_y) {
  POLYPT_IT pt_it = out_it->data ()->polypts ();
  FCOORD pt;
  *min_x = 9999.0f;
  *min_y = 9999.0f;
  *max_x = 0.0f;
  *max_y = 0.0f;

  for (pt_it.mark_cycle_pt (); !pt_it.cycled_list (); pt_it.forward ()) {
    pt = pt_it.data ()->pos;
    maintain_limits (min_x, max_x, pt.x ());
    maintain_limits (min_y, max_y, pt.y ());
  }
}


static void maintain_limits(float *min_x, float *max_x, float x) {
  if (x > *max_x)
    *max_x = x;
  if (x < *min_x)
    *min_x = x;
}
