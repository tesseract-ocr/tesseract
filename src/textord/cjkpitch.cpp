///////////////////////////////////////////////////////////////////////
// File:        cjkpitch.cpp
// Description: Code to determine fixed pitchness and the pitch if fixed,
//              for CJK text.
// Author:      takenaka@google.com (Hiroshi Takenaka)
//
// Copyright 2011 Google Inc. All Rights Reserved.
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

#include "cjkpitch.h"
#include <tesseract/genericvector.h>
#include "topitch.h"
#include "tovars.h"

#include <algorithm>
#include <vector>       // for std::vector

static BOOL_VAR(textord_space_size_is_variable, false,
                "If true, word delimiter spaces are assumed to have "
                "variable width, even though characters have fixed pitch.");

namespace {

// Allow +/-10% error for character pitch / body size.
static const float kFPTolerance = 0.1f;

// Minimum ratio of "good" character pitch for a row to be considered
// to be fixed-pitch.
static const float kFixedPitchThreshold = 0.35f;

// rank statistics for a small collection of float values.
class SimpleStats {
 public:
  SimpleStats(): finalized_(false), values_() { }
  ~SimpleStats() { }

  void Clear() {
    values_.clear();
    finalized_ = false;
  }

  void Add(float value) {
    values_.push_back(value);
    finalized_ = false;
  }

  void Finish() {
    values_.sort(float_compare);
    finalized_ = true;
  }

  float ile(double frac) {
    if (!finalized_) Finish();
    if (values_.empty()) return 0.0;
    if (frac >= 1.0) return values_.back();
    if (frac <= 0.0 || values_.size() == 1) return values_[0];
    int index = static_cast<int>((values_.size() - 1) * frac);
    float reminder = (values_.size() - 1) * frac - index;

    return values_[index] * (1.0 - reminder) +
        values_[index + 1] * reminder;
  }

  float median() {
    return ile(0.5);
  }

  float minimum() {
    if (!finalized_) Finish();
    if (values_.empty()) return 0.0;
    return values_[0];
  }

  int size() const {
    return values_.size();
  }

 private:
  static int float_compare(const void* a, const void* b) {
    const auto* f_a = static_cast<const float*>(a);
    const auto* f_b = static_cast<const float*>(b);
    return (*f_a > *f_b) ? 1 : ((*f_a < *f_b) ? -1 : 0);
  }

  bool finalized_;
  GenericVector<float> values_;
};

// statistics for a small collection of float pairs (x, y).
// EstimateYFor(x, r) returns the estimated y at x, based on
// existing samples between x*(1-r) ~ x*(1+r).
class LocalCorrelation {
 public:
  struct float_pair {
    float x, y;
    int vote;
  };

  LocalCorrelation(): finalized_(false) { }
  ~LocalCorrelation() { }

  void Finish() {
    values_.sort(float_pair_compare);
    finalized_ = true;
  }

  void Clear() {
    finalized_ = false;
  }

  void Add(float x, float y, int v) {
    struct float_pair value;
    value.x = x;
    value.y = y;
    value.vote = v;
    values_.push_back(value);
    finalized_ = false;
  }

  float EstimateYFor(float x, float r) {
    ASSERT_HOST(finalized_);
    int start = 0, end = values_.size();
    // Because the number of samples (used_) is assumed to be small,
    // just use linear search to find values within the range.
    while (start < values_.size() && values_[start].x < x * (1.0 - r)) start++;
    while (end - 1 >= 0 && values_[end - 1].x > x * (1.0 + r)) end--;

    // Fall back to the global average if there are no data within r
    // of x.
    if (start >= end) {
      start = 0;
      end = values_.size();
    }

    // Compute weighted average of the values.
    float rc = 0;
    int vote = 0;
    for (int i = start; i < end; i++) {
      rc += values_[i].vote * x * values_[i].y / values_[i].x;
      vote += values_[i].vote;
    }

    return rc / vote;
  }

 private:
  static int float_pair_compare(const void* a, const void* b) {
    const auto* f_a = static_cast<const float_pair*>(a);
    const auto* f_b = static_cast<const float_pair*>(b);
    return (f_a->x > f_b->x) ? 1 : ((f_a->x < f_b->x) ? -1 : 0);
  }

