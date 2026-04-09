/**********************************************************************
 * File:        paragraphs.cpp
 * Description: Paragraph detection for tesseract.
 * Author:      David Eger
 *
 * (C) Copyright 2011, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "paragraphs.h"

#include "helpers.h"             // for UpdateRange, ClipToRange
#include "host.h"                // for NearlyEqual
#include "mutableiterator.h"     // for MutableIterator
#include "ocrblock.h"            // for BLOCK
#include "ocrpara.h"             // for ParagraphModel, PARA, PARA_IT, PARA...
#include "ocrrow.h"              // for ROW
#include "pageres.h"             // for PAGE_RES_IT, WERD_RES, ROW_RES, BLO...
#include "paragraphs_internal.h" // for RowScratchRegisters, SetOfModels
#include "pdblock.h"             // for PDBLK
#include "polyblk.h"             // for POLY_BLOCK
#include "ratngs.h"              // for WERD_CHOICE
#include "rect.h"                // for TBOX
#include "statistc.h"            // for STATS
#include "tesserrstream.h"       // for tesserr
#include "tprintf.h"             // for tprintf
#include "unicharset.h"          // for UNICHARSET
#include "werd.h"                // for WERD, W_REP_CHAR

#include <tesseract/pageiterator.h> // for PageIterator
#include <tesseract/publictypes.h>  // for JUSTIFICATION_LEFT, JUSTIFICATION_R...
#include <tesseract/unichar.h>      // for UNICHAR, UNICHAR_ID

#include <algorithm> // for max
#include <cctype>    // for isspace
#include <cmath>     // for abs
#include <cstdio>    // for snprintf
#include <cstdlib>   // for abs
#include <cstring>   // for strchr, strlen
#include <memory>    // for unique_ptr

static const char *const kRLE = "\u202A"; // Right-to-Left Embedding
static const char *const kPDF = "\u202C"; // Pop Directional Formatting

namespace tesseract {

// Special "weak" ParagraphModels.
const ParagraphModel *kCrownLeft =
    reinterpret_cast<ParagraphModel *>(static_cast<uintptr_t>(0xDEAD111F));
const ParagraphModel *kCrownRight =
    reinterpret_cast<ParagraphModel *>(static_cast<uintptr_t>(0xDEAD888F));

// Do the text and geometry of two rows support a paragraph break between them?
static bool LikelyParagraphStart(const RowScratchRegisters &before,
                                 const RowScratchRegisters &after,
                                 tesseract::ParagraphJustification j);

// Given the width of a typical space between words, what is the threshold
// by which by which we think left and right alignments for paragraphs
// can vary and still be aligned.
static int Epsilon(int space_pix) {
  return space_pix * 4 / 5;
}

static bool AcceptableRowArgs(int debug_level, int min_num_rows, const char *function_name,
                              const std::vector<RowScratchRegisters> *rows, int row_start,
                              int row_end) {
  if (row_start < 0 || static_cast<size_t>(row_end) > rows->size() || row_start > row_end) {
    tesserr << "Invalid arguments rows[" << row_start << ", " << row_end
            << ") while rows is of size " << rows->size() << ".\n";
    return false;
  }
  if (row_end - row_start < min_num_rows) {
    if (debug_level > 1) {
      tprintf("# Too few rows[%d, %d) for %s.\n", row_start, row_end, function_name);
    }
    return false;
  }
  return true;
}

// =============================== Debug Code ================================

// Given a row-major matrix of unicode text and a column separator, print
// a formatted table.  For ASCII, we get good column alignment.
static void PrintTable(const std::vector<std::vector<std::string>> &rows, const char *colsep) {
  std::vector<int> max_col_widths;
  for (const auto &row : rows) {
    auto num_columns = row.size();
    for (size_t c = 0; c < num_columns; c++) {
      int num_unicodes = 0;
      for (char i : row[c]) {
        if ((i & 0xC0) != 0x80) {
          num_unicodes++;
        }
      }
      if (c >= max_col_widths.size()) {
        max_col_widths.push_back(num_unicodes);
      } else {
        if (num_unicodes > max_col_widths[c]) {
          max_col_widths[c] = num_unicodes;
        }
      }
    }
  }

  std::vector<std::string> col_width_patterns;
  col_width_patterns.reserve(max_col_widths.size());
  for (int max_col_width : max_col_widths) {
    col_width_patterns.push_back(std::string("%-") + std::to_string(max_col_width) + "s");
  }

  for (const auto &row : rows) {
    for (unsigned c = 0; c < row.size(); c++) {
      if (c > 0) {
        tprintf("%s", colsep);
      }
      tprintf(col_width_patterns[c].c_str(), row[c].c_str());
    }
    tprintf("\n");
  }
}

static std::string RtlEmbed(const std::string &word, bool rtlify) {
  if (rtlify) {
    return std::string(kRLE) + word + std::string(kPDF);
  }
  return word;
}

// Print the current thoughts of the paragraph detector.
static void PrintDetectorState(const ParagraphTheory &theory,
                               const std::vector<RowScratchRegisters> &rows) {
  std::vector<std::vector<std::string>> output;
  output.emplace_back();
  output.back().push_back("#row");
  output.back().push_back("space");
  output.back().push_back("..");
  output.back().push_back("lword[widthSEL]");
  output.back().push_back("rword[widthSEL]");
  RowScratchRegisters::AppendDebugHeaderFields(output.back());
  output.back().push_back("text");

  for (unsigned i = 0; i < rows.size(); i++) {
    output.emplace_back();
    std::vector<std::string> &row = output.back();
    const RowInfo &ri = *rows[i].ri_;
    row.push_back(std::to_string(i));
    row.push_back(std::to_string(ri.average_interword_space));
    row.emplace_back(ri.has_leaders ? ".." : " ");
    row.push_back(RtlEmbed(ri.lword_text, !ri.ltr) + "[" + std::to_string(ri.lword_box.width()) +
                  (ri.lword_likely_starts_idea ? "S" : "s") +
                  (ri.lword_likely_ends_idea ? "E" : "e") +
                  (ri.lword_indicates_list_item ? "L" : "l") + "]");
    row.push_back(RtlEmbed(ri.rword_text, !ri.ltr) + "[" + std::to_string(ri.rword_box.width()) +
                  (ri.rword_likely_starts_idea ? "S" : "s") +
                  (ri.rword_likely_ends_idea ? "E" : "e") +
                  (ri.rword_indicates_list_item ? "L" : "l") + "]");
    rows[i].AppendDebugInfo(theory, row);
    row.push_back(RtlEmbed(ri.text, !ri.ltr));
  }
  PrintTable(output, " ");

  tprintf("Active Paragraph Models:\n");
  unsigned m = 0;
  for (const auto &model : theory.models()) {
    tprintf(" %d: %s\n", ++m, model->ToString().c_str());
  }
}

static void DebugDump(bool should_print, const char *phase, const ParagraphTheory &theory,
                      const std::vector<RowScratchRegisters> &rows) {
  if (!should_print) {
    return;
  }
  tprintf("# %s\n", phase);
  PrintDetectorState(theory, rows);
}

// Print out the text for rows[row_start, row_end)
static void PrintRowRange(const std::vector<RowScratchRegisters> &rows, int row_start,
                          int row_end) {
  tprintf("======================================\n");
  for (int row = row_start; row < row_end; row++) {
    tprintf("%s\n", rows[row].ri_->text.c_str());
  }
  tprintf("======================================\n");
}

// ============= Brain Dead Language Model (ASCII Version) ===================

static bool IsLatinLetter(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static bool IsDigitLike(int ch) {
  return ch == 'o' || ch == 'O' || ch == 'l' || ch == 'I';
}

static bool IsOpeningPunct(int ch) {
  return strchr("'\"({[", ch) != nullptr;
}

static bool IsTerminalPunct(int ch) {
  return strchr(":'\".?!]})", ch) != nullptr;
}

// Return a pointer after consuming as much text as qualifies as roman numeral.
static const char *SkipChars(const char *str, const char *toskip) {
  while (*str != '\0' && strchr(toskip, *str)) {
    str++;
  }
  return str;
}

static const char *SkipChars(const char *str, bool (*skip)(int)) {
  while (*str != '\0' && skip(*str)) {
    str++;
  }
  return str;
}

static const char *SkipOne(const char *str, const char *toskip) {
  if (*str != '\0' && strchr(toskip, *str)) {
    return str + 1;
  }
  return str;
}

// Return whether it is very likely that this is a numeral marker that could
// start a list item.  Some examples include:
//   A   I   iii.   VI   (2)   3.5.   [C-4]
static bool LikelyListNumeral(const std::string &word) {
  const char *kRomans = "ivxlmdIVXLMD";
  const char *kDigits = "012345789";
  const char *kOpen = "[{(";
  const char *kSep = ":;-.,";
  const char *kClose = "]})";

  int num_segments = 0;
  const char *pos = word.c_str();
  while (*pos != '\0' && num_segments < 3) {
    // skip up to two open parens.
    const char *numeral_start = SkipOne(SkipOne(pos, kOpen), kOpen);
    const char *numeral_end = SkipChars(numeral_start, kRomans);
    if (numeral_end != numeral_start) {
      // Got Roman Numeral. Great.
    } else {
      numeral_end = SkipChars(numeral_start, kDigits);
      if (numeral_end == numeral_start) {
        // If there's a single latin letter, we can use that.
        numeral_end = SkipChars(numeral_start, IsLatinLetter);
        if (numeral_end - numeral_start != 1) {
          break;
        }
      }
    }
    // We got some sort of numeral.
    num_segments++;
    // Skip any trailing parens or punctuation.
    pos = SkipChars(SkipChars(numeral_end, kClose), kSep);
    if (pos == numeral_end) {
      break;
    }
  }
  return *pos == '\0';
}

static bool LikelyListMark(const std::string &word) {
  const char *kListMarks = "0Oo*.,+.";
  return word.size() == 1 && strchr(kListMarks, word[0]) != nullptr;
}

bool AsciiLikelyListItem(const std::string &word) {
  return LikelyListMark(word) || LikelyListNumeral(word);
}

// ========== Brain Dead Language Model (Tesseract Version) ================

// Return the first Unicode Codepoint from werd[pos].
static int UnicodeFor(const UNICHARSET *u, const WERD_CHOICE *werd, unsigned pos) {
  if (!u || !werd || pos > werd->length()) {
    return 0;
  }
  return UNICHAR(u->id_to_unichar(werd->unichar_id(pos)), -1).first_uni();
}

// A useful helper class for finding the first j >= i so that word[j]
// does not have given character type.
class UnicodeSpanSkipper {
public:
  UnicodeSpanSkipper(const UNICHARSET *unicharset, const WERD_CHOICE *word)
      : u_(unicharset), word_(word), wordlen_(word->length()) {
  }

  // Given an input position, return the first position >= pos not punc.
  unsigned SkipPunc(unsigned pos);
  // Given an input position, return the first position >= pos not digit.
  unsigned SkipDigits(unsigned pos);
  // Given an input position, return the first position >= pos not roman.
  unsigned SkipRomans(unsigned pos);
  // Given an input position, return the first position >= pos not alpha.
  unsigned SkipAlpha(unsigned pos);

private:
  const UNICHARSET *u_;
  const WERD_CHOICE *word_;
  unsigned wordlen_;
};

unsigned UnicodeSpanSkipper::SkipPunc(unsigned pos) {
  while (pos < wordlen_ && u_->get_ispunctuation(word_->unichar_id(pos))) {
    pos++;
  }
  return pos;
}

unsigned UnicodeSpanSkipper::SkipDigits(unsigned pos) {
  while (pos < wordlen_ &&
         (u_->get_isdigit(word_->unichar_id(pos)) || IsDigitLike(UnicodeFor(u_, word_, pos)))) {
    pos++;
  }
  return pos;
}

unsigned UnicodeSpanSkipper::SkipRomans(unsigned pos) {
  const char *kRomans = "ivxlmdIVXLMD";
  while (pos < wordlen_) {
    int ch = UnicodeFor(u_, word_, pos);
    if (ch >= 0xF0 || strchr(kRomans, ch) == nullptr) {
      break;
    }
    pos++;
  }
  return pos;
}

unsigned UnicodeSpanSkipper::SkipAlpha(unsigned pos) {
  while (pos < wordlen_ && u_->get_isalpha(word_->unichar_id(pos))) {
    pos++;
  }
  return pos;
}

static bool LikelyListMarkUnicode(int ch) {
  if (ch < 0x80) {
    std::string single_ch;
    single_ch += ch;
    return LikelyListMark(single_ch);
  }
  switch (ch) {
    // TODO(eger) expand this list of unicodes as needed.
    case 0x00B0: // degree sign
    case 0x2022: // bullet
    case 0x25E6: // white bullet
    case 0x00B7: // middle dot
    case 0x25A1: // white square
    case 0x25A0: // black square
    case 0x25AA: // black small square
    case 0x2B1D: // black very small square
    case 0x25BA: // black right-pointing pointer
    case 0x25CF: // black circle
    case 0x25CB: // white circle
      return true;
    default:
      break; // fall through
  }
  return false;
}

// Return whether it is very likely that this is a numeral marker that could
// start a list item.  Some examples include:
//   A   I   iii.   VI   (2)   3.5.   [C-4]
static bool UniLikelyListItem(const UNICHARSET *u, const WERD_CHOICE *werd) {
  if (werd->length() == 1 && LikelyListMarkUnicode(UnicodeFor(u, werd, 0))) {
    return true;
  }

  UnicodeSpanSkipper m(u, werd);
  int num_segments = 0;
  unsigned pos = 0;
  while (pos < werd->length() && num_segments < 3) {
    auto numeral_start = m.SkipPunc(pos);
    if (numeral_start > pos + 1) {
      break;
    }
    auto numeral_end = m.SkipRomans(numeral_start);
    if (numeral_end == numeral_start) {
      numeral_end = m.SkipDigits(numeral_start);
      if (numeral_end == numeral_start) {
        // If there's a single latin letter, we can use that.
        numeral_end = m.SkipAlpha(numeral_start);
        if (numeral_end - numeral_start != 1) {
          break;
        }
      }
    }
    // We got some sort of numeral.
    num_segments++;
    // Skip any trailing punctuation.
    pos = m.SkipPunc(numeral_end);
    if (pos == numeral_end) {
      break;
    }
  }
  return pos == werd->length();
}

template<class T>
void push_back_new(std::vector<T> &vector, const T &data) {
  if (std::find(vector.begin(), vector.end(), data) == vector.end()) {
    vector.push_back(data);
  }
}

// ========= Brain Dead Language Model (combined entry points) ================

// Given the leftmost word of a line either as a Tesseract unicharset + werd
// or a utf8 string, set the following attributes for it:
//   is_list -      this word might be a list number or bullet.
//   starts_idea -  this word is likely to start a sentence.
//   ends_idea -    this word is likely to end a sentence.
void LeftWordAttributes(const UNICHARSET *unicharset, const WERD_CHOICE *werd, const std::string &utf8,
                        bool *is_list, bool *starts_idea, bool *ends_idea) {
  *is_list = false;
  *starts_idea = false;
  *ends_idea = false;
  if (utf8.empty() || (werd != nullptr && werd->empty())) { // Empty
    *ends_idea = true;
    return;
  }

  if (unicharset && werd) { // We have a proper werd and unicharset so use it.
    if (UniLikelyListItem(unicharset, werd)) {
      *is_list = true;
      *starts_idea = true;
      *ends_idea = true;
    }
    if (unicharset->get_isupper(werd->unichar_id(0))) {
      *starts_idea = true;
    }
    if (unicharset->get_ispunctuation(werd->unichar_id(0))) {
      *starts_idea = true;
      *ends_idea = true;
    }
  } else { // Assume utf8 is mostly ASCII
    if (AsciiLikelyListItem(utf8)) {
      *is_list = true;
      *starts_idea = true;
    }
    int start_letter = utf8[0];
    if (IsOpeningPunct(start_letter)) {
      *starts_idea = true;
    }
    if (IsTerminalPunct(start_letter)) {
      *ends_idea = true;
    }
    if (start_letter >= 'A' && start_letter <= 'Z') {
      *starts_idea = true;
    }
  }
}

// Given the rightmost word of a line either as a Tesseract unicharset + werd
// or a utf8 string, set the following attributes for it:
//   is_list -      this word might be a list number or bullet.
//   starts_idea -  this word is likely to start a sentence.
//   ends_idea -    this word is likely to end a sentence.
void RightWordAttributes(const UNICHARSET *unicharset, const WERD_CHOICE *werd, const std::string &utf8,
                         bool *is_list, bool *starts_idea, bool *ends_idea) {
  *is_list = false;
  *starts_idea = false;
  *ends_idea = false;
  if (utf8.empty() || (werd != nullptr && werd->empty())) { // Empty
    *ends_idea = true;
    return;
  }

  if (unicharset && werd) { // We have a proper werd and unicharset so use it.
    if (UniLikelyListItem(unicharset, werd)) {
      *is_list = true;
      *starts_idea = true;
    }
    UNICHAR_ID last_letter = werd->unichar_id(werd->length() - 1);
    if (unicharset->get_ispunctuation(last_letter)) {
      *ends_idea = true;
    }
  } else { // Assume utf8 is mostly ASCII
    if (AsciiLikelyListItem(utf8)) {
      *is_list = true;
      *starts_idea = true;
    }
    int last_letter = utf8[utf8.size() - 1];
    if (IsOpeningPunct(last_letter) || IsTerminalPunct(last_letter)) {
      *ends_idea = true;
    }
  }
}

// =============== Implementation of RowScratchRegisters =====================
/* static */
void RowScratchRegisters::AppendDebugHeaderFields(std::vector<std::string> &header) {
  header.emplace_back("[lmarg,lind;rind,rmarg]");
  header.emplace_back("model");
}

