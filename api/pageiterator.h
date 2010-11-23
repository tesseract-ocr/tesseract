///////////////////////////////////////////////////////////////////////
// File:        pageiterator.h
// Description: Iterator for tesseract page structure that avoids using
//              tesseract internal data structures.
// Author:      Ray Smith
// Created:     Fri Feb 26 11:01:06 PST 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TESSERACT_API_PAGEITERATOR_H__
#define TESSERACT_API_PAGEITERATOR_H__

#include "apitypes.h"

class C_BLOB_IT;
class PBLOB_IT;
class PAGE_RES;
class PAGE_RES_IT;
class WERD;
struct Pix;

namespace tesseract {

class Tesseract;

// Class to iterate over tesseract page structure, providing access to all
// levels of the page hierarchy, without including any tesseract headers or
// having to handle any tesseract structures.
// WARNING! This class points to data held within the TessBaseAPI class, and
// therefore can only be used while the TessBaseAPI class still exists and
// has not been subjected to a call of Init, SetImage, Recognize, Clear, End
// DetectOS, or anything else that changes the internal PAGE_RES.
// See apitypes.h for the definition of PageIteratorLevel.
// See also ResultIterator, derived from PageIterator, which adds in the
// ability to access OCR output with text-specific methods.

class PageIterator {
 public:
  // page_res and tesseract come directly from the BaseAPI.
  // The rectangle parameters are copied indirectly from the Thresholder,
  // via the BaseAPI. They represent the coordinates of some rectangle in an
  // original image (in top-left-origin coordinates) and therefore the top-left
  // needs to be added to any output boxes in order to specify coordinates
  // in the original image. See TessBaseAPI::SetRectangle.
  // The scale and scaled_yres are in case the Thresholder scaled the image
  // rectangle prior to thresholding. Any coordinates in tesseract's image
  // must be divided by scale before adding (rect_left, rect_top).
  // The scaled_yres indicates the effective resolution of the binary image
  // that tesseract has been given by the Thresholder.
  // After the constructor, Begin has already been called.
  PageIterator(PAGE_RES* page_res, Tesseract* tesseract,
               int scale, int scaled_yres,
               int rect_left, int rect_top,
               int rect_width, int rect_height);
  virtual ~PageIterator();

  // Page/ResultIterators may be copied! This makes it possible to iterate over
  // all the objects at a lower level, while maintaining an iterator to
  // objects at a higher level. These constructors DO NOT CALL Begin, so
  // iterations will continue from the location of src.
  PageIterator(const PageIterator& src);
  const PageIterator& operator=(const PageIterator& src);

  // ============= Moving around within the page ============.

  // Moves the iterator to point to the start of the page to begin an iteration.
  void Begin();

  // Moves to the start of the next object at the given level in the
  // page hierarchy, and returns false if the end of the page was reached.
  // NOTE that RIL_SYMBOL will skip non-text blocks, but all other
  // PageIteratorLevel level values will visit each non-text block once.
  // Think of non text blocks as containing a single para, with a single line,
  // with a single imaginary word.
  // Calls to Next with different levels may be freely intermixed.
  // This function iterates words in right-to-left scripts correctly, if
  // the appropriate language has been loaded into Tesseract.
  bool Next(PageIteratorLevel level);

  // Returns true if the iterator is at the start of an object at the given
  // level. Possible uses include determining if a call to Next(RIL_WORD)
  // moved to the start of a RIL_PARA.
  bool IsAtBeginningOf(PageIteratorLevel level) const;

  // Returns whether the iterator is positioned at the last element in a
  // given level. (e.g. the last word in a line, the last line in a block)
  bool IsAtFinalElement(PageIteratorLevel level,
                        PageIteratorLevel element) const;

  // ============= Accessing data ==============.
  // Coordinate system:
  // Integer coordinates are at the cracks between the pixels.
  // The top-left corner of the top-left pixel in the image is at (0,0).
  // The bottom-right corner of the bottom-right pixel in the image is at
  // (width, height).
  // Every bounding box goes from the top-left of the top-left contained
  // pixel to the bottom-right of the bottom-right contained pixel, so
  // the bounding box of the single top-left pixel in the image is:
  // (0,0)->(1,1).
  // If an image rectangle has been set in the API, then returned coordinates
  // relate to the original (full) image, rather than the rectangle.

  // Returns the bounding rectangle of the current object at the given level.
  // See comment on coordinate system above.
  // Returns false if there is no such object at the current position.
  // The returned bounding box is guaranteed to match the size and position
  // of the image returned by GetBinaryImage, but may clip foreground pixels
  // from a grey image. The padding argument to GetImage can be used to expand
  // the image to include more foreground pixels. See GetImage below.
  bool BoundingBox(PageIteratorLevel level,
                   int* left, int* top, int* right, int* bottom) const;

  // Returns the type of the current block. See apitypes.h for PolyBlockType.
  PolyBlockType BlockType() const;

  // Returns a binary image of the current object at the given level.
  // The position and size match the return from BoundingBox.
  // Use pixDestroy to delete the image after use.
  Pix* GetBinaryImage(PageIteratorLevel level) const;

  // Returns an image of the current object at the given level in greyscale
  // if available in the input. To guarantee a binary image use BinaryImage.
  // NOTE that in order to give the best possible image, the bounds are
  // expanded slightly over the binary connected component, by the supplied
  // padding, so the top-left position of the returned image is returned
  // in (left,top). These will most likely not match the coordinates
  // returned by BoundingBox.
  // Use pixDestroy to delete the image after use.
  Pix* GetImage(PageIteratorLevel level, int padding,
                int* left, int* top) const;

  // Returns the baseline of the current object at the given level.
  // The baseline is the line that passes through (x1, y1) and (x2, y2).
  // WARNING: with vertical text, baselines may be vertical!
  // Returns false if there is no baseline at the current position.
  bool Baseline(PageIteratorLevel level,
                int* x1, int* y1, int* x2, int* y2) const;

 protected:
  // Sets up the internal data for iterating the blobs of a new word, then
  // moves the iterator to the given offset.
  void BeginWord(int offset);

  // Pointer to the page_res owned by the API.
  PAGE_RES* page_res_;
  // Pointer to the Tesseract object owned by the API.
  Tesseract* tesseract_;
  // The iterator to the page_res_. Owned by this ResultIterator.
  // A pointer just to avoid dragging in Tesseract includes.
  PAGE_RES_IT* it_;
  // The current input WERD being iterated. If there is an output from OCR,
  // then word_ is NULL. Owned by the API.
  WERD* word_;
  // The length of the current word_.
  int word_length_;
  // The current blob index within the word.
  int blob_index_;
  // Iterator to the blobs within the word. If NULL, then we are iterating
  // OCR results in the box_word.
  // Owned by this ResultIterator.
  C_BLOB_IT* cblob_it_;
  // Parameters saved from the Thresholder. Needed to rebuild coordinates.
  int scale_;
  int scaled_yres_;
  int rect_left_;
  int rect_top_;
  int rect_width_;
  int rect_height_;
};

}  // namespace tesseract.

#endif  // TESSERACT_API_PAGEITERATOR_H__