  bool finalized_;
  GenericVector<struct float_pair> values_;
};

// Class to represent a character on a fixed pitch row.  A FPChar may
// consist of multiple blobs (BLOBNBOX's).
class FPChar {
 public:
  enum Alignment {
    ALIGN_UNKNOWN, ALIGN_GOOD, ALIGN_BAD
  };

  FPChar(): box_(), real_body_(),
            from_(nullptr), to_(nullptr), num_blobs_(0), max_gap_(0),
            final_(false), alignment_(ALIGN_UNKNOWN),
            merge_to_prev_(false), delete_flag_(false) {
  }

  // Initialize from blob.
  void Init(BLOBNBOX *blob) {
    box_ = blob->bounding_box();
    real_body_ = box_;
    from_ = to_ = blob;
    num_blobs_ = 1;
  }

  // Merge this character with "next". The "next" character should
  // consist of succeeding blobs on the same row.
  void Merge(const FPChar &next) {
    int gap = real_body_.x_gap(next.real_body_);
    if (gap > max_gap_) max_gap_ = gap;

    box_ += next.box_;
    real_body_ += next.real_body_;
    to_ = next.to_;
    num_blobs_ += next.num_blobs_;
  }

  // Accessors.
  const TBOX &box() const { return box_; }
  void set_box(const TBOX &box) {
    box_ = box;
  }
  const TBOX &real_body() const { return real_body_; }

  bool is_final() const { return final_; }
  void set_final(bool flag) {
    final_ = flag;
  }

  const Alignment& alignment() const {
    return alignment_;
  }
  void set_alignment(Alignment alignment) {
    alignment_ = alignment;
  }

  bool merge_to_prev() const {
    return merge_to_prev_;
  }
  void set_merge_to_prev(bool flag) {
    merge_to_prev_ = flag;
  }

  bool delete_flag() const {
    return delete_flag_;
  }
  void set_delete_flag(bool flag) {
    delete_flag_ = flag;
  }

  int max_gap() const {
    return max_gap_;
  }

  int num_blobs() const {
    return num_blobs_;
  }

 private:
  TBOX box_;  // Rectangle region considered to be occupied by this
  // character.  It could be bigger than the bounding box.
  TBOX real_body_;  // Real bounding box of this character.
  BLOBNBOX *from_;  // The first blob of this character.
  BLOBNBOX *to_;  // The last blob of this character.
  int num_blobs_;  // Number of blobs that belong to this character.
  int max_gap_;  // Maximum x gap between the blobs.

  bool final_;  // True if alignment/fragmentation decision for this
  // character is finalized.

  Alignment alignment_;  // Alignment status.
  bool merge_to_prev_;  // True if this is a fragmented blob that
  // needs to be merged to the previous
  // character.

  int delete_flag_;  // True if this character is merged to another
  // one and needs to be deleted.
};

// Class to represent a fixed pitch row, as a linear collection of
// FPChar's.
class FPRow {
 public:
  FPRow() : all_pitches_(), all_gaps_(), good_pitches_(), good_gaps_(),
            heights_(), characters_() {
  }

  ~FPRow() { }

  // Initialize from TD_ROW.
  void Init(TO_ROW *row);

  // Estimate character pitch of this row, based on current alignment
  // status of underlying FPChar's.  The argument pass1 can be set to
  // true if the function is called after Pass1Analyze(), to eliminate
  // some redundant computation.
  void EstimatePitch(bool pass1);

  // Check each character if it has good character pitches between its
  // predecessor and its successor and set its alignment status.  If
  // we already calculated the estimated pitch for this row, the value
  // is used.  If we didn't, a character is considered to be good, if
  // the pitches between its predecessor and its successor are almost
  // equal.
  void Pass1Analyze();

  // Find characters that fit nicely into one imaginary body next to a
  // character which is already finalized. Then mark them as character
  // fragments.
  bool Pass2Analyze();

  // Merge FPChars marked as character fragments into one.
  void MergeFragments();

  // Finalize characters that are already large enough and cannot be
  // merged with others any more.
  void FinalizeLargeChars();

  // Output pitch estimation results to attributes of TD_ROW.
  void OutputEstimations();

  void DebugOutputResult(int row_index);

  int good_pitches() {
    return good_pitches_.size();
  }

  float pitch() {
    return pitch_;
  }

  float estimated_pitch() {
    return estimated_pitch_;
  }

  void set_estimated_pitch(float v) {
    estimated_pitch_ = v;
  }

