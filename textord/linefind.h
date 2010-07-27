///////////////////////////////////////////////////////////////////////
// File:        linefind.h
// Description: Class to find vertical lines in an image and create
//              a corresponding list of empty blobs.
// Author:      Ray Smith
// Created:     Thu Mar 20 09:49:01 PDT 2008
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

#ifndef TESSERACT_TEXTORD_LINEFIND_H__
#define TESSERACT_TEXTORD_LINEFIND_H__

struct Pix;
struct Boxa;
class C_BLOB_LIST;
class BLOBNBOX_LIST;
class ICOORD;

namespace tesseract {

class TabVector_LIST;

/**
 * The LineFinder class is a simple static function wrapper class that mainly
 * exposes the FindVerticalLines function.
 */
class LineFinder {
 public:
  /**
   * Finds vertical line objects in the given pix.
   *
   * Uses the given resolution to determine size thresholds instead of any
   * that may be present in the pix.
   *
   * The output vertical_x and vertical_y contain a sum of the output vectors,
   * thereby giving the mean vertical direction.
   *
   * The output vectors are owned by the list and Frozen (cannot refit) by
   * having no boxes, as there is no need to refit or merge separator lines.
   */
  static void FindVerticalLines(int resolution,  Pix* pix,
                                int* vertical_x, int* vertical_y,
                                TabVector_LIST* vectors);

  /**
   * Finds horizontal line objects in the given pix.
   *
   * Uses the given resolution to determine size thresholds instead of any
   * that may be present in the pix.
   *
   * The output vectors are owned by the list and Frozen (cannot refit) by
   * having no boxes, as there is no need to refit or merge separator lines.
   */
  static void FindHorizontalLines(int resolution,  Pix* pix,
                                  TabVector_LIST* vectors);

  /**
   * Converts the Boxa array to a list of C_BLOB, getting rid of severely
   * overlapping outlines and those that are children of a bigger one.
   *
   * The output is a list of C_BLOBs that are owned by the list.
   *
   * The C_OUTLINEs in the C_BLOBs contain no outline data - just empty
   * bounding boxes. The Boxa is consumed and destroyed.
   */
  static void ConvertBoxaToBlobs(int image_width, int image_height,
                                 Boxa** boxes, C_BLOB_LIST* blobs);

 private:
  /**
   * Finds vertical lines in the given list of BLOBNBOXes. bleft and tright
   * are the bounds of the image on which the input line_bblobs were found.
   *
   * The input line_bblobs list is const really.
   *
   * The output vertical_x and vertical_y are the total of all the vectors.
   * The output list of TabVector makes no reference to the input BLOBNBOXes.
   */
  static void FindLineVectors(const ICOORD& bleft, const ICOORD& tright,
                              BLOBNBOX_LIST* line_bblobs,
                              int* vertical_x, int* vertical_y,
                              TabVector_LIST* vectors);

  /**
   * Get a set of bounding boxes of possible vertical lines in the image.
   *
   * The input resolution overrides any resolution set in src_pix.
   *
   * The output line_pix contains just all the detected lines.
   */
  static Boxa* GetVLineBoxes(int resolution, Pix* src_pix, Pix** line_pix);

  /**
   * Get a set of bounding boxes of possible horizontal lines in the image.
   *
   * The input resolution overrides any resolution set in src_pix.
   *
   * The output line_pix contains just all the detected lines.
   *
   * The output boxes undergo the transformation (x,y)->(height-y,x) so the
   * lines can be found with a vertical line finder afterwards.
   *
   * This transformation allows a simple x/y flip to reverse it in tesseract
   * coordinates and it is faster to flip the lines than rotate the image.
   */
  static Boxa* GetHLineBoxes(int resolution, Pix* src_pix, Pix** line_pix);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_LINEFIND_H__


