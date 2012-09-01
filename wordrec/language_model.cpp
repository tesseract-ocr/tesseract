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
#include "intproto.h"
#include "matrix.h"
#include "params.h"
#include "params_training_featdef.h"

namespace tesseract {

ELISTIZE(ViterbiStateEntry);

const float LanguageModel::kInitialPainPointPriorityAdjustment = 5.0f;
const float LanguageModel::kDefaultPainPointPriorityAdjustment = 2.0f;
const float LanguageModel::kBestChoicePainPointPriorityAdjustment = 0.5f;
const float LanguageModel::kCriticalPainPointPriorityAdjustment = 0.1f;
const float LanguageModel::kMaxAvgNgramCost = 25.0f;
const int LanguageModel::kMinFixedLengthDawgLength = 2;
const float LanguageModel::kLooseMaxCharWhRatio = 2.5f;

LanguageModel::LanguageModel(const UnicityTable<FontInfo> *fontinfo_table,
                             Dict *dict)
  : INT_MEMBER(language_model_debug_level, 0, "Language model debug level",
               dict->getImage()->getCCUtil()->params()),
    BOOL_INIT_MEMBER(language_model_ngram_on, false,
                     "Turn on/off the use of character ngram model",
                     dict->getImage()->getCCUtil()->params()),
    INT_MEMBER(language_model_ngram_order, 8,
               "Maximum order of the character ngram model",
               dict->getImage()->getCCUtil()->params()),
    INT_MEMBER(language_model_viterbi_list_max_num_prunable, 10,
               "Maximum number of prunable (those for which"
               " PrunablePath() is true) entries in each viterbi list"
               " recorded in BLOB_CHOICEs",
               dict->getImage()->getCCUtil()->params()),
    INT_MEMBER(language_model_viterbi_list_max_size, 500,
               "Maximum size of viterbi lists recorded in BLOB_CHOICEs",
               dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_small_prob, 0.000001,
                  "To avoid overly small denominators use this as the "
                  "floor of the probability returned by the ngram model.",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_nonmatch_score, -40.0,
                  "Average classifier score of a non-matching unichar.",
                  dict->getImage()->getCCUtil()->params()),
    BOOL_MEMBER(language_model_ngram_use_only_first_uft8_step, false,
                "Use only the first UTF8 step of the given string"
                " when computing log probabilities.",
                dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_ngram_scale_factor, 0.03,
                  "Strength of the character ngram model relative to the"
                  " character classifier ",
                  dict->getImage()->getCCUtil()->params()),
    BOOL_MEMBER(language_model_ngram_space_delimited_language, true,
                "Words are delimited by space",
                dict->getImage()->getCCUtil()->params()),
    INT_MEMBER(language_model_min_compound_length, 3,
               "Minimum length of compound words",
               dict->getImage()->getCCUtil()->params()),
    INT_MEMBER(language_model_fixed_length_choices_depth, 3,
               "Depth of blob choice lists to explore"
               " when fixed length dawgs are on",
               dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_non_freq_dict_word, 0.1,
                  "Penalty for words not in the frequent word dictionary",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_non_dict_word, 0.15,
                  "Penalty for non-dictionary words",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_punc, 0.2,
                  "Penalty for inconsistent punctuation",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_case, 0.1,
                  "Penalty for inconsistent case",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_script, 0.5,
                  "Penalty for inconsistent script",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_chartype, 0.3,
                  "Penalty for inconsistent character type",
                  dict->getImage()->getCCUtil()->params()),
    // TODO(daria, rays): enable font consistency checking
    // after improving font analysis.
    double_MEMBER(language_model_penalty_font, 0.00,
                  "Penalty for inconsistent font",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_spacing, 0.05,
                  "Penalty for inconsistent spacing",
                  dict->getImage()->getCCUtil()->params()),
    double_MEMBER(language_model_penalty_increment, 0.01,
                  "Penalty increment",
                  dict->getImage()->getCCUtil()->params()),
    BOOL_INIT_MEMBER(language_model_use_sigmoidal_certainty, false,
                     "Use sigmoidal score for certainty",
                     dict->getImage()->getCCUtil()->params()),
  fontinfo_table_(fontinfo_table), dict_(dict),
  fixed_pitch_(false), max_char_wh_ratio_(0.0),
  acceptable_choice_found_(false) {
  ASSERT_HOST(dict_ != NULL);
  dawg_args_ = new DawgArgs(NULL, NULL, new DawgInfoVector(),
                            new DawgInfoVector(),
                            0.0, NO_PERM, kAnyWordLength, -1);
  beginning_active_dawgs_ = new DawgInfoVector();
  beginning_constraints_ = new DawgInfoVector();
  fixed_length_beginning_active_dawgs_ = new DawgInfoVector();
  empty_dawg_info_vec_ = new DawgInfoVector();
}

LanguageModel::~LanguageModel() {
  delete beginning_active_dawgs_;
  delete beginning_constraints_;
  delete fixed_length_beginning_active_dawgs_;
  delete empty_dawg_info_vec_;
  delete dawg_args_->updated_active_dawgs;
  delete dawg_args_->updated_constraints;
  delete dawg_args_;
}

void LanguageModel::InitForWord(
    const WERD_CHOICE *prev_word,
    bool fixed_pitch, float best_choice_cert, float max_char_wh_ratio,
    float rating_cert_scale, HEAP *pain_points, CHUNKS_RECORD *chunks_record,
    BlamerBundle *blamer_bundle, bool debug_blamer) {
  fixed_pitch_ = fixed_pitch;
  max_char_wh_ratio_ = max_char_wh_ratio;
  rating_cert_scale_ = rating_cert_scale;
  acceptable_choice_found_ = false;
  correct_segmentation_explored_ = false;

  // For each cell, generate a "pain point" if the cell is not classified
  // and has a left or right neighbor that was classified.
  MATRIX *ratings = chunks_record->ratings;
  for (int col = 0; col < ratings->dimension(); ++col) {
    for (int row = col+1; row < ratings->dimension(); ++row) {
      if ((row > 0 && ratings->get(col, row-1) != NOT_CLASSIFIED) ||
          (col+1 < ratings->dimension() &&
           ratings->get(col+1, row) != NOT_CLASSIFIED)) {
        float worst_piece_cert;
        bool fragmented;
        GetWorstPieceCertainty(col, row, chunks_record->ratings,
                               &worst_piece_cert, &fragmented);
        GeneratePainPoint(col, row, true, kInitialPainPointPriorityAdjustment,
                          worst_piece_cert, fragmented, best_choice_cert,
                          max_char_wh_ratio_, NULL, NULL,
                          chunks_record, pain_points);
      }
    }
  }

  // Initialize vectors with beginning DawgInfos.
  beginning_active_dawgs_->clear();
  dict_->init_active_dawgs(kAnyWordLength, beginning_active_dawgs_, false);
  beginning_constraints_->clear();
  dict_->init_constraints(beginning_constraints_);
  if (dict_->GetMaxFixedLengthDawgIndex() >= 0) {
    fixed_length_beginning_active_dawgs_->clear();
    for (int i = 0; i < beginning_active_dawgs_->size(); ++i) {
      int dawg_index = (*beginning_active_dawgs_)[i].dawg_index;
      if (dawg_index <= dict_->GetMaxFixedLengthDawgIndex() &&
          dawg_index >= kMinFixedLengthDawgLength) {
        *fixed_length_beginning_active_dawgs_ += (*beginning_active_dawgs_)[i];
      }
    }
  }

  max_penalty_adjust_ = (dict_->segment_penalty_dict_nonword -
                         dict_->segment_penalty_dict_case_ok);

  // Fill prev_word_str_ with the last language_model_ngram_order
  // unichars from prev_word.
  if (language_model_ngram_on) {
    if (prev_word != NULL && prev_word->unichar_string() != NULL) {
      prev_word_str_ = prev_word->unichar_string();
      if (language_model_ngram_space_delimited_language) prev_word_str_ += ' ';
    } else {
      prev_word_str_ += ' ';
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

  // Initialize blamer-related information: map character boxes recorded in
  // blamer_bundle->norm_truth_word to the corresponding i,j indices in the
  // ratings matrix. We expect this step to succeed, since when running the
  // chopper we checked that the correct chops are present.
  if (blamer_bundle != NULL &&
      blamer_bundle->incorrect_result_reason == IRR_CORRECT &&
      blamer_bundle->truth_has_char_boxes) {
    STRING blamer_debug;
    blamer_debug += "Blamer computing correct_segmentation_cols\n";
    int curr_box_col = 0;
    int next_box_col = 0;
    TBLOB *blob = chunks_record->chunks;
    inT16 next_box_x = (blob != NULL) ? blob->bounding_box().right() : 0;
    for (int truth_idx = 0;
        blob != NULL && truth_idx < blamer_bundle->norm_truth_word.length();
        blob = blob->next) {
      ++next_box_col;
      inT16 curr_box_x = next_box_x;
      if (blob->next != NULL) next_box_x = blob->next->bounding_box().right();
      inT16 truth_x = blamer_bundle->norm_truth_word.BlobBox(truth_idx).right();
      blamer_debug.add_str_int("Box x coord vs. truth: ", curr_box_x);
      blamer_debug.add_str_int(" ", truth_x);
      blamer_debug += "\n";
      if (curr_box_x > (truth_x + blamer_bundle->norm_box_tolerance)) {
        break;  // failed to find a matching box
      } else if (curr_box_x >=
                  (truth_x - blamer_bundle->norm_box_tolerance) &&  // matched
                  (blob->next == NULL ||  // next box can't be included
                   next_box_x > truth_x + blamer_bundle->norm_box_tolerance)) {
        blamer_bundle->correct_segmentation_cols.push_back(curr_box_col);
        blamer_bundle->correct_segmentation_rows.push_back(next_box_col-1);
        ++truth_idx;
        blamer_debug.add_str_int("col=", curr_box_col);
        blamer_debug.add_str_int(" row=", next_box_col-1);
        blamer_debug += "\n";
        curr_box_col = next_box_col;
      }
    }
    if (blob != NULL ||  // trailing blobs
        blamer_bundle->correct_segmentation_cols.length() !=
        blamer_bundle->norm_truth_word.length()) {
      blamer_debug.add_str_int("Blamer failed to find correct segmentation"
          " (tolerance=", blamer_bundle->norm_box_tolerance);
      if (blob == NULL) blamer_debug += " blob == NULL";
      blamer_debug += ")\n";
      blamer_debug.add_str_int(
          " path length ", blamer_bundle->correct_segmentation_cols.length());
      blamer_debug.add_str_int(" vs. truth ",
                               blamer_bundle->norm_truth_word.length());
      blamer_debug += "\n";
      blamer_bundle->SetBlame(IRR_UNKNOWN, blamer_debug, NULL, debug_blamer);
      blamer_bundle->correct_segmentation_cols.clear();
      blamer_bundle->correct_segmentation_rows.clear();
    }
  } // end if (blamer_bundle != NULL)

  // Start a new hypothesis list for this run of segmentation search.
  if (blamer_bundle != NULL) {
    blamer_bundle->params_training_bundle.StartHypothesisList();
  }
}

void LanguageModel::CleanUp() {
  for (int i = 0; i < updated_flags_.size(); ++i) {
    *(updated_flags_[i]) = false;
  }
  updated_flags_.clear();
}

void LanguageModel::DeleteState(BLOB_CHOICE_LIST *choices) {
  BLOB_CHOICE_IT b_it(choices);
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    if (b_it.data()->language_model_state() != NULL) {
      LanguageModelState *state = reinterpret_cast<LanguageModelState *>(
          b_it.data()->language_model_state());
      delete state;
      b_it.data()->set_language_model_state(NULL);
    }
  }
}

LanguageModelFlagsType LanguageModel::UpdateState(
    LanguageModelFlagsType changed,
    int curr_col, int curr_row,
    BLOB_CHOICE_LIST *curr_list,
    BLOB_CHOICE_LIST *parent_list,
    HEAP *pain_points,
    BestPathByColumn *best_path_by_column[],
    CHUNKS_RECORD *chunks_record,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  if (language_model_debug_level > 0) {
    tprintf("\nUpdateState: col=%d row=%d (changed=0x%x parent=%p)\n",
            curr_col, curr_row, changed, parent_list);
  }
  // Initialize helper variables.
  bool word_end = (curr_row+1 >= chunks_record->ratings->dimension());
  bool just_classified = (changed & kJustClassifiedFlag);
  LanguageModelFlagsType new_changed = 0x0;
  float denom = (language_model_ngram_on) ? ComputeDenom(curr_list) : 1.0f;

  // Call AddViterbiStateEntry() for each parent+child ViterbiStateEntry.
  ViterbiStateEntry_IT vit;
  BLOB_CHOICE_IT c_it(curr_list);
  int c_it_counter = 0;
  bool first_iteration = true;
  BLOB_CHOICE *first_lower = NULL;
  BLOB_CHOICE *first_upper = NULL;
  GetTopChoiceLowerUpper(changed, curr_list, &first_lower, &first_upper);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    if (dict_->GetMaxFixedLengthDawgIndex() >= 0 &&
        c_it_counter++ >= language_model_fixed_length_choices_depth) {
      break;
    }
    // Skip NULL unichars unless it is the only choice.
    if (!curr_list->singleton() && c_it.data()->unichar_id() == 0) continue;
    if (dict_->getUnicharset().get_fragment(c_it.data()->unichar_id())) {
      continue;  // skip fragments
    }
    // Set top choice flags.
    LanguageModelFlagsType top_choice_flags = 0x0;
    if (first_iteration && (changed | kSmallestRatingFlag)) {
      top_choice_flags |= kSmallestRatingFlag;
    }
    if (first_lower == c_it.data()) top_choice_flags |= kLowerCaseFlag;
    if (first_upper == c_it.data()) top_choice_flags |= kUpperCaseFlag;

    if (parent_list == NULL) {  // process the beginning of a word
      new_changed |= AddViterbiStateEntry(
          top_choice_flags, denom, word_end, curr_col, curr_row,
          c_it.data(), NULL, NULL, pain_points, best_path_by_column,
          chunks_record, best_choice_bundle, blamer_bundle);
    } else {  // get viterbi entries from each of the parent BLOB_CHOICEs
      BLOB_CHOICE_IT p_it(parent_list);
      for (p_it.mark_cycle_pt(); !p_it.cycled_list(); p_it.forward()) {
        LanguageModelState *parent_lms =
          reinterpret_cast<LanguageModelState *>(
              p_it.data()->language_model_state());
        if (parent_lms == NULL || parent_lms->viterbi_state_entries.empty()) {
          continue;
        }
        vit.set_to_list(&(parent_lms->viterbi_state_entries));
        int vit_counter = 0;
        for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
          // Skip pruned entries and do not look at prunable entries if already
          // examined language_model_viterbi_list_max_num_prunable of those.
          if (PrunablePath(vit.data()->top_choice_flags,
                           vit.data()->dawg_info) &&
              (++vit_counter > language_model_viterbi_list_max_num_prunable ||
               (language_model_ngram_on && vit.data()->ngram_info->pruned))) {
            continue;
          }
          // Only consider the parent if it has been updated or
          // if the current ratings cell has just been classified.
          if (!just_classified && !vit.data()->updated) continue;
          // Create a new ViterbiStateEntry if BLOB_CHOICE in c_it.data()
          // looks good according to the Dawgs or character ngram model.
          new_changed |= AddViterbiStateEntry(
              top_choice_flags, denom, word_end, curr_col, curr_row,
              c_it.data(), p_it.data(), vit.data(), pain_points,
              best_path_by_column, chunks_record,
              best_choice_bundle, blamer_bundle);
        }
      }  // done looking at parents for this c_it.data()
    }
    first_iteration = false;
  }
  return new_changed;
}

