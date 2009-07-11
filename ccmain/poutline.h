/**********************************************************************
 * File:        poutline.h  (Formerly outline.h)
 * Description: OUTLINE class definition.
 * Author:		Ray Smith
 * Created:		Wed Oct 23 10:42:40 BST 1991
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

#ifndef           POUTLINE_H
#define           POUTLINE_H

#include          "scrollview.h"
#include          "polyvert.h"
#include          "rect.h"
#include          "blckerr.h"

#define INTERSECTING    MAX_INT16//no winding number

class OUTLINE;                   //forward declaration

ELISTIZEH_S (OUTLINE)
class OUTLINE:public ELIST_LINK
{
  public:
    OUTLINE() {  //empty constructor
    }
    OUTLINE(                        //constructor
            const ICOORD &startpt,  //start point
            inT8 *compactloop,      //from Tess format
            BOOL8 reverse,          //reverse it
            ICOORD bot_left,        //bounding box
            ICOORD top_right);
    OUTLINE(                      //constructor
            POLYPT_IT *poly_it);  //from list of pts

    OUTLINE_LIST *child() {  //get child list
      return &children;
    }

                                 //access function
    const TBOX &bounding_box() const {
      return box;
    }
    void compute_bb();  //set bounding box

                                 //get start position
    const ICOORD &start_pos() const {
      return start;
    }
    float area();  //return area
    POLYPT_LIST *polypts() {  //get poly
      return &outline;
    }

    BOOL8 operator< (            //containment test
      OUTLINE & other);
    BOOL8 operator> (            //containment test
    OUTLINE & other) {
      return other < *this;      //use the < to do it
    }
    inT16 winding_number(                        //get winding number
                         const FCOORD &testpt);  //around this point
    void reverse();  //reverse it

    void move(                    // reposition outline
              const FCOORD vec);  // by FLOAT vector

    void scale(                 // scale outline
               const float f);  // by multiplier
    void scale(                    // scale outline
               const FCOORD vec);  // by FLOAT vector

    void rotate(               // rotate outline
        const FCOORD vector);  // by fcoord

    void plot(                 //draw one
              ScrollView* window,   //window to draw in
              ScrollView::Color colour);  //colour to draw it

    void prep_serialise() {  //set ptrs to counts
      outline.prep_serialise ();
      children.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      outline.dump (f);
      children.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      outline.de_dump (f);
      children.de_dump (f);
    }

                                 //assignment
    make_serialise(OUTLINE)

    OUTLINE& operator=(const OUTLINE& source);

    static OUTLINE* deep_copy(const OUTLINE* src) {
      OUTLINE* outline = new OUTLINE;
      *outline = *src;
      return outline;
    }

  private:
    TBOX box;                     //boudning box
    ICOORD start;                //start coord
    POLYPT_LIST outline;         //outline points
    OUTLINE_LIST children;       //child elements
};
#endif
