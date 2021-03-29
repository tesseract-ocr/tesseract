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

#include <memory>

#ifdef DISABLED_LEGACY_ENGINE

#  include "params.h"

namespace tesseract {
Wordrec::Wordrec()
    : // control parameters

    BOOL_MEMBER(wordrec_debug_blamer, false, "Print blamer debug messages", params())
    ,

    BOOL_MEMBER(wordrec_run_blamer, false, "Try to set the blame for errors", params()) {
  prev_word_best_choice_ = nullptr;
}

} // namespace tesseract

#else // DISABLED_LEGACY_ENGINE not defined

#  include "language_model.h"
#  include "params.h"

namespace tesseract {
Wordrec::Wordrec()
    : // control parameters
    BOOL_MEMBER(merge_fragments_in_matrix, true,
                "Merge the fragments in the ratings matrix and delete them"
                " after merging",
                params())
    , BOOL_MEMBER(wordrec_enable_assoc, true, "Associator Enable", params())
    , BOOL_MEMBER(force_word_assoc, false,
                  "force associator to run regardless of what enable_assoc is."
                  " This is used for CJK where component grouping is necessary.",
                  CCUtil::params())
    , INT_MEMBER(repair_unchopped_blobs, 1, "Fix blobs that aren't chopped", params())
    , double_MEMBER(tessedit_certainty_threshold, -2.25, "Good blob limit", params())
    , INT_MEMBER(chop_debug, 0, "Chop debug", params())
    , BOOL_MEMBER(chop_enable, 1, "Chop enable", params())
    , BOOL_MEMBER(chop_vertical_creep, 0, "Vertical creep", params())
    , INT_MEMBER(chop_split_length, 10000, "Split Length", params())
    , INT_MEMBER(chop_same_distance, 2, "Same distance", params())
    , INT_MEMBER(chop_min_outline_points, 6, "Min Number of Points on Outline", params())
    , INT_MEMBER(chop_seam_pile_size, 150, "Max number of seams in seam_pile", params())
    , BOOL_MEMBER(chop_new_seam_pile, 1, "Use new seam_pile", params())
    , INT_MEMBER(chop_inside_angle, -50, "Min Inside Angle Bend", params())
    , INT_MEMBER(chop_min_outline_area, 2000, "Min Outline Area", params())
    , double_MEMBER(chop_split_dist_knob, 0.5, "Split length adjustment", params())
    , double_MEMBER(chop_overlap_knob, 0.9, "Split overlap adjustment", params())
    , double_MEMBER(chop_center_knob, 0.15, "Split center adjustment", params())
    , INT_MEMBER(chop_centered_maxwidth, 90,
                 "Width of (smaller) chopped blobs "
                 "above which we don't care that a chop is not near the center.",
                 params())
    , double_MEMBER(chop_sharpness_knob, 0.06, "Split sharpness adjustment", params())
    , double_MEMBER(chop_width_change_knob, 5.0, "Width change adjustment", params())
    , double_MEMBER(chop_ok_split, 100.0, "OK split limit", params())
    , double_MEMBER(chop_good_split, 50.0, "Good split limit", params())
    , INT_MEMBER(chop_x_y_weight, 3, "X / Y  length weight", params())
    , BOOL_MEMBER(assume_fixed_pitch_char_segment, false,
                  "include fixed-pitch heuristics in char segmentation", params())
    , INT_MEMBER(wordrec_debug_level, 0, "Debug level for wordrec", params())
    , INT_MEMBER(wordrec_max_join_chunks, 4, "Max number of broken pieces to associate", params())
    , BOOL_MEMBER(wordrec_skip_no_truth_words, false,
                  "Only run OCR for words that had truth recorded in BlamerBundle", params())
    , BOOL_MEMBER(wordrec_debug_blamer, false, "Print blamer debug messages", params())
    , BOOL_MEMBER(wordrec_run_blamer, false, "Try to set the blame for errors", params())
    , INT_MEMBER(segsearch_debug_level, 0, "SegSearch debug level", params())
    , INT_MEMBER(segsearch_max_pain_points, 2000,
                 "Maximum number of pain points stored in the queue", params())
    , INT_MEMBER(segsearch_max_futile_classifications, 20,
                 "Maximum number of pain point classifications per chunk that"
                 " did not result in finding a better word choice.",
                 params())
    , double_MEMBER(segsearch_max_char_wh_ratio, 2.0, "Maximum character width-to-height ratio",
                    params())
    , BOOL_MEMBER(save_alt_choices, true,
                  "Save alternative paths found during chopping"
                  " and segmentation search",
                  params())
    , pass2_ok_split(0.0f) {
  prev_word_best_choice_ = nullptr;
  language_model_ = std::make_unique<LanguageModel>(&get_fontinfo_table(), &(getDict()));
  fill_lattice_ = nullptr;
}

} // namespace tesseract

#endif // DISABLED_LEGACY_ENGINE