bool LanguageModel::ProblematicPath(const ViterbiStateEntry &vse,
                                    UNICHAR_ID unichar_id, bool word_end) {
  // The path is problematic if it is inconsistent and has a parent that
  // is consistent (or a NULL parent).
  if (!vse.Consistent() && (vse.parent_vse == NULL ||
                            vse.parent_vse->Consistent())) {
    if (language_model_debug_level > 0) {
      tprintf("ProblematicPath: inconsistent\n");
    }
    return true;
  }
  // The path is problematic if it does not represent a dictionary word,
  // while its parent does.
  if (vse.dawg_info == NULL &&
      (vse.parent_vse == NULL || vse.parent_vse->dawg_info != NULL)) {
    if (language_model_debug_level > 0) {
      tprintf("ProblematicPath: dict word terminated\n");
    }
    return true;
  }
  // The path is problematic if ngram info indicates that this path is
  // an unlikely sequence of character, while its parent is does not have
  // extreme dips in ngram probabilities.
  if (vse.ngram_info != NULL && vse.ngram_info->pruned &&
      (vse.parent_vse == NULL || !vse.parent_vse->ngram_info->pruned)) {
    if (language_model_debug_level > 0) {
      tprintf("ProblematicPath: small ngram prob\n");
    }
    return true;
  }
  // The path is problematic if there is a non-alpha character in the
  // middle of the path (unless it is a digit in the middle of a path
  // that represents a number).
  if ((vse.parent_vse != NULL) && !word_end &&              // is middle
      !(dict_->getUnicharset().get_isalpha(unichar_id) ||   // alpha
        (dict_->getUnicharset().get_isdigit(unichar_id) &&  // ok digit
         vse.dawg_info != NULL && vse.dawg_info->permuter == NUMBER_PERM))) {
    if (language_model_debug_level > 0) {
      tprintf("ProblematicPath: non-alpha middle\n");
    }
    return true;
  }
  return false;
}

void LanguageModel::GetTopChoiceLowerUpper(LanguageModelFlagsType changed,
                                           BLOB_CHOICE_LIST *curr_list,
                                           BLOB_CHOICE **first_lower,
                                           BLOB_CHOICE **first_upper) {
  if (!(changed & kLowerCaseFlag || changed & kUpperCaseFlag)) return;
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
    if (*first_upper == NULL && unicharset.get_isupper(unichar_id)) {
      *first_upper = c_it.data();
    }
  }
  ASSERT_HOST(first_unichar != NULL);
  if (*first_lower == NULL) *first_lower = first_unichar;
  if (*first_upper == NULL) *first_upper = first_unichar;
}

