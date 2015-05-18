/**********************************************************************
 * File:        ocrblock.cpp  (Formerly block.c)
 * Description: BLOCK member functions and iterator functions.
 * Author:		Ray Smith
 * Created:		Fri Mar 15 09:41:28 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include <stdlib.h>
#include "blckerr.h"
#include "ocrblock.h"
#include "stepblob.h"
#include "tprintf.h"

#define BLOCK_LABEL_HEIGHT  150  //char height of block id

ELISTIZE (BLOCK)
/**
 * BLOCK::BLOCK
 *
 * Constructor for a simple rectangular block.
 */
BLOCK::BLOCK(const char *name,                //< filename
             BOOL8 prop,                      //< proportional
             inT16 kern,                      //< kerning
             inT16 space,                     //< spacing
             inT16 xmin,                      //< bottom left
             inT16 ymin, inT16 xmax,          //< top right
             inT16 ymax)
  : PDBLK (xmin, ymin, xmax, ymax),
    filename(name),
    re_rotation_(1.0f, 0.0f),
    classify_rotation_(1.0f, 0.0f),
    skew_(1.0f, 0.0f) {
  ICOORDELT_IT left_it = &leftside;
  ICOORDELT_IT right_it = &rightside;

  proportional = prop;
  right_to_left_ = false;
  kerning = kern;
  spacing = space;
  font_class = -1;               //not assigned
  cell_over_xheight_ = 2.0f;
  hand_poly = NULL;
  left_it.set_to_list (&leftside);
  right_it.set_to_list (&rightside);
                                 //make default box
  left_it.add_to_end (new ICOORDELT (xmin, ymin));
  left_it.add_to_end (new ICOORDELT (xmin, ymax));
  right_it.add_to_end (new ICOORDELT (xmax, ymin));
  right_it.add_to_end (new ICOORDELT (xmax, ymax));
}

/**
 * decreasing_top_order
 *
 * Sort Comparator: Return <0 if row1 top < row2 top
 */

int decreasing_top_order(  //
                         const void *row1,
                         const void *row2) {
  return (*(ROW **) row2)->bounding_box ().top () -
    (*(ROW **) row1)->bounding_box ().top ();
}


/**
 * BLOCK::rotate
 *
 * Rotate the polygon by the given rotation and recompute the bounding_box.
 */
void BLOCK::rotate(const FCOORD& rotation) {
  poly_block()->rotate(rotation);
  box = *poly_block()->bounding_box();
}

/**
 * BLOCK::reflect_polygon_in_y_axis
 *
 * Reflects the polygon in the y-axis and recompute the bounding_box.
 * Does nothing to any contained rows/words/blobs etc.
 */
void BLOCK::reflect_polygon_in_y_axis() {
  poly_block()->reflect_in_y_axis();
  box = *poly_block()->bounding_box();
}

/**
 * BLOCK::sort_rows
 *
 * Order rows so that they are in order of decreasing Y coordinate
 */

void BLOCK::sort_rows() {  // order on "top"
  ROW_IT row_it(&rows);

  row_it.sort (decreasing_top_order);
}


/**
 * BLOCK::compress
 *
 * Delete space between the rows. (And maybe one day, compress the rows)
 * Fill space of block from top down, left aligning rows.
 */

void BLOCK::compress() {  // squash it up
  #define           ROW_SPACING 5

  ROW_IT row_it(&rows);
  ROW *row;
  ICOORD row_spacing (0, ROW_SPACING);

  ICOORDELT_IT icoordelt_it;

  sort_rows();

  box = TBOX (box.topleft (), box.topleft ());
  box.move_bottom_edge (ROW_SPACING);
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    row->move (box.botleft () - row_spacing -
      row->bounding_box ().topleft ());
    box += row->bounding_box ();
  }

  leftside.clear ();
  icoordelt_it.set_to_list (&leftside);
  icoordelt_it.add_to_end (new ICOORDELT (box.left (), box.bottom ()));
  icoordelt_it.add_to_end (new ICOORDELT (box.left (), box.top ()));
  rightside.clear ();
  icoordelt_it.set_to_list (&rightside);
  icoordelt_it.add_to_end (new ICOORDELT (box.right (), box.bottom ()));
  icoordelt_it.add_to_end (new ICOORDELT (box.right (), box.top ()));
}


