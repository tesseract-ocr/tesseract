/**********************************************************************
 * File:        tovars.cpp  (Formerly to_vars.c)
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

#include "tovars.h"
#include "params.h"

namespace tesseract {

BOOL_VAR(textord_show_initial_words, false, "Display separate words");
BOOL_VAR(textord_blocksall_fixed, false, "Moan about prop blocks");
BOOL_VAR(textord_blocksall_prop, false, "Moan about fixed pitch blocks");
INT_VAR(textord_dotmatrix_gap, 3, "Max pixel gap for broken pixed pitch");
INT_VAR(textord_debug_block, 0, "Block to do debug on");
INT_VAR(textord_pitch_range, 2, "Max range test on pitch");
double_VAR(textord_wordstats_smooth_factor, 0.05, "Smoothing gap stats");
double_VAR(textord_words_maxspace, 4.0, "Multiple of xheight");
double_VAR(textord_words_default_maxspace, 3.5, "Max believable third space");
double_VAR(textord_words_default_minspace, 0.6, "Fraction of xheight");
double_VAR(textord_words_min_minspace, 0.3, "Fraction of xheight");
double_VAR(textord_words_default_nonspace, 0.2, "Fraction of xheight");
double_VAR(textord_words_initial_lower, 0.25, "Max initial cluster size");
double_VAR(textord_words_initial_upper, 0.15, "Min initial cluster spacing");
double_VAR(textord_words_minlarge, 0.75, "Fraction of valid gaps needed");
double_VAR(textord_words_pitchsd_threshold, 0.040, "Pitch sync threshold");
double_VAR(textord_words_def_fixed, 0.016, "Threshold for definite fixed");
double_VAR(textord_words_def_prop, 0.090, "Threshold for definite prop");
INT_VAR(textord_words_veto_power, 5, "Rows required to outvote a veto");
double_VAR(textord_pitch_rowsimilarity, 0.08, "Fraction of xheight for sameness");
BOOL_VAR(textord_pitch_scalebigwords, false, "Scale scores on big words");
double_VAR(words_initial_lower, 0.5, "Max initial cluster size");
double_VAR(words_initial_upper, 0.15, "Min initial cluster spacing");
double_VAR(words_default_prop_nonspace, 0.25, "Fraction of xheight");
double_VAR(words_default_fixed_space, 0.75, "Fraction of xheight");
double_VAR(words_default_fixed_limit, 0.6, "Allowed size variance");
double_VAR(textord_words_definite_spread, 0.30, "Non-fuzzy spacing region");
double_VAR(textord_spacesize_ratioprop, 2.0, "Min ratio space/nonspace");
double_VAR(textord_fpiqr_ratio, 1.5, "Pitch IQR/Gap IQR threshold");
double_VAR(textord_max_pitch_iqr, 0.20, "Xh fraction noise in pitch");

} // namespace tesseract