LanguageModelFlagsType LanguageModel::AddViterbiStateEntry(
    LanguageModelFlagsType top_choice_flags,
    float denom,
    bool word_end,
    int curr_col, int curr_row,
    BLOB_CHOICE *b,
    BLOB_CHOICE *parent_b,
    ViterbiStateEntry *parent_vse,
    HEAP *pain_points,
    BestPathByColumn *best_path_by_column[],
    CHUNKS_RECORD *chunks_record,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  ViterbiStateEntry_IT vit;
  if (language_model_debug_level > 0) {
    tprintf("\nAddViterbiStateEntry for unichar %s rating=%.4f"
            " certainty=%.4f top_choice_flags=0x%x parent_vse=%p\n",
            dict_->getUnicharset().id_to_unichar(b->unichar_id()),
            b->rating(), b->certainty(), top_choice_flags, parent_vse);
    if (language_model_debug_level > 3 && b->language_model_state() != NULL) {
      tprintf("Existing viterbi list:\n");
      vit.set_to_list(&(reinterpret_cast<LanguageModelState *>(
          b->language_model_state())->viterbi_state_entries));
      for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
        PrintViterbiStateEntry("", vit.data(), b, chunks_record);
      }
    }
  }
  // Check whether the list is full.
  if (b->language_model_state() != NULL &&
      (reinterpret_cast<LanguageModelState *>(
          b->language_model_state())->viterbi_state_entries_length >=
          language_model_viterbi_list_max_size)) {
    if (language_model_debug_level > 1) {
      tprintf("AddViterbiStateEntry: viterbi list is full!\n");
    }
    return 0x0;
  }

  LanguageModelFlagsType changed = 0x0;
  float optimistic_cost = 0.0f;
  if (!language_model_ngram_on) optimistic_cost += b->rating();
  if (parent_vse != NULL) optimistic_cost += parent_vse->cost;
  // Discard this entry if it will not beat best choice.
  if (optimistic_cost >= best_choice_bundle->best_choice->rating()) {
    if (language_model_debug_level > 1) {
      tprintf("Discarded ViterbiEntry with high cost %.4f"
              " best_choice->rating()=%.4f\n", optimistic_cost,
              best_choice_bundle->best_choice->rating());
    }
    return 0x0;
  }

  // Check consistency of the path and set the relevant consistency_info.
  LanguageModelConsistencyInfo consistency_info;
  FillConsistencyInfo(curr_col, word_end, b, parent_vse, parent_b,
                      chunks_record, &consistency_info);

  // Invoke Dawg language model component.
  LanguageModelDawgInfo *dawg_info =
    GenerateDawgInfo(word_end, consistency_info.script_id,
                     curr_col, curr_row, *b, parent_vse, &changed);

  // Invoke TopChoice language model component
  float ratings_sum = b->rating();
  if (parent_vse != NULL) ratings_sum += parent_vse->ratings_sum;
  GenerateTopChoiceInfo(ratings_sum, dawg_info, consistency_info,
                        parent_vse, b, &top_choice_flags, &changed);

  // Invoke Ngram language model component.
  LanguageModelNgramInfo *ngram_info = NULL;
  if (language_model_ngram_on) {
    ngram_info = GenerateNgramInfo(
        dict_->getUnicharset().id_to_unichar(b->unichar_id()), b->certainty(),
        denom, curr_col, curr_row, parent_vse, parent_b, &changed);
    ASSERT_HOST(ngram_info != NULL);
  }

  // Prune non-top choice paths with inconsistent scripts.
  if (consistency_info.inconsistent_script) {
    if (!(top_choice_flags & kSmallestRatingFlag)) changed = 0x0;
    if (ngram_info != NULL) ngram_info->pruned = true;
  }

  // If language model components did not like this unichar - return
  if (!changed) {
    if (language_model_debug_level > 0) {
      tprintf("Language model components did not like this entry\n");
    }
    delete dawg_info;
    delete ngram_info;
    return 0x0;
  }

  // Compute cost of associating the blobs that represent the current unichar.
  AssociateStats associate_stats;
  ComputeAssociateStats(curr_col, curr_row, max_char_wh_ratio_,
                        parent_vse, chunks_record, &associate_stats);
  if (parent_vse != NULL) {
    associate_stats.shape_cost += parent_vse->associate_stats.shape_cost;
    associate_stats.bad_shape |= parent_vse->associate_stats.bad_shape;
  }

  // Compute the aggregate cost (adjusted ratings sum).
  float cost = ComputeAdjustedPathCost(
      ratings_sum,
      (parent_vse == NULL) ? 1 : (parent_vse->length+1),
      (dawg_info == NULL) ? 0.0f : 1.0f,
      dawg_info, ngram_info, consistency_info, associate_stats, parent_vse);

  if (b->language_model_state() == NULL) {
    b->set_language_model_state(new LanguageModelState(curr_col, curr_row));
  }
  LanguageModelState *lms =
    reinterpret_cast<LanguageModelState *>(b->language_model_state());

  // Discard this entry if it represents a prunable path and
  // language_model_viterbi_list_max_num_prunable such entries with a lower
  // cost have already been recorded.
  if (PrunablePath(top_choice_flags, dawg_info) &&
      (lms->viterbi_state_entries_prunable_length >=
       language_model_viterbi_list_max_num_prunable) &&
      cost >= lms->viterbi_state_entries_prunable_max_cost) {
    if (language_model_debug_level > 1) {
      tprintf("Discarded ViterbiEntry with high cost %g max cost %g\n",
              cost, lms->viterbi_state_entries_prunable_max_cost);
    }
    delete dawg_info;
    delete ngram_info;
    return 0x0;
  }

  // Create the new ViterbiStateEntry and add it to lms->viterbi_state_entries
  ViterbiStateEntry *new_vse = new ViterbiStateEntry(
      parent_b, parent_vse, b, cost, ComputeOutlineLength(b), consistency_info,
      associate_stats, top_choice_flags, dawg_info, ngram_info);
  updated_flags_.push_back(&(new_vse->updated));
  lms->viterbi_state_entries.add_sorted(ViterbiStateEntry::Compare,
                                        false, new_vse);
  lms->viterbi_state_entries_length++;
  if (PrunablePath(top_choice_flags, dawg_info)) {
    lms->viterbi_state_entries_prunable_length++;
  }

  // Update lms->viterbi_state_entries_prunable_max_cost and clear
  // top_choice_flags of entries with ratings_sum than new_vse->ratings_sum.
  if ((lms->viterbi_state_entries_prunable_length >=
       language_model_viterbi_list_max_num_prunable) || top_choice_flags) {
    ASSERT_HOST(!lms->viterbi_state_entries.empty());
    int prunable_counter = language_model_viterbi_list_max_num_prunable;
    vit.set_to_list(&(lms->viterbi_state_entries));
    for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
      ViterbiStateEntry *curr_vse = vit.data();
      // Clear the appropriate top choice flags of the entries in the
      // list that have ratings_sum higher thank new_entry->ratings_sum
      // (since they will not be top choices any more).
      if (curr_vse->top_choice_flags && curr_vse != new_vse &&
          ComputeConsistencyAdjustedRatingsSum(
              curr_vse->ratings_sum, curr_vse->dawg_info,
              curr_vse->consistency_info) >
          ComputeConsistencyAdjustedRatingsSum(
              new_vse->ratings_sum, new_vse->dawg_info,
              new_vse->consistency_info)) {
        curr_vse->top_choice_flags &= ~(top_choice_flags);
      }
      if (prunable_counter > 0 &&
          PrunablePath(curr_vse->top_choice_flags, curr_vse->dawg_info)) {
        --prunable_counter;
      }
      // Update lms->viterbi_state_entries_prunable_max_cost.
      if (prunable_counter == 0) {
        lms->viterbi_state_entries_prunable_max_cost = vit.data()->cost;
        if (language_model_debug_level > 1) {
          tprintf("Set viterbi_state_entries_prunable_max_cost to %.4f\n",
                  lms->viterbi_state_entries_prunable_max_cost);
        }
        prunable_counter = -1;  // stop counting
      }
    }
  }

  // Print the newly created ViterbiStateEntry.
  if (language_model_debug_level > 2) {
    PrintViterbiStateEntry("New", new_vse, b, chunks_record);
    if (language_model_debug_level > 3) {
      tprintf("Updated viterbi list (max cost %g):\n",
              lms->viterbi_state_entries_prunable_max_cost);
      vit.set_to_list(&(lms->viterbi_state_entries));
      for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
        PrintViterbiStateEntry("", vit.data(), b, chunks_record);
      }
    }
  }

  // Update best choice if needed.
  if (word_end) {
    UpdateBestChoice(b, new_vse, pain_points, chunks_record,
                     best_choice_bundle, blamer_bundle);
  }

  // Update stats in best_path_by_column.
  if (new_vse->Consistent() || new_vse->dawg_info != NULL ||
      (new_vse->ngram_info != NULL && !new_vse->ngram_info->pruned)) {
    float avg_cost = new_vse->cost / static_cast<float>(curr_row+1);
    for (int c = curr_col; c <= curr_row; ++c) {
      if (avg_cost < (*best_path_by_column)[c].avg_cost) {
        (*best_path_by_column)[c].avg_cost = avg_cost;
        (*best_path_by_column)[c].best_vse = new_vse;
        (*best_path_by_column)[c].best_b = b;
        if (language_model_debug_level > 0) {
          tprintf("Set best_path_by_column[%d]=(%g %p)\n",
                  c, avg_cost, new_vse);
        }
      }
    }
  }
  return changed;
}

