///////////////////////////////////////////////////////////////////////
// File:        language_model.cpp
// Description: Functions that utilize the knowledge about the properties,
//              structure and statistics of the language to help recognition.
// Author:      Daria Antonova
// Created:     Mon Nov 11 11:26:43 PST 2009
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

#include <math.h>

#include "language_model.h"

#include "dawg.h"
#include "freelist.h"
#include "intproto.h"
#include "helpers.h"
#include "lm_state.h"
#include "lm_pain_points.h"
#include "matrix.h"
#include "params.h"
#include "params_training_featdef.h"

#if (defined(_MSC_VER) && _MSC_VER < 1900) || defined(ANDROID)
double log2(double n) {
  return log(n) / log(2.0);
}
#endif  // _MSC_VER

namespace tesseract {

const float LanguageModel::kMaxAvgNgramCost = 25.0f;

LanguageModel::LanguageModel(const UnicityTable<FontInfo> *fontinfo_table,
                             Dict *dict)
  : INT_MEMBER(language_model_debug_level, 0, "Language model debug level",
               dict->getCCUtil()->params()),
    BOOL_INIT_MEMBER(language_model_ngram_on, false,
                     "Turn on/off the use of character ngram model",
                     dict->getCCUtil()->params()),
    INT_MEMBER(language_model_ngram_order, 8,
               "Maximum order of the character ngram model",
               dict->getCCUtil()->params()),
    INT_MEMBER(language_model_viterbi_list_max_num_prunable, 10,
               "Maximum number of prunable (those for which"
               " PrunablePath() is true) entries in each viterbi list"
               " recorded in BLOB_CHOICEs",
               dict->getCCUtil()->params()),
    INT_MEMBER(language_model_viterbi_list_max_size, 500,
               "Maximum size of viterbi lists recorded in BLOB_CHOICEs",
               dict->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_small_prob, 0.000001,
                  "To avoid overly small denominators use this as the "
                  "floor of the probability returned by the ngram model.",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_nonmatch_score, -40.0,
                  "Average classifier score of a non-matching unichar.",
                  dict->getCCUtil()->params()),
    BOOL_MEMBER(language_model_ngram_use_only_first_uft8_step, false,
                "Use only the first UTF8 step of the given string"
                " when computing log probabilities.",
                dict->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_scale_factor, 0.03,
                  "Strength of the character ngram model relative to the"
                  " character classifier ",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_rating_factor, 16.0,
                  "Factor to bring log-probs into the same range as ratings"
                  " when multiplied by outline length ",
                  dict->getCCUtil()->params()),
    BOOL_MEMBER(language_model_ngram_space_delimited_language, true,
                "Words are delimited by space",
                dict->getCCUtil()->params()),
    INT_MEMBER(language_model_min_compound_length, 3,
               "Minimum length of compound words",
               dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_non_freq_dict_word, 0.1,
                  "Penalty for words not in the frequent word dictionary",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_non_dict_word, 0.15,
                  "Penalty for non-dictionary words",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_punc, 0.2,
                  "Penalty for inconsistent punctuation",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_case, 0.1,
                  "Penalty for inconsistent case",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_script, 0.5,
                  "Penalty for inconsistent script",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_chartype, 0.3,
                  "Penalty for inconsistent character type",
                  dict->getCCUtil()->params()),
    // TODO(daria, rays): enable font consistency checking
    // after improving font analysis.
    double_MEMBER(language_model_penalty_font, 0.00,
                  "Penalty for inconsistent font",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_spacing, 0.05,
                  "Penalty for inconsistent spacing",
                  dict->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_increment, 0.01,
                  "Penalty increment",
                  dict->getCCUtil()->params()),
    INT_MEMBER(wordrec_display_segmentations, 0, "Display Segmentations",
               dict->getCCUtil()->params()),
    BOOL_INIT_MEMBER(language_model_use_sigmoidal_certainty, false,
                     "Use sigmoidal score for certainty",
                     dict->getCCUtil()->params()),
  fontinfo_table_(fontinfo_table), dict_(dict),
  fixed_pitch_(false), max_char_wh_ratio_(0.0),
  acceptable_choice_found_(false) {
  ASSERT_HOST(dict_ != NULL);
  dawg_args_ = new DawgArgs(NULL, new DawgPositionVector(), NO_PERM);
  very_beginning_active_dawgs_ = new DawgPositionVector();
  beginning_active_dawgs_ = new DawgPositionVector();
}

LanguageModel::~LanguageModel() {
  delete very_beginning_active_dawgs_;
  delete beginning_active_dawgs_;
  delete dawg_args_->updated_dawgs;
  delete dawg_args_;
}

void LanguageModel::InitForWord(const WERD_CHOICE *prev_word,
                                bool fixed_pitch, float max_char_wh_ratio,
                                float rating_cert_scale) {
  fixed_pitch_ = fixed_pitch;
  max_char_wh_ratio_ = max_char_wh_ratio;
  rating_cert_scale_ = rating_cert_scale;
  acceptable_choice_found_ = false;
  correct_segmentation_explored_ = false;

  // Initialize vectors with beginning DawgInfos.
  very_beginning_active_dawgs_->clear();
  dict_->init_active_dawgs(very_beginning_active_dawgs_, false);
  beginning_active_dawgs_->clear();
  dict_->default_dawgs(beginning_active_dawgs_, false);

  // Fill prev_word_str_ with the last language_model_ngram_order
  // unichars from prev_word.
  if (language_model_ngram_on) {
    if (prev_word != NULL && prev_word->unichar_string() != NULL) {
      prev_word_str_ = prev_word->unichar_string();
      if (language_model_ngram_space_delimited_language) prev_word_str_ += ' ';
    } else {
      prev_word_str_ = " ";
    }
    const char *str_ptr = prev_word_str_.string();
    const char *str_end = str_ptr + prev_word_str_.length();
    int step;
    prev_word_unichar_step_len_ = 0;
    while (str_ptr != str_end && (step = UNICHAR::utf8_step(str_ptr))) {
      str_ptr += step;
      ++prev_word_unichar_step_len_;
    }
    ASSERT_HOST(str_ptr == str_end);
  }
}

/**
 * Helper scans the collection of predecessors for competing siblings that
 * have the same letter with the opposite case, setting competing_vse.
 */
static void ScanParentsForCaseMix(const UNICHARSET& unicharset,
                                  LanguageModelState* parent_node) {
  if (parent_node == NULL) return;
  ViterbiStateEntry_IT vit(&parent_node->viterbi_state_entries);
  for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
    ViterbiStateEntry* vse = vit.data();
    vse->competing_vse = NULL;
    UNICHAR_ID unichar_id = vse->curr_b->unichar_id();
    if (unicharset.get_isupper(unichar_id) ||
        unicharset.get_islower(unichar_id)) {
      UNICHAR_ID other_case = unicharset.get_other_case(unichar_id);
      if (other_case == unichar_id) continue;  // Not in unicharset.
      // Find other case in same list. There could be multiple entries with
      // the same unichar_id, but in theory, they should all point to the
      // same BLOB_CHOICE, and that is what we will be using to decide
      // which to keep.
      ViterbiStateEntry_IT vit2(&parent_node->viterbi_state_entries);
      for (vit2.mark_cycle_pt(); !vit2.cycled_list() &&
           vit2.data()->curr_b->unichar_id() != other_case;
           vit2.forward()) {}
      if (!vit2.cycled_list()) {
        vse->competing_vse = vit2.data();
      }
    }
  }
}

/**
 * Helper returns true if the given choice has a better case variant before
 * it in the choice_list that is not distinguishable by size.
 */
static bool HasBetterCaseVariant(const UNICHARSET& unicharset,
                                 const BLOB_CHOICE* choice,
                                 BLOB_CHOICE_LIST* choices) {
  UNICHAR_ID choice_id = choice->unichar_id();
  UNICHAR_ID other_case = unicharset.get_other_case(choice_id);
  if (other_case == choice_id || other_case == INVALID_UNICHAR_ID)
    return false;  // Not upper or lower or not in unicharset.
  if (unicharset.SizesDistinct(choice_id, other_case))
    return false;  // Can be separated by size.
  BLOB_CHOICE_IT bc_it(choices);
  for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
    BLOB_CHOICE* better_choice = bc_it.data();
    if (better_choice->unichar_id() == other_case)
      return true;  // Found an earlier instance of other_case.
    else if (better_choice == choice)
      return false;  // Reached the original choice.
  }
  return false;  // Should never happen, but just in case.
}

