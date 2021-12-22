/******************************************************************************
 ** Filename:    intmatcher.cpp
 ** Purpose:     Generic high level classification routines.
 ** Author:      Robert Moss
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "intmatcher.h"

#include "classify.h"
#include "float2int.h"
#include "fontinfo.h"
#include "intproto.h"
#include "scrollview.h"
#include "shapetable.h"

#include "helpers.h"

#include <cassert>
#include <cmath>

namespace tesseract {

/*----------------------------------------------------------------------------
                    Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
// Parameters of the sigmoid used to convert similarity to evidence in the
// similarity_evidence_table_ that is used to convert distance metric to an
// 8 bit evidence value in the secondary matcher. (See IntMatcher::Init).
const float IntegerMatcher::kSEExponentialMultiplier = 0.0f;
const float IntegerMatcher::kSimilarityCenter = 0.0075f;

static const uint8_t offset_table[] = {
    255, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2,
    0,   1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
    1,   0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1,
    0,   3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0,
    2,   0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0,   1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
    1,   0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,
    0,   2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0,
    3,   0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

static const uint8_t next_table[] = {
    0,    0,    0,    0x2,  0,    0x4,  0x4,  0x6,  0,    0x8,  0x8,  0x0a, 0x08, 0x0c, 0x0c, 0x0e,
    0,    0x10, 0x10, 0x12, 0x10, 0x14, 0x14, 0x16, 0x10, 0x18, 0x18, 0x1a, 0x18, 0x1c, 0x1c, 0x1e,
    0,    0x20, 0x20, 0x22, 0x20, 0x24, 0x24, 0x26, 0x20, 0x28, 0x28, 0x2a, 0x28, 0x2c, 0x2c, 0x2e,
    0x20, 0x30, 0x30, 0x32, 0x30, 0x34, 0x34, 0x36, 0x30, 0x38, 0x38, 0x3a, 0x38, 0x3c, 0x3c, 0x3e,
    0,    0x40, 0x40, 0x42, 0x40, 0x44, 0x44, 0x46, 0x40, 0x48, 0x48, 0x4a, 0x48, 0x4c, 0x4c, 0x4e,
    0x40, 0x50, 0x50, 0x52, 0x50, 0x54, 0x54, 0x56, 0x50, 0x58, 0x58, 0x5a, 0x58, 0x5c, 0x5c, 0x5e,
    0x40, 0x60, 0x60, 0x62, 0x60, 0x64, 0x64, 0x66, 0x60, 0x68, 0x68, 0x6a, 0x68, 0x6c, 0x6c, 0x6e,
    0x60, 0x70, 0x70, 0x72, 0x70, 0x74, 0x74, 0x76, 0x70, 0x78, 0x78, 0x7a, 0x78, 0x7c, 0x7c, 0x7e,
    0,    0x80, 0x80, 0x82, 0x80, 0x84, 0x84, 0x86, 0x80, 0x88, 0x88, 0x8a, 0x88, 0x8c, 0x8c, 0x8e,
    0x80, 0x90, 0x90, 0x92, 0x90, 0x94, 0x94, 0x96, 0x90, 0x98, 0x98, 0x9a, 0x98, 0x9c, 0x9c, 0x9e,
    0x80, 0xa0, 0xa0, 0xa2, 0xa0, 0xa4, 0xa4, 0xa6, 0xa0, 0xa8, 0xa8, 0xaa, 0xa8, 0xac, 0xac, 0xae,
    0xa0, 0xb0, 0xb0, 0xb2, 0xb0, 0xb4, 0xb4, 0xb6, 0xb0, 0xb8, 0xb8, 0xba, 0xb8, 0xbc, 0xbc, 0xbe,
    0x80, 0xc0, 0xc0, 0xc2, 0xc0, 0xc4, 0xc4, 0xc6, 0xc0, 0xc8, 0xc8, 0xca, 0xc8, 0xcc, 0xcc, 0xce,
    0xc0, 0xd0, 0xd0, 0xd2, 0xd0, 0xd4, 0xd4, 0xd6, 0xd0, 0xd8, 0xd8, 0xda, 0xd8, 0xdc, 0xdc, 0xde,
    0xc0, 0xe0, 0xe0, 0xe2, 0xe0, 0xe4, 0xe4, 0xe6, 0xe0, 0xe8, 0xe8, 0xea, 0xe8, 0xec, 0xec, 0xee,
    0xe0, 0xf0, 0xf0, 0xf2, 0xf0, 0xf4, 0xf4, 0xf6, 0xf0, 0xf8, 0xf8, 0xfa, 0xf8, 0xfc, 0xfc, 0xfe};

// See http://b/19318793 (#6) for a complete discussion.

/**
 * Sort Key array in ascending order using heap sort
 * algorithm.  Also sort Index array that is tied to
 * the key array.
 * @param n Number of elements to sort
 * @param ra     Key array [1..n]
 * @param rb     Index array [1..n]
 */
static void HeapSort(int n, int ra[], int rb[]) {
  int i, rra, rrb;
  int l, j, ir;

  l = (n >> 1) + 1;
  ir = n;
  for (;;) {
    if (l > 1) {
      rra = ra[--l];
      rrb = rb[l];
    } else {
      rra = ra[ir];
      rrb = rb[ir];
      ra[ir] = ra[1];
      rb[ir] = rb[1];
      if (--ir == 1) {
        ra[1] = rra;
        rb[1] = rrb;
        return;
      }
    }
    i = l;
    j = l << 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j + 1]) {
        ++j;
      }
      if (rra < ra[j]) {
        ra[i] = ra[j];
        rb[i] = rb[j];
        j += (i = j);
      } else {
        j = ir + 1;
      }
    }
    ra[i] = rra;
    rb[i] = rrb;
  }
}