void LanguageModel::PrintViterbiStateEntry(
    const char *msg, ViterbiStateEntry *vse,
    BLOB_CHOICE *b, CHUNKS_RECORD *chunks_record) {
  tprintf("%s ViterbiStateEntry %p with ratings_sum=%.4f length=%d cost=%.4f",
          msg, vse, vse->ratings_sum, vse->length, vse->cost);
  if (vse->top_choice_flags) {
    tprintf(" top_choice_flags=0x%x", vse->top_choice_flags);
  }
  if (!vse->Consistent()) {
    tprintf(" inconsistent=(punc %d case %d chartype %d script %d)\n",
            vse->consistency_info.NumInconsistentPunc(),
            vse->consistency_info.NumInconsistentCase(),
            vse->consistency_info.NumInconsistentChartype(),
            vse->consistency_info.inconsistent_script);
  }
  if (vse->dawg_info) tprintf(" permuter=%d", vse->dawg_info->permuter);
  if (vse->ngram_info) {
    tprintf(" ngram_cost=%g context=%s ngram pruned=%d",
            vse->ngram_info->ngram_cost,
            vse->ngram_info->context.string(),
            vse->ngram_info->pruned);
  }
  if (vse->associate_stats.shape_cost > 0.0f) {
    tprintf(" shape_cost=%g", vse->associate_stats.shape_cost);
  }
  if (language_model_debug_level > 3) {
    STRING wd_str;
    WERD_CHOICE *wd = ConstructWord(b, vse, chunks_record,
                                    NULL, NULL, NULL, NULL, NULL, NULL);
    wd->string_and_lengths(&wd_str, NULL);
    delete wd;
    tprintf(" str=%s", wd_str.string());
  }
  tprintf("\n");
}

void LanguageModel::GenerateTopChoiceInfo(
    float ratings_sum,
    const LanguageModelDawgInfo *dawg_info,
    const LanguageModelConsistencyInfo &consistency_info,
    const ViterbiStateEntry *parent_vse,
    BLOB_CHOICE *b,
    LanguageModelFlagsType *top_choice_flags,
    LanguageModelFlagsType *changed) {
  ratings_sum = ComputeConsistencyAdjustedRatingsSum(
      ratings_sum, dawg_info, consistency_info);
  // Clear flags that do not agree with parent_vse->top_choice_flags.
  if (parent_vse != NULL) *top_choice_flags &= parent_vse->top_choice_flags;
  if (consistency_info.Consistent()) *top_choice_flags |= kConsistentFlag;
  if (*top_choice_flags == 0x0) return;
  LanguageModelState *lms =
    reinterpret_cast<LanguageModelState *>(b->language_model_state());
  if (lms != NULL && !lms->viterbi_state_entries.empty()) {
    ViterbiStateEntry_IT vit(&(lms->viterbi_state_entries));
    for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
      if (ratings_sum >= ComputeConsistencyAdjustedRatingsSum(
          vit.data()->ratings_sum, vit.data()->dawg_info,
          vit.data()->consistency_info)) {
        // Clear the appropriate flags if the list already contains
        // a top choice entry with a lower cost.
        *top_choice_flags &= ~(vit.data()->top_choice_flags);
      }
    }
  }
  if (language_model_debug_level > 0) {
    tprintf("GenerateTopChoiceInfo: top_choice_flags=0x%x\n",
            *top_choice_flags);
  }
  *changed |= *top_choice_flags;
}

LanguageModelDawgInfo *LanguageModel::GenerateDawgInfo(
    bool word_end, int script_id,
    int curr_col, int curr_row,
    const BLOB_CHOICE &b,
    const ViterbiStateEntry *parent_vse,
    LanguageModelFlagsType *changed) {
  bool use_fixed_length_dawgs = !fixed_length_beginning_active_dawgs_->empty();

  // Initialize active_dawgs and constraints from parent_vse if it is not NULL,
  // otherwise use beginning_active_dawgs_ and beginning_constraints_.
  if (parent_vse == NULL ||
      (use_fixed_length_dawgs && parent_vse->dawg_info == NULL)) {
    dawg_args_->active_dawgs = beginning_active_dawgs_;
    dawg_args_->constraints = beginning_constraints_;
    dawg_args_->permuter = NO_PERM;
  } else {
    if (parent_vse->dawg_info == NULL) return NULL;  // not a dict word path
    dawg_args_->active_dawgs = parent_vse->dawg_info->active_dawgs;
    dawg_args_->constraints = parent_vse->dawg_info->constraints;
    dawg_args_->permuter = parent_vse->dawg_info->permuter;
  }

  // Deal with hyphenated words.
  if (!use_fixed_length_dawgs && word_end &&
      dict_->has_hyphen_end(b.unichar_id(), curr_col == 0)) {
    if (language_model_debug_level > 0) tprintf("Hyphenated word found\n");
    *changed |= kDawgFlag;
    return new LanguageModelDawgInfo(dawg_args_->active_dawgs,
                                     dawg_args_->constraints,
                                     COMPOUND_PERM);
  }

  // Deal with compound words.
  if (!use_fixed_length_dawgs && dict_->compound_marker(b.unichar_id()) &&
      (parent_vse == NULL || parent_vse->dawg_info->permuter != NUMBER_PERM)) {
    if (language_model_debug_level > 0) tprintf("Found compound marker");
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
      const DawgInfo &info = (*parent_vse->dawg_info->active_dawgs)[i];
      const Dawg *pdawg = dict_->GetDawg(info.dawg_index);
      assert(pdawg != NULL);
      if (pdawg->type() == DAWG_TYPE_WORD && info.ref != NO_EDGE &&
          pdawg->end_of_word(info.ref)) {
        has_word_ending = true;
        break;
      }
    }
    if (!has_word_ending) return NULL;

    // Return LanguageModelDawgInfo with active_dawgs set to
    // the beginning dawgs of type DAWG_TYPE_WORD.
    if (language_model_debug_level > 0) tprintf("Compound word found\n");
    DawgInfoVector beginning_word_dawgs;
    for (i = 0; i < beginning_active_dawgs_->size(); ++i) {
      const Dawg *bdawg =
        dict_->GetDawg((*beginning_active_dawgs_)[i].dawg_index);
      if (bdawg->type() == DAWG_TYPE_WORD) {
        beginning_word_dawgs += (*beginning_active_dawgs_)[i];
      }
    }
    *changed |= kDawgFlag;
    return new LanguageModelDawgInfo(&(beginning_word_dawgs),
                                     dawg_args_->constraints,
                                     COMPOUND_PERM);
  }  // done dealing with compound words

  LanguageModelDawgInfo *dawg_info = NULL;

  // Call LetterIsOkay().
  dict_->LetterIsOkay(dawg_args_, b.unichar_id(), word_end);
  if (dawg_args_->permuter != NO_PERM) {
    *changed |= kDawgFlag;
    dawg_info = new LanguageModelDawgInfo(dawg_args_->updated_active_dawgs,
                                          dawg_args_->updated_constraints,
                                          dawg_args_->permuter);
  }

  // For non-space delimited languages: since every letter could be
  // a valid word, a new word could start at every unichar. Thus append
  // fixed_length_beginning_active_dawgs_ to dawg_info->active_dawgs.
  if (use_fixed_length_dawgs) {
    if (dawg_info == NULL) {
      *changed |= kDawgFlag;
      dawg_info = new LanguageModelDawgInfo(
          fixed_length_beginning_active_dawgs_,
          empty_dawg_info_vec_, SYSTEM_DAWG_PERM);
    } else {
      *(dawg_info->active_dawgs) += *(fixed_length_beginning_active_dawgs_);
    }
  }  // done dealing with fixed-length dawgs

  return dawg_info;
}