/**
 * UpdateState has the job of combining the ViterbiStateEntry lists on each
 * of the choices on parent_list with each of the blob choices in curr_list,
 * making a new ViterbiStateEntry for each sensible path.
 *
 * This could be a huge set of combinations, creating a lot of work only to
 * be truncated by some beam limit, but only certain kinds of paths will
 * continue at the next step:
 * - paths that are liked by the language model: either a DAWG or the n-gram
 *   model, where active.
 * - paths that represent some kind of top choice. The old permuter permuted
 *   the top raw classifier score, the top upper case word and the top lower-
 *   case word. UpdateState now concentrates its top-choice paths on top
 *   lower-case, top upper-case (or caseless alpha), and top digit sequence,
 *   with allowance for continuation of these paths through blobs where such
 *   a character does not appear in the choices list.
 *
 * GetNextParentVSE enforces some of these models to minimize the number of
 * calls to AddViterbiStateEntry, even prior to looking at the language model.
 * Thus an n-blob sequence of [l1I] will produce 3n calls to
 * AddViterbiStateEntry instead of 3^n.
 *
 * Of course it isn't quite that simple as Title Case is handled by allowing
 * lower case to continue an upper case initial, but it has to be detected
 * in the combiner so it knows which upper case letters are initial alphas.
 */
bool LanguageModel::UpdateState(
    bool just_classified,
    int curr_col, int curr_row,
    BLOB_CHOICE_LIST *curr_list,
    LanguageModelState *parent_node,
    LMPainPoints *pain_points,
    WERD_RES *word_res,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  if (language_model_debug_level > 0) {
    tprintf("\nUpdateState: col=%d row=%d %s",
            curr_col, curr_row, just_classified ? "just_classified" : "");
    if (language_model_debug_level > 5)
      tprintf("(parent=%p)\n", parent_node);
    else
      tprintf("\n");
  }
  // Initialize helper variables.
  bool word_end = (curr_row+1 >= word_res->ratings->dimension());
  bool new_changed = false;
  float denom = (language_model_ngram_on) ? ComputeDenom(curr_list) : 1.0f;
  const UNICHARSET& unicharset = dict_->getUnicharset();
  BLOB_CHOICE *first_lower = NULL;
  BLOB_CHOICE *first_upper = NULL;
  BLOB_CHOICE *first_digit = NULL;
  bool has_alnum_mix = false;
  if (parent_node != NULL) {
    int result = SetTopParentLowerUpperDigit(parent_node);
    if (result < 0) {
      if (language_model_debug_level > 0)
        tprintf("No parents found to process\n");
      return false;
    }
    if (result > 0)
      has_alnum_mix = true;
  }
  if (!GetTopLowerUpperDigit(curr_list, &first_lower, &first_upper,
                             &first_digit))
    has_alnum_mix = false;;
  ScanParentsForCaseMix(unicharset, parent_node);
  if (language_model_debug_level > 3 && parent_node != NULL) {
    parent_node->Print("Parent viterbi list");
  }
  LanguageModelState *curr_state = best_choice_bundle->beam[curr_row];

  // Call AddViterbiStateEntry() for each parent+child ViterbiStateEntry.
  ViterbiStateEntry_IT vit;
  BLOB_CHOICE_IT c_it(curr_list);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    BLOB_CHOICE* choice = c_it.data();
    // TODO(antonova): make sure commenting this out if ok for ngram
    // model scoring (I think this was introduced to fix ngram model quirks).
    // Skip NULL unichars unless it is the only choice.
    //if (!curr_list->singleton() && c_it.data()->unichar_id() == 0) continue;
    UNICHAR_ID unichar_id = choice->unichar_id();
    if (unicharset.get_fragment(unichar_id)) {
      continue;  // Skip fragments.
    }
    // Set top choice flags.
    LanguageModelFlagsType blob_choice_flags = kXhtConsistentFlag;
    if (c_it.at_first() || !new_changed)
      blob_choice_flags |= kSmallestRatingFlag;
    if (first_lower == choice) blob_choice_flags |= kLowerCaseFlag;
    if (first_upper == choice) blob_choice_flags |= kUpperCaseFlag;
    if (first_digit == choice) blob_choice_flags |= kDigitFlag;

    if (parent_node == NULL) {
      // Process the beginning of a word.
      // If there is a better case variant that is not distinguished by size,
      // skip this blob choice, as we have no choice but to accept the result
      // of the character classifier to distinguish between them, even if
      // followed by an upper case.
      // With words like iPoc, and other CamelBackWords, the lower-upper
      // transition can only be achieved if the classifier has the correct case
      // as the top choice, and leaving an initial I lower down the list
      // increases the chances of choosing IPoc simply because it doesn't
      // include such a transition. iPoc will beat iPOC and ipoc because
      // the other words are baseline/x-height inconsistent.
      if (HasBetterCaseVariant(unicharset, choice, curr_list))
        continue;
      // Upper counts as lower at the beginning of a word.
      if (blob_choice_flags & kUpperCaseFlag)
        blob_choice_flags |= kLowerCaseFlag;
      new_changed |= AddViterbiStateEntry(
          blob_choice_flags, denom, word_end, curr_col, curr_row,
          choice, curr_state, NULL, pain_points,
          word_res, best_choice_bundle, blamer_bundle);
    } else {
      // Get viterbi entries from each parent ViterbiStateEntry.
      vit.set_to_list(&parent_node->viterbi_state_entries);
      int vit_counter = 0;
      vit.mark_cycle_pt();
      ViterbiStateEntry* parent_vse = NULL;
      LanguageModelFlagsType top_choice_flags;
      while ((parent_vse = GetNextParentVSE(just_classified, has_alnum_mix,
                                            c_it.data(), blob_choice_flags,
                                            unicharset, word_res, &vit,
                                            &top_choice_flags)) != NULL) {
        // Skip pruned entries and do not look at prunable entries if already
        // examined language_model_viterbi_list_max_num_prunable of those.
        if (PrunablePath(*parent_vse) &&
            (++vit_counter > language_model_viterbi_list_max_num_prunable ||
             (language_model_ngram_on && parent_vse->ngram_info->pruned))) {
          continue;
        }
        // If the parent has no alnum choice, (ie choice is the first in a
        // string of alnum), and there is a better case variant that is not
        // distinguished by size, skip this blob choice/parent, as with the
        // initial blob treatment above.
        if (!parent_vse->HasAlnumChoice(unicharset) &&
            HasBetterCaseVariant(unicharset, choice, curr_list))
          continue;
        // Create a new ViterbiStateEntry if BLOB_CHOICE in c_it.data()
        // looks good according to the Dawgs or character ngram model.
        new_changed |= AddViterbiStateEntry(
            top_choice_flags, denom, word_end, curr_col, curr_row,
            c_it.data(), curr_state, parent_vse, pain_points,
            word_res, best_choice_bundle, blamer_bundle);
      }
    }
  }
  return new_changed;
}

/**
 * Finds the first lower and upper case letter and first digit in curr_list.
 * For non-upper/lower languages, alpha counts as upper.
 * Uses the first character in the list in place of empty results.
 * Returns true if both alpha and digits are found.
 */