// Encapsulation of the intermediate data and computations made by the class
// pruner. The class pruner implements a simple linear classifier on binary
// features by heavily quantizing the feature space, and applying
// NUM_BITS_PER_CLASS (2)-bit weights to the features. Lack of resolution in
// weights is compensated by a non-constant bias that is dependent on the
// number of features present.
class ClassPruner {
public:
  ClassPruner(int max_classes) {
    // The unrolled loop in ComputeScores means that the array sizes need to
    // be rounded up so that the array is big enough to accommodate the extra
    // entries accessed by the unrolling. Each pruner word is of sized
    // BITS_PER_WERD and each entry is NUM_BITS_PER_CLASS, so there are
    // BITS_PER_WERD / NUM_BITS_PER_CLASS entries.
    // See ComputeScores.
    max_classes_ = max_classes;
    rounded_classes_ =
        RoundUp(max_classes, WERDS_PER_CP_VECTOR * BITS_PER_WERD / NUM_BITS_PER_CLASS);
    class_count_ = new int[rounded_classes_];
    norm_count_ = new int[rounded_classes_];
    sort_key_ = new int[rounded_classes_ + 1];
    sort_index_ = new int[rounded_classes_ + 1];
    for (int i = 0; i < rounded_classes_; i++) {
      class_count_[i] = 0;
    }
    pruning_threshold_ = 0;
    num_features_ = 0;
    num_classes_ = 0;
  }

  ~ClassPruner() {
    delete[] class_count_;
    delete[] norm_count_;
    delete[] sort_key_;
    delete[] sort_index_;
  }

  /// Computes the scores for every class in the character set, by summing the
  /// weights for each feature and stores the sums internally in class_count_.
  void ComputeScores(const INT_TEMPLATES_STRUCT *int_templates, int num_features,
                     const INT_FEATURE_STRUCT *features) {
    num_features_ = num_features;
    auto num_pruners = int_templates->NumClassPruners;
    for (int f = 0; f < num_features; ++f) {
      const INT_FEATURE_STRUCT *feature = &features[f];
      // Quantize the feature to NUM_CP_BUCKETS*NUM_CP_BUCKETS*NUM_CP_BUCKETS.
      int x = feature->X * NUM_CP_BUCKETS >> 8;
      int y = feature->Y * NUM_CP_BUCKETS >> 8;
      int theta = feature->Theta * NUM_CP_BUCKETS >> 8;
      int class_id = 0;
      // Each CLASS_PRUNER_STRUCT only covers CLASSES_PER_CP(32) classes, so
      // we need a collection of them, indexed by pruner_set.
      for (unsigned pruner_set = 0; pruner_set < num_pruners; ++pruner_set) {
        // Look up quantized feature in a 3-D array, an array of weights for
        // each class.
        const uint32_t *pruner_word_ptr = int_templates->ClassPruners[pruner_set]->p[x][y][theta];
        for (int word = 0; word < WERDS_PER_CP_VECTOR; ++word) {
          uint32_t pruner_word = *pruner_word_ptr++;
          // This inner loop is unrolled to speed up the ClassPruner.
          // Currently gcc would not unroll it unless it is set to O3
          // level of optimization or -funroll-loops is specified.
          /*
uint32_t class_mask = (1 << NUM_BITS_PER_CLASS) - 1;
for (int bit = 0; bit < BITS_PER_WERD/NUM_BITS_PER_CLASS; bit++) {
  class_count_[class_id++] += pruner_word & class_mask;
  pruner_word >>= NUM_BITS_PER_CLASS;
}
*/
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
          pruner_word >>= NUM_BITS_PER_CLASS;
          class_count_[class_id++] += pruner_word & CLASS_PRUNER_CLASS_MASK;
        }
      }
    }
  }

  /// Adjusts the scores according to the number of expected features. Used
  /// in lieu of a constant bias, this penalizes classes that expect more
  /// features than there are present. Thus an actual c will score higher for c
  /// than e, even though almost all the features match e as well as c, because
  /// e expects more features to be present.
  void AdjustForExpectedNumFeatures(const uint16_t *expected_num_features, int cutoff_strength) {
    for (int class_id = 0; class_id < max_classes_; ++class_id) {
      if (num_features_ < expected_num_features[class_id]) {
        int deficit = expected_num_features[class_id] - num_features_;
        class_count_[class_id] -=
            class_count_[class_id] * deficit / (num_features_ * cutoff_strength + deficit);
      }
    }
  }

  /// Zeros the scores for classes disabled in the unicharset.
  /// Implements the black-list to recognize a subset of the character set.
  void DisableDisabledClasses(const UNICHARSET &unicharset) {
    for (int class_id = 0; class_id < max_classes_; ++class_id) {
      if (!unicharset.get_enabled(class_id)) {
        class_count_[class_id] = 0; // This char is disabled!
      }
    }
  }

  /** Zeros the scores of fragments. */
  void DisableFragments(const UNICHARSET &unicharset) {
    for (int class_id = 0; class_id < max_classes_; ++class_id) {
      // Do not include character fragments in the class pruner
      // results if disable_character_fragments is true.
      if (unicharset.get_fragment(class_id)) {
        class_count_[class_id] = 0;
      }
    }
  }

  /// Normalizes the counts for xheight, putting the normalized result in
  /// norm_count_. Applies a simple subtractive penalty for incorrect vertical
  /// position provided by the normalization_factors array, indexed by
  /// character class, and scaled by the norm_multiplier.
  void NormalizeForXheight(int norm_multiplier, const uint8_t *normalization_factors) {
    for (int class_id = 0; class_id < max_classes_; class_id++) {
      norm_count_[class_id] =
          class_count_[class_id] - ((norm_multiplier * normalization_factors[class_id]) >> 8);
    }
  }

