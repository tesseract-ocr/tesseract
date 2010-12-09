///////////////////////////////////////////////////////////////////////
// File:        resultiterator.cpp
// Description: Iterator for tesseract results that avoids using tesseract
//              internal data structures
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

#include "resultiterator.h"
#include "allheaders.h"
#include "pageres.h"
#include "tesseractclass.h"

namespace tesseract {

ResultIterator::ResultIterator(PAGE_RES* page_res, Tesseract* tesseract,
                               int scale, int scaled_yres,
                               int rect_left, int rect_top,
                               int rect_width, int rect_height)
  : PageIterator(page_res, tesseract, scale, scaled_yres,
    rect_left, rect_top, rect_width, rect_height) {
}

ResultIterator::~ResultIterator() {
}

// Returns the null terminated UTF-8 encoded text string for the current
// object at the given level. Use delete [] to free after use.
char* ResultIterator::GetUTF8Text(PageIteratorLevel level) const {
  if (it_->word() == NULL) return NULL;  // Already at the end!
  STRING text;
  PAGE_RES_IT res_it(*it_);
  WERD_CHOICE* best_choice = res_it.word()->best_choice;
  ASSERT_HOST(best_choice != NULL);
  switch (level) {
    case RIL_BLOCK:
    case RIL_PARA:
      do {
        best_choice = res_it.word()->best_choice;
        ASSERT_HOST(best_choice != NULL);
        text += best_choice->unichar_string();
        text += res_it.word()->word->flag(W_EOL) ? "\n" : " ";
        res_it.forward();
      } while (res_it.block() == res_it.prev_block());
      break;
    case RIL_TEXTLINE:
      do {
        best_choice = res_it.word()->best_choice;
        ASSERT_HOST(best_choice != NULL);
        text += best_choice->unichar_string();
        text += res_it.word()->word->flag(W_EOL) ? "\n" : " ";
         res_it.forward();
      } while (res_it.row() == res_it.prev_row());
      break;
    case RIL_WORD:
      text = best_choice->unichar_string();
      break;
    case RIL_SYMBOL:
      text = tesseract_->unicharset.id_to_unichar(
          best_choice->unichar_id(blob_index_));
  }
  int length = text.length() + 1;
  char* result = new char[length];
  strncpy(result, text.string(), length);
  return result;
}

// Returns the mean confidence of the current object at the given level.
// The number should be interpreted as a percent probability. (0.0f-100.0f)
float ResultIterator::Confidence(PageIteratorLevel level) const {
  if (it_->word() == NULL) return 0.0f;  // Already at the end!
  float mean_certainty = 0.0f;
  int certainty_count = 0;
  PAGE_RES_IT res_it(*it_);
  WERD_CHOICE* best_choice = res_it.word()->best_choice;
  ASSERT_HOST(best_choice != NULL);
  switch (level) {
    case RIL_BLOCK:
    case RIL_PARA:
      do {
        best_choice = res_it.word()->best_choice;
        ASSERT_HOST(best_choice != NULL);
        mean_certainty += best_choice->certainty();
        ++certainty_count;
        res_it.forward();
      } while (res_it.block() == res_it.prev_block());
      break;
    case RIL_TEXTLINE:
      do {
        best_choice = res_it.word()->best_choice;
        ASSERT_HOST(best_choice != NULL);
        mean_certainty += best_choice->certainty();
        ++certainty_count;
        res_it.forward();
      } while (res_it.row() == res_it.prev_row());
      break;
    case RIL_WORD:
      mean_certainty += best_choice->certainty();
     ++certainty_count;
      break;
    case RIL_SYMBOL:
      BLOB_CHOICE_LIST_CLIST* choices = best_choice->blob_choices();
      if (choices != NULL) {
        BLOB_CHOICE_LIST_C_IT blob_choices_it(choices);
        for (int blob = 0; blob < blob_index_; ++blob)
          blob_choices_it.forward();
        BLOB_CHOICE_IT choice_it(blob_choices_it.data());
        for (choice_it.mark_cycle_pt();
             !choice_it.cycled_list();
             choice_it.forward()) {
          if (choice_it.data()->unichar_id() ==
              best_choice->unichar_id(blob_index_))
            break;
        }
        mean_certainty += choice_it.data()->certainty();
      } else {
        mean_certainty += best_choice->certainty();
      }
      ++certainty_count;
  }
  if (certainty_count > 0) {
    mean_certainty /= certainty_count;
    float confidence = 100 + 5 * mean_certainty;
    if (confidence < 0.0f) confidence = 0.0f;
    if (confidence > 100.0f) confidence = 100.0f;
    return confidence;
  }
  return 0.0f;
}

// Returns the font attributes of the current word. If iterating at a higher
// level object than words, eg textlines, then this will return the
// attributes of the first word in that textline.
// The actual return value is a string representing a font name. It points
// to an internal table and SHOULD NOT BE DELETED. Lifespan is the same as
// the iterator itself, ie rendered invalid by various members of
// TessBaseAPI, including Init, SetImage, End or deleting the TessBaseAPI.
// Pointsize is returned in printers points (1/72 inch.)
const char* ResultIterator::WordFontAttributes(bool* is_bold,
                                               bool* is_italic,
                                               bool* is_underlined,
                                               bool* is_monospace,
                                               bool* is_serif,
                                               bool* is_smallcaps,
                                               int* pointsize,
                                               int* font_id) const {
  if (it_->word() == NULL) return NULL;  // Already at the end!
  *font_id = it_->word()->font1;
  if (*font_id < 0) return NULL;  // No font available.
  const UnicityTable<FontInfo> &font_table = tesseract_->get_fontinfo_table();
  FontInfo font_info = font_table.get(*font_id);
  *is_bold = font_info.is_bold();
  *is_italic = font_info.is_italic();
  *is_underlined = false;  // TODO(rays) fix this!
  *is_monospace = font_info.is_fixed_pitch();
  *is_serif = font_info.is_serif();
  *is_smallcaps = it_->word()->small_caps;
  // The font size is calculated from a multiple of the x-height
  // that came from the block.
  float row_height = it_->row()->row->x_height() *
      it_->block()->block->cell_over_xheight();
  // Convert from pixels to printers points.
  *pointsize = scaled_yres_ > 0
    ? static_cast<int>(row_height * kPointsPerInch / scaled_yres_ + 0.5)
    : 0;

  return font_info.name;
}

// Returns true if the current word was found in a dictionary.
bool ResultIterator::WordIsFromDictionary() const {
  if (it_->word() == NULL) return false;  // Already at the end!
  int permuter = it_->word()->best_choice->permuter();
  return permuter == SYSTEM_DAWG_PERM || permuter == FREQ_DAWG_PERM ||
         permuter == USER_DAWG_PERM;
}

// Returns true if the current word is numeric.
bool ResultIterator::WordIsNumeric() const {
  if (it_->word() == NULL) return false;  // Already at the end!
  int permuter = it_->word()->best_choice->permuter();
  return permuter == NUMBER_PERM;
}

// Returns true if the current symbol is a superscript.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool ResultIterator::SymbolIsSuperscript() const {
  if (cblob_it_ == NULL && it_->word() != NULL)
    return it_->word()->box_word->BlobPosition(blob_index_) == SP_SUPERSCRIPT;
  return false;
}