void RowScratchRegisters::AppendDebugInfo(const ParagraphTheory &theory,
                                          std::vector<std::string> &dbg) const {
  char s[60];
  // The largest (positive and negative) numbers are reported for lindent & rindent.
  // While the column header has widths 5,4,4,5, it is therefore opportune to slightly
  // offset the widths in the format string here to allow ample space for lindent & rindent
  // while keeping the final table output nicely readable: 4,5,5,4.
  snprintf(s, sizeof(s), "[%4d,%5d;%5d,%4d]", lmargin_, lindent_, rindent_, rmargin_);
  dbg.emplace_back(s);
  std::string model_string;
  model_string += static_cast<char>(GetLineType());
  model_string += ":";

  int model_numbers = 0;
  for (const auto &hypothese : hypotheses_) {
    if (hypothese.model == nullptr) {
      continue;
    }
    if (model_numbers > 0) {
      model_string += ",";
    }
    if (StrongModel(hypothese.model)) {
      model_string += std::to_string(1 + theory.IndexOf(hypothese.model));
    } else if (hypothese.model == kCrownLeft) {
      model_string += "CrL";
    } else if (hypothese.model == kCrownRight) {
      model_string += "CrR";
    }
    model_numbers++;
  }
  if (model_numbers == 0) {
    model_string += "0";
  }

  dbg.push_back(model_string);
}

void RowScratchRegisters::Init(const RowInfo &row) {
  ri_ = &row;
  lmargin_ = 0;
  lindent_ = row.pix_ldistance;
  rmargin_ = 0;
  rindent_ = row.pix_rdistance;
}

LineType RowScratchRegisters::GetLineType() const {
  if (hypotheses_.empty()) {
    return LT_UNKNOWN;
  }
  bool has_start = false;
  bool has_body = false;
  for (const auto &hypothese : hypotheses_) {
    switch (hypothese.ty) {
      case LT_START:
        has_start = true;
        break;
      case LT_BODY:
        has_body = true;
        break;
      default:
        tprintf("Encountered bad value in hypothesis list: %c\n", hypothese.ty);
        break;
    }
  }
  if (has_start && has_body) {
    return LT_MULTIPLE;
  }
  return has_start ? LT_START : LT_BODY;
}

LineType RowScratchRegisters::GetLineType(const ParagraphModel *model) const {
  if (hypotheses_.empty()) {
    return LT_UNKNOWN;
  }
  bool has_start = false;
  bool has_body = false;
  for (const auto &hypothese : hypotheses_) {
    if (hypothese.model != model) {
      continue;
    }
    switch (hypothese.ty) {
      case LT_START:
        has_start = true;
        break;
      case LT_BODY:
        has_body = true;
        break;
      default:
        tprintf("Encountered bad value in hypothesis list: %c\n", hypothese.ty);
        break;
    }
  }
  if (has_start && has_body) {
    return LT_MULTIPLE;
  }
  return has_start ? LT_START : LT_BODY;
}

void RowScratchRegisters::SetStartLine() {
  LineType current_lt = GetLineType();
  if (current_lt != LT_UNKNOWN && current_lt != LT_START) {
    tprintf("Trying to set a line to be START when it's already BODY.\n");
  }
  if (current_lt == LT_UNKNOWN || current_lt == LT_BODY) {
    push_back_new(hypotheses_, LineHypothesis(LT_START, nullptr));
  }
}

void RowScratchRegisters::SetBodyLine() {
  LineType current_lt = GetLineType();
  if (current_lt != LT_UNKNOWN && current_lt != LT_BODY) {
    tprintf("Trying to set a line to be BODY when it's already START.\n");
  }
  if (current_lt == LT_UNKNOWN || current_lt == LT_START) {
    push_back_new(hypotheses_, LineHypothesis(LT_BODY, nullptr));
  }
}

void RowScratchRegisters::AddStartLine(const ParagraphModel *model) {
  push_back_new(hypotheses_, LineHypothesis(LT_START, model));
  auto found = std::find(hypotheses_.begin(), hypotheses_.end(), LineHypothesis(LT_START, nullptr));
  if (found != hypotheses_.end()) {
    hypotheses_.erase(found);
  }
}

void RowScratchRegisters::AddBodyLine(const ParagraphModel *model) {
  push_back_new(hypotheses_, LineHypothesis(LT_BODY, model));
  auto found = std::find(hypotheses_.begin(), hypotheses_.end(), LineHypothesis(LT_BODY, nullptr));
  if (found != hypotheses_.end()) {
    hypotheses_.erase(found);
  }
}

void RowScratchRegisters::StartHypotheses(SetOfModels *models) const {
  for (const auto &hypothese : hypotheses_) {
    if (hypothese.ty == LT_START && StrongModel(hypothese.model)) {
      push_back_new(*models, hypothese.model);
    }
  }
}

void RowScratchRegisters::StrongHypotheses(SetOfModels *models) const {
  for (const auto &hypothese : hypotheses_) {
    if (StrongModel(hypothese.model)) {
      push_back_new(*models, hypothese.model);
    }
  }
}

void RowScratchRegisters::NonNullHypotheses(SetOfModels *models) const {
  for (const auto &hypothese : hypotheses_) {
    if (hypothese.model != nullptr) {
      push_back_new(*models, hypothese.model);
    }
  }
}

const ParagraphModel *RowScratchRegisters::UniqueStartHypothesis() const {
  if (hypotheses_.size() != 1 || hypotheses_[0].ty != LT_START) {
    return nullptr;
  }
  return hypotheses_[0].model;
}

const ParagraphModel *RowScratchRegisters::UniqueBodyHypothesis() const {
  if (hypotheses_.size() != 1 || hypotheses_[0].ty != LT_BODY) {
    return nullptr;
  }
  return hypotheses_[0].model;
}

// Discard any hypotheses whose model is not in the given list.
void RowScratchRegisters::DiscardNonMatchingHypotheses(const SetOfModels &models) {
  if (models.empty()) {
    return;
  }
  for (int h = hypotheses_.size() - 1; h >= 0; h--) {
    if (!contains(models, hypotheses_[h].model)) {
      hypotheses_.erase(hypotheses_.begin() + h);
    }
  }
}

// ============ Geometry based Paragraph Detection Algorithm =================

struct Cluster {
  Cluster() : center(0), count(0) {}
  Cluster(int cen, int num) : center(cen), count(num) {}

  int center; // The center of the cluster.
  int count;  // The number of entries within the cluster.
};

class SimpleClusterer {
public:
  explicit SimpleClusterer(int max_cluster_width) : max_cluster_width_(max_cluster_width) {}
  void Add(int value) {
    values_.push_back(value);
  }
  size_t size() const {
    return values_.size();
  }
  void GetClusters(std::vector<Cluster> *clusters);

private:
  int max_cluster_width_;
  std::vector<int> values_;
};

// Return the index of the cluster closest to value.
static int ClosestCluster(const std::vector<Cluster> &clusters, int value) {
  unsigned best_index = 0;
  for (unsigned i = 0; i < clusters.size(); i++) {
    if (abs(value - clusters[i].center) < abs(value - clusters[best_index].center)) {
      best_index = i;
    }
  }
  return best_index;
}

