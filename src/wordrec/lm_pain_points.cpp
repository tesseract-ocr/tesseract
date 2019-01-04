///////////////////////////////////////////////////////////////////////
// File:        pain_points.cpp
// Description: Functions that utilize the knowledge about the properties
//              of the paths explored by the segmentation search in order
//              to "pain points" - the locations in the ratings matrix
//              which should be classified next.
// Author:      Rika Antonova
// Created:     Mon Jun 20 11:26:43 PST 2012
//
// (C) Copyright 2012, Google Inc.
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

#include "lm_pain_points.h"

#include "associate.h"
#include "dict.h"
#include "genericheap.h"
#include "lm_state.h"
#include "matrix.h"
#include "pageres.h"

#include <algorithm>

namespace tesseract {

const float LMPainPoints::kDefaultPainPointPriorityAdjustment = 2.0f;
const float LMPainPoints::kLooseMaxCharWhRatio = 2.5f;

LMPainPointsType LMPainPoints::Deque(MATRIX_COORD *pp, float *priority) {
  for (int h = 0; h < LM_PPTYPE_NUM; ++h) {
    if (pain_points_heaps_[h].empty()) continue;
    *priority = pain_points_heaps_[h].PeekTop().key;
    *pp = pain_points_heaps_[h].PeekTop().data;
    pain_points_heaps_[h].Pop(nullptr);
    return static_cast<LMPainPointsType>(h);
  }
  return LM_PPTYPE_NUM;
}

void LMPainPoints::GenerateInitial(WERD_RES *word_res) {
  MATRIX *ratings = word_res->ratings;
  AssociateStats associate_stats;
  for (int col = 0; col < ratings->dimension(); ++col) {
    int row_end = std::min(ratings->dimension(), col + ratings->bandwidth() + 1);
    for (int row = col + 1; row < row_end; ++row) {
      MATRIX_COORD coord(col, row);
      if (coord.Valid(*ratings) &&
          ratings->get(col, row) != NOT_CLASSIFIED) continue;
      // Add an initial pain point if needed.
      if (ratings->Classified(col, row - 1, dict_->WildcardID()) ||
          (col + 1 < ratings->dimension() &&
              ratings->Classified(col + 1, row, dict_->WildcardID()))) {
        GeneratePainPoint(col, row, LM_PPTYPE_SHAPE, 0.0,
                          true, max_char_wh_ratio_, word_res);
      }
    }
  }
}

void LMPainPoints::GenerateFromPath(float rating_cert_scale,
                                    ViterbiStateEntry *vse,
                                    WERD_RES *word_res) {
  ViterbiStateEntry *curr_vse = vse;
  BLOB_CHOICE *curr_b = vse->curr_b;
  // The following pain point generation and priority calculation approaches
  // prioritize exploring paths with low average rating of the known part of
  // the path, while not relying on the ratings of the pieces to be combined.
  //
  // A pain point to combine the neighbors is generated for each pair of
  // neighboring blobs on the path (the path is represented by vse argument
  // given to GenerateFromPath()). The priority of each pain point is set to
  // the average rating (per outline length) of the path, not including the
  // ratings of the blobs to be combined.
  // The ratings of the blobs to be combined are not used to calculate the
  // priority, since it is not possible to determine from their magnitude
  // whether it will be beneficial to combine the blobs. The reason is that
  // chopped junk blobs (/ | - ') can have very good (low) ratings, however
  // combining them will be beneficial. Blobs with high ratings might be
  // over-joined pieces of characters, but also could be blobs from an unseen
  // font or chopped pieces of complex characters.
  while (curr_vse->parent_vse != nullptr) {
    ViterbiStateEntry* parent_vse = curr_vse->parent_vse;
    const MATRIX_COORD& curr_cell = curr_b->matrix_cell();
    const MATRIX_COORD& parent_cell = parent_vse->curr_b->matrix_cell();
    MATRIX_COORD pain_coord(parent_cell.col, curr_cell.row);
    if (!pain_coord.Valid(*word_res->ratings) ||
        !word_res->ratings->Classified(parent_cell.col, curr_cell.row,
                                       dict_->WildcardID())) {
      // rat_subtr contains ratings sum of the two adjacent blobs to be merged.
      // rat_subtr will be subtracted from the ratings sum of the path, since
      // the blobs will be joined into a new blob, whose rating is yet unknown.
      float rat_subtr = curr_b->rating() + parent_vse->curr_b->rating();
      // ol_subtr contains the outline length of the blobs that will be joined.
      float ol_subtr =
          AssociateUtils::ComputeOutlineLength(rating_cert_scale, *curr_b) +
          AssociateUtils::ComputeOutlineLength(rating_cert_scale,
                                               *(parent_vse->curr_b));
      // ol_dif is the outline of the path without the two blobs to be joined.
      float ol_dif = vse->outline_length - ol_subtr;
      // priority is set to the average rating of the path per unit of outline,
      // not counting the ratings of the pieces to be joined.
      float priority = ol_dif > 0 ? (vse->ratings_sum-rat_subtr)/ol_dif : 0.0;
      GeneratePainPoint(pain_coord.col, pain_coord.row, LM_PPTYPE_PATH,
                        priority, true, max_char_wh_ratio_, word_res);
    } else if (debug_level_ > 3) {
      tprintf("NO pain point (Classified) for col=%d row=%d type=%s\n",
              pain_coord.col, pain_coord.row,
              LMPainPointsTypeName[LM_PPTYPE_PATH]);
      BLOB_CHOICE_IT b_it(word_res->ratings->get(pain_coord.col,
                                                 pain_coord.row));
      for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
        BLOB_CHOICE* choice = b_it.data();
        choice->print_full();
      }
    }

    curr_vse = parent_vse;
    curr_b = curr_vse->curr_b;
  }
}

