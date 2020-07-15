/**********************************************************************
 * File: ratngs.cpp  (Formerly ratings.c)
 * Description: Code to manipulate the BLOB_CHOICE and WERD_CHOICE classes.
 * Author: Ray Smith
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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


#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "ratngs.h"

#include <algorithm>
#include <string>
#include "blobs.h"
#include <tesseract/genericvector.h>
#include "matrix.h"
#include "normalis.h"  // kBlnBaselineOffset.
#include "unicharset.h"

using tesseract::ScriptPos;

ELISTIZE(BLOB_CHOICE)
ELISTIZE(WERD_CHOICE)

const float WERD_CHOICE::kBadRating = 100000.0;
// Min offset in baseline-normalized coords to make a character a subscript.
const int kMinSubscriptOffset = 20;
// Min offset in baseline-normalized coords to make a character a superscript.
const int kMinSuperscriptOffset = 20;
// Max y of bottom of a drop-cap blob.
const int kMaxDropCapBottom = -128;
// Max fraction of x-height to use as denominator in measuring x-height overlap.
const double kMaxOverlapDenominator = 0.125;
// Min fraction of x-height range that should be in agreement for matching
// x-heights.
const double kMinXHeightMatch = 0.5;
// Max tolerance on baseline position as a fraction of x-height for matching
// baselines.
const double kMaxBaselineDrift = 0.0625;

static const char kPermuterTypeNoPerm[] = "None";
static const char kPermuterTypePuncPerm[] = "Punctuation";
static const char kPermuterTypeTopPerm[] = "Top Choice";
static const char kPermuterTypeLowerPerm[] = "Top Lower Case";
static const char kPermuterTypeUpperPerm[] = "Top Upper Case";
static const char kPermuterTypeNgramPerm[] = "Ngram";
static const char kPermuterTypeNumberPerm[] = "Number";
static const char kPermuterTypeUserPatPerm[] = "User Pattern";
static const char kPermuterTypeSysDawgPerm[] = "System Dictionary";
static const char kPermuterTypeDocDawgPerm[] = "Document Dictionary";
static const char kPermuterTypeUserDawgPerm[] = "User Dictionary";
static const char kPermuterTypeFreqDawgPerm[] = "Frequent Words Dictionary";
static const char kPermuterTypeCompoundPerm[] = "Compound";

static const char * const kPermuterTypeNames[] = {
    kPermuterTypeNoPerm,        // 0
    kPermuterTypePuncPerm,      // 1
    kPermuterTypeTopPerm,       // 2
    kPermuterTypeLowerPerm,     // 3
    kPermuterTypeUpperPerm,     // 4
    kPermuterTypeNgramPerm,     // 5
    kPermuterTypeNumberPerm,    // 6
    kPermuterTypeUserPatPerm,   // 7
    kPermuterTypeSysDawgPerm,   // 8
    kPermuterTypeDocDawgPerm,   // 9
    kPermuterTypeUserDawgPerm,  // 10
    kPermuterTypeFreqDawgPerm,  // 11
    kPermuterTypeCompoundPerm   // 12
};

/**
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from a char, rating and certainty.
 */
BLOB_CHOICE::BLOB_CHOICE(UNICHAR_ID src_unichar_id, // character id
                         float src_rating,         // rating
                         float src_cert,           // certainty
                         int src_script_id,        // script
                         float min_xheight,        // min xheight allowed
                         float max_xheight,        // max xheight by this char
                         float yshift,             // yshift out of position
                         BlobChoiceClassifier c) {  // adapted match or other
  unichar_id_ = src_unichar_id;
  rating_ = src_rating;
  certainty_ = src_cert;
  fontinfo_id_ = -1;
  fontinfo_id2_ = -1;
  script_id_ = src_script_id;
  min_xheight_ = min_xheight;
  max_xheight_ = max_xheight;
  yshift_ = yshift;
  classifier_ = c;
}

/**
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from another BLOB_CHOICE.
 */