  float height() {
    return height_;
  }

  float height_pitch_ratio() {
    if (good_pitches_.size() < 2) return -1.0;
    return height_ / good_pitches_.median();
  }

  float gap() {
    return gap_;
  }

  size_t num_chars() {
    return characters_.size();
  }
  FPChar *character(int i) {
    return &characters_[i];
  }

  const TBOX &box(int i) {
    return characters_[i].box();
  }

  const TBOX &real_body(int i) {
    return characters_[i].real_body();
  }

  bool is_box_modified(int i) {
    return !(characters_[i].box() == characters_[i].real_body());
  }

  float center_x(int i) {
    return (characters_[i].box().left() + characters_[i].box().right()) / 2.0;
  }

  bool is_final(int i) {
    return characters_[i].is_final();
  }

  void finalize(int i) {
    characters_[i].set_final(true);
  }

  bool is_good(int i) {
    return characters_[i].alignment() == FPChar::ALIGN_GOOD;
  }

  void mark_good(int i) {
    characters_[i].set_alignment(FPChar::ALIGN_GOOD);
  }

  void mark_bad(int i) {
    characters_[i].set_alignment(FPChar::ALIGN_BAD);
  }

  void clear_alignment(int i) {
    characters_[i].set_alignment(FPChar::ALIGN_UNKNOWN);
  }

 private:
  static float x_overlap_fraction(const TBOX& box1, const TBOX& box2) {
    if (std::min(box1.width(), box2.width()) == 0) return 0.0;
    return -box1.x_gap(box2) / static_cast<float>(std::min(box1.width(), box2.width()));
  }

  static bool mostly_overlap(const TBOX& box1, const TBOX& box2) {
    return x_overlap_fraction(box1, box2) > 0.9;
  }

  static bool significant_overlap(const TBOX& box1, const TBOX& box2) {
    if (std::min(box1.width(), box2.width()) == 0) return false;
    int overlap = -box1.x_gap(box2);
    return overlap > 1 || x_overlap_fraction(box1, box2) > 0.1;
  }

  static float box_pitch(const TBOX& ref, const TBOX& box) {
    return abs(ref.left() + ref.right() - box.left() - box.right()) / 2.0;
  }

  // Check if two neighboring characters satisfy the fixed pitch model.
  static bool is_good_pitch(float pitch, const TBOX& box1, const TBOX& box2) {
    // Character box shouldn't exceed pitch.
    if (box1.width() >= pitch * (1.0 + kFPTolerance) ||
        box2.width() >= pitch * (1.0 + kFPTolerance) ||
        box1.height() >= pitch * (1.0 + kFPTolerance) ||
        box2.height() >= pitch * (1.0 + kFPTolerance)) return false;

    const float real_pitch = box_pitch(box1, box2);
    if (fabs(real_pitch - pitch) < pitch * kFPTolerance) return true;

    if (textord_space_size_is_variable) {
      // Hangul characters usually have fixed pitch, but words are
      // delimited by space which can be narrower than characters.
      if (real_pitch > pitch && real_pitch < pitch * 2.0 &&
          real_pitch - box1.x_gap(box2) < pitch) {
        return true;
      }
    }
    return false;
  }

  static bool is_interesting_blob(const BLOBNBOX *blob) {
    return !blob->joined_to_prev() && blob->flow() != BTFT_LEADER;
  }

  // Cleanup chars that are already merged to others.
  void DeleteChars() {
    int index = 0;
    for (int i = 0; i < characters_.size(); ++i) {
      if (!characters_[i].delete_flag()) {
        if (index != i) characters_[index] = characters_[i];
        index++;
      }
    }
    characters_.truncate(index);
  }

  float pitch_ = 0.0f; // Character pitch.
  float estimated_pitch_ = 0.0f; // equal to pitch_ if pitch_ is considered
  // to be good enough.
  float height_ = 0.0f; // Character height.
  float gap_ = 0.0f; // Minimum gap between characters.

  // Pitches between any two successive characters.
  SimpleStats all_pitches_;
  // Gaps between any two successive characters.
  SimpleStats all_gaps_;
  // Pitches between any two successive characters that are consistent
  // with the fixed pitch model.
  SimpleStats good_pitches_;
  // Gaps between any two successive characters that are consistent
  // with the fixed pitch model.
  SimpleStats good_gaps_;

  SimpleStats heights_;