void SimpleClusterer::GetClusters(std::vector<Cluster> *clusters) {
  clusters->clear();
  std::sort(values_.begin(), values_.end());
  for (unsigned i = 0; i < values_.size();) {
    int orig_i = i;
    int lo = values_[i];
    int hi = lo;
    while (++i < values_.size() && values_[i] <= lo + max_cluster_width_) {
      hi = values_[i];
    }
    clusters->push_back(Cluster((hi + lo) / 2, i - orig_i));
  }
}

// Calculate left- and right-indent tab stop values seen in
// rows[row_start, row_end) given a tolerance of tolerance.
static void CalculateTabStops(std::vector<RowScratchRegisters> *rows, int row_start, int row_end,
                              int tolerance, std::vector<Cluster> *left_tabs,
                              std::vector<Cluster> *right_tabs) {
  if (!AcceptableRowArgs(0, 1, __func__, rows, row_start, row_end)) {
    return;
  }
  // First pass: toss all left and right indents into clusterers.
  SimpleClusterer initial_lefts(tolerance);
  SimpleClusterer initial_rights(tolerance);
  std::vector<Cluster> initial_left_tabs;
  std::vector<Cluster> initial_right_tabs;
  for (int i = row_start; i < row_end; i++) {
    initial_lefts.Add((*rows)[i].lindent_);
    initial_rights.Add((*rows)[i].rindent_);
  }
  initial_lefts.GetClusters(&initial_left_tabs);
  initial_rights.GetClusters(&initial_right_tabs);

  // Second pass: cluster only lines that are not "stray"
  //   An example of a stray line is a page number -- a line whose start
  //   and end tab-stops are far outside the typical start and end tab-stops
  //   for the block.
  //   Put another way, we only cluster data from lines whose start or end
  //   tab stop is frequent.
  SimpleClusterer lefts(tolerance);
  SimpleClusterer rights(tolerance);

  // Outlier elimination.  We might want to switch this to test outlier-ness
  // based on how strange a position an outlier is in instead of or in addition
  // to how rare it is.  These outliers get re-added if we end up having too
  // few tab stops, to work with, however.
  int infrequent_enough_to_ignore = 0;
  if (row_end - row_start >= 8) {
    infrequent_enough_to_ignore = 1;
  }
  if (row_end - row_start >= 20) {
    infrequent_enough_to_ignore = 2;
  }

  for (int i = row_start; i < row_end; i++) {
    int lidx = ClosestCluster(initial_left_tabs, (*rows)[i].lindent_);
    int ridx = ClosestCluster(initial_right_tabs, (*rows)[i].rindent_);
    if (initial_left_tabs[lidx].count > infrequent_enough_to_ignore ||
        initial_right_tabs[ridx].count > infrequent_enough_to_ignore) {
      lefts.Add((*rows)[i].lindent_);
      rights.Add((*rows)[i].rindent_);
    }
  }
  lefts.GetClusters(left_tabs);
  rights.GetClusters(right_tabs);

  if ((left_tabs->size() == 1 && right_tabs->size() >= 4) ||
      (right_tabs->size() == 1 && left_tabs->size() >= 4)) {
    // One side is really ragged, and the other only has one tab stop,
    // so those "insignificant outliers" are probably important, actually.
    // This often happens on a page of an index.  Add back in the ones
    // we omitted in the first pass.
    for (int i = row_start; i < row_end; i++) {
      int lidx = ClosestCluster(initial_left_tabs, (*rows)[i].lindent_);
      int ridx = ClosestCluster(initial_right_tabs, (*rows)[i].rindent_);
      if (!(initial_left_tabs[lidx].count > infrequent_enough_to_ignore ||
            initial_right_tabs[ridx].count > infrequent_enough_to_ignore)) {
        lefts.Add((*rows)[i].lindent_);
        rights.Add((*rows)[i].rindent_);
      }
    }
  }
  lefts.GetClusters(left_tabs);
  rights.GetClusters(right_tabs);

  // If one side is almost a two-indent aligned side, and the other clearly
  // isn't, try to prune out the least frequent tab stop from that side.
  if (left_tabs->size() == 3 && right_tabs->size() >= 4) {
    int to_prune = -1;
    for (int i = left_tabs->size() - 1; i >= 0; i--) {
      if (to_prune < 0 || (*left_tabs)[i].count < (*left_tabs)[to_prune].count) {
        to_prune = i;
      }
    }
    if (to_prune >= 0 && (*left_tabs)[to_prune].count <= infrequent_enough_to_ignore) {
      left_tabs->erase(left_tabs->begin() + to_prune);
    }
  }
  if (right_tabs->size() == 3 && left_tabs->size() >= 4) {
    int to_prune = -1;
    for (int i = right_tabs->size() - 1; i >= 0; i--) {
      if (to_prune < 0 || (*right_tabs)[i].count < (*right_tabs)[to_prune].count) {
        to_prune = i;
      }
    }
    if (to_prune >= 0 && (*right_tabs)[to_prune].count <= infrequent_enough_to_ignore) {
      right_tabs->erase(right_tabs->begin() + to_prune);
    }
  }
}

// Given a paragraph model mark rows[row_start, row_end) as said model
// start or body lines.
//
// Case 1: model->first_indent_ != model->body_indent_
//   Differentiating the paragraph start lines from the paragraph body lines in
//   this case is easy, we just see how far each line is indented.
//
// Case 2: model->first_indent_ == model->body_indent_
//   Here, we find end-of-paragraph lines by looking for "short lines."
//   What constitutes a "short line" changes depending on whether the text
//   ragged-right[left] or fully justified (aligned left and right).
//
//   Case 2a: Ragged Right (or Left) text.  (eop_threshold == 0)
//     We have a new paragraph it the first word would have at the end
//     of the previous line.
//
//   Case 2b: Fully Justified.  (eop_threshold > 0)
//     We mark a line as short (end of paragraph) if the offside indent
//     is greater than eop_threshold.
static void MarkRowsWithModel(std::vector<RowScratchRegisters> *rows, int row_start, int row_end,
                              const ParagraphModel *model, bool ltr, int eop_threshold) {
  if (!AcceptableRowArgs(0, 0, __func__, rows, row_start, row_end)) {
    return;
  }
  for (int row = row_start; row < row_end; row++) {
    bool valid_first = ValidFirstLine(rows, row, model);
    bool valid_body = ValidBodyLine(rows, row, model);
    if (valid_first && !valid_body) {
      (*rows)[row].AddStartLine(model);
    } else if (valid_body && !valid_first) {
      (*rows)[row].AddBodyLine(model);
    } else if (valid_body && valid_first) {
      bool after_eop = (row == row_start);
      if (row > row_start) {
        if (eop_threshold > 0) {
          if (model->justification() == JUSTIFICATION_LEFT) {
            after_eop = (*rows)[row - 1].rindent_ > eop_threshold;
          } else {
            after_eop = (*rows)[row - 1].lindent_ > eop_threshold;
          }
        } else {
          after_eop = FirstWordWouldHaveFit((*rows)[row - 1], (*rows)[row], model->justification());
        }
      }
      if (after_eop) {
        (*rows)[row].AddStartLine(model);
      } else {
        (*rows)[row].AddBodyLine(model);
      }
    } else {
      // Do nothing. Stray row.
    }
  }
}

// GeometricClassifierState holds all of the information we'll use while
// trying to determine a paragraph model for the text lines in a block of
// text:
//   + the rows under consideration [row_start, row_end)
//   + the common left- and right-indent tab stops
//   + does the block start out left-to-right or right-to-left
// Further, this struct holds the data we amass for the (single) ParagraphModel
// we'll assign to the text lines (assuming we get that far).
struct GeometricClassifierState {
  GeometricClassifierState(int dbg_level, std::vector<RowScratchRegisters> *r, int r_start,
                           int r_end)
      : debug_level(dbg_level), rows(r), row_start(r_start), row_end(r_end) {
    tolerance = InterwordSpace(*r, r_start, r_end);
    CalculateTabStops(r, r_start, r_end, tolerance, &left_tabs, &right_tabs);
    if (debug_level >= 3) {
      tesserr << "Geometry: TabStop cluster tolerance = " << tolerance << "; "
              << left_tabs.size() << " left tabs; "
              << right_tabs.size() << " right tabs\n";
    }
    ltr = (*r)[r_start].ri_->ltr;
  }

  void AssumeLeftJustification() {
    just = tesseract::JUSTIFICATION_LEFT;
    margin = (*rows)[row_start].lmargin_;
  }

  void AssumeRightJustification() {
    just = tesseract::JUSTIFICATION_RIGHT;
    margin = (*rows)[row_start].rmargin_;
  }

  // Align tabs are the tab stops the text is aligned to.
  const std::vector<Cluster> &AlignTabs() const {
    if (just == tesseract::JUSTIFICATION_RIGHT) {
      return right_tabs;
    }
    return left_tabs;
  }

  // Offside tabs are the tab stops opposite the tabs used to align the text.
  //
  // Note that for a left-to-right text which is aligned to the right such as
  //     this function comment, the offside tabs are the horizontal tab stops
  //                 marking the beginning of ("Note", "this" and "marking").
  const std::vector<Cluster> &OffsideTabs() const {
    if (just == tesseract::JUSTIFICATION_RIGHT) {
      return left_tabs;
    }
    return right_tabs;
  }

  // Return whether the i'th row extends from the leftmost left tab stop
  // to the right most right tab stop.
  bool IsFullRow(int i) const {
    return ClosestCluster(left_tabs, (*rows)[i].lindent_) == 0 &&
           ClosestCluster(right_tabs, (*rows)[i].rindent_) == 0;
  }

  int AlignsideTabIndex(int row_idx) const {
    return ClosestCluster(AlignTabs(), (*rows)[row_idx].AlignsideIndent(just));
  }

  // Given what we know about the paragraph justification (just), would the
  // first word of row_b have fit at the end of row_a?
  bool FirstWordWouldHaveFit(int row_a, int row_b) {
    return ::tesseract::FirstWordWouldHaveFit((*rows)[row_a], (*rows)[row_b], just);
  }

  void PrintRows() const {
    PrintRowRange(*rows, row_start, row_end);
  }

  void Fail(int min_debug_level, const char *why) const {
    if (debug_level < min_debug_level) {
      return;
    }
    tprintf("# %s\n", why);
    PrintRows();
  }

  ParagraphModel Model() const {
    return ParagraphModel(just, margin, first_indent, body_indent, tolerance);
  }

  // We print out messages with a debug level at least as great as debug_level.
  int debug_level = 0;

  // The Geometric Classifier was asked to find a single paragraph model
  // to fit the text rows (*rows)[row_start, row_end)
  std::vector<RowScratchRegisters> *rows;
  int row_start = 0;
  int row_end = 0;

  // The amount by which we expect the text edge can vary and still be aligned.
  int tolerance = 0;

  // Is the script in this text block left-to-right?
  // HORRIBLE ROUGH APPROXIMATION.  TODO(eger): Improve
  bool ltr = false;

  // These left and right tab stops were determined to be the common tab
  // stops for the given text.
  std::vector<Cluster> left_tabs;
  std::vector<Cluster> right_tabs;

  // These are parameters we must determine to create a ParagraphModel.
  tesseract::ParagraphJustification just = JUSTIFICATION_UNKNOWN;
  int margin = 0;
  int first_indent = 0;
  int body_indent = 0;

  // eop_threshold > 0 if the text is fully justified.  See MarkRowsWithModel()
  int eop_threshold = 0;
};

