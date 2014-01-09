/**********************************************************************
 * File:        boxchar.cpp
 * Description: Simple class to associate a Tesseract classification unit with
 *              its bounding box so that the boxes can be rotated as the image
 *              is rotated for degradation.  Also includes routines to output
 *              the character-tagged boxes to a boxfile.
 * Author:      Ray Smith
 * Created:     Mon Nov 18 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

#include "boxchar.h"

#include <stddef.h>

#include "fileio.h"

namespace tesseract {

BoxChar::BoxChar(const char* utf8_str, int len) : ch_(utf8_str, len) {
  box_ = NULL;
}

BoxChar::~BoxChar() {
  boxDestroy(&box_);
}

void BoxChar::AddBox(int x, int y, int width, int height) {
  box_ = boxCreate(x, y, width, height);
}

/* static */
void BoxChar::TranslateBoxes(int xshift, int yshift,
                             vector<BoxChar*>* boxes) {
  for (int i = 0; i < boxes->size(); ++i) {
    BOX* box = (*boxes)[i]->box_;
    if (box != NULL) {
      box->x += xshift;
      box->y += yshift;
    }
  }
}

// Rotate the boxes in [start_box, end_box) by the given rotation.
// The rotation is in radians clockwise about the given center.
/* static */
void BoxChar::RotateBoxes(float rotation,
                          int xcenter,
                          int ycenter,
                          int start_box,
                          int end_box,
                          vector<BoxChar*>* boxes) {
  Boxa* orig = boxaCreate(0);
  for (int i = start_box; i < end_box; ++i) {
    BOX* box = (*boxes)[i]->box_;
    if (box) boxaAddBox(orig, box, L_CLONE);
  }
  Boxa* rotated = boxaRotate(orig, xcenter, ycenter, rotation);
  boxaDestroy(&orig);
  for (int i = start_box, box_ind = 0; i < end_box; ++i) {
    if ((*boxes)[i]->box_) {
      boxDestroy(&((*boxes)[i]->box_));
      (*boxes)[i]->box_ = boxaGetBox(rotated, box_ind++, L_CLONE);
    }
  }
  boxaDestroy(&rotated);
}

/* static */
void BoxChar::WriteTesseractBoxFile(const string& filename, int height,
                                    const vector<BoxChar*>& boxes) {
  string output;
  const int kMaxLineLength = 1024;
  char buffer[kMaxLineLength];
  for (int i = 0; i < boxes.size(); ++i) {
    if (boxes[i]->box_ != NULL) {
      int nbytes = snprintf(buffer, kMaxLineLength,
                            "%s %d %d %d %d %d\n",
                            boxes[i]->ch_.c_str(),
                            boxes[i]->box_->x,
                            height - boxes[i]->box_->y - boxes[i]->box_->h,
                            boxes[i]->box_->x + boxes[i]->box_->w,
                            height - boxes[i]->box_->y,
                            boxes[i]->page_);
      output.append(buffer, nbytes);
    }
  }
  File::WriteStringToFileOrDie(output, filename);
}
}  // namespace tesseract