  /** The nop normalization copies the class_count_ array to norm_count_. */
  void NoNormalization() {
    for (int class_id = 0; class_id < max_classes_; class_id++) {
      norm_count_[class_id] = class_count_[class_id];
    }
  }

  /// Prunes the classes using &lt;the maximum count> * pruning_factor/256 as a
  /// threshold for keeping classes. If max_of_non_fragments, then ignore
  /// fragments in computing the maximum count.
  void PruneAndSort(int pruning_factor, int keep_this, bool max_of_non_fragments,
                    const UNICHARSET &unicharset) {
    int max_count = 0;
    for (int c = 0; c < max_classes_; ++c) {
      if (norm_count_[c] > max_count &&
          // This additional check is added in order to ensure that
          // the classifier will return at least one non-fragmented
          // character match.
          // TODO(daria): verify that this helps accuracy and does not
          // hurt performance.
          (!max_of_non_fragments || !unicharset.get_fragment(c))) {
        max_count = norm_count_[c];
      }
    }
    // Prune Classes.
    pruning_threshold_ = (max_count * pruning_factor) >> 8;
    // Select Classes.
    if (pruning_threshold_ < 1) {
      pruning_threshold_ = 1;
    }
    num_classes_ = 0;
    for (int class_id = 0; class_id < max_classes_; class_id++) {
      if (norm_count_[class_id] >= pruning_threshold_ || class_id == keep_this) {
        ++num_classes_;
        sort_index_[num_classes_] = class_id;
        sort_key_[num_classes_] = norm_count_[class_id];
      }
    }

    // Sort Classes using Heapsort Algorithm.
    if (num_classes_ > 1) {
      HeapSort(num_classes_, sort_key_, sort_index_);
    }
  }

  /** Prints debug info on the class pruner matches for the pruned classes only.
   */
  void DebugMatch(const Classify &classify, const INT_TEMPLATES_STRUCT *int_templates,
                  const INT_FEATURE_STRUCT *features) const {
    int num_pruners = int_templates->NumClassPruners;
    int max_num_classes = int_templates->NumClasses;
    for (int f = 0; f < num_features_; ++f) {
      const INT_FEATURE_STRUCT *feature = &features[f];
      tprintf("F=%3d(%d,%d,%d),", f, feature->X, feature->Y, feature->Theta);
      // Quantize the feature to NUM_CP_BUCKETS*NUM_CP_BUCKETS*NUM_CP_BUCKETS.
      int x = feature->X * NUM_CP_BUCKETS >> 8;
      int y = feature->Y * NUM_CP_BUCKETS >> 8;
      int theta = feature->Theta * NUM_CP_BUCKETS >> 8;
      int class_id = 0;
      for (int pruner_set = 0; pruner_set < num_pruners; ++pruner_set) {
        // Look up quantized feature in a 3-D array, an array of weights for
        // each class.
        const uint32_t *pruner_word_ptr = int_templates->ClassPruners[pruner_set]->p[x][y][theta];
        for (int word = 0; word < WERDS_PER_CP_VECTOR; ++word) {
          uint32_t pruner_word = *pruner_word_ptr++;
          for (int word_class = 0; word_class < 16 && class_id < max_num_classes;
               ++word_class, ++class_id) {
            if (norm_count_[class_id] >= pruning_threshold_) {
              tprintf(" %s=%d,", classify.ClassIDToDebugStr(int_templates, class_id, 0).c_str(),
                      pruner_word & CLASS_PRUNER_CLASS_MASK);
            }
            pruner_word >>= NUM_BITS_PER_CLASS;
          }
        }
        tprintf("\n");
      }
    }
  }

  /** Prints a summary of the pruner result. */
  void SummarizeResult(const Classify &classify, const INT_TEMPLATES_STRUCT *int_templates,
                       const uint16_t *expected_num_features, int norm_multiplier,
                       const uint8_t *normalization_factors) const {
    tprintf("CP:%d classes, %d features:\n", num_classes_, num_features_);
    for (int i = 0; i < num_classes_; ++i) {
      int class_id = sort_index_[num_classes_ - i];
      std::string class_string = classify.ClassIDToDebugStr(int_templates, class_id, 0);
      tprintf(
          "%s:Initial=%d, E=%d, Xht-adj=%d, N=%d, Rat=%.2f\n", class_string.c_str(),
          class_count_[class_id], expected_num_features[class_id],
          (norm_multiplier * normalization_factors[class_id]) >> 8, sort_key_[num_classes_ - i],
          100.0 - 100.0 * sort_key_[num_classes_ - i] / (CLASS_PRUNER_CLASS_MASK * num_features_));
    }
  }

  /// Copies the pruned, sorted classes into the output results and returns
  /// the number of classes.
  int SetupResults(std::vector<CP_RESULT_STRUCT> *results) const {
    results->clear();
    results->resize(num_classes_);
    for (int c = 0; c < num_classes_; ++c) {
      (*results)[c].Class = sort_index_[num_classes_ - c];
      (*results)[c].Rating =
          1.0f - sort_key_[num_classes_ - c] /
                     (static_cast<float>(CLASS_PRUNER_CLASS_MASK) * num_features_);
    }
    return num_classes_;
  }

private:
  /** Array[rounded_classes_] of initial counts for each class. */
  int *class_count_;
  /// Array[rounded_classes_] of modified counts for each class after
  /// normalizing for expected number of features, disabled classes, fragments,
  /// and xheights.
  int *norm_count_;
  /** Array[rounded_classes_ +1] of pruned counts that gets sorted */
  int *sort_key_;
  /** Array[rounded_classes_ +1] of classes corresponding to sort_key_. */
  int *sort_index_;
  /** Number of classes in this class pruner. */
  int max_classes_;
  /** Rounded up number of classes used for array sizes. */
  int rounded_classes_;
  /** Threshold count applied to prune classes. */
  int pruning_threshold_;
  /** The number of features used to compute the scores. */
  int num_features_;
  /** Final number of pruned classes. */
  int num_classes_;
};

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/**
 * Runs the class pruner from int_templates on the given features, returning
 * the number of classes output in results.
 * @param int_templates          Class pruner tables
 * @param num_features           Number of features in blob
 * @param features               Array of features
 * @param normalization_factors  Array of fudge factors from blob
 *                               normalization process (by CLASS_INDEX)
 * @param expected_num_features  Array of expected number of features
 *                               for each class (by CLASS_INDEX)
 * @param results                Sorted Array of pruned classes. Must be an
 *                               array of size at least
 *                               int_templates->NumClasses.
 * @param keep_this
 */
