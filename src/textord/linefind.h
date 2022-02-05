///////////////////////////////////////////////////////////////////////
// File:        linefind.h
// Description: Class to find vertical lines in an image and create
//              a corresponding list of empty blobs.
// Author:      Ray Smith
//
// (C) Copyright 2008, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_TEXTORD_LINEFIND_H_
#define TESSERACT_TEXTORD_LINEFIND_H_

namespace tesseract {

class TabVector_LIST;

/**
 * The LineFinder class is a simple static function wrapper class that mainly
 * exposes the FindVerticalLines function.
 */
class LineFinder {
public:
  /**
   * Finds vertical and horizontal line objects in the given pix and removes
   * them.
   *
   * Uses the given resolution to determine size thresholds instead of any
   * that may be present in the pix.
   *
   * The output vertical_x and vertical_y contain a sum of the output vectors,
   * thereby giving the mean vertical direction.
   *
   * If pix_music_mask != nullptr, and music is detected, a mask of the staves
   * and anything that is connected (bars, notes etc.) will be returned in
   * pix_music_mask, the mask subtracted from pix, and the lines will not
   * appear in v_lines or h_lines.
   *
   * The output vectors are owned by the list and Frozen (cannot refit) by
   * having no boxes, as there is no need to refit or merge separator lines.
   *
   * The detected lines are removed from the pix.
   */
  static void FindAndRemoveLines(int resolution, bool debug, Image pix, int *vertical_x,
                                 int *vertical_y, Image *pix_music_mask, TabVector_LIST *v_lines,
                                 TabVector_LIST *h_lines);
};

} // namespace tesseract.

#endif // TESSERACT_TEXTORD_LINEFIND_H_