BLOB_CHOICE::BLOB_CHOICE(const BLOB_CHOICE &other) : ELIST_LINK(other) {
  unichar_id_ = other.unichar_id();
  rating_ = other.rating();
  certainty_ = other.certainty();
  fontinfo_id_ = other.fontinfo_id();
  fontinfo_id2_ = other.fontinfo_id2();
  script_id_ = other.script_id();
  matrix_cell_ = other.matrix_cell_;
  min_xheight_ = other.min_xheight_;
  max_xheight_ = other.max_xheight_;
  yshift_ = other.yshift();
  classifier_ = other.classifier_;
#ifndef DISABLED_LEGACY_ENGINE
  fonts_ = other.fonts_;
#endif  // ndef DISABLED_LEGACY_ENGINE
}

// Copy assignment operator.
BLOB_CHOICE& BLOB_CHOICE::operator=(const BLOB_CHOICE& other) {
  ELIST_LINK::operator=(other);
  unichar_id_ = other.unichar_id();
  rating_ = other.rating();
  certainty_ = other.certainty();
  fontinfo_id_ = other.fontinfo_id();
  fontinfo_id2_ = other.fontinfo_id2();
  script_id_ = other.script_id();
  matrix_cell_ = other.matrix_cell_;
  min_xheight_ = other.min_xheight_;
  max_xheight_ = other.max_xheight_;
  yshift_ = other.yshift();
  classifier_ = other.classifier_;
#ifndef DISABLED_LEGACY_ENGINE
  fonts_ = other.fonts_;
#endif  // ndef DISABLED_LEGACY_ENGINE
  return *this;
}

// Returns true if *this and other agree on the baseline and x-height
// to within some tolerance based on a given estimate of the x-height.
bool BLOB_CHOICE::PosAndSizeAgree(const BLOB_CHOICE& other, float x_height,
                                  bool debug) const {
  double baseline_diff = fabs(yshift() - other.yshift());
  if (baseline_diff > kMaxBaselineDrift * x_height) {
    if (debug) {
      tprintf("Baseline diff %g for %d v %d\n",
              baseline_diff, unichar_id_, other.unichar_id_);
    }
    return false;
  }
  double this_range = max_xheight() - min_xheight();
  double other_range = other.max_xheight() - other.min_xheight();
  double denominator = ClipToRange(std::min(this_range, other_range),
                                   1.0, kMaxOverlapDenominator * x_height);
  double overlap = std::min(max_xheight(), other.max_xheight()) -
          std::max(min_xheight(), other.min_xheight());
  overlap /= denominator;
  if (debug) {
    tprintf("PosAndSize for %d v %d: bl diff = %g, ranges %g, %g / %g ->%g\n",
            unichar_id_, other.unichar_id_, baseline_diff,
            this_range, other_range, denominator, overlap);
  }

  return overlap >= kMinXHeightMatch;
}

// Helper to find the BLOB_CHOICE in the bc_list that matches the given
// unichar_id, or nullptr if there is no match.
BLOB_CHOICE* FindMatchingChoice(UNICHAR_ID char_id,
                                BLOB_CHOICE_LIST* bc_list) {
  // Find the corresponding best BLOB_CHOICE.
  BLOB_CHOICE_IT choice_it(bc_list);
  for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
       choice_it.forward()) {
    BLOB_CHOICE* choice = choice_it.data();
    if (choice->unichar_id() == char_id) {
      return choice;
    }
  }
  return nullptr;
}

const char *WERD_CHOICE::permuter_name(uint8_t permuter) {
  return kPermuterTypeNames[permuter];
}

namespace tesseract {

const char *ScriptPosToString(enum ScriptPos script_pos) {
  switch (script_pos) {
    case SP_NORMAL: return "NORM";
    case SP_SUBSCRIPT: return "SUB";
    case SP_SUPERSCRIPT: return "SUPER";
    case SP_DROPCAP: return "DROPC";
  }
  return "SP_UNKNOWN";
}

}  // namespace tesseract.

/**
 * WERD_CHOICE::WERD_CHOICE
 *
 * Constructor to build a WERD_CHOICE from the given string.
 * The function assumes that src_string is not nullptr.
 */
