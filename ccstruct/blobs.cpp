/* -*-C-*-
 ********************************************************************************
 *
 * File:        blobs.c  (Formerly blobs.c)
 * Description:  Blob definition
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 27 15:39:52 1989
 * Modified:     Thu Mar 28 15:33:26 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "mfcpch.h"
#include "blobs.h"
#include "ccstruct.h"
#include "cutil.h"
#include "emalloc.h"
#include "helpers.h"
#include "ndminx.h"
#include "normalis.h"
#include "ocrblock.h"
#include "ocrrow.h"
#include "points.h"
#include "polyaprx.h"
#include "structures.h"
#include "werd.h"

using tesseract::CCStruct;

// A Vector representing the "vertical" direction when measuring the
// divisiblity of blobs into multiple blobs just by separating outlines.
// See divisible_blob below for the use.
const TPOINT kDivisibleVerticalUpright = {0, 1};
// A vector representing the "vertical" direction for italic text for use
// when separating outlines. Using it actually deteriorates final accuracy,
// so it is only used for ApplyBoxes chopping to get a better segmentation.
const TPOINT kDivisibleVerticalItalic = {1, 5};

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
// Consume the circular list of EDGEPTs to make a TESSLINE.
TESSLINE* TESSLINE::BuildFromOutlineList(EDGEPT* outline) {
  TESSLINE* result = new TESSLINE;
  result->loop = outline;
  result->SetupFromPos();
  return result;
}

// Copies the data and the outline, but leaves next untouched.
void TESSLINE::CopyFrom(const TESSLINE& src) {
  Clear();
  topleft = src.topleft;
  botright = src.botright;
  start = src.start;
  is_hole = src.is_hole;
  if (src.loop != NULL) {
    EDGEPT* prevpt = NULL;
    EDGEPT* newpt = NULL;
    EDGEPT* srcpt = src.loop;
    do {
      newpt = new EDGEPT(*srcpt);
      if (prevpt == NULL) {
        loop = newpt;
      } else {
        newpt->prev = prevpt;
        prevpt->next = newpt;
      }
      prevpt = newpt;
      srcpt = srcpt->next;
    } while (srcpt != src.loop);
    loop->prev = newpt;
    newpt->next = loop;
  }
}

// Deletes owned data.
void TESSLINE::Clear() {
  if (loop == NULL)
    return;

  EDGEPT* this_edge = loop;
  do {
    EDGEPT* next_edge = this_edge->next;
    delete this_edge;
    this_edge = next_edge;
  } while (this_edge != loop);
  loop = NULL;
}

// Normalize in-place using the DENORM.
void TESSLINE::Normalize(const DENORM& denorm) {
  EDGEPT* pt = loop;
  do {
    denorm.LocalNormTransform(pt->pos, &pt->pos);
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Rotates by the given rotation in place.
void TESSLINE::Rotate(const FCOORD rot) {
  EDGEPT* pt = loop;
  do {
    int tmp = static_cast<int>(floor(pt->pos.x * rot.x() -
                                     pt->pos.y * rot.y() + 0.5));
    pt->pos.y = static_cast<int>(floor(pt->pos.y * rot.x() +
                                       pt->pos.x * rot.y() + 0.5));
    pt->pos.x = tmp;
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Moves by the given vec in place.
void TESSLINE::Move(const ICOORD vec) {
  EDGEPT* pt = loop;
  do {
    pt->pos.x += vec.x();
    pt->pos.y += vec.y();
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Scales by the given factor in place.
void TESSLINE::Scale(float factor) {
  EDGEPT* pt = loop;
  do {
    pt->pos.x = static_cast<int>(floor(pt->pos.x * factor + 0.5));
    pt->pos.y = static_cast<int>(floor(pt->pos.y * factor + 0.5));
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Sets up the start and vec members of the loop from the pos members.
void TESSLINE::SetupFromPos() {
  EDGEPT* pt = loop;
  do {
    pt->vec.x = pt->next->pos.x - pt->pos.x;
    pt->vec.y = pt->next->pos.y - pt->pos.y;
    pt = pt->next;
  } while (pt != loop);
  start = pt->pos;
  ComputeBoundingBox();
}

// Recomputes the bounding box from the points in the loop.
void TESSLINE::ComputeBoundingBox() {
  int minx = MAX_INT32;
  int miny = MAX_INT32;
  int maxx = -MAX_INT32;
  int maxy = -MAX_INT32;

  // Find boundaries.
  start = loop->pos;
  EDGEPT* this_edge = loop;
  do {
    if (!this_edge->IsHidden() || !this_edge->prev->IsHidden()) {
      if (this_edge->pos.x < minx)
        minx = this_edge->pos.x;
      if (this_edge->pos.y < miny)
        miny = this_edge->pos.y;
      if (this_edge->pos.x > maxx)
        maxx = this_edge->pos.x;
      if (this_edge->pos.y > maxy)
        maxy = this_edge->pos.y;
    }
    this_edge = this_edge->next;
  } while (this_edge != loop);
  // Reset bounds.
  topleft.x = minx;
  topleft.y = maxy;
  botright.x = maxx;
  botright.y = miny;
}

// Computes the min and max cross product of the outline points with the
// given vec and returns the results in min_xp and max_xp. Geometrically
// this is the left and right edge of the outline perpendicular to the
// given direction, but to get the distance units correct, you would
// have to divide by the modulus of vec.
void TESSLINE::MinMaxCrossProduct(const TPOINT vec,
                                  int* min_xp, int* max_xp) const {
  *min_xp = MAX_INT32;
  *max_xp = MIN_INT32;
  EDGEPT* this_edge = loop;
  do {
    if (!this_edge->IsHidden() || !this_edge->prev->IsHidden()) {
      int product = CROSS(this_edge->pos, vec);
      UpdateRange(product, min_xp, max_xp);
    }
    this_edge = this_edge->next;
  } while (this_edge != loop);
}

TBOX TESSLINE::bounding_box() const {
  return TBOX(topleft.x, botright.y, botright.x, topleft.y);
}

void TESSLINE::plot(ScrollView* window, ScrollView::Color color,
                    ScrollView::Color child_color) {
  if (is_hole)
    window->Pen(child_color);
  else
    window->Pen(color);
  window->SetCursor(start.x, start.y);
  EDGEPT* pt = loop;
  do {
    bool prev_hidden = pt->IsHidden();
    pt = pt->next;
    if (prev_hidden)
      window->SetCursor(pt->pos.x, pt->pos.y);
    else
      window->DrawTo(pt->pos.x, pt->pos.y);
  } while (pt != loop);
}

// Iterate the given list of outlines, converting to TESSLINE by polygonal
// approximation and recursively any children, returning the current tail
// of the resulting list of TESSLINEs.
static TESSLINE** ApproximateOutlineList(C_OUTLINE_LIST* outlines,
                                         bool children,
                                         TESSLINE** tail) {
  C_OUTLINE_IT ol_it(outlines);
  for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
    C_OUTLINE* outline = ol_it.data();
    TESSLINE* tessline = ApproximateOutline(outline);
    tessline->is_hole = children;
    *tail = tessline;
    tail = &tessline->next;
    if (!outline->child()->empty()) {
      tail = ApproximateOutlineList(outline->child(), true, tail);
    }
  }
  return tail;
}

// Factory to build a TBLOB from a C_BLOB with polygonal
// approximation along the way.
TBLOB* TBLOB::PolygonalCopy(C_BLOB* src) {
  C_OUTLINE_IT ol_it = src->out_list();
  TBLOB* tblob = new TBLOB;
  ApproximateOutlineList(src->out_list(), false, &tblob->outlines);
  return tblob;
}

// Copies the data and the outline, but leaves next untouched.
void TBLOB::CopyFrom(const TBLOB& src) {
  Clear();
  TESSLINE* prev_outline = NULL;
  for (TESSLINE* srcline = src.outlines; srcline != NULL;
       srcline = srcline->next) {
    TESSLINE* new_outline = new TESSLINE(*srcline);
    if (outlines == NULL)
      outlines = new_outline;
    else
      prev_outline->next = new_outline;
    prev_outline = new_outline;
  }
}

// Deletes owned data.
void TBLOB::Clear() {
  for (TESSLINE* next_outline = NULL; outlines != NULL;
       outlines = next_outline) {
    next_outline = outlines->next;
    delete outlines;
  }
}
// Normalize in-place using the DENORM.
void TBLOB::Normalize(const DENORM& denorm) {
  // TODO(rays) outline->Normalize is more accurate, but breaks tests due
  // the changes it makes. Reinstate this code with a retraining.
#if 0
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next) {
    outline->Normalize(denorm);
  }
#else
  denorm.LocalNormBlob(this);
#endif
}

// Rotates by the given rotation in place.
void TBLOB::Rotate(const FCOORD rotation) {
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next) {
    outline->Rotate(rotation);
  }
}

// Moves by the given vec in place.
void TBLOB::Move(const ICOORD vec) {
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next) {
    outline->Move(vec);
  }
}

// Scales by the given factor in place.
void TBLOB::Scale(float factor) {
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next) {
    outline->Scale(factor);
  }
}

// Recomputes the bounding boxes of the outlines.
void TBLOB::ComputeBoundingBoxes() {
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next) {
    outline->ComputeBoundingBox();
  }
}

// Returns the number of outlines.
int TBLOB::NumOutlines() const {
  int result = 0;
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next)
    ++result;
  return result;
}

TBOX TBLOB::bounding_box() const {
  TPOINT topleft;
  TPOINT botright;
  blob_bounding_box(this, &topleft, &botright);
  TBOX box(topleft.x, botright.y, botright.x, topleft.y);
  return box;
}

void TBLOB::plot(ScrollView* window, ScrollView::Color color,
                 ScrollView::Color child_color) {
  for (TESSLINE* outline = outlines; outline != NULL; outline = outline->next)
    outline->plot(window, color, child_color);
}

// Factory to build a TWERD from a (C_BLOB) WERD, with polygonal
// approximation along the way.
TWERD* TWERD::PolygonalCopy(WERD* src) {
  TWERD* tessword = new TWERD;
  tessword->latin_script = src->flag(W_SCRIPT_IS_LATIN);
  C_BLOB_IT b_it(src->cblob_list());
  TBLOB *tail = NULL;
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    C_BLOB* blob = b_it.data();
    TBLOB* tblob = TBLOB::PolygonalCopy(blob);
    if (tail == NULL) {
      tessword->blobs = tblob;
    } else {
      tail->next = tblob;
    }
    tail = tblob;
  }
  return tessword;
}

// Normalize in-place and record the normalization in the DENORM.
void TWERD::SetupBLNormalize(const BLOCK* block, const ROW* row,
                             float x_height, bool numeric_mode,
                             DENORM* denorm) const {
  int num_segments = 0;
  DENORM_SEG* segs = NULL;
  if (numeric_mode) {
    segs = new DENORM_SEG[NumBlobs()];
    for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
      TBOX blob_box = blob->bounding_box();
      float factor = kBlnXHeight / x_height;
      factor = ClipToRange(kBlnXHeight * 4.0f / (3 * blob_box.height()),
                           factor, factor * 1.5f);
      segs[num_segments].xstart = blob_box.left();
      segs[num_segments].ycoord = blob_box.bottom();
      segs[num_segments++].scale_factor = factor;
    }
  }
  denorm->SetupBLNormalize(block, row, x_height, bounding_box(),
                           num_segments, segs);
  delete [] segs;
}

// Normalize in-place using the DENORM.
void TWERD::Normalize(const DENORM& denorm) {
  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
    blob->Normalize(denorm);
  }
}

// Copies the data and the blobs, but leaves next untouched.
void TWERD::CopyFrom(const TWERD& src) {
  Clear();
  latin_script = src.latin_script;
  TBLOB* prev_blob = NULL;
  for (TBLOB* srcblob = src.blobs; srcblob != NULL; srcblob = srcblob->next) {
    TBLOB* new_blob = new TBLOB(*srcblob);
    if (blobs == NULL)
      blobs = new_blob;
    else
      prev_blob->next = new_blob;
    prev_blob = new_blob;
  }
}

// Deletes owned data.
void TWERD::Clear() {
  for (TBLOB* next_blob = NULL; blobs != NULL; blobs = next_blob) {
    next_blob = blobs->next;
    delete blobs;
  }
}

// Recomputes the bounding boxes of the blobs.
void TWERD::ComputeBoundingBoxes() {
  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
    blob->ComputeBoundingBoxes();
  }
}

TBOX TWERD::bounding_box() const {
  TBOX result;
  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
    TBOX box = blob->bounding_box();
    result += box;
  }
  return result;
}

// Merges the blobs from start to end, not including end, and deletes
// the blobs between start and end.
void TWERD::MergeBlobs(int start, int end) {
  TBLOB* blob = blobs;
  for (int i = 0; i < start && blob != NULL; ++i)
    blob = blob->next;
  if (blob == NULL || blob->next == NULL)
    return;
  TBLOB* next_blob = blob->next;
  TESSLINE* outline = blob->outlines;
  for (int i = start + 1; i < end && next_blob != NULL; ++i) {
    // Take the outlines from the next blob.
    if (outline == NULL) {
      blob->outlines = next_blob->outlines;
      outline = blob->outlines;
    } else {
      while (outline->next != NULL)
        outline = outline->next;
      outline->next = next_blob->outlines;
      next_blob->outlines = NULL;
    }
    // Delete the next blob and move on.
    TBLOB* dead_blob = next_blob;
    next_blob = next_blob->next;
    blob->next = next_blob;
    delete dead_blob;
  }
}

void TWERD::plot(ScrollView* window) {
  ScrollView::Color color = WERD::NextColor(ScrollView::BLACK);
  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
    blob->plot(window, color, ScrollView::BROWN);
    color = WERD::NextColor(color);
  }
}

/**********************************************************************
 * blob_origin
 *
 * Compute the origin of a compound blob, define to be the centre
 * of the bounding box.
 **********************************************************************/