bool LanguageModel::GetTopLowerUpperDigit(BLOB_CHOICE_LIST *curr_list,
                                          BLOB_CHOICE **first_lower,
                                          BLOB_CHOICE **first_upper,
                                          BLOB_CHOICE **first_digit) const {
  BLOB_CHOICE_IT c_it(curr_list);
  const UNICHARSET &unicharset = dict_->getUnicharset();
  BLOB_CHOICE *first_unichar = NULL;
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    UNICHAR_ID unichar_id = c_it.data()->unichar_id();
    if (unicharset.get_fragment(unichar_id)) continue;  // skip fragments
    if (first_unichar == NULL) first_unichar = c_it.data();
    if (*first_lower == NULL && unicharset.get_islower(unichar_id)) {
      *first_lower = c_it.data();
    }
    if (*first_upper == NULL && unicharset.get_isalpha(unichar_id) &&
        !unicharset.get_islower(unichar_id)) {
      *first_upper = c_it.data();
    }
    if (*first_digit == NULL && unicharset.get_isdigit(unichar_id)) {
      *first_digit = c_it.data();
    }
  }
  ASSERT_HOST(first_unichar != NULL);
  bool mixed = (*first_lower != NULL || *first_upper != NULL) &&
      *first_digit != NULL;
  if (*first_lower == NULL) *first_lower = first_unichar;
  if (*first_upper == NULL) *first_upper = first_unichar;
  if (*first_digit == NULL) *first_digit = first_unichar;
  return mixed;
}

/**
 * Forces there to be at least one entry in the overall set of the
 * viterbi_state_entries of each element of parent_node that has the
 * top_choice_flag set for lower, upper and digit using the same rules as
 * GetTopLowerUpperDigit, setting the flag on the first found suitable
 * candidate, whether or not the flag is set on some other parent.
 * Returns 1 if both alpha and digits are found among the parents, -1 if no
 * parents are found at all (a legitimate case), and 0 otherwise.
 */
int LanguageModel::SetTopParentLowerUpperDigit(
    LanguageModelState *parent_node) const {
  if (parent_node == NULL) return -1;
  UNICHAR_ID top_id = INVALID_UNICHAR_ID;
  ViterbiStateEntry* top_lower = NULL;
  ViterbiStateEntry* top_upper = NULL;
  ViterbiStateEntry* top_digit = NULL;
  ViterbiStateEntry* top_choice = NULL;
  float lower_rating = 0.0f;
  float upper_rating = 0.0f;
  float digit_rating = 0.0f;
  float top_rating = 0.0f;
  const UNICHARSET &unicharset = dict_->getUnicharset();
  ViterbiStateEntry_IT vit(&parent_node->viterbi_state_entries);
  for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
    ViterbiStateEntry* vse = vit.data();
    // INVALID_UNICHAR_ID should be treated like a zero-width joiner, so scan
    // back to the real character if needed.
    ViterbiStateEntry* unichar_vse = vse;
    UNICHAR_ID unichar_id = unichar_vse->curr_b->unichar_id();
    float rating = unichar_vse->curr_b->rating();
    while (unichar_id == INVALID_UNICHAR_ID &&
           unichar_vse->parent_vse != NULL) {
      unichar_vse = unichar_vse->parent_vse;
      unichar_id = unichar_vse->curr_b->unichar_id();
      rating = unichar_vse->curr_b->rating();
    }
    if (unichar_id != INVALID_UNICHAR_ID) {
      if (unicharset.get_islower(unichar_id)) {
        if (top_lower == NULL || lower_rating > rating) {
          top_lower = vse;
          lower_rating = rating;
        }
      } else if (unicharset.get_isalpha(unichar_id)) {
        if (top_upper == NULL || upper_rating > rating) {
          top_upper = vse;
          upper_rating = rating;
        }
      } else if (unicharset.get_isdigit(unichar_id)) {
        if (top_digit == NULL || digit_rating > rating) {
          top_digit = vse;
          digit_rating = rating;
        }
      }
    }
    if (top_choice == NULL || top_rating > rating) {
      top_choice = vse;
      top_rating = rating;
      top_id = unichar_id;
    }
  }
  if (top_choice == NULL) return -1;
  bool mixed = (top_lower != NULL || top_upper != NULL) &&
      top_digit != NULL;
  if (top_lower == NULL) top_lower = top_choice;
  top_lower->top_choice_flags |= kLowerCaseFlag;
  if (top_upper == NULL) top_upper = top_choice;
  top_upper->top_choice_flags |= kUpperCaseFlag;
  if (top_digit == NULL) top_digit = top_choice;
  top_digit->top_choice_flags |= kDigitFlag;
  top_choice->top_choice_flags |= kSmallestRatingFlag;
  if (top_id != INVALID_UNICHAR_ID && dict_->compound_marker(top_id) &&
      (top_choice->top_choice_flags &
          (kLowerCaseFlag | kUpperCaseFlag | kDigitFlag))) {
    // If the compound marker top choice carries any of the top alnum flags,
    // then give it all of them, allowing words like I-295 to be chosen.
    top_choice->top_choice_flags |=
        kLowerCaseFlag | kUpperCaseFlag | kDigitFlag;
  }
  return mixed ? 1 : 0;
}

/**
 * Finds the next ViterbiStateEntry with which the given unichar_id can
 * combine sensibly, taking into account any mixed alnum/mixed case
 * situation, and whether this combination has been inspected before.
 */
ViterbiStateEntry* LanguageModel::GetNextParentVSE(
    bool just_classified, bool mixed_alnum, const BLOB_CHOICE* bc,
    LanguageModelFlagsType blob_choice_flags, const UNICHARSET& unicharset,
    WERD_RES* word_res, ViterbiStateEntry_IT* vse_it,
    LanguageModelFlagsType* top_choice_flags) const {
  for (; !vse_it->cycled_list(); vse_it->forward()) {
    ViterbiStateEntry* parent_vse = vse_it->data();
    // Only consider the parent if it has been updated or
    // if the current ratings cell has just been classified.
    if (!just_classified && !parent_vse->updated) continue;
    if (language_model_debug_level > 2)
      parent_vse->Print("Considering");
    // If the parent is non-alnum, then upper counts as lower.
    *top_choice_flags = blob_choice_flags;
    if ((blob_choice_flags & kUpperCaseFlag) &&
        !parent_vse->HasAlnumChoice(unicharset)) {
      *top_choice_flags |= kLowerCaseFlag;
    }
    *top_choice_flags &= parent_vse->top_choice_flags;
    UNICHAR_ID unichar_id = bc->unichar_id();
    const BLOB_CHOICE* parent_b = parent_vse->curr_b;
    UNICHAR_ID parent_id = parent_b->unichar_id();
    // Digits do not bind to alphas if there is a mix in both parent and current
    // or if the alpha is not the top choice.
    if (unicharset.get_isdigit(unichar_id) &&
        unicharset.get_isalpha(parent_id) &&
        (mixed_alnum || *top_choice_flags == 0))
      continue;  // Digits don't bind to alphas.
    // Likewise alphas do not bind to digits if there is a mix in both or if
    // the digit is not the top choice.
    if (unicharset.get_isalpha(unichar_id) &&
        unicharset.get_isdigit(parent_id) &&
        (mixed_alnum || *top_choice_flags == 0))
      continue;  // Alphas don't bind to digits.
    // If there is a case mix of the same alpha in the parent list, then
    // competing_vse is non-null and will be used to determine whether
    // or not to bind the current blob choice.
    if (parent_vse->competing_vse != NULL) {
      const BLOB_CHOICE* competing_b = parent_vse->competing_vse->curr_b;
      UNICHAR_ID other_id = competing_b->unichar_id();
      if (language_model_debug_level >= 5) {
        tprintf("Parent %s has competition %s\n",
                unicharset.id_to_unichar(parent_id),
                unicharset.id_to_unichar(other_id));
      }
      if (unicharset.SizesDistinct(parent_id, other_id)) {
        // If other_id matches bc wrt position and size, and parent_id, doesn't,
        // don't bind to the current parent.
        if (bc->PosAndSizeAgree(*competing_b, word_res->x_height,
                                language_model_debug_level >= 5) &&
            !bc->PosAndSizeAgree(*parent_b, word_res->x_height,
                                language_model_debug_level >= 5))
          continue;  // Competing blobchoice has a better vertical match.
      }
    }
    vse_it->forward();
    return parent_vse;  // This one is good!
  }
  return NULL;  // Ran out of possibilities.
}