// Given a section of text where strong textual clues did not help identifying
// paragraph breaks, and for which the left and right indents have exactly
// three tab stops between them, attempt to find the paragraph breaks based
// solely on the outline of the text and whether the script is left-to-right.
//
// Algorithm Detail:
//   The selected rows are in the form of a rectangle except
//   for some number of "short lines" of the same length:
//
//   (A1)  xxxxxxxxxxxxx  (B1) xxxxxxxxxxxx
//           xxxxxxxxxxx       xxxxxxxxxx    # A "short" line.
//         xxxxxxxxxxxxx       xxxxxxxxxxxx
//         xxxxxxxxxxxxx       xxxxxxxxxxxx
//
//   We have a slightly different situation if the only short
//   line is at the end of the excerpt.
//
//   (A2) xxxxxxxxxxxxx  (B2) xxxxxxxxxxxx
//        xxxxxxxxxxxxx       xxxxxxxxxxxx
//        xxxxxxxxxxxxx       xxxxxxxxxxxx
//          xxxxxxxxxxx       xxxxxxxxxx     # A "short" line.
//
//   We'll interpret these as follows based on the reasoning in the comment for
//   GeometricClassify():
//       [script direction: first indent, body indent]
//   (A1) LtR: 2,0  RtL: 0,0   (B1) LtR: 0,0  RtL: 2,0
//   (A2) LtR: 2,0  RtL: CrR   (B2) LtR: CrL  RtL: 2,0
static void GeometricClassifyThreeTabStopTextBlock(int debug_level, GeometricClassifierState &s,
                                                   ParagraphTheory *theory) {
  int num_rows = s.row_end - s.row_start;
  int num_full_rows = 0;
  int last_row_full = 0;
  for (int i = s.row_start; i < s.row_end; i++) {
    if (s.IsFullRow(i)) {
      num_full_rows++;
      if (i == s.row_end - 1) {
        last_row_full++;
      }
    }
  }

  if (num_full_rows < 0.7 * num_rows) {
    s.Fail(1, "Not enough full lines to know which lines start paras.");
    return;
  }

  // eop_threshold gets set if we're fully justified; see MarkRowsWithModel()
  s.eop_threshold = 0;

  if (s.ltr) {
    s.AssumeLeftJustification();
  } else {
    s.AssumeRightJustification();
  }

  if (debug_level > 0) {
    tprintf(
        "# Not enough variety for clear outline classification. "
        "Guessing these are %s aligned based on script.\n",
        s.ltr ? "left" : "right");
    s.PrintRows();
  }

  if (s.AlignTabs().size() == 2) { // case A1 or A2
    s.first_indent = s.AlignTabs()[1].center;
    s.body_indent = s.AlignTabs()[0].center;
  } else { // case B1 or B2
    if (num_rows - 1 == num_full_rows - last_row_full) {
      // case B2
      const ParagraphModel *model = s.ltr ? kCrownLeft : kCrownRight;
      (*s.rows)[s.row_start].AddStartLine(model);
      for (int i = s.row_start + 1; i < s.row_end; i++) {
        (*s.rows)[i].AddBodyLine(model);
      }
      return;
    } else {
      // case B1
      s.first_indent = s.body_indent = s.AlignTabs()[0].center;
      s.eop_threshold = (s.OffsideTabs()[0].center + s.OffsideTabs()[1].center) / 2;
    }
  }
  const ParagraphModel *model = theory->AddModel(s.Model());
  MarkRowsWithModel(s.rows, s.row_start, s.row_end, model, s.ltr, s.eop_threshold);
  return;
}

// This function is called if strong textual clues were not available, but
// the caller hopes that the paragraph breaks will be super obvious just
// by the outline of the text.
//
// The particularly difficult case is figuring out what's going on if you
// don't have enough short paragraph end lines to tell us what's going on.
//
// For instance, let's say you have the following outline:
//
//   (A1)  xxxxxxxxxxxxxxxxxxxxxx
//           xxxxxxxxxxxxxxxxxxxx
//         xxxxxxxxxxxxxxxxxxxxxx
//         xxxxxxxxxxxxxxxxxxxxxx
//
// Even if we know that the text is left-to-right and so will probably be
// left-aligned, both of the following are possible texts:
//
//  (A1a)  1. Here our list item
//           with two full lines.
//         2. Here a second item.
//         3. Here our third one.
//
//  (A1b)  so ends paragraph one.
//           Here  starts another
//         paragraph  we want  to
//         read.  This  continues
//
// These examples are obvious from the text and should have been caught
// by the StrongEvidenceClassify pass.  However, for languages where we don't
// have capital letters to go on (e.g. Hebrew, Arabic, Hindi, Chinese),
// it's worth guessing that (A1b) is the correct interpretation if there are
// far more "full" lines than "short" lines.
static void GeometricClassify(int debug_level, std::vector<RowScratchRegisters> *rows,
                              int row_start, int row_end, ParagraphTheory *theory) {
  if (!AcceptableRowArgs(debug_level, 4, __func__, rows, row_start, row_end)) {
    return;
  }
  if (debug_level > 1) {
    tprintf("###############################################\n");
    tprintf("##### GeometricClassify( rows[%d:%d) )   ####\n", row_start, row_end);
    tprintf("###############################################\n");
  }
  RecomputeMarginsAndClearHypotheses(rows, row_start, row_end, 10);

  GeometricClassifierState s(debug_level, rows, row_start, row_end);
  if (s.left_tabs.size() > 2 && s.right_tabs.size() > 2) {
    s.Fail(2, "Too much variety for simple outline classification.");
    return;
  }
  if (s.left_tabs.size() <= 1 && s.right_tabs.size() <= 1) {
    s.Fail(1, "Not enough variety for simple outline classification.");
    return;
  }
  if (s.left_tabs.size() + s.right_tabs.size() == 3) {
    GeometricClassifyThreeTabStopTextBlock(debug_level, s, theory);
    return;
  }

  // At this point, we know that one side has at least two tab stops, and the
  // other side has one or two tab stops.
  // Left to determine:
  //   (1) Which is the body indent and which is the first line indent?
  //   (2) Is the text fully justified?

  // If one side happens to have three or more tab stops, assume that side
  // is opposite of the aligned side.
  if (s.right_tabs.size() > 2) {
    s.AssumeLeftJustification();
  } else if (s.left_tabs.size() > 2) {
    s.AssumeRightJustification();
  } else if (s.ltr) { // guess based on script direction
    s.AssumeLeftJustification();
  } else {
    s.AssumeRightJustification();
  }

  if (s.AlignTabs().size() == 2) {
    // For each tab stop on the aligned side, how many of them appear
    // to be paragraph start lines?  [first lines]
    int firsts[2] = {0, 0};
    // Count the first line as a likely paragraph start line.
    firsts[s.AlignsideTabIndex(s.row_start)]++;
    // For each line, if the first word would have fit on the previous
    // line count it as a likely paragraph start line.
    bool jam_packed = true;
    for (int i = s.row_start + 1; i < s.row_end; i++) {
      if (s.FirstWordWouldHaveFit(i - 1, i)) {
        firsts[s.AlignsideTabIndex(i)]++;
        jam_packed = false;
      }
    }
    // Make an extra accounting for the last line of the paragraph just
    // in case it's the only short line in the block.  That is, take its
    // first word as typical and see if this looks like the *last* line
    // of a paragraph.  If so, mark the *other* indent as probably a first.
    if (jam_packed && s.FirstWordWouldHaveFit(s.row_end - 1, s.row_end - 1)) {
      firsts[1 - s.AlignsideTabIndex(s.row_end - 1)]++;
    }

    int percent0firsts, percent1firsts;
    percent0firsts = (100 * firsts[0]) / s.AlignTabs()[0].count;
    percent1firsts = (100 * firsts[1]) / s.AlignTabs()[1].count;

    // TODO(eger): Tune these constants if necessary.
    if ((percent0firsts < 20 && 30 < percent1firsts) || percent0firsts + 30 < percent1firsts) {
      s.first_indent = s.AlignTabs()[1].center;
      s.body_indent = s.AlignTabs()[0].center;
    } else if ((percent1firsts < 20 && 30 < percent0firsts) ||
               percent1firsts + 30 < percent0firsts) {
      s.first_indent = s.AlignTabs()[0].center;
      s.body_indent = s.AlignTabs()[1].center;
    } else {
      // Ambiguous! Probably lineated (poetry)
      if (debug_level > 1) {
        tprintf("# Cannot determine %s indent likely to start paragraphs.\n",
                s.just == tesseract::JUSTIFICATION_LEFT ? "left" : "right");
        tprintf("# Indent of %d looks like a first line %d%% of the time.\n",
                s.AlignTabs()[0].center, percent0firsts);
        tprintf("# Indent of %d looks like a first line %d%% of the time.\n",
                s.AlignTabs()[1].center, percent1firsts);
        s.PrintRows();
      }
      return;
    }
  } else {
    // There's only one tab stop for the "aligned to" side.
    s.first_indent = s.body_indent = s.AlignTabs()[0].center;
  }

  // At this point, we have our model.
  const ParagraphModel *model = theory->AddModel(s.Model());

  // Now all we have to do is figure out if the text is fully justified or not.
  // eop_threshold: default to fully justified unless we see evidence below.
  //    See description on MarkRowsWithModel()
  s.eop_threshold = (s.OffsideTabs()[0].center + s.OffsideTabs()[1].center) / 2;
  // If the text is not fully justified, re-set the eop_threshold to 0.
  if (s.AlignTabs().size() == 2) {
    // Paragraphs with a paragraph-start indent.
    for (int i = s.row_start; i < s.row_end - 1; i++) {
      if (ValidFirstLine(s.rows, i + 1, model) &&
          !NearlyEqual(s.OffsideTabs()[0].center, (*s.rows)[i].OffsideIndent(s.just),
                       s.tolerance)) {
        // We found a non-end-of-paragraph short line: not fully justified.
        s.eop_threshold = 0;
        break;
      }
    }
  } else {
    // Paragraphs with no paragraph-start indent.
    for (int i = s.row_start; i < s.row_end - 1; i++) {
      if (!s.FirstWordWouldHaveFit(i, i + 1) &&
          !NearlyEqual(s.OffsideTabs()[0].center, (*s.rows)[i].OffsideIndent(s.just),
                       s.tolerance)) {
        // We found a non-end-of-paragraph short line: not fully justified.
        s.eop_threshold = 0;
        break;
      }
    }
  }
  MarkRowsWithModel(rows, row_start, row_end, model, s.ltr, s.eop_threshold);
}

// =============== Implementation of ParagraphTheory =====================

const ParagraphModel *ParagraphTheory::AddModel(const ParagraphModel &model) {
  for (const auto &m : *models_) {
    if (m->Comparable(model)) {
      return m;
    }
  }
  auto *m = new ParagraphModel(model);
  models_->push_back(m);
  push_back_new(models_we_added_, m);
  return m;
}

void ParagraphTheory::DiscardUnusedModels(const SetOfModels &used_models) {
  size_t w = 0;
  for (size_t r = 0; r < models_->size(); r++) {
    ParagraphModel *m = (*models_)[r];
    if (!contains(used_models, static_cast<const ParagraphModel *>(m)) && contains(models_we_added_, m)) {
      delete m;
    } else {
      if (r > w) {
        (*models_)[w] = m;
      }
      w++;
    }
  }
  models_->resize(w);
}

// Examine rows[start, end) and try to determine if an existing non-centered
// paragraph model would fit them perfectly.  If so, return a pointer to it.
// If not, return nullptr.
const ParagraphModel *ParagraphTheory::Fits(const std::vector<RowScratchRegisters> *rows,
                                            int start, int end) const {
  for (const auto *model : *models_) {
    if (model->justification() != JUSTIFICATION_CENTER && RowsFitModel(rows, start, end, model)) {
      return model;
    }
  }
  return nullptr;
}

void ParagraphTheory::NonCenteredModels(SetOfModels *models) {
  for (const auto *model : *models_) {
    if (model->justification() != JUSTIFICATION_CENTER) {
      push_back_new(*models, model);
    }
  }
}

int ParagraphTheory::IndexOf(const ParagraphModel *model) const {
  int i = 0;
  for (const auto *m : *models_) {
    if (m == model) {
      return i;
    }
    i++;
  }
  return -1;
}

bool ValidFirstLine(const std::vector<RowScratchRegisters> *rows, int row,
                    const ParagraphModel *model) {
  if (!StrongModel(model)) {
    tprintf("ValidFirstLine() should only be called with strong models!\n");
  }
  return StrongModel(model) && model->ValidFirstLine((*rows)[row].lmargin_, (*rows)[row].lindent_,
                                                     (*rows)[row].rindent_, (*rows)[row].rmargin_);
}

bool ValidBodyLine(const std::vector<RowScratchRegisters> *rows, int row,
                   const ParagraphModel *model) {
  if (!StrongModel(model)) {
    tprintf("ValidBodyLine() should only be called with strong models!\n");
  }
  return StrongModel(model) && model->ValidBodyLine((*rows)[row].lmargin_, (*rows)[row].lindent_,
                                                    (*rows)[row].rindent_, (*rows)[row].rmargin_);
}

