/**********************************************************************
 * File:        stepblob.h  (Formerly cblob.h)
 * Description: Code for C_BLOB class.
 * Author:      Ray Smith
 * Created:     Tue Oct 08 10:41:13 BST 1991
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

#ifndef STEPBLOB_H
#define STEPBLOB_H

#include <cstdint>             // for int32_t, int16_t
#include "coutln.h"            // for C_OUTLINE_LIST, C_OUTLINE
#include "elst.h"              // for ELIST_ITERATOR, ELISTIZEH, ELIST_LINK
#include "points.h"            // for FCOORD, ICOORD (ptr only)
#include "rect.h"              // for TBOX
#include "scrollview.h"        // for ScrollView, ScrollView::Color

class C_BLOB;
class DENORM;

struct Pix;

ELISTIZEH(C_BLOB)

class C_BLOB:public ELIST_LINK
{
  public:
    C_BLOB() = default;
    explicit C_BLOB(C_OUTLINE_LIST *outline_list);
    // Simpler constructor to build a blob from a single outline that has
    // already been fully initialized.
    explicit C_BLOB(C_OUTLINE* outline);

    // Builds a set of one or more blobs from a list of outlines.
    // Input: one outline on outline_list contains all the others, but the
    // nesting and order are undefined.
    // If good_blob is true, the blob is added to good_blobs_it, unless
    // an illegal (generation-skipping) parent-child relationship is found.
    // If so, the parent blob goes to bad_blobs_it, and the immediate children
    // are promoted to the top level, recursively being sent to good_blobs_it.
    // If good_blob is false, all created blobs will go to the bad_blobs_it.
    // Output: outline_list is empty. One or more blobs are added to
    // good_blobs_it and/or bad_blobs_it.
    static void ConstructBlobsFromOutlines(bool good_blob,
                                           C_OUTLINE_LIST* outline_list,
                                           C_BLOB_IT* good_blobs_it,
                                           C_BLOB_IT* bad_blobs_it);

    // Sets the COUT_INVERSE flag appropriately on the outlines and their
    // children recursively, reversing the outlines if needed so that
    // everything has an anticlockwise top-level.
    void CheckInverseFlagAndDirection();

    // Build and return a fake blob containing a single fake outline with no
    // steps.
    static C_BLOB* FakeBlob(const TBOX& box);

    C_OUTLINE_LIST *out_list() {  //get outline list
      return &outlines;
    }

    TBOX bounding_box() const;  // compute bounding box
    int32_t area();  //compute area
    int32_t perimeter();  // Total perimeter of outlines and 1st level children.
    int32_t outer_area();  //compute area
    int32_t count_transitions(                   //count maxima
                            int32_t threshold);  //size threshold

    void move(const ICOORD vec);  // repostion blob by vector
    void rotate(const FCOORD& rotation);  // Rotate by given vector.

    // Adds sub-pixel resolution EdgeOffsets for the outlines using greyscale
    // if the supplied pix is 8-bit or the binary edges if nullptr.
    void ComputeEdgeOffsets(int threshold, Pix* pix);

    // Estimates and returns the baseline position based on the shape of the
    // outlines.
    int16_t EstimateBaselinePosition();

    // Returns a Pix rendering of the blob. pixDestroy after use.
    Pix* render();
    // Returns a Pix rendering of the outline of the blob. (no fill).
    // pixDestroy after use.
    Pix* render_outline();

    #ifndef GRAPHICS_DISABLED
    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color blob_colour,    //for outer bits
              ScrollView::Color child_colour);  //for holes
    // Draws the blob in the given colour, and child_colour, normalized
    // using the given denorm, making use of sub-pixel accurate information
    // if available.
    void plot_normed(const DENORM& denorm,
                     ScrollView::Color blob_colour,
                     ScrollView::Color child_colour,
                     ScrollView* window);
    #endif  // GRAPHICS_DISABLED

    C_BLOB& operator= (const C_BLOB & source) {
      if (!outlines.empty ())
        outlines.clear();
      outlines.deep_copy(&source.outlines, &C_OUTLINE::deep_copy);
      return *this;
    }

    static C_BLOB* deep_copy(const C_BLOB* src) {
      auto* blob = new C_BLOB;
      *blob = *src;
      return blob;
    }

    static int SortByXMiddle(const void *v1, const void *v2) {
      const C_BLOB* blob1 = *static_cast<const C_BLOB* const*>(v1);
      const C_BLOB* blob2 = *static_cast<const C_BLOB* const*>(v2);
      return blob1->bounding_box().x_middle() -
             blob2->bounding_box().x_middle();
    }


  private:
    C_OUTLINE_LIST outlines;     //master elements
};

#endif