void blob_origin(TBLOB *blob,       /*blob to compute on */
                 TPOINT *origin) {  /*return value */
  TPOINT topleft;                /*bounding box */
  TPOINT botright;

                                 /*find bounding box */
  blob_bounding_box(blob, &topleft, &botright); 
                                 /*centre of box */
  origin->x = (topleft.x + botright.x) / 2;
  origin->y = (topleft.y + botright.y) / 2;
}


/**********************************************************************
 * blob_bounding_box
 *
 * Compute the bounding_box of a compound blob, define to be the
 * max coordinate value of the bounding boxes of all the top-level
 * outlines in the box.
 **********************************************************************/
void blob_bounding_box(const TBLOB *blob,         // blob to compute on.
                       TPOINT *topleft,           // bounding box.
                       TPOINT *botright) {
  register TESSLINE *outline;    // Current outline.

  if (blob == NULL || blob->outlines == NULL) {
    topleft->x = topleft->y = 0;
    *botright = *topleft;        // Default value.
  } else {
    outline = blob->outlines;
    *topleft = outline->topleft;
    *botright = outline->botright;
    for (outline = outline->next; outline != NULL; outline = outline->next) {
      UpdateRange(outline->topleft.x, outline->botright.x,
                  &topleft->x, &botright->x);
      UpdateRange(outline->botright.y, outline->topleft.y,
                  &botright->y, &topleft->y);
    }
  }
}