WERD_CHOICE::WERD_CHOICE(const char *src_string,
                         const UNICHARSET &unicharset)
    : unicharset_(&unicharset){
  GenericVector<UNICHAR_ID> encoding;
  GenericVector<char> lengths;
  std::string cleaned = unicharset.CleanupString(src_string);
  if (unicharset.encode_string(cleaned.c_str(), true, &encoding, &lengths,
                               nullptr)) {
    lengths.push_back('\0');
    STRING src_lengths = &lengths[0];
    this->init(cleaned.c_str(), src_lengths.c_str(), 0.0, 0.0, NO_PERM);
  } else {  // There must have been an invalid unichar in the string.
    this->init(8);
    this->make_bad();
  }
}

/**
 * WERD_CHOICE::init
 *
 * Helper function to build a WERD_CHOICE from the given string,
 * fragment lengths, rating, certainty and permuter.
 *
 * The function assumes that src_string is not nullptr.
 * src_lengths argument could be nullptr, in which case the unichars
 * in src_string are assumed to all be of length 1.
 */
void WERD_CHOICE::init(const char *src_string,
                       const char *src_lengths,
                       float src_rating,
                       float src_certainty,
                       uint8_t src_permuter) {
  int src_string_len = strlen(src_string);
  if (src_string_len == 0) {
    this->init(8);
  } else {
    this->init(src_lengths ? strlen(src_lengths): src_string_len);
    length_ = reserved_;
    int offset = 0;
    for (int i = 0; i < length_; ++i) {
      int unichar_length = src_lengths ? src_lengths[i] : 1;
      unichar_ids_[i] =
          unicharset_->unichar_to_id(src_string+offset, unichar_length);
      state_[i] = 1;
      certainties_[i] = src_certainty;
      offset += unichar_length;
    }
  }
  adjust_factor_ = 1.0f;
  rating_ = src_rating;
  certainty_ = src_certainty;
  permuter_ = src_permuter;
  dangerous_ambig_found_ = false;
}

/**
 * WERD_CHOICE::~WERD_CHOICE
 */
WERD_CHOICE::~WERD_CHOICE() {
  delete[] unichar_ids_;
  delete[] script_pos_;
  delete[] state_;
  delete[] certainties_;
}

const char *WERD_CHOICE::permuter_name() const {
  return kPermuterTypeNames[permuter_];
}

// Returns the BLOB_CHOICE_LIST corresponding to the given index in the word,
// taken from the appropriate cell in the ratings MATRIX.
// Borrowed pointer, so do not delete.
BLOB_CHOICE_LIST* WERD_CHOICE::blob_choices(int index, MATRIX* ratings) const {
  MATRIX_COORD coord = MatrixCoord(index);
  BLOB_CHOICE_LIST* result = ratings->get(coord.col, coord.row);
  if (result == nullptr) {
    result = new BLOB_CHOICE_LIST;
    ratings->put(coord.col, coord.row, result);
  }
  return result;
}

// Returns the MATRIX_COORD corresponding to the location in the ratings
// MATRIX for the given index into the word.
MATRIX_COORD WERD_CHOICE::MatrixCoord(int index) const {
  int col = 0;
  for (int i = 0; i < index; ++i)
    col += state_[i];
  int row = col + state_[index] - 1;
  return MATRIX_COORD(col, row);
}

// Sets the entries for the given index from the BLOB_CHOICE, assuming
// unit fragment lengths, but setting the state for this index to blob_count.
void WERD_CHOICE::set_blob_choice(int index, int blob_count,
                                  const BLOB_CHOICE* blob_choice) {
  unichar_ids_[index] = blob_choice->unichar_id();
  script_pos_[index] = tesseract::SP_NORMAL;
  state_[index] = blob_count;
  certainties_[index] = blob_choice->certainty();
}


/**
 * contains_unichar_id
 *
 * Returns true if unichar_ids_ contain the given unichar_id, false otherwise.
 */