LanguageModelNgramInfo *LanguageModel::GenerateNgramInfo(
    const char *unichar, float certainty, float denom,
    int curr_col, int curr_row,
    const ViterbiStateEntry *parent_vse,
    BLOB_CHOICE *parent_b,
    LanguageModelFlagsType *changed) {
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
  float ngram_prob;
  float ngram_cost = ComputeNgramCost(unichar, certainty, denom,
                                      pcontext_ptr, &unichar_step_len,
                                      &pruned, &ngram_prob);
  // First attempt to normalize ngram_cost for strings of different
  // lengths - we multiply ngram_cost by P(char | context) as many times
  // as the number of chunks occupied by char. This makes the ngram costs
  // of all the paths ending at the current BLOB_CHOICE comparable.
  // TODO(daria): it would be worth looking at different ways of normalization.
  if (curr_row > curr_col) {
    ngram_cost += (curr_row - curr_col) * ngram_cost;
    ngram_prob += (curr_row - curr_col) * ngram_prob;
  }
  // Add the ngram_cost of the parent.
  if (parent_vse != NULL) {
    ngram_cost += parent_vse->ngram_info->ngram_cost;
    ngram_prob += parent_vse->ngram_info->ngram_prob;
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
  if (!pruned) *changed |= kNgramFlag;

  // Construct and return the new LanguageModelNgramInfo.
  LanguageModelNgramInfo *ngram_info = new LanguageModelNgramInfo(
      pcontext_ptr, pcontext_unichar_step_len, pruned, ngram_prob, ngram_cost);
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
                                      float *ngram_prob) {
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
  }
  *ngram_prob = -1.0*log(prob);
  float cost = -1.0*log(CertaintyScore(certainty)/denom) +
      *ngram_prob * language_model_ngram_scale_factor;
  if (language_model_debug_level > 1) {
    tprintf("-log [ p(%s) * p(%s | %s) ] = -log(%g*%g) = %g\n", unichar,
            unichar, context_ptr, CertaintyScore(certainty)/denom, prob, cost);
  }
  if (modified_context != NULL) delete[] modified_context;
  return cost;
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
    BLOB_CHOICE *parent_b,
    CHUNKS_RECORD *chunks_record,
    LanguageModelConsistencyInfo *consistency_info) {
  const UNICHARSET &unicharset = dict_->getUnicharset();
  UNICHAR_ID unichar_id = b->unichar_id();
  if (parent_vse != NULL) *consistency_info = parent_vse->consistency_info;

  // Check punctuation validity.
  if (unicharset.get_ispunctuation(unichar_id)) consistency_info->num_punc++;
  if (dict_->GetPuncDawg() != NULL && !consistency_info->invalid_punc) {
    if (dict_->compound_marker(unichar_id) && parent_b != NULL &&
        (unicharset.get_isalpha(parent_b->unichar_id()) ||
         unicharset.get_isdigit(parent_b->unichar_id()))) {
      // reset punc_ref for compound words
      consistency_info->punc_ref = NO_EDGE;
    } else {
      UNICHAR_ID pattern_unichar_id =
        (unicharset.get_isalpha(unichar_id) ||
         unicharset.get_isdigit(unichar_id)) ?
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
  if (parent_b != NULL) {
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
    bool expected_gap_found = false;
    float expected_gap;
    int temp_gap;
    if (fontinfo_id >= 0) {  // found a common font
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
        if (temp_fid >= 0 && fontinfo_table_->get(temp_fid).get_spacing(
            parent_b->unichar_id(), unichar_id, &temp_gap)) {
          expected_gap += temp_gap;
          num_addends++;
        }
      }
      expected_gap_found = (num_addends > 0);
      if (num_addends > 0) expected_gap /= static_cast<float>(num_addends);
    }
    if (expected_gap_found) {
      float actual_gap =
          static_cast<float>(AssociateUtils::GetChunksGap(
              chunks_record->chunk_widths, curr_col-1));
      float gap_ratio = expected_gap / actual_gap;
      // TODO(daria): find a good way to tune this heuristic estimate.
      if (gap_ratio < 1/2 || gap_ratio > 2) {
        consistency_info->num_inconsistent_spaces++;
      }
      if(language_model_debug_level > 1) {
        tprintf("spacing for %s(%d) %s(%d) col %d: expected %g actual %g\n",
                unicharset.id_to_unichar(parent_b->unichar_id()),
                parent_b->unichar_id(), unicharset.id_to_unichar(unichar_id),
                unichar_id, curr_col, expected_gap, actual_gap);
      }
    }
  }
}

float LanguageModel::ComputeAdjustedPathCost(
    float ratings_sum, int length, float dawg_score,
    const LanguageModelDawgInfo *dawg_info,
    const LanguageModelNgramInfo *ngram_info,
    const LanguageModelConsistencyInfo &consistency_info,
    const AssociateStats &associate_stats,
    ViterbiStateEntry *parent_vse) {
  float adjustment = 1.0f;
  if (dawg_info == NULL || dawg_info->permuter != FREQ_DAWG_PERM) {
    adjustment += language_model_penalty_non_freq_dict_word;
  }
  if (dawg_score == 0.0f) {
    adjustment += language_model_penalty_non_dict_word;
    if (length > language_model_min_compound_length) {
      adjustment += ((length - language_model_min_compound_length) *
                     language_model_penalty_increment);
    }
  } else if (dawg_score < 1.0f) {
    adjustment += (1.0f - dawg_score) * language_model_penalty_non_dict_word;
  }
  if (associate_stats.shape_cost > 0) {
    adjustment += associate_stats.shape_cost / static_cast<float>(length);
  }
  if (language_model_ngram_on) {
    ASSERT_HOST(ngram_info != NULL);
    return ngram_info->ngram_cost * adjustment;
  } else {
    adjustment += ComputeConsistencyAdjustment(dawg_info, consistency_info);
    return ratings_sum * adjustment;
  }
}

void LanguageModel::UpdateBestChoice(
    BLOB_CHOICE *b,
    ViterbiStateEntry *vse,
    HEAP *pain_points,
    CHUNKS_RECORD *chunks_record,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  int i;
  BLOB_CHOICE_LIST_VECTOR temp_best_char_choices(vse->length);
  for (i = 0; i < vse->length; ++i) {
    temp_best_char_choices.push_back(NULL);
  }
  float *certainties = new float[vse->length];
  STATE temp_state;
  // The fraction of letters in the path that are "covered" by dawgs.
  // For space delimited languages this number will be 0.0 for nonwords
  // and 1.0 for dictionary words. For non-space delimited languages
  // this number will be in the [0.0, 1.0] range.
  float dawg_score;
  bool truth_path;
  WERD_CHOICE *word = ConstructWord(b, vse, chunks_record,
                                    &temp_best_char_choices, certainties,
                                    &dawg_score, &temp_state,
                                    blamer_bundle, &truth_path);
  ASSERT_HOST(word != NULL);
  bool not_blaming =
      (blamer_bundle == NULL || !blamer_bundle->segsearch_is_looking_for_blame);

  // Log new segmentation (for dict_->LogNewChoice()).
  if (not_blaming) {
    PIECES_STATE pieces_widths;
    bin_to_pieces(&temp_state, chunks_record->ratings->dimension() - 1,
                  pieces_widths);
    dict_->LogNewSegmentation(pieces_widths);
  }

  if (language_model_debug_level > 0) {
    STRING word_str;
    word->string_and_lengths(&word_str, NULL);
    tprintf("UpdateBestChoice() constructed word %s\n", word_str.string());
    if (language_model_debug_level > 2) word->print();
  };

  // Update raw_choice if needed.
  if ((vse->top_choice_flags & kSmallestRatingFlag) &&
      word->rating() < best_choice_bundle->raw_choice->rating() &&
      not_blaming) {
    dict_->LogNewChoice(1.0, certainties, true, word, temp_best_char_choices);
    *(best_choice_bundle->raw_choice) = *word;
    best_choice_bundle->raw_choice->set_permuter(TOP_CHOICE_PERM);
    if (language_model_debug_level > 0) tprintf("Updated raw choice\n");
  }

  // When working with non-space delimited languages we re-adjust the cost
  // taking into account the final dawg_score and a more precise shape cost.
  // While constructing the paths we assume that they are all dictionary words
  // (since a single character would be a valid dictionary word). At the end
  // we compute dawg_score which reflects how many characters on the path are
  // "covered" by dictionary words of length > 1.
  if (vse->associate_stats.full_wh_ratio_var != 0.0f ||
      (dict_->GetMaxFixedLengthDawgIndex() >= 0 && dawg_score < 1.0f)) {
    vse->cost = ComputeAdjustedPathCost(
        vse->ratings_sum, vse->length, dawg_score, vse->dawg_info,
        vse->ngram_info, vse->consistency_info, vse->associate_stats,
        vse->parent_vse);
    if (language_model_debug_level > 0) {
      tprintf("Updated vse cost to %g (dawg_score %g full_wh_ratio_var %g)\n",
              vse->cost, dawg_score, vse->associate_stats.full_wh_ratio_var);
    }
  }

  // Update best choice and best char choices if needed.
  // TODO(daria): re-write AcceptableChoice() and NoDangerousAmbig()
  // to fit better into the new segmentation search.
  word->set_rating(vse->cost);
  if (word->rating() < best_choice_bundle->best_choice->rating() &&
      not_blaming) {
    dict_->LogNewChoice(vse->cost / (language_model_ngram_on ?
                                     vse->ngram_info->ngram_cost :
                                     vse->ratings_sum),
                        certainties, false, word, temp_best_char_choices);
    // Since the rating of the word could have been modified by
    // Dict::LogNewChoice() - check again.
    if (word->rating() < best_choice_bundle->best_choice->rating()) {
      bool modified_blobs;  // not used
      DANGERR fixpt;
      if (dict_->AcceptableChoice(&temp_best_char_choices, word, &fixpt,
                                  ASSOCIATOR_CALLER, &modified_blobs) &&
          AcceptablePath(*vse)) {
        acceptable_choice_found_ = true;
      }
      // Update best_choice_bundle.
      *(best_choice_bundle->best_choice) = *word;
      best_choice_bundle->updated = true;
      best_choice_bundle->best_char_choices->delete_data_pointers();
      best_choice_bundle->best_char_choices->clear();
      for (i = 0; i < temp_best_char_choices.size(); ++i) {
        BLOB_CHOICE_LIST *cc_list = new BLOB_CHOICE_LIST();
        cc_list->deep_copy(temp_best_char_choices[i], &BLOB_CHOICE::deep_copy);
        best_choice_bundle->best_char_choices->push_back(cc_list);
      }
      best_choice_bundle->best_state->part2 = temp_state.part2;
      best_choice_bundle->best_state->part1 = temp_state.part1;
      if (language_model_debug_level > 0) {
        tprintf("Updated best choice\n");
        print_state("New state ", best_choice_bundle->best_state,
                    chunks_record->ratings->dimension()-1);
      }
      // Update hyphen state if we are dealing with a dictionary word.
      if (vse->dawg_info != NULL && dict_->GetMaxFixedLengthDawgIndex() < 0) {
        if (dict_->has_hyphen_end(*word)) {
          dict_->set_hyphen_word(*word, *(dawg_args_->active_dawgs),
                                 *(dawg_args_->constraints));
        } else {
          dict_->reset_hyphen_vars(true);
        }
      }
      best_choice_bundle->best_vse = vse;
      best_choice_bundle->best_b = b;
      best_choice_bundle->fixpt = fixpt;

      if (blamer_bundle != NULL) {
        blamer_bundle->best_choice_is_dict_and_top_choice =
            (vse->dawg_info != NULL &&
             dict_->GetMaxFixedLengthDawgIndex() < 0 &&
             (vse->top_choice_flags));
      }
    }
  }
  if (blamer_bundle != NULL) {
    // Record the current hypothesis in params_training_bundle.
    ParamsTrainingHypothesis &hyp =
        blamer_bundle->params_training_bundle.AddHypothesis();
    word->string_and_lengths(&(hyp.str), NULL);
    ExtractRawFeaturesFromPath(*vse, hyp.features);
    if (truth_path &&  // update best truth path rating
        word->rating() < blamer_bundle->best_correctly_segmented_rating) {
      blamer_bundle->best_correctly_segmented_rating = word->rating();
    }
  }

  // Clean up.
  delete[] certainties;
  delete word;
}

void LanguageModel::ExtractRawFeaturesFromPath(const ViterbiStateEntry &vse,
                                            float *features) {
  for (int f = 0; f < PTRAIN_NUM_RAW_FEATURE_TYPES; ++f) features[f] = 0.0;
  // Dictionary-related features.
  if (vse.dawg_info != NULL) {
    features[PTRAIN_RAW_FEATURE_DICT_MATCH_TYPE] = vse.dawg_info->permuter;

    // Mark as unambiguous if unambig_dawg is found among active dawgs.
    for (int d = 0; d < vse.dawg_info->active_dawgs->size(); ++d) {
      if (dict_->GetDawg(vse.dawg_info->active_dawgs->get(d).dawg_index) ==
          dict_->GetUnambigDawg()) {
        features[PTRAIN_RAW_FEATURE_UNAMBIG_DICT_MATCH] = 1.0;
        break;
      }
    }
  }
  if (vse.associate_stats.shape_cost > 0) {
    features[PTRAIN_RAW_FEATURE_SHAPE_COST] = vse.associate_stats.shape_cost;
  }
  if (language_model_ngram_on) {
    ASSERT_HOST(vse.ngram_info != NULL);
    features[PTRAIN_RAW_FEATURE_NGRAM_PROB] = vse.ngram_info->ngram_prob;
  }
  // Consistency-related features.
  features[PTRAIN_RAW_FEATURE_NUM_BAD_PUNC] =
      vse.consistency_info.NumInconsistentPunc();
  features[PTRAIN_RAW_FEATURE_NUM_BAD_CASE] =
      vse.consistency_info.NumInconsistentCase();
  features[PTRAIN_RAW_FEATURE_NUM_BAD_CHAR_TYPE] =
      vse.consistency_info.NumInconsistentChartype();
  features[PTRAIN_RAW_FEATURE_NUM_BAD_SPACING] =
      vse.consistency_info.NumInconsistentSpaces();
  features[PTRAIN_RAW_FEATURE_NUM_BAD_SCRIPT] =
      vse.consistency_info.inconsistent_script;
  features[PTRAIN_RAW_FEATURE_NUM_BAD_FONT] =
      vse.consistency_info.inconsistent_font;
  // Classifier-related features.
  features[PTRAIN_RAW_FEATURE_WORST_CERT] = vse.min_certainty;
  features[PTRAIN_RAW_FEATURE_RATING] = vse.ratings_sum;
  features[PTRAIN_RAW_FEATURE_ADAPTED] = vse.adapted;
  // Normalization-related features.
  features[PTRAIN_RAW_FEATURE_NUM_UNICHARS] = vse.length;
  features[PTRAIN_RAW_FEATURE_OUTLINE_LEN] = vse.outline_length;
}

WERD_CHOICE *LanguageModel::ConstructWord(
    BLOB_CHOICE *b,
    ViterbiStateEntry *vse,
    CHUNKS_RECORD *chunks_record,
    BLOB_CHOICE_LIST_VECTOR *best_char_choices,
    float certainties[],
    float *dawg_score,
    STATE *state,
    BlamerBundle *blamer_bundle,
    bool *truth_path) {
  uinT64 state_uint = 0x0;
  if (truth_path != NULL) {
    *truth_path =
        (blamer_bundle != NULL &&
         !blamer_bundle->correct_segmentation_cols.empty() &&
         vse->length == blamer_bundle->correct_segmentation_cols.length());
  }
  const uinT64 kHighestBitOn = 0x8000000000000000LL;
  BLOB_CHOICE *curr_b = b;
  LanguageModelState *curr_lms =
    reinterpret_cast<LanguageModelState *>(b->language_model_state());
  ViterbiStateEntry *curr_vse = vse;

  int i;
  bool compound = dict_->hyphenated();  // treat hyphenated words as compound
  bool dawg_score_done = true;
  if (dawg_score != NULL) {
    *dawg_score = 0.0f;
    // For space-delimited languages the presence of dawg_info in the last
    // ViterbyStateEntry on the path means that the whole path represents
    // a valid dictionary word.
    if (dict_->GetMaxFixedLengthDawgIndex() < 0) {
      if (vse->dawg_info != NULL) *dawg_score = 1.0f;
    } else if (vse->length == 1) {
      *dawg_score = 1.0f;       // each one-letter word is legal
       dawg_score_done = true;  // in non-space delimited languages
    } else {
      dawg_score_done = false;  // do more work to compute dawg_score
    }
  }
  // For non-space delimited languages we compute the fraction of letters
  // "covered" by fixed length dawgs (i.e. words of length > 1 on the path).
  int covered_by_fixed_length_dawgs = 0;
  // The number of unichars remaining that should be skipped because
  // they are covered by the previous word from fixed length dawgs.
  int fixed_length_num_unichars_to_skip = 0;

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
  WERD_CHOICE *word = new WERD_CHOICE(chunks_record->word_res->uch_set,
                                      vse->length);
  word->set_length(vse->length);
  for (i = (vse->length-1); i >= 0; --i) {
    if (blamer_bundle != NULL && truth_path != NULL && *truth_path) {
      if ((blamer_bundle->correct_segmentation_cols[i] !=
           curr_lms->contained_in_col) ||
           (blamer_bundle->correct_segmentation_rows[i] !=
               curr_lms->contained_in_row)) {
        *truth_path = false;
      }
    }
    word->set_unichar_id(curr_b->unichar_id(), i);
    word->set_fragment_length(1, i);
    if (certainties != NULL) certainties[i] = curr_b->certainty();
    if (best_char_choices != NULL) {
      best_char_choices->set(chunks_record->ratings->get(
          curr_lms->contained_in_col, curr_lms->contained_in_row), i);
    }
    if (state != NULL) {
      // Record row minus col zeroes in the reverse state to mark the number
      // of joins done by using a blob from this cell in the ratings matrix.
      state_uint >>= (curr_lms->contained_in_row - curr_lms->contained_in_col);
      // Record a one in the reverse state to indicate the split before
      // the blob from the next cell in the ratings matrix (unless we are
      // at the first cell, in which case there is no next blob).
      if (i != 0) {
        state_uint >>= 1;
        state_uint |= kHighestBitOn;
      }
    }
    // For non-space delimited languages: find word endings recorded while
    // trying to separate the current path into words (for words found in
    // fixed length dawgs.
    if (!dawg_score_done && curr_vse->dawg_info != NULL) {
      UpdateCoveredByFixedLengthDawgs(*(curr_vse->dawg_info->active_dawgs),
                                      i, vse->length,
                                      &fixed_length_num_unichars_to_skip,
                                      &covered_by_fixed_length_dawgs,
                                      dawg_score, &dawg_score_done);
    }
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
    if (curr_vse->parent_b == NULL) break;
    curr_b = curr_vse->parent_b;
    curr_lms =
      reinterpret_cast<LanguageModelState *>(curr_b->language_model_state());
    curr_vse = curr_vse->parent_vse;
  }
  ASSERT_HOST(i == 0);  // check that we recorded all the unichar ids
  // Re-adjust shape cost to include the updated width-to-height variance.
  if (full_wh_ratio_mean != 0.0f) {
    vse->associate_stats.shape_cost += vse->associate_stats.full_wh_ratio_var;
  }

  if (state != NULL) {
    state_uint >>= (64 - (chunks_record->ratings->dimension()-1));
    state->part2 = state_uint;
    state_uint >>= 32;
    state->part1 = state_uint;
  }
  word->set_rating(vse->ratings_sum);
  word->set_certainty(vse->min_certainty);
  if (vse->dawg_info != NULL && dict_->GetMaxFixedLengthDawgIndex() < 0) {
    word->set_permuter(compound ? COMPOUND_PERM : vse->dawg_info->permuter);
  } else if (language_model_ngram_on && !vse->ngram_info->pruned) {
    word->set_permuter(NGRAM_PERM);
  } else if (vse->top_choice_flags) {
    word->set_permuter(TOP_CHOICE_PERM);
  } else {
    word->set_permuter(NO_PERM);
  }
  return word;
}

void LanguageModel::UpdateCoveredByFixedLengthDawgs(
    const DawgInfoVector &active_dawgs, int word_index, int word_length,
    int *skip, int *covered, float *dawg_score, bool *dawg_score_done) {
  if (language_model_debug_level > 3) {
    tprintf("UpdateCoveredByFixedLengthDawgs for index %d skip=%d\n",
            word_index, *skip, word_length);
  }

  if (*skip > 0) {
    --(*skip);
  } else {
    int best_index = -1;
    for (int d = 0; d < active_dawgs.size(); ++d) {
      int dawg_index = (active_dawgs[d]).dawg_index;
      if (dawg_index > dict_->GetMaxFixedLengthDawgIndex()) {
        // If active_dawgs of the last ViterbiStateEntry on the path
        // contain a non-fixed length dawg, this means that the whole
        // path represents a word from a non-fixed length word dawg.
        if (word_index == (word_length - 1)) {
          *dawg_score = 1.0f;
          *dawg_score_done = true;
          return;
        }
      } else if (dawg_index >= kMinFixedLengthDawgLength) {
        const Dawg *curr_dawg = dict_->GetDawg(dawg_index);
        ASSERT_HOST(curr_dawg != NULL);
        if ((active_dawgs[d]).ref != NO_EDGE &&
            curr_dawg->end_of_word((active_dawgs[d]).ref) &&
            dawg_index > best_index) {
          best_index = dawg_index;
        }

        if (language_model_debug_level > 3) {
          tprintf("dawg_index %d, ref %d, eow %d\n", dawg_index,
                  (active_dawgs[d]).ref,
                  ((active_dawgs[d]).ref != NO_EDGE &&
                   curr_dawg->end_of_word((active_dawgs[d]).ref)));
        }
      }
    }  // end for
    if (best_index != -1) {
      *skip = best_index - 1;
      *covered += best_index;
    }
  }  // end if/else skip

  if (word_index == 0) {
    ASSERT_HOST(*covered <= word_length);
    *dawg_score = (static_cast<float>(*covered) /
                   static_cast<float>(word_length));
    *dawg_score_done = true;
  }
}

void LanguageModel::GeneratePainPointsFromColumn(
    int col,
    const GenericVector<int> &non_empty_rows,
    float best_choice_cert,
    HEAP *pain_points,
    BestPathByColumn *best_path_by_column[],
    CHUNKS_RECORD *chunks_record) {
  for (int i = 0; i < non_empty_rows.length(); ++i) {
    int row = non_empty_rows[i];
    if (language_model_debug_level > 0) {
      tprintf("\nLooking for pain points in col=%d row=%d\n", col, row);
    }
    if (language_model_ngram_on) {
      GenerateNgramModelPainPointsFromColumn(
          col, row, pain_points, chunks_record);
    } else {
      GenerateProblematicPathPainPointsFromColumn(
          col, row, best_choice_cert, pain_points,
          best_path_by_column, chunks_record);
    }
  }
}

void LanguageModel::GenerateNgramModelPainPointsFromColumn(
    int col, int row, HEAP *pain_points, CHUNKS_RECORD *chunks_record) {
  // Find the first top choice path recorded for this cell.
  // If this path is pruned - generate a pain point.
  ASSERT_HOST(chunks_record->ratings->get(col, row) != NULL);
  BLOB_CHOICE_IT bit(chunks_record->ratings->get(col, row));
  bool fragmented = false;
  for (bit.mark_cycle_pt(); !bit.cycled_list(); bit.forward()) {
    if (dict_->getUnicharset().get_fragment(bit.data()->unichar_id())) {
      fragmented = true;
      continue;
    }
    LanguageModelState *lms = reinterpret_cast<LanguageModelState *>(
        bit.data()->language_model_state());
    if (lms == NULL) continue;
    ViterbiStateEntry_IT vit(&(lms->viterbi_state_entries));
    for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
      const ViterbiStateEntry *vse = vit.data();
      if (!vse->top_choice_flags) continue;
      ASSERT_HOST(vse->ngram_info != NULL);
      if (vse->ngram_info->pruned && (vse->parent_vse == NULL ||
                                      !vse->parent_vse->ngram_info->pruned)) {
        if (vse->parent_vse != NULL) {
          LanguageModelState *pp_lms = reinterpret_cast<LanguageModelState *>(
              vse->parent_b->language_model_state());
          GeneratePainPoint(pp_lms->contained_in_col, row, false,
                            kDefaultPainPointPriorityAdjustment,
                            -1.0f, fragmented, -1.0f,
                            max_char_wh_ratio_,
                            vse->parent_vse->parent_b,
                            vse->parent_vse->parent_vse,
                            chunks_record, pain_points);
        }
        if (vse->parent_vse != NULL &&
            vse->parent_vse->parent_vse != NULL &&
            dict_->getUnicharset().get_ispunctuation(
                vse->parent_b->unichar_id())) {
          // If the dip in the ngram probability is due to punctuation in the
          // middle of the word - go two unichars back to combine this
          // punctuation mark with the previous character on the path.
          LanguageModelState *pp_lms = reinterpret_cast<LanguageModelState *>(
              vse->parent_vse->parent_b->language_model_state());
          GeneratePainPoint(pp_lms->contained_in_col, col-1, false,
                            kDefaultPainPointPriorityAdjustment,
                            -1.0f, fragmented, -1.0f,
                            max_char_wh_ratio_,
                            vse->parent_vse->parent_vse->parent_b,
                            vse->parent_vse->parent_vse->parent_vse,
                            chunks_record, pain_points);
        } else if (row+1 < chunks_record->ratings->dimension()) {
          GeneratePainPoint(col, row+1, true,
                            kDefaultPainPointPriorityAdjustment,
                            -1.0f, fragmented, -1.0f,
                            max_char_wh_ratio_,
                            vse->parent_b,
                            vse->parent_vse,
                            chunks_record, pain_points);
        }
      }
      return;  // examined the lowest cost top choice path
    }
  }
}