/**********************************************************************
 * blobs_bounding_box
 *
 * Return the smallest extreme point that contain this word.
 **********************************************************************/
void blobs_bounding_box(TBLOB *blobs, TPOINT *topleft, TPOINT *botright) { 
  TPOINT tl;
  TPOINT br;
  /* Start with first blob */
  blob_bounding_box(blobs, topleft, botright); 

  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) { 
    blob_bounding_box(blob, &tl, &br); 

    if (tl.x < topleft->x)
      topleft->x = tl.x;
    if (tl.y > topleft->y)
      topleft->y = tl.y;
    if (br.x > botright->x)
      botright->x = br.x;
    if (br.y < botright->y)
      botright->y = br.y;
  }
}


/**********************************************************************
 * blobs_origin
 *
 * Compute the origin of a compound blob, define to be the centre
 * of the bounding box.
 **********************************************************************/
void blobs_origin(TBLOB *blobs,      /*blob to compute on */
                  TPOINT *origin) {  /*return value */
  TPOINT topleft;                /*bounding box */
  TPOINT botright;

                                 /*find bounding box */
  blobs_bounding_box(blobs, &topleft, &botright); 
                                 /*center of box */
  origin->x = (topleft.x + botright.x) / 2;
  origin->y = (topleft.y + botright.y) / 2;
}