bool WERD_CHOICE::contains_unichar_id(UNICHAR_ID unichar_id) const {
  for (int i = 0; i < length_; ++i) {
    if (unichar_ids_[i] == unichar_id) {
      return true;
    }
  }
  return false;
}

/**
 * remove_unichar_ids
 *
 * Removes num unichar ids starting from index start from unichar_ids_
 * and updates length_ and fragment_lengths_ to reflect this change.
 * Note: this function does not modify rating_ and certainty_.
 */
void WERD_CHOICE::remove_unichar_ids(int start, int num) {
  ASSERT_HOST(start >= 0 && start + num <= length_);
  // Accumulate the states to account for the merged blobs.
  for (int i = 0; i < num; ++i) {
    if (start > 0)
      state_[start - 1] += state_[start + i];
    else if (start + num < length_)
      state_[start + num] += state_[start + i];
  }
  for (int i = start; i + num < length_; ++i) {
    unichar_ids_[i] = unichar_ids_[i + num];
    script_pos_[i] = script_pos_[i + num];
    state_[i] = state_[i + num];
    certainties_[i] = certainties_[i + num];
  }
  length_ -= num;
}

/**
 * reverse_and_mirror_unichar_ids
 *
 * Reverses and mirrors unichars in unichar_ids.
 */
void WERD_CHOICE::reverse_and_mirror_unichar_ids() {
  for (int i = 0; i < length_ / 2; ++i) {
    UNICHAR_ID tmp_id = unichar_ids_[i];
    unichar_ids_[i] = unicharset_->get_mirror(unichar_ids_[length_-1-i]);
    unichar_ids_[length_-1-i] = unicharset_->get_mirror(tmp_id);
  }
  if (length_ % 2 != 0) {
    unichar_ids_[length_/2] = unicharset_->get_mirror(unichar_ids_[length_/2]);
  }
}

/**
 * punct_stripped
 *
 * Returns the half-open interval of unichar_id indices [start, end) which
 * enclose the core portion of this word -- the part after stripping
 * punctuation from the left and right.
 */
void WERD_CHOICE::punct_stripped(int *start, int *end) const {
  *start = 0;
  *end = length() - 1;
  while (*start < length() &&
         unicharset()->get_ispunctuation(unichar_id(*start))) {
    (*start)++;
  }
  while (*end > -1 &&
         unicharset()->get_ispunctuation(unichar_id(*end))) {
    (*end)--;
  }
  (*end)++;
}

void WERD_CHOICE::GetNonSuperscriptSpan(int *pstart, int *pend) const {
  int end = length();
  while (end > 0 &&
         unicharset_->get_isdigit(unichar_ids_[end - 1]) &&
         BlobPosition(end - 1) == tesseract::SP_SUPERSCRIPT) {
    end--;
  }
  int start = 0;
  while (start < end &&
         unicharset_->get_isdigit(unichar_ids_[start]) &&
         BlobPosition(start) == tesseract::SP_SUPERSCRIPT) {
    start++;
  }
  *pstart = start;
  *pend = end;
}

WERD_CHOICE WERD_CHOICE::shallow_copy(int start, int end) const {
  ASSERT_HOST(start >= 0 && start <= length_);
  ASSERT_HOST(end >= 0 && end <= length_);
  if (end < start) { end = start; }
  WERD_CHOICE retval(unicharset_, end - start);
  for (int i = start; i < end; i++) {
    retval.append_unichar_id_space_allocated(
        unichar_ids_[i], state_[i], 0.0f, certainties_[i]);
  }
  return retval;
}

/**
 * has_rtl_unichar_id
 *
 * Returns true if unichar_ids contain at least one "strongly" RTL unichar.
 */
bool WERD_CHOICE::has_rtl_unichar_id() const {
  int i;
  for (i = 0; i < length_; ++i) {
    UNICHARSET::Direction dir = unicharset_->get_direction(unichar_ids_[i]);
    if (dir == UNICHARSET::U_RIGHT_TO_LEFT ||
        dir == UNICHARSET::U_RIGHT_TO_LEFT_ARABIC) {
      return true;
    }
  }
  return false;
}

