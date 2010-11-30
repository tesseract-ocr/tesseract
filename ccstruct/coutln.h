/**********************************************************************
 * File:						coutln.c      (Formerly:  coutline.c)
 * Description: Code for the C_OUTLINE class.
 * Author:					Ray Smith
 * Created:					Mon Oct 07 16:01:57 BST 1991
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

#ifndef           COUTLN_H
#define           COUTLN_H

#include          "crakedge.h"
#include          "mod128.h"
#include          "bits16.h"
#include          "rect.h"
#include          "blckerr.h"
#include          "scrollview.h"

#define INTERSECTING    MAX_INT16//no winding number

                                 //mask to get step
#define STEP_MASK       3

enum C_OUTLINE_FLAGS
{
  COUT_INVERSE                   //White on black blob
};

class DLLSYM C_OUTLINE;          //forward declaration
struct Pix;

ELISTIZEH (C_OUTLINE)
class DLLSYM C_OUTLINE:public ELIST_LINK
{
  public:
    C_OUTLINE() {  //empty constructor
      steps = NULL;
    }
    C_OUTLINE(                     //constructor
              CRACKEDGE *startpt,  //from edge detector
              ICOORD bot_left,     //bounding box //length of loop
              ICOORD top_right,
              inT16 length);
    C_OUTLINE(ICOORD startpt,    //start of loop
              DIR128 *new_steps,  //steps in loop
              inT16 length);     //length of loop
                                 //outline to copy
    C_OUTLINE(C_OUTLINE *srcline, FCOORD rotation);  //and rotate

    // Build a fake outline, given just a bounding box and append to the list.
    static void FakeOutline(const TBOX& box, C_OUTLINE_LIST* outlines);

    ~C_OUTLINE () {              //destructor
      if (steps != NULL)
        free_mem(steps);
      steps = NULL;
    }

    BOOL8 flag(                               //test flag
               C_OUTLINE_FLAGS mask) const {  //flag to test
      return flags.bit (mask);
    }
    void set_flag(                       //set flag value
                  C_OUTLINE_FLAGS mask,  //flag to test
                  BOOL8 value) {         //value to set
      flags.set_bit (mask, value);
    }

    C_OUTLINE_LIST *child() {  //get child list
      return &children;
    }

                                 //access function
    const TBOX &bounding_box() const {
      return box;
    }
    void set_step(                    //set a step
                  inT16 stepindex,    //index of step
                  inT8 stepdir) {     //chain code
      int shift = stepindex%4 * 2;
      uinT8 mask = 3 << shift;
      steps[stepindex/4] = ((stepdir << shift) & mask) |
                           (steps[stepindex/4] & ~mask);
      //squeeze 4 into byte
    }
    void set_step(                    //set a step
                  inT16 stepindex,    //index of step
                  DIR128 stepdir) {   //direction
                                 //clean it
      inT8 chaindir = stepdir.get_dir() >> (DIRBITS - 2);
                                 //difference
      set_step(stepindex, chaindir);
      //squeeze 4 into byte
    }

                                 //get start position
    const ICOORD &start_pos() const {
      return start;
    }
    inT32 pathlength() const {  //get path length
      return stepcount;
    }
    // Return step at a given index as a DIR128.
    DIR128 step_dir(inT16 index) const {
      return DIR128((inT16)(((steps[index/4] >> (index%4 * 2)) & STEP_MASK) <<
                      (DIRBITS - 2)));
    }
    // Return the step vector for the given outline position.
    ICOORD step(inT16 index) const { //index of step
      return step_coords[(steps[index/4] >> (index%4 * 2)) & STEP_MASK];
    }

    inT32 area();  //return area
    inT32 perimeter();  // Total perimeter of self and 1st level children.
    inT32 outer_area();  //return area
    inT32 count_transitions(                   //count maxima
                            inT32 threshold);  //size threshold

    BOOL8 operator< (            //containment test
      const C_OUTLINE & other) const;
    BOOL8 operator> (            //containment test
      C_OUTLINE & other) const
    {
      return other < *this;      //use the < to do it
    }
    inT16 winding_number(                       //get winding number
                         ICOORD testpt) const;  //around this point
                                 //get direction
    inT16 turn_direction() const;
    void reverse();  //reverse direction

    void move(                    // reposition outline
              const ICOORD vec);  // by vector

    // If this outline is smaller than the given min_size, delete this and
    // remove from its list, via *it, after checking that *it points to this.
    // Otherwise, if any children of this are too small, delete them.
    // On entry, *it must be an iterator pointing to this. If this gets deleted
    // then this is extracted from *it, so an iteration can continue.
    void RemoveSmallRecursive(int min_size, C_OUTLINE_IT* it);

    // Renders the outline to the given pix, with left and top being
    // the coords of the upper-left corner of the pix.
    void render(int left, int top, Pix* pix);

    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color colour) const;  //colour to draw it

    C_OUTLINE& operator=(const C_OUTLINE& source);

    static C_OUTLINE* deep_copy(const C_OUTLINE* src) {
      C_OUTLINE* outline = new C_OUTLINE;
      *outline = *src;
      return outline;
    }

    static ICOORD chain_step(int chaindir);

  private:
    int step_mem() const { return (stepcount+3) / 4; }

    TBOX box;                     //boudning box
    ICOORD start;                //start coord
    uinT8 *steps;                //step array
    inT16 stepcount;             //no of steps
    BITS16 flags;                //flags about outline
    C_OUTLINE_LIST children;     //child elements
    static ICOORD step_coords[4];
};
#endif