  GenericVector<FPChar> characters_;
  TO_ROW *real_row_ = nullptr; // Underlying TD_ROW for this row.
};

void FPRow::Init(TO_ROW *row) {
  ASSERT_HOST(row != nullptr);
  ASSERT_HOST(row->xheight > 0);
  real_row_ = row;
  real_row_->pitch_decision = PITCH_CORR_PROP;  // Default decision.

  BLOBNBOX_IT blob_it = row->blob_list();
  // Initialize characters_ and compute the initial estimation of
  // character height.
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    if (is_interesting_blob(blob_it.data())) {
      FPChar fp_char;
      fp_char.Init(blob_it.data());
      // Merge unconditionally if two blobs overlap.
      if (!characters_.empty() &&
          significant_overlap(fp_char.box(), characters_.back().box())) {
        characters_.back().Merge(fp_char);
      } else {
        characters_.push_back(fp_char);
      }
      TBOX bound = blob_it.data()->bounding_box();
      if (bound.height() * 3.0 > bound.width()) {
        heights_.Add(bound.height());
      }
    }
  }
  heights_.Finish();
  height_ = heights_.ile(0.875);
}

void FPRow::OutputEstimations() {
  if (good_pitches_.size() == 0) {
    pitch_ = 0.0f;
    real_row_->pitch_decision = PITCH_CORR_PROP;
    return;
  }

  pitch_ = good_pitches_.median();
  real_row_->fixed_pitch = pitch_;
  // good_gaps_.ile(0.125) can be large if most characters on the row
  // are skinny. Use pitch_ - height_ instead if it's smaller, but
  // positive.
  real_row_->kern_size = real_row_->pr_nonsp =
          std::min(good_gaps_.ile(0.125), std::max(pitch_ - height_, 0.0f));
  real_row_->body_size = pitch_ - real_row_->kern_size;

  if (good_pitches_.size() < all_pitches_.size() * kFixedPitchThreshold) {
    // If more than half of the characters of a line don't fit to the
    // fixed pitch model, consider the line to be proportional. 50%
    // seems to be a good threshold in practice as well.
    // Anyway we store estimated values (fixed_pitch, kern_size, etc.) in
    // real_row_ as a partial estimation result and try to use them in the
    // normalization process.
    real_row_->pitch_decision = PITCH_CORR_PROP;
    return;
  } else if (good_pitches_.size() > all_pitches_.size() * 0.75) {
    real_row_->pitch_decision = PITCH_DEF_FIXED;
  } else {
    real_row_->pitch_decision = PITCH_CORR_FIXED;
  }

  real_row_->space_size = real_row_->pr_space = pitch_;
  // Set min_space to 50% of character pitch so that we can break CJK
  // text at a half-width space after punctuation.
  real_row_->min_space = (pitch_ + good_gaps_.minimum()) * 0.5;

  // Don't consider a quarter space as a real space, because it's used
  // for line justification in traditional Japanese books.
  real_row_->max_nonspace = std::max(pitch_ * 0.25 + good_gaps_.minimum(),
                                static_cast<double>(good_gaps_.ile(0.875)));

  int space_threshold =
          std::min((real_row_->max_nonspace + real_row_->min_space) / 2,
                   static_cast<int>(real_row_->xheight));

  // Make max_nonspace larger than any intra-character gap so that
  // make_prop_words() won't break a row at the middle of a character.
  for (size_t i = 0; i < num_chars(); ++i) {
    if (characters_[i].max_gap() > real_row_->max_nonspace) {
      real_row_->max_nonspace = characters_[i].max_gap();
    }
  }
  real_row_->space_threshold =
          std::min((real_row_->max_nonspace + real_row_->min_space) / 2,
                   static_cast<int>(real_row_->xheight));
  real_row_->used_dm_model = false;

  // Setup char_cells.
  ICOORDELT_IT cell_it = &real_row_->char_cells;
  auto *cell = new ICOORDELT(real_body(0).left(), 0);
  cell_it.add_after_then_move(cell);

  int right = real_body(0).right();
  for (size_t i = 1; i < num_chars(); ++i) {
    // Put a word break if gap between two characters is bigger than
    // space_threshold.  Don't break if none of two characters
    // couldn't be "finalized", because maybe they need to be merged
    // to one character.
    if ((is_final(i - 1) || is_final(i)) &&
        real_body(i - 1).x_gap(real_body(i)) > space_threshold) {
      cell = new ICOORDELT(right + 1, 0);
      cell_it.add_after_then_move(cell);
      while (right + pitch_ < box(i).left()) {
        right += pitch_;
        cell = new ICOORDELT(right + 1, 0);
        cell_it.add_after_then_move(cell);
      }
      right = box(i).left();
    }
    cell = new ICOORDELT((right + real_body(i).left()) / 2, 0);
    cell_it.add_after_then_move(cell);
    right = real_body(i).right();
  }

  cell = new ICOORDELT(right + 1, 0);
  cell_it.add_after_then_move(cell);

  // TODO(takenaka): add code to store alignment/fragmentation
  // information to blobs so that it can be reused later, e.g. in
  // recognition phase.
}