// Returns true if the current symbol is a subscript.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool ResultIterator::SymbolIsSubscript() const {
  if (cblob_it_ == NULL && it_->word() != NULL)
    return it_->word()->box_word->BlobPosition(blob_index_) == SP_SUBSCRIPT;
  return false;
}

// Returns true if the current symbol is a dropcap.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool ResultIterator::SymbolIsDropcap() const {
  if (cblob_it_ == NULL && it_->word() != NULL)
    return it_->word()->box_word->BlobPosition(blob_index_) == SP_DROPCAP;
  return false;
}

ChoiceIterator::ChoiceIterator(const ResultIterator& result_it) {
  ASSERT_HOST(result_it.it_->word() != NULL);
  tesseract_ = result_it.tesseract_;
  PAGE_RES_IT res_it(*result_it.it_);
  WERD_CHOICE* best_choice = res_it.word()->best_choice;
  BLOB_CHOICE_LIST_CLIST* choices = best_choice->blob_choices();
  if (choices != NULL) {
    BLOB_CHOICE_LIST_C_IT blob_choices_it(choices);
    for (int blob = 0; blob < result_it.blob_index_; ++blob)
      blob_choices_it.forward();
    choice_it_ = new BLOB_CHOICE_IT(blob_choices_it.data());
    choice_it_->mark_cycle_pt();
  } else {
    choice_it_ = NULL;
  }
}

ChoiceIterator::~ChoiceIterator() {
  delete choice_it_;
}

// Moves to the next choice for the symbol and returns false if there
// are none left.
bool ChoiceIterator::Next() {
  if (choice_it_ == NULL)
    return false;
  choice_it_->forward();
  return !choice_it_->cycled_list();
}

// Returns the null terminated UTF-8 encoded text string for the current
// choice. Use delete [] to free after use.
const char* ChoiceIterator::GetUTF8Text() const {
  if (choice_it_ == NULL)
    return NULL;
  UNICHAR_ID id = choice_it_->data()->unichar_id();
  if (id < 0 || id >= tesseract_->unicharset.size() ||
      id == INVALID_UNICHAR_ID)
    return NULL;
  return tesseract_->unicharset.id_to_unichar(id);
}

// Returns the confidence of the current choice.
// The number should be interpreted as a percent probability. (0.0f-100.0f)
float ChoiceIterator::Confidence() const {
  if (choice_it_ == NULL)
    return 0.0f;
  float confidence = 100 + 5 * choice_it_->data()->certainty();
  if (confidence < 0.0f) confidence = 0.0f;
  if (confidence > 100.0f) confidence = 100.0f;
  return confidence;
}


}  // namespace tesseract.