bool CrownCompatible(const std::vector<RowScratchRegisters> *rows, int a, int b,
                     const ParagraphModel *model) {
  if (model != kCrownRight && model != kCrownLeft) {
    tprintf("CrownCompatible() should only be called with crown models!\n");
    return false;
  }
  auto &row_a = (*rows)[a];
  auto &row_b = (*rows)[b];
  if (model == kCrownRight) {
    return NearlyEqual(row_a.rindent_ + row_a.rmargin_, row_b.rindent_ + row_b.rmargin_,
                       Epsilon(row_a.ri_->average_interword_space));
  }
  return NearlyEqual(row_a.lindent_ + row_a.lmargin_, row_b.lindent_ + row_b.lmargin_,
                     Epsilon(row_a.ri_->average_interword_space));
}

// =============== Implementation of ParagraphModelSmearer ====================

ParagraphModelSmearer::ParagraphModelSmearer(std::vector<RowScratchRegisters> *rows,
                                             int row_start, int row_end, ParagraphTheory *theory)
    : theory_(theory), rows_(rows), row_start_(row_start), row_end_(row_end) {
  if (!AcceptableRowArgs(0, 0, __func__, rows, row_start, row_end)) {
    row_start_ = 0;
    row_end_ = 0;
    return;
  }
  open_models_.resize(open_models_.size() + row_end - row_start + 2);
}

// see paragraphs_internal.h
void ParagraphModelSmearer::CalculateOpenModels(int row_start, int row_end) {
  SetOfModels no_models;
  if (row_start < row_start_) {
    row_start = row_start_;
  }
  if (row_end > row_end_) {
    row_end = row_end_;
  }

  for (int row = (row_start > 0) ? row_start - 1 : row_start; row < row_end; row++) {
    if ((*rows_)[row].ri_->num_words == 0) {
      OpenModels(row + 1) = no_models;
    } else {
      SetOfModels &opened = OpenModels(row);
      (*rows_)[row].StartHypotheses(&opened);

      // Which models survive the transition from row to row + 1?
      SetOfModels still_open;
      for (auto &m : opened) {
        if (ValidFirstLine(rows_, row, m) || ValidBodyLine(rows_, row, m)) {
          // This is basic filtering; we check likely paragraph starty-ness down
          // below in Smear() -- you know, whether the first word would have fit
          // and such.
          push_back_new(still_open, m);
        }
      }
      OpenModels(row + 1) = std::move(still_open);
    }
  }
}

// see paragraphs_internal.h
void ParagraphModelSmearer::Smear() {
  CalculateOpenModels(row_start_, row_end_);

  // For each row which we're unsure about (that is, it is LT_UNKNOWN or
  // we have multiple LT_START hypotheses), see if there's a model that
  // was recently used (an "open" model) which might model it well.
  for (int i = row_start_; i < row_end_; i++) {
    RowScratchRegisters &row = (*rows_)[i];
    if (row.ri_->num_words == 0) {
      continue;
    }

    // Step One:
    //   Figure out if there are "open" models which are left-alined or
    //   right-aligned.  This is important for determining whether the
    //   "first" word in a row would fit at the "end" of the previous row.
    bool left_align_open = false;
    bool right_align_open = false;
    for (auto &m : OpenModels(i)) {
      switch (m->justification()) {
        case JUSTIFICATION_LEFT:
          left_align_open = true;
          break;
        case JUSTIFICATION_RIGHT:
          right_align_open = true;
          break;
        default:
          left_align_open = right_align_open = true;
      }
    }
    // Step Two:
    //   Use that knowledge to figure out if this row is likely to
    //   start a paragraph.
    bool likely_start;
    if (i == 0) {
      likely_start = true;
    } else {
      if ((left_align_open && right_align_open) || (!left_align_open && !right_align_open)) {
        likely_start = LikelyParagraphStart((*rows_)[i - 1], row, JUSTIFICATION_LEFT) ||
                       LikelyParagraphStart((*rows_)[i - 1], row, JUSTIFICATION_RIGHT);
      } else if (left_align_open) {
        likely_start = LikelyParagraphStart((*rows_)[i - 1], row, JUSTIFICATION_LEFT);
      } else {
        likely_start = LikelyParagraphStart((*rows_)[i - 1], row, JUSTIFICATION_RIGHT);
      }
    }

    // Step Three:
    //   If this text line seems like an obvious first line of an
    //   open model, or an obvious continuation of an existing
    //   modelled paragraph, mark it up.
    if (likely_start) {
      // Add Start Hypotheses for all Open models that fit.
      for (unsigned m = 0; m < OpenModels(i).size(); m++) {
        if (ValidFirstLine(rows_, i, OpenModels(i)[m])) {
          row.AddStartLine(OpenModels(i)[m]);
        }
      }
    } else {
      // Add relevant body line hypotheses.
      SetOfModels last_line_models;
      if (i > 0) {
        (*rows_)[i - 1].StrongHypotheses(&last_line_models);
      } else {
        theory_->NonCenteredModels(&last_line_models);
      }
      for (auto model : last_line_models) {
        if (ValidBodyLine(rows_, i, model)) {
          row.AddBodyLine(model);
        }
      }
    }

    // Step Four:
    //   If we're still quite unsure about this line, go through all
    //   models in our theory and see if this row could be the start
    //   of any of our  models.
    if (row.GetLineType() == LT_UNKNOWN ||
        (row.GetLineType() == LT_START && !row.UniqueStartHypothesis())) {
      SetOfModels all_models;
      theory_->NonCenteredModels(&all_models);
      for (auto &all_model : all_models) {
        if (ValidFirstLine(rows_, i, all_model)) {
          row.AddStartLine(all_model);
        }
      }
    }
    // Step Five:
    //   Since we may have updated the hypotheses about this row, we need
    //   to recalculate the Open models for the rest of rows[i + 1, row_end)
    if (row.GetLineType() != LT_UNKNOWN) {
      CalculateOpenModels(i + 1, row_end_);
    }
  }
}

// ================ Main Paragraph Detection Algorithm =======================

// Find out what ParagraphModels are actually used, and discard any
// that are not.
static void DiscardUnusedModels(const std::vector<RowScratchRegisters> &rows,
                                ParagraphTheory *theory) {
  SetOfModels used_models;
  for (const auto &row : rows) {
    row.StrongHypotheses(&used_models);
  }
  theory->DiscardUnusedModels(used_models);
}

// DowngradeWeakestToCrowns:
//   Forget any flush-{left, right} models unless we see two or more
//   of them in sequence.
//
// In pass 3, we start to classify even flush-left paragraphs (paragraphs
// where the first line and body indent are the same) as having proper Models.
// This is generally dangerous, since if you start imagining that flush-left
// is a typical paragraph model when it is not, it will lead you to chop normal
// indented paragraphs in the middle whenever a sentence happens to start on a
// new line (see "This" above).  What to do?
//   What we do is to take any paragraph which is flush left and is not
// preceded by another paragraph of the same model and convert it to a "Crown"
// paragraph.  This is a weak pseudo-ParagraphModel which is a placeholder
// for later.  It means that the paragraph is flush, but it would be desirable
// to mark it as the same model as following text if it fits.  This downgrade
// FlushLeft -> CrownLeft -> Model of following paragraph.  Means that we
// avoid making flush left Paragraph Models whenever we see a top-of-the-page
// half-of-a-paragraph. and instead we mark it the same as normal body text.
//
// Implementation:
//
//   Comb backwards through the row scratch registers, and turn any
//   sequences of body lines of equivalent type abutted against the beginning
//   or a body or start line of a different type into a crown paragraph.
static void DowngradeWeakestToCrowns(int debug_level, ParagraphTheory *theory,
                                     std::vector<RowScratchRegisters> *rows) {
  int start;
  for (int end = rows->size(); end > 0; end = start) {
    // Search back for a body line of a unique type.
    const ParagraphModel *model = nullptr;
    while (end > 0 && (model = (*rows)[end - 1].UniqueBodyHypothesis()) == nullptr) {
      end--;
    }
    if (end == 0) {
      break;
    }
    start = end - 1;
    while (start >= 0 && (*rows)[start].UniqueBodyHypothesis() == model) {
      start--; // walk back to the first line that is not the same body type.
    }
    if (start >= 0 && (*rows)[start].UniqueStartHypothesis() == model && StrongModel(model) &&
        NearlyEqual(model->first_indent(), model->body_indent(), model->tolerance())) {
      start--;
    }
    start++;
    // Now rows[start, end) is a sequence of unique body hypotheses of model.
    if (StrongModel(model) && model->justification() == JUSTIFICATION_CENTER) {
      continue;
    }
    if (!StrongModel(model)) {
      while (start > 0 && CrownCompatible(rows, start - 1, start, model)) {
        start--;
      }
    }
    if (start == 0 || (!StrongModel(model)) ||
        (StrongModel(model) && !ValidFirstLine(rows, start - 1, model))) {
      // crownify rows[start, end)
      const ParagraphModel *crown_model = model;
      if (StrongModel(model)) {
        if (model->justification() == JUSTIFICATION_LEFT) {
          crown_model = kCrownLeft;
        } else {
          crown_model = kCrownRight;
        }
      }
      (*rows)[start].SetUnknown();
      (*rows)[start].AddStartLine(crown_model);
      for (int row = start + 1; row < end; row++) {
        (*rows)[row].SetUnknown();
        (*rows)[row].AddBodyLine(crown_model);
      }
    }
  }
  DiscardUnusedModels(*rows, theory);
}

// Clear all hypotheses about lines [start, end) and reset margins.
//
// The empty space between the left of a row and the block boundary (and
// similarly for the right) is split into two pieces: margin and indent.
// In initial processing, we assume the block is tight and the margin for
// all lines is set to zero.   However, if our first pass does not yield
// models for  everything,  it may be  due to an  inset paragraph like a
// block-quote.   In that case, we make a second pass over that unmarked
// section of the page and reset the "margin" portion of the empty space
// to the common amount of space at  the ends of the lines under consid-
// eration.    This would be equivalent to percentile set to 0. However,
// sometimes we have a single character sticking out in the right margin
// of a text block  (like the 'r' in 'for' on line 3 above),  and we can
// really  just ignore it as an outlier.   To express this, we allow the
// user to specify  the percentile (0..100)  of indent values  to use as
// the common margin for each row in the run of rows[start, end).
void RecomputeMarginsAndClearHypotheses(std::vector<RowScratchRegisters> *rows, int start,
                                        int end, int percentile) {
  if (!AcceptableRowArgs(0, 0, __func__, rows, start, end)) {
    return;
  }

  int lmin, lmax, rmin, rmax;
  lmin = lmax = (*rows)[start].lmargin_ + (*rows)[start].lindent_;
  rmin = rmax = (*rows)[start].rmargin_ + (*rows)[start].rindent_;
  for (int i = start; i < end; i++) {
    RowScratchRegisters &sr = (*rows)[i];
    sr.SetUnknown();
    if (sr.ri_->num_words == 0) {
      continue;
    }
    UpdateRange(sr.lmargin_ + sr.lindent_, &lmin, &lmax);
    UpdateRange(sr.rmargin_ + sr.rindent_, &rmin, &rmax);
  }
  STATS lefts(lmin, lmax);
  STATS rights(rmin, rmax);
  for (int i = start; i < end; i++) {
    RowScratchRegisters &sr = (*rows)[i];
    if (sr.ri_->num_words == 0) {
      continue;
    }
    lefts.add(sr.lmargin_ + sr.lindent_, 1);
    rights.add(sr.rmargin_ + sr.rindent_, 1);
  }
  int ignorable_left = lefts.ile(ClipToRange(percentile, 0, 100) / 100.0);
  int ignorable_right = rights.ile(ClipToRange(percentile, 0, 100) / 100.0);
  for (int i = start; i < end; i++) {
    RowScratchRegisters &sr = (*rows)[i];
    int ldelta = ignorable_left - sr.lmargin_;
    sr.lmargin_ += ldelta;
    sr.lindent_ -= ldelta;
    int rdelta = ignorable_right - sr.rmargin_;
    sr.rmargin_ += rdelta;
    sr.rindent_ -= rdelta;
  }
}

// Return the median inter-word space in rows[row_start, row_end).
int InterwordSpace(const std::vector<RowScratchRegisters> &rows, int row_start, int row_end) {
  if (row_end < row_start + 1) {
    return 1;
  }
  int word_height =
      (rows[row_start].ri_->lword_box.height() + rows[row_end - 1].ri_->lword_box.height()) / 2;
  int word_width =
      (rows[row_start].ri_->lword_box.width() + rows[row_end - 1].ri_->lword_box.width()) / 2;
  STATS spacing_widths(0, 4 + word_width);
  for (int i = row_start; i < row_end; i++) {
    if (rows[i].ri_->num_words > 1) {
      spacing_widths.add(rows[i].ri_->average_interword_space, 1);
    }
  }
  int minimum_reasonable_space = word_height / 3;
  if (minimum_reasonable_space < 2) {
    minimum_reasonable_space = 2;
  }
  int median = spacing_widths.median();
  return (median > minimum_reasonable_space) ? median : minimum_reasonable_space;
}

