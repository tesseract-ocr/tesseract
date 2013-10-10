///////////////////////////////////////////////////////////////////////
// File:        lm_state.cpp
// Description: Structures and functionality for capturing the state of
//              segmentation search guided by the language model.
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

#include "lm_state.h"

namespace tesseract {

ELISTIZE(ViterbiStateEntry);

void ViterbiStateEntry::Print(const char *msg) const {
  tprintf("%s ViterbiStateEntry", msg);
  if (updated) tprintf("(NEW)");
  if (this->debug_str != NULL) {
    tprintf(" str=%s", this->debug_str->string());
  }
  tprintf(" with ratings_sum=%.4f length=%d cost=%.6f",
          this->ratings_sum, this->length, this->cost);
  if (this->top_choice_flags) {
    tprintf(" top_choice_flags=0x%x", this->top_choice_flags);
  }
  if (!this->Consistent()) {
    tprintf(" inconsistent=(punc %d case %d chartype %d script %d font %d)",
            this->consistency_info.NumInconsistentPunc(),
            this->consistency_info.NumInconsistentCase(),
            this->consistency_info.NumInconsistentChartype(),
            this->consistency_info.inconsistent_script,
            this->consistency_info.inconsistent_font);
  }
  if (this->dawg_info) tprintf(" permuter=%d", this->dawg_info->permuter);
  if (this->ngram_info) {
    tprintf(" ngram_cl_cost=%g context=%s ngram pruned=%d",
            this->ngram_info->ngram_and_classifier_cost,
            this->ngram_info->context.string(),
            this->ngram_info->pruned);
  }
  if (this->associate_stats.shape_cost > 0.0f) {
    tprintf(" shape_cost=%g", this->associate_stats.shape_cost);
  }
  tprintf(" %s",
          XHeightConsistencyEnumName[this->consistency_info.xht_decision]);

  tprintf("\n");
}

// Clears the viterbi search state back to its initial conditions.
void LanguageModelState::Clear() {
  viterbi_state_entries.clear();
  viterbi_state_entries_prunable_length = 0;
  viterbi_state_entries_prunable_max_cost = MAX_FLOAT32;
  viterbi_state_entries_length = 0;
}

void LanguageModelState::Print(const char *msg) {
  tprintf("%s VSEs (max_cost=%g prn_len=%d tot_len=%d):\n",
          msg, viterbi_state_entries_prunable_max_cost,
          viterbi_state_entries_prunable_length, viterbi_state_entries_length);
  ViterbiStateEntry_IT vit(&viterbi_state_entries);
  for (vit.mark_cycle_pt(); !vit.cycled_list(); vit.forward()) {
    vit.data()->Print("");
  }
}


}  // namespace tesseract
