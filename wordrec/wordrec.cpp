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
  BOOL_MEMBER(wordrec_no_block, FALSE, "Don't output block information",
              this->params()),
  BOOL_MEMBER(wordrec_enable_assoc, TRUE, "Associator Enable",
              this->params()),
  BOOL_MEMBER(force_word_assoc, FALSE,
              "force associator to run regardless of what enable_assoc is."
              "This is used for CJK where component grouping is necessary.",
              this->params()),
  INT_MEMBER(wordrec_num_seg_states, 30, "Segmentation states",
             this->params()),
  double_MEMBER(wordrec_worst_state, 1.0, "Worst segmentation state",
                this->params()),
  BOOL_MEMBER(fragments_guide_chopper, FALSE,
              "Use information from fragments to guide chopping process",
              this->params()),
  INT_MEMBER(repair_unchopped_blobs, 1, "Fix blobs that aren't chopped",
             this->params()),
  double_MEMBER(tessedit_certainty_threshold, -2.25, "Good blob limit",
                this->params()),
  INT_MEMBER(chop_debug, 0, "Chop debug",
             this->params()),
  BOOL_MEMBER(chop_enable, 1, "Chop enable",
              this->params()),
  BOOL_MEMBER(chop_vertical_creep, 0, "Vertical creep",
            this->params()),
  INT_MEMBER(chop_split_length, 10000, "Split Length",
             this->params()),
  INT_MEMBER(chop_same_distance, 2, "Same distance",
             this->params()),
  INT_MEMBER(chop_min_outline_points, 6, "Min Number of Points on Outline",
             this->params()),
  INT_MEMBER(chop_inside_angle, -50, "Min Inside Angle Bend",
             this->params()),
  INT_MEMBER(chop_min_outline_area, 2000, "Min Outline Area",
             this->params()),
  double_MEMBER(chop_split_dist_knob, 0.5, "Split length adjustment",
                this->params()),
  double_MEMBER(chop_overlap_knob, 0.9, "Split overlap adjustment",
                this->params()),
  double_MEMBER(chop_center_knob, 0.15, "Split center adjustment",
                this->params()),
  double_MEMBER(chop_sharpness_knob, 0.06, "Split sharpness adjustment",
                this->params()),
  double_MEMBER(chop_width_change_knob, 5.0, "Width change adjustment",
                this->params()),
  double_MEMBER(chop_ok_split, 100.0, "OK split limit",
                this->params()),
  double_MEMBER(chop_good_split, 50.0, "Good split limit",
                this->params()),
  INT_MEMBER(chop_x_y_weight, 3, "X / Y  length weight",
             this->params()),
  INT_MEMBER(segment_adjust_debug, 0, "Segmentation adjustment debug",
             this->params()),
  BOOL_MEMBER(assume_fixed_pitch_char_segment, FALSE,
              "include fixed-pitch heuristics in char segmentation",
              this->params()),
  BOOL_MEMBER(use_new_state_cost, FALSE,
              "use new state cost heuristics for segmentation state evaluation",
              this->params()),
  double_MEMBER(heuristic_segcost_rating_base, 1.25,
                "base factor for adding segmentation cost into word rating."
                "It's a multiplying factor, the larger the value above 1, "
                "the bigger the effect of segmentation cost.",
                this->params()),
  double_MEMBER(heuristic_weight_rating, 1.0,
                "weight associated with char rating in combined cost of state",
                this->params()),
  double_MEMBER(heuristic_weight_width, 1000.0,
                "weight associated with width evidence in combined cost of"
                " state", this->params()),
  double_MEMBER(heuristic_weight_seamcut, 0.0,
                "weight associated with seam cut in combined cost of state",
                this->params()),
  double_MEMBER(heuristic_max_char_wh_ratio, 2.0,
                "max char width-to-height ratio allowed in segmentation",
                this->params()),
  INT_MEMBER(wordrec_debug_level, 0,
             "Debug level for wordrec", this->params()),
  BOOL_INIT_MEMBER(enable_new_segsearch, false,
                   "Enable new segmentation search path.", this->params()),
  INT_MEMBER(segsearch_debug_level, 0,
             "SegSearch debug level", this->params()),
  INT_MEMBER(segsearch_max_pain_points, 2000,
             "Maximum number of pain points stored in the queue",
             this->params()),
  INT_MEMBER(segsearch_max_futile_classifications, 10,
             "Maximum number of pain point classifications per word that"
             "did not result in finding a better word choice.",
             this->params()),
  double_MEMBER(segsearch_max_char_wh_ratio, 2.0,
                "Maximum character width-to-height ratio", this->params()),
  double_MEMBER(segsearch_max_fixed_pitch_char_wh_ratio, 2.0,
                "Maximum character width-to-height ratio for"
                " fixed-pitch fonts",
                this->params()) {
  states_before_best = NULL;
  best_certainties[0] = NULL;
  best_certainties[1] = NULL;
  character_widths = NULL;
  language_model_ = new LanguageModel(&get_fontinfo_table(),
                                      &(getDict()),
                                      &(prev_word_best_choice_));
  pass2_seg_states = 0;
  num_joints = 0;
  num_pushed = 0;
  num_popped = 0;
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

}  // namespace tesseract