/**********************************************************************
 * blobs_widths
 *
 * Compute the widths of a list of blobs. Return an array of the widths
 * and gaps.
 **********************************************************************/
WIDTH_RECORD *blobs_widths(TBLOB *blobs) {  /*blob to compute on */
  WIDTH_RECORD *width_record;
  TPOINT topleft;                /*bounding box */
  TPOINT botright;
  int i = 0;
  int blob_end;
  int num_blobs = count_blobs (blobs);

  /* Get memory */
  width_record = (WIDTH_RECORD *) memalloc (sizeof (int) * num_blobs * 2);
  width_record->num_chars = num_blobs;

  blob_bounding_box(blobs, &topleft, &botright); 
  width_record->widths[i++] = botright.x - topleft.x;
  /* First width */
  blob_end = botright.x;

  for (TBLOB* blob = blobs->next; blob != NULL; blob = blob->next) {
    blob_bounding_box(blob, &topleft, &botright); 
    width_record->widths[i++] = topleft.x - blob_end;
    width_record->widths[i++] = botright.x - topleft.x;
    blob_end = botright.x;
  }
  return (width_record);
}


/**********************************************************************
 * count_blobs
 *
 * Return a count of the number of blobs attached to this one.
 **********************************************************************/
int count_blobs(TBLOB *blobs) { 
  int x = 0;

  for (TBLOB* b = blobs; b != NULL; b = b->next)
    x++;
  return x;
}