void LanguageModel::GenerateProblematicPathPainPointsFromColumn(
    int col, int row, float best_choice_cert,
    HEAP *pain_points, BestPathByColumn *best_path_by_column[],
    CHUNKS_RECORD *chunks_record) {
  MATRIX *ratings = chunks_record->ratings;

  // Get the best path from this matrix cell.
  BLOB_CHOICE_LIST *blist = ratings->get(col, row);
  ASSERT_HOST(blist != NULL);
  if (blist->empty()) return;
  BLOB_CHOICE_IT bit(blist);
  bool fragment = false;
  while (dict_->getUnicharset().get_fragment(bit.data()->unichar_id()) &&
         !bit.at_last()) {  // skip fragments
    fragment = true;
    bit.forward();
  }
  if (bit.data()->language_model_state() == NULL) return;
  ViterbiStateEntry_IT vit(&(reinterpret_cast<LanguageModelState *>(
      bit.data()->language_model_state())->viterbi_state_entries));
  if (vit.empty()) return;
  ViterbiStateEntry *vse = vit.data();
  // Check whether this path is promising.
  bool path_is_promising = true;
  if (vse->parent_vse != NULL) {
    float potential_avg_cost =
      ((vse->parent_vse->cost + bit.data()->rating()*0.5f) /
       static_cast<float>(row+1));
    if (language_model_debug_level > 0) {
      tprintf("potential_avg_cost %g best cost %g\n",
              potential_avg_cost, (*best_path_by_column)[col].avg_cost);
    }
    if (potential_avg_cost >= (*best_path_by_column)[col].avg_cost) {
      path_is_promising = false;
    }
  }
  // Set best_parent_vse to the best parent recorded in best_path_by_column.
  ViterbiStateEntry *best_parent_vse = vse->parent_vse;
  BLOB_CHOICE *best_parent_b = vse->parent_b;
  if (col > 0 && (*best_path_by_column)[col-1].best_vse != NULL) {
    ASSERT_HOST((*best_path_by_column)[col-1].best_b != NULL);
    LanguageModelState *best_lms = reinterpret_cast<LanguageModelState *>(
        ((*best_path_by_column)[col-1].best_b)->language_model_state());
    if (best_lms->contained_in_row == col-1) {
      best_parent_vse = (*best_path_by_column)[col-1].best_vse;
      best_parent_b = (*best_path_by_column)[col-1].best_b;
      if (language_model_debug_level > 0) {
        tprintf("Setting best_parent_vse to %p\n", best_parent_vse);
      }
    }
  }
  // Check whether this entry terminates the best parent path
  // recorded in best_path_by_column.
  bool best_not_prolonged = (best_parent_vse != vse->parent_vse);

  // If this path is problematic because of the last unichar - generate
  // a pain point to combine it with its left and right neighbor.
  BLOB_CHOICE_IT tmp_bit;
  if (best_not_prolonged ||
      (path_is_promising &&
       ProblematicPath(*vse, bit.data()->unichar_id(),
                         row+1 == ratings->dimension()))) {
    float worst_piece_cert;
    bool fragmented;
    if (col-1 > 0) {
      GetWorstPieceCertainty(col-1, row, chunks_record->ratings,
                             &worst_piece_cert, &fragmented);
      GeneratePainPoint(col-1, row, false,
                        kDefaultPainPointPriorityAdjustment,
                        worst_piece_cert, fragmented, best_choice_cert,
                        max_char_wh_ratio_, best_parent_b, best_parent_vse,
                        chunks_record, pain_points);
    }
    if (row+1 < ratings->dimension()) {
      GetWorstPieceCertainty(col, row+1, chunks_record->ratings,
                             &worst_piece_cert, &fragmented);
      GeneratePainPoint(col, row+1, true, kDefaultPainPointPriorityAdjustment,
                        worst_piece_cert, fragmented, best_choice_cert,
                        max_char_wh_ratio_, best_parent_b, best_parent_vse,
                        chunks_record, pain_points);
    }
  } // for ProblematicPath
}

