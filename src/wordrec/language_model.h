///////////////////////////////////////////////////////////////////////
// File:        language_model.h
// Description: Functions that utilize the knowledge about the properties,
//              structure and statistics of the language to help segmentation
//              search.
// Author:      Daria Antonova
//
// (C) Copyright 2009, Google Inc.
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

#ifndef TESSERACT_WORDREC_LANGUAGE_MODEL_H_
#define TESSERACT_WORDREC_LANGUAGE_MODEL_H_

#include "associate.h"      // for AssociateStats (ptr only), AssociateUtils
#include "dawg.h"           // for DawgPositionVector
#include "dict.h"           // for DawgArgs, Dict
#include "lm_consistency.h" // for LMConsistencyInfo
#include "lm_state.h"       // for ViterbiStateEntry, LanguageModelFlagsType
#include "params.h"         // for DoubleParam, double_VAR_H, IntParam, Boo...
#include "params_model.h"   // for ParamsModel
#include "ratngs.h"         // for BLOB_CHOICE (ptr only), BLOB_CHOICE_LIST...
#include "stopper.h"        // for DANGERR

#include <cmath> // for exp

namespace tesseract {

class UNICHARSET;
class WERD_RES;

struct BlamerBundle;

template <typename T>
class UnicityTable;

class LMPainPoints;
struct FontInfo;

// This class that contains the data structures and functions necessary
// to represent and use the knowledge about the language.
class LanguageModel {
public:
  // Masks for keeping track of top choices that should not be pruned out.
  static const LanguageModelFlagsType kSmallestRatingFlag = 0x1;
  static const LanguageModelFlagsType kLowerCaseFlag = 0x2;
  static const LanguageModelFlagsType kUpperCaseFlag = 0x4;
  static const LanguageModelFlagsType kDigitFlag = 0x8;
  static const LanguageModelFlagsType kXhtConsistentFlag = 0x10;

  // Denominator for normalizing per-letter ngram cost when deriving
  // penalty adjustments.
  static const float kMaxAvgNgramCost;

  LanguageModel(const UnicityTable<FontInfo> *fontinfo_table, Dict *dict);
  ~LanguageModel();

  // Fills the given floats array with features extracted from path represented
  // by the given ViterbiStateEntry. See ccstruct/params_training_featdef.h
  // for feature information.
  // Note: the function assumes that features points to an array of size
  // PTRAIN_NUM_FEATURE_TYPES.
  static void ExtractFeaturesFromPath(const ViterbiStateEntry &vse, float features[]);

  // Updates data structures that are used for the duration of the segmentation
  // search on the current word;
  void InitForWord(const WERD_CHOICE *prev_word, bool fixed_pitch, float max_char_wh_ratio,
                   float rating_cert_scale);

  // Updates language model state of the given BLOB_CHOICE_LIST (from
  // the ratings matrix) a its parent. Updates pain_points if new
  // problematic points are found in the segmentation graph.
  //
  // At most language_model_viterbi_list_size are kept in each
  // LanguageModelState.viterbi_state_entries list.
  // At most language_model_viterbi_list_max_num_prunable of those are prunable
  // (non-dictionary) paths.
  // The entries that represent dictionary word paths are kept at the front
  // of the list.
  // The list ordered by cost that is computed collectively by several
  // language model components (currently dawg and ngram components).
  bool UpdateState(bool just_classified, int curr_col, int curr_row, BLOB_CHOICE_LIST *curr_list,
                   LanguageModelState *parent_node, LMPainPoints *pain_points, WERD_RES *word_res,
                   BestChoiceBundle *best_choice_bundle, BlamerBundle *blamer_bundle);

  // Returns true if an acceptable best choice was discovered.
  inline bool AcceptableChoiceFound() {
    return acceptable_choice_found_;
  }
  inline void SetAcceptableChoiceFound(bool val) {
    acceptable_choice_found_ = val;
  }
  // Returns the reference to ParamsModel.
  inline ParamsModel &getParamsModel() {
    return params_model_;
  }

protected:
  inline float CertaintyScore(float cert) {
    if (language_model_use_sigmoidal_certainty) {
      // cert is assumed to be between 0 and -dict_->certainty_scale.
      // If you enable language_model_use_sigmoidal_certainty, you
      // need to adjust language_model_ngram_nonmatch_score as well.
      cert = -cert / dict_->certainty_scale;
      return 1.0f / (1.0f + exp(10.0f * cert));
    } else {
      return (-1.0f / cert);
    }
  }