// Return whether the first word on the after line can fit in the space at
// the end of the before line (knowing which way the text is aligned and read).
bool FirstWordWouldHaveFit(const RowScratchRegisters &before, const RowScratchRegisters &after,
                           tesseract::ParagraphJustification justification) {
  if (before.ri_->num_words == 0 || after.ri_->num_words == 0) {
    return true;
  }

  if (justification == JUSTIFICATION_UNKNOWN) {
    tprintf("Don't call FirstWordWouldHaveFit(r, s, JUSTIFICATION_UNKNOWN).\n");
  }
  int available_space;
  if (justification == JUSTIFICATION_CENTER) {
    available_space = before.lindent_ + before.rindent_;
  } else {
    available_space = before.OffsideIndent(justification);
  }
  available_space -= before.ri_->average_interword_space;

  if (before.ri_->ltr) {
    return after.ri_->lword_box.width() < available_space;
  }
  return after.ri_->rword_box.width() < available_space;
}

// Return whether the first word on the after line can fit in the space at
// the end of the before line (not knowing which way the text goes) in a left
// or right alignment.
bool FirstWordWouldHaveFit(const RowScratchRegisters &before, const RowScratchRegisters &after) {
  if (before.ri_->num_words == 0 || after.ri_->num_words == 0) {
    return true;
  }

  int available_space = before.lindent_;
  if (before.rindent_ > available_space) {
    available_space = before.rindent_;
  }
  available_space -= before.ri_->average_interword_space;

  if (before.ri_->ltr) {
    return after.ri_->lword_box.width() < available_space;
  }
  return after.ri_->rword_box.width() < available_space;
}

static bool TextSupportsBreak(const RowScratchRegisters &before, const RowScratchRegisters &after) {
  if (before.ri_->ltr) {
    return before.ri_->rword_likely_ends_idea && after.ri_->lword_likely_starts_idea;
  } else {
    return before.ri_->lword_likely_ends_idea && after.ri_->rword_likely_starts_idea;
  }
}

static bool LikelyParagraphStart(const RowScratchRegisters &before,
                                 const RowScratchRegisters &after,
                                 tesseract::ParagraphJustification j) {
  return before.ri_->num_words == 0 ||
         (FirstWordWouldHaveFit(before, after, j) && TextSupportsBreak(before, after));
}

// Examine rows[start, end) and try to determine what sort of ParagraphModel
// would fit them as a single paragraph.
// If we can't produce a unique model justification_ = JUSTIFICATION_UNKNOWN.
// If the rows given could be a consistent start to a paragraph, set *consistent
// true.
static ParagraphModel InternalParagraphModelByOutline(
    const std::vector<RowScratchRegisters> *rows, int start, int end, int tolerance,
    bool *consistent) {
  int ltr_line_count = 0;
  for (int i = start; i < end; i++) {
    ltr_line_count += static_cast<int>((*rows)[i].ri_->ltr);
  }
  bool ltr = (ltr_line_count >= (end - start) / 2);

  *consistent = true;
  if (!AcceptableRowArgs(0, 2, __func__, rows, start, end)) {
    return ParagraphModel();
  }

  // Ensure the caller only passed us a region with a common rmargin and
  // lmargin.
  int lmargin = (*rows)[start].lmargin_;
  int rmargin = (*rows)[start].rmargin_;
  int lmin, lmax, rmin, rmax, cmin, cmax;
  lmin = lmax = (*rows)[start + 1].lindent_;
  rmin = rmax = (*rows)[start + 1].rindent_;
  cmin = cmax = 0;
  for (int i = start + 1; i < end; i++) {
    if ((*rows)[i].lmargin_ != lmargin || (*rows)[i].rmargin_ != rmargin) {
      tprintf("Margins don't match! Software error.\n");
      *consistent = false;
      return ParagraphModel();
    }
    UpdateRange((*rows)[i].lindent_, &lmin, &lmax);
    UpdateRange((*rows)[i].rindent_, &rmin, &rmax);
    UpdateRange((*rows)[i].rindent_ - (*rows)[i].lindent_, &cmin, &cmax);
  }
  int ldiff = lmax - lmin;
  int rdiff = rmax - rmin;
  int cdiff = cmax - cmin;
  if (rdiff > tolerance && ldiff > tolerance) {
    if (cdiff < tolerance * 2) {
      if (end - start < 3) {
        return ParagraphModel();
      }
      return ParagraphModel(JUSTIFICATION_CENTER, 0, 0, 0, tolerance);
    }
    *consistent = false;
    return ParagraphModel();
  }
  if (end - start < 3) { // Don't return a model for two line paras.
    return ParagraphModel();
  }

  // These booleans keep us from saying something is aligned left when the body
  // left variance is too large.
  bool body_admits_left_alignment = ldiff < tolerance;
  bool body_admits_right_alignment = rdiff < tolerance;

  ParagraphModel left_model = ParagraphModel(JUSTIFICATION_LEFT, lmargin, (*rows)[start].lindent_,
                                             (lmin + lmax) / 2, tolerance);
  ParagraphModel right_model = ParagraphModel(JUSTIFICATION_RIGHT, rmargin, (*rows)[start].rindent_,
                                              (rmin + rmax) / 2, tolerance);

  // These booleans keep us from having an indent on the "wrong side" for the
  // first line.
  bool text_admits_left_alignment = ltr || left_model.is_flush();
  bool text_admits_right_alignment = !ltr || right_model.is_flush();

  // At least one of the edges is less than tolerance in variance.
  // If the other is obviously ragged, it can't be the one aligned to.
  // [Note the last line is included in this raggedness.]
  if (tolerance < rdiff) {
    if (body_admits_left_alignment && text_admits_left_alignment) {
      return left_model;
    }
    *consistent = false;
    return ParagraphModel();
  }
  if (tolerance < ldiff) {
    if (body_admits_right_alignment && text_admits_right_alignment) {
      return right_model;
    }
    *consistent = false;
    return ParagraphModel();
  }

  // At this point, we know the body text doesn't vary much on either side.

  // If the first line juts out oddly in one direction or the other,
  // that likely indicates the side aligned to.
  int first_left = (*rows)[start].lindent_;
  int first_right = (*rows)[start].rindent_;

  if (ltr && body_admits_left_alignment && (first_left < lmin || first_left > lmax)) {
    return left_model;
  }
  if (!ltr && body_admits_right_alignment && (first_right < rmin || first_right > rmax)) {
    return right_model;
  }

  *consistent = false;
  return ParagraphModel();
}

// Examine rows[start, end) and try to determine what sort of ParagraphModel
// would fit them as a single paragraph.   If nothing fits,
// justification_ = JUSTIFICATION_UNKNOWN and print the paragraph to debug
// output if we're debugging.
static ParagraphModel ParagraphModelByOutline(int debug_level,
                                              const std::vector<RowScratchRegisters> *rows,
                                              int start, int end, int tolerance) {
  bool unused_consistent;
  ParagraphModel retval =
      InternalParagraphModelByOutline(rows, start, end, tolerance, &unused_consistent);
  if (debug_level >= 2 && retval.justification() == JUSTIFICATION_UNKNOWN) {
    tprintf("Could not determine a model for this paragraph:\n");
    PrintRowRange(*rows, start, end);
  }
  return retval;
}

// Do rows[start, end) form a single instance of the given paragraph model?
bool RowsFitModel(const std::vector<RowScratchRegisters> *rows, int start, int end,
                  const ParagraphModel *model) {
  if (!AcceptableRowArgs(0, 1, __func__, rows, start, end)) {
    return false;
  }
  if (!ValidFirstLine(rows, start, model)) {
    return false;
  }
  for (int i = start + 1; i < end; i++) {
    if (!ValidBodyLine(rows, i, model)) {
      return false;
    }
  }
  return true;
}

// Examine rows[row_start, row_end) as an independent section of text,
// and mark rows that are exceptionally clear as start-of-paragraph
// and paragraph-body lines.
//
// We presume that any lines surrounding rows[row_start, row_end) may
// have wildly different paragraph models, so we don't key any data off
// of those lines.
//
// We only take the very strongest signals, as we don't want to get
// confused and marking up centered text, poetry, or source code as
// clearly part of a typical paragraph.
static void MarkStrongEvidence(std::vector<RowScratchRegisters> *rows, int row_start,
                               int row_end) {
  // Record patently obvious body text.
  for (int i = row_start + 1; i < row_end; i++) {
    const RowScratchRegisters &prev = (*rows)[i - 1];
    RowScratchRegisters &curr = (*rows)[i];
    tesseract::ParagraphJustification typical_justification =
        prev.ri_->ltr ? JUSTIFICATION_LEFT : JUSTIFICATION_RIGHT;
    if (!curr.ri_->rword_likely_starts_idea && !curr.ri_->lword_likely_starts_idea &&
        !FirstWordWouldHaveFit(prev, curr, typical_justification)) {
      curr.SetBodyLine();
    }
  }

  // Record patently obvious start paragraph lines.
  //
  // It's an extremely good signal of the start of a paragraph that
  // the first word would have fit on the end of the previous line.
  // However, applying just that signal would have us mark random
  // start lines of lineated text (poetry and source code) and some
  // centered headings as paragraph start lines.  Therefore, we use
  // a second qualification for a paragraph start: Not only should
  // the first word of this line have fit on the previous line,
  // but also, this line should go full to the right of the block,
  // disallowing a subsequent word from having fit on this line.

  // First row:
  {
    RowScratchRegisters &curr = (*rows)[row_start];
    RowScratchRegisters &next = (*rows)[row_start + 1];
    tesseract::ParagraphJustification j = curr.ri_->ltr ? JUSTIFICATION_LEFT : JUSTIFICATION_RIGHT;
    if (curr.GetLineType() == LT_UNKNOWN && !FirstWordWouldHaveFit(curr, next, j) &&
        (curr.ri_->lword_likely_starts_idea || curr.ri_->rword_likely_starts_idea)) {
      curr.SetStartLine();
    }
  }
  // Middle rows
  for (int i = row_start + 1; i < row_end - 1; i++) {
    RowScratchRegisters &prev = (*rows)[i - 1];
    RowScratchRegisters &curr = (*rows)[i];
    RowScratchRegisters &next = (*rows)[i + 1];
    tesseract::ParagraphJustification j = curr.ri_->ltr ? JUSTIFICATION_LEFT : JUSTIFICATION_RIGHT;
    if (curr.GetLineType() == LT_UNKNOWN && !FirstWordWouldHaveFit(curr, next, j) &&
        LikelyParagraphStart(prev, curr, j)) {
      curr.SetStartLine();
    }
  }
  // Last row
  { // the short circuit at the top means we have at least two lines.
    RowScratchRegisters &prev = (*rows)[row_end - 2];
    RowScratchRegisters &curr = (*rows)[row_end - 1];
    tesseract::ParagraphJustification j = curr.ri_->ltr ? JUSTIFICATION_LEFT : JUSTIFICATION_RIGHT;
    if (curr.GetLineType() == LT_UNKNOWN && !FirstWordWouldHaveFit(curr, curr, j) &&
        LikelyParagraphStart(prev, curr, j)) {
      curr.SetStartLine();
    }
  }
}

