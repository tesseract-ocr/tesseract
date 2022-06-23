///////////////////////////////////////////////////////////////////////
// File:        imagefind.h
// Description: Class to find image and drawing regions in an image
//              and create a corresponding list of empty blobs.
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

#ifndef TESSERACT_TEXTORD_IMAGEFIND_H_
#define TESSERACT_TEXTORD_IMAGEFIND_H_

#include "debugpixa.h"

#include <cstdint>

namespace tesseract {

class ColPartitionGrid;
class ColPartition_LIST;
class TabFind;
class TBOX;
class FCOORD;
class TO_BLOCK;

// The ImageFind class is a simple static function wrapper class that
// exposes the FindImages function and some useful helper functions.
class ImageFind {
public:
  // Finds image regions within the BINARY source pix (page image) and returns
  // the image regions as a mask image.
  // The returned pix may be nullptr, meaning no images found.
  // If not nullptr, it must be PixDestroyed by the caller.
  // If textord_tabfind_show_images, debug images are appended to pixa_debug.
  static Image FindImages(Image pix, DebugPixa *pixa_debug);

  // Given an input pix, and a bounding rectangle, the sides of the rectangle
  // are shrunk inwards until they bound any black pixels found within the
  // original rectangle. Returns false if the rectangle contains no black
  // pixels at all.
  static bool BoundsWithinRect(Image pix, int *x_start, int *y_start, int *x_end, int *y_end);

  // Given a point in 3-D (RGB) space, returns the squared Euclidean distance
  // of the point from the given line, defined by a pair of points in the 3-D
  // (RGB) space, line1 and line2.
  static double ColorDistanceFromLine(const uint8_t *line1, const uint8_t *line2,
                                      const uint8_t *point);

  // Returns true if there are no black pixels in between the boxes.
  // The im_box must represent the bounding box of the pix in tesseract
  // coordinates, which may be negative, due to rotations to make the textlines
  // horizontal. The boxes are rotated by rotation, which should undo such
  // rotations, before mapping them onto the pix.
  static bool BlankImageInBetween(const TBOX &box1, const TBOX &box2, const TBOX &im_box,
                                  const FCOORD &rotation, Image pix);

  // Returns the number of pixels in box in the pix.
  // The im_box must represent the bounding box of the pix in tesseract
  // coordinates, which may be negative, due to rotations to make the textlines
  // horizontal. The boxes are rotated by rotation, which should undo such
  // rotations, before mapping them onto the pix.
  static int CountPixelsInRotatedBox(TBOX box, const TBOX &im_box, const FCOORD &rotation,
                                     Image pix);

  // Locates all the image partitions in the part_grid, that were found by a
  // previous call to FindImagePartitions, marks them in the image_mask,
  // removes them from the grid, and deletes them. This makes it possible to
  // call FindImagePartitions again to produce less broken-up and less
  // overlapping image partitions.
  // rerotation specifies how to rotate the partition coords to match
  // the image_mask, since this function is used after orientation correction.
  static void TransferImagePartsToImageMask(const FCOORD &rerotation, ColPartitionGrid *part_grid,
                                            Image image_mask);

  // Runs a CC analysis on the image_pix mask image, and creates
  // image partitions from them, cutting out strong text, and merging with
  // nearby image regions such that they don't interfere with text.
  // Rotation and rerotation specify how to rotate image coords to match
  // the blob and partition coords and back again.
  // The input/output part_grid owns all the created partitions, and
  // the partitions own all the fake blobs that belong in the partitions.
  // Since the other blobs in the other partitions will be owned by the block,
  // ColPartitionGrid::ReTypeBlobs must be called afterwards to fix this
  // situation and collect the image blobs.
  static void FindImagePartitions(Image image_pix, const FCOORD &rotation, const FCOORD &rerotation,
                                  TO_BLOCK *block, TabFind *tab_grid, DebugPixa *pixa_debug,
                                  ColPartitionGrid *part_grid, ColPartition_LIST *big_parts);
};

} // namespace tesseract.

#endif // TESSERACT_TEXTORD_LINEFIND_H_