bool LanguageModel::AddViterbiStateEntry(
    LanguageModelFlagsType top_choice_flags,
    float denom,
    bool word_end,
    int curr_col, int curr_row,
    BLOB_CHOICE *b,
    LanguageModelState *curr_state,
    ViterbiStateEntry *parent_vse,
    LMPainPoints *pain_points,
    WERD_RES *word_res,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  ViterbiStateEntry_IT vit;
  if (language_model_debug_level > 1) {
    tprintf("AddViterbiStateEntry for unichar %s rating=%.4f"
            " certainty=%.4f top_choice_flags=0x%x",
            dict_->getUnicharset().id_to_unichar(b->unichar_id()),
            b->rating(), b->certainty(), top_choice_flags);
    if (language_model_debug_level > 5)
      tprintf(" parent_vse=%p\n", parent_vse);
    else
      tprintf("\n");
  }
  // Check whether the list is full.
  if (curr_state != NULL &&
      curr_state->viterbi_state_entries_length >=
          language_model_viterbi_list_max_size) {
    if (language_model_debug_level > 1) {
      tprintf("AddViterbiStateEntry: viterbi list is full!\n");
    }
    return false;
  }

  // Invoke Dawg language model component.
  LanguageModelDawgInfo *dawg_info =
    GenerateDawgInfo(word_end, curr_col, curr_row, *b, parent_vse);

  float outline_length =
      AssociateUtils::ComputeOutlineLength(rating_cert_scale_, *b);
  // Invoke Ngram language model component.
  LanguageModelNgramInfo *ngram_info = NULL;
  if (language_model_ngram_on) {
    ngram_info = GenerateNgramInfo(
        dict_->getUnicharset().id_to_unichar(b->unichar_id()), b->certainty(),
        denom, curr_col, curr_row, outline_length, parent_vse);
    ASSERT_HOST(ngram_info != NULL);
  }
  bool liked_by_language_model = dawg_info != NULL ||
      (ngram_info != NULL && !ngram_info->pruned);
  // Quick escape if not liked by the language model, can't be consistent
  // xheight, and not top choice.
  if (!liked_by_language_model && top_choice_flags == 0) {
    if (language_model_debug_level > 1) {
      tprintf("Language model components very early pruned this entry\n");
    }
    delete ngram_info;
    delete dawg_info;
    return false;
  }

  // Check consistency of the path and set the relevant consistency_info.
  LMConsistencyInfo consistency_info(
    parent_vse != NULL ? &parent_vse->consistency_info : NULL);
  // Start with just the x-height consistency, as it provides significant
  // pruning opportunity.
  consistency_info.ComputeXheightConsistency(
      b, dict_->getUnicharset().get_ispunctuation(b->unichar_id()));
  // Turn off xheight consistent flag if not consistent.
  if (consistency_info.InconsistentXHeight()) {
    top_choice_flags &= ~kXhtConsistentFlag;
  }

  // Quick escape if not liked by the language model, not consistent xheight,
  // and not top choice.
  if (!liked_by_language_model && top_choice_flags == 0) {
    if (language_model_debug_level > 1) {
      tprintf("Language model components early pruned this entry\n");
    }
    delete ngram_info;
    delete dawg_info;
    return false;
  }

  // Compute the rest of the consistency info.
  FillConsistencyInfo(curr_col, word_end, b, parent_vse,
                      word_res, &consistency_info);
  if (dawg_info != NULL && consistency_info.invalid_punc) {
    consistency_info.invalid_punc = false;  // do not penalize dict words
  }

  // Compute cost of associating the blobs that represent the current unichar.
  AssociateStats associate_stats;
  ComputeAssociateStats(curr_col, curr_row, max_char_wh_ratio_,
                        parent_vse, word_res, &associate_stats);
  if (parent_vse != NULL) {
    associate_stats.shape_cost += parent_vse->associate_stats.shape_cost;
    associate_stats.bad_shape |= parent_vse->associate_stats.bad_shape;
  }

  // Create the new ViterbiStateEntry compute the adjusted cost of the path.
  ViterbiStateEntry *new_vse = new ViterbiStateEntry(
      parent_vse, b, 0.0, outline_length,
      consistency_info, associate_stats, top_choice_flags, dawg_info,
      ngram_info, (language_model_debug_level > 0) ?
          dict_->getUnicharset().id_to_unichar(b->unichar_id()) : NULL);
  new_vse->cost = ComputeAdjustedPathCost(new_vse);
  if (language_model_debug_level >= 3)
    tprintf("Adjusted cost = %g\n", new_vse->cost);

  // Invoke Top Choice language model component to make the final adjustments
  // to new_vse->top_choice_flags.
  if (!curr_state->viterbi_state_entries.empty() && new_vse->top_choice_flags) {
    GenerateTopChoiceInfo(new_vse, parent_vse, curr_state);
  }

  // If language model components did not like this unichar - return.
  bool keep = new_vse->top_choice_flags || liked_by_language_model;
  if (!(top_choice_flags & kSmallestRatingFlag) &&  // no non-top choice paths
      consistency_info.inconsistent_script) {       // with inconsistent script
    keep = false;
  }
  if (!keep) {
    if (language_model_debug_level > 1) {
      tprintf("Language model components did not like this entry\n");
    }
    delete new_vse;
    return false;
  }

  // Discard this entry if it represents a prunable path and
  // language_model_viterbi_list_max_num_prunable such entries with a lower
  // cost have already been recorded.
  if (PrunablePath(*new_vse) &&
      (curr_state->viterbi_state_entries_prunable_length >=
       language_model_viterbi_list_max_num_prunable) &&
      new_vse->cost >= curr_state->viterbi_state_entries_prunable_max_cost) {
    if (language_model_debug_level > 1) {
      tprintf("Discarded ViterbiEntry with high cost %g max cost %g\n",
              new_vse->cost,
              curr_state->viterbi_state_entries_prunable_max_cost);
    }
    delete new_vse;
    return false;
  }

  // Update best choice if needed.
  if (word_end) {
    UpdateBestChoice(new_vse, pain_points, word_res,
                     best_choice_bundle, blamer_bundle);
    // Discard the entry if UpdateBestChoice() found flaws in it.
    if (new_vse->cost >= WERD_CHOICE::kBadRating &&
        new_vse != best_choice_bundle->best_vse) {
      if (language_model_debug_level > 1) {
        tprintf("Discarded ViterbiEntry with high cost %g\n", new_vse->cost);
      }
      delete new_vse;
      return false;
    }
  }

  // Add the new ViterbiStateEntry and to curr_state->viterbi_state_entries.
  curr_state->viterbi_state_entries.add_sorted(ViterbiStateEntry::Compare,
                                               false, new_vse);
  curr_state->viterbi_state_entries_length++;
  if (PrunablePath(*new_vse)) {
    curr_state->viterbi_state_entries_prunable_length++;
  }

  // Update lms->viterbi_state_entries_prunable_max_cost and clear
  // top_choice_flags of entries with ratings_sum than new_vse->ratings_sum.
  if ((curr_state->viterbi_state_entries_prunable_length >=
       language_model_viterbi_list_max_num_prunable) ||
      new_vse->top_choice_flags) {
    ASSERT_HOST(!curr_state->viterbi_state_entries.empty());
    int prunable_counter = language_model_viterbi_list_max_num_prunable;
    vit.set_to_list(&(curr_state->viterbi_state_entries));
    for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
      ViterbiStateEntry *curr_vse = vit.data();
      // Clear the appropriate top choice flags of the entries in the
      // list that have cost higher thank new_entry->cost
      // (since they will not be top choices any more).
      if (curr_vse->top_choice_flags && curr_vse != new_vse &&
          curr_vse->cost > new_vse->cost) {
        curr_vse->top_choice_flags &= ~(new_vse->top_choice_flags);
      }
      if (prunable_counter > 0 && PrunablePath(*curr_vse)) --prunable_counter;
      // Update curr_state->viterbi_state_entries_prunable_max_cost.
      if (prunable_counter == 0) {
        curr_state->viterbi_state_entries_prunable_max_cost = vit.data()->cost;
        if (language_model_debug_level > 1) {
          tprintf("Set viterbi_state_entries_prunable_max_cost to %g\n",
                  curr_state->viterbi_state_entries_prunable_max_cost);
        }
        prunable_counter = -1;  // stop counting
      }
    }
  }

  // Print the newly created ViterbiStateEntry.
  if (language_model_debug_level > 2) {
    new_vse->Print("New");
    if (language_model_debug_level > 5)
      curr_state->Print("Updated viterbi list");
  }

  return true;
}