/**
 * string_and_lengths
 *
 * Populates the given word_str with unichars from unichar_ids and
 * and word_lengths_str with the corresponding unichar lengths.
 */
void WERD_CHOICE::string_and_lengths(STRING *word_str,
                                     STRING *word_lengths_str) const {
  *word_str = "";
  if (word_lengths_str != nullptr) *word_lengths_str = "";
  for (int i = 0; i < length_; ++i) {
    const char *ch = unicharset_->id_to_unichar_ext(unichar_ids_[i]);
    *word_str += ch;
    if (word_lengths_str != nullptr) {
      *word_lengths_str += (char)strlen(ch);
    }
  }
}

/**
 * append_unichar_id
 *
 * Make sure there is enough space in the word for the new unichar id
 * and call append_unichar_id_space_allocated().
 */
void WERD_CHOICE::append_unichar_id(
    UNICHAR_ID unichar_id, int blob_count,
    float rating, float certainty) {
  if (length_ == reserved_) {
    this->double_the_size();
  }
  this->append_unichar_id_space_allocated(unichar_id, blob_count,
                                          rating, certainty);
}

/**
 * WERD_CHOICE::operator+=
 *
 * Cat a second word rating on the end of this current one.
 * The ratings are added and the confidence is the min.
 * If the permuters are NOT the same the permuter is set to COMPOUND_PERM
 */
WERD_CHOICE & WERD_CHOICE::operator+= (const WERD_CHOICE & second) {
  ASSERT_HOST(unicharset_ == second.unicharset_);
  while (reserved_ < length_ + second.length()) {
    this->double_the_size();
  }
  const UNICHAR_ID *other_unichar_ids = second.unichar_ids();
  for (int i = 0; i < second.length(); ++i) {
    unichar_ids_[length_ + i] = other_unichar_ids[i];
    state_[length_ + i] = second.state_[i];
    certainties_[length_ + i] = second.certainties_[i];
    script_pos_[length_ + i] = second.BlobPosition(i);
  }
  length_ += second.length();
  if (second.adjust_factor_ > adjust_factor_)
    adjust_factor_ = second.adjust_factor_;
  rating_ += second.rating();  // add ratings
  if (second.certainty() < certainty_) // take min
    certainty_ = second.certainty();
  if (second.dangerous_ambig_found_)
    dangerous_ambig_found_ = true;
  if (permuter_ == NO_PERM) {
    permuter_ = second.permuter();
  } else if (second.permuter() != NO_PERM &&
             second.permuter() != permuter_) {
    permuter_ = COMPOUND_PERM;
  }
  return *this;
}


/**
 * WERD_CHOICE::operator=
 *
 * Allocate enough memory to hold a copy of source and copy over
 * all the information from source to this WERD_CHOICE.
 */
WERD_CHOICE& WERD_CHOICE::operator=(const WERD_CHOICE& source) {
  while (reserved_ < source.length()) {
    this->double_the_size();
  }

  unicharset_ = source.unicharset_;
  const UNICHAR_ID *other_unichar_ids = source.unichar_ids();
  for (int i = 0; i < source.length(); ++i) {
    unichar_ids_[i] = other_unichar_ids[i];
    state_[i] = source.state_[i];
    certainties_[i] = source.certainties_[i];
    script_pos_[i] = source.BlobPosition(i);
  }
  length_ = source.length();
  adjust_factor_ = source.adjust_factor_;
  rating_ = source.rating();
  certainty_ = source.certainty();
  min_x_height_ = source.min_x_height();
  max_x_height_ = source.max_x_height();
  permuter_ = source.permuter();
  dangerous_ambig_found_ = source.dangerous_ambig_found_;
  return *this;
}

