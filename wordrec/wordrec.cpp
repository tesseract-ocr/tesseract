///////////////////////////////////////////////////////////////////////
// File:        wordrec.cpp
// Description: wordrec class.
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

#include "wordrec.h"

#include "language_model.h"
#include "params.h"


namespace tesseract {
Wordrec::Wordrec() :
  // control parameters
  BOOL_MEMBER(merge_fragments_in_matrix, TRUE,
              "Merge the fragments in the ratings matrix and delete them"
              " after merging", params()),
  BOOL_MEMBER(wordrec_no_block, FALSE, "Don't output block information",
              params()),
  BOOL_MEMBER(wordrec_enable_assoc, TRUE, "Associator Enable",
              params()),
  BOOL_MEMBER(force_word_assoc, FALSE,
              "force associator to run regardless of what enable_assoc is."
              "This is used for CJK where component grouping is necessary.",
              CCUtil::params()),
  INT_MEMBER(wordrec_num_seg_states, 30, "Segmentation states",
             CCUtil::params()),
  double_MEMBER(wordrec_worst_state, 1.0, "Worst segmentation state",
                params()),
  BOOL_MEMBER(fragments_guide_chopper, FALSE,
              "Use information from fragments to guide chopping process",
              params()),
  INT_MEMBER(repair_unchopped_blobs, 1, "Fix blobs that aren't chopped",
             params()),
  double_MEMBER(tessedit_certainty_threshold, -2.25, "Good blob limit",
                params()),
  INT_MEMBER(chop_debug, 0, "Chop debug",
             params()),
  BOOL_MEMBER(chop_enable, 1, "Chop enable",
              params()),
  BOOL_MEMBER(chop_vertical_creep, 0, "Vertical creep",
            params()),
  INT_MEMBER(chop_split_length, 10000, "Split Length",
             params()),
  INT_MEMBER(chop_same_distance, 2, "Same distance",
             params()),
  INT_MEMBER(chop_min_outline_points, 6, "Min Number of Points on Outline",
             params()),
  INT_MEMBER(chop_inside_angle, -50, "Min Inside Angle Bend",
             params()),
  INT_MEMBER(chop_min_outline_area, 2000, "Min Outline Area",
             params()),
  double_MEMBER(chop_split_dist_knob, 0.5, "Split length adjustment",
                params()),
  double_MEMBER(chop_overlap_knob, 0.9, "Split overlap adjustment",
                params()),
  double_MEMBER(chop_center_knob, 0.15, "Split center adjustment",
                params()),
  double_MEMBER(chop_sharpness_knob, 0.06, "Split sharpness adjustment",
                params()),
  double_MEMBER(chop_width_change_knob, 5.0, "Width change adjustment",
                params()),
  double_MEMBER(chop_ok_split, 100.0, "OK split limit",
                params()),
  double_MEMBER(chop_good_split, 50.0, "Good split limit",
                params()),
  INT_MEMBER(chop_x_y_weight, 3, "X / Y  length weight",
             params()),
  INT_MEMBER(segment_adjust_debug, 0, "Segmentation adjustment debug",
             params()),
  BOOL_MEMBER(assume_fixed_pitch_char_segment, FALSE,
              "include fixed-pitch heuristics in char segmentation",
              params()),
  BOOL_MEMBER(use_new_state_cost, FALSE,
              "use new state cost heuristics for segmentation state evaluation",
              params()),
  double_MEMBER(heuristic_segcost_rating_base, 1.25,
                "base factor for adding segmentation cost into word rating."
                "It's a multiplying factor, the larger the value above 1, "
                "the bigger the effect of segmentation cost.",
                params()),
  double_MEMBER(heuristic_weight_rating, 1.0,
                "weight associated with char rating in combined cost of state",
                params()),
  double_MEMBER(heuristic_weight_width, 1000.0,
                "weight associated with width evidence in combined cost of"
                " state", params()),
  double_MEMBER(heuristic_weight_seamcut, 0.0,
                "weight associated with seam cut in combined cost of state",
                params()),
  double_MEMBER(heuristic_max_char_wh_ratio, 2.0,
                "max char width-to-height ratio allowed in segmentation",
                params()),
  INT_MEMBER(wordrec_debug_level, 0,
             "Debug level for wordrec", params()),
  BOOL_MEMBER(wordrec_debug_blamer, false,
              "Print blamer debug messages", params()),
  BOOL_MEMBER(wordrec_run_blamer, false,
              "Try to set the blame for errors", params()),
  BOOL_MEMBER(enable_new_segsearch, true,
                   "Enable new segmentation search path.", params()),
  INT_MEMBER(segsearch_debug_level, 0,
             "SegSearch debug level", params()),
  INT_MEMBER(segsearch_max_pain_points, 2000,
             "Maximum number of pain points stored in the queue",
             params()),
  INT_MEMBER(segsearch_max_futile_classifications, 10,
             "Maximum number of pain point classifications per word that"
             "did not result in finding a better word choice.",
             params()),
  double_MEMBER(segsearch_max_char_wh_ratio, 2.0,
                "Maximum character width-to-height ratio", params()),
  double_MEMBER(segsearch_max_fixed_pitch_char_wh_ratio, 2.0,
                "Maximum character width-to-height ratio for"
                " fixed-pitch fonts",
                params()),
  BOOL_MEMBER(save_alt_choices, false,
              "Save alternative paths found during chopping"
              " and segmentation search",
              params()) {
  prev_word_best_choice_ = NULL;
  language_model_ = new LanguageModel(&get_fontinfo_table(),
                                      &(getDict()));
  pass2_seg_states = 0;
  num_joints = 0;
  num_pushed = 0;
  num_popped = 0;
  fill_lattice_ = NULL;
}

Wordrec::~Wordrec() {
  delete language_model_;
}

void Wordrec::CopyCharChoices(const BLOB_CHOICE_LIST_VECTOR &from,
                              BLOB_CHOICE_LIST_VECTOR *to) {
  to->delete_data_pointers();
  to->clear();
  for (int i = 0; i < from.size(); ++i) {
    BLOB_CHOICE_LIST *cc_list = new BLOB_CHOICE_LIST();
    cc_list->deep_copy(from[i], &BLOB_CHOICE::deep_copy);
    to->push_back(cc_list);
  }
}

bool Wordrec::ChoiceIsCorrect(const UNICHARSET &uni_set,
                              const WERD_CHOICE *choice,
                              const GenericVector<STRING> &truth_text) {
  if (choice == NULL) return false;
  int i;
  STRING truth_str;
  for (i = 0; i < truth_text.length(); ++i) truth_str += truth_text[i];
  STRING normed_choice_str;
  for (i = 0; i < choice->length(); ++i) {
    normed_choice_str += uni_set.get_normed_unichar(choice->unichar_id(i));
  }
  return (truth_str == normed_choice_str);
}

void Wordrec::SaveAltChoices(const LIST &best_choices, WERD_RES *word) {
  ASSERT_HOST(word->alt_choices.empty());
  ASSERT_HOST(word->alt_states.empty());
  LIST list_it;
  iterate_list(list_it, best_choices) {
    VIABLE_CHOICE choice =
        reinterpret_cast<VIABLE_CHOICE>(first_node(list_it));
    CHAR_CHOICE *char_choice = &(choice->Blob[0]);
    WERD_CHOICE *alt_choice = new WERD_CHOICE(word->uch_set, choice->Length);
    word->alt_states.push_back(GenericVector<int>(choice->Length));
    GenericVector<int> &alt_state = word->alt_states.back();
    for (int i = 0; i < choice->Length; char_choice++, i++) {
      alt_choice->append_unichar_id_space_allocated(
          char_choice->Class, 1, 0, 0);
      alt_state.push_back(char_choice->NumChunks);
    }
    alt_choice->set_rating(choice->Rating);
    alt_choice->set_certainty(choice->Certainty);

    ASSERT_HOST(choice->blob_choices != NULL);
    alt_choice->set_blob_choices(choice->blob_choices);
    choice->blob_choices = NULL;

    word->alt_choices.push_back(alt_choice);
    if (wordrec_debug_level > 0) {
      tprintf("SaveAltChoices: %s %g\n",
              alt_choice->unichar_string().string(), alt_choice->rating());
    }
  }
}

}  // namespace tesseract