/**
 * BLOCK::check_pitch
 *
 * Check whether the block is fixed or prop, set the flag, and set
 * the pitch if it is fixed.
 */

void BLOCK::check_pitch() {  // check prop
  //      tprintf("Missing FFT fixed pitch stuff!\n");
  pitch = -1;
}


/**
 * BLOCK::compress
 *
 * Compress and move in a single operation.
 */

void BLOCK::compress(                  // squash it up
                     const ICOORD vec  // and move
                    ) {
  box.move (vec);
  compress();
}


/**
 * BLOCK::print
 *
 * Print the info on a block
 */

void BLOCK::print(            //print list of sides
                  FILE *,     //< file to print on
                  BOOL8 dump  //< print full detail
                 ) {
  ICOORDELT_IT it = &leftside;   //iterator

  box.print ();
  tprintf ("Proportional= %s\n", proportional ? "TRUE" : "FALSE");
  tprintf ("Kerning= %d\n", kerning);
  tprintf ("Spacing= %d\n", spacing);
  tprintf ("Fixed_pitch=%d\n", pitch);
  tprintf ("Filename= %s\n", filename.string ());

  if (dump) {
    tprintf ("Left side coords are:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      tprintf ("(%d,%d) ", it.data ()->x (), it.data ()->y ());
    tprintf ("\n");
    tprintf ("Right side coords are:\n");
    it.set_to_list (&rightside);
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      tprintf ("(%d,%d) ", it.data ()->x (), it.data ()->y ());
    tprintf ("\n");
  }
}

/**
 * BLOCK::operator=
 *
 * Assignment - duplicate the block structure, but with an EMPTY row list.
 */

BLOCK & BLOCK::operator= (       //assignment
const BLOCK & source             //from this
) {
  this->ELIST_LINK::operator= (source);
  this->PDBLK::operator= (source);
  proportional = source.proportional;
  kerning = source.kerning;
  spacing = source.spacing;
  filename = source.filename;    //STRINGs assign ok
  if (!rows.empty ())
    rows.clear ();
  re_rotation_ = source.re_rotation_;
  classify_rotation_ = source.classify_rotation_;
  skew_ = source.skew_;
  return *this;
}

// This function is for finding the approximate (horizontal) distance from
// the x-coordinate of the left edge of a symbol to the left edge of the
// text block which contains it.  We are passed:
//   segments - output of PB_LINE_IT::get_line() which contains x-coordinate
//       intervals for the scan line going through the symbol's y-coordinate.
//       Each element of segments is of the form (x()=start_x, y()=length).
//   x - the x coordinate of the symbol we're interested in.
//   margin - return value, the distance from x,y to the left margin of the
//       block containing it.
// If all segments were to the right of x, we return false and 0.
bool LeftMargin(ICOORDELT_LIST *segments, int x, int *margin) {
  bool found = false;
  *margin = 0;
  if (segments->empty())
    return found;
  ICOORDELT_IT seg_it(segments);
  for (seg_it.mark_cycle_pt(); !seg_it.cycled_list(); seg_it.forward()) {
    int cur_margin = x - seg_it.data()->x();
    if (cur_margin >= 0) {
      if (!found) {
        *margin = cur_margin;
      } else if (cur_margin < *margin) {
        *margin = cur_margin;
      }
      found = true;
    }
  }
  return found;
}

// This function is for finding the approximate (horizontal) distance from
// the x-coordinate of the right edge of a symbol to the right edge of the
// text block which contains it.  We are passed:
//   segments - output of PB_LINE_IT::get_line() which contains x-coordinate
//       intervals for the scan line going through the symbol's y-coordinate.
//       Each element of segments is of the form (x()=start_x, y()=length).
//   x - the x coordinate of the symbol we're interested in.
//   margin - return value, the distance from x,y to the right margin of the
//       block containing it.
// If all segments were to the left of x, we return false and 0.
bool RightMargin(ICOORDELT_LIST *segments, int x, int *margin) {
  bool found = false;
  *margin = 0;
  if (segments->empty())
    return found;
  ICOORDELT_IT seg_it(segments);
  for (seg_it.mark_cycle_pt(); !seg_it.cycled_list(); seg_it.forward()) {
    int cur_margin = seg_it.data()->x() + seg_it.data()->y() - x;
    if (cur_margin >= 0) {
      if (!found) {
        *margin = cur_margin;
      } else if (cur_margin < *margin) {
        *margin = cur_margin;
      }
      found = true;
    }
  }
  return found;
}

// Compute the distance from the left and right ends of each row to the
// left and right edges of the block's polyblock.  Illustration:
//  ____________________________   _______________________
//  |  Howdy neighbor!         |  |rectangular blocks look|
//  |  This text is  written to|  |more like stacked pizza|
//  |illustrate how useful poly-  |boxes.                 |
//  |blobs  are   in -----------  ------   The    polyblob|
//  |dealing    with|     _________     |for a BLOCK  rec-|
//  |harder   layout|   /===========\   |ords the possibly|
//  |issues.        |    |  _    _  |   |skewed    pseudo-|
//  |  You  see this|    | |_| \|_| |   |rectangular      |
//  |text is  flowed|    |      }   |   |boundary     that|
//  |around  a  mid-|     \   ____  |   |forms the  ideal-|
//  |cloumn portrait._____ \       /  __|ized  text margin|
//  |  Polyblobs     exist| \    /   |from which we should|
//  |to account for insets|  |   |   |measure    paragraph|
//  |which make  otherwise|  -----   |indentation.        |
//  -----------------------          ----------------------
//
// If we identify a drop-cap, we measure the left margin for the lines
// below the first line relative to one space past the drop cap.  The
// first line's margin and those past the drop cap area are measured
// relative to the enclosing polyblock.
//
// TODO(rays): Before this will work well, we'll need to adjust the
//             polyblob tighter around the text near images, as in:
//             UNLV_AUTO:mag.3G0  page 2
//             UNLV_AUTO:mag.3G4  page 16
void BLOCK::compute_row_margins() {
  if (row_list()->empty() || row_list()->singleton()) {
    return;
  }

  // If Layout analysis was not called, default to this.
  POLY_BLOCK rect_block(bounding_box(), PT_FLOWING_TEXT);
  POLY_BLOCK *pblock = &rect_block;
  if (poly_block() != NULL) {
    pblock = poly_block();
  }

  // Step One: Determine if there is a drop-cap.
  //           TODO(eger): Fix up drop cap code for RTL languages.
  ROW_IT r_it(row_list());
  ROW *first_row = r_it.data();
  ROW *second_row = r_it.data_relative(1);

  // initialize the bottom of a fictitious drop cap far above the first line.
  int drop_cap_bottom = first_row->bounding_box().top() +
                        first_row->bounding_box().height();
  int drop_cap_right = first_row->bounding_box().left();
  int mid_second_line = second_row->bounding_box().top() -
                        second_row->bounding_box().height() / 2;
  WERD_IT werd_it(r_it.data()->word_list());  // words of line one
  if (!werd_it.empty()) {
    C_BLOB_IT cblob_it(werd_it.data()->cblob_list());
    for (cblob_it.mark_cycle_pt(); !cblob_it.cycled_list();
         cblob_it.forward()) {
      TBOX bbox = cblob_it.data()->bounding_box();
      if (bbox.bottom() <= mid_second_line) {
        // we found a real drop cap
        first_row->set_has_drop_cap(true);
        if (drop_cap_bottom >  bbox.bottom())
          drop_cap_bottom = bbox.bottom();
        if (drop_cap_right < bbox.right())
          drop_cap_right = bbox.right();
      }
    }
  }

  // Step Two: Calculate the margin from the text of each row to the block
  //           (or drop-cap) boundaries.
  PB_LINE_IT lines(pblock);
  r_it.set_to_list(row_list());
  for (r_it.mark_cycle_pt(); !r_it.cycled_list(); r_it.forward()) {
    ROW *row = r_it.data();
    TBOX row_box = row->bounding_box();
    int left_y = row->base_line(row_box.left()) + row->x_height();
    int left_margin;
    ICOORDELT_LIST *segments = lines.get_line(left_y);
    LeftMargin(segments, row_box.left(), &left_margin);
    delete segments;

    if (row_box.top() >= drop_cap_bottom) {
      int drop_cap_distance = row_box.left() - row->space() - drop_cap_right;
      if (drop_cap_distance < 0)
        drop_cap_distance = 0;
      if (drop_cap_distance < left_margin)
        left_margin = drop_cap_distance;
    }

    int right_y = row->base_line(row_box.right()) + row->x_height();
    int right_margin;
    segments = lines.get_line(right_y);
    RightMargin(segments, row_box.right(), &right_margin);
    delete segments;
    row->set_lmargin(left_margin);
    row->set_rmargin(right_margin);
  }
}

/**********************************************************************
 * PrintSegmentationStats
 *
 * Prints segmentation stats for the given block list.
 **********************************************************************/

void PrintSegmentationStats(BLOCK_LIST* block_list) {
  int num_blocks = 0;
  int num_rows = 0;
  int num_words = 0;
  int num_blobs = 0;
  BLOCK_IT block_it(block_list);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK* block = block_it.data();
    ++num_blocks;
    ROW_IT row_it(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      ++num_rows;
      ROW* row = row_it.data();
      // Iterate over all werds in the row.
      WERD_IT werd_it(row->word_list());
      for (werd_it.mark_cycle_pt(); !werd_it.cycled_list(); werd_it.forward()) {
        WERD* werd = werd_it.data();
        ++num_words;
        num_blobs += werd->cblob_list()->length();
      }
    }
  }
  tprintf("Block list stats:\nBlocks = %d\nRows = %d\nWords = %d\nBlobs = %d\n",
          num_blocks, num_rows, num_words, num_blobs);
}