void LanguageModel::GenerateTopChoiceInfo(ViterbiStateEntry *new_vse,
                                          const ViterbiStateEntry *parent_vse,
                                          LanguageModelState *lms) {
  ViterbiStateEntry_IT vit(&(lms->viterbi_state_entries));
  for (vit.mark_cycle_pt(); !vit.cycled_list() && new_vse->top_choice_flags &&
       new_vse->cost >= vit.data()->cost; vit.forward()) {
    // Clear the appropriate flags if the list already contains
    // a top choice entry with a lower cost.
    new_vse->top_choice_flags &= ~(vit.data()->top_choice_flags);
  }
  if (language_model_debug_level > 2) {
    tprintf("GenerateTopChoiceInfo: top_choice_flags=0x%x\n",
            new_vse->top_choice_flags);
  }
}

LanguageModelDawgInfo *LanguageModel::GenerateDawgInfo(
    bool word_end,
    int curr_col, int curr_row,
    const BLOB_CHOICE &b,
    const ViterbiStateEntry *parent_vse) {
  // Initialize active_dawgs from parent_vse if it is not NULL.
  // Otherwise use very_beginning_active_dawgs_.
  if (parent_vse == NULL) {
    dawg_args_->active_dawgs = very_beginning_active_dawgs_;
    dawg_args_->permuter = NO_PERM;
  } else {
    if (parent_vse->dawg_info == NULL) return NULL;  // not a dict word path
    dawg_args_->active_dawgs = parent_vse->dawg_info->active_dawgs;
    dawg_args_->permuter = parent_vse->dawg_info->permuter;
  }

  // Deal with hyphenated words.
  if (word_end && dict_->has_hyphen_end(b.unichar_id(), curr_col == 0)) {
    if (language_model_debug_level > 0) tprintf("Hyphenated word found\n");
    return new LanguageModelDawgInfo(dawg_args_->active_dawgs,
                                     COMPOUND_PERM);
  }

  // Deal with compound words.
  if (dict_->compound_marker(b.unichar_id()) &&
      (parent_vse == NULL || parent_vse->dawg_info->permuter != NUMBER_PERM)) {
    if (language_model_debug_level > 0) tprintf("Found compound marker\n");
    // Do not allow compound operators at the beginning and end of the word.
    // Do not allow more than one compound operator per word.
    // Do not allow compounding of words with lengths shorter than
    // language_model_min_compound_length
    if (parent_vse == NULL || word_end ||
        dawg_args_->permuter == COMPOUND_PERM ||
        parent_vse->length < language_model_min_compound_length) return NULL;

    int i;
    // Check a that the path terminated before the current character is a word.
    bool has_word_ending = false;
    for (i = 0; i < parent_vse->dawg_info->active_dawgs->size(); ++i) {
      const DawgPosition &pos = (*parent_vse->dawg_info->active_dawgs)[i];
      const Dawg *pdawg = pos.dawg_index < 0
          ? NULL : dict_->GetDawg(pos.dawg_index);
      if (pdawg == NULL || pos.back_to_punc) continue;;
      if (pdawg->type() == DAWG_TYPE_WORD && pos.dawg_ref != NO_EDGE &&
          pdawg->end_of_word(pos.dawg_ref)) {
        has_word_ending = true;
        break;
      }
    }
    if (!has_word_ending) return NULL;

    if (language_model_debug_level > 0) tprintf("Compound word found\n");
    return new LanguageModelDawgInfo(beginning_active_dawgs_, COMPOUND_PERM);
  }  // done dealing with compound words

  LanguageModelDawgInfo *dawg_info = NULL;

  // Call LetterIsOkay().
  // Use the normalized IDs so that all shapes of ' can be allowed in words
  // like don't.
  const GenericVector<UNICHAR_ID>& normed_ids =
      dict_->getUnicharset().normed_ids(b.unichar_id());
  DawgPositionVector tmp_active_dawgs;
  for (int i = 0; i < normed_ids.size(); ++i) {
    if (language_model_debug_level > 2)
      tprintf("Test Letter OK for unichar %d, normed %d\n",
              b.unichar_id(), normed_ids[i]);
    dict_->LetterIsOkay(dawg_args_, normed_ids[i],
                        word_end && i == normed_ids.size() - 1);
    if (dawg_args_->permuter == NO_PERM) {
      break;
    } else if (i < normed_ids.size() - 1) {
      tmp_active_dawgs = *dawg_args_->updated_dawgs;
      dawg_args_->active_dawgs = &tmp_active_dawgs;
    }
    if (language_model_debug_level > 2)
      tprintf("Letter was OK for unichar %d, normed %d\n",
              b.unichar_id(), normed_ids[i]);
  }
  dawg_args_->active_dawgs = NULL;
  if (dawg_args_->permuter != NO_PERM) {
    dawg_info = new LanguageModelDawgInfo(dawg_args_->updated_dawgs,
                                          dawg_args_->permuter);
  } else if (language_model_debug_level > 3) {
    tprintf("Letter %s not OK!\n",
            dict_->getUnicharset().id_to_unichar(b.unichar_id()));
  }

  return dawg_info;
}

LanguageModelNgramInfo *LanguageModel::GenerateNgramInfo(
    const char *unichar, float certainty, float denom,
    int curr_col, int curr_row, float outline_length,
    const ViterbiStateEntry *parent_vse) {
  // Initialize parent context.
  const char *pcontext_ptr = "";
  int pcontext_unichar_step_len = 0;
  if (parent_vse == NULL) {
    pcontext_ptr = prev_word_str_.string();
    pcontext_unichar_step_len = prev_word_unichar_step_len_;
  } else {
    pcontext_ptr = parent_vse->ngram_info->context.string();
    pcontext_unichar_step_len =
      parent_vse->ngram_info->context_unichar_step_len;
  }
  // Compute p(unichar | parent context).
  int unichar_step_len = 0;
  bool pruned = false;
  float ngram_cost;
  float ngram_and_classifier_cost =
      ComputeNgramCost(unichar, certainty, denom,
                       pcontext_ptr, &unichar_step_len,
                       &pruned, &ngram_cost);
  // Normalize just the ngram_and_classifier_cost by outline_length.
  // The ngram_cost is used by the params_model, so it needs to be left as-is,
  // and the params model cost will be normalized by outline_length.
  ngram_and_classifier_cost *=
      outline_length / language_model_ngram_rating_factor;
  // Add the ngram_cost of the parent.
  if (parent_vse != NULL) {
    ngram_and_classifier_cost +=
        parent_vse->ngram_info->ngram_and_classifier_cost;
    ngram_cost += parent_vse->ngram_info->ngram_cost;
  }

  // Shorten parent context string by unichar_step_len unichars.
  int num_remove = (unichar_step_len + pcontext_unichar_step_len -
                    language_model_ngram_order);
  if (num_remove > 0) pcontext_unichar_step_len -= num_remove;
  while (num_remove > 0 && *pcontext_ptr != '\0') {
    pcontext_ptr += UNICHAR::utf8_step(pcontext_ptr);
    --num_remove;
  }

  // Decide whether to prune this ngram path and update changed accordingly.
  if (parent_vse != NULL && parent_vse->ngram_info->pruned) pruned = true;

  // Construct and return the new LanguageModelNgramInfo.
  LanguageModelNgramInfo *ngram_info = new LanguageModelNgramInfo(
      pcontext_ptr, pcontext_unichar_step_len, pruned, ngram_cost,
      ngram_and_classifier_cost);
  ngram_info->context += unichar;
  ngram_info->context_unichar_step_len += unichar_step_len;
  assert(ngram_info->context_unichar_step_len <= language_model_ngram_order);
  return ngram_info;
}