  inline float ComputeAdjustment(int num_problems, float penalty) {
    if (num_problems == 0) {
      return 0.0f;
    }
    if (num_problems == 1) {
      return penalty;
    }
    return (penalty + (language_model_penalty_increment * static_cast<float>(num_problems - 1)));
  }

  // Computes the adjustment to the ratings sum based on the given
  // consistency_info. The paths with invalid punctuation, inconsistent
  // case and character type are penalized proportionally to the number
  // of inconsistencies on the path.
  inline float ComputeConsistencyAdjustment(const LanguageModelDawgInfo *dawg_info,
                                            const LMConsistencyInfo &consistency_info) {
    if (dawg_info != nullptr) {
      return ComputeAdjustment(consistency_info.NumInconsistentCase(),
                               language_model_penalty_case) +
             (consistency_info.inconsistent_script ? language_model_penalty_script : 0.0f);
    }
    return (ComputeAdjustment(consistency_info.NumInconsistentPunc(), language_model_penalty_punc) +
            ComputeAdjustment(consistency_info.NumInconsistentCase(), language_model_penalty_case) +
            ComputeAdjustment(consistency_info.NumInconsistentChartype(),
                              language_model_penalty_chartype) +
            ComputeAdjustment(consistency_info.NumInconsistentSpaces(),
                              language_model_penalty_spacing) +
            (consistency_info.inconsistent_script ? language_model_penalty_script : 0.0f) +
            (consistency_info.inconsistent_font ? language_model_penalty_font : 0.0f));
  }

  // Returns an adjusted ratings sum that includes inconsistency penalties,
  // penalties for non-dictionary paths and paths with dips in ngram
  // probability.
  float ComputeAdjustedPathCost(ViterbiStateEntry *vse);

  // Finds the first lower and upper case letter and first digit in curr_list.
  // Uses the first character in the list in place of empty results.
  // Returns true if both alpha and digits are found.
  bool GetTopLowerUpperDigit(BLOB_CHOICE_LIST *curr_list, BLOB_CHOICE **first_lower,
                             BLOB_CHOICE **first_upper, BLOB_CHOICE **first_digit) const;
  // Forces there to be at least one entry in the overall set of the
  // viterbi_state_entries of each element of parent_node that has the
  // top_choice_flag set for lower, upper and digit using the same rules as
  // GetTopLowerUpperDigit, setting the flag on the first found suitable
  // candidate, whether or not the flag is set on some other parent.
  // Returns 1 if both alpha and digits are found among the parents, -1 if no
  // parents are found at all (a legitimate case), and 0 otherwise.
  int SetTopParentLowerUpperDigit(LanguageModelState *parent_node) const;

  // Finds the next ViterbiStateEntry with which the given unichar_id can
  // combine sensibly, taking into account any mixed alnum/mixed case
  // situation, and whether this combination has been inspected before.
  ViterbiStateEntry *GetNextParentVSE(bool just_classified, bool mixed_alnum, const BLOB_CHOICE *bc,
                                      LanguageModelFlagsType blob_choice_flags,
                                      const UNICHARSET &unicharset, WERD_RES *word_res,
                                      ViterbiStateEntry_IT *vse_it,
                                      LanguageModelFlagsType *top_choice_flags) const;
  // Helper function that computes the cost of the path composed of the
  // path in the given parent ViterbiStateEntry and the given BLOB_CHOICE.
  // If the new path looks good enough, adds a new ViterbiStateEntry to the
  // list of viterbi entries in the given BLOB_CHOICE and returns true.
  bool AddViterbiStateEntry(LanguageModelFlagsType top_choice_flags, float denom, bool word_end,
                            int curr_col, int curr_row, BLOB_CHOICE *b,
                            LanguageModelState *curr_state, ViterbiStateEntry *parent_vse,
                            LMPainPoints *pain_points, WERD_RES *word_res,
                            BestChoiceBundle *best_choice_bundle, BlamerBundle *blamer_bundle);

  // Determines whether a potential entry is a true top choice and
  // updates changed accordingly.
  //
  // Note: The function assumes that b, top_choice_flags and changed
  // are not nullptr.
  void GenerateTopChoiceInfo(ViterbiStateEntry *new_vse, const ViterbiStateEntry *parent_vse,
                             LanguageModelState *lms);