// Sets up the script_pos_ member using the blobs_list to get the bln
// bounding boxes, *this to get the unichars, and this->unicharset
// to get the target positions. If small_caps is true, sub/super are not
// considered, but dropcaps are.
// NOTE: blobs_list should be the chopped_word blobs. (Fully segemented.)
void WERD_CHOICE::SetScriptPositions(bool small_caps, TWERD* word, int debug) {
  // Initialize to normal.
  for (int i = 0; i < length_; ++i)
    script_pos_[i] = tesseract::SP_NORMAL;
  if (word->blobs.empty() || word->NumBlobs() != TotalOfStates()) {
    return;
  }

  int position_counts[4] = { 0, 0, 0, 0 };

  int chunk_index = 0;
  for (int blob_index = 0; blob_index < length_; ++blob_index, ++chunk_index) {
    TBLOB* tblob = word->blobs[chunk_index];
    int uni_id = unichar_id(blob_index);
    TBOX blob_box = tblob->bounding_box();
    if (state_ != nullptr) {
      for (int i = 1; i <  state_[blob_index]; ++i) {
        ++chunk_index;
        tblob = word->blobs[chunk_index];
        blob_box += tblob->bounding_box();
      }
    }
    script_pos_[blob_index] = ScriptPositionOf(false, *unicharset_, blob_box,
                                               uni_id);
    if (small_caps && script_pos_[blob_index] != tesseract::SP_DROPCAP) {
      script_pos_[blob_index] = tesseract::SP_NORMAL;
    }
    position_counts[script_pos_[blob_index]]++;
  }
  // If almost everything looks like a superscript or subscript,
  // we most likely just got the baseline wrong.
  if (position_counts[tesseract::SP_SUBSCRIPT] > 0.75 * length_ ||
      position_counts[tesseract::SP_SUPERSCRIPT] > 0.75 * length_) {
    if (debug >= 2) {
      tprintf("Most characters of %s are subscript or superscript.\n"
              "That seems wrong, so I'll assume we got the baseline wrong\n",
              unichar_string().c_str());
    }
    for (int i = 0; i < length_; i++) {
      ScriptPos sp = script_pos_[i];
      if (sp == tesseract::SP_SUBSCRIPT || sp == tesseract::SP_SUPERSCRIPT) {
        position_counts[sp]--;
        position_counts[tesseract::SP_NORMAL]++;
        script_pos_[i] = tesseract::SP_NORMAL;
      }
    }
  }

  if ((debug >= 1 && position_counts[tesseract::SP_NORMAL] < length_) ||
      debug >= 2) {
    tprintf("SetScriptPosition on %s\n", unichar_string().c_str());
    int chunk_index = 0;
    for (int blob_index = 0; blob_index < length_; ++blob_index) {
      if (debug >= 2 || script_pos_[blob_index] != tesseract::SP_NORMAL) {
        TBLOB* tblob = word->blobs[chunk_index];
        ScriptPositionOf(true, *unicharset_, tblob->bounding_box(),
                         unichar_id(blob_index));
      }
      chunk_index += state_ != nullptr ? state_[blob_index] : 1;
    }
  }
}
// Sets the script_pos_ member from some source positions with a given length.
void WERD_CHOICE::SetScriptPositions(const tesseract::ScriptPos* positions,
                                     int length) {
  ASSERT_HOST(length == length_);
  if (positions != script_pos_) {
    delete [] script_pos_;
    script_pos_ = new ScriptPos[length];
    memcpy(script_pos_, positions, sizeof(positions[0]) * length);
  }
}
// Sets all the script_pos_ positions to the given position.
void WERD_CHOICE::SetAllScriptPositions(tesseract::ScriptPos position) {
  for (int i = 0; i < length_; ++i)
    script_pos_[i] = position;
}