float LanguageModel::ComputeNgramCost(const char *unichar,
                                      float certainty,
                                      float denom,
                                      const char *context,
                                      int *unichar_step_len,
                                      bool *found_small_prob,
                                      float *ngram_cost) {
  const char *context_ptr = context;
  char *modified_context = NULL;
  char *modified_context_end = NULL;
  const char *unichar_ptr = unichar;
  const char *unichar_end = unichar_ptr + strlen(unichar_ptr);
  float prob = 0.0f;
  int step = 0;
  while (unichar_ptr < unichar_end &&
         (step = UNICHAR::utf8_step(unichar_ptr)) > 0) {
    if (language_model_debug_level > 1) {
      tprintf("prob(%s | %s)=%g\n", unichar_ptr, context_ptr,
              dict_->ProbabilityInContext(context_ptr, -1, unichar_ptr, step));
    }
    prob += dict_->ProbabilityInContext(context_ptr, -1, unichar_ptr, step);
    ++(*unichar_step_len);
    if (language_model_ngram_use_only_first_uft8_step) break;
    unichar_ptr += step;
    // If there are multiple UTF8 characters present in unichar, context is
    // updated to include the previously examined characters from str,
    // unless use_only_first_uft8_step is true.
    if (unichar_ptr < unichar_end) {
      if (modified_context == NULL) {
        int context_len = strlen(context);
        modified_context =
          new char[context_len + strlen(unichar_ptr) + step + 1];
        strncpy(modified_context, context, context_len);
        modified_context_end = modified_context + context_len;
        context_ptr = modified_context;
      }
      strncpy(modified_context_end, unichar_ptr - step, step);
      modified_context_end += step;
      *modified_context_end = '\0';
    }
  }
  prob /= static_cast<float>(*unichar_step_len);  // normalize
  if (prob < language_model_ngram_small_prob) {
    if (language_model_debug_level > 0) tprintf("Found small prob %g\n", prob);
    *found_small_prob = true;
    prob = language_model_ngram_small_prob;
  }
  *ngram_cost = -1.0*log2(prob);
  float ngram_and_classifier_cost =
      -1.0*log2(CertaintyScore(certainty)/denom) +
      *ngram_cost * language_model_ngram_scale_factor;
  if (language_model_debug_level > 1) {
    tprintf("-log [ p(%s) * p(%s | %s) ] = -log2(%g*%g) = %g\n", unichar,
            unichar, context_ptr, CertaintyScore(certainty)/denom, prob,
            ngram_and_classifier_cost);
  }
  delete[] modified_context;
  return ngram_and_classifier_cost;
}

float LanguageModel::ComputeDenom(BLOB_CHOICE_LIST *curr_list) {
  if (curr_list->empty()) return 1.0f;
  float denom = 0.0f;
  int len = 0;
  BLOB_CHOICE_IT c_it(curr_list);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    ASSERT_HOST(c_it.data() != NULL);
    ++len;
    denom += CertaintyScore(c_it.data()->certainty());
  }
  assert(len != 0);
  // The ideal situation would be to have the classifier scores for
  // classifying each position as each of the characters in the unicharset.
  // Since we can not do this because of speed, we add a very crude estimate
  // of what these scores for the "missing" classifications would sum up to.
  denom += (dict_->getUnicharset().size() - len) *
    CertaintyScore(language_model_ngram_nonmatch_score);

  return denom;
}

void LanguageModel::FillConsistencyInfo(
    int curr_col,
    bool word_end,
    BLOB_CHOICE *b,
    ViterbiStateEntry *parent_vse,
    WERD_RES *word_res,
    LMConsistencyInfo *consistency_info) {
  const UNICHARSET &unicharset = dict_->getUnicharset();
  UNICHAR_ID unichar_id = b->unichar_id();
  BLOB_CHOICE* parent_b = parent_vse != NULL ? parent_vse->curr_b : NULL;

  // Check punctuation validity.
  if (unicharset.get_ispunctuation(unichar_id)) consistency_info->num_punc++;
  if (dict_->GetPuncDawg() != NULL && !consistency_info->invalid_punc) {
    if (dict_->compound_marker(unichar_id) && parent_b != NULL &&
        (unicharset.get_isalpha(parent_b->unichar_id()) ||
         unicharset.get_isdigit(parent_b->unichar_id()))) {
      // reset punc_ref for compound words
      consistency_info->punc_ref = NO_EDGE;
    } else {
      bool is_apos = dict_->is_apostrophe(unichar_id);
      bool prev_is_numalpha = (parent_b != NULL &&
          (unicharset.get_isalpha(parent_b->unichar_id()) ||
           unicharset.get_isdigit(parent_b->unichar_id())));
      UNICHAR_ID pattern_unichar_id =
        (unicharset.get_isalpha(unichar_id) ||
         unicharset.get_isdigit(unichar_id) ||
         (is_apos && prev_is_numalpha)) ?
        Dawg::kPatternUnicharID : unichar_id;
      if (consistency_info->punc_ref == NO_EDGE ||
          pattern_unichar_id != Dawg::kPatternUnicharID ||
          dict_->GetPuncDawg()->edge_letter(consistency_info->punc_ref) !=
          Dawg::kPatternUnicharID) {
        NODE_REF node = Dict::GetStartingNode(dict_->GetPuncDawg(),
                                              consistency_info->punc_ref);
        consistency_info->punc_ref =
          (node != NO_EDGE) ? dict_->GetPuncDawg()->edge_char_of(
              node, pattern_unichar_id, word_end) : NO_EDGE;
        if (consistency_info->punc_ref == NO_EDGE) {
          consistency_info->invalid_punc = true;
        }
      }
    }
  }

  // Update case related counters.
  if (parent_vse != NULL && !word_end && dict_->compound_marker(unichar_id)) {
    // Reset counters if we are dealing with a compound word.
    consistency_info->num_lower = 0;
    consistency_info->num_non_first_upper = 0;
  }
  else if (unicharset.get_islower(unichar_id)) {
    consistency_info->num_lower++;
  } else if ((parent_b != NULL) && unicharset.get_isupper(unichar_id)) {
    if (unicharset.get_isupper(parent_b->unichar_id()) ||
        consistency_info->num_lower > 0 ||
        consistency_info->num_non_first_upper > 0) {
      consistency_info->num_non_first_upper++;
    }
  }

  // Initialize consistency_info->script_id (use script of unichar_id
  // if it is not Common, use script id recorded by the parent otherwise).
  // Set inconsistent_script to true if the script of the current unichar
  // is not consistent with that of the parent.
  consistency_info->script_id = unicharset.get_script(unichar_id);
  // Hiragana and Katakana can mix with Han.
  if (dict_->getUnicharset().han_sid() != dict_->getUnicharset().null_sid()) {
    if ((unicharset.hiragana_sid() != unicharset.null_sid() &&
         consistency_info->script_id == unicharset.hiragana_sid()) ||
        (unicharset.katakana_sid() != unicharset.null_sid() &&
         consistency_info->script_id == unicharset.katakana_sid())) {
      consistency_info->script_id = dict_->getUnicharset().han_sid();
    }
  }

  if (parent_vse != NULL &&
      (parent_vse->consistency_info.script_id !=
       dict_->getUnicharset().common_sid())) {
    int parent_script_id = parent_vse->consistency_info.script_id;
    // If script_id is Common, use script id of the parent instead.
    if (consistency_info->script_id == dict_->getUnicharset().common_sid()) {
      consistency_info->script_id = parent_script_id;
    }
    if (consistency_info->script_id != parent_script_id) {
      consistency_info->inconsistent_script = true;
    }
  }

  // Update chartype related counters.
  if (unicharset.get_isalpha(unichar_id)) {
    consistency_info->num_alphas++;
  } else if (unicharset.get_isdigit(unichar_id)) {
    consistency_info->num_digits++;
  } else if (!unicharset.get_ispunctuation(unichar_id)) {
    consistency_info->num_other++;
  }

  // Check font and spacing consistency.
  if (fontinfo_table_->size() > 0 && parent_b != NULL) {
    int fontinfo_id = -1;
    if (parent_b->fontinfo_id() == b->fontinfo_id() ||
        parent_b->fontinfo_id2() == b->fontinfo_id()) {
      fontinfo_id = b->fontinfo_id();
    } else if (parent_b->fontinfo_id() == b->fontinfo_id2() ||
                parent_b->fontinfo_id2() == b->fontinfo_id2()) {
      fontinfo_id = b->fontinfo_id2();
    }
    if(language_model_debug_level > 1) {
      tprintf("pfont %s pfont %s font %s font2 %s common %s(%d)\n",
              (parent_b->fontinfo_id() >= 0) ?
                  fontinfo_table_->get(parent_b->fontinfo_id()).name : "" ,
              (parent_b->fontinfo_id2() >= 0) ?
                  fontinfo_table_->get(parent_b->fontinfo_id2()).name : "",
              (b->fontinfo_id() >= 0) ?
                  fontinfo_table_->get(b->fontinfo_id()).name : "",
              (fontinfo_id >= 0) ? fontinfo_table_->get(fontinfo_id).name : "",
              (fontinfo_id >= 0) ? fontinfo_table_->get(fontinfo_id).name : "",
              fontinfo_id);
    }
    if (!word_res->blob_widths.empty()) {  // if we have widths/gaps info
      bool expected_gap_found = false;
      float expected_gap;
      int temp_gap;
      if (fontinfo_id >= 0) {  // found a common font
        ASSERT_HOST(fontinfo_id < fontinfo_table_->size());
        if (fontinfo_table_->get(fontinfo_id).get_spacing(
            parent_b->unichar_id(), unichar_id, &temp_gap)) {
          expected_gap = temp_gap;
          expected_gap_found = true;
        }
      } else {
        consistency_info->inconsistent_font = true;
        // Get an average of the expected gaps in each font
        int num_addends = 0;
        expected_gap = 0;
        int temp_fid;
        for (int i = 0; i < 4; ++i) {
          if (i == 0) {
            temp_fid = parent_b->fontinfo_id();
          } else if (i == 1) {
            temp_fid = parent_b->fontinfo_id2();
          } else if (i == 2) {
            temp_fid = b->fontinfo_id();
          } else {
            temp_fid = b->fontinfo_id2();
          }
          ASSERT_HOST(temp_fid < 0 || fontinfo_table_->size());
          if (temp_fid >= 0 && fontinfo_table_->get(temp_fid).get_spacing(
              parent_b->unichar_id(), unichar_id, &temp_gap)) {
            expected_gap += temp_gap;
            num_addends++;
          }
        }
        expected_gap_found = (num_addends > 0);
        if (num_addends > 0) {
          expected_gap /= static_cast<float>(num_addends);
        }
      }
      if (expected_gap_found) {
        float actual_gap =
            static_cast<float>(word_res->GetBlobsGap(curr_col-1));
        float gap_ratio = expected_gap / actual_gap;
        // TODO(rays) The gaps seem to be way off most of the time, saved by
        // the error here that the ratio was compared to 1/2, when it should
        // have been 0.5f. Find the source of the gaps discrepancy and put
        // the 0.5f here in place of 0.0f.
        // Test on 2476595.sj, pages 0 to 6. (In French.)
        if (gap_ratio < 0.0f || gap_ratio > 2.0f) {
          consistency_info->num_inconsistent_spaces++;
        }
        if (language_model_debug_level > 1) {
          tprintf("spacing for %s(%d) %s(%d) col %d: expected %g actual %g\n",
                  unicharset.id_to_unichar(parent_b->unichar_id()),
                  parent_b->unichar_id(), unicharset.id_to_unichar(unichar_id),
                  unichar_id, curr_col, expected_gap, actual_gap);
        }
      }
    }
  }
}