int Classify::PruneClasses(const INT_TEMPLATES_STRUCT *int_templates, int num_features,
                           int keep_this, const INT_FEATURE_STRUCT *features,
                           const uint8_t *normalization_factors,
                           const uint16_t *expected_num_features,
                           std::vector<CP_RESULT_STRUCT> *results) {
  ClassPruner pruner(int_templates->NumClasses);
  // Compute initial match scores for all classes.
  pruner.ComputeScores(int_templates, num_features, features);
  // Adjust match scores for number of expected features.
  pruner.AdjustForExpectedNumFeatures(expected_num_features, classify_cp_cutoff_strength);
  // Apply disabled classes in unicharset - only works without a shape_table.
  if (shape_table_ == nullptr) {
    pruner.DisableDisabledClasses(unicharset);
  }
  // If fragments are disabled, remove them, also only without a shape table.
  if (disable_character_fragments && shape_table_ == nullptr) {
    pruner.DisableFragments(unicharset);
  }

  // If we have good x-heights, apply the given normalization factors.
  if (normalization_factors != nullptr) {
    pruner.NormalizeForXheight(classify_class_pruner_multiplier, normalization_factors);
  } else {
    pruner.NoNormalization();
  }
  // Do the actual pruning and sort the short-list.
  pruner.PruneAndSort(classify_class_pruner_threshold, keep_this, shape_table_ == nullptr,
                      unicharset);

  if (classify_debug_level > 2) {
    pruner.DebugMatch(*this, int_templates, features);
  }
  if (classify_debug_level > 1) {
    pruner.SummarizeResult(*this, int_templates, expected_num_features,
                           classify_class_pruner_multiplier, normalization_factors);
  }
  // Convert to the expected output format.
  return pruner.SetupResults(results);
}

/**
 * IntegerMatcher returns the best configuration and rating
 * for a single class.  The class matched against is determined
 * by the uniqueness of the ClassTemplate parameter.  The
 * best rating and its associated configuration are returned.
 *
 * Globals:
 * - local_matcher_multiplier_ Normalization factor multiplier
 * param ClassTemplate Prototypes & tables for a class
 * param NumFeatures Number of features in blob
 * param Features Array of features
 * param NormalizationFactor Fudge factor from blob normalization process
 * param Result Class rating & configuration: (0.0 -> 1.0), 0=bad, 1=good
 * param Debug Debugger flag: 1=debugger on
 */
void IntegerMatcher::Match(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask, BIT_VECTOR ConfigMask,
                           int16_t NumFeatures, const INT_FEATURE_STRUCT *Features,
                           UnicharRating *Result, int AdaptFeatureThreshold, int Debug,
                           bool SeparateDebugWindows) {
  auto *tables = new ScratchEvidence();
  int Feature;

  if (MatchDebuggingOn(Debug)) {
    tprintf("Integer Matcher -------------------------------------------\n");
  }

  tables->Clear(ClassTemplate);
  Result->feature_misses = 0;

  for (Feature = 0; Feature < NumFeatures; Feature++) {
    int csum = UpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask, Feature,
                                      &Features[Feature], tables, Debug);
    // Count features that were missed over all configs.
    if (csum == 0) {
      ++Result->feature_misses;
    }
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn(Debug) || PrintMatchSummaryOn(Debug)) {
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables, NumFeatures, Debug);
  }

  if (DisplayProtoMatchesOn(Debug)) {
    DisplayProtoDebugInfo(ClassTemplate, ConfigMask, *tables, SeparateDebugWindows);
  }

  if (DisplayFeatureMatchesOn(Debug)) {
    DisplayFeatureDebugInfo(ClassTemplate, ProtoMask, ConfigMask, NumFeatures, Features,
                            AdaptFeatureThreshold, Debug, SeparateDebugWindows);
  }
#endif

  tables->UpdateSumOfProtoEvidences(ClassTemplate, ConfigMask);
  tables->NormalizeSums(ClassTemplate, NumFeatures);

  FindBestMatch(ClassTemplate, *tables, Result);

#ifndef GRAPHICS_DISABLED
  if (PrintMatchSummaryOn(Debug)) {
    Result->Print();
  }

  if (MatchDebuggingOn(Debug)) {
    tprintf("Match Complete --------------------------------------------\n");
  }
#endif

  delete tables;
}

/**
 * FindGoodProtos finds all protos whose normalized proto-evidence
 * exceed AdaptProtoThreshold.  The list is ordered by increasing
 * proto id number.
 *
 * Globals:
 * - local_matcher_multiplier_    Normalization factor multiplier
 * param ClassTemplate Prototypes & tables for a class
 * param ProtoMask AND Mask for proto word
 * param ConfigMask AND Mask for config word
 * param NumFeatures Number of features in blob
 * param Features Array of features
 * param ProtoArray Array of good protos
 * param AdaptProtoThreshold Threshold for good protos
 * param Debug Debugger flag: 1=debugger on
 * @return Number of good protos in ProtoArray.
 */
