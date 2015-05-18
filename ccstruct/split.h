/* -*-C-*-
 ********************************************************************************
 *
 * File:        split.h  (Formerly split.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 13 10:49:23 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *****************************************************************************/
#ifndef SPLIT_H
#define SPLIT_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "blobs.h"
#include "scrollview.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
struct SPLIT {
  SPLIT() : point1(NULL), point2(NULL) {}
  SPLIT(EDGEPT* pt1, EDGEPT* pt2) : point1(pt1), point2(pt2) {}

  // Returns the bounding box of all the points in the split.
  TBOX bounding_box() const;
  // Returns the bounding box of the outline from point1 to point2.
  TBOX Box12() const { return point1->SegmentBox(point2); }
  // Returns the bounding box of the outline from point1 to point1.
  TBOX Box21() const { return point2->SegmentBox(point1); }
  // Returns the bounding box of the out

  // Hides the SPLIT so the outlines appear not to be cut by it.
  void Hide() const;
  // Undoes hide, so the outlines are cut by the SPLIT.
  void Reveal() const;

  // Returns true if the given EDGEPT is used by this SPLIT, checking only
  // the EDGEPT pointer, not the coordinates.
  bool UsesPoint(const EDGEPT* point) const {
    return point1 == point || point2 == point;
  }
  // Returns true if the other SPLIT has any position shared with *this.
  bool SharesPosition(const SPLIT& other) const {
    return point1->EqualPos(*other.point1) || point1->EqualPos(*other.point2) ||
           point2->EqualPos(*other.point1) || point2->EqualPos(*other.point2);
  }
  // Returns true if both points are contained within the blob.
  bool ContainedByBlob(const TBLOB& blob) const {
    return blob.Contains(point1->pos) && blob.Contains(point2->pos);
  }
  // Returns true if both points are contained within the outline.
  bool ContainedByOutline(const TESSLINE& outline) const {
    return outline.Contains(point1->pos) && outline.Contains(point2->pos);
  }
  // Compute a split priority based on the bounding boxes of the parts.
  // The arguments here are config parameters defined in Wordrec. Add chop_
  // to the beginning of the name.
  float FullPriority(int xmin, int xmax, double overlap_knob,
                     int centered_maxwidth, double center_knob,
                     double width_change_knob) const;
  // Returns true if *this SPLIT appears OK in the sense that it does not cross
  // any outlines and does not chop off any ridiculously small pieces.
  bool IsHealthy(const TBLOB& blob, int min_points, int min_area) const;
  // Returns true if the split generates a small chunk in terms of either area
  // or number of points.
  bool IsLittleChunk(int min_points, int min_area) const;

  void Print() const;
#ifndef GRAPHICS_DISABLED
  // Draws the split in the given window.
  void Mark(ScrollView* window) const;
#endif

  // Creates two outlines out of one by splitting the original one in half.
  // Inserts the resulting outlines into the given list.
  void SplitOutlineList(TESSLINE* outlines) const;
  // Makes a split between these two edge points, but does not affect the
  // outlines to which they belong.
  void SplitOutline() const;
  // Undoes the effect of SplitOutlineList, correcting the outlines for undoing
  // the split, but possibly leaving some duplicate outlines.
  void UnsplitOutlineList(TBLOB* blob) const;
  // Removes the split that was put between these two points.
  void UnsplitOutlines() const;

  EDGEPT *point1;
  EDGEPT *point2;
};

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

extern BOOL_VAR_H(wordrec_display_splits, 0, "Display splits");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
EDGEPT *make_edgept(int x, int y, EDGEPT *next, EDGEPT *prev);

void remove_edgept(EDGEPT *point);

#endif