/* static */
ScriptPos WERD_CHOICE::ScriptPositionOf(bool print_debug,
                                        const UNICHARSET& unicharset,
                                        const TBOX& blob_box,
                                        UNICHAR_ID unichar_id) {
  ScriptPos retval = tesseract::SP_NORMAL;
  int top = blob_box.top();
  int bottom = blob_box.bottom();
  int min_bottom, max_bottom, min_top, max_top;
  unicharset.get_top_bottom(unichar_id,
                            &min_bottom, &max_bottom,
                            &min_top, &max_top);

  int sub_thresh_top = min_top - kMinSubscriptOffset;
  int sub_thresh_bot = kBlnBaselineOffset - kMinSubscriptOffset;
  int sup_thresh_bot = max_bottom + kMinSuperscriptOffset;
  if (bottom <= kMaxDropCapBottom) {
    retval = tesseract::SP_DROPCAP;
  } else if (top < sub_thresh_top && bottom < sub_thresh_bot) {
    retval = tesseract::SP_SUBSCRIPT;
  } else if (bottom > sup_thresh_bot) {
    retval = tesseract::SP_SUPERSCRIPT;
  }

  if (print_debug) {
    const char *pos = ScriptPosToString(retval);
    tprintf("%s Character %s[bot:%d top: %d]  "
            "bot_range[%d,%d]  top_range[%d, %d] "
            "sub_thresh[bot:%d top:%d]  sup_thresh_bot %d\n",
            pos, unicharset.id_to_unichar(unichar_id),
            bottom, top,
            min_bottom, max_bottom, min_top, max_top,
            sub_thresh_bot, sub_thresh_top,
            sup_thresh_bot);
  }
  return retval;
}

// Returns the script-id (eg Han) of the dominant script in the word.
int WERD_CHOICE::GetTopScriptID() const {
  int max_script = unicharset_->get_script_table_size();
  int *sid = new int[max_script];
  int x;
  for (x = 0; x < max_script; x++) sid[x] = 0;
  for (x = 0; x < length_; ++x) {
    int script_id = unicharset_->get_script(unichar_id(x));
    sid[script_id]++;
  }
  if (unicharset_->han_sid() != unicharset_->null_sid()) {
    // Add the Hiragana & Katakana counts to Han and zero them out.
    if (unicharset_->hiragana_sid() != unicharset_->null_sid()) {
      sid[unicharset_->han_sid()] += sid[unicharset_->hiragana_sid()];
      sid[unicharset_->hiragana_sid()] = 0;
    }
    if (unicharset_->katakana_sid() != unicharset_->null_sid()) {
      sid[unicharset_->han_sid()] += sid[unicharset_->katakana_sid()];
      sid[unicharset_->katakana_sid()] = 0;
    }
  }
  // Note that high script ID overrides lower one on a tie, thus biasing
  // towards non-Common script (if sorted that way in unicharset file).
  int max_sid = 0;
  for (x = 1; x < max_script; x++)
    if (sid[x] >= sid[max_sid]) max_sid = x;
  if (sid[max_sid] < length_ / 2)
    max_sid = unicharset_->null_sid();
  delete[] sid;
  return max_sid;
}

// Fixes the state_ for a chop at the given blob_posiiton.
void WERD_CHOICE::UpdateStateForSplit(int blob_position) {
  int total_chunks = 0;
  for (int i = 0; i < length_; ++i) {
    total_chunks += state_[i];
    if (total_chunks > blob_position) {
      ++state_[i];
      return;
    }
  }
}

// Returns the sum of all the state elements, being the total number of blobs.
int WERD_CHOICE::TotalOfStates() const {
  int total_chunks = 0;
  for (int i = 0; i < length_; ++i) {
    total_chunks += state_[i];
  }
  return total_chunks;
}

/**
 * WERD_CHOICE::print
 *
 * Print WERD_CHOICE to stdout.
 */
void WERD_CHOICE::print(const char *msg) const {
  tprintf("%s : ", msg);
  for (int i = 0; i < length_; ++i) {
    tprintf("%s", unicharset_->id_to_unichar(unichar_ids_[i]));
  }
  tprintf(" : R=%g, C=%g, F=%g, Perm=%d, xht=[%g,%g], ambig=%d\n",
          rating_, certainty_, adjust_factor_, permuter_,
          min_x_height_, max_x_height_, dangerous_ambig_found_);
  tprintf("pos");
  for (int i = 0; i < length_; ++i) {
    tprintf("\t%s", ScriptPosToString(script_pos_[i]));
  }
  tprintf("\nstr");
  for (int i = 0; i < length_; ++i) {
    tprintf("\t%s", unicharset_->id_to_unichar(unichar_ids_[i]));
  }
  tprintf("\nstate:");
  for (int i = 0; i < length_; ++i) {
    tprintf("\t%d ", state_[i]);
  }
  tprintf("\nC");
  for (int i = 0; i < length_; ++i) {
    tprintf("\t%.3f", certainties_[i]);
  }
  tprintf("\n");
}