int IntegerMatcher::FindGoodProtos(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask,
                                   BIT_VECTOR ConfigMask, int16_t NumFeatures,
                                   INT_FEATURE_ARRAY Features, PROTO_ID *ProtoArray,
                                   int AdaptProtoThreshold, int Debug) {
  auto *tables = new ScratchEvidence();
  int NumGoodProtos = 0;

  /* DEBUG opening heading */
  if (MatchDebuggingOn(Debug)) {
    tprintf("Find Good Protos -------------------------------------------\n");
  }

  tables->Clear(ClassTemplate);

  for (int Feature = 0; Feature < NumFeatures; Feature++) {
    UpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask, Feature, &(Features[Feature]),
                           tables, Debug);
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn(Debug) || PrintMatchSummaryOn(Debug)) {
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables, NumFeatures, Debug);
  }
#endif

  /* Average Proto Evidences & Find Good Protos */
  for (int proto = 0; proto < ClassTemplate->NumProtos; proto++) {
    /* Compute Average for Actual Proto */
    int Temp = 0;
    for (uint8_t i = 0; i < MAX_PROTO_INDEX && i < ClassTemplate->ProtoLengths[proto]; i++) {
      Temp += tables->proto_evidence_[proto][i];
    }

    Temp /= ClassTemplate->ProtoLengths[proto];

    /* Find Good Protos */
    if (Temp >= AdaptProtoThreshold) {
      *ProtoArray = proto;
      ProtoArray++;
      NumGoodProtos++;
    }
  }

  if (MatchDebuggingOn(Debug)) {
    tprintf("Match Complete --------------------------------------------\n");
  }
  delete tables;

  return NumGoodProtos;
}

/**
 * FindBadFeatures finds all features with maximum feature-evidence <
 * AdaptFeatureThresh. The list is ordered by increasing feature number.
 * @param ClassTemplate Prototypes & tables for a class
 * @param ProtoMask AND Mask for proto word
 * @param ConfigMask AND Mask for config word
 * @param NumFeatures Number of features in blob
 * @param Features Array of features
 * @param FeatureArray Array of bad features
 * @param AdaptFeatureThreshold Threshold for bad features
 * @param Debug Debugger flag: 1=debugger on
 * @return Number of bad features in FeatureArray.
 */
int IntegerMatcher::FindBadFeatures(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask,
                                    BIT_VECTOR ConfigMask, int16_t NumFeatures,
                                    INT_FEATURE_ARRAY Features, FEATURE_ID *FeatureArray,
                                    int AdaptFeatureThreshold, int Debug) {
  auto *tables = new ScratchEvidence();
  int NumBadFeatures = 0;

  /* DEBUG opening heading */
  if (MatchDebuggingOn(Debug)) {
    tprintf("Find Bad Features -------------------------------------------\n");
  }

  tables->Clear(ClassTemplate);

  for (int Feature = 0; Feature < NumFeatures; Feature++) {
    UpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask, Feature, &Features[Feature],
                           tables, Debug);

    /* Find Best Evidence for Current Feature */
    int best = 0;
    assert(ClassTemplate->NumConfigs < MAX_NUM_CONFIGS);
    for (int i = 0; i < MAX_NUM_CONFIGS && i < ClassTemplate->NumConfigs; i++) {
      if (tables->feature_evidence_[i] > best) {
        best = tables->feature_evidence_[i];
      }
    }

    /* Find Bad Features */
    if (best < AdaptFeatureThreshold) {
      *FeatureArray = Feature;
      FeatureArray++;
      NumBadFeatures++;
    }
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn(Debug) || PrintMatchSummaryOn(Debug)) {
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables, NumFeatures, Debug);
  }
#endif

  if (MatchDebuggingOn(Debug)) {
    tprintf("Match Complete --------------------------------------------\n");
  }

  delete tables;
  return NumBadFeatures;
}

IntegerMatcher::IntegerMatcher(tesseract::IntParam *classify_debug_level)
    : classify_debug_level_(classify_debug_level) {
  /* Initialize table for evidence to similarity lookup */
  for (int i = 0; i < SE_TABLE_SIZE; i++) {
    uint32_t IntSimilarity = i << (27 - SE_TABLE_BITS);
    double Similarity = (static_cast<double>(IntSimilarity)) / 65536.0 / 65536.0;
    double evidence = Similarity / kSimilarityCenter;
    evidence = 255.0 / (evidence * evidence + 1.0);

    if (kSEExponentialMultiplier > 0.0) {
      double scale =
          1.0 - std::exp(-kSEExponentialMultiplier) *
                    exp(kSEExponentialMultiplier * (static_cast<double>(i) / SE_TABLE_SIZE));
      evidence *= ClipToRange(scale, 0.0, 1.0);
    }

    similarity_evidence_table_[i] = static_cast<uint8_t>(evidence + 0.5);
  }

  /* Initialize evidence computation variables */
  evidence_table_mask_ = ((1 << kEvidenceTableBits) - 1) << (9 - kEvidenceTableBits);
  mult_trunc_shift_bits_ = (14 - kIntEvidenceTruncBits);
  table_trunc_shift_bits_ = (27 - SE_TABLE_BITS - (mult_trunc_shift_bits_ << 1));
  evidence_mult_mask_ = ((1 << kIntEvidenceTruncBits) - 1);
}

/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/
void ScratchEvidence::Clear(const INT_CLASS_STRUCT *class_template) {
  memset(sum_feature_evidence_, 0, class_template->NumConfigs * sizeof(sum_feature_evidence_[0]));
  memset(proto_evidence_, 0, class_template->NumProtos * sizeof(proto_evidence_[0]));
}

void ScratchEvidence::ClearFeatureEvidence(const INT_CLASS_STRUCT *class_template) {
  memset(feature_evidence_, 0, class_template->NumConfigs * sizeof(feature_evidence_[0]));
}

/**
 * Print debugging information for Configurations
 */