void FPRow::EstimatePitch(bool pass1) {
  good_pitches_.Clear();
  all_pitches_.Clear();
  good_gaps_.Clear();
  all_gaps_.Clear();
  heights_.Clear();
  if (num_chars() == 0) return;

  int32_t cx0, cx1;
  bool prev_was_good = is_good(0);
  cx0 = center_x(0);

  heights_.Add(box(0).height());
  for (size_t i = 1; i < num_chars(); i++) {
    cx1 = center_x(i);
    int32_t pitch = cx1 - cx0;
    int32_t gap = std::max(0, real_body(i - 1).x_gap(real_body(i)));

    heights_.Add(box(i).height());
    // Ignore if the pitch is too close.  But don't ignore wide pitch
    // may be the result of large tracking.
    if (pitch > height_ * 0.5) {
      all_pitches_.Add(pitch);
      all_gaps_.Add(gap);
      if (is_good(i)) {
        // In pass1 (after Pass1Analyze()), all characters marked as
        // "good" have a good consistent pitch with their previous
        // characters.  However, it's not true in pass2 and a good
        // character may have a good pitch only between its successor.
        // So we collect only pitch values between two good
        // characters. and within tolerance in pass2.
        if (pass1 || (prev_was_good &&
                      fabs(estimated_pitch_ - pitch) <
                          kFPTolerance * estimated_pitch_)) {
          good_pitches_.Add(pitch);
          if (!is_box_modified(i - 1) && !is_box_modified(i)) {
            good_gaps_.Add(gap);
          }
        }
        prev_was_good = true;
      } else {
        prev_was_good = false;
      }
    }
    cx0 = cx1;
  }

  good_pitches_.Finish();
  all_pitches_.Finish();
  good_gaps_.Finish();
  all_gaps_.Finish();
  heights_.Finish();

  height_ = heights_.ile(0.875);
  if (all_pitches_.size() == 0) {
    pitch_ = 0.0f;
    gap_ = 0.0f;
  } else if (good_pitches_.size() < 2) {
    // We don't have enough data to estimate the pitch of this row yet.
    // Use median of all pitches as the initial guess.
    pitch_ = all_pitches_.median();
    ASSERT_HOST(pitch_ > 0.0f);
    gap_ = all_gaps_.ile(0.125);
  } else {
    pitch_ = good_pitches_.median();
    ASSERT_HOST(pitch_ > 0.0f);
    gap_ = good_gaps_.ile(0.125);
  }
}

void FPRow::DebugOutputResult(int row_index) {
  if (num_chars() > 0) {
    tprintf("Row %d: pitch_decision=%d, fixed_pitch=%f, max_nonspace=%d, "
            "space_size=%f, space_threshold=%d, xheight=%f\n",
            row_index, static_cast<int>(real_row_->pitch_decision),
            real_row_->fixed_pitch, real_row_->max_nonspace,
            real_row_->space_size, real_row_->space_threshold,
            real_row_->xheight);

    for (unsigned i = 0; i < num_chars(); i++) {
      tprintf("Char %u: is_final=%d is_good=%d num_blobs=%d: ",
              i, is_final(i), is_good(i), character(i)->num_blobs());
      box(i).print();
    }
  }
}

