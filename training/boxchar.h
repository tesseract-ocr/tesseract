/**********************************************************************
 * File:        boxchar.h
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

#ifndef TESSERACT_TRAINING_BOXCHAR_H_
#define TESSERACT_TRAINING_BOXCHAR_H_

#include <string>
#include <vector>

#include "allheaders.h"  // from Leptonica

#ifdef USE_STD_NAMESPACE
using std::string;
using std::vector;
#endif

struct Box;

namespace tesseract {

class BoxChar {
 public:
  BoxChar(const char* utf8_str, int len);

  ~BoxChar();

  // Accessors.
  const string& ch() const { return ch_; }
  const Box* box() const   { return box_; }
  const int& page() const  { return page_; }


  // Set the box_ member.
  void AddBox(int x, int y, int width, int height);

  void set_page(int page) { page_ = page; }

  string* mutable_ch() { return &ch_; }
  Box* mutable_box()   { return box_; }

  static void TranslateBoxes(int xshift, int yshift,
                             vector<BoxChar*>* boxes);

  // Rotate the vector of boxes between start and end by the given rotation.
  // The rotation is in radians clockwise about the given center.
  static void RotateBoxes(float rotation,
                          int xcenter,
                          int ycenter,
                          int start_box,
                          int end_box,
                          vector<BoxChar*>* boxes);

  // Create a tesseract box file from the vector of boxes. The image height
  // is needed to convert to tesseract coordinates.
  static void WriteTesseractBoxFile(const string& name, int height,
                                    const vector<BoxChar*>& boxes);

 private:
  string ch_;
  Box* box_;
  int page_;
};
}  // namespace tesseract

#endif  // TESSERACT_TRAINING_BOXCHAR_H_
