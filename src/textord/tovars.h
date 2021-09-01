/**********************************************************************
 * File:        tovars.h  (Formerly to_vars.h)
 * Description: Variables used by textord.
 * Author:    Ray Smith
 * Created:   Tue Aug 24 16:55:02 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef TOVARS_H
#define TOVARS_H

#include "params.h"

namespace tesseract {

extern BOOL_VAR_H(textord_show_initial_words);
extern BOOL_VAR_H(textord_blocksall_fixed);
extern BOOL_VAR_H(textord_blocksall_prop);
extern INT_VAR_H(textord_dotmatrix_gap);
extern INT_VAR_H(textord_debug_block);
extern INT_VAR_H(textord_pitch_range);
extern double_VAR_H(textord_wordstats_smooth_factor);
extern double_VAR_H(textord_words_maxspace);
extern double_VAR_H(textord_words_default_maxspace);
extern double_VAR_H(textord_words_default_minspace);
extern double_VAR_H(textord_words_min_minspace);
extern double_VAR_H(textord_words_default_nonspace);
extern double_VAR_H(textord_words_initial_lower);
extern double_VAR_H(textord_words_initial_upper);
extern double_VAR_H(textord_words_minlarge);
extern double_VAR_H(textord_words_pitchsd_threshold);
extern double_VAR_H(textord_words_def_fixed);
extern double_VAR_H(textord_words_def_prop);
extern INT_VAR_H(textord_words_veto_power);
extern double_VAR_H(textord_pitch_rowsimilarity);
extern BOOL_VAR_H(textord_pitch_scalebigwords);
extern double_VAR_H(words_initial_lower);
extern double_VAR_H(words_initial_upper);
extern double_VAR_H(words_default_prop_nonspace);
extern double_VAR_H(words_default_fixed_space);
extern double_VAR_H(words_default_fixed_limit);
extern double_VAR_H(textord_words_definite_spread);
extern double_VAR_H(textord_spacesize_ratioprop);
extern double_VAR_H(textord_fpiqr_ratio);
extern double_VAR_H(textord_max_pitch_iqr);

} // namespace tesseract

#endif