void FPRow::Pass1Analyze() {
  if (num_chars() < 2) return;

  if (estimated_pitch_ > 0.0f) {
    for (size_t i = 2; i < num_chars(); i++) {
      if (is_good_pitch(estimated_pitch_, box(i - 2), box(i-1)) &&
          is_good_pitch(estimated_pitch_, box(i - 1), box(i))) {
        mark_good(i - 1);
      }
    }
  } else {
    for (size_t i = 2; i < num_chars(); i++) {
      if (is_good_pitch(box_pitch(box(i-2), box(i-1)), box(i - 1), box(i))) {
        mark_good(i - 1);
      }
    }
  }
  character(0)->set_alignment(character(1)->alignment());
  character(num_chars() - 1)->set_alignment(
      character(num_chars() - 2)->alignment());
}

bool FPRow::Pass2Analyze() {
  bool changed = false;
  if (num_chars() <= 1 || estimated_pitch_ == 0.0f) {
    return false;
  }
  for (size_t i = 0; i < num_chars(); i++) {
    if (is_final(i)) continue;

    FPChar::Alignment alignment = character(i)->alignment();
    bool intersecting = false;
    bool not_intersecting = false;

    if (i < num_chars() - 1 && is_final(i + 1)) {
      // Next character is already finalized. Estimate the imaginary
      // body including this character based on the character. Skip
      // whitespace if necessary.
      bool skipped_whitespaces = false;
      float c1 = center_x(i + 1)  - 1.5 * estimated_pitch_;
      while (c1 > box(i).right()) {
        skipped_whitespaces = true;
        c1 -= estimated_pitch_;
      }
      TBOX ibody(c1, box(i).bottom(), c1 + estimated_pitch_, box(i).top());

      // Collect all characters that mostly fit in the region.
      // Also, their union height shouldn't be too big.
      int j = i;
      TBOX merged;
      while (j >= 0 && !is_final(j) && mostly_overlap(ibody, box(j)) &&
             merged.bounding_union(box(j)).height() <
             estimated_pitch_ * (1 + kFPTolerance)) {
        merged += box(j);
        j--;
      }

      if (j >= 0 && significant_overlap(ibody, box(j))) {
        // character(j) lies on the character boundary and doesn't fit
        // well into the imaginary body.
        if (!is_final(j)) intersecting = true;
      } else {
        not_intersecting = true;
        if (i - j > 0) {
          // Merge character(j+1) ... character(i) because they fit
          // into the body nicely.
          if (i - j == 1) {
            // Only one char in the imaginary body.
            if (!skipped_whitespaces) mark_good(i);
            // set ibody as bounding box of this character to get
            // better pitch analysis result for halfwidth glyphs
            // followed by a halfwidth space.
            if (box(i).width() <= estimated_pitch_ * 0.5) {
              ibody += box(i);
              character(i)->set_box(ibody);
            }
            character(i)->set_merge_to_prev(false);
            finalize(i);
          } else {
            for (int k = i; k > j + 1; k--) {
              character(k)->set_merge_to_prev(true);
            }
          }
        }
      }
    }
    if (i > 0 && is_final(i - 1)) {
      // Now we repeat everything from the opposite side.  Previous
      // character is already finalized. Estimate the imaginary body
      // including this character based on the character.
      bool skipped_whitespaces = false;
      float c1 = center_x(i - 1) + 1.5 * estimated_pitch_;
      while (c1 < box(i).left()) {
        skipped_whitespaces = true;
        c1 += estimated_pitch_;
      }
      TBOX ibody(c1 - estimated_pitch_, box(i).bottom(), c1, box(i).top());

      size_t j = i;
      TBOX merged;
      while (j < num_chars() && !is_final(j) && mostly_overlap(ibody, box(j)) &&
             merged.bounding_union(box(j)).height() <
             estimated_pitch_ * (1 + kFPTolerance)) {
        merged += box(j);
        j++;
      }

      if (j < num_chars() && significant_overlap(ibody, box(j))) {
        if (!is_final(j)) intersecting = true;
      } else {
        not_intersecting = true;
        if (j - i > 0) {
          if (j - i == 1) {
            if (!skipped_whitespaces) mark_good(i);
            if (box(i).width() <= estimated_pitch_ * 0.5) {
              ibody += box(i);
              character(i)->set_box(ibody);
            }
            character(i)->set_merge_to_prev(false);
            finalize(i);
          } else {
            for (size_t k = i + 1; k < j; k++) {
              character(k)->set_merge_to_prev(true);
            }
          }
        }
      }
    }

    // This character doesn't fit well into the estimated imaginary
    // bodies. Mark it as bad.
    if (intersecting && !not_intersecting) mark_bad(i);
    if (character(i)->alignment() != alignment ||
        character(i)->merge_to_prev()) {
      changed = true;
    }
  }

  return changed;
}

