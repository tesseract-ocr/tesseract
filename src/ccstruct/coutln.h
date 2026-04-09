/**********************************************************************
 * File:        coutln.h
 * Description: Code for the C_OUTLINE class.
 * Author:      Ray Smith
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

#ifndef COUTLN_H
#define COUTLN_H

#include "elst.h"       // for ELIST_ITERATOR, ELISTIZEH, ELIST_LINK
#include "mod128.h"     // for DIR128, DIRBITS
#include "points.h"     // for ICOORD, FCOORD
#include "rect.h"       // for TBOX
#include "scrollview.h" // for ScrollView, ScrollView::Color

#include <tesseract/export.h> // for DLLSYM

#include <cstdint> // for int16_t, int32_t
#include <bitset>  // for std::bitset<16>

struct Pix;

namespace tesseract {

class CRACKEDGE;
class DENORM;

#define INTERSECTING INT16_MAX // no winding number

// mask to get step
#define STEP_MASK 3

enum C_OUTLINE_FLAGS {
  COUT_INVERSE // White on black blob
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
  int8_t offset_numerator;
  uint8_t pixel_diff;
  uint8_t direction;
};

class C_OUTLINE; // forward declaration

ELISTIZEH(C_OUTLINE)
class C_OUTLINE : public ELIST<C_OUTLINE>::LINK {
public:
  C_OUTLINE() {
    stepcount = 0;
    offsets = nullptr;
  }
  C_OUTLINE(              // constructor
      CRACKEDGE *startpt, // from edge detector
      ICOORD bot_left,    // bounding box //length of loop
      ICOORD top_right, int16_t length);
  C_OUTLINE(ICOORD startpt,                       // start of loop
            DIR128 *new_steps,                    // steps in loop
            int16_t length);                      // length of loop
                                                  // outline to copy
  C_OUTLINE(C_OUTLINE *srcline, FCOORD rotation); // and rotate

  // Build a fake outline, given just a bounding box and append to the list.
  static void FakeOutline(const TBOX &box, C_OUTLINE_LIST *outlines);

  ~C_OUTLINE() { // destructor
    delete[] offsets;
  }

  bool flag(                        // test flag
      C_OUTLINE_FLAGS mask) const { // flag to test
    return flags[mask];
  }
  void set_flag(            // set flag value
      C_OUTLINE_FLAGS mask, // flag to test
      bool value) {         // value to set
    flags.set(mask, value);
  }

  C_OUTLINE_LIST *child() { // get child list
    return &children;
  }

  // access function
  const TBOX &bounding_box() const {
    return box;
  }
  void set_step(         // set a step
      int16_t stepindex, // index of step
      int8_t stepdir) {  // chain code
    int shift = stepindex % 4 * 2;
    uint8_t mask = 3 << shift;
    steps[stepindex / 4] = ((stepdir << shift) & mask) | (steps[stepindex / 4] & ~mask);
    // squeeze 4 into byte
  }
  void set_step(         // set a step
      int16_t stepindex, // index of step
      DIR128 stepdir) {  // direction
    // clean it
    int8_t chaindir = stepdir.get_dir() >> (DIRBITS - 2);
    // difference
    set_step(stepindex, chaindir);
    // squeeze 4 into byte
  }

  int32_t pathlength() const { // get path length
    return stepcount;
  }
  // Return step at a given index as a DIR128.
  DIR128 step_dir(int index) const {
    return DIR128(
        static_cast<int16_t>(((steps[index / 4] >> (index % 4 * 2)) & STEP_MASK) << (DIRBITS - 2)));
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
    for (int i = 0; i < index; ++i) {
      pos += step(i);
    }
    return pos;
  }
  // Returns the sub-pixel accurate position given the integer position pos
  // at the given index on the outline. pos may be a return value of
  // position_at_index, or computed by repeatedly adding step to the
  // start_pos() in the usual way.
  FCOORD sub_pixel_pos_at_index(const ICOORD &pos, int index) const {
    const ICOORD &step_to_next(step(index));
    FCOORD f_pos(pos.x() + step_to_next.x() / 2.0f, pos.y() + step_to_next.y() / 2.0f);
    if (offsets != nullptr && offsets[index].pixel_diff > 0) {
      float offset = offsets[index].offset_numerator;
      offset /= offsets[index].pixel_diff;
      if (step_to_next.x() != 0) {
        f_pos.set_y(f_pos.y() + offset);
      } else {
        f_pos.set_x(f_pos.x() + offset);
      }
    }
    return f_pos;
  }
  // Returns the step direction for the given index or -1 if there is none.
  int direction_at_index(int index) const {
    if (offsets != nullptr && offsets[index].pixel_diff > 0) {
      return offsets[index].direction;
    }
    return -1;
  }
  // Returns the edge strength for the given index.
  // If there are no recorded edge strengths, returns 1 (assuming the image
  // is binary). Returns 0 if the gradient direction conflicts with the
  // step direction, indicating that this position could be skipped.
  int edge_strength_at_index(int index) const {
    if (offsets != nullptr) {
      return offsets[index].pixel_diff;
    }
    return 1;
  }
  // Return the step as a chain code (0-3) related to the standard feature
  // direction of binary_angle_plus_pi by:
  // chain_code * 64 = feature direction.
  int chain_code(int index) const { // index of step
    return (steps[index / 4] >> (index % 4 * 2)) & STEP_MASK;
  }

  int32_t area() const;       // Returns area of self and 1st level children.
  int32_t perimeter() const;  // Total perimeter of self and 1st level children.
  int32_t outer_area() const; // Returns area of self only.
  int32_t count_transitions(  // count maxima
      int32_t threshold);     // size threshold

  bool operator<( // containment test
      const C_OUTLINE &other) const;
  bool operator>( // containment test
      C_OUTLINE &other) const {
    return other < *this; // use the < to do it
  }
  int16_t winding_number(   // get winding number
      ICOORD testpt) const; // around this point
                            // get direction
  int16_t turn_direction() const;
  void reverse(); // reverse direction

  void move(             // reposition outline
      const ICOORD vec); // by vector

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
  void RemoveSmallRecursive(int min_size, C_OUTLINE_IT *it);

  // Adds sub-pixel resolution EdgeOffsets for the outline if the supplied
  // pix is 8-bit. Does nothing otherwise.
  void ComputeEdgeOffsets(int threshold, Image pix);
  // Adds sub-pixel resolution EdgeOffsets for the outline using only
  // a binary image source.
  void ComputeBinaryOffsets();

  // Renders the outline to the given pix, with left and top being
  // the coords of the upper-left corner of the pix.
  void render(int left, int top, Image pix) const;

  // Renders just the outline to the given pix (no fill), with left and top
  // being the coords of the upper-left corner of the pix.
  void render_outline(int left, int top, Image pix) const;

#ifndef GRAPHICS_DISABLED
  void plot(                           // draw one
      ScrollView *window,              // window to draw in
      ScrollView::Color colour) const; // colour to draw it
  // Draws the outline in the given colour, normalized using the given denorm,
  // making use of sub-pixel accurate information if available.
  void plot_normed(const DENORM &denorm, ScrollView::Color colour, ScrollView *window) const;
#endif // !GRAPHICS_DISABLED

  C_OUTLINE &operator=(const C_OUTLINE &source);

  static C_OUTLINE *deep_copy(const C_OUTLINE *src) {
    auto *outline = new C_OUTLINE;
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
  void increment_step(int s, int increment, ICOORD *pos, int *dir_counts, int *pos_totals) const;
  int step_mem() const {
    return (stepcount + 3) / 4;
  }

  TBOX box;                // bounding box
  ICOORD start;            // start coord
  int16_t stepcount;       // no of steps
  std::bitset<16> flags;   // flags about outline
  std::vector<uint8_t> steps; // step array
  EdgeOffset *offsets;     // Higher precision edge.
  C_OUTLINE_LIST children; // child elements
  static ICOORD step_coords[4];
};

} // namespace tesseract

#endif
