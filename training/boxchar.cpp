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
#include <algorithm>

#include "fileio.h"
#include "genericvector.h"
#include "ndminx.h"
#include "normstrngs.h"
#include "tprintf.h"
#include "unicharset.h"
#include "unicode/uchar.h"  // from libicu

// Absolute Ratio of dx:dy or dy:dx to be a newline.
const int kMinNewlineRatio = 5;

namespace tesseract {

BoxChar::BoxChar(const char* utf8_str, int len) : ch_(utf8_str, len) {
  box_ = NULL;
}

BoxChar::~BoxChar() { boxDestroy(&box_); }

void BoxChar::AddBox(int x, int y, int width, int height) {
  box_ = boxCreate(x, y, width, height);
}

/* static */
void BoxChar::TranslateBoxes(int xshift, int yshift, vector<BoxChar*>* boxes) {
  for (int i = 0; i < boxes->size(); ++i) {
    BOX* box = (*boxes)[i]->box_;
    if (box != NULL) {
      box->x += xshift;
      box->y += yshift;
    }
  }
}

// Prepares for writing the boxes to a file by inserting newlines, spaces,
// and re-ordering so the boxes are strictly left-to-right.
/* static */
void BoxChar::PrepareToWrite(vector<BoxChar*>* boxes) {
  bool rtl_rules = ContainsMostlyRTL(*boxes);
  bool vertical_rules = MostlyVertical(*boxes);
  InsertNewlines(rtl_rules, vertical_rules, boxes);
  InsertSpaces(rtl_rules, vertical_rules, boxes);
  for (int i = 0; i < boxes->size(); ++i) {
    if ((*boxes)[i]->box_ == NULL) tprintf("Null box at index %d\n", i);
  }
  if (rtl_rules) {
    ReorderRTLText(boxes);
  }
  tprintf("Rtl = %d ,vertical=%d\n", rtl_rules, vertical_rules);
}

// Inserts newline (tab) characters into the vector at newline positions.
/* static */
void BoxChar::InsertNewlines(bool rtl_rules, bool vertical_rules,
                             vector<BoxChar*>* boxes) {
  int prev_i = -1;
  int max_shift = 0;
  for (int i = 0; i < boxes->size(); ++i) {
    Box* box = (*boxes)[i]->box_;
    if (box == NULL) {
      if (prev_i < 0 || prev_i < i - 1 || i + 1 == boxes->size()) {
        // Erase null boxes at the start of a line and after another null box.
        do {
          delete (*boxes)[i];
          boxes->erase(boxes->begin() + i);
          --i;
        } while (i >= 0 && i + 1 == boxes->size() && (*boxes)[i]->box_ == NULL);
      }
      continue;
    }
    if (prev_i >= 0) {
      Box* prev_box = (*boxes)[prev_i]->box_;
      int shift = box->x - prev_box->x;
      if (vertical_rules) {
        shift = box->y - prev_box->y;
      } else if (rtl_rules) {
        shift = -shift;
      }
      if (-shift > max_shift) {
        // This is a newline.
        int width = prev_box->w;
        int height = prev_box->h;
        int x = prev_box->x + width;
        int y = prev_box->y;
        if (vertical_rules) {
          x = prev_box->x;
          y = prev_box->y + height;
        } else if (rtl_rules) {
          x = prev_box->x - width;
          if (x < 0) {
            tprintf("prev x = %d, width=%d\n", prev_box->x, width);
            x = 0;
          }
        }
        if (prev_i == i - 1) {
          // New character needed.
          BoxChar* new_box = new BoxChar("\t", 1);
          new_box->AddBox(x, y, width, height);
          new_box->page_ = (*boxes)[i]->page_;
          boxes->insert(boxes->begin() + i, new_box);
          ++i;
        } else {
          (*boxes)[i - 1]->AddBox(x, y, width, height);
          (*boxes)[i - 1]->ch_ = "\t";
        }
        max_shift = 0;
      } else if (shift > max_shift) {
        max_shift = shift;
      }
    }
    prev_i = i;
  }
}

// Converts NULL boxes to space characters, with appropriate bounding boxes.
/* static */
void BoxChar::InsertSpaces(bool rtl_rules, bool vertical_rules,
                           vector<BoxChar*>* boxes) {
  // After InsertNewlines, any remaining null boxes are not newlines, and are
  // singletons, so add a box to each remaining null box.
  for (int i = 1; i + 1 < boxes->size(); ++i) {
    Box* box = (*boxes)[i]->box_;
    if (box == NULL) {
      Box* prev = (*boxes)[i - 1]->box_;
      Box* next = (*boxes)[i + 1]->box_;
      ASSERT_HOST(prev != NULL && next != NULL);
      int top = MIN(prev->y, next->y);
      int bottom = MAX(prev->y + prev->h, next->y + next->h);
      int left = prev->x + prev->w;
      int right = next->x;
      if (vertical_rules) {
        top = prev->y + prev->h;
        bottom = next->y;
        left = MIN(prev->x, next->x);
        right = MAX(prev->x + prev->w, next->x + next->w);
      } else if (rtl_rules) {
        // With RTL we have to account for BiDi.
        // Right becomes the min left of all prior boxes back to the first
        // space or newline.
        right = prev->x;
        left = next->x + next->w;
        for (int j = i - 2;
             j >= 0 && (*boxes)[j]->ch_ != " " && (*boxes)[j]->ch_ != "\t";
             --j) {
          prev = (*boxes)[j]->box_;
          ASSERT_HOST(prev != NULL);
          if (prev->x < right) {
            right = prev->x;
          }
        }
        // Left becomes the max right of all next boxes forward to the first
        // space or newline.
        for (int j = i + 2; j < boxes->size() && (*boxes)[j]->box_ != NULL &&
                                (*boxes)[j]->ch_ != "\t";
             ++j) {
          next = (*boxes)[j]->box_;
          if (next->x + next->w > left) {
            left = next->x + next->w;
          }
        }
      }
      // Italic and stylized characters can produce negative spaces, which
      // Leptonica doesn't like, so clip to a positive size.
      if (right <= left) right = left + 1;
      if (bottom <= top) bottom = top + 1;
      (*boxes)[i]->AddBox(left, top, right - left, bottom - top);
      (*boxes)[i]->ch_ = " ";
    }
  }
}

// Reorders text in a right-to-left script in left-to-right order.
/* static */
void BoxChar::ReorderRTLText(vector<BoxChar*>* boxes) {
  // After adding newlines and spaces, this task is simply a matter of sorting
  // by left each group of boxes between newlines.
  BoxCharPtrSort sorter;
  int end = 0;
  for (int start = 0; start < boxes->size(); start = end + 1) {
    end = start + 1;
    while (end < boxes->size() && (*boxes)[end]->ch_ != "\t") ++end;
    sort(boxes->begin() + start, boxes->begin() + end, sorter);
  }
}

// Returns true if the vector contains mostly RTL characters.
/* static */
bool BoxChar::ContainsMostlyRTL(const vector<BoxChar*>& boxes) {
  int num_rtl = 0, num_ltr = 0;
  for (int i = 0; i < boxes.size(); ++i) {
    // Convert the unichar to UTF32 representation
    GenericVector<char32> uni_vector;
    if (!UNICHAR::UTF8ToUnicode(boxes[i]->ch_.c_str(), &uni_vector)) {
      tprintf("Illegal utf8 in boxchar %d string:%s = ", i,
              boxes[i]->ch_.c_str());
      for (int c = 0; c < boxes[i]->ch_.size(); ++c) {
        tprintf(" 0x%x", boxes[i]->ch_[c]);
      }
      tprintf("\n");
      continue;
    }
    for (int j = 0; j < uni_vector.size(); ++j) {
      UCharDirection dir = u_charDirection(uni_vector[j]);
      if (dir == U_RIGHT_TO_LEFT || dir == U_RIGHT_TO_LEFT_ARABIC ||
          dir == U_ARABIC_NUMBER) {
        ++num_rtl;
      } else {
        ++num_ltr;
      }
    }
  }
  return num_rtl > num_ltr;
}

// Returns true if the text is mostly laid out vertically.
/* static */
bool BoxChar::MostlyVertical(const vector<BoxChar*>& boxes) {
  inT64 total_dx = 0, total_dy = 0;
  for (int i = 1; i < boxes.size(); ++i) {
    if (boxes[i - 1]->box_ != NULL && boxes[i]->box_ != NULL &&
        boxes[i - 1]->page_ == boxes[i]->page_) {
      int dx = boxes[i]->box_->x - boxes[i - 1]->box_->x;
      int dy = boxes[i]->box_->y - boxes[i - 1]->box_->y;
      if (abs(dx) > abs(dy) * kMinNewlineRatio ||
          abs(dy) > abs(dx) * kMinNewlineRatio) {
        total_dx += dx * dx;
        total_dy += dy * dy;
      }
    }
  }
  return total_dy > total_dx;
}

// Returns the total length of all the strings in the boxes.
/* static */
int BoxChar::TotalByteLength(const vector<BoxChar*>& boxes) {
  int total_length = 0;
  for (int i = 0; i < boxes.size(); ++i) total_length += boxes[i]->ch_.size();
  return total_length;
}

// Rotate the boxes in [start_box, end_box) by the given rotation.
// The rotation is in radians clockwise about the given center.
/* static */
void BoxChar::RotateBoxes(float rotation, int xcenter, int ycenter,
                          int start_box, int end_box, vector<BoxChar*>* boxes) {
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
/* static */
void BoxChar::WriteTesseractBoxFile(const string& filename, int height,
                                    const vector<BoxChar*>& boxes) {
  string output;
  char buffer[kMaxLineLength];
  for (int i = 0; i < boxes.size(); ++i) {
    const Box* box = boxes[i]->box_;
    if (box == NULL) {
      tprintf("Error: Call PrepareToWrite before WriteTesseractBoxFile!!\n");
      return;
    }
    int nbytes =
        snprintf(buffer, kMaxLineLength, "%s %d %d %d %d %d\n",
                 boxes[i]->ch_.c_str(), box->x, height - box->y - box->h,
                 box->x + box->w, height - box->y, boxes[i]->page_);
    output.append(buffer, nbytes);
  }
  File::WriteStringToFileOrDie(output, filename);
}
}  // namespace tesseract
