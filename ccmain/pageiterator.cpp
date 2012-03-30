///////////////////////////////////////////////////////////////////////
// File:        pageiterator.cpp
// Description: Iterator for tesseract page structure that avoids using
//              tesseract internal data structures.
// Author:      Ray Smith
// Created:     Fri Feb 26 14:32:09 PST 2010
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

#include "pageiterator.h"
#include "allheaders.h"
#include "helpers.h"
#include "pageres.h"
#include "tesseractclass.h"

namespace tesseract {

PageIterator::PageIterator(PAGE_RES* page_res, Tesseract* tesseract,
                           int scale, int scaled_yres,
                           int rect_left, int rect_top,
                           int rect_width, int rect_height)
  : page_res_(page_res), tesseract_(tesseract),
    word_(NULL), word_length_(0), blob_index_(0), cblob_it_(NULL),
    scale_(scale), scaled_yres_(scaled_yres),
    rect_left_(rect_left), rect_top_(rect_top),
    rect_width_(rect_width), rect_height_(rect_height) {
  it_ = new PAGE_RES_IT(page_res);
  PageIterator::Begin();
}

PageIterator::~PageIterator() {
  delete it_;
  delete cblob_it_;
}

/**
 * PageIterators may be copied! This makes it possible to iterate over
 * all the objects at a lower level, while maintaining an iterator to
 * objects at a higher level.
 */
PageIterator::PageIterator(const PageIterator& src)
  : page_res_(src.page_res_), tesseract_(src.tesseract_),
    word_(NULL), word_length_(src.word_length_),
    blob_index_(src.blob_index_), cblob_it_(NULL),
    scale_(src.scale_), scaled_yres_(src.scaled_yres_),
    rect_left_(src.rect_left_), rect_top_(src.rect_top_),
    rect_width_(src.rect_width_), rect_height_(src.rect_height_) {
  it_ = new PAGE_RES_IT(*src.it_);
  BeginWord(src.blob_index_);
}

const PageIterator& PageIterator::operator=(const PageIterator& src) {
  page_res_ = src.page_res_;
  tesseract_ = src.tesseract_;
  scale_ = src.scale_;
  scaled_yres_ = src.scaled_yres_;
  rect_left_ = src.rect_left_;
  rect_top_ = src.rect_top_;
  rect_width_ = src.rect_width_;
  rect_height_ = src.rect_height_;
  if (it_ != NULL) delete it_;
  it_ = new PAGE_RES_IT(*src.it_);
  BeginWord(src.blob_index_);
  return *this;
}

bool PageIterator::PositionedAtSameWord(const PAGE_RES_IT* other) const {
  return (it_ == NULL && it_ == other) ||
     ((other != NULL) && (it_ != NULL) && (*it_ == *other));
}

// ============= Moving around within the page ============.

/** Resets the iterator to point to the start of the page. */
void PageIterator::Begin() {
  it_->restart_page_with_empties();
  BeginWord(0);
}

void PageIterator::RestartParagraph() {
  if (it_->block() == NULL) return; // At end of the document.
  PAGE_RES_IT para(page_res_);
  PAGE_RES_IT next_para(para);
  next_para.forward_paragraph();
  while (next_para.cmp(*it_) <= 0) {
    para = next_para;
    next_para.forward_paragraph();
  }
  *it_ = para;
  BeginWord(0);
}

bool PageIterator::IsWithinFirstTextlineOfParagraph() const {
  PageIterator p_start(*this);
  p_start.RestartParagraph();
  return p_start.it_->row() == it_->row();
}

void PageIterator::RestartRow() {
  it_->restart_row();
  BeginWord(0);
}

/**
 * Moves to the start of the next object at the given level in the
 * page hierarchy, and returns false if the end of the page was reached.
 * NOTE (CHANGED!) that ALL PageIteratorLevel level values will visit each
 * non-text block at least once.
 * Think of non text blocks as containing a single para, with at least one
 * line, with a single imaginary word, containing a single symbol.
 * The bounding boxes mark out any polygonal nature of the block, and
 * PTIsTextType(BLockType()) is false for non-text blocks.
 * Calls to Next with different levels may be freely intermixed.
 * This function iterates words in right-to-left scripts correctly, if
 * the appropriate language has been loaded into Tesseract.
 */
bool PageIterator::Next(PageIteratorLevel level) {
  if (it_->block() == NULL) return false;  // Already at the end!
  if (it_->word() == NULL)
    level = RIL_BLOCK;

  switch (level) {
    case RIL_BLOCK:
      it_->forward_block();
      break;
    case RIL_PARA:
      it_->forward_paragraph();
      break;
    case RIL_TEXTLINE:
      for (it_->forward_with_empties(); it_->row() == it_->prev_row();
           it_->forward_with_empties());
      break;
    case RIL_WORD:
      it_->forward_with_empties();
      break;
    case RIL_SYMBOL:
      if (cblob_it_ != NULL)
        cblob_it_->forward();
      ++blob_index_;
      if (blob_index_ >= word_length_)
        it_->forward_with_empties();
      else
        return true;
      break;
  }
  BeginWord(0);
  return it_->block() != NULL;
}

/**
 * Returns true if the iterator is at the start of an object at the given
 * level. Possible uses include determining if a call to Next(RIL_WORD)
 * moved to the start of a RIL_PARA.
 */
bool PageIterator::IsAtBeginningOf(PageIteratorLevel level) const {
  if (it_->block() == NULL) return false;  // Already at the end!
  if (it_->word() == NULL) return true;  // In an image block.
  switch (level) {
    case RIL_BLOCK:
      return blob_index_ == 0 && it_->block() != it_->prev_block();
    case RIL_PARA:
      return blob_index_ == 0 &&
          (it_->block() != it_->prev_block() ||
           it_->row()->row->para() != it_->prev_row()->row->para());
    case RIL_TEXTLINE:
      return blob_index_ == 0 && it_->row() != it_->prev_row();
    case RIL_WORD:
      return blob_index_ == 0;
    case RIL_SYMBOL:
      return true;
  }
  return false;
}

/**
 * Returns whether the iterator is positioned at the last element in a
 * given level. (e.g. the last word in a line, the last line in a block)
 */
bool PageIterator::IsAtFinalElement(PageIteratorLevel level,
                                    PageIteratorLevel element) const {
  if (Empty(element)) return true;  // Already at the end!
  // The result is true if we step forward by element and find we are
  // at the the end of the page or at beginning of *all* levels in:
  // [level, element).
  // When there is more than one level difference between element and level,
  // we could for instance move forward one symbol and still be at the first
  // word on a line, so we also have to be at the first symbol in a word.
  PageIterator next(*this);
  next.Next(element);
  if (next.Empty(element)) return true;  // Reached the end of the page.
  while (element > level) {
    element = static_cast<PageIteratorLevel>(element - 1);
    if (!next.IsAtBeginningOf(element))
      return false;
  }
  return true;
}

/**
 * Returns whether this iterator is positioned
 *   before other:   -1
 *   equal to other:  0
 *   after other:     1
 */
int PageIterator::Cmp(const PageIterator &other) const {
  int word_cmp = it_->cmp(*other.it_);
  if (word_cmp != 0)
    return word_cmp;
  if (blob_index_ < other.blob_index_)
    return -1;
  if (blob_index_ == other.blob_index_)
    return 0;
  return 1;
}

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

/**
 * Returns the bounding rectangle of the current object at the given level in
 * the coordinates of the working image that is pix_binary().
 * See comment on coordinate system above.
 * Returns false if there is no such object at the current position.
 */
bool PageIterator::BoundingBoxInternal(PageIteratorLevel level,
                                       int* left, int* top,
                                       int* right, int* bottom) const {
  if (Empty(level))
    return false;
  TBOX box;
  PARA *para = NULL;
  switch (level) {
    case RIL_BLOCK:
      box = it_->block()->block->bounding_box();
      break;
    case RIL_PARA:
      para = it_->row()->row->para();
      // explicit fall-through.
    case RIL_TEXTLINE:
      box = it_->row()->row->bounding_box();
      break;
    case RIL_WORD:
      box = it_->word()->word->bounding_box();
      break;
    case RIL_SYMBOL:
      if (cblob_it_ == NULL)
        box = it_->word()->box_word->BlobBox(blob_index_);
      else
        box = cblob_it_->data()->bounding_box();
  }
  if (level == RIL_PARA) {
    PageIterator other = *this;
    other.Begin();
    do {
      if (other.it_->block() &&
          other.it_->block()->block == it_->block()->block &&
          other.it_->row() && other.it_->row()->row &&
          other.it_->row()->row->para() == para) {
        box = box.bounding_union(other.it_->row()->row->bounding_box());
      }
    } while (other.Next(RIL_TEXTLINE));
  }
  if (level != RIL_SYMBOL || cblob_it_ != NULL)
    box.rotate(it_->block()->block->re_rotation());
  // Now we have a box in tesseract coordinates relative to the image rectangle,
  // we have to convert the coords to a top-down system.
  const int pix_height = pixGetHeight(tesseract_->pix_binary());
  const int pix_width = pixGetWidth(tesseract_->pix_binary());
  *left = ClipToRange(static_cast<int>(box.left()), 0, pix_width);
  *top = ClipToRange(pix_height - box.top(), 0, pix_height);
  *right = ClipToRange(static_cast<int>(box.right()), *left, pix_width);
  *bottom = ClipToRange(pix_height - box.bottom(), *top, pix_height);
  return true;
}

/**
 * Returns the bounding rectangle of the current object at the given level in
 * coordinates of the original image.
 * See comment on coordinate system above.
 * Returns false if there is no such object at the current position.
 */
bool PageIterator::BoundingBox(PageIteratorLevel level,
                               int* left, int* top,
                               int* right, int* bottom) const {
  if (!BoundingBoxInternal(level, left, top, right, bottom))
    return false;
  // Convert to the coordinate system of the original image.
  *left = ClipToRange(*left / scale_ + rect_left_,
                      rect_left_, rect_left_ + rect_width_);
  *top = ClipToRange(*top / scale_ + rect_top_,
                     rect_top_, rect_top_ + rect_height_);
  *right = ClipToRange((*right + scale_ - 1) / scale_ + rect_left_,
                       *left, rect_left_ + rect_width_);
  *bottom = ClipToRange((*bottom + scale_ - 1) / scale_ + rect_top_,
                        *top, rect_top_ + rect_height_);
  return true;
}

/** Return that there is no such object at a given level. */
bool PageIterator::Empty(PageIteratorLevel level) const {
  if (it_->block() == NULL) return true;  // Already at the end!
  if (it_->word() == NULL && level != RIL_BLOCK) return true;  // image block
  if (level == RIL_SYMBOL && blob_index_ >= word_length_)
    return true;  // Zero length word, or already at the end of it.
  return false;
}

/** Returns the type of the current block. See apitypes.h for PolyBlockType. */
PolyBlockType PageIterator::BlockType() const {
  if (it_->block() == NULL || it_->block()->block == NULL)
    return PT_UNKNOWN;  // Already at the end!
  if (it_->block()->block->poly_block() == NULL)
    return PT_FLOWING_TEXT;  // No layout analysis used - assume text.
  return it_->block()->block->poly_block()->isA();
}

/**
 * Returns a binary image of the current object at the given level.
 * The position and size match the return from BoundingBoxInternal, and so this
 * could be upscaled with respect to the original input image.
 * Use pixDestroy to delete the image after use.
 * The following methods are used to generate the images:
 * RIL_BLOCK: mask the page image with the block polygon.
 * RIL_TEXTLINE: Clip the rectangle of the line box from the page image.
 * TODO(rays) fix this to generate and use a line polygon.
 * RIL_WORD: Clip the rectangle of the word box from the page image.
 * RIL_SYMBOL: Render the symbol outline to an image for cblobs (prior
 * to recognition) or the bounding box otherwise.
 * A reconstruction of the original image (using xor to check for double
 * representation) should be reasonably accurate,
 * apart from removed noise, at the block level. Below the block level, the
 * reconstruction will be missing images and line separators.
 * At the symbol level, kerned characters will be invade the bounding box
 * if rendered after recognition, making an xor reconstruction inaccurate, but
 * an or construction better. Before recognition, symbol-level reconstruction
 * should be good, even with xor, since the images come from the connected
 * components.
 */
Pix* PageIterator::GetBinaryImage(PageIteratorLevel level) const {
  int left, top, right, bottom;
  if (!BoundingBoxInternal(level, &left, &top, &right, &bottom))
    return NULL;
  Pix* pix = NULL;
  switch (level) {
    case RIL_BLOCK:
    case RIL_PARA:
      int bleft, btop, bright, bbottom;
      BoundingBoxInternal(RIL_BLOCK, &bleft, &btop, &bright, &bbottom);
      pix = it_->block()->block->render_mask();
      // AND the mask and the image.
      pixRasterop(pix, 0, 0, pixGetWidth(pix), pixGetHeight(pix),
                  PIX_SRC & PIX_DST, tesseract_->pix_binary(),
                  bleft, btop);
      if (level == RIL_PARA) {
        // RIL_PARA needs further attention:
        //   clip the paragraph from the block mask.
        Box* box = boxCreate(left - bleft, top - btop,
                             right - left, bottom - top);
        Pix* pix2 = pixClipRectangle(pix, box, NULL);
        boxDestroy(&box);
        pixDestroy(&pix);
        pix = pix2;
      }
      break;
    case RIL_TEXTLINE:
    case RIL_WORD:
    case RIL_SYMBOL:
      if (level == RIL_SYMBOL && cblob_it_ != NULL &&
          cblob_it_->data()->area() != 0)
        return cblob_it_->data()->render();
      // Just clip from the bounding box.
      Box* box = boxCreate(left, top, right - left, bottom - top);
      pix = pixClipRectangle(tesseract_->pix_binary(), box, NULL);
      boxDestroy(&box);
      break;
  }
  return pix;
}

/**
 * Returns an image of the current object at the given level in greyscale
 * if available in the input. To guarantee a binary image use BinaryImage.
 * NOTE that in order to give the best possible image, the bounds are
 * expanded slightly over the binary connected component, by the supplied
 * padding, so the top-left position of the returned image is returned
 * in (left,top). These will most likely not match the coordinates
 * returned by BoundingBox.
 * Use pixDestroy to delete the image after use.
 */
Pix* PageIterator::GetImage(PageIteratorLevel level, int padding,
                            int* left, int* top) const {
  int right, bottom;
  if (!BoundingBox(level, left, top, &right, &bottom))
    return NULL;
  Pix* pix = tesseract_->pix_grey();
  if (pix == NULL)
    return GetBinaryImage(level);

  // Expand the box.
  *left = MAX(*left - padding, 0);
  *top = MAX(*top - padding, 0);
  right = MIN(right + padding, rect_width_);
  bottom = MIN(bottom + padding, rect_height_);
  Box* box = boxCreate(*left, *top, right - *left, bottom - *top);
  Pix* grey_pix = pixClipRectangle(pix, box, NULL);
  boxDestroy(&box);
  if (level == RIL_BLOCK) {
    Pix* mask = it_->block()->block->render_mask();
    Pix* expanded_mask = pixCreate(right - *left, bottom - *top, 1);
    pixRasterop(expanded_mask, padding, padding,
                pixGetWidth(mask), pixGetHeight(mask),
                PIX_SRC, mask, 0, 0);
    pixDestroy(&mask);
    pixDilateBrick(expanded_mask, expanded_mask, 2*padding + 1, 2*padding + 1);
    pixInvert(expanded_mask, expanded_mask);
    pixSetMasked(grey_pix, expanded_mask, 255);
    pixDestroy(&expanded_mask);
  }
  return grey_pix;
}

/**
 * Returns the baseline of the current object at the given level.
 * The baseline is the line that passes through (x1, y1) and (x2, y2).
 * WARNING: with vertical text, baselines may be vertical!
 */
bool PageIterator::Baseline(PageIteratorLevel level,
                            int* x1, int* y1, int* x2, int* y2) const {
  if (it_->word() == NULL) return false;  // Already at the end!
  ROW* row = it_->row()->row;
  WERD* word = it_->word()->word;
  TBOX box = (level == RIL_WORD || level == RIL_SYMBOL)
           ? word->bounding_box()
           : row->bounding_box();
  int left = box.left();
  ICOORD startpt(left, static_cast<inT16>(row->base_line(left) + 0.5));
  int right = box.right();
  ICOORD endpt(right, static_cast<inT16>(row->base_line(right) + 0.5));
  // Rotate to image coordinates and convert to global image coords.
  startpt.rotate(it_->block()->block->re_rotation());
  endpt.rotate(it_->block()->block->re_rotation());
  *x1 = startpt.x() / scale_ + rect_left_;
  *y1 = (rect_height_ - startpt.y()) / scale_ + rect_top_;
  *x2 = endpt.x() / scale_ + rect_left_;
  *y2 = (rect_height_ - endpt.y()) / scale_ + rect_top_;
  return true;
}

void PageIterator::Orientation(tesseract::Orientation *orientation,
                               tesseract::WritingDirection *writing_direction,
                               tesseract::TextlineOrder *textline_order,
                               float *deskew_angle) const {
  BLOCK* block = it_->block()->block;

  // Orientation
  FCOORD up_in_image(0.0, 1.0);
  up_in_image.unrotate(block->classify_rotation());
  up_in_image.rotate(block->re_rotation());

  if (up_in_image.x() == 0.0F) {
    if (up_in_image.y() > 0.0F) {
      *orientation = ORIENTATION_PAGE_UP;
    } else {
      *orientation = ORIENTATION_PAGE_DOWN;
    }
  } else if (up_in_image.x() > 0.0F) {
    *orientation = ORIENTATION_PAGE_RIGHT;
  } else {
    *orientation = ORIENTATION_PAGE_LEFT;
  }

  // Writing direction
  bool is_vertical_text = (block->classify_rotation().x() == 0.0);
  bool right_to_left = block->right_to_left();
  *writing_direction =
      is_vertical_text
          ? WRITING_DIRECTION_TOP_TO_BOTTOM
          : (right_to_left
                ? WRITING_DIRECTION_RIGHT_TO_LEFT
                : WRITING_DIRECTION_LEFT_TO_RIGHT);

  // Textline Order
  bool is_mongolian = false;  // TODO(eger): fix me
  *textline_order = is_vertical_text
      ? (is_mongolian
         ? TEXTLINE_ORDER_LEFT_TO_RIGHT
         : TEXTLINE_ORDER_RIGHT_TO_LEFT)
      : TEXTLINE_ORDER_TOP_TO_BOTTOM;

  // Deskew angle
  FCOORD skew = block->skew();  // true horizontal for textlines
  *deskew_angle = -skew.angle();
}

void PageIterator::ParagraphInfo(tesseract::ParagraphJustification *just,
                                 bool *is_list_item,
                                 bool *is_crown,
                                 int *first_line_indent) const {
  *just = tesseract::JUSTIFICATION_UNKNOWN;
  if (!it_->row() || !it_->row()->row || !it_->row()->row->para() ||
      !it_->row()->row->para()->model)
    return;

  PARA *para = it_->row()->row->para();
  *is_list_item = para->is_list_item;
  *is_crown = para->is_very_first_or_continuation;
  *first_line_indent = para->model->first_indent() -
      para->model->body_indent();
}

/**
 * Sets up the internal data for iterating the blobs of a new word, then
 * moves the iterator to the given offset.
 */
void PageIterator::BeginWord(int offset) {
  WERD_RES* word_res = it_->word();
  if (word_res == NULL) {
    // This is a non-text block, so there is no word.
    word_length_ = 0;
    blob_index_ = 0;
    word_ = NULL;
    return;
  }
  if (word_res->best_choice != NULL) {
    // Recognition has been done, so we are using the box_word, which
    // is already baseline denormalized.
    word_length_ = word_res->best_choice->length();
    ASSERT_HOST(word_res->box_word != NULL);
    if (word_res->box_word->length() != word_length_) {
      tprintf("Corrupted word! best_choice[len=%d] = %s, box_word[len=%d]: ",
              word_length_, word_res->best_choice->unichar_string().string(),
              word_res->box_word->length());
      word_res->box_word->bounding_box().print();
    }
    ASSERT_HOST(word_res->box_word->length() == word_length_);
    word_ = NULL;
    // We will be iterating the box_word.
    if (cblob_it_ != NULL) {
      delete cblob_it_;
      cblob_it_ = NULL;
    }
  } else {
    // No recognition yet, so a "symbol" is a cblob.
    word_ = word_res->word;
    ASSERT_HOST(word_->cblob_list() != NULL);
    word_length_ = word_->cblob_list()->length();
    if (cblob_it_ == NULL) cblob_it_ = new C_BLOB_IT;
    cblob_it_->set_to_list(word_->cblob_list());
  }
  for (blob_index_ = 0; blob_index_ < offset; ++blob_index_) {
    if (cblob_it_ != NULL)
      cblob_it_->forward();
  }
}

}  // namespace tesseract.
