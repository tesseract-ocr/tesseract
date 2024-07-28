///////////////////////////////////////////////////////////////////////
// File:        ltrresultiterator.cpp
// Description: Iterator for tesseract results in strict left-to-right
//              order that avoids using tesseract internal data structures.
// Author:      Ray Smith
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

#include <tesseract/ltrresultiterator.h>

#include "helpers.h"  // for copy_string
#include "pageres.h"
#include "tesseractclass.h"

#include <allheaders.h>

namespace tesseract {

LTRResultIterator::LTRResultIterator(PAGE_RES *page_res, Tesseract *tesseract, int scale,
                                     int scaled_yres, int rect_left, int rect_top, int rect_width,
                                     int rect_height)
    : PageIterator(page_res, tesseract, scale, scaled_yres, rect_left, rect_top, rect_width,
                   rect_height)
    , line_separator_("\n")
    , paragraph_separator_("\n") {}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
LTRResultIterator::~LTRResultIterator() = default;

// Returns the null terminated UTF-8 encoded text string for the current
// object at the given level. Use delete [] to free after use.
char *LTRResultIterator::GetUTF8Text(PageIteratorLevel level) const {
  if (it_->word() == nullptr) {
    return nullptr; // Already at the end!
  }
  std::string text;
  PAGE_RES_IT res_it(*it_);
  WERD_CHOICE *best_choice = res_it.word()->best_choice;
  ASSERT_HOST(best_choice != nullptr);
  if (level == RIL_SYMBOL) {
    text = res_it.word()->BestUTF8(blob_index_, false);
  } else if (level == RIL_WORD) {
    text = best_choice->unichar_string();
  } else {
    bool eol = false; // end of line?
    bool eop = false; // end of paragraph?
    do {              // for each paragraph in a block
      do {            // for each text line in a paragraph
        do {          // for each word in a text line
          best_choice = res_it.word()->best_choice;
          ASSERT_HOST(best_choice != nullptr);
          text += best_choice->unichar_string();
          text += " ";
          res_it.forward();
          eol = res_it.row() != res_it.prev_row();
        } while (!eol);
        text.resize(text.length() - 1);
        text += line_separator_;
        eop = res_it.block() != res_it.prev_block() ||
              res_it.row()->row->para() != res_it.prev_row()->row->para();
      } while (level != RIL_TEXTLINE && !eop);
      if (eop) {
        text += paragraph_separator_;
      }
    } while (level == RIL_BLOCK && res_it.block() == res_it.prev_block());
  }
  return copy_string(text);
}

// Set the string inserted at the end of each text line. "\n" by default.
void LTRResultIterator::SetLineSeparator(const char *new_line) {
  line_separator_ = new_line;
}

// Set the string inserted at the end of each paragraph. "\n" by default.
void LTRResultIterator::SetParagraphSeparator(const char *new_para) {
  paragraph_separator_ = new_para;
}

// Returns the mean confidence of the current object at the given level.
// The number should be interpreted as a percent probability. (0.0f-100.0f)
float LTRResultIterator::Confidence(PageIteratorLevel level) const {
  if (it_->word() == nullptr) {
    return 0.0f; // Already at the end!
  }
  float mean_certainty = 0.0f;
  int certainty_count = 0;
  PAGE_RES_IT res_it(*it_);
  WERD_CHOICE *best_choice;
  switch (level) {
    case RIL_BLOCK:
      do {
        best_choice = res_it.word()->best_choice;
        mean_certainty += best_choice->certainty();
        ++certainty_count;
        res_it.forward();
      } while (res_it.block() == res_it.prev_block());
      break;
    case RIL_PARA:
      do {
        best_choice = res_it.word()->best_choice;
        mean_certainty += best_choice->certainty();
        ++certainty_count;
        res_it.forward();
      } while (res_it.block() == res_it.prev_block() &&
               res_it.row()->row->para() == res_it.prev_row()->row->para());
      break;
    case RIL_TEXTLINE:
      do {
        best_choice = res_it.word()->best_choice;
        mean_certainty += best_choice->certainty();
        ++certainty_count;
        res_it.forward();
      } while (res_it.row() == res_it.prev_row());
      break;
    case RIL_WORD:
      best_choice = res_it.word()->best_choice;
      mean_certainty = best_choice->certainty();
      certainty_count = 1;
      break;
    case RIL_SYMBOL:
      best_choice = res_it.word()->best_choice;
      mean_certainty = best_choice->certainty(blob_index_);
      certainty_count = 1;
  }
  if (certainty_count > 0) {
    mean_certainty /= certainty_count;
    return ClipToRange(100 + 5 * mean_certainty, 0.0f, 100.0f);
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
const char *LTRResultIterator::WordFontAttributes(bool *is_bold, bool *is_italic,
                                                  bool *is_underlined, bool *is_monospace,
                                                  bool *is_serif, bool *is_smallcaps,
                                                  int *pointsize, int *font_id) const {
  const char *result = nullptr;

  if (it_->word() == nullptr) {
    // Already at the end!
    *pointsize = 0;
  } else {
    float row_height =
        it_->row()->row->x_height() + it_->row()->row->ascenders() - it_->row()->row->descenders();
    // Convert from pixels to printers points.
    *pointsize =
        scaled_yres_ > 0 ? static_cast<int>(row_height * kPointsPerInch / scaled_yres_ + 0.5) : 0;

#ifndef DISABLED_LEGACY_ENGINE
    const FontInfo *font_info = it_->word()->fontinfo;
    if (font_info) {
      // Font information available.
      *font_id = font_info->universal_id;
      *is_bold = font_info->is_bold();
      *is_italic = font_info->is_italic();
      *is_underlined = false; // TODO(rays) fix this!
      *is_monospace = font_info->is_fixed_pitch();
      *is_serif = font_info->is_serif();
      result = font_info->name;
    }
#endif // ndef DISABLED_LEGACY_ENGINE

    *is_smallcaps = it_->word()->small_caps;
  }

  if (!result) {
    *is_bold = false;
    *is_italic = false;
    *is_underlined = false;
    *is_monospace = false;
    *is_serif = false;
    *is_smallcaps = false;
    *font_id = -1;
  }

  return result;
}

// Returns the name of the language used to recognize this word.
const char *LTRResultIterator::WordRecognitionLanguage() const {
  if (it_->word() == nullptr || it_->word()->tesseract == nullptr) {
    return nullptr;
  }
  return it_->word()->tesseract->lang.c_str();
}

// Return the overall directionality of this word.
StrongScriptDirection LTRResultIterator::WordDirection() const {
  if (it_->word() == nullptr) {
    return DIR_NEUTRAL;
  }
  bool has_rtl = it_->word()->AnyRtlCharsInWord();
  bool has_ltr = it_->word()->AnyLtrCharsInWord();
  if (has_rtl && !has_ltr) {
    return DIR_RIGHT_TO_LEFT;
  }
  if (has_ltr && !has_rtl) {
    return DIR_LEFT_TO_RIGHT;
  }
  if (!has_ltr && !has_rtl) {
    return DIR_NEUTRAL;
  }
  return DIR_MIX;
}

// Returns true if the current word was found in a dictionary.
bool LTRResultIterator::WordIsFromDictionary() const {
  if (it_->word() == nullptr) {
    return false; // Already at the end!
  }
  int permuter = it_->word()->best_choice->permuter();
  return permuter == SYSTEM_DAWG_PERM || permuter == FREQ_DAWG_PERM || permuter == USER_DAWG_PERM;
}

// Returns the number of blanks before the current word.
int LTRResultIterator::BlanksBeforeWord() const {
  if (it_->word() == nullptr) {
    return 1;
  }
  return it_->word()->word->space();
}

// Returns true if the current word is numeric.
bool LTRResultIterator::WordIsNumeric() const {
  if (it_->word() == nullptr) {
    return false; // Already at the end!
  }
  int permuter = it_->word()->best_choice->permuter();
  return permuter == NUMBER_PERM;
}

// Returns true if the word contains blamer information.
bool LTRResultIterator::HasBlamerInfo() const {
  return it_->word() != nullptr && it_->word()->blamer_bundle != nullptr &&
         it_->word()->blamer_bundle->HasDebugInfo();
}

#ifndef DISABLED_LEGACY_ENGINE
// Returns the pointer to ParamsTrainingBundle stored in the BlamerBundle
// of the current word.
const void *LTRResultIterator::GetParamsTrainingBundle() const {
  return (it_->word() != nullptr && it_->word()->blamer_bundle != nullptr)
             ? &(it_->word()->blamer_bundle->params_training_bundle())
             : nullptr;
}
#endif // ndef DISABLED_LEGACY_ENGINE

// Returns the pointer to the string with blamer information for this word.
// Assumes that the word's blamer_bundle is not nullptr.
const char *LTRResultIterator::GetBlamerDebug() const {
  return it_->word()->blamer_bundle->debug().c_str();
}

// Returns the pointer to the string with misadaption information for this word.
// Assumes that the word's blamer_bundle is not nullptr.
const char *LTRResultIterator::GetBlamerMisadaptionDebug() const {
  return it_->word()->blamer_bundle->misadaption_debug().c_str();
}

// Returns true if a truth string was recorded for the current word.
bool LTRResultIterator::HasTruthString() const {
  if (it_->word() == nullptr) {
    return false; // Already at the end!
  }
  if (it_->word()->blamer_bundle == nullptr || it_->word()->blamer_bundle->NoTruth()) {
    return false; // no truth information for this word
  }
  return true;
}

// Returns true if the given string is equivalent to the truth string for
// the current word.
bool LTRResultIterator::EquivalentToTruth(const char *str) const {
  if (!HasTruthString()) {
    return false;
  }
  ASSERT_HOST(it_->word()->uch_set != nullptr);
  WERD_CHOICE str_wd(str, *(it_->word()->uch_set));
  return it_->word()->blamer_bundle->ChoiceIsCorrect(&str_wd);
}

// Returns the null terminated UTF-8 encoded truth string for the current word.
// Use delete [] to free after use.
char *LTRResultIterator::WordTruthUTF8Text() const {
  if (!HasTruthString()) {
    return nullptr;
  }
  return copy_string(it_->word()->blamer_bundle->TruthString());
}

// Returns the null terminated UTF-8 encoded normalized OCR string for the
// current word. Use delete [] to free after use.
char *LTRResultIterator::WordNormedUTF8Text() const {
  if (it_->word() == nullptr) {
    return nullptr; // Already at the end!
  }
  std::string ocr_text;
  WERD_CHOICE *best_choice = it_->word()->best_choice;
  const UNICHARSET *unicharset = it_->word()->uch_set;
  for (unsigned i = 0; i < best_choice->length(); ++i) {
    ocr_text += unicharset->get_normed_unichar(best_choice->unichar_id(i));
  }
  return copy_string(ocr_text);
}

// Returns a pointer to serialized choice lattice.
// Fills lattice_size with the number of bytes in lattice data.
const char *LTRResultIterator::WordLattice(int *lattice_size) const {
  if (it_->word() == nullptr) {
    return nullptr; // Already at the end!
  }
  if (it_->word()->blamer_bundle == nullptr) {
    return nullptr;
  }
  *lattice_size = it_->word()->blamer_bundle->lattice_size();
  return it_->word()->blamer_bundle->lattice_data();
}

// Returns true if the current symbol is a superscript.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool LTRResultIterator::SymbolIsSuperscript() const {
  if (cblob_it_ == nullptr && it_->word() != nullptr) {
    return it_->word()->best_choice->BlobPosition(blob_index_) == SP_SUPERSCRIPT;
  }
  return false;
}

// Returns true if the current symbol is a subscript.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool LTRResultIterator::SymbolIsSubscript() const {
  if (cblob_it_ == nullptr && it_->word() != nullptr) {
    return it_->word()->best_choice->BlobPosition(blob_index_) == SP_SUBSCRIPT;
  }
  return false;
}

// Returns true if the current symbol is a dropcap.
// If iterating at a higher level object than symbols, eg words, then
// this will return the attributes of the first symbol in that word.
bool LTRResultIterator::SymbolIsDropcap() const {
  if (cblob_it_ == nullptr && it_->word() != nullptr) {
    return it_->word()->best_choice->BlobPosition(blob_index_) == SP_DROPCAP;
  }
  return false;
}

ChoiceIterator::ChoiceIterator(const LTRResultIterator &result_it) {
  ASSERT_HOST(result_it.it_->word() != nullptr);
  word_res_ = result_it.it_->word();
  oemLSTM_ = word_res_->tesseract->AnyLSTMLang();
  // Is there legacy engine related trained data?
  bool oemLegacy = word_res_->tesseract->AnyTessLang();
  // Is lstm_choice_mode activated?
  bool lstm_choice_mode = word_res_->tesseract->lstm_choice_mode;
  rating_coefficient_ = word_res_->tesseract->lstm_rating_coefficient;
  blanks_before_word_ = result_it.BlanksBeforeWord();
  BLOB_CHOICE_LIST *choices = nullptr;
  tstep_index_ = &result_it.blob_index_;
  if (oemLSTM_ && !word_res_->CTC_symbol_choices.empty()) {
    if (!word_res_->CTC_symbol_choices[0].empty() &&
        strcmp(word_res_->CTC_symbol_choices[0][0].first, " ")) {
      blanks_before_word_ = 0;
    }
    unsigned index = *tstep_index_;
    index += blanks_before_word_;
    if (index < word_res_->CTC_symbol_choices.size()) {
      LSTM_choices_ = &word_res_->CTC_symbol_choices[index];
      filterSpaces();
    }
  }
  if ((oemLegacy || !lstm_choice_mode) && word_res_->ratings != nullptr) {
    choices = word_res_->GetBlobChoices(result_it.blob_index_);
  }
  if (choices != nullptr && !choices->empty()) {
    choice_it_ = new BLOB_CHOICE_IT(choices);
    choice_it_->mark_cycle_pt();
  } else {
    choice_it_ = nullptr;
  }
  if (LSTM_choices_ != nullptr && !LSTM_choices_->empty()) {
    LSTM_choice_it_ = LSTM_choices_->begin();
  }
}
ChoiceIterator::~ChoiceIterator() {
  delete choice_it_;
}

// Moves to the next choice for the symbol and returns false if there
// are none left.
bool ChoiceIterator::Next() {
  if (oemLSTM_ && LSTM_choices_ != nullptr && !LSTM_choices_->empty()) {
    if (LSTM_choice_it_ == LSTM_choices_->end() ||
        next(LSTM_choice_it_) == LSTM_choices_->end()) {
      return false;
    } else {
      ++LSTM_choice_it_;
      return true;
    }
  } else {
    if (choice_it_ == nullptr) {
      return false;
    }
    choice_it_->forward();
    return !choice_it_->cycled_list();
  }
}

// Returns the null terminated UTF-8 encoded text string for the current
// choice. Do NOT use delete [] to free after use.
const char *ChoiceIterator::GetUTF8Text() const {
  if (oemLSTM_ && LSTM_choices_ != nullptr && !LSTM_choices_->empty()) {
    std::pair<const char *, float> choice = *LSTM_choice_it_;
    return choice.first;
  } else {
    if (choice_it_ == nullptr) {
      return nullptr;
    }
    UNICHAR_ID id = choice_it_->data()->unichar_id();
    return word_res_->uch_set->id_to_unichar_ext(id);
  }
}

// Returns the confidence of the current choice depending on the used language
// data. If only LSTM traineddata is used the value range is 0.0f - 1.0f. All
// choices for one symbol should roughly add up to 1.0f.
// If only traineddata of the legacy engine is used, the number should be
// interpreted as a percent probability. (0.0f-100.0f) In this case
// probabilities won't add up to 100. Each one stands on its own.
float ChoiceIterator::Confidence() const {
  float confidence;
  if (oemLSTM_ && LSTM_choices_ != nullptr && !LSTM_choices_->empty()) {
    std::pair<const char *, float> choice = *LSTM_choice_it_;
    confidence = 100 - rating_coefficient_ * choice.second;
  } else {
    if (choice_it_ == nullptr) {
      return 0.0f;
    }
    confidence = 100 + 5 * choice_it_->data()->certainty();
  }
  return ClipToRange(confidence, 0.0f, 100.0f);
}

// Returns the set of timesteps which belong to the current symbol
std::vector<std::vector<std::pair<const char *, float>>> *ChoiceIterator::Timesteps() const {
  unsigned offset = *tstep_index_ + blanks_before_word_;
  if (offset >= word_res_->segmented_timesteps.size() || !oemLSTM_) {
    return nullptr;
  }
  return &word_res_->segmented_timesteps[offset];
}

void ChoiceIterator::filterSpaces() {
  if (LSTM_choices_->empty()) {
    return;
  }
  std::vector<std::pair<const char *, float>>::iterator it;
  for (it = LSTM_choices_->begin(); it != LSTM_choices_->end();) {
    if (!strcmp(it->first, " ")) {
      it = LSTM_choices_->erase(it);
    } else {
      ++it;
    }
  }
}
} // namespace tesseract.
