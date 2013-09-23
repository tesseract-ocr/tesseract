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

class DENORM;

#define INTERSECTING    MAX_INT16//no winding number

                                 //mask to get step
#define STEP_MASK       3

enum C_OUTLINE_FLAGS
{
  COUT_INVERSE                   //White on black blob
};

// Simple struct to hold the 3 values needed to compute a more precise edge
// position and direction. The offset_numerator is the difference between the
// grey threshold and the mean pixel value. pixel_diff is the difference between
// the pixels in the edge. Consider the following row of pixels: p1 p2 p3 p4 p5
// Say the image was thresholded  at threshold t, making p1, p2, p3 black
// and p4, p5 white (p1, p2, p3 < t, and p4, p5 >= t), but suppose that
// max(p[i+1] - p[i]) is p3 - p2. Then the extrapolated position of the edge,
// based on the maximum gradient, is at the crack between p2 and p3 plus the
// offset (t - (p2+p3)/2)/(p3 - p2). We store the pixel difference p3-p2
// denominator in pixel_diff and the offset numerator, relative to the original
// binary edge (t - (p2+p3)/2) - (p3 -p2) in offset_numerator.
// The sign of offset_numerator and pixel_diff are manipulated to ensure
// that the pixel_diff, which will be used as a weight, is always positive.
// The direction stores the quantized feature direction for the given step
// computed from the edge gradient. (Using binary_angle_plus_pi.)
// If the pixel_diff is zero, it means that the direction of the gradient
// is in conflict with the step direction, so this step is to be ignored.
struct EdgeOffset {
  inT8 offset_numerator;
  uinT8 pixel_diff;
  uinT8 direction;
};

class DLLSYM C_OUTLINE;          //forward declaration
struct Pix;