void FPRow::MergeFragments() {
  int last_char = 0;

  for (size_t j = 0; j < num_chars(); ++j) {
    if (character(j)->merge_to_prev()) {
      character(last_char)->Merge(*character(j));
      character(j)->set_delete_flag(true);
      clear_alignment(last_char);
      character(j-1)->set_merge_to_prev(false);
    } else {
      last_char = j;
    }
  }
  DeleteChars();
}

void FPRow::FinalizeLargeChars() {
  float row_pitch = estimated_pitch();
  for (size_t i = 0; i < num_chars(); i++) {
    if (is_final(i)) continue;

    // Finalize if both neighbors are finalized. We have no other choice.
    if (i > 0 && is_final(i - 1) && i < num_chars() - 1 && is_final(i + 1)) {
      finalize(i);
      continue;
    }

    float cx = center_x(i);
    TBOX ibody(cx - 0.5 * row_pitch, 0, cx + 0.5 * row_pitch, 1);
    if (i > 0) {
      // The preceding character significantly intersects with the
      // imaginary body of this character. Let Pass2Analyze() handle
      // this case.
      if (x_overlap_fraction(ibody, box(i - 1)) > 0.1) continue;
      if (!is_final(i - 1)) {
        TBOX merged = box(i);
        merged += box(i - 1);
        if (merged.width() < row_pitch) continue;
        // This character cannot be finalized yet because it can be
        // merged with the previous one.  Again, let Pass2Analyze()
        // handle this case.
      }
    }
    if (i < num_chars() - 1) {
      if (x_overlap_fraction(ibody, box(i + 1)) > 0.1) continue;
      if (!is_final(i + 1)) {
        TBOX merged = box(i);
        merged += box(i + 1);
        if (merged.width() < row_pitch) continue;
      }
    }
    finalize(i);
  }

  // Update alignment decision.  We only consider finalized characters
  // in pass2.  E.g. if a finalized character C has another finalized
  // character L on its left and a not-finalized character R on its
  // right, we mark C as good if the pitch between C and L is good,
  // regardless of the pitch between C and R.
  for (size_t i = 0; i < num_chars(); i++) {
    if (!is_final(i)) continue;
    bool good_pitch = false;
    bool bad_pitch = false;
    if (i > 0 && is_final(i - 1)) {
      if (is_good_pitch(row_pitch, box(i - 1), box(i))) {
        good_pitch = true;
      } else {
        bad_pitch = true;
      }
    }
    if (i < num_chars() - 1 && is_final(i + 1)) {
      if (is_good_pitch(row_pitch, box(i), box(i + 1))) {
        good_pitch = true;
      } else {
        bad_pitch = true;
      }
    }
    if (good_pitch && !bad_pitch) mark_good(i);
    else if (!good_pitch && bad_pitch) mark_bad(i);
  }
}

class FPAnalyzer {
 public:
  FPAnalyzer(ICOORD page_tr, TO_BLOCK_LIST *port_blocks);
  ~FPAnalyzer() { }

  void Pass1Analyze() {
    for (auto & row : rows_) row.Pass1Analyze();
  }

  // Estimate character pitch for each row.  The argument pass1 can be
  // set to true if the function is called after Pass1Analyze(), to
  // eliminate some redundant computation.
  void EstimatePitch(bool pass1);

  bool maybe_fixed_pitch() {
    if (rows_.empty() ||
        rows_.size() <= num_bad_rows_ + num_tall_rows_ + 1) return false;
    return true;
  }

  void MergeFragments() {
    for (auto & row : rows_) row.MergeFragments();
  }

  void FinalizeLargeChars() {
    for (auto & row : rows_) row.FinalizeLargeChars();
  }

  bool Pass2Analyze() {
    bool changed = false;
    for (auto & row : rows_) {
      if (row.Pass2Analyze()) {
        changed = true;
      }
    }
    return changed;
  }

  void OutputEstimations() {
    for (auto & row : rows_) row.OutputEstimations();
    // Don't we need page-level estimation of gaps/spaces?
  }

  void DebugOutputResult() {
    tprintf("FPAnalyzer: final result\n");
    for (size_t i = 0; i < rows_.size(); i++) rows_[i].DebugOutputResult(i);
  }