/**********************************************************************
 * ExtractBlobsFromSegmentation
 *
 * Extracts blobs from the given block list and adds them to the output list.
 * The block list must have been created by performing a page segmentation.
 **********************************************************************/

void ExtractBlobsFromSegmentation(BLOCK_LIST* blocks,
                                  C_BLOB_LIST* output_blob_list) {
  C_BLOB_IT return_list_it(output_blob_list);
  BLOCK_IT block_it(blocks);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK* block = block_it.data();
    ROW_IT row_it(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      ROW* row = row_it.data();
      // Iterate over all werds in the row.
      WERD_IT werd_it(row->word_list());
      for (werd_it.mark_cycle_pt(); !werd_it.cycled_list(); werd_it.forward()) {
        WERD* werd = werd_it.data();
        return_list_it.move_to_last();
        return_list_it.add_list_after(werd->cblob_list());
        return_list_it.move_to_last();
        return_list_it.add_list_after(werd->rej_cblob_list());
      }
    }
  }
}

/**********************************************************************
 * RefreshWordBlobsFromNewBlobs()
 *
 * Refreshes the words in the block_list by using blobs in the
 * new_blobs list.
 * Block list must have word segmentation in it.
 * It consumes the blobs provided in the new_blobs list. The blobs leftover in
 * the new_blobs list after the call weren't matched to any blobs of the words
 * in block list.
 * The output not_found_blobs is a list of blobs from the original segmentation
 * in the block_list for which no corresponding new blobs were found.
 **********************************************************************/