static void IMDebugConfiguration(int FeatureNum, uint16_t ActualProtoNum, uint8_t Evidence,
                                 uint32_t ConfigWord) {
  tprintf("F = %3d, P = %3d, E = %3d, Configs = ", FeatureNum, static_cast<int>(ActualProtoNum),
          static_cast<int>(Evidence));
  while (ConfigWord) {
    if (ConfigWord & 1) {
      tprintf("1");
    } else {
      tprintf("0");
    }
    ConfigWord >>= 1;
  }
  tprintf("\n");
}

/**
 * Print debugging information for Configurations
 */
static void IMDebugConfigurationSum(int FeatureNum, uint8_t *FeatureEvidence, int32_t ConfigCount) {
  tprintf("F=%3d, C=", FeatureNum);
  for (int ConfigNum = 0; ConfigNum < ConfigCount; ConfigNum++) {
    tprintf("%4d", FeatureEvidence[ConfigNum]);
  }
  tprintf("\n");
}

/**
 * For the given feature: prune protos, compute evidence,
 * update Feature Evidence, Proto Evidence, and Sum of Feature
 * Evidence tables.
 * @param ClassTemplate Prototypes & tables for a class
 * @param FeatureNum Current feature number (for DEBUG only)
 * @param Feature Pointer to a feature struct
 * @param tables Evidence tables
 * @param Debug Debugger flag: 1=debugger on
 * @return sum of feature evidence tables
 */
int IntegerMatcher::UpdateTablesForFeature(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask,
                                           BIT_VECTOR ConfigMask, int FeatureNum,
                                           const INT_FEATURE_STRUCT *Feature,
                                           ScratchEvidence *tables, int Debug) {
  uint32_t ConfigWord;
  uint32_t ProtoWord;
  uint32_t ProtoNum;
  uint32_t ActualProtoNum;
  uint8_t proto_byte;
  int32_t proto_word_offset;
  int32_t proto_offset;
  PROTO_SET_STRUCT *ProtoSet;
  uint32_t *ProtoPrunerPtr;
  INT_PROTO_STRUCT *Proto;
  int ProtoSetIndex;
  uint8_t Evidence;
  uint32_t XFeatureAddress;
  uint32_t YFeatureAddress;
  uint32_t ThetaFeatureAddress;

  tables->ClearFeatureEvidence(ClassTemplate);

  /* Precompute Feature Address offset for Proto Pruning */
  XFeatureAddress = ((Feature->X >> 2) << 1);
  YFeatureAddress = (NUM_PP_BUCKETS << 1) + ((Feature->Y >> 2) << 1);
  ThetaFeatureAddress = (NUM_PP_BUCKETS << 2) + ((Feature->Theta >> 2) << 1);

  for (ProtoSetIndex = 0, ActualProtoNum = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
       ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ProtoPrunerPtr = reinterpret_cast<uint32_t *>((*ProtoSet).ProtoPruner);
    for (ProtoNum = 0; ProtoNum < PROTOS_PER_PROTO_SET; ProtoNum += (PROTOS_PER_PROTO_SET >> 1),
        ActualProtoNum += (PROTOS_PER_PROTO_SET >> 1), ProtoMask++, ProtoPrunerPtr++) {
      /* Prune Protos of current Proto Set */
      ProtoWord = *(ProtoPrunerPtr + XFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + YFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + ThetaFeatureAddress);
      ProtoWord &= *ProtoMask;

      if (ProtoWord != 0) {
        proto_byte = ProtoWord & 0xff;
        ProtoWord >>= 8;
        proto_word_offset = 0;
        while (ProtoWord != 0 || proto_byte != 0) {
          while (proto_byte == 0) {
            proto_byte = ProtoWord & 0xff;
            ProtoWord >>= 8;
            proto_word_offset += 8;
          }
          proto_offset = offset_table[proto_byte] + proto_word_offset;
          proto_byte = next_table[proto_byte];
          Proto = &(ProtoSet->Protos[ProtoNum + proto_offset]);
          ConfigWord = Proto->Configs[0];
          int32_t A3 = (((Proto->A * (Feature->X - 128)) * 2) - (Proto->B * (Feature->Y - 128)) +
                        (Proto->C * 512));
          int32_t M3 = ((static_cast<int8_t>(Feature->Theta - Proto->Angle)) * kIntThetaFudge) * 2;

          if (A3 < 0) {
            A3 = ~A3;
          }
          if (M3 < 0) {
            M3 = ~M3;
          }
          A3 >>= mult_trunc_shift_bits_;
          M3 >>= mult_trunc_shift_bits_;
          if (static_cast<uint32_t>(A3) > evidence_mult_mask_) {
            A3 = evidence_mult_mask_;
          }
          if (static_cast<uint32_t>(M3) > evidence_mult_mask_) {
            M3 = evidence_mult_mask_;
          }

          uint32_t A4 = (A3 * A3) + (M3 * M3);
          A4 >>= table_trunc_shift_bits_;
          if (A4 > evidence_table_mask_) {
            Evidence = 0;
          } else {
            Evidence = similarity_evidence_table_[A4];
          }

          if (PrintFeatureMatchesOn(Debug)) {
            IMDebugConfiguration(FeatureNum, ActualProtoNum + proto_offset, Evidence, ConfigWord);
          }

          ConfigWord &= *ConfigMask;

          uint8_t feature_evidence_index = 0;
          uint8_t config_byte = 0;
          while (ConfigWord != 0 || config_byte != 0) {
            while (config_byte == 0) {
              config_byte = ConfigWord & 0xff;
              ConfigWord >>= 8;
              feature_evidence_index += 8;
            }
            const uint8_t config_offset = offset_table[config_byte] + feature_evidence_index - 8;
            config_byte = next_table[config_byte];
            if (Evidence > tables->feature_evidence_[config_offset]) {
              tables->feature_evidence_[config_offset] = Evidence;
            }
          }

          uint8_t ProtoIndex = ClassTemplate->ProtoLengths[ActualProtoNum + proto_offset];
          if (ProtoIndex > MAX_PROTO_INDEX) {
            // Avoid buffer overflow.
            // TODO: A better fix is still open.
            ProtoIndex = MAX_PROTO_INDEX;
          }
          uint8_t *UINT8Pointer = &(tables->proto_evidence_[ActualProtoNum + proto_offset][0]);
          for (; Evidence > 0 && ProtoIndex > 0; ProtoIndex--, UINT8Pointer++) {
            if (Evidence > *UINT8Pointer) {
              uint8_t Temp = *UINT8Pointer;
              *UINT8Pointer = Evidence;
              Evidence = Temp;
            }
          }
        }
      }
    }
  }

  if (PrintFeatureMatchesOn(Debug)) {
    IMDebugConfigurationSum(FeatureNum, tables->feature_evidence_, ClassTemplate->NumConfigs);
  }

  int *IntPointer = tables->sum_feature_evidence_;
  uint8_t *UINT8Pointer = tables->feature_evidence_;
  int SumOverConfigs = 0;
  for (int ConfigNum = ClassTemplate->NumConfigs; ConfigNum > 0; ConfigNum--) {
    int evidence = *UINT8Pointer++;
    SumOverConfigs += evidence;
    *IntPointer++ += evidence;
  }
  return SumOverConfigs;
}