void LanguageModel::GeneratePainPointsFromBestChoice(
    HEAP *pain_points,
    CHUNKS_RECORD *chunks_record,
    BestChoiceBundle *best_choice_bundle) {
  // Variables to backtrack best_vse path;
  ViterbiStateEntry *curr_vse = best_choice_bundle->best_vse;
  BLOB_CHOICE *curr_b = best_choice_bundle->best_b;

  // Begins and ends in DANGERR vector record the positions in the blob choice
  // list of the best choice. We need to translate these endpoints into the
  // beginning column and ending row for the pain points. We maintain
  // danger_begin and danger_end arrays indexed by the position in
  // best_choice_bundle->best_char_choices (which is equal to the position
  // on the best_choice_bundle->best_vse path).
  // danger_end[d] stores the DANGERR_INFO structs with end==d and is
  // initialized at the beginning of this function.
  // danger_begin[d] stores the DANGERR_INFO struct with begin==d and
  // has end set to the row of the end of this ambiguity.
  // The translation from end in terms of the best choice index to the end row
  // is done while following the parents of best_choice_bundle->best_vse.
  assert(best_choice_bundle->best_char_choices->length() ==
         best_choice_bundle->best_vse->length);
  DANGERR *danger_begin = NULL;
  DANGERR *danger_end = NULL;
  int d;
  if (!best_choice_bundle->fixpt.empty()) {
    danger_begin = new DANGERR[best_choice_bundle->best_vse->length];
    danger_end = new DANGERR[best_choice_bundle->best_vse->length];
    for (d = 0; d < best_choice_bundle->fixpt.size(); ++d) {
      const DANGERR_INFO &danger = best_choice_bundle->fixpt[d];
      // Only use n->1 ambiguities.
      if (danger.end > danger.begin && !danger.correct_is_ngram &&
          (!language_model_ngram_on || danger.dangerous)) {
        danger_end[danger.end].push_back(danger);
      }
    }
  }

  // Variables to keep track of punctuation/number streaks.
  int punc_streak_end_row = -1;
  int punc_streak_length = 0;
  float punc_streak_min_cert = 0.0f;

  if (language_model_debug_level > 0) {
    tprintf("\nGenerating pain points for best path=%p\n", curr_vse);
  }

  int word_index = best_choice_bundle->best_vse->length;
  while (curr_vse != NULL) {
    word_index--;
    ASSERT_HOST(word_index >= 0);
    ASSERT_HOST(curr_b != NULL);
    if (language_model_debug_level > 0) {
      tprintf("Looking at unichar %s\n",
              dict_->getUnicharset().id_to_unichar(curr_b->unichar_id()));
    }

    int pp_col = reinterpret_cast<LanguageModelState *>(
        curr_b->language_model_state())->contained_in_col;
    int pp_row = reinterpret_cast<LanguageModelState *>(
        curr_b->language_model_state())->contained_in_row;

    // Generate pain points for ambiguities found by NoDangerousAmbig().
    if (danger_end != NULL) {
      // Translate end index of an ambiguity to an end row.
      for (d = 0; d < danger_end[word_index].size(); ++d) {
        danger_end[word_index][d].end = pp_row;
        danger_begin[danger_end[word_index][d].begin].push_back(
            danger_end[word_index][d]);
      }
      // Generate a pain point to combine unchars in the "wrong" part
      // of the ambiguity.
      for (d = 0; d < danger_begin[word_index].size(); ++d) {
        if (language_model_debug_level > 0) {
          tprintf("Generating pain point from %sambiguity\n",
                  danger_begin[word_index][d].dangerous ? "dangerous " : "");
        }
        GeneratePainPoint(pp_col, danger_begin[word_index][d].end, false,
                          danger_begin[word_index][d].dangerous ?
                          kCriticalPainPointPriorityAdjustment :
                          kBestChoicePainPointPriorityAdjustment,
                          best_choice_bundle->best_choice->certainty(), true,
                          best_choice_bundle->best_choice->certainty(),
                          kLooseMaxCharWhRatio,
                          curr_vse->parent_b, curr_vse->parent_vse,
                          chunks_record, pain_points);
      }
    }

    if (!language_model_ngram_on) {  // no need to use further heuristics if we
                                     // are guided by the character ngram model
      // Generate pain points for problematic sub-paths.
      if (ProblematicPath(*curr_vse, curr_b->unichar_id(),
                          pp_row+1 == chunks_record->ratings->dimension())) {
        if (language_model_debug_level > 0) {
          tprintf("Generating pain point from a problematic sub-path\n");
        }
        float worst_piece_cert;
        bool fragment;
        if (pp_col > 0) {
          GetWorstPieceCertainty(pp_col-1, pp_row, chunks_record->ratings,
                                 &worst_piece_cert, &fragment);
          GeneratePainPoint(pp_col-1, pp_row, false,
                            kBestChoicePainPointPriorityAdjustment,
                            worst_piece_cert, true,
                            best_choice_bundle->best_choice->certainty(),
                            max_char_wh_ratio_, NULL, NULL,
                            chunks_record, pain_points);
        }
        if (pp_row+1 < chunks_record->ratings->dimension()) {
          GetWorstPieceCertainty(pp_col, pp_row+1, chunks_record->ratings,
                                 &worst_piece_cert, &fragment);
          GeneratePainPoint(pp_col, pp_row+1, true,
                            kBestChoicePainPointPriorityAdjustment,
                            worst_piece_cert, true,
                            best_choice_bundle->best_choice->certainty(),
                            max_char_wh_ratio_, NULL, NULL,
                            chunks_record, pain_points);
        }
      }

      // Generate a pain point if we encountered a punctuation/number streak to
      // combine all punctuation marks into one blob and try to classify it.
      bool is_alpha = dict_->getUnicharset().get_isalpha(curr_b->unichar_id());
      if (!is_alpha) {
        if (punc_streak_end_row == -1) punc_streak_end_row = pp_row;
        punc_streak_length++;
        if (curr_b->certainty() < punc_streak_min_cert)
          punc_streak_min_cert = curr_b->certainty();
      }
      if (is_alpha || curr_vse->parent_vse == NULL) {
        if (punc_streak_end_row != -1 && punc_streak_length > 1) {
          if (language_model_debug_level > 0) {
            tprintf("Generating pain point from a punctuation streak\n");
          }
          if (is_alpha ||
              (curr_vse->parent_vse == NULL && punc_streak_length > 2)) {
            GeneratePainPoint(pp_row+1, punc_streak_end_row, false,
                              kBestChoicePainPointPriorityAdjustment,
                              punc_streak_min_cert, true,
                              best_choice_bundle->best_choice->certainty(),
                              max_char_wh_ratio_, curr_b, curr_vse,
                              chunks_record, pain_points);
          }
          // Handle punctuation/number streaks starting from the first unichar.
          if (curr_vse->parent_vse == NULL) {
            GeneratePainPoint(0, punc_streak_end_row, false,
                              kBestChoicePainPointPriorityAdjustment,
                              punc_streak_min_cert, true,
                              best_choice_bundle->best_choice->certainty(),
                              max_char_wh_ratio_, NULL, NULL,
                              chunks_record, pain_points);
          }
        }
        punc_streak_end_row = -1;
        punc_streak_length = 0;
        punc_streak_min_cert = 0.0f;
      }  // end handling punctuation streaks
    }

    curr_b = curr_vse->parent_b;
    curr_vse = curr_vse->parent_vse;
  }  // end looking at best choice subpaths

  // Clean up.
  if (danger_end != NULL) {
    delete[] danger_begin;
    delete[] danger_end;
  }
}