float LanguageModel::ComputeAdjustedPathCost(ViterbiStateEntry *vse) {
  ASSERT_HOST(vse != NULL);
  if (params_model_.Initialized()) {
    float features[PTRAIN_NUM_FEATURE_TYPES];
    ExtractFeaturesFromPath(*vse, features);
    float cost = params_model_.ComputeCost(features);
    if (language_model_debug_level > 3) {
      tprintf("ComputeAdjustedPathCost %g ParamsModel features:\n", cost);
      if (language_model_debug_level >= 5) {
        for (int f = 0; f < PTRAIN_NUM_FEATURE_TYPES; ++f) {
          tprintf("%s=%g\n", kParamsTrainingFeatureTypeName[f], features[f]);
        }
      }
    }
    return cost * vse->outline_length;
  } else {
    float adjustment = 1.0f;
    if (vse->dawg_info == NULL || vse->dawg_info->permuter != FREQ_DAWG_PERM) {
      adjustment += language_model_penalty_non_freq_dict_word;
    }
    if (vse->dawg_info == NULL) {
      adjustment += language_model_penalty_non_dict_word;
      if (vse->length > language_model_min_compound_length) {
        adjustment += ((vse->length - language_model_min_compound_length) *
            language_model_penalty_increment);
      }
    }
    if (vse->associate_stats.shape_cost > 0) {
      adjustment += vse->associate_stats.shape_cost /
          static_cast<float>(vse->length);
    }
    if (language_model_ngram_on) {
      ASSERT_HOST(vse->ngram_info != NULL);
      return vse->ngram_info->ngram_and_classifier_cost * adjustment;
    } else {
      adjustment += ComputeConsistencyAdjustment(vse->dawg_info,
                                                 vse->consistency_info);
      return vse->ratings_sum * adjustment;
    }
  }
}

void LanguageModel::UpdateBestChoice(
    ViterbiStateEntry *vse,
    LMPainPoints *pain_points,
    WERD_RES *word_res,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  bool truth_path;
  WERD_CHOICE *word = ConstructWord(vse, word_res, &best_choice_bundle->fixpt,
                                    blamer_bundle, &truth_path);
  ASSERT_HOST(word != NULL);
  if (dict_->stopper_debug_level >= 1) {
    STRING word_str;
    word->string_and_lengths(&word_str, NULL);
    vse->Print(word_str.string());
  }
  if (language_model_debug_level > 0) {
    word->print("UpdateBestChoice() constructed word");
  }
  // Record features from the current path if necessary.
  ParamsTrainingHypothesis curr_hyp;
  if (blamer_bundle != NULL) {
    if (vse->dawg_info != NULL) vse->dawg_info->permuter =
        static_cast<PermuterType>(word->permuter());
    ExtractFeaturesFromPath(*vse, curr_hyp.features);
    word->string_and_lengths(&(curr_hyp.str), NULL);
    curr_hyp.cost = vse->cost;  // record cost for error rate computations
    if (language_model_debug_level > 0) {
      tprintf("Raw features extracted from %s (cost=%g) [ ",
              curr_hyp.str.string(), curr_hyp.cost);
      for (int deb_i = 0; deb_i < PTRAIN_NUM_FEATURE_TYPES; ++deb_i) {
        tprintf("%g ", curr_hyp.features[deb_i]);
      }
      tprintf("]\n");
    }
    // Record the current hypothesis in params_training_bundle.
    blamer_bundle->AddHypothesis(curr_hyp);
    if (truth_path)
      blamer_bundle->UpdateBestRating(word->rating());
  }
  if (blamer_bundle != NULL && blamer_bundle->GuidedSegsearchStillGoing()) {
    // The word was constructed solely for blamer_bundle->AddHypothesis, so
    // we no longer need it.
    delete word;
    return;
  }
  if (word_res->chopped_word != NULL && !word_res->chopped_word->blobs.empty())
    word->SetScriptPositions(false, word_res->chopped_word);
  // Update and log new raw_choice if needed.
  if (word_res->raw_choice == NULL ||
      word->rating() < word_res->raw_choice->rating()) {
    if (word_res->LogNewRawChoice(word) && language_model_debug_level > 0)
      tprintf("Updated raw choice\n");
  }
  // Set the modified rating for best choice to vse->cost and log best choice.
  word->set_rating(vse->cost);
  // Call LogNewChoice() for best choice from Dict::adjust_word() since it
  // computes adjust_factor that is used by the adaption code (e.g. by
  // ClassifyAdaptableWord() to compute adaption acceptance thresholds).
  // Note: the rating of the word is not adjusted.
  dict_->adjust_word(word, vse->dawg_info == NULL,
                     vse->consistency_info.xht_decision, 0.0,
                     false, language_model_debug_level > 0);
  // Hand ownership of the word over to the word_res.
  if (!word_res->LogNewCookedChoice(dict_->tessedit_truncate_wordchoice_log,
                                    dict_->stopper_debug_level >= 1, word)) {
    // The word was so bad that it was deleted.
    return;
  }
  if (word_res->best_choice == word) {
    // Word was the new best.
    if (dict_->AcceptableChoice(*word, vse->consistency_info.xht_decision) &&
        AcceptablePath(*vse)) {
      acceptable_choice_found_ = true;
    }
    // Update best_choice_bundle.
    best_choice_bundle->updated = true;
    best_choice_bundle->best_vse = vse;
    if (language_model_debug_level > 0) {
      tprintf("Updated best choice\n");
      word->print_state("New state ");
    }
    // Update hyphen state if we are dealing with a dictionary word.
    if (vse->dawg_info != NULL) {
      if (dict_->has_hyphen_end(*word)) {
        dict_->set_hyphen_word(*word, *(dawg_args_->active_dawgs));
      } else {
        dict_->reset_hyphen_vars(true);
      }
    }

    if (blamer_bundle != NULL) {
      blamer_bundle->set_best_choice_is_dict_and_top_choice(
          vse->dawg_info != NULL && vse->top_choice_flags);
    }
  }
  if (wordrec_display_segmentations && word_res->chopped_word != NULL) {
    word->DisplaySegmentation(word_res->chopped_word);
  }
}

