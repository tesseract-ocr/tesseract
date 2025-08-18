/**********************************************************************
 * File:        boxchar.cpp
 * Description: Simple class to associate a Tesseract classification unit with
 *              its bounding box so that the boxes can be rotated as the image
 *              is rotated for degradation.  Also includes routines to output
 *              the character-tagged boxes to a boxfile.
 * Author:      Ray Smith
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

#include "fileio.h"
#include "normstrngs.h"
#include "tesserrstream.h"  // for tesserr
#include "tprintf.h"
#include "unicharset.h"
#include "unicode/uchar.h" // from libicu

#include <algorithm>
#include <cstddef>
#include <vector>

// Absolute Ratio of dx:dy or dy:dx to be a newline.
const int kMinNewlineRatio = 5;

namespace tesseract {

BoxChar::BoxChar(const char *utf8_str, int len)
    : ch_(utf8_str, len), box_(nullptr), page_(0), rtl_index_(-1) {}

BoxChar::~BoxChar() {
  boxDestroy(&box_);
}

void BoxChar::AddBox(int x, int y, int width, int height) {
  box_ = boxCreate(x, y, width, height);
}

// Increments *num_rtl and *num_ltr according to the directionality of
// characters in the box.
void BoxChar::GetDirection(int *num_rtl, int *num_ltr) const {
  // Convert the unichar to UTF32 representation
  std::vector<char32> uni_vector = UNICHAR::UTF8ToUTF32(ch_.c_str());
  if (uni_vector.empty()) {
    tprintf("Illegal utf8 in boxchar string:%s = ", ch_.c_str());
    for (char c : ch_) {
      tprintf(" 0x%x", c);
    }
    tprintf("\n");
    return;
  }
  for (char32 ch : uni_vector) {
    UCharDirection dir = u_charDirection(ch);
    if (dir == U_RIGHT_TO_LEFT || dir == U_RIGHT_TO_LEFT_ARABIC || dir == U_RIGHT_TO_LEFT_ISOLATE) {
      ++*num_rtl;
    } else if ((dir == U_ARABIC_NUMBER) ||
               (dir != U_DIR_NON_SPACING_MARK && dir != U_BOUNDARY_NEUTRAL)) {
      ++*num_ltr;
    }
  }
}

// Reverses the order of unicodes within the box. If Pango generates a
// ligature, these will get reversed on output, so reverse now.
void BoxChar::ReverseUnicodesInBox() {
  std::vector<char32> unicodes = UNICHAR::UTF8ToUTF32(ch_.c_str());
  std::reverse(unicodes.begin(), unicodes.end());
  ch_ = UNICHAR::UTF32ToUTF8(unicodes);
}

/* static */
void BoxChar::TranslateBoxes(int xshift, int yshift, std::vector<BoxChar *> *boxes) {
  for (auto &boxe : *boxes) {
    Box *box = boxe->box_;
    if (box != nullptr) {
      int32_t box_x;
      int32_t box_y;
      boxGetGeometry(box, &box_x, &box_y, nullptr, nullptr);
      boxSetGeometry(box, box_x + xshift, box_y + yshift, -1, -1);
    }
  }
}

// Prepares for writing the boxes to a file by inserting newlines, spaces,
// and re-ordering so the boxes are strictly left-to-right.
/* static */
void BoxChar::PrepareToWrite(std::vector<BoxChar *> *boxes) {
  bool rtl_rules = ContainsMostlyRTL(*boxes);
  bool vertical_rules = MostlyVertical(*boxes);
  InsertNewlines(rtl_rules, vertical_rules, boxes);
  InsertSpaces(rtl_rules, vertical_rules, boxes);
  for (size_t i = 0; i < boxes->size(); ++i) {
    if ((*boxes)[i]->box_ == nullptr) {
      tesserr << "Null box at index " << i << '\n';
    }
  }
  if (rtl_rules) {
    ReorderRTLText(boxes);
  }
}

