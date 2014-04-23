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
#include "ndminx.h"

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

const int kMaxLineLength = 1024;
// Helper appends a tab box to the string to indicate a newline. We can't use
// an actual newline as the file format is line-based text.
static void AppendTabBox(const Box* box, int height, int page, string* output) {
  char buffer[kMaxLineLength];
  int nbytes = snprintf(buffer, kMaxLineLength, "\t %d %d %d %d %d\n",
                        box->x + box->w, height - box->y - box->h,
                        box->x + box->w + 10, height - box->y, page);
  output->append(buffer, nbytes);
}

/* static */
void BoxChar::WriteTesseractBoxFile(const string& filename, int height,
                                    const vector<BoxChar*>& boxes) {
  string output;
  char buffer[kMaxLineLength];
  for (int i = 0; i < boxes.size(); ++i) {
    const Box* box = boxes[i]->box_;
    if (box != NULL) {
      if (i > 0 && boxes[i - 1]->box_ != NULL &&
          boxes[i - 1]->page_ == boxes[i]->page_ &&
          box->x + box->w < boxes[i - 1]->box_->x) {
        // We are on a newline. Output a tab character to indicate the newline.
        AppendTabBox(boxes[i - 1]->box_, height, boxes[i]->page_, &output);
      }
      int nbytes = snprintf(buffer, kMaxLineLength,
                            "%s %d %d %d %d %d\n",
                            boxes[i]->ch_.c_str(),
                            box->x, height - box->y - box->h,
                            box->x + box->w, height - box->y,
                            boxes[i]->page_);
      output.append(buffer, nbytes);
    } else if (i > 0 && boxes[i - 1]->box_ != NULL) {
      int j = i + 1;
      // Find the next non-null box, as there may be multiple spaces.
      while (j < boxes.size() && boxes[j]->box_ == NULL) ++j;
      if (j < boxes.size() && boxes[i - 1]->page_ == boxes[j]->page_) {
        const Box* prev = boxes[i - 1]->box_;
        const Box* next = boxes[j]->box_;
        if (next->x + next->w < prev->x) {
          // We are on a newline. Output a tab character to indicate it.
          AppendTabBox(prev, height, boxes[j]->page_, &output);
        } else {
          // Space between words.
          int nbytes = snprintf(buffer, kMaxLineLength,
                                "  %d %d %d %d %d\n",
                                prev->x + prev->w,
                                height - MAX(prev->y + prev->h,
                                             next->y + next->h),
                                next->x, height - MIN(prev->y, next->y),
                                boxes[i - 1]->page_);
          output.append(buffer, nbytes);
        }
      }
    }
  }
  File::WriteStringToFileOrDie(output, filename);
}
}  // namespace tesseract