/**
 * Print debugging information for Configurations
 */
#ifndef GRAPHICS_DISABLED
void IntegerMatcher::DebugFeatureProtoError(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask,
                                            BIT_VECTOR ConfigMask, const ScratchEvidence &tables,
                                            int16_t NumFeatures, int Debug) {
  float ProtoConfigs[MAX_NUM_CONFIGS];
  int ConfigNum;
  uint32_t ConfigWord;
  int ProtoSetIndex;
  uint16_t ProtoNum;
  uint8_t ProtoWordNum;
  PROTO_SET_STRUCT *ProtoSet;
  uint16_t ActualProtoNum;

  if (PrintMatchSummaryOn(Debug)) {
    tprintf("Configuration Mask:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      tprintf("%1d", (((*ConfigMask) >> ConfigNum) & 1));
    }
    tprintf("\n");

    tprintf("Feature Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      tprintf(" %5.1f", 100.0 * (1.0 - static_cast<float>(tables.sum_feature_evidence_[ConfigNum]) /
                                           NumFeatures / 256.0));
    }
    tprintf("\n\n\n");
  }

  if (PrintMatchSummaryOn(Debug)) {
    tprintf("Proto Mask:\n");
    for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets; ProtoSetIndex++) {
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoWordNum = 0; ProtoWordNum < 2; ProtoWordNum++, ProtoMask++) {
        ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
        for (ProtoNum = 0; ((ProtoNum < (PROTOS_PER_PROTO_SET >> 1)) &&
                            (ActualProtoNum < ClassTemplate->NumProtos));
             ProtoNum++, ActualProtoNum++) {
          tprintf("%1d", (((*ProtoMask) >> ProtoNum) & 1));
        }
        tprintf("\n");
      }
    }
    tprintf("\n");
  }

  for (int i = 0; i < ClassTemplate->NumConfigs; i++) {
    ProtoConfigs[i] = 0;
  }

  if (PrintProtoMatchesOn(Debug)) {
    tprintf("Proto Evidence:\n");
    for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets; ProtoSetIndex++) {
      ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoNum = 0;
           ((ProtoNum < PROTOS_PER_PROTO_SET) && (ActualProtoNum < ClassTemplate->NumProtos));
           ProtoNum++, ActualProtoNum++) {
        tprintf("P %3d =", ActualProtoNum);
        int temp = 0;
        for (uint8_t j = 0; j < ClassTemplate->ProtoLengths[ActualProtoNum]; j++) {
          uint8_t data = tables.proto_evidence_[ActualProtoNum][j];
          tprintf(" %d", data);
          temp += data;
        }

        tprintf(" = %6.4f%%\n", temp / 256.0 / ClassTemplate->ProtoLengths[ActualProtoNum]);

        ConfigWord = ProtoSet->Protos[ProtoNum].Configs[0];
        ConfigNum = 0;
        while (ConfigWord) {
          tprintf("%5d", ConfigWord & 1 ? temp : 0);
          if (ConfigWord & 1) {
            ProtoConfigs[ConfigNum] += temp;
          }
          ConfigNum++;
          ConfigWord >>= 1;
        }
        tprintf("\n");
      }
    }
  }

  if (PrintMatchSummaryOn(Debug)) {
    tprintf("Proto Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      tprintf(" %5.1f", 100.0 * (1.0 - ProtoConfigs[ConfigNum] /
                                           ClassTemplate->ConfigLengths[ConfigNum] / 256.0));
    }
    tprintf("\n\n");
  }

  if (PrintProtoMatchesOn(Debug)) {
    tprintf("Proto Sum for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      tprintf(" %4.1f", ProtoConfigs[ConfigNum] / 256.0);
    }
    tprintf("\n\n");

    tprintf("Proto Length for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      tprintf(" %4.1f", static_cast<float>(ClassTemplate->ConfigLengths[ConfigNum]));
    }
    tprintf("\n\n");
  }
}