void LanguageModel::ExtractFeaturesFromPath(
    const ViterbiStateEntry &vse, float features[]) {
  memset(features, 0, sizeof(float) * PTRAIN_NUM_FEATURE_TYPES);
  // Record dictionary match info.
  int len = vse.length <= kMaxSmallWordUnichars ? 0 :
      vse.length <= kMaxMediumWordUnichars ? 1 : 2;
  if (vse.dawg_info != NULL) {
    int permuter = vse.dawg_info->permuter;
    if (permuter == NUMBER_PERM || permuter == USER_PATTERN_PERM) {
      if (vse.consistency_info.num_digits == vse.length) {
        features[PTRAIN_DIGITS_SHORT+len] = 1.0;
      } else {
        features[PTRAIN_NUM_SHORT+len] = 1.0;
      }
    } else if (permuter == DOC_DAWG_PERM) {
      features[PTRAIN_DOC_SHORT+len] = 1.0;
    } else if (permuter == SYSTEM_DAWG_PERM || permuter == USER_DAWG_PERM ||
        permuter == COMPOUND_PERM) {
      features[PTRAIN_DICT_SHORT+len] = 1.0;
    } else if (permuter == FREQ_DAWG_PERM) {
      features[PTRAIN_FREQ_SHORT+len] = 1.0;
    }
  }
  // Record shape cost feature (normalized by path length).
  features[PTRAIN_SHAPE_COST_PER_CHAR] =
      vse.associate_stats.shape_cost / static_cast<float>(vse.length);
  // Record ngram cost. (normalized by the path length).
  features[PTRAIN_NGRAM_COST_PER_CHAR] = 0.0;
  if (vse.ngram_info != NULL) {
    features[PTRAIN_NGRAM_COST_PER_CHAR] =
        vse.ngram_info->ngram_cost / static_cast<float>(vse.length);
  }
  // Record consistency-related features.
  // Disabled this feature for due to its poor performance.
  // features[PTRAIN_NUM_BAD_PUNC] = vse.consistency_info.NumInconsistentPunc();
  features[PTRAIN_NUM_BAD_CASE] = vse.consistency_info.NumInconsistentCase();
  features[PTRAIN_XHEIGHT_CONSISTENCY] = vse.consistency_info.xht_decision;
  features[PTRAIN_NUM_BAD_CHAR_TYPE] = vse.dawg_info == NULL ?
      vse.consistency_info.NumInconsistentChartype() : 0.0;
  features[PTRAIN_NUM_BAD_SPACING] =
      vse.consistency_info.NumInconsistentSpaces();
  // Disabled this feature for now due to its poor performance.
  // features[PTRAIN_NUM_BAD_FONT] = vse.consistency_info.inconsistent_font;

  // Classifier-related features.
  features[PTRAIN_RATING_PER_CHAR] =
      vse.ratings_sum / static_cast<float>(vse.outline_length);
}

WERD_CHOICE *LanguageModel::ConstructWord(
    ViterbiStateEntry *vse,
    WERD_RES *word_res,
    DANGERR *fixpt,
    BlamerBundle *blamer_bundle,
    bool *truth_path) {
  if (truth_path != NULL) {
    *truth_path =
        (blamer_bundle != NULL &&
         vse->length == blamer_bundle->correct_segmentation_length());
  }
  BLOB_CHOICE *curr_b = vse->curr_b;
  ViterbiStateEntry *curr_vse = vse;

  int i;
  bool compound = dict_->hyphenated();  // treat hyphenated words as compound

  // Re-compute the variance of the width-to-height ratios (since we now
  // can compute the mean over the whole word).
  float full_wh_ratio_mean = 0.0f;
  if (vse->associate_stats.full_wh_ratio_var != 0.0f) {
    vse->associate_stats.shape_cost -= vse->associate_stats.full_wh_ratio_var;
    full_wh_ratio_mean = (vse->associate_stats.full_wh_ratio_total /
                          static_cast<float>(vse->length));
    vse->associate_stats.full_wh_ratio_var = 0.0f;
  }

  // Construct a WERD_CHOICE by tracing parent pointers.
  WERD_CHOICE *word = new WERD_CHOICE(word_res->uch_set, vse->length);
  word->set_length(vse->length);
  int total_blobs = 0;
  for (i = (vse->length-1); i >= 0; --i) {
    if (blamer_bundle != NULL && truth_path != NULL && *truth_path &&
        !blamer_bundle->MatrixPositionCorrect(i, curr_b->matrix_cell())) {
        *truth_path = false;
    }
    // The number of blobs used for this choice is row - col + 1.
    int num_blobs = curr_b->matrix_cell().row - curr_b->matrix_cell().col + 1;
    total_blobs += num_blobs;
    word->set_blob_choice(i, num_blobs, curr_b);
    // Update the width-to-height ratio variance. Useful non-space delimited
    // languages to ensure that the blobs are of uniform width.
    // Skip leading and trailing punctuation when computing the variance.
    if ((full_wh_ratio_mean != 0.0f &&
         ((curr_vse != vse && curr_vse->parent_vse != NULL) ||
          !dict_->getUnicharset().get_ispunctuation(curr_b->unichar_id())))) {
      vse->associate_stats.full_wh_ratio_var +=
        pow(full_wh_ratio_mean - curr_vse->associate_stats.full_wh_ratio, 2);
      if (language_model_debug_level > 2) {
        tprintf("full_wh_ratio_var += (%g-%g)^2\n",
                full_wh_ratio_mean, curr_vse->associate_stats.full_wh_ratio);
      }
    }

    // Mark the word as compound if compound permuter was set for any of
    // the unichars on the path (usually this will happen for unichars
    // that are compounding operators, like "-" and "/").
    if (!compound && curr_vse->dawg_info &&
        curr_vse->dawg_info->permuter == COMPOUND_PERM) compound = true;

    // Update curr_* pointers.
    curr_vse = curr_vse->parent_vse;
    if (curr_vse == NULL) break;
    curr_b = curr_vse->curr_b;
  }
  ASSERT_HOST(i == 0);  // check that we recorded all the unichar ids.
  ASSERT_HOST(total_blobs == word_res->ratings->dimension());
  // Re-adjust shape cost to include the updated width-to-height variance.
  if (full_wh_ratio_mean != 0.0f) {
    vse->associate_stats.shape_cost += vse->associate_stats.full_wh_ratio_var;
  }

  word->set_rating(vse->ratings_sum);
  word->set_certainty(vse->min_certainty);
  word->set_x_heights(vse->consistency_info.BodyMinXHeight(),
                      vse->consistency_info.BodyMaxXHeight());
  if (vse->dawg_info != NULL) {
    word->set_permuter(compound ? COMPOUND_PERM : vse->dawg_info->permuter);
  } else if (language_model_ngram_on && !vse->ngram_info->pruned) {
    word->set_permuter(NGRAM_PERM);
  } else if (vse->top_choice_flags) {
    word->set_permuter(TOP_CHOICE_PERM);
  } else {
    word->set_permuter(NO_PERM);
  }
  word->set_dangerous_ambig_found_(!dict_->NoDangerousAmbig(word, fixpt, true,
                                                            word_res->ratings));
  return word;
}

}  // namespace tesseract