void RefreshWordBlobsFromNewBlobs(BLOCK_LIST* block_list,
                                  C_BLOB_LIST* new_blobs,
                                  C_BLOB_LIST* not_found_blobs) {
  // Now iterate over all the blobs in the segmentation_block_list_, and just
  // replace the corresponding c-blobs inside the werds.
  BLOCK_IT block_it(block_list);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK* block = block_it.data();
    if (block->poly_block() != NULL && !block->poly_block()->IsText())
      continue;  // Don't touch non-text blocks.
    // Iterate over all rows in the block.
    ROW_IT row_it(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      ROW* row = row_it.data();
      // Iterate over all werds in the row.
      WERD_IT werd_it(row->word_list());
      WERD_LIST new_words;
      WERD_IT new_words_it(&new_words);
      for (werd_it.mark_cycle_pt(); !werd_it.cycled_list(); werd_it.forward()) {
        WERD* werd = werd_it.extract();
        WERD* new_werd = werd->ConstructWerdWithNewBlobs(new_blobs,
                                                         not_found_blobs);
        if (new_werd) {
          // Insert this new werd into the actual row's werd-list. Remove the
          // existing one.
          new_words_it.add_after_then_move(new_werd);
          delete werd;
        } else {
          // Reinsert the older word back, for lack of better options.
          // This is critical since dropping the words messes up segmentation:
          // eg. 1st word in the row might otherwise have W_FUZZY_NON turned on.
          new_words_it.add_after_then_move(werd);
        }
      }
      // Get rid of the old word list & replace it with the new one.
      row->word_list()->clear();
      werd_it.move_to_first();
      werd_it.add_list_after(&new_words);
    }
  }
}