ELISTIZEH (C_OUTLINE)
class DLLSYM C_OUTLINE:public ELIST_LINK {
 public:
  C_OUTLINE() {  //empty constructor
      steps = NULL;
      offsets = NULL;
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
      delete [] offsets;
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

    inT32 pathlength() const {  //get path length
      return stepcount;
    }
    // Return step at a given index as a DIR128.
    DIR128 step_dir(int index) const {
      return DIR128((inT16)(((steps[index/4] >> (index%4 * 2)) & STEP_MASK) <<
                      (DIRBITS - 2)));
    }
    // Return the step vector for the given outline position.
    ICOORD step(int index) const { // index of step
      return step_coords[chain_code(index)];
    }
                                 // get start position
    const ICOORD &start_pos() const {
      return start;
    }
    // Returns the position at the given index on the outline.
    // NOT to be used lightly, as it has to iterate the outline to find out.
    ICOORD position_at_index(int index) const {
      ICOORD pos = start;
      for (int i = 0; i < index; ++i)
        pos += step(i);
      return pos;
    }
    // Returns the sub-pixel accurate position given the integer position pos
    // at the given index on the outline. pos may be a return value of
    // position_at_index, or computed by repeatedly adding step to the
    // start_pos() in the usual way.
    FCOORD sub_pixel_pos_at_index(const ICOORD& pos, int index) const {
      const ICOORD& step_to_next(step(index));
      FCOORD f_pos(pos.x() + step_to_next.x() / 2.0f,
                   pos.y() + step_to_next.y() / 2.0f);
      if (offsets != NULL && offsets[index].pixel_diff > 0) {
        float offset = offsets[index].offset_numerator;
        offset /= offsets[index].pixel_diff;
        if (step_to_next.x() != 0)
          f_pos.set_y(f_pos.y() + offset);
        else
          f_pos.set_x(f_pos.x() + offset);
      }
      return f_pos;
    }
    // Returns the step direction for the given index or -1 if there is none.
    int direction_at_index(int index) const {
      if (offsets != NULL && offsets[index].pixel_diff > 0)
        return offsets[index].direction;
      return -1;
    }
    // Returns the edge strength for the given index.
    // If there are no recorded edge strengths, returns 1 (assuming the image
    // is binary). Returns 0 if the gradient direction conflicts with the
    // step direction, indicating that this position could be skipped.
    int edge_strength_at_index(int index) const {
      if (offsets != NULL)
        return offsets[index].pixel_diff;
      return 1;
    }
    // Return the step as a chain code (0-3) related to the standard feature
    // direction of binary_angle_plus_pi by:
    // chain_code * 64 = feature direction.
    int chain_code(int index) const {  // index of step
      return (steps[index / 4] >> (index % 4 * 2)) & STEP_MASK;
    }

    inT32 area() const;  // Returns area of self and 1st level children.
    inT32 perimeter() const;  // Total perimeter of self and 1st level children.
    inT32 outer_area() const;  // Returns area of self only.
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

    // Returns true if *this and its children are legally nested.
    // The outer area of a child should have the opposite sign to the
    // parent. If not, it means we have discarded an outline in between
    // (probably due to excessive length).
    bool IsLegallyNested() const;

    // If this outline is smaller than the given min_size, delete this and
    // remove from its list, via *it, after checking that *it points to this.
    // Otherwise, if any children of this are too small, delete them.
    // On entry, *it must be an iterator pointing to this. If this gets deleted
    // then this is extracted from *it, so an iteration can continue.
    void RemoveSmallRecursive(int min_size, C_OUTLINE_IT* it);

    // Adds sub-pixel resolution EdgeOffsets for the outline if the supplied
    // pix is 8-bit. Does nothing otherwise.
    void ComputeEdgeOffsets(int threshold, Pix* pix);
    // Adds sub-pixel resolution EdgeOffsets for the outline using only
    // a binary image source.
    void ComputeBinaryOffsets();

    // Renders the outline to the given pix, with left and top being
    // the coords of the upper-left corner of the pix.
    void render(int left, int top, Pix* pix) const;

    // Renders just the outline to the given pix (no fill), with left and top
    // being the coords of the upper-left corner of the pix.
    void render_outline(int left, int top, Pix* pix) const;

    #ifndef GRAPHICS_DISABLED
    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color colour) const;  //colour to draw it
    // Draws the outline in the given colour, normalized using the given denorm,
    // making use of sub-pixel accurate information if available.
    void plot_normed(const DENORM& denorm, ScrollView::Color colour,
                     ScrollView* window) const;
    #endif  // GRAPHICS_DISABLED

    C_OUTLINE& operator=(const C_OUTLINE& source);

    static C_OUTLINE* deep_copy(const C_OUTLINE* src) {
      C_OUTLINE* outline = new C_OUTLINE;
      *outline = *src;
      return outline;
    }

    static ICOORD chain_step(int chaindir);

    // The maximum length of any outline. The stepcount is stored as 16 bits,
    // but it is probably not a good idea to increase this constant by much
    // and switch to 32 bits, as it plays an important role in keeping huge
    // outlines invisible, which prevents bad speed behavior.
    static const int kMaxOutlineLength = 16000;

  private:
    // Helper for ComputeBinaryOffsets. Increments pos, dir_counts, pos_totals
    // by the step, increment, and vertical step ? x : y position * increment
    // at step s Mod stepcount respectively. Used to add or subtract the
    // direction and position to/from accumulators of a small neighbourhood.
    void increment_step(int s, int increment, ICOORD* pos, int* dir_counts,
                        int* pos_totals) const;
    int step_mem() const { return (stepcount+3) / 4; }

    TBOX box;                    // bounding box
    ICOORD start;                // start coord
    inT16 stepcount;             // no of steps
    BITS16 flags;                // flags about outline
    uinT8 *steps;                // step array
    EdgeOffset* offsets;         // Higher precision edge.
    C_OUTLINE_LIST children;     // child elements
    static ICOORD step_coords[4];
};
#endif
