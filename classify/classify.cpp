///////////////////////////////////////////////////////////////////////
// File:        classify.cpp
// Description: classify class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#include "classify.h"
#include "intproto.h"
#include "mfoutline.h"
#include "scrollview.h"
#include "unicity_table.h"
#include <string.h>

namespace {

// Compare FontInfo structures.
bool compare_fontinfo(const FontInfo& fi1, const FontInfo& fi2) {
  // The font properties are required to be the same for two font with the same
  // name, so there is no need to test them.
  // Consequently, querying the table with only its font name as information is
  // enough to retrieve its properties.
  return strcmp(fi1.name, fi2.name) == 0;
}
// Compare FontSet structures.
bool compare_font_set(const FontSet& fs1, const FontSet& fs2) {
  if (fs1.size != fs2.size)
    return false;
  for (int i = 0; i < fs1.size; ++i) {
    if (fs1.configs[i] != fs2.configs[i])
      return false;
  }
  return true;
}

void delete_callback(FontInfo f) {
  if (f.spacing_vec != NULL) {
    f.spacing_vec->delete_data_pointers();
    delete f.spacing_vec;
  }
  delete[] f.name;
}
void delete_callback_fs(FontSet fs) {
  delete[] fs.configs;
}

}

namespace tesseract {
Classify::Classify()
  : INT_MEMBER(tessedit_single_match, FALSE,
               "Top choice only from CP", this->params()),
    BOOL_MEMBER(classify_enable_learning, true,
                "Enable adaptive classifier", this->params()),
    INT_MEMBER(classify_debug_level, 0, "Classify debug level",
               this->params()),
    INT_MEMBER(classify_norm_method, character, "Normalization Method   ...",
               this->params()),
    double_MEMBER(classify_char_norm_range, 0.2,
                  "Character Normalization Range ...", this->params()),
    double_MEMBER(classify_min_norm_scale_x, 0.0, "Min char x-norm scale ...",
                  this->params()),  /* PREV DEFAULT 0.1 */
    double_MEMBER(classify_max_norm_scale_x, 0.325, "Max char x-norm scale ...",
                  this->params()),  /* PREV DEFAULT 0.3 */
    double_MEMBER(classify_min_norm_scale_y, 0.0, "Min char y-norm scale ...",
                  this->params()),  /* PREV DEFAULT 0.1 */
    double_MEMBER(classify_max_norm_scale_y, 0.325, "Max char y-norm scale ...",
                  this->params()),  /* PREV DEFAULT 0.3 */
    BOOL_MEMBER(tess_cn_matching, 0, "Character Normalized Matching",
                this->params()),
    BOOL_MEMBER(tess_bn_matching, 0, "Baseline Normalized Matching",
                this->params()),
    BOOL_MEMBER(classify_enable_adaptive_matcher, 1,
                "Enable adaptive classifier",
                this->params()),
    BOOL_MEMBER(classify_use_pre_adapted_templates, 0,
                "Use pre-adapted classifier templates", this->params()),
    BOOL_MEMBER(classify_save_adapted_templates, 0,
               "Save adapted templates to a file", this->params()),
    BOOL_MEMBER(classify_enable_adaptive_debugger, 0, "Enable match debugger",
                this->params()),
    INT_MEMBER(matcher_debug_level, 0, "Matcher Debug Level", this->params()),
    INT_MEMBER(matcher_debug_flags, 0, "Matcher Debug Flags", this->params()),
    INT_MEMBER(classify_learning_debug_level, 0, "Learning Debug Level: ",
               this->params()),
    double_MEMBER(matcher_good_threshold, 0.125, "Good Match (0-1)",
                  this->params()),
    double_MEMBER(matcher_great_threshold, 0.0, "Great Match (0-1)",
                  this->params()),
    double_MEMBER(matcher_perfect_threshold, 0.02, "Perfect Match (0-1)",
                  this->params()),
    double_MEMBER(matcher_bad_match_pad, 0.15, "Bad Match Pad (0-1)",
                  this->params()),
    double_MEMBER(matcher_rating_margin, 0.1, "New template margin (0-1)",
                  this->params()),
    double_MEMBER(matcher_avg_noise_size, 12.0, "Avg. noise blob length",
                  this->params()),
    INT_MEMBER(matcher_permanent_classes_min, 1, "Min # of permanent classes",
               this->params()),
    INT_MEMBER(matcher_min_examples_for_prototyping, 3,
               "Reliable Config Threshold", this->params()),
    INT_MEMBER(matcher_sufficient_examples_for_prototyping, 5,
               "Enable adaption even if the ambiguities have not been seen",
               this->params()),
    double_MEMBER(matcher_clustering_max_angle_delta, 0.015,
                  "Maximum angle delta for prototype clustering",
                  this->params()),
    double_MEMBER(classify_misfit_junk_penalty, 0.0,
                  "Penalty to apply when a non-alnum is vertically out of "
                  "its expected textline position",
                  this->params()),
    BOOL_MEMBER(classify_enable_int_fx, 1, "Enable integer fx",
                this->params()),
    BOOL_MEMBER(classify_enable_new_adapt_rules, 1,
                "Enable new adaptation rules", this->params()),
    double_MEMBER(rating_scale, 1.5, "Rating scaling factor", this->params()),
    double_MEMBER(certainty_scale, 20.0, "Certainty scaling factor",
                  this->params()),
    double_MEMBER(tessedit_class_miss_scale, 0.00390625,
                  "Scale factor for features not used", this->params()),
    INT_MEMBER(classify_adapt_proto_threshold, 230,
               "Threshold for good protos during adaptive 0-255",
               this->params()),
    INT_MEMBER(classify_adapt_feature_threshold, 230,
               "Threshold for good features during adaptive 0-255",
               this->params()),
    BOOL_MEMBER(disable_character_fragments, TRUE,
                "Do not include character fragments in the"
                " results of the classifier", this->params()),
    double_MEMBER(classify_character_fragments_garbage_certainty_threshold,
                  -3.0, "Exclude fragments that do not look like whole"
                  " characters from training and adaption", this->params()),
    BOOL_MEMBER(classify_debug_character_fragments, FALSE,
                "Bring up graphical debugging windows for fragments training",
                this->params()),
    BOOL_MEMBER(matcher_debug_separate_windows, FALSE,
                "Use two different windows for debugging the matching: "
                "One for the protos and one for the features.", this->params()),
    STRING_MEMBER(classify_learn_debug_str, "", "Class str to debug learning",
                  this->params()),
    INT_INIT_MEMBER(classify_class_pruner_threshold, 229,
                    "Class Pruner Threshold 0-255:        ", this->params()),
    INT_INIT_MEMBER(classify_class_pruner_multiplier, 30,
                    "Class Pruner Multiplier 0-255:       ", this->params()),
    INT_INIT_MEMBER(classify_cp_cutoff_strength, 7,
                    "Class Pruner CutoffStrength:         ", this->params()),
    INT_INIT_MEMBER(classify_integer_matcher_multiplier, 14,
                    "Integer Matcher Multiplier  0-255:   ", this->params()),
    EnableLearning(true),
    INT_MEMBER(il1_adaption_test, 0, "Dont adapt to i/I at beginning of word",
               this->params()),
    BOOL_MEMBER(classify_bln_numeric_mode, 0,
                "Assume the input is numbers [0-9].", this->params()),
    dict_(&image_) {
  fontinfo_table_.set_compare_callback(
      NewPermanentTessCallback(compare_fontinfo));
  fontinfo_table_.set_clear_callback(
      NewPermanentTessCallback(delete_callback));
  fontset_table_.set_compare_callback(
      NewPermanentTessCallback(compare_font_set));
  fontset_table_.set_clear_callback(
      NewPermanentTessCallback(delete_callback_fs));
  AdaptedTemplates = NULL;
  PreTrainedTemplates = NULL;
  AllProtosOn = NULL;
  PrunedProtos = NULL;
  AllConfigsOn = NULL;
  AllProtosOff = NULL;
  AllConfigsOff = NULL;
  TempProtoMask = NULL;
  NormProtos = NULL;

  AdaptiveMatcherCalls = 0;
  BaselineClassifierCalls = 0;
  CharNormClassifierCalls = 0;
  AmbigClassifierCalls = 0;
  NumWordsAdaptedTo = 0;
  NumCharsAdaptedTo = 0;
  NumBaselineClassesTried = 0;
  NumCharNormClassesTried = 0;
  NumAmbigClassesTried = 0;
  NumClassesOutput = 0;
  NumAdaptationsFailed = 0;

  FeaturesHaveBeenExtracted = false;
  FeaturesOK = true;
  learn_debug_win_ = NULL;
  learn_fragmented_word_debug_win_ = NULL;
  learn_fragments_debug_win_ = NULL;
}

Classify::~Classify() {
  EndAdaptiveClassifier();
  delete learn_debug_win_;
  delete learn_fragmented_word_debug_win_;
  delete learn_fragments_debug_win_;
}

}  // namespace tesseract