void LMPainPoints::GenerateFromAmbigs(const DANGERR &fixpt,
                                      ViterbiStateEntry *vse,
                                      WERD_RES *word_res) {
  // Begins and ends in DANGERR vector now record the blob indices as used
  // by the ratings matrix.
  for (int d = 0; d < fixpt.size(); ++d) {
    const DANGERR_INFO &danger = fixpt[d];
    // Only use dangerous ambiguities.
    if (danger.dangerous) {
      GeneratePainPoint(danger.begin, danger.end - 1,
                        LM_PPTYPE_AMBIG, vse->cost, true,
                        kLooseMaxCharWhRatio, word_res);
    }
  }
}

bool LMPainPoints::GeneratePainPoint(
    int col, int row, LMPainPointsType pp_type, float special_priority,
    bool ok_to_extend, float max_char_wh_ratio,
    WERD_RES *word_res) {
  MATRIX_COORD coord(col, row);
  if (coord.Valid(*word_res->ratings) &&
      word_res->ratings->Classified(col, row, dict_->WildcardID())) {
    return false;
  }
  if (debug_level_ > 3) {
    tprintf("Generating pain point for col=%d row=%d type=%s\n",
            col, row, LMPainPointsTypeName[pp_type]);
  }
  // Compute associate stats.
  AssociateStats associate_stats;
  AssociateUtils::ComputeStats(col, row, nullptr, 0, fixed_pitch_,
                               max_char_wh_ratio, word_res, debug_level_,
                               &associate_stats);
  // For fixed-pitch fonts/languages: if the current combined blob overlaps
  // the next blob on the right and it is ok to extend the blob, try extending
  // the blob until there is no overlap with the next blob on the right or
  // until the width-to-height ratio becomes too large.
  if (ok_to_extend) {
    while (associate_stats.bad_fixed_pitch_right_gap &&
           row + 1 < word_res->ratings->dimension() &&
           !associate_stats.bad_fixed_pitch_wh_ratio) {
      AssociateUtils::ComputeStats(col, ++row, nullptr, 0, fixed_pitch_,
                                   max_char_wh_ratio, word_res, debug_level_,
                                   &associate_stats);
    }
  }
  if (associate_stats.bad_shape) {
    if (debug_level_ > 3) {
      tprintf("Discarded pain point with a bad shape\n");
    }
    return false;
  }

  // Insert the new pain point into pain_points_heap_.
  if (pain_points_heaps_[pp_type].size() < max_heap_size_) {
    // Compute pain point priority.
    float priority;
    if (pp_type == LM_PPTYPE_PATH) {
      priority = special_priority;
    } else {
      priority = associate_stats.gap_sum;
    }
    MatrixCoordPair pain_point(priority, MATRIX_COORD(col, row));
    pain_points_heaps_[pp_type].Push(&pain_point);
    if (debug_level_) {
      tprintf("Added pain point with priority %g\n", priority);
    }
    return true;
  } else {
    if (debug_level_) tprintf("Pain points heap is full\n");
    return false;
  }
}

/**
 * Adjusts the pain point coordinates to cope with expansion of the ratings
 * matrix due to a split of the blob with the given index.
 */
void LMPainPoints::RemapForSplit(int index) {
  for (int i = 0; i < LM_PPTYPE_NUM; ++i) {
    GenericVector<MatrixCoordPair>* heap = pain_points_heaps_[i].heap();
    for (int j = 0; j < heap->size(); ++j)
      (*heap)[j].data.MapForSplit(index);
  }
}

}  //  namespace tesseract
