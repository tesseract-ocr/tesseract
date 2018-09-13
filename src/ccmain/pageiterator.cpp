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

#include <algorithm>

namespace tesseract {

PageIterator::PageIterator(PAGE_RES* page_res, Tesseract* tesseract, int scale,
                           int scaled_yres, int rect_left, int rect_top,
                           int rect_width, int rect_height)
    : page_res_(page_res),
      tesseract_(tesseract),
      word_(nullptr),
      word_length_(0),
      blob_index_(0),
      cblob_it_(nullptr),
      include_upper_dots_(false),
      include_lower_dots_(false),
      scale_(scale),
      scaled_yres_(scaled_yres),
      rect_left_(rect_left),
      rect_top_(rect_top),
      rect_width_(rect_width),
      rect_height_(rect_height) {
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
    : page_res_(src.page_res_),
      tesseract_(src.tesseract_),
      word_(nullptr),
      word_length_(src.word_length_),
      blob_index_(src.blob_index_),
      cblob_it_(nullptr),
      include_upper_dots_(src.include_upper_dots_),
      include_lower_dots_(src.include_lower_dots_),
      scale_(src.scale_),
      scaled_yres_(src.scaled_yres_),
      rect_left_(src.rect_left_),
      rect_top_(src.rect_top_),
      rect_width_(src.rect_width_),
      rect_height_(src.rect_height_) {
  it_ = new PAGE_RES_IT(*src.it_);
  BeginWord(src.blob_index_);
}

const PageIterator& PageIterator::operator=(const PageIterator& src) {
  page_res_ = src.page_res_;
  tesseract_ = src.tesseract_;
  include_upper_dots_ = src.include_upper_dots_;
  include_lower_dots_ = src.include_lower_dots_;
  scale_ = src.scale_;
  scaled_yres_ = src.scaled_yres_;
  rect_left_ = src.rect_left_;
  rect_top_ = src.rect_top_;
  rect_width_ = src.rect_width_;
  rect_height_ = src.rect_height_;
  delete it_;
  it_ = new PAGE_RES_IT(*src.it_);
  BeginWord(src.blob_index_);
  return *this;
}

bool PageIterator::PositionedAtSameWord(const PAGE_RES_IT* other) const {
  return (it_ == nullptr && it_ == other) ||
     ((other != nullptr) && (it_ != nullptr) && (*it_ == *other));
}

// ============= Moving around within the page ============.

/** Resets the iterator to point to the start of the page. */
void PageIterator::Begin() {
  it_->restart_page_with_empties();
  BeginWord(0);
}

void PageIterator::RestartParagraph() {
  if (it_->block() == nullptr) return; // At end of the document.
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
  if (it_->block() == nullptr) return false;  // Already at the end!
  if (it_->word() == nullptr)
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
      if (cblob_it_ != nullptr)
        cblob_it_->forward();
      ++blob_index_;
      if (blob_index_ >= word_length_)
        it_->forward_with_empties();
      else
        return true;
      break;
  }
  BeginWord(0);
  return it_->block() != nullptr;
}

/**
 * Returns true if the iterator is at the start of an object at the given
 * level. Possible uses include determining if a call to Next(RIL_WORD)
 * moved to the start of a RIL_PARA.
 */
bool PageIterator::IsAtBeginningOf(PageIteratorLevel level) const {
  if (it_->block() == nullptr) return false;  // Already at the end!
  if (it_->word() == nullptr) return true;  // In an image block.
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
  PARA *para = nullptr;
  switch (level) {
    case RIL_BLOCK:
      box = it_->block()->block->restricted_bounding_box(include_upper_dots_,
                                                         include_lower_dots_);
      break;
    case RIL_PARA:
      para = it_->row()->row->para();
      // explicit fall-through.
    case RIL_TEXTLINE:
      box = it_->row()->row->restricted_bounding_box(include_upper_dots_,
                                                     include_lower_dots_);
      break;
    case RIL_WORD:
      box = it_->word()->word->restricted_bounding_box(include_upper_dots_,
                                                       include_lower_dots_);
      break;
    case RIL_SYMBOL:
      if (cblob_it_ == nullptr)
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
  if (level != RIL_SYMBOL || cblob_it_ != nullptr)
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
  return BoundingBox(level, 0, left, top, right, bottom);
}

bool PageIterator::BoundingBox(PageIteratorLevel level, const int padding,
                               int* left, int* top,
                               int* right, int* bottom) const {
  if (!BoundingBoxInternal(level, left, top, right, bottom))
    return false;
  // Convert to the coordinate system of the original image.
  *left = ClipToRange(*left / scale_ + rect_left_ - padding,
                      rect_left_, rect_left_ + rect_width_);
  *top = ClipToRange(*top / scale_ + rect_top_ - padding,
                     rect_top_, rect_top_ + rect_height_);
  *right = ClipToRange((*right + scale_ - 1) / scale_ + rect_left_ + padding,
                       *left, rect_left_ + rect_width_);
  *bottom = ClipToRange((*bottom + scale_ - 1) / scale_ + rect_top_ + padding,
                        *top, rect_top_ + rect_height_);
  return true;
}

/** Return that there is no such object at a given level. */
bool PageIterator::Empty(PageIteratorLevel level) const {
  if (it_->block() == nullptr) return true;  // Already at the end!
  if (it_->word() == nullptr && level != RIL_BLOCK) return true;  // image block
  if (level == RIL_SYMBOL && blob_index_ >= word_length_)
    return true;  // Zero length word, or already at the end of it.
  return false;
}

/** Returns the type of the current block. See apitypes.h for PolyBlockType. */
PolyBlockType PageIterator::BlockType() const {
  if (it_->block() == nullptr || it_->block()->block == nullptr)
    return PT_UNKNOWN;  // Already at the end!
  if (it_->block()->block->pdblk.poly_block() == nullptr)
    return PT_FLOWING_TEXT;  // No layout analysis used - assume text.
  return it_->block()->block->pdblk.poly_block()->isA();
}

/** Returns the polygon outline of the current block. The returned Pta must
 *  be ptaDestroy-ed after use. */
Pta* PageIterator::BlockPolygon() const {
  if (it_->block() == nullptr || it_->block()->block == nullptr)
    return nullptr;  // Already at the end!
  if (it_->block()->block->pdblk.poly_block() == nullptr)
    return nullptr;  // No layout analysis used - no polygon.
  ICOORDELT_IT it(it_->block()->block->pdblk.poly_block()->points());
  Pta* pta = ptaCreate(it.length());
  int num_pts = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), ++num_pts) {
    ICOORD* pt = it.data();
    // Convert to top-down coords within the input image.
    float x = static_cast<float>(pt->x()) / scale_ + rect_left_;
    float y = rect_top_ + rect_height_ - static_cast<float>(pt->y()) / scale_;
    ptaAddPt(pta, x, y);
  }
  return pta;
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
    return nullptr;
  if (level == RIL_SYMBOL && cblob_it_ != nullptr &&
      cblob_it_->data()->area() != 0)
    return cblob_it_->data()->render();
  Box* box = boxCreate(left, top, right - left, bottom - top);
  Pix* pix = pixClipRectangle(tesseract_->pix_binary(), box, nullptr);
  boxDestroy(&box);
  if (level == RIL_BLOCK || level == RIL_PARA) {
    // Clip to the block polygon as well.
    TBOX mask_box;
    Pix* mask = it_->block()->block->render_mask(&mask_box);
    int mask_x = left - mask_box.left();
    int mask_y = top - (tesseract_->ImageHeight() - mask_box.top());
    // AND the mask and pix, putting the result in pix.
    pixRasterop(pix, std::max(0, -mask_x), std::max(0, -mask_y), pixGetWidth(pix),
                pixGetHeight(pix), PIX_SRC & PIX_DST, mask, std::max(0, mask_x),
                std::max(0, mask_y));
    pixDestroy(&mask);
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
 * If you do not supply an original image, you will get a binary one.
 * Use pixDestroy to delete the image after use.
 */
Pix* PageIterator::GetImage(PageIteratorLevel level, int padding,
                            Pix* original_img,
                            int* left, int* top) const {
  int right, bottom;
  if (!BoundingBox(level, left, top, &right, &bottom))
    return nullptr;
  if (original_img == nullptr)
    return GetBinaryImage(level);

  // Expand the box.
  *left = std::max(*left - padding, 0);
  *top = std::max(*top - padding, 0);
  right = std::min(right + padding, rect_width_);
  bottom = std::min(bottom + padding, rect_height_);
  Box* box = boxCreate(*left, *top, right - *left, bottom - *top);
  Pix* grey_pix = pixClipRectangle(original_img, box, nullptr);
  boxDestroy(&box);
  if (level == RIL_BLOCK || level == RIL_PARA) {
    // Clip to the block polygon as well.
    TBOX mask_box;
    Pix* mask = it_->block()->block->render_mask(&mask_box);
    // Copy the mask registered correctly into an image the size of grey_pix.
    int mask_x = *left - mask_box.left();
    int mask_y = *top - (pixGetHeight(original_img) - mask_box.top());
    int width = pixGetWidth(grey_pix);
    int height = pixGetHeight(grey_pix);
    Pix* resized_mask = pixCreate(width, height, 1);
    pixRasterop(resized_mask, std::max(0, -mask_x), std::max(0, -mask_y), width, height,
                PIX_SRC, mask, std::max(0, mask_x), std::max(0, mask_y));
    pixDestroy(&mask);
    pixDilateBrick(resized_mask, resized_mask, 2 * padding + 1,
                   2 * padding + 1);
    pixInvert(resized_mask, resized_mask);
    pixSetMasked(grey_pix, resized_mask, UINT32_MAX);
    pixDestroy(&resized_mask);
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
  if (it_->word() == nullptr) return false;  // Already at the end!
  ROW* row = it_->row()->row;
  WERD* word = it_->word()->word;
  TBOX box = (level == RIL_WORD || level == RIL_SYMBOL)
           ? word->bounding_box()
           : row->bounding_box();
  int left = box.left();
  ICOORD startpt(left, static_cast<int16_t>(row->base_line(left) + 0.5));
  int right = box.right();
  ICOORD endpt(right, static_cast<int16_t>(row->base_line(right) + 0.5));
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
  const bool is_mongolian = false;  // TODO(eger): fix me
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
  *just = para->model->justification();
}

/**
 * Sets up the internal data for iterating the blobs of a new word, then
 * moves the iterator to the given offset.
 */
void PageIterator::BeginWord(int offset) {
  WERD_RES* word_res = it_->word();
  if (word_res == nullptr) {
    // This is a non-text block, so there is no word.
    word_length_ = 0;
    blob_index_ = 0;
    word_ = nullptr;
    return;
  }
  if (word_res->best_choice != nullptr) {
    // Recognition has been done, so we are using the box_word, which
    // is already baseline denormalized.
    word_length_ = word_res->best_choice->length();
    if (word_res->box_word != nullptr) {
      if (word_res->box_word->length() != word_length_) {
        tprintf("Corrupted word! best_choice[len=%d] = %s, box_word[len=%d]: ",
                word_length_, word_res->best_choice->unichar_string().string(),
                word_res->box_word->length());
        word_res->box_word->bounding_box().print();
      }
      ASSERT_HOST(word_res->box_word->length() == word_length_);
    }
    word_ = nullptr;
    // We will be iterating the box_word.
    delete cblob_it_;
    cblob_it_ = nullptr;
  } else {
    // No recognition yet, so a "symbol" is a cblob.
    word_ = word_res->word;
    ASSERT_HOST(word_->cblob_list() != nullptr);
    word_length_ = word_->cblob_list()->length();
    if (cblob_it_ == nullptr) cblob_it_ = new C_BLOB_IT;
    cblob_it_->set_to_list(word_->cblob_list());
  }
  for (blob_index_ = 0; blob_index_ < offset; ++blob_index_) {
    if (cblob_it_ != nullptr)
      cblob_it_->forward();
  }
}

bool PageIterator::SetWordBlamerBundle(BlamerBundle *blamer_bundle) {
  if (it_->word() != nullptr) {
    it_->word()->blamer_bundle = blamer_bundle;
    return true;
  } else {
    return false;
  }
}

}  // namespace tesseract.