/**********************************************************************
 * divisible_blob
 *
 * Returns true if the blob contains multiple outlines than can be
 * separated using divide_blobs. Sets the location to be used in the
 * call to divide_blobs.
 **********************************************************************/
bool divisible_blob(TBLOB *blob, bool italic_blob, TPOINT* location) {
  if (blob->outlines == NULL || blob->outlines->next == NULL)
    return false;  // Need at least 2 outlines for it to be possible.
  int max_gap = 0;
  TPOINT vertical = italic_blob ? kDivisibleVerticalItalic
                                : kDivisibleVerticalUpright;
  for (TESSLINE* outline1 = blob->outlines; outline1 != NULL;
       outline1 = outline1->next) {
    if (outline1->is_hole)
      continue;  // Holes do not count as separable.
    TPOINT mid_pt1 = {(outline1->topleft.x + outline1->botright.x) / 2,
                      (outline1->topleft.y + outline1->botright.y) / 2};
    int mid_prod1 = CROSS(mid_pt1, vertical);
    int min_prod1, max_prod1;
    outline1->MinMaxCrossProduct(vertical, &min_prod1, &max_prod1);
    for (TESSLINE* outline2 = outline1->next; outline2 != NULL;
         outline2 = outline2->next) {
      if (outline2->is_hole)
        continue;  // Holes do not count as separable.
      TPOINT mid_pt2 = {  (outline2->topleft.x + outline2->botright.x) / 2,
                        (outline2->topleft.y + outline2->botright.y) / 2};
      int mid_prod2 = CROSS(mid_pt2, vertical);
      int min_prod2, max_prod2;
      outline2->MinMaxCrossProduct(vertical, &min_prod2, &max_prod2);
      int mid_gap = abs(mid_prod2 - mid_prod1);
      int overlap = MIN(max_prod1, max_prod2) - MAX(min_prod1, min_prod2);
      if (mid_gap - overlap / 2 > max_gap) {
        max_gap = mid_gap - overlap / 2;
        *location = mid_pt1;
        *location += mid_pt2;
        *location /= 2;
      }
    }
  }
  // Use the y component of the vertical vector as an approximation to its
  // length.
  return max_gap > vertical.y;
}

/**********************************************************************
 * divide_blobs
 *
 * Create two blobs by grouping the outlines in the appropriate blob.
 * The outlines that are beyond the location point are moved to the
 * other blob.  The ones whose x location is less than that point are
 * retained in the original blob.
 **********************************************************************/
void divide_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                  const TPOINT& location) {
  TPOINT vertical = italic_blob ? kDivisibleVerticalItalic
                                : kDivisibleVerticalUpright;
  TESSLINE *outline1 = NULL;
  TESSLINE *outline2 = NULL;

  TESSLINE *outline = blob->outlines;
  blob->outlines = NULL;
  int location_prod = CROSS(location, vertical);

  while (outline != NULL) {
    TPOINT mid_pt = {(outline->topleft.x + outline->botright.x) / 2,
                     (outline->topleft.y + outline->botright.y) / 2};
    int mid_prod = CROSS(mid_pt, vertical);
    if (mid_prod < location_prod) {
      // Outline is in left blob.
      if (outline1)
        outline1->next = outline;
      else
        blob->outlines = outline;
      outline1 = outline;
    } else {
      // Outline is in right blob.
      if (outline2)
        outline2->next = outline;
      else
        other_blob->outlines = outline;
      outline2 = outline;
    }
    outline = outline->next;
  }

  if (outline1)
    outline1->next = NULL;
  if (outline2)
    outline2->next = NULL;
}