// Look for sequences of a start line followed by some body lines in
// rows[row_start, row_end) and create ParagraphModels for them if
// they seem coherent.
static void ModelStrongEvidence(int debug_level, std::vector<RowScratchRegisters> *rows,
                                int row_start, int row_end, bool allow_flush_models,
                                ParagraphTheory *theory) {
  if (!AcceptableRowArgs(debug_level, 2, __func__, rows, row_start, row_end)) {
    return;
  }

  int start = row_start;
  while (start < row_end) {
    while (start < row_end && (*rows)[start].GetLineType() != LT_START) {
      start++;
    }
    if (start >= row_end - 1) {
      break;
    }

    int tolerance = Epsilon((*rows)[start + 1].ri_->average_interword_space);
    int end = start;
    ParagraphModel last_model;
    bool next_consistent;
    do {
      ++end;
      // rows[row, end) was consistent.
      // If rows[row, end + 1) is not consistent,
      //   just model rows[row, end)
      if (end < row_end - 1) {
        RowScratchRegisters &next = (*rows)[end];
        LineType lt = next.GetLineType();
        next_consistent = lt == LT_BODY || (lt == LT_UNKNOWN &&
                                            !FirstWordWouldHaveFit((*rows)[end - 1], (*rows)[end]));
      } else {
        next_consistent = false;
      }
      if (next_consistent) {
        ParagraphModel next_model =
            InternalParagraphModelByOutline(rows, start, end + 1, tolerance, &next_consistent);
        if (((*rows)[start].ri_->ltr && last_model.justification() == JUSTIFICATION_LEFT &&
             next_model.justification() != JUSTIFICATION_LEFT) ||
            (!(*rows)[start].ri_->ltr && last_model.justification() == JUSTIFICATION_RIGHT &&
             next_model.justification() != JUSTIFICATION_RIGHT)) {
          next_consistent = false;
        }
        last_model = next_model;
      } else {
        next_consistent = false;
      }
    } while (next_consistent && end < row_end);
    // At this point, rows[start, end) looked like it could have been a
    // single paragraph.  If we can make a good ParagraphModel for it,
    // do so and mark this sequence with that model.
    if (end > start + 1) {
      // emit a new paragraph if we have more than one line.
      const ParagraphModel *model = nullptr;
      ParagraphModel new_model = ParagraphModelByOutline(
          debug_level, rows, start, end, Epsilon(InterwordSpace(*rows, start, end)));
      if (new_model.justification() == JUSTIFICATION_UNKNOWN) {
        // couldn't create a good model, oh well.
      } else if (new_model.is_flush()) {
        if (end == start + 2) {
          // It's very likely we just got two paragraph starts in a row.
          end = start + 1;
        } else if (start == row_start) {
          // Mark this as a Crown.
          if (new_model.justification() == JUSTIFICATION_LEFT) {
            model = kCrownLeft;
          } else {
            model = kCrownRight;
          }
        } else if (allow_flush_models) {
          model = theory->AddModel(new_model);
        }
      } else {
        model = theory->AddModel(new_model);
      }
      if (model) {
        (*rows)[start].AddStartLine(model);
        for (int i = start + 1; i < end; i++) {
          (*rows)[i].AddBodyLine(model);
        }
      }
    }
    start = end;
  }
}

// We examine rows[row_start, row_end) and do the following:
//   (1) Clear all existing hypotheses for the rows being considered.
//   (2) Mark up any rows as exceptionally likely to be paragraph starts
//       or paragraph body lines as such using both geometric and textual
//       clues.
//   (3) Form models for any sequence of start + continuation lines.
//   (4) Smear the paragraph models to cover surrounding text.
static void StrongEvidenceClassify(int debug_level, std::vector<RowScratchRegisters> *rows,
                                   int row_start, int row_end, ParagraphTheory *theory) {
  if (!AcceptableRowArgs(debug_level, 2, __func__, rows, row_start, row_end)) {
    return;
  }

  if (debug_level > 1) {
    tprintf("#############################################\n");
    tprintf("# StrongEvidenceClassify( rows[%d:%d) )\n", row_start, row_end);
    tprintf("#############################################\n");
  }

  RecomputeMarginsAndClearHypotheses(rows, row_start, row_end, 10);
  MarkStrongEvidence(rows, row_start, row_end);

  DebugDump(debug_level > 2, "Initial strong signals.", *theory, *rows);

  // Create paragraph models.
  ModelStrongEvidence(debug_level, rows, row_start, row_end, false, theory);

  DebugDump(debug_level > 2, "Unsmeared hypotheses.s.", *theory, *rows);

  // At this point, some rows are marked up as paragraphs with model numbers,
  // and some rows are marked up as either LT_START or LT_BODY.  Now let's
  // smear any good paragraph hypotheses forward and backward.
  ParagraphModelSmearer smearer(rows, row_start, row_end, theory);
  smearer.Smear();
}

static void SeparateSimpleLeaderLines(std::vector<RowScratchRegisters> *rows, int row_start,
                                      int row_end, ParagraphTheory *theory) {
  for (int i = row_start + 1; i < row_end - 1; i++) {
    if ((*rows)[i - 1].ri_->has_leaders && (*rows)[i].ri_->has_leaders &&
        (*rows)[i + 1].ri_->has_leaders) {
      const ParagraphModel *model =
          theory->AddModel(ParagraphModel(JUSTIFICATION_UNKNOWN, 0, 0, 0, 0));
      (*rows)[i].AddStartLine(model);
    }
  }
}

// Collect sequences of unique hypotheses in row registers and create proper
// paragraphs for them, referencing the paragraphs in row_owners.
static void ConvertHypothesizedModelRunsToParagraphs(int debug_level,
                                                     std::vector<RowScratchRegisters> &rows,
                                                     std::vector<PARA *> *row_owners,
                                                     ParagraphTheory *theory) {
  int end = rows.size();
  int start;
  for (; end > 0; end = start) {
    start = end - 1;
    const ParagraphModel *model = nullptr;
    // TODO(eger): Be smarter about dealing with multiple hypotheses.
    bool single_line_paragraph = false;
    SetOfModels models;
    rows[start].NonNullHypotheses(&models);
    if (!models.empty()) {
      model = models[0];
      if (rows[start].GetLineType(model) != LT_BODY) {
        single_line_paragraph = true;
      }
    }
    if (model && !single_line_paragraph) {
      // walk back looking for more body lines and then a start line.
      while (--start > 0 && rows[start].GetLineType(model) == LT_BODY) {
        // do nothing
      }
      if (start < 0 || rows[start].GetLineType(model) != LT_START) {
        model = nullptr;
      }
    }
    if (model == nullptr) {
      continue;
    }
    // rows[start, end) should be a paragraph.
    PARA *p = new PARA();
    if (model == kCrownLeft || model == kCrownRight) {
      p->is_very_first_or_continuation = true;
      // Crown paragraph.
      //   If we can find an existing ParagraphModel that fits, use it,
      //   else create a new one.
      for (unsigned row = end; row < rows.size(); row++) {
        if ((*row_owners)[row] &&
            (ValidBodyLine(&rows, start, (*row_owners)[row]->model) &&
             (start == 0 || ValidFirstLine(&rows, start, (*row_owners)[row]->model)))) {
          model = (*row_owners)[row]->model;
          break;
        }
      }
      if (model == kCrownLeft) {
        // No subsequent model fits, so cons one up.
        model = theory->AddModel(ParagraphModel(JUSTIFICATION_LEFT,
                                                rows[start].lmargin_ + rows[start].lindent_, 0, 0,
                                                Epsilon(rows[start].ri_->average_interword_space)));
      } else if (model == kCrownRight) {
        // No subsequent model fits, so cons one up.
        model = theory->AddModel(ParagraphModel(JUSTIFICATION_RIGHT,
                                                rows[start].rmargin_ + rows[start].rmargin_, 0, 0,
                                                Epsilon(rows[start].ri_->average_interword_space)));
      }
    }
    rows[start].SetUnknown();
    rows[start].AddStartLine(model);
    for (int i = start + 1; i < end; i++) {
      rows[i].SetUnknown();
      rows[i].AddBodyLine(model);
    }
    p->model = model;
    p->has_drop_cap = rows[start].ri_->has_drop_cap;
    p->is_list_item = model->justification() == JUSTIFICATION_RIGHT
                          ? rows[start].ri_->rword_indicates_list_item
                          : rows[start].ri_->lword_indicates_list_item;
    for (int row = start; row < end; row++) {
      if ((*row_owners)[row] != nullptr) {
        tprintf(
            "Memory leak! ConvertHypothesizeModelRunsToParagraphs() called "
            "more than once!\n");
        delete (*row_owners)[row];
      }
      (*row_owners)[row] = p;
    }
  }
}

struct Interval {
  Interval() : begin(0), end(0) {}
  Interval(int b, int e) : begin(b), end(e) {}

  int begin;
  int end;
};

// Return whether rows[row] appears to be stranded, meaning that the evidence
// for this row is very weak due to context.  For instance, two lines of source
// code may happen to be indented at the same tab vector as body text starts,
// leading us to think they are two start-of-paragraph lines.  This is not
// optimal.  However, we also don't want to mark a sequence of short dialog
// as "weak," so our heuristic is:
//   (1) If a line is surrounded by lines of unknown type, it's weak.
//   (2) If two lines in a row are start lines for a given paragraph type, but
//       after that the same paragraph type does not continue, they're weak.
static bool RowIsStranded(const std::vector<RowScratchRegisters> &rows, int row) {
  SetOfModels row_models;
  rows[row].StrongHypotheses(&row_models);

  for (auto &row_model : row_models) {
    bool all_starts = rows[row].GetLineType();
    int run_length = 1;
    bool continues = true;
    for (int i = row - 1; i >= 0 && continues; i--) {
      SetOfModels models;
      rows[i].NonNullHypotheses(&models);
      switch (rows[i].GetLineType(row_model)) {
        case LT_START:
          run_length++;
          break;
        case LT_MULTIPLE: // explicit fall-through
        case LT_BODY:
          run_length++;
          all_starts = false;
          break;
        case LT_UNKNOWN: // explicit fall-through
        default:
          continues = false;
      }
    }
    continues = true;
    for (unsigned i = row + 1; i < rows.size() && continues; i++) {
      SetOfModels models;
      rows[i].NonNullHypotheses(&models);
      switch (rows[i].GetLineType(row_model)) {
        case LT_START:
          run_length++;
          break;
        case LT_MULTIPLE: // explicit fall-through
        case LT_BODY:
          run_length++;
          all_starts = false;
          break;
        case LT_UNKNOWN: // explicit fall-through
        default:
          continues = false;
      }
    }
    if (run_length > 2 || (!all_starts && run_length > 1)) {
      return false;
    }
  }
  return true;
}

// Go through rows[row_start, row_end) and gather up sequences that need better
// classification.
// + Sequences of non-empty rows without hypotheses.
// + Crown paragraphs not immediately followed by a strongly modeled line.
// + Single line paragraphs surrounded by text that doesn't match the
//   model.
static void LeftoverSegments(const std::vector<RowScratchRegisters> &rows,
                             std::vector<Interval> *to_fix, int row_start, int row_end) {
  to_fix->clear();
  for (int i = row_start; i < row_end; i++) {
    bool needs_fixing = false;

    SetOfModels models;
    SetOfModels models_w_crowns;
    rows[i].StrongHypotheses(&models);
    rows[i].NonNullHypotheses(&models_w_crowns);
    if (models.empty() && !models_w_crowns.empty()) {
      // Crown paragraph.  Is it followed by a modeled line?
      for (unsigned end = i + 1; end < rows.size(); end++) {
        SetOfModels end_models;
        SetOfModels strong_end_models;
        rows[end].NonNullHypotheses(&end_models);
        rows[end].StrongHypotheses(&strong_end_models);
        if (end_models.empty()) {
          needs_fixing = true;
          break;
        } else if (!strong_end_models.empty()) {
          needs_fixing = false;
          break;
        }
      }
    } else if (models.empty() && rows[i].ri_->num_words > 0) {
      // No models at all.
      needs_fixing = true;
    }

    if (!needs_fixing && !models.empty()) {
      needs_fixing = RowIsStranded(rows, i);
    }

    if (needs_fixing) {
      if (!to_fix->empty() && to_fix->back().end == i - 1) {
        to_fix->back().end = i;
      } else {
        to_fix->push_back(Interval(i, i));
      }
    }
  }
  // Convert inclusive intervals to half-open intervals.
  for (auto &i : *to_fix) {
    i.end = i.end + 1;
  }
}

// Given a set of row_owners pointing to PARAs or nullptr (no paragraph known),
// normalize each row_owner to point to an actual PARA, and output the
// paragraphs in order onto paragraphs.
void CanonicalizeDetectionResults(std::vector<PARA *> *row_owners, PARA_LIST *paragraphs) {
  std::vector<PARA *> &rows = *row_owners;
  paragraphs->clear();
  PARA_IT out(paragraphs);
  PARA *formerly_null = nullptr;
  for (unsigned i = 0; i < rows.size(); i++) {
    if (rows[i] == nullptr) {
      if (i == 0 || rows[i - 1] != formerly_null) {
        rows[i] = formerly_null = new PARA();
      } else {
        rows[i] = formerly_null;
        continue;
      }
    } else if (i > 0 && rows[i - 1] == rows[i]) {
      continue;
    }
    out.add_after_then_move(rows[i]);
  }
}