  size_t num_rows() {
    return rows_.size();
  }

  // Returns the upper limit for pass2 loop iteration.
  unsigned max_iteration() {
    // We're fixing at least one character per iteration. So basically
    // we shouldn't require more than max_chars_per_row_ iterations.
    return max_chars_per_row_ + 100;
  }

 private:
  ICOORD page_tr_;
  std::vector<FPRow> rows_;
  unsigned num_tall_rows_;
  unsigned num_bad_rows_;
  // TODO: num_empty_rows_ is incremented, but never used otherwise.
  unsigned num_empty_rows_;
  unsigned max_chars_per_row_;
};

FPAnalyzer::FPAnalyzer(ICOORD page_tr, TO_BLOCK_LIST *port_blocks)
: page_tr_(page_tr),
  num_tall_rows_(0),
  num_bad_rows_(0),
  num_empty_rows_(0),
  max_chars_per_row_(0)
{
  TO_BLOCK_IT block_it(port_blocks);

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    TO_BLOCK *block = block_it.data();
    if (!block->get_rows()->empty()) {
      ASSERT_HOST(block->xheight > 0);
      find_repeated_chars(block, false);
    }
  }

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    TO_ROW_IT row_it = block_it.data()->get_rows();
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      FPRow row;
      row.Init(row_it.data());
      rows_.push_back(row);
      size_t num_chars = rows_.back().num_chars();
      if (num_chars <= 1) num_empty_rows_++;
      if (num_chars > max_chars_per_row_) max_chars_per_row_ = num_chars;
    }
  }
}

void FPAnalyzer::EstimatePitch(bool pass1) {
  LocalCorrelation pitch_height_stats;

  num_tall_rows_ = 0;
  num_bad_rows_ = 0;
  pitch_height_stats.Clear();
  for (auto & row : rows_) {
    row.EstimatePitch(pass1);
    if (row.good_pitches()) {
      pitch_height_stats.Add(row.height() + row.gap(),
                             row.pitch(), row.good_pitches());
      if (row.height_pitch_ratio() > 1.1) num_tall_rows_++;
    } else {
      num_bad_rows_++;
    }
  }

  pitch_height_stats.Finish();
  for (auto & row : rows_) {
    if (row.good_pitches() >= 5) {
      // We have enough evidences. Just use the pitch estimation
      // from this row.
      row.set_estimated_pitch(row.pitch());
    } else if (row.num_chars() > 1) {
      float estimated_pitch =
          pitch_height_stats.EstimateYFor(row.height() + row.gap(),
                                          0.1f);
      // CJK characters are more likely to be fragmented than poorly
      // chopped. So trust the page-level estimation of character
      // pitch only if it's larger than row-level estimation or
      // row-level estimation is too large (2x bigger than row height).
      if (estimated_pitch > row.pitch() ||
          row.pitch() > row.height() * 2.0) {
        row.set_estimated_pitch(estimated_pitch);
      } else {
        row.set_estimated_pitch(row.pitch());
      }
    }
  }
}

}  // namespace

void compute_fixed_pitch_cjk(ICOORD page_tr,
                             TO_BLOCK_LIST *port_blocks) {
  FPAnalyzer analyzer(page_tr, port_blocks);
  if (analyzer.num_rows() == 0) return;

  analyzer.Pass1Analyze();
  analyzer.EstimatePitch(true);

  // Perform pass1 analysis again with the initial estimation of row
  // pitches, for better estimation.
  analyzer.Pass1Analyze();
  analyzer.EstimatePitch(true);

  // Early exit if the page doesn't seem to contain fixed pitch rows.
  if (!analyzer.maybe_fixed_pitch()) {
    if (textord_debug_pitch_test) {
      tprintf("Page doesn't seem to contain fixed pitch rows\n");
    }
    return;
  }

  unsigned iteration = 0;
  do {
    analyzer.MergeFragments();
    analyzer.FinalizeLargeChars();
    analyzer.EstimatePitch(false);
    iteration++;
  } while (analyzer.Pass2Analyze() && iteration < analyzer.max_iteration());

  if (textord_debug_pitch_test) {
    tprintf("compute_fixed_pitch_cjk finished after %u iteration (limit=%u)\n",
            iteration, analyzer.max_iteration());
  }

  analyzer.OutputEstimations();
  if (textord_debug_pitch_test) analyzer.DebugOutputResult();
}
