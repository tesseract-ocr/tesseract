///////////////////////////////////////////////////////////////////////
// File:        alignedblob.h
// Description: A class to find vertically aligned blobs in a BBGrid,
//              and a struct to hold control parameters.
// Author:      Ray Smith
// Created:     Fri Mar 21 15:03:01 PST 2008
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

#ifndef TESSERACT_TEXTORD_ALIGNEDBLOB_H_
#define TESSERACT_TEXTORD_ALIGNEDBLOB_H_

#include "bbgrid.h"
#include "blobbox.h"
#include "strngs.h"
#include "tabvector.h"

extern INT_VAR_H(textord_debug_bugs, 0,
                 "Turn on output related to bugs in tab finding");
extern INT_VAR_H(textord_debug_tabfind, 2, "Debug tab finding");
extern BOOL_VAR_H(textord_debug_printable, false,
                  "Make debug windows printable");

namespace tesseract {

// Simple structure to hold the search parameters for AlignedBlob.
// The members are mostly derived from constants, which are
// conditioned on the alignment parameter.
// For finding vertical lines, a different set of constants are
// used, conditioned on the different constructor.
struct AlignedBlobParams {
  // Constructor to set the parameters for finding aligned and ragged tabs.
  // Vertical_x and vertical_y are the current estimates of the true vertical
  // direction (up) in the image. Height is the height of the starter blob.
  // v_gap_multiple is the multiple of height that will be used as a limit
  // on vertical gap before giving up and calling the line ended.
  // resolution is the original image resolution, and align0 indicates the
  // type of tab stop to be found.
  AlignedBlobParams(int vertical_x, int vertical_y, int height,
                    int v_gap_multiple, int min_gutter_width, int resolution,
                    TabAlignment alignment0);
  // Constructor to set the parameters for finding vertical lines.
  // Vertical_x and vertical_y are the current estimates of the true vertical
  // direction (up) in the image. Width is the width of the starter blob.
  AlignedBlobParams(int vertical_x, int vertical_y, int width);

  // Fit the vertical vector into an ICOORD, which is 16 bit.
  void set_vertical(int vertical_x, int vertical_y);

  double gutter_fraction;  // Multiple of height used for min_gutter.
  bool right_tab;          // We are looking at right edges.
  bool ragged;             // We are looking for a ragged (vs aligned) edge.
  TabAlignment alignment;  // The type we are trying to produce.
  TabType confirmed_type;  // Type to flag blobs if accepted.
  int max_v_gap;           // Max vertical gap to be tolerated.
  int min_gutter;          // Minimum gutter between columns.
  // Tolerances allowed on horizontal alignment of aligned edges.
  int l_align_tolerance;   // Left edges.
  int r_align_tolerance;   // Right edges.
  // Conditions for accepting a line.
  int min_points;          // Minimum number of points to be OK.
  int min_length;          // Min length of completed line.

  ICOORD vertical;         // Current estimate of logical vertical.
};

// The AlignedBlob class contains code to find vertically aligned blobs.
// This is factored out into a separate class, so it can be used by both
// vertical line finding (LineFind) and tabstop finding (TabFind).
class AlignedBlob : public BlobGrid {
 public:
  AlignedBlob(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~AlignedBlob();

  // Return true if the given coordinates are within the test rectangle
  // and the debug level is at least the given detail level.
  static bool WithinTestRegion(int detail_level, int x, int y);

  // Display the tab codes of the BLOBNBOXes in this grid.
  ScrollView* DisplayTabs(const char* window_name, ScrollView* tab_win);

  // Finds a vector corresponding to a set of vertically aligned blob edges
  // running through the given box. The type of vector returned and the
  // search parameters are determined by the AlignedBlobParams.
  // vertical_x and y are updated with an estimate of the real
  // vertical direction. (skew finding.)
  // Returns nullptr if no decent vector can be found.
  TabVector* FindVerticalAlignment(AlignedBlobParams align_params,
                                   BLOBNBOX* bbox,
                                   int* vertical_x, int* vertical_y);

 private:
  // Find a set of blobs that are aligned in the given vertical
  // direction with the given blob. Returns a list of aligned
  // blobs and the number in the list.
  // For other parameters see FindAlignedBlob below.
  int AlignTabs(const AlignedBlobParams& params,
                bool top_to_bottom, BLOBNBOX* bbox,
                BLOBNBOX_CLIST* good_points, int* end_y);

  // Search vertically for a blob that is aligned with the input bbox.
  // The search parameters are determined by AlignedBlobParams.
  // top_to_bottom tells whether to search down or up.
  // The return value is nullptr if nothing was found in the search box
  // or if a blob was found in the gutter. On a nullptr return, end_y
  // is set to the edge of the search box or the leading edge of the
  // gutter blob if one was found.
  BLOBNBOX* FindAlignedBlob(const AlignedBlobParams& p,
                            bool top_to_bottom, BLOBNBOX* bbox,
                            int x_start, int* end_y);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_ALIGNEDBLOB_H_
