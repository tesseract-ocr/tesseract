///////////////////////////////////////////////////////////////////////
// File:        segsearch.cpp
// Description: Segmentation search functions.
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

#include <cstdint>           // for INT32_MAX
#include "blamer.h"          // for BlamerBundle
#include "errcode.h"         // for ASSERT_HOST
#include "genericvector.h"   // for GenericVector
#include "lm_pain_points.h"  // for LMPainPoints, LM_PPTYPE_SHAPE, LMPainPoi...
#include "lm_state.h"        // for BestChoiceBundle, ViterbiStateEntry
#include "matrix.h"          // for MATRIX_COORD, MATRIX
#include "pageres.h"         // for WERD_RES
#include "params.h"          // for BoolParam, IntParam, DoubleParam
#include "ratngs.h"          // for BLOB_CHOICE_LIST, BLOB_CHOICE_IT
#include "strngs.h"          // for STRING
#include "tesscallback.h"    // for TessResultCallback2
#include "tprintf.h"         // for tprintf
#include "wordrec.h"         // for Wordrec, SegSearchPending (ptr only)

namespace tesseract {

void Wordrec::DoSegSearch(WERD_RES* word_res) {
  BestChoiceBundle best_choice_bundle(word_res->ratings->dimension());
  // Run Segmentation Search.
  SegSearch(word_res, &best_choice_bundle, nullptr);
}

void Wordrec::SegSearch(WERD_RES* word_res,
                        BestChoiceBundle* best_choice_bundle,
                        BlamerBundle* blamer_bundle) {
  LMPainPoints pain_points(segsearch_max_pain_points,
                           segsearch_max_char_wh_ratio,
                           assume_fixed_pitch_char_segment,
                           &getDict(), segsearch_debug_level);
  // Compute scaling factor that will help us recover blob outline length
  // from classifier rating and certainty for the blob.
  float rating_cert_scale = -1.0 * getDict().certainty_scale / rating_scale;
  GenericVector<SegSearchPending> pending;
  InitialSegSearch(word_res, &pain_points, &pending, best_choice_bundle,
                   blamer_bundle);

  if (!SegSearchDone(0)) {  // find a better choice
    if (chop_enable && word_res->chopped_word != nullptr) {
      improve_by_chopping(rating_cert_scale, word_res, best_choice_bundle,
                          blamer_bundle, &pain_points, &pending);
    }
    if (chop_debug) SEAM::PrintSeams("Final seam list:", word_res->seam_array);

    if (blamer_bundle != nullptr &&
        !blamer_bundle->ChoiceIsCorrect(word_res->best_choice)) {
      blamer_bundle->SetChopperBlame(word_res, wordrec_debug_blamer);
    }
  }
  // Keep trying to find a better path by fixing the "pain points".

  MATRIX_COORD pain_point;
  float pain_point_priority;
  int num_futile_classifications = 0;
  STRING blamer_debug;
  while (wordrec_enable_assoc &&
      (!SegSearchDone(num_futile_classifications) ||
          (blamer_bundle != nullptr &&
              blamer_bundle->GuidedSegsearchStillGoing()))) {
    // Get the next valid "pain point".
    bool found_nothing = true;
    LMPainPointsType pp_type;
    while ((pp_type = pain_points.Deque(&pain_point, &pain_point_priority)) !=
        LM_PPTYPE_NUM) {
      if (!pain_point.Valid(*word_res->ratings)) {
        word_res->ratings->IncreaseBandSize(
            pain_point.row - pain_point.col + 1);
      }
      if (pain_point.Valid(*word_res->ratings) &&
          !word_res->ratings->Classified(pain_point.col, pain_point.row,
                                         getDict().WildcardID())) {
        found_nothing = false;
        break;
      }
    }
    if (found_nothing) {
      if (segsearch_debug_level > 0) tprintf("Pain points queue is empty\n");
      break;
    }
    ProcessSegSearchPainPoint(pain_point_priority, pain_point,
                              LMPainPoints::PainPointDescription(pp_type),
                              &pending, word_res, &pain_points, blamer_bundle);

    UpdateSegSearchNodes(rating_cert_scale, pain_point.col, &pending,
                         word_res, &pain_points, best_choice_bundle,
                         blamer_bundle);
    if (!best_choice_bundle->updated) ++num_futile_classifications;

    if (segsearch_debug_level > 0) {
      tprintf("num_futile_classifications %d\n", num_futile_classifications);
    }

    best_choice_bundle->updated = false;  // reset updated

    // See if it's time to terminate SegSearch or time for starting a guided
    // search for the true path to find the blame for the incorrect best_choice.
    if (SegSearchDone(num_futile_classifications) &&
        blamer_bundle != nullptr &&
        blamer_bundle->GuidedSegsearchNeeded(word_res->best_choice)) {
      InitBlamerForSegSearch(word_res, &pain_points, blamer_bundle,
                             &blamer_debug);
    }
  }  // end while loop exploring alternative paths
  if (blamer_bundle != nullptr) {
    blamer_bundle->FinishSegSearch(word_res->best_choice,
                                   wordrec_debug_blamer, &blamer_debug);
  }

  if (segsearch_debug_level > 0) {
    tprintf("Done with SegSearch (AcceptableChoiceFound: %d)\n",
            language_model_->AcceptableChoiceFound());
  }
}

// Setup and run just the initial segsearch on an established matrix,
// without doing any additional chopping or joining.
// (Internal factored version that can be used as part of the main SegSearch.)
void Wordrec::InitialSegSearch(WERD_RES* word_res, LMPainPoints* pain_points,
                               GenericVector<SegSearchPending>* pending,
                               BestChoiceBundle* best_choice_bundle,
                               BlamerBundle* blamer_bundle) {
  if (segsearch_debug_level > 0) {
    tprintf("Starting SegSearch on ratings matrix%s:\n",
            wordrec_enable_assoc ? " (with assoc)" : "");
    word_res->ratings->print(getDict().getUnicharset());
  }

  pain_points->GenerateInitial(word_res);

  // Compute scaling factor that will help us recover blob outline length
  // from classifier rating and certainty for the blob.
  float rating_cert_scale = -1.0 * getDict().certainty_scale / rating_scale;

  language_model_->InitForWord(prev_word_best_choice_,
                               assume_fixed_pitch_char_segment,
                               segsearch_max_char_wh_ratio, rating_cert_scale);

  // Initialize blamer-related information: map character boxes recorded in
  // blamer_bundle->norm_truth_word to the corresponding i,j indices in the
  // ratings matrix. We expect this step to succeed, since when running the
  // chopper we checked that the correct chops are present.
  if (blamer_bundle != nullptr) {
    blamer_bundle->SetupCorrectSegmentation(word_res->chopped_word,
                                            wordrec_debug_blamer);
  }

  // pending[col] tells whether there is update work to do to combine
  // best_choice_bundle->beam[col - 1] with some BLOB_CHOICEs in matrix[col, *].
  // As the language model state is updated, pending entries are modified to
  // minimize duplication of work. It is important that during the update the
  // children are considered in the non-decreasing order of their column, since
  // this guarantees that all the parents would be up to date before an update
  // of a child is done.
  pending->init_to_size(word_res->ratings->dimension(), SegSearchPending());

  // Search the ratings matrix for the initial best path.
  (*pending)[0].SetColumnClassified();
  UpdateSegSearchNodes(rating_cert_scale, 0, pending, word_res,
                       pain_points, best_choice_bundle, blamer_bundle);
}

void Wordrec::UpdateSegSearchNodes(
    float rating_cert_scale,
    int starting_col,
    GenericVector<SegSearchPending>* pending,
    WERD_RES *word_res,
    LMPainPoints *pain_points,
    BestChoiceBundle *best_choice_bundle,
    BlamerBundle *blamer_bundle) {
  MATRIX *ratings = word_res->ratings;
  ASSERT_HOST(ratings->dimension() == pending->size());
  ASSERT_HOST(ratings->dimension() == best_choice_bundle->beam.size());
  for (int col = starting_col; col < ratings->dimension(); ++col) {
    if (!(*pending)[col].WorkToDo()) continue;
    int first_row = col;
    int last_row = std::min(ratings->dimension() - 1,
                       col + ratings->bandwidth() - 1);
    if ((*pending)[col].SingleRow() >= 0) {
      first_row = last_row = (*pending)[col].SingleRow();
    }
    if (segsearch_debug_level > 0) {
      tprintf("\n\nUpdateSegSearchNodes: col=%d, rows=[%d,%d], alljust=%d\n",
              col, first_row, last_row,
              (*pending)[col].IsRowJustClassified(INT32_MAX));
    }
    // Iterate over the pending list for this column.
    for (int row = first_row; row <= last_row; ++row) {
      // Update language model state of this child+parent pair.
      BLOB_CHOICE_LIST *current_node = ratings->get(col, row);
      LanguageModelState *parent_node =
          col == 0 ? nullptr : best_choice_bundle->beam[col - 1];
      if (current_node != nullptr &&
          language_model_->UpdateState((*pending)[col].IsRowJustClassified(row),
                                       col, row, current_node, parent_node,
                                       pain_points, word_res,
                                       best_choice_bundle, blamer_bundle) &&
          row + 1 < ratings->dimension()) {
        // Since the language model state of this entry changed, process all
        // the child column.
        (*pending)[row + 1].RevisitWholeColumn();
        if (segsearch_debug_level > 0) {
          tprintf("Added child col=%d to pending\n", row + 1);
        }
      }  // end if UpdateState.
    }  // end for row.
  }  // end for col.
  if (best_choice_bundle->best_vse != nullptr) {
    ASSERT_HOST(word_res->StatesAllValid());
    if (best_choice_bundle->best_vse->updated) {
      pain_points->GenerateFromPath(rating_cert_scale,
                                    best_choice_bundle->best_vse, word_res);
      if (!best_choice_bundle->fixpt.empty()) {
        pain_points->GenerateFromAmbigs(best_choice_bundle->fixpt,
                                        best_choice_bundle->best_vse, word_res);
      }
    }
  }
  // The segsearch is completed. Reset all updated flags on all VSEs and reset
  // all pendings.
  for (int col = 0; col < pending->size(); ++col) {
    (*pending)[col].Clear();
    ViterbiStateEntry_IT
        vse_it(&best_choice_bundle->beam[col]->viterbi_state_entries);
    for (vse_it.mark_cycle_pt(); !vse_it.cycled_list(); vse_it.forward()) {
      vse_it.data()->updated = false;
    }
  }
}

void Wordrec::ProcessSegSearchPainPoint(
    float pain_point_priority,
    const MATRIX_COORD &pain_point, const char* pain_point_type,
    GenericVector<SegSearchPending>* pending, WERD_RES *word_res,
    LMPainPoints *pain_points, BlamerBundle *blamer_bundle) {
  if (segsearch_debug_level > 0) {
    tprintf("Classifying pain point %s priority=%.4f, col=%d, row=%d\n",
            pain_point_type, pain_point_priority,
            pain_point.col, pain_point.row);
  }
  ASSERT_HOST(pain_points != nullptr);
  MATRIX *ratings = word_res->ratings;
  // Classify blob [pain_point.col pain_point.row]
  if (!pain_point.Valid(*ratings)) {
    ratings->IncreaseBandSize(pain_point.row + 1 - pain_point.col);
  }
  ASSERT_HOST(pain_point.Valid(*ratings));
  BLOB_CHOICE_LIST *classified = classify_piece(word_res->seam_array,
                                                pain_point.col, pain_point.row,
                                                pain_point_type,
                                                word_res->chopped_word,
                                                blamer_bundle);
  BLOB_CHOICE_LIST *lst = ratings->get(pain_point.col, pain_point.row);
  if (lst == nullptr) {
    ratings->put(pain_point.col, pain_point.row, classified);
  } else {
    // We can not delete old BLOB_CHOICEs, since they might contain
    // ViterbiStateEntries that are parents of other "active" entries.
    // Thus if the matrix cell already contains classifications we add
    // the new ones to the beginning of the list.
    BLOB_CHOICE_IT it(lst);
    it.add_list_before(classified);
    delete classified;  // safe to delete, since empty after add_list_before()
    classified = nullptr;
  }

  if (segsearch_debug_level > 0) {
    print_ratings_list("Updated ratings matrix with a new entry:",
                       ratings->get(pain_point.col, pain_point.row),
                       getDict().getUnicharset());
    ratings->print(getDict().getUnicharset());
  }

  // Insert initial "pain points" to join the newly classified blob
  // with its left and right neighbors.
  if (classified != nullptr && !classified->empty()) {
    if (pain_point.col > 0) {
      pain_points->GeneratePainPoint(
          pain_point.col - 1, pain_point.row, LM_PPTYPE_SHAPE, 0.0,
          true, segsearch_max_char_wh_ratio, word_res);
    }
    if (pain_point.row + 1 < ratings->dimension()) {
      pain_points->GeneratePainPoint(
          pain_point.col, pain_point.row + 1, LM_PPTYPE_SHAPE, 0.0,
          true, segsearch_max_char_wh_ratio, word_res);
    }
  }
  (*pending)[pain_point.col].SetBlobClassified(pain_point.row);
}

// Resets enough of the results so that the Viterbi search is re-run.
// Needed when the n-gram model is enabled, as the multi-length comparison
// implementation will re-value existing paths to worse values.
void Wordrec::ResetNGramSearch(WERD_RES* word_res,
                               BestChoiceBundle* best_choice_bundle,
                               GenericVector<SegSearchPending>* pending) {
  // TODO(rays) More refactoring required here.
  // Delete existing viterbi states.
  for (int col = 0; col < best_choice_bundle->beam.size(); ++col) {
    best_choice_bundle->beam[col]->Clear();
  }
  // Reset best_choice_bundle.
  word_res->ClearWordChoices();
  best_choice_bundle->best_vse = nullptr;
  // Clear out all existing pendings and add a new one for the first column.
  (*pending)[0].SetColumnClassified();
  for (int i = 1; i < pending->size(); ++i)
    (*pending)[i].Clear();
}

void Wordrec::InitBlamerForSegSearch(WERD_RES *word_res,
                                     LMPainPoints *pain_points,
                                     BlamerBundle *blamer_bundle,
                                     STRING *blamer_debug) {
  pain_points->Clear();  // Clear pain points heap.
  TessResultCallback2<bool, int, int>* pp_cb = NewPermanentTessCallback(
      pain_points, &LMPainPoints::GenerateForBlamer,
      static_cast<double>(segsearch_max_char_wh_ratio), word_res);
  blamer_bundle->InitForSegSearch(word_res->best_choice, word_res->ratings,
                                  getDict().WildcardID(), wordrec_debug_blamer,
                                  blamer_debug, pp_cb);
  delete pp_cb;
}

}  // namespace tesseract