  // Calls dict_->LetterIsOk() with DawgArgs initialized from parent_vse and
  // unichar from b.unichar_id(). Constructs and returns LanguageModelDawgInfo
  // with updated active dawgs, constraints and permuter.
  //
  // Note: the caller is responsible for deleting the returned pointer.
  LanguageModelDawgInfo *GenerateDawgInfo(bool word_end, int curr_col, int curr_row,
                                          const BLOB_CHOICE &b,
                                          const ViterbiStateEntry *parent_vse);

  // Computes p(unichar | parent context) and records it in ngram_cost.
  // If b.unichar_id() is an unlikely continuation of the parent context
  // sets found_small_prob to true and returns nullptr.
  // Otherwise creates a new LanguageModelNgramInfo entry containing the
  // updated context (that includes b.unichar_id() at the end) and returns it.
  //
  // Note: the caller is responsible for deleting the returned pointer.
  LanguageModelNgramInfo *GenerateNgramInfo(const char *unichar, float certainty, float denom,
                                            int curr_col, int curr_row, float outline_length,
                                            const ViterbiStateEntry *parent_vse);

  // Computes -(log(prob(classifier)) + log(prob(ngram model)))
  // for the given unichar in the given context. If there are multiple
  // unichars at one position - takes the average of their probabilities.
  // UNICHAR::utf8_step() is used to separate out individual UTF8 characters,
  // since probability_in_context() can only handle one at a time (while
  // unicharset might contain ngrams and glyphs composed from multiple UTF8
  // characters).
  float ComputeNgramCost(const char *unichar, float certainty, float denom, const char *context,
                         int *unichar_step_len, bool *found_small_prob, float *ngram_prob);

  // Computes the normalization factors for the classifier confidences
  // (used by ComputeNgramCost()).
  float ComputeDenom(BLOB_CHOICE_LIST *curr_list);

  // Fills the given consistenty_info based on parent_vse.consistency_info
  // and on the consistency of the given unichar_id with parent_vse.
  void FillConsistencyInfo(int curr_col, bool word_end, BLOB_CHOICE *b,
                           ViterbiStateEntry *parent_vse, WERD_RES *word_res,
                           LMConsistencyInfo *consistency_info);

  // Constructs WERD_CHOICE by recording unichar_ids of the BLOB_CHOICEs
  // on the path represented by the given BLOB_CHOICE and language model
  // state entries (lmse, dse). The path is re-constructed by following
  // the parent pointers in the lang model state entries). If the
  // constructed WERD_CHOICE is better than the best/raw choice recorded
  // in the best_choice_bundle, this function updates the corresponding
  // fields and sets best_choice_bunldle->updated to true.
  void UpdateBestChoice(ViterbiStateEntry *vse, LMPainPoints *pain_points, WERD_RES *word_res,
                        BestChoiceBundle *best_choice_bundle, BlamerBundle *blamer_bundle);

  // Constructs a WERD_CHOICE by tracing parent pointers starting with
  // the given LanguageModelStateEntry. Returns the constructed word.
  // Updates best_char_choices, certainties and state if they are not
  // nullptr (best_char_choices and certainties are assumed to have the
  // length equal to lmse->length).
  // The caller is responsible for freeing memory associated with the
  // returned WERD_CHOICE.
  WERD_CHOICE *ConstructWord(ViterbiStateEntry *vse, WERD_RES *word_res, DANGERR *fixpt,
                             BlamerBundle *blamer_bundle, bool *truth_path);

  // Wrapper around AssociateUtils::ComputeStats().
  inline void ComputeAssociateStats(int col, int row, float max_char_wh_ratio,
                                    ViterbiStateEntry *parent_vse, WERD_RES *word_res,
                                    AssociateStats *associate_stats) {
    AssociateUtils::ComputeStats(
        col, row, (parent_vse != nullptr) ? &(parent_vse->associate_stats) : nullptr,
        (parent_vse != nullptr) ? parent_vse->length : 0, fixed_pitch_, max_char_wh_ratio, word_res,
        language_model_debug_level > 2, associate_stats);
  }

  // Returns true if the path with such top_choice_flags and dawg_info
  // could be pruned out (i.e. is neither a system/user/frequent dictionary
  // nor a top choice path).
  // In non-space delimited languages all paths can be "somewhat" dictionary
  // words. In such languages we cannot do dictionary-driven path pruning,
  // so paths with non-empty dawg_info are considered prunable.
  inline bool PrunablePath(const ViterbiStateEntry &vse) {
    if (vse.top_choice_flags) {
      return false;
    }
    if (vse.dawg_info != nullptr &&
        (vse.dawg_info->permuter == SYSTEM_DAWG_PERM || vse.dawg_info->permuter == USER_DAWG_PERM ||
         vse.dawg_info->permuter == FREQ_DAWG_PERM)) {
      return false;
    }
    return true;
  }