// Prints the segmentation state with an introductory message.
void WERD_CHOICE::print_state(const char *msg) const {
  tprintf("%s", msg);
  for (int i = 0; i < length_; ++i)
    tprintf(" %d", state_[i]);
  tprintf("\n");
}

#ifndef GRAPHICS_DISABLED

// Displays the segmentation state of *this (if not the same as the last
// one displayed) and waits for a click in the window.
void WERD_CHOICE::DisplaySegmentation(TWERD* word) {
  // Number of different colors to draw with.
  const int kNumColors = 6;
  static ScrollView *segm_window = nullptr;
  // Check the state against the static prev_drawn_state.
  static GenericVector<int> prev_drawn_state;
  bool already_done = prev_drawn_state.size() == length_;
  if (!already_done) prev_drawn_state.init_to_size(length_, 0);
  for (int i = 0; i < length_; ++i) {
    if (prev_drawn_state[i] != state_[i]) {
      already_done = false;
    }
    prev_drawn_state[i] = state_[i];
  }
  if (already_done || word->blobs.empty()) return;

  // Create the window if needed.
  if (segm_window == nullptr) {
    segm_window = new ScrollView("Segmentation", 5, 10, 500, 256,
                                 2000.0, 256.0, true);
  } else {
    segm_window->Clear();
  }

  TBOX bbox;
  int blob_index = 0;
  for (int c = 0; c < length_; ++c) {
    auto color =
        static_cast<ScrollView::Color>(c % kNumColors + 3);
    for (int i = 0; i < state_[c]; ++i, ++blob_index) {
      TBLOB* blob = word->blobs[blob_index];
      bbox += blob->bounding_box();
      blob->plot(segm_window, color, color);
    }
  }
  segm_window->ZoomToRectangle(bbox.left(), bbox.top(),
                               bbox.right(), bbox.bottom());
  segm_window->Update();
  segm_window->Wait();
}

#endif // !GRAPHICS_DISABLED

bool EqualIgnoringCaseAndTerminalPunct(const WERD_CHOICE &word1,
                                       const WERD_CHOICE &word2) {
  const UNICHARSET *uchset = word1.unicharset();
  if (word2.unicharset() != uchset) return false;
  int w1start, w1end;
  word1.punct_stripped(&w1start, &w1end);
  int w2start, w2end;
  word2.punct_stripped(&w2start, &w2end);
  if (w1end - w1start != w2end - w2start) return false;
  for (int i = 0; i < w1end - w1start; i++) {
    if (uchset->to_lower(word1.unichar_id(w1start + i)) !=
        uchset->to_lower(word2.unichar_id(w2start + i))) {
        return false;
    }
  }
  return true;
}

/**
 * print_ratings_list
 *
 * Send all the ratings out to the logfile.
 *
 * @param msg intro message
 * @param ratings list of ratings
 * @param current_unicharset unicharset that can be used
 * for id-to-unichar conversion
 */
void print_ratings_list(const char *msg,
                        BLOB_CHOICE_LIST *ratings,
                        const UNICHARSET &current_unicharset) {
  if (ratings->length() == 0) {
    tprintf("%s:<none>\n", msg);
    return;
  }
  if (*msg != '\0') {
    tprintf("%s\n", msg);
  }
  BLOB_CHOICE_IT c_it;
  c_it.set_to_list(ratings);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    c_it.data()->print(&current_unicharset);
    if (!c_it.at_last()) tprintf("\n");
  }
  tprintf("\n");
  fflush(stdout);
}