// Inserts newline (tab) characters into the vector at newline positions.
/* static */
void BoxChar::InsertNewlines(bool rtl_rules, bool vertical_rules, std::vector<BoxChar *> *boxes) {
  size_t prev_i = SIZE_MAX;
  int max_shift = 0;
  for (size_t i = 0; i < boxes->size(); ++i) {
    Box *box = (*boxes)[i]->box_;
    if (box == nullptr) {
      if (prev_i == SIZE_MAX || prev_i + 1 < i || i + 1 == boxes->size()) {
        // Erase null boxes at the start of a line and after another null box.
        do {
          delete (*boxes)[i];
          boxes->erase(boxes->begin() + i);
          if (i == 0) {
            break;
          }
        } while (i-- == boxes->size() && (*boxes)[i]->box_ == nullptr);
      }
      continue;
    }
    if (prev_i != SIZE_MAX) {
      int32_t box_x;
      int32_t box_y;
      boxGetGeometry(box, &box_x, &box_y, nullptr, nullptr);
      Box *prev_box = (*boxes)[prev_i]->box_;
      int32_t prev_box_x;
      int32_t prev_box_y;
      int32_t prev_box_w;
      int32_t prev_box_h;
      boxGetGeometry(prev_box, &prev_box_x, &prev_box_y, &prev_box_w, &prev_box_h);
      int shift = box_x - prev_box_x;
      if (vertical_rules) {
        shift = box_y - prev_box_y;
      } else if (rtl_rules) {
        shift = -shift;
      }
      if (-shift > max_shift) {
        // This is a newline. Since nothing cares about the size of the box,
        // except the out-of-bounds checker, minimize the chance of creating
        // a box outside the image by making the width and height 1.
        int width = 1;
        int height = 1;
        int x = prev_box_x + prev_box_w;
        int y = prev_box_y;
        if (vertical_rules) {
          x = prev_box_x;
          y = prev_box_y + prev_box_h;
        } else if (rtl_rules) {
          x = prev_box_x - width;
          if (x < 0) {
            tprintf("prev x = %d, width=%d\n", prev_box_x, width);
            x = 0;
          }
        }
        if (prev_i + 1 == i) {
          // New character needed.
          auto *new_box = new BoxChar("\t", 1);
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

// Converts nullptr boxes to space characters, with appropriate bounding boxes.
/* static */
void BoxChar::InsertSpaces(bool rtl_rules, bool vertical_rules, std::vector<BoxChar *> *boxes) {
  // After InsertNewlines, any remaining null boxes are not newlines, and are
  // singletons, so add a box to each remaining null box.
  for (size_t i = 1; i + 1 < boxes->size(); ++i) {
    Box *box = (*boxes)[i]->box_;
    if (box == nullptr) {
      Box *prev = (*boxes)[i - 1]->box_;
      Box *next = (*boxes)[i + 1]->box_;
      int32_t prev_x;
      int32_t prev_y;
      int32_t prev_w;
      int32_t prev_h;
      int32_t next_x;
      int32_t next_y;
      int32_t next_w;
      int32_t next_h;
      ASSERT_HOST(prev != nullptr && next != nullptr);
      boxGetGeometry(prev, &prev_x, &prev_y, &prev_w, &prev_h);
      boxGetGeometry(next, &next_x, &next_y, &next_w, &next_h);
      int top = std::min(prev_y, next_y);
      int bottom = std::max(prev_y + prev_h, next_y + next_h);
      int left = prev_x + prev_w;
      int right = next_x;
      if (vertical_rules) {
        top = prev_y + prev_h;
        bottom = next_y;
        left = std::min(prev_x, next_x);
        right = std::max(prev_x + prev_w, next_x + next_w);
      } else if (rtl_rules) {
        // With RTL we have to account for BiDi.
        // Right becomes the min left of all prior boxes back to the first
        // space or newline.
        right = prev_x;
        left = next_x + next_w;
        for (int j = i - 2; j >= 0 && (*boxes)[j]->ch_ != " " && (*boxes)[j]->ch_ != "\t"; --j) {
          prev = (*boxes)[j]->box_;
          ASSERT_HOST(prev != nullptr);
          boxGetGeometry(prev, &prev_x, nullptr, nullptr, nullptr);
          if (prev_x < right) {
            right = prev_x;
          }
        }
        // Left becomes the max right of all next boxes forward to the first
        // space or newline.
        for (size_t j = i + 2;
             j < boxes->size() && (*boxes)[j]->box_ != nullptr && (*boxes)[j]->ch_ != "\t"; ++j) {
          next = (*boxes)[j]->box_;
          boxGetGeometry(next, &next_x, nullptr, &next_w, nullptr);
          if (next_x + next_w > left) {
            left = next_x + next_w;
          }
        }
      }
      // Italic and stylized characters can produce negative spaces, which
      // Leptonica doesn't like, so clip to a positive size.
      if (right <= left) {
        right = left + 1;
      }
      if (bottom <= top) {
        bottom = top + 1;
      }
      (*boxes)[i]->AddBox(left, top, right - left, bottom - top);
      (*boxes)[i]->ch_ = " ";
    }
  }
}

// Reorders text in a right-to-left script in left-to-right order.
/* static */
void BoxChar::ReorderRTLText(std::vector<BoxChar *> *boxes) {
  // Ideally we need the inverse of the algorithm used by ResultIterator.
  // For now, let's try a sort that reverses original positions for RTL
  // characters, otherwise by x-position. This should be much closer to
  // correct than just sorting by x-position.
  size_t num_boxes = boxes->size();
  for (size_t i = 0; i < num_boxes; ++i) {
    int num_rtl = 0, num_ltr = 0;
    (*boxes)[i]->GetDirection(&num_rtl, &num_ltr);
    if (num_rtl > num_ltr) {
      (*boxes)[i]->set_rtl_index(i);
      (*boxes)[i]->ReverseUnicodesInBox();
    }
  }
  BoxCharPtrSort sorter;
  size_t end = 0;
  for (size_t start = 0; start < boxes->size(); start = end + 1) {
    end = start + 1;
    while (end < boxes->size() && (*boxes)[end]->ch_ != "\t") {
      ++end;
    }
    std::sort(boxes->begin() + start, boxes->begin() + end, sorter);
  }
}

// Returns true if the vector contains mostly RTL characters.
/* static */
bool BoxChar::ContainsMostlyRTL(const std::vector<BoxChar *> &boxes) {
  int num_rtl = 0, num_ltr = 0;
  for (auto boxe : boxes) {
    boxe->GetDirection(&num_rtl, &num_ltr);
  }
  return num_rtl > num_ltr;
}

// Returns true if the text is mostly laid out vertically.
/* static */
bool BoxChar::MostlyVertical(const std::vector<BoxChar *> &boxes) {
  int64_t total_dx = 0, total_dy = 0;
  for (size_t i = 1; i < boxes.size(); ++i) {
    if (boxes[i - 1]->box_ != nullptr && boxes[i]->box_ != nullptr &&
        boxes[i - 1]->page_ == boxes[i]->page_) {
      int32_t x0;
      int32_t y0;
      boxGetGeometry(boxes[i]->box_, &x0, &y0, nullptr, nullptr);
      int32_t x1;
      int32_t y1;
      boxGetGeometry(boxes[i - 1]->box_, &x1, &y1, nullptr, nullptr);
      int dx = x0 - x1;
      int dy = y0 - y1;
      if (abs(dx) > abs(dy) * kMinNewlineRatio || abs(dy) > abs(dx) * kMinNewlineRatio) {
        total_dx += static_cast<int64_t>(dx) * dx;
        total_dy += static_cast<int64_t>(dy) * dy;
      }
    }
  }
  return total_dy > total_dx;
}

// Returns the total length of all the strings in the boxes.
/* static */
int BoxChar::TotalByteLength(const std::vector<BoxChar *> &boxes) {
  int total_length = 0;
  for (auto boxe : boxes) {
    total_length += boxe->ch_.size();
  }
  return total_length;
}

// Rotate the boxes in [start_box, end_box) by the given rotation.
// The rotation is in radians clockwise about the given center.
/* static */
void BoxChar::RotateBoxes(float rotation, int xcenter, int ycenter, int start_box, int end_box,
                          std::vector<BoxChar *> *boxes) {
  Boxa *orig = boxaCreate(0);
  for (int i = start_box; i < end_box; ++i) {
    Box *box = (*boxes)[i]->box_;
    if (box) {
      boxaAddBox(orig, box, L_CLONE);
    }
  }
  Boxa *rotated = boxaRotate(orig, xcenter, ycenter, rotation);
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
void BoxChar::WriteTesseractBoxFile(const std::string &filename, int height,
                                    const std::vector<BoxChar *> &boxes) {
  std::string output = GetTesseractBoxStr(height, boxes);
  File::WriteStringToFileOrDie(output, filename);
}

/* static */
std::string BoxChar::GetTesseractBoxStr(int height, const std::vector<BoxChar *> &boxes) {
  std::string output;
  char buffer[kMaxLineLength];
  for (auto boxe : boxes) {
    const Box *box = boxe->box_;
    if (box == nullptr) {
      tprintf("Error: Call PrepareToWrite before WriteTesseractBoxFile!!\n");
      return "";
    }
    int32_t box_x;
    int32_t box_y;
    int32_t box_w;
    int32_t box_h;
    boxGetGeometry(const_cast<Box *>(box), &box_x, &box_y, &box_w, &box_h);
    int nbytes = snprintf(buffer, kMaxLineLength, "%s %d %d %d %d %d\n",
                          boxe->ch_.c_str(), box_x, height - box_y - box_h,
                          box_x + box_w, height - box_y, boxe->page_);
    output.append(buffer, nbytes);
  }
  return output;
}

} // namespace tesseract