bool LanguageModel::GeneratePainPoint(
    int col, int row, bool ok_to_extend, float priority,
    float worst_piece_cert, bool fragmented, float best_choice_cert,
    float max_char_wh_ratio,
    BLOB_CHOICE *parent_b, ViterbiStateEntry *parent_vse,
    CHUNKS_RECORD *chunks_record, HEAP *pain_points) {
  if (col < 0 || row >= chunks_record->ratings->dimension() ||
      chunks_record->ratings->get(col, row) != NOT_CLASSIFIED) {
    return false;
  }
  if (language_model_debug_level > 3) {
    tprintf("\nGenerating pain point for col=%d row=%d priority=%g parent=",
            col, row, priority);
    if (parent_vse != NULL) {
      PrintViterbiStateEntry("", parent_vse, parent_b, chunks_record);
    } else {
      tprintf("NULL");
    }
    tprintf("\n");
  }

  AssociateStats associate_stats;
  ComputeAssociateStats(col, row, max_char_wh_ratio, parent_vse,
                        chunks_record, &associate_stats);
  // For fixed-pitch fonts/languages: if the current combined blob overlaps
  // the next blob on the right and it is ok to extend the blob, try expending
  // the blob until there is no overlap with the next blob on the right or
  // until the width-to-height ratio becomes too large.
  if (ok_to_extend) {
    while (associate_stats.bad_fixed_pitch_right_gap &&
           row+1 < chunks_record->ratings->dimension() &&
           !associate_stats.bad_fixed_pitch_wh_ratio) {
      ComputeAssociateStats(col, ++row, max_char_wh_ratio, parent_vse,
                            chunks_record, &associate_stats);
    }
  }

  if (associate_stats.bad_shape) {
    if (language_model_debug_level > 3) {
      tprintf("Discarded pain point with a bad shape\n");
    }
    return false;
  }

  // Compute pain point priority.
  if (associate_stats.shape_cost > 0) {
    priority *= associate_stats.shape_cost;
  }
  if (worst_piece_cert < best_choice_cert) {
    worst_piece_cert = best_choice_cert;
  }
  priority *= CertaintyScore(worst_piece_cert);
  if (fragmented) priority /= kDefaultPainPointPriorityAdjustment;

  if (language_model_debug_level > 3) {
    tprintf("worst_piece_cert=%g fragmented=%d\n",
            worst_piece_cert, fragmented);
  }

  if (parent_vse != NULL) {
    priority *= sqrt(parent_vse->cost / static_cast<float>(col));
    if (parent_vse->dawg_info != NULL) {
      priority /= kDefaultPainPointPriorityAdjustment;
      if (parent_vse->length > language_model_min_compound_length) {
        priority /= sqrt(static_cast<double>(parent_vse->length));
      }
    }
  }

  MATRIX_COORD *pain_point = new MATRIX_COORD(col, row);
  if (HeapPushCheckSize(pain_points, priority, pain_point)) {
    if (language_model_debug_level) {
      tprintf("Added pain point with priority %g\n", priority);
    }
    return true;
  } else {
    delete pain_point;
    if (language_model_debug_level) tprintf("Pain points heap is full\n");
    return false;
  }
}

}  // namespace tesseract