  // Returns true if the given ViterbiStateEntry represents an acceptable path.
  inline bool AcceptablePath(const ViterbiStateEntry &vse) {
    return (vse.dawg_info != nullptr || vse.Consistent() ||
            (vse.ngram_info != nullptr && !vse.ngram_info->pruned));
  }

public:
  // Parameters.
  INT_VAR_H(language_model_debug_level);
  BOOL_VAR_H(language_model_ngram_on);
  INT_VAR_H(language_model_ngram_order);
  INT_VAR_H(language_model_viterbi_list_max_num_prunable);
  INT_VAR_H(language_model_viterbi_list_max_size);
  double_VAR_H(language_model_ngram_small_prob);
  double_VAR_H(language_model_ngram_nonmatch_score);
  BOOL_VAR_H(language_model_ngram_use_only_first_uft8_step);
  double_VAR_H(language_model_ngram_scale_factor);
  double_VAR_H(language_model_ngram_rating_factor);
  BOOL_VAR_H(language_model_ngram_space_delimited_language);
  INT_VAR_H(language_model_min_compound_length);
  // Penalties used for adjusting path costs and final word rating.
  double_VAR_H(language_model_penalty_non_freq_dict_word);
  double_VAR_H(language_model_penalty_non_dict_word);
  double_VAR_H(language_model_penalty_punc);
  double_VAR_H(language_model_penalty_case);
  double_VAR_H(language_model_penalty_script);
  double_VAR_H(language_model_penalty_chartype);
  double_VAR_H(language_model_penalty_font);
  double_VAR_H(language_model_penalty_spacing);
  double_VAR_H(language_model_penalty_increment);
  INT_VAR_H(wordrec_display_segmentations);
  BOOL_VAR_H(language_model_use_sigmoidal_certainty);

protected:
  // Member Variables.

  // Temporary DawgArgs struct that is re-used across different words to
  // avoid dynamic memory re-allocation (should be cleared before each use).
  DawgArgs dawg_args_;
  // Scaling for recovering blob outline length from rating and certainty.
  float rating_cert_scale_ = 0.0f;

  // The following variables are set at construction time.

  // Pointer to fontinfo table (not owned by LanguageModel).
  const UnicityTable<FontInfo> *fontinfo_table_ = nullptr;

  // Pointer to Dict class, that is used for querying the dictionaries
  // (the pointer is not owned by LanguageModel).
  Dict *dict_ = nullptr;

  // TODO(daria): the following variables should become LanguageModel params
  // when the old code in bestfirst.cpp and heuristic.cpp is deprecated.
  //
  // Set to true if we are dealing with fixed pitch text
  // (set to assume_fixed_pitch_char_segment).
  bool fixed_pitch_ = false;
  // Max char width-to-height ratio allowed
  // (set to segsearch_max_char_wh_ratio).
  float max_char_wh_ratio_ = 0.0f;

  // The following variables are initialized with InitForWord().

  // String representation of the classification of the previous word
  // (since this is only used by the character ngram model component,
  // only the last language_model_ngram_order of the word are stored).
  std::string prev_word_str_;
  int prev_word_unichar_step_len_ = 0;
  // Active dawg vector.
  DawgPositionVector very_beginning_active_dawgs_; // includes continuation
  DawgPositionVector beginning_active_dawgs_;
  // Set to true if acceptable choice was discovered.
  // Note: it would be nice to use this to terminate the search once an
  // acceptable choices is found. However we do not do that and once an
  // acceptable choice is found we finish looking for alternative choices
  // in the current segmentation graph and then exit the search (no more
  // classifications are done after an acceptable choice is found).
  // This is needed in order to let the search find the words very close to
  // the best choice in rating (e.g. what/What, Cat/cat, etc) and log these
  // choices. This way the stopper will know that the best choice is not
  // ambiguous (i.e. there are best choices in the best choice list that have
  // ratings close to the very best one) and will be less likely to mis-adapt.
  bool acceptable_choice_found_ = false;
  // Set to true if a choice representing correct segmentation was explored.
  bool correct_segmentation_explored_ = false;

  // Params models containing weights for computing ViterbiStateEntry costs.
  ParamsModel params_model_;
};

} // namespace tesseract

#endif // TESSERACT_WORDREC_LANGUAGE_MODEL_H_
