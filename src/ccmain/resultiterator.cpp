///////////////////////////////////////////////////////////////////////
// File:        resultiterator.cpp
// Description: Iterator for tesseract results that is capable of
//              iterating in proper reading order over Bi Directional
//              (e.g. mixed Hebrew and English) text.
// Author:      David Eger
// Created:     Fri May 27 13:58:06 PST 2011
//
// (C) Copyright 2011, Google Inc.
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
#include "strngs.h"
#include "tesseractclass.h"
#include "unicharset.h"
#include "unicodes.h"
#include <set>
#include <vector>

namespace tesseract {

ResultIterator::ResultIterator(const LTRResultIterator &resit)
    : LTRResultIterator(resit) {
  in_minor_direction_ = false;
  at_beginning_of_minor_run_ = false;
  preserve_interword_spaces_ = false;

  BoolParam *p = ParamUtils::FindParam<BoolParam>(
      "preserve_interword_spaces", GlobalParams()->bool_params,
      tesseract_->params()->bool_params);
  if (p != nullptr) preserve_interword_spaces_ = (bool)(*p);

  current_paragraph_is_ltr_ = CurrentParagraphIsLtr();
  MoveToLogicalStartOfTextline();
}

ResultIterator *ResultIterator::StartOfParagraph(
    const LTRResultIterator &resit) {
  return new ResultIterator(resit);
}

bool ResultIterator::ParagraphIsLtr() const {
  return current_paragraph_is_ltr_;
}

bool ResultIterator::CurrentParagraphIsLtr() const {
  if (!it_->word())
    return true;  // doesn't matter.
  LTRResultIterator it(*this);
  it.RestartParagraph();
  // Try to figure out the ltr-ness of the paragraph.  The rules below
  // make more sense in the context of a difficult paragraph example.
  // Here we denote {ltr characters, RTL CHARACTERS}:
  //
  //   "don't go in there!" DAIS EH
  //   EHT OTNI DEPMUJ FELSMIH NEHT DNA
  //                  .GNIDLIUB GNINRUB
  //
  // On the first line, the left-most word is LTR and the rightmost word
  // is RTL.  Thus, we are better off taking the majority direction for
  // the whole paragraph contents.  So instead of "the leftmost word is LTR"
  // indicating an LTR paragraph, we use a heuristic about what RTL paragraphs
  // would not do:  Typically an RTL paragraph would *not* start with an LTR
  // word.  So our heuristics are as follows:
  //
  // (1) If the first text line has an RTL word in the left-most position
  //     it is RTL.
  // (2) If the first text line has an LTR word in the right-most position
  //     it is LTR.
  // (3) If neither of the above is true, take the majority count for the
  //     paragraph -- if there are more rtl words, it is RTL.  If there
  //     are more LTR words, it's LTR.
  bool leftmost_rtl = it.WordDirection() == DIR_RIGHT_TO_LEFT;
  bool rightmost_ltr = it.WordDirection() == DIR_LEFT_TO_RIGHT;
  int num_ltr, num_rtl;
  num_rtl = leftmost_rtl ? 1 : 0;
  num_ltr = (it.WordDirection() == DIR_LEFT_TO_RIGHT) ? 1 : 0;
  for (it.Next(RIL_WORD);
       !it.Empty(RIL_WORD) && !it.IsAtBeginningOf(RIL_TEXTLINE);
       it.Next(RIL_WORD)) {
    StrongScriptDirection dir = it.WordDirection();
    rightmost_ltr = (dir == DIR_LEFT_TO_RIGHT);
    num_rtl += (dir == DIR_RIGHT_TO_LEFT) ? 1 : 0;
    num_ltr += rightmost_ltr ? 1 : 0;
  }
  if (leftmost_rtl)
    return false;
  if (rightmost_ltr)
    return true;
  // First line is ambiguous.  Take statistics on the whole paragraph.
  if (!it.Empty(RIL_WORD) && !it.IsAtBeginningOf(RIL_PARA)) do {
    StrongScriptDirection dir = it.WordDirection();
    num_rtl += (dir == DIR_RIGHT_TO_LEFT) ? 1 : 0;
    num_ltr += (dir == DIR_LEFT_TO_RIGHT) ? 1 : 0;
  } while (it.Next(RIL_WORD) && !it.IsAtBeginningOf(RIL_PARA));
  return num_ltr >= num_rtl;
}

const int ResultIterator::kMinorRunStart = -1;
const int ResultIterator::kMinorRunEnd = -2;
const int ResultIterator::kComplexWord = -3;

void ResultIterator::CalculateBlobOrder(
    GenericVector<int> *blob_indices) const {
  bool context_is_ltr = current_paragraph_is_ltr_ ^ in_minor_direction_;
  blob_indices->clear();
  if (Empty(RIL_WORD)) return;
  if (context_is_ltr || it_->word()->UnicharsInReadingOrder()) {
    // Easy! just return the blobs in order;
    for (int i = 0; i < word_length_; i++)
      blob_indices->push_back(i);
    return;
  }

  // The blobs are in left-to-right order, but the current reading context
  // is right-to-left.
  const int U_LTR = UNICHARSET::U_LEFT_TO_RIGHT;
  const int U_RTL = UNICHARSET::U_RIGHT_TO_LEFT;
  const int U_EURO_NUM = UNICHARSET::U_EUROPEAN_NUMBER;
  const int U_EURO_NUM_SEP = UNICHARSET::U_EUROPEAN_NUMBER_SEPARATOR;
  const int U_EURO_NUM_TERM = UNICHARSET::U_EUROPEAN_NUMBER_TERMINATOR;
  const int U_COMMON_NUM_SEP = UNICHARSET::U_COMMON_NUMBER_SEPARATOR;
  const int U_OTHER_NEUTRAL = UNICHARSET::U_OTHER_NEUTRAL;

  // Step 1: Scan for and mark European Number sequences
  //   [:ET:]*[:EN:]+(([:ES:]|[:CS:])?[:EN:]+)*[:ET:]*
  GenericVector<int> letter_types;
  for (int i = 0; i < word_length_; i++) {
    letter_types.push_back(it_->word()->SymbolDirection(i));
  }
  // Convert a single separtor sandwiched between two EN's into an EN.
  for (int i = 0; i + 2 < word_length_; i++) {
    if (letter_types[i] == U_EURO_NUM && letter_types[i + 2] == U_EURO_NUM &&
        (letter_types[i + 1] == U_EURO_NUM_SEP ||
         letter_types[i + 1] == U_COMMON_NUM_SEP)) {
      letter_types[i + 1] = U_EURO_NUM;
    }
  }
  // Scan for sequences of European Number Terminators around ENs and convert
  // them to ENs.
  for (int i = 0; i < word_length_; i++) {
    if (letter_types[i] == U_EURO_NUM_TERM) {
      int j = i + 1;
      while (j < word_length_ && letter_types[j] == U_EURO_NUM_TERM) { j++; }
      if (j < word_length_ && letter_types[j] == U_EURO_NUM) {
        // The sequence [i..j] should be converted to all European Numbers.
        for (int k = i; k < j; k++) letter_types[k] = U_EURO_NUM;
      }
      j = i - 1;
      while (j > -1 && letter_types[j] == U_EURO_NUM_TERM) { j--; }
      if (j > -1 && letter_types[j] == U_EURO_NUM) {
        // The sequence [j..i] should be converted to all European Numbers.
        for (int k = j; k <= i; k++) letter_types[k] = U_EURO_NUM;
      }
    }
  }
  // Step 2: Convert all remaining types to either L or R.
  // Sequences ([:L:]|[:EN:])+ (([:CS:]|[:ON:])+ ([:L:]|[:EN:])+)* -> L.
  // All other are R.
  for (int i = 0; i < word_length_;) {
    int ti = letter_types[i];
    if (ti == U_LTR || ti == U_EURO_NUM) {
      // Left to right sequence; scan to the end of it.
      int last_good = i;
      for (int j = i + 1; j < word_length_; j++) {
        int tj = letter_types[j];
        if (tj == U_LTR || tj == U_EURO_NUM) {
          last_good = j;
        } else if (tj == U_COMMON_NUM_SEP || tj == U_OTHER_NEUTRAL) {
          // do nothing.
        } else {
          break;
        }
      }
      // [i..last_good] is the L sequence
      for (int k = i; k <= last_good; k++) letter_types[k] = U_LTR;
      i = last_good + 1;
    } else {
      letter_types[i] = U_RTL;
      i++;
    }
  }

  // At this point, letter_types is entirely U_LTR or U_RTL.
  for (int i = word_length_ - 1; i >= 0;) {
    if (letter_types[i] == U_RTL) {
      blob_indices->push_back(i);
      i--;
    } else {
      // left to right sequence.  scan to the beginning.
      int j = i - 1;
      for (; j >= 0 && letter_types[j] != U_RTL; j--) { }  // pass
      // Now (j, i] is LTR
      for (int k = j + 1; k <= i; k++) blob_indices->push_back(k);
      i = j;
    }
  }
  ASSERT_HOST(blob_indices->size() == word_length_);
}

static void PrintScriptDirs(const GenericVector<StrongScriptDirection> &dirs) {
  for (int i = 0; i < dirs.size(); i++) {
    switch (dirs[i]) {
      case DIR_NEUTRAL: tprintf ("N "); break;
      case DIR_LEFT_TO_RIGHT: tprintf("L "); break;
      case DIR_RIGHT_TO_LEFT: tprintf("R "); break;
      case DIR_MIX: tprintf("Z "); break;
      default: tprintf("? "); break;
    }
  }
  tprintf("\n");
}

void ResultIterator::CalculateTextlineOrder(
    bool paragraph_is_ltr,
    const LTRResultIterator &resit,
    GenericVectorEqEq<int> *word_indices) const {
  GenericVector<StrongScriptDirection> directions;
  CalculateTextlineOrder(paragraph_is_ltr, resit, &directions, word_indices);
}

void ResultIterator::CalculateTextlineOrder(
    bool paragraph_is_ltr,
    const LTRResultIterator &resit,
    GenericVector<StrongScriptDirection> *dirs_arg,
    GenericVectorEqEq<int> *word_indices) const {
  GenericVector<StrongScriptDirection> dirs;
  GenericVector<StrongScriptDirection> *directions;
  directions = (dirs_arg != nullptr) ? dirs_arg : &dirs;
  directions->truncate(0);

  // A LTRResultIterator goes strictly left-to-right word order.
  LTRResultIterator ltr_it(resit);
  ltr_it.RestartRow();
  if (ltr_it.Empty(RIL_WORD)) return;
  do {
    directions->push_back(ltr_it.WordDirection());
  } while (ltr_it.Next(RIL_WORD) && !ltr_it.IsAtBeginningOf(RIL_TEXTLINE));

  word_indices->truncate(0);
  CalculateTextlineOrder(paragraph_is_ltr, *directions, word_indices);
}

void ResultIterator::CalculateTextlineOrder(
    bool paragraph_is_ltr,
    const GenericVector<StrongScriptDirection> &word_dirs,
    GenericVectorEqEq<int> *reading_order) {
  reading_order->truncate(0);
  if (word_dirs.size() == 0) return;

  // Take all of the runs of minor direction words and insert them
  // in reverse order.
  int minor_direction, major_direction, major_step, start, end;
  if (paragraph_is_ltr) {
    start = 0;
    end = word_dirs.size();
    major_step = 1;
    major_direction = DIR_LEFT_TO_RIGHT;
    minor_direction = DIR_RIGHT_TO_LEFT;
  } else {
    start = word_dirs.size() - 1;
    end = -1;
    major_step = -1;
    major_direction = DIR_RIGHT_TO_LEFT;
    minor_direction = DIR_LEFT_TO_RIGHT;
    // Special rule: if there are neutral words at the right most side
    //   of a line adjacent to a left-to-right word in the middle of the
    //   line, we interpret the end of the line as a single LTR sequence.
    if (word_dirs[start] == DIR_NEUTRAL) {
      int neutral_end = start;
      while (neutral_end > 0 && word_dirs[neutral_end] == DIR_NEUTRAL) {
        neutral_end--;
      }
      if (neutral_end >= 0 && word_dirs[neutral_end] == DIR_LEFT_TO_RIGHT) {
        // LTR followed by neutrals.
        // Scan for the beginning of the minor left-to-right run.
        int left = neutral_end;
        for (int i = left; i >= 0 && word_dirs[i] != DIR_RIGHT_TO_LEFT; i--) {
          if (word_dirs[i] == DIR_LEFT_TO_RIGHT) left = i;
        }
        reading_order->push_back(kMinorRunStart);
        for (int i = left; i < word_dirs.size(); i++) {
          reading_order->push_back(i);
          if (word_dirs[i] == DIR_MIX) reading_order->push_back(kComplexWord);
        }
        reading_order->push_back(kMinorRunEnd);
        start = left - 1;
      }
    }
  }
  for (int i = start; i != end;) {
    if (word_dirs[i] == minor_direction) {
      int j = i;
      while (j != end && word_dirs[j] != major_direction)
        j += major_step;
      if (j == end) j -= major_step;
      while (j != i && word_dirs[j] != minor_direction)
        j -= major_step;
      //  [j..i] is a minor direction run.
      reading_order->push_back(kMinorRunStart);
      for (int k = j; k != i; k -= major_step) {
        reading_order->push_back(k);
      }
      reading_order->push_back(i);
      reading_order->push_back(kMinorRunEnd);
      i = j + major_step;
    } else {
      reading_order->push_back(i);
      if (word_dirs[i] == DIR_MIX) reading_order->push_back(kComplexWord);
      i += major_step;
    }
  }
}

int ResultIterator::LTRWordIndex() const {
  int this_word_index = 0;
  LTRResultIterator textline(*this);
  textline.RestartRow();
  while (!textline.PositionedAtSameWord(it_)) {
    this_word_index++;
    textline.Next(RIL_WORD);
  }
  return this_word_index;
}

void ResultIterator::MoveToLogicalStartOfWord() {
  if (word_length_ == 0) {
    BeginWord(0);
    return;
  }
  GenericVector<int> blob_order;
  CalculateBlobOrder(&blob_order);
  if (blob_order.size() == 0 || blob_order[0] == 0) return;
  BeginWord(blob_order[0]);
}

bool ResultIterator::IsAtFinalSymbolOfWord() const {
  if (!it_->word()) return true;
  GenericVector<int> blob_order;
  CalculateBlobOrder(&blob_order);
  return blob_order.size() == 0 || blob_order.back() == blob_index_;
}

bool ResultIterator::IsAtFirstSymbolOfWord() const {
  if (!it_->word()) return true;
  GenericVector<int> blob_order;
  CalculateBlobOrder(&blob_order);
  return blob_order.size() == 0 || blob_order[0] == blob_index_;
}

void ResultIterator::AppendSuffixMarks(STRING *text) const {
  if (!it_->word()) return;
  bool reading_direction_is_ltr =
      current_paragraph_is_ltr_ ^ in_minor_direction_;
  // scan forward to see what meta-information the word ordering algorithm
  // left us.
  // If this word is at the  *end* of a minor run, insert the other
  // direction's mark;  else if this was a complex word, insert the
  // current reading order's mark.
  GenericVectorEqEq<int> textline_order;
  CalculateTextlineOrder(current_paragraph_is_ltr_,
                         *this, &textline_order);
  int this_word_index = LTRWordIndex();
  int i = textline_order.get_index(this_word_index);
  if (i < 0) return;

  int last_non_word_mark = 0;
  for (i++; i < textline_order.size() && textline_order[i] < 0; i++) {
    last_non_word_mark = textline_order[i];
  }
  if (last_non_word_mark == kComplexWord) {
    *text += reading_direction_is_ltr ? kLRM : kRLM;
  } else if (last_non_word_mark == kMinorRunEnd) {
    if (current_paragraph_is_ltr_) {
      *text += kLRM;
    } else {
      *text += kRLM;
    }
  }
}

void ResultIterator::MoveToLogicalStartOfTextline() {
  GenericVectorEqEq<int> word_indices;
  RestartRow();
  CalculateTextlineOrder(current_paragraph_is_ltr_,
                         dynamic_cast<const LTRResultIterator&>(*this),
                         &word_indices);
  int i = 0;
  for (; i < word_indices.size() && word_indices[i] < 0; i++) {
    if (word_indices[i] == kMinorRunStart) in_minor_direction_ = true;
    else if (word_indices[i] == kMinorRunEnd) in_minor_direction_ = false;
  }
  if (in_minor_direction_) at_beginning_of_minor_run_ = true;
  if (i >= word_indices.size()) return;
  int first_word_index = word_indices[i];
  for (int j = 0; j < first_word_index; j++) {
    PageIterator::Next(RIL_WORD);
  }
  MoveToLogicalStartOfWord();
}

void ResultIterator::Begin() {
  LTRResultIterator::Begin();
  current_paragraph_is_ltr_ = CurrentParagraphIsLtr();
  in_minor_direction_ = false;
  at_beginning_of_minor_run_ = false;
  MoveToLogicalStartOfTextline();
}

bool ResultIterator::Next(PageIteratorLevel level) {
  if (it_->block() == nullptr) return false; // already at end!
  switch (level) {
    case RIL_BLOCK:  // explicit fall-through
    case RIL_PARA:   // explicit fall-through
    case RIL_TEXTLINE:
      if (!PageIterator::Next(level)) return false;
      if (IsWithinFirstTextlineOfParagraph()) {
        // if we've advanced to a new paragraph,
        // recalculate current_paragraph_is_ltr_
        current_paragraph_is_ltr_ = CurrentParagraphIsLtr();
      }
      in_minor_direction_ = false;
      MoveToLogicalStartOfTextline();
      return it_->block() != nullptr;
    case RIL_SYMBOL:
    {
      GenericVector<int> blob_order;
      CalculateBlobOrder(&blob_order);
      int next_blob = 0;
      while (next_blob < blob_order.size() &&
             blob_index_ != blob_order[next_blob])
        next_blob++;
      next_blob++;
      if (next_blob < blob_order.size()) {
        // we're in the same word; simply advance one blob.
        BeginWord(blob_order[next_blob]);
        at_beginning_of_minor_run_ = false;
        return true;
      }
      level = RIL_WORD;  // we've fallen through to the next word.
    }
    case RIL_WORD:  // explicit fall-through.
    {
      if (it_->word() == nullptr) return Next(RIL_BLOCK);
      GenericVectorEqEq<int> word_indices;
      int this_word_index = LTRWordIndex();
      CalculateTextlineOrder(current_paragraph_is_ltr_,
                             *this,
                             &word_indices);
      int final_real_index = word_indices.size() - 1;
      while (final_real_index > 0 && word_indices[final_real_index] < 0)
        final_real_index--;
      for (int i = 0; i < final_real_index; i++) {
        if (word_indices[i] == this_word_index) {
          int j = i + 1;
          for (; j < final_real_index && word_indices[j] < 0; j++) {
            if (word_indices[j] == kMinorRunStart) in_minor_direction_ = true;
            if (word_indices[j] == kMinorRunEnd) in_minor_direction_ = false;
          }
          at_beginning_of_minor_run_ = (word_indices[j - 1] == kMinorRunStart);
          // awesome, we move to word_indices[j]
          if (BidiDebug(3)) {
            tprintf("Next(RIL_WORD): %d -> %d\n",
                    this_word_index, word_indices[j]);
          }
          PageIterator::RestartRow();
          for (int k = 0; k < word_indices[j]; k++) {
            PageIterator::Next(RIL_WORD);
          }
          MoveToLogicalStartOfWord();
          return true;
        }
      }
      if (BidiDebug(3)) {
        tprintf("Next(RIL_WORD): %d -> EOL\n", this_word_index);
      }
      // we're going off the end of the text line.
      return Next(RIL_TEXTLINE);
    }
  }
  ASSERT_HOST(false);  // shouldn't happen.
  return false;
}

bool ResultIterator::IsAtBeginningOf(PageIteratorLevel level) const {
  if (it_->block() == nullptr) return false;  // Already at the end!
  if (it_->word() == nullptr) return true;  // In an image block.
  if (level == RIL_SYMBOL) return true;  // Always at beginning of a symbol.

  bool at_word_start = IsAtFirstSymbolOfWord();
  if (level == RIL_WORD) return at_word_start;

  ResultIterator line_start(*this);
  // move to the first word in the line...
  line_start.MoveToLogicalStartOfTextline();

  bool at_textline_start = at_word_start && *line_start.it_ == *it_;
  if (level == RIL_TEXTLINE) return at_textline_start;

  // now we move to the left-most word...
  line_start.RestartRow();
  bool at_block_start = at_textline_start &&
      line_start.it_->block() != line_start.it_->prev_block();
  if (level == RIL_BLOCK) return at_block_start;

  bool at_para_start = at_block_start ||
      (at_textline_start &&
       line_start.it_->row()->row->para() !=
           line_start.it_->prev_row()->row->para());
  if (level == RIL_PARA) return at_para_start;

  ASSERT_HOST(false);  // shouldn't happen.
  return false;
}

/**
 * NOTE! This is an exact copy of PageIterator::IsAtFinalElement with the
 *   change that the variable next is now a ResultIterator instead of a
 *   PageIterator.
 */
bool ResultIterator::IsAtFinalElement(PageIteratorLevel level,
                                      PageIteratorLevel element) const {
  if (Empty(element)) return true;  // Already at the end!
  // The result is true if we step forward by element and find we are
  // at the the end of the page or at beginning of *all* levels in:
  // [level, element).
  // When there is more than one level difference between element and level,
  // we could for instance move forward one symbol and still be at the first
  // word on a line, so we also have to be at the first symbol in a word.
  ResultIterator next(*this);
  next.Next(element);
  if (next.Empty(element)) return true;  // Reached the end of the page.
  while (element > level) {
    element = static_cast<PageIteratorLevel>(element - 1);
    if (!next.IsAtBeginningOf(element))
      return false;
  }
  return true;
}

// Returns the number of blanks before the current word.
int ResultIterator::BlanksBeforeWord() const {
  if (CurrentParagraphIsLtr()) return LTRResultIterator::BlanksBeforeWord();
  return IsAtBeginningOf(RIL_TEXTLINE) ? 0 : 1;
}

/**
 * Returns the null terminated UTF-8 encoded text string for the current
 * object at the given level. Use delete [] to free after use.
 */
char* ResultIterator::GetUTF8Text(PageIteratorLevel level) const {
  if (it_->word() == nullptr) return nullptr;  // Already at the end!
  STRING text;
  switch (level) {
    case RIL_BLOCK:
      {
        ResultIterator pp(*this);
        do {
          pp.AppendUTF8ParagraphText(&text);
        } while (pp.Next(RIL_PARA) && pp.it_->block() == it_->block());
      }
      break;
    case RIL_PARA:
      AppendUTF8ParagraphText(&text);
      break;
    case RIL_TEXTLINE:
      {
        ResultIterator it(*this);
        it.MoveToLogicalStartOfTextline();
        it.IterateAndAppendUTF8TextlineText(&text);
      }
      break;
    case RIL_WORD:
      AppendUTF8WordText(&text);
      break;
    case RIL_SYMBOL:
      {
        bool reading_direction_is_ltr =
          current_paragraph_is_ltr_ ^ in_minor_direction_;
        if (at_beginning_of_minor_run_) {
          text += reading_direction_is_ltr ? kLRM : kRLM;
        }
        text = it_->word()->BestUTF8(blob_index_, false);
        if (IsAtFinalSymbolOfWord()) AppendSuffixMarks(&text);
      }
      break;
  }
  int length = text.length() + 1;
  char* result = new char[length];
  strncpy(result, text.string(), length);
  return result;
}

std::vector<std::vector<std::pair<const char*, float>>>* ResultIterator::GetBestLSTMSymbolChoices() const {
  if (it_->word() != nullptr) {
    return &it_->word()->timesteps;
  } else {
    return nullptr;
  }
}

void ResultIterator::AppendUTF8WordText(STRING *text) const {
  if (!it_->word()) return;
  ASSERT_HOST(it_->word()->best_choice != nullptr);
  bool reading_direction_is_ltr =
      current_paragraph_is_ltr_ ^ in_minor_direction_;
  if (at_beginning_of_minor_run_) {
    *text += reading_direction_is_ltr ? kLRM : kRLM;
  }

  GenericVector<int> blob_order;
  CalculateBlobOrder(&blob_order);
  for (int i = 0; i < blob_order.size(); i++) {
    *text += it_->word()->BestUTF8(blob_order[i], false);
  }
  AppendSuffixMarks(text);
}

void ResultIterator::IterateAndAppendUTF8TextlineText(STRING *text) {
  if (Empty(RIL_WORD)) {
    Next(RIL_WORD);
    return;
  }
  if (BidiDebug(1)) {
    GenericVectorEqEq<int> textline_order;
    GenericVector<StrongScriptDirection> dirs;
    CalculateTextlineOrder(current_paragraph_is_ltr_,
                           *this, &dirs, &textline_order);
    tprintf("Strong Script dirs     [%p/P=%s]: ", it_->row(),
            current_paragraph_is_ltr_ ? "ltr" : "rtl");
    PrintScriptDirs(dirs);
    tprintf("Logical textline order [%p/P=%s]: ", it_->row(),
            current_paragraph_is_ltr_ ? "ltr" : "rtl");
    for (int i = 0; i < textline_order.size(); i++) {
      tprintf("%d ", textline_order[i]);
    }
    tprintf("\n");
  }

  int words_appended = 0;
  do {
    int numSpaces = preserve_interword_spaces_ ? it_->word()->word->space()
                                               : (words_appended > 0);
    for (int i = 0; i < numSpaces; ++i) {
      *text += " ";
    }
    AppendUTF8WordText(text);
    words_appended++;
    if (BidiDebug(2)) {
      tprintf("Num spaces=%d, text=%s\n", numSpaces, text->string());
    }
  } while (Next(RIL_WORD) && !IsAtBeginningOf(RIL_TEXTLINE));
  if (BidiDebug(1)) {
    tprintf("%d words printed\n", words_appended);
  }
  *text += line_separator_;
  // If we just finished a paragraph, add an extra newline.
  if (IsAtBeginningOf(RIL_PARA)) {
    *text += paragraph_separator_;
  }
}

void ResultIterator::AppendUTF8ParagraphText(STRING *text) const {
  ResultIterator it(*this);
  it.RestartParagraph();
  it.MoveToLogicalStartOfTextline();
  if (it.Empty(RIL_WORD)) return;
  do {
    it.IterateAndAppendUTF8TextlineText(text);
  } while (it.it_->block() != nullptr && !it.IsAtBeginningOf(RIL_PARA));
}

bool ResultIterator::BidiDebug(int min_level) const {
  int debug_level = 1;
  IntParam *p = ParamUtils::FindParam<IntParam>(
      "bidi_debug", GlobalParams()->int_params,
      tesseract_->params()->int_params);
  if (p != nullptr) debug_level = (int32_t)(*p);
  return debug_level >= min_level;
}

}  // namespace tesseract.
