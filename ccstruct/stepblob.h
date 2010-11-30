/**********************************************************************
 * File:        stepblob.h  (Formerly cblob.h)
 * Description: Code for C_BLOB class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 10:41:13 BST 1991
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

#ifndef           STEPBLOB_H
#define           STEPBLOB_H

#include          "coutln.h"
#include          "rect.h"

struct Pix;

class C_BLOB:public ELIST_LINK
{
  public:
    C_BLOB() {
    }
    explicit C_BLOB(C_OUTLINE_LIST *outline_list);
    // Simpler constructor to build a blob from a single outline that has
    // already been fully initialized.
    explicit C_BLOB(C_OUTLINE* outline);

    // Build and return a fake blob containing a single fake outline with no
    // steps.
    static C_BLOB* FakeBlob(const TBOX& box);

    C_OUTLINE_LIST *out_list() {  //get outline list
      return &outlines;
    }

    TBOX bounding_box();  //compute bounding box
    inT32 area();  //compute area
    inT32 perimeter();  // Total perimeter of outlines and 1st level children.
    inT32 outer_area();  //compute area
    inT32 count_transitions(                   //count maxima
                            inT32 threshold);  //size threshold

    void move(const ICOORD vec);  // repostion blob by vector
    void rotate(const FCOORD& rotation);  // Rotate by given vector.

    // Returns a Pix rendering of the blob. pixDestroy after use.
    Pix* render();

    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color blob_colour,    //for outer bits
              ScrollView::Color child_colour);  //for holes

    C_BLOB& operator= (const C_BLOB & source) {
      if (!outlines.empty ())
        outlines.clear();
      outlines.deep_copy(&source.outlines, &C_OUTLINE::deep_copy);
      return *this;
    }

    static C_BLOB* deep_copy(const C_BLOB* src) {
      C_BLOB* blob = new C_BLOB;
      *blob = *src;
      return blob;
    }

  private:
    C_OUTLINE_LIST outlines;     //master elements
};

ELISTIZEH (C_BLOB)
#endif
