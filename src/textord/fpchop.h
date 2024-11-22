/**********************************************************************
 * File:        fpchop.h  (Formerly fp_chop.h)
 * Description: Code to chop fixed pitch text into character cells.
 * Author:      Ray Smith
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef FPCHOP_H
#define FPCHOP_H

#include "blobbox.h"
#include "params.h"

namespace tesseract {

class C_OUTLINE_FRAG : public ELIST<C_OUTLINE_FRAG>::LINK {
public:
  C_OUTLINE_FRAG() { // empty constructor
    steps = nullptr;
    stepcount = 0;
  }
  ~C_OUTLINE_FRAG() {
    delete[] steps;
  }
  // start coord
  C_OUTLINE_FRAG(ICOORD start_pt,
                 ICOORD end_pt,      // end coord
                 C_OUTLINE *outline, // source of steps
                 int16_t start_index, int16_t end_index);
  // other end
  C_OUTLINE_FRAG(C_OUTLINE_FRAG *head, int16_t tail_y);
  C_OUTLINE *close();        // copy to outline
  C_OUTLINE_FRAG &operator=( // assign
      const C_OUTLINE_FRAG &src);

  ICOORD start;              // start coord
  ICOORD end;                // end coord
  DIR128 *steps;             // step array
  int32_t stepcount;         // no of steps
  C_OUTLINE_FRAG *other_end; // head if a tail
  int16_t ycoord;            // coord of cut pt

private:
  // Copy constructor (currently unused, therefore private).
  C_OUTLINE_FRAG(const C_OUTLINE_FRAG &other) = delete;
};

ELISTIZEH(C_OUTLINE_FRAG)

extern INT_VAR_H(textord_fp_chop_error);

ROW *fixed_pitch_words( // find lines
    TO_ROW *row,        // row to do
    FCOORD rotation     // for drawing
);

void split_to_blob(                 // split the blob
    BLOBNBOX *blob,                 // blob to split
    int16_t chop_coord,             // place to chop
    float pitch_error,              // allowed deviation
    C_OUTLINE_LIST *left_coutlines, // for cblobs
    C_OUTLINE_LIST *right_coutlines);

} // namespace tesseract

#endif