// Main entry point for Paragraph Detection Algorithm.
//
// Given a set of equally spaced textlines (described by row_infos),
// Split them into paragraphs.
//
// Output:
//   row_owners - one pointer for each row, to the paragraph it belongs to.
//   paragraphs - this is the actual list of PARA objects.
//   models - the list of paragraph models referenced by the PARA objects.
//            caller is responsible for deleting the models.
void DetectParagraphs(int debug_level, std::vector<RowInfo> *row_infos,
                      std::vector<PARA *> *row_owners, PARA_LIST *paragraphs,
                      std::vector<ParagraphModel *> *models) {
  ParagraphTheory theory(models);

  // Initialize row_owners to be a bunch of nullptr pointers.
  row_owners->clear();
  row_owners->resize(row_infos->size());

  // Set up row scratch registers for the main algorithm.
  std::vector<RowScratchRegisters> rows(row_infos->size());
  for (unsigned i = 0; i < row_infos->size(); i++) {
    rows[i].Init((*row_infos)[i]);
  }

  // Pass 1:
  //   Detect sequences of lines that all contain leader dots (.....)
  //   These are likely Tables of Contents.  If there are three text lines in
  //   a row with leader dots, it's pretty safe to say the middle one should
  //   be a paragraph of its own.
  SeparateSimpleLeaderLines(&rows, 0, rows.size(), &theory);

  DebugDump(debug_level > 1, "End of Pass 1", theory, rows);

  std::vector<Interval> leftovers;
  LeftoverSegments(rows, &leftovers, 0, rows.size());
  for (auto &leftover : leftovers) {
    // Pass 2a:
    //   Find any strongly evidenced start-of-paragraph lines.  If they're
    //   followed by two lines that look like body lines, make a paragraph
    //   model for that and see if that model applies throughout the text
    //   (that is, "smear" it).
    StrongEvidenceClassify(debug_level, &rows, leftover.begin, leftover.end, &theory);

    // Pass 2b:
    //   If we had any luck in pass 2a, we got part of the page and didn't
    //   know how to classify a few runs of rows. Take the segments that
    //   didn't find a model and reprocess them individually.
    std::vector<Interval> leftovers2;
    LeftoverSegments(rows, &leftovers2, leftover.begin, leftover.end);
    bool pass2a_was_useful =
        leftovers2.size() > 1 ||
        (leftovers2.size() == 1 && (leftovers2[0].begin != 0 || static_cast<size_t>(leftovers2[0].end) != rows.size()));
    if (pass2a_was_useful) {
      for (auto &leftover2 : leftovers2) {
        StrongEvidenceClassify(debug_level, &rows, leftover2.begin, leftover2.end, &theory);
      }
    }
  }

  DebugDump(debug_level > 1, "End of Pass 2", theory, rows);

  // Pass 3:
  //   These are the dregs for which we didn't have enough strong textual
  //   and geometric clues to form matching models for.  Let's see if
  //   the geometric clues are simple enough that we could just use those.
  LeftoverSegments(rows, &leftovers, 0, rows.size());
  for (auto &leftover : leftovers) {
    GeometricClassify(debug_level, &rows, leftover.begin, leftover.end, &theory);
  }

  // Undo any flush models for which there's little evidence.
  DowngradeWeakestToCrowns(debug_level, &theory, &rows);

  DebugDump(debug_level > 1, "End of Pass 3", theory, rows);

  // Pass 4:
  //   Take everything that's still not marked up well and clear all markings.
  LeftoverSegments(rows, &leftovers, 0, rows.size());
  for (auto &leftover : leftovers) {
    for (int j = leftover.begin; j < leftover.end; j++) {
      rows[j].SetUnknown();
    }
  }

  DebugDump(debug_level > 1, "End of Pass 4", theory, rows);

  // Convert all of the unique hypothesis runs to PARAs.
  ConvertHypothesizedModelRunsToParagraphs(debug_level, rows, row_owners, &theory);

  DebugDump(debug_level > 0, "Final Paragraph Segmentation", theory, rows);

  // Finally, clean up any dangling nullptr row paragraph parents.
  CanonicalizeDetectionResults(row_owners, paragraphs);
}

// ============ Code interfacing with the rest of Tesseract ==================

static void InitializeTextAndBoxesPreRecognition(const MutableIterator &it, RowInfo *info) {
  // Set up text, lword_text, and rword_text (mostly for debug printing).
  std::string fake_text;
  PageIterator pit(static_cast<const PageIterator &>(it));
  if (!pit.Empty(RIL_WORD)) {
    bool first_word = true;
    do {
      fake_text += "x";
      if (first_word) {
        info->lword_text += "x";
      }
      info->rword_text += "x";
      if (pit.IsAtFinalElement(RIL_WORD, RIL_SYMBOL) &&
          !pit.IsAtFinalElement(RIL_TEXTLINE, RIL_SYMBOL)) {
        fake_text += " ";
        info->rword_text = "";
        first_word = false;
      }
    } while (!pit.IsAtFinalElement(RIL_TEXTLINE, RIL_SYMBOL) && pit.Next(RIL_SYMBOL));
  }
  if (fake_text.empty()) {
    return;
  }

  int lspaces = info->pix_ldistance / info->average_interword_space;
  for (int i = 0; i < lspaces; i++) {
    info->text += ' ';
  }
  info->text += fake_text;

  // Set up lword_box, rword_box, and num_words.
  PAGE_RES_IT page_res_it = *it.PageResIt();
  WERD_RES *word_res = page_res_it.restart_row();
  ROW_RES *this_row = page_res_it.row();

  WERD_RES *lword = nullptr;
  WERD_RES *rword = nullptr;
  info->num_words = 0;
  do {
    if (word_res) {
      if (!lword) {
        lword = word_res;
      }
      if (rword != word_res) {
        info->num_words++;
      }
      rword = word_res;
    }
    word_res = page_res_it.forward();
  } while (page_res_it.row() == this_row);

  if (lword) {
    info->lword_box = lword->word->bounding_box();
  }
  if (rword) {
    info->rword_box = rword->word->bounding_box();
  }
}

// Given a Tesseract Iterator pointing to a text line, fill in the paragraph
// detector RowInfo with all relevant information from the row.
static void InitializeRowInfo(bool after_recognition, const MutableIterator &it, RowInfo *info) {
  if (it.PageResIt()->row() != nullptr) {
    ROW *row = it.PageResIt()->row()->row;
    info->pix_ldistance = row->lmargin();
    info->pix_rdistance = row->rmargin();
    info->average_interword_space =
        row->space() > 0 ? row->space() : std::max(static_cast<int>(row->x_height()), 1);
    info->pix_xheight = row->x_height();
    info->has_leaders = false;
    info->has_drop_cap = row->has_drop_cap();
    info->ltr = true; // set below depending on word scripts
  } else {
    info->pix_ldistance = info->pix_rdistance = 0;
    info->average_interword_space = 1;
    info->pix_xheight = 1.0;
    info->has_leaders = false;
    info->has_drop_cap = false;
    info->ltr = true;
  }

  info->num_words = 0;
  info->lword_indicates_list_item = false;
  info->lword_likely_starts_idea = false;
  info->lword_likely_ends_idea = false;
  info->rword_indicates_list_item = false;
  info->rword_likely_starts_idea = false;
  info->rword_likely_ends_idea = false;
  info->has_leaders = false;
  info->ltr = true;

  if (!after_recognition) {
    InitializeTextAndBoxesPreRecognition(it, info);
    return;
  }
  info->text = "";
  const std::unique_ptr<const char[]> text(it.GetUTF8Text(RIL_TEXTLINE));
  int trailing_ws_idx = strlen(text.get()); // strip trailing space
  while (trailing_ws_idx > 0 &&
         // isspace() only takes ASCII
         isascii(text[trailing_ws_idx - 1]) && isspace(text[trailing_ws_idx - 1])) {
    trailing_ws_idx--;
  }
  if (trailing_ws_idx > 0) {
    int lspaces = info->pix_ldistance / info->average_interword_space;
    for (int i = 0; i < lspaces; i++) {
      info->text += ' ';
    }
    for (int i = 0; i < trailing_ws_idx; i++) {
      info->text += text[i];
    }
  }

  if (info->text.empty()) {
    return;
  }

  PAGE_RES_IT page_res_it = *it.PageResIt();
  std::vector<WERD_RES *> werds;
  WERD_RES *word_res = page_res_it.restart_row();
  ROW_RES *this_row = page_res_it.row();
  int num_leaders = 0;
  int ltr = 0;
  int rtl = 0;
  do {
    if (word_res && word_res->best_choice->unichar_string().length() > 0) {
      werds.push_back(word_res);
      ltr += word_res->AnyLtrCharsInWord() ? 1 : 0;
      rtl += word_res->AnyRtlCharsInWord() ? 1 : 0;
      if (word_res->word->flag(W_REP_CHAR)) {
        num_leaders++;
      }
    }
    word_res = page_res_it.forward();
  } while (page_res_it.row() == this_row);
  info->ltr = ltr >= rtl;
  info->has_leaders = num_leaders > 3;
  info->num_words = werds.size();
  if (!werds.empty()) {
    WERD_RES *lword = werds[0], *rword = werds[werds.size() - 1];
    info->lword_text = lword->best_choice->unichar_string().c_str();
    info->rword_text = rword->best_choice->unichar_string().c_str();
    info->lword_box = lword->word->bounding_box();
    info->rword_box = rword->word->bounding_box();
    LeftWordAttributes(lword->uch_set, lword->best_choice, info->lword_text,
                       &info->lword_indicates_list_item, &info->lword_likely_starts_idea,
                       &info->lword_likely_ends_idea);
    RightWordAttributes(rword->uch_set, rword->best_choice, info->rword_text,
                        &info->rword_indicates_list_item, &info->rword_likely_starts_idea,
                        &info->rword_likely_ends_idea);
  }
}

// This is called after rows have been identified and words are recognized.
// Much of this could be implemented before word recognition, but text helps
// to identify bulleted lists and gives good signals for sentence boundaries.
void DetectParagraphs(int debug_level, bool after_text_recognition,
                      const MutableIterator *block_start, std::vector<ParagraphModel *> *models) {
  // Clear out any preconceived notions.
  if (block_start->Empty(RIL_TEXTLINE)) {
    return;
  }
  BLOCK *block = block_start->PageResIt()->block()->block;
  block->para_list()->clear();
  bool is_image_block = block->pdblk.poly_block() && !block->pdblk.poly_block()->IsText();

  // Convert the Tesseract structures to RowInfos
  // for the paragraph detection algorithm.
  MutableIterator row(*block_start);
  if (row.Empty(RIL_TEXTLINE)) {
    return; // end of input already.
  }

  std::vector<RowInfo> row_infos;
  do {
    if (!row.PageResIt()->row()) {
      continue; // empty row.
    }
    row.PageResIt()->row()->row->set_para(nullptr);
    row_infos.emplace_back();
    RowInfo &ri = row_infos.back();
    InitializeRowInfo(after_text_recognition, row, &ri);
  } while (!row.IsAtFinalElement(RIL_BLOCK, RIL_TEXTLINE) && row.Next(RIL_TEXTLINE));

  // If we're called before text recognition, we might not have
  // tight block bounding boxes, so trim by the minimum on each side.
  if (!row_infos.empty()) {
    int min_lmargin = row_infos[0].pix_ldistance;
    int min_rmargin = row_infos[0].pix_rdistance;
    for (unsigned i = 1; i < row_infos.size(); i++) {
      if (row_infos[i].pix_ldistance < min_lmargin) {
        min_lmargin = row_infos[i].pix_ldistance;
      }
      if (row_infos[i].pix_rdistance < min_rmargin) {
        min_rmargin = row_infos[i].pix_rdistance;
      }
    }
    if (min_lmargin > 0 || min_rmargin > 0) {
      for (auto &row_info : row_infos) {
        row_info.pix_ldistance -= min_lmargin;
        row_info.pix_rdistance -= min_rmargin;
      }
    }
  }

  // Run the paragraph detection algorithm.
  std::vector<PARA *> row_owners;
  if (!is_image_block) {
    DetectParagraphs(debug_level, &row_infos, &row_owners, block->para_list(), models);
  } else {
    row_owners.resize(row_infos.size());
    CanonicalizeDetectionResults(&row_owners, block->para_list());
  }

  // Now stitch in the row_owners into the rows.
  row = *block_start;
  for (auto &row_owner : row_owners) {
    while (!row.PageResIt()->row()) {
      row.Next(RIL_TEXTLINE);
    }
    row.PageResIt()->row()->row->set_para(row_owner);
    row.Next(RIL_TEXTLINE);
  }
}

} // namespace tesseract