void IntegerMatcher::DisplayProtoDebugInfo(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ConfigMask,
                                           const ScratchEvidence &tables,
                                           bool SeparateDebugWindows) {
  uint16_t ProtoNum;
  uint16_t ActualProtoNum;
  PROTO_SET_STRUCT *ProtoSet;
  int ProtoSetIndex;

  InitIntMatchWindowIfReqd();
  if (SeparateDebugWindows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }

  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets; ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = ProtoSetIndex * PROTOS_PER_PROTO_SET;
    for (ProtoNum = 0;
         ((ProtoNum < PROTOS_PER_PROTO_SET) && (ActualProtoNum < ClassTemplate->NumProtos));
         ProtoNum++, ActualProtoNum++) {
      /* Compute Average for Actual Proto */
      int temp = 0;
      for (uint8_t i = 0; i < ClassTemplate->ProtoLengths[ActualProtoNum]; i++) {
        temp += tables.proto_evidence_[ActualProtoNum][i];
      }

      temp /= ClassTemplate->ProtoLengths[ActualProtoNum];

      if ((ProtoSet->Protos[ProtoNum]).Configs[0] & (*ConfigMask)) {
        DisplayIntProto(ClassTemplate, ActualProtoNum, temp / 255.0);
      }
    }
  }
}

void IntegerMatcher::DisplayFeatureDebugInfo(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ProtoMask,
                                             BIT_VECTOR ConfigMask, int16_t NumFeatures,
                                             const INT_FEATURE_STRUCT *Features,
                                             int AdaptFeatureThreshold, int Debug,
                                             bool SeparateDebugWindows) {
  auto *tables = new ScratchEvidence();

  tables->Clear(ClassTemplate);

  InitIntMatchWindowIfReqd();
  if (SeparateDebugWindows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }

  for (int Feature = 0; Feature < NumFeatures; Feature++) {
    UpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask, Feature, &Features[Feature],
                           tables, 0);

    /* Find Best Evidence for Current Feature */
    int best = 0;
    assert(ClassTemplate->NumConfigs < MAX_NUM_CONFIGS);
    for (int i = 0; i < MAX_NUM_CONFIGS && i < ClassTemplate->NumConfigs; i++) {
      if (tables->feature_evidence_[i] > best) {
        best = tables->feature_evidence_[i];
      }
    }

    /* Update display for current feature */
    if (ClipMatchEvidenceOn(Debug)) {
      if (best < AdaptFeatureThreshold) {
        DisplayIntFeature(&Features[Feature], 0.0);
      } else {
        DisplayIntFeature(&Features[Feature], 1.0);
      }
    } else {
      DisplayIntFeature(&Features[Feature], best / 255.0);
    }
  }

  delete tables;
}
#endif

/**
 * Add sum of Proto Evidences into Sum Of Feature Evidence Array
 */
void ScratchEvidence::UpdateSumOfProtoEvidences(INT_CLASS_STRUCT *ClassTemplate, BIT_VECTOR ConfigMask) {
  int *IntPointer;
  uint32_t ConfigWord;
  int ProtoSetIndex;
  uint16_t ProtoNum;
  PROTO_SET_STRUCT *ProtoSet;
  int NumProtos;
  uint16_t ActualProtoNum;

  NumProtos = ClassTemplate->NumProtos;

  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets; ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0; ((ProtoNum < PROTOS_PER_PROTO_SET) && (ActualProtoNum < NumProtos));
         ProtoNum++, ActualProtoNum++) {
      int temp = 0;
      for (uint8_t i = 0; i < MAX_PROTO_INDEX && i < ClassTemplate->ProtoLengths[ActualProtoNum];
           i++) {
        temp += proto_evidence_[ActualProtoNum][i];
      }

      ConfigWord = ProtoSet->Protos[ProtoNum].Configs[0];
      ConfigWord &= *ConfigMask;
      IntPointer = sum_feature_evidence_;
      while (ConfigWord) {
        if (ConfigWord & 1) {
          *IntPointer += temp;
        }
        IntPointer++;
        ConfigWord >>= 1;
      }
    }
  }
}

/**
 * Normalize Sum of Proto and Feature Evidence by dividing by the sum of
 * the Feature Lengths and the Proto Lengths for each configuration.
 */
void ScratchEvidence::NormalizeSums(INT_CLASS_STRUCT *ClassTemplate, int16_t NumFeatures) {
  // ClassTemplate->NumConfigs can become larger than MAX_NUM_CONFIGS.
  for (int i = 0; i < MAX_NUM_CONFIGS && i < ClassTemplate->NumConfigs; i++) {
    sum_feature_evidence_[i] =
        (sum_feature_evidence_[i] << 8) / (NumFeatures + ClassTemplate->ConfigLengths[i]);
  }
}

/**
 * Find the best match for the current class and update the Result
 * with the configuration and match rating.
 * @return The best normalized sum of evidences
 */
int IntegerMatcher::FindBestMatch(INT_CLASS_STRUCT *class_template, const ScratchEvidence &tables,
                                  UnicharRating *result) {
  int best_match = 0;
  result->config = 0;
  result->fonts.clear();
  result->fonts.reserve(class_template->NumConfigs);

  // Find best match.
  // ClassTemplate->NumConfigs can become larger than MAX_NUM_CONFIGS.
  for (int c = 0; c < MAX_NUM_CONFIGS && c < class_template->NumConfigs; ++c) {
    int rating = tables.sum_feature_evidence_[c];
    if (*classify_debug_level_ > 2) {
      tprintf("Config %d, rating=%d\n", c, rating);
    }
    if (rating > best_match) {
      result->config = c;
      best_match = rating;
    }
    result->fonts.emplace_back(c, rating);
  }

  // Compute confidence on a Probability scale.
  result->rating = best_match / 65536.0f;

  return best_match;
}

/**
 * Applies the CN normalization factor to the given rating and returns
 * the modified rating.
 */
float IntegerMatcher::ApplyCNCorrection(float rating, int blob_length, int normalization_factor,
                                        int matcher_multiplier) {
  int divisor = blob_length + matcher_multiplier;
  return divisor == 0
             ? 1.0f
             : (rating * blob_length + matcher_multiplier * normalization_factor / 256.0f) /
                   divisor;
}

} // namespace tesseract
