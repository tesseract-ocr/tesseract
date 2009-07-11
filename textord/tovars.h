/**********************************************************************
 * File:        tovars.h  (Formerly to_vars.h)
 * Description: Variables used by textord.
 * Author:		Ray Smith
 * Created:		Tue Aug 24 16:55:02 BST 1993
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

#ifndef           TOVARS_H
#define           TOVARS_H

#include          "varable.h"
#include          "notdll.h"

extern BOOL_VAR_H (textord_show_initial_words, FALSE,
"Display separate words");
extern BOOL_VAR_H (textord_show_new_words, FALSE, "Display separate words");
extern BOOL_VAR_H (textord_show_fixed_words, FALSE,
"Display forced fixed pitch words");
extern BOOL_VAR_H (textord_blocksall_fixed, FALSE, "Moan about prop blocks");
extern BOOL_VAR_H (textord_blocksall_prop, FALSE,
"Moan about fixed pitch blocks");
extern BOOL_VAR_H (textord_blocksall_testing, FALSE,
"Dump stats when moaning");
extern BOOL_VAR_H (textord_test_mode, FALSE, "Do current test");
extern BOOL_VAR_H (textord_repeat_extraction, TRUE, "Extract repeated chars");
extern INT_VAR_H (textord_dotmatrix_gap, 3,
"Max pixel gap for broken pixed pitch");
extern INT_VAR_H (textord_repeat_threshold, 4,
"Min multiple for repeated char");
extern INT_VAR_H (textord_debug_block, 0, "Block to do debug on");
extern INT_VAR_H (textord_pitch_range, 2, "Max range test on pitch");
extern double_VAR_H (textord_repeat_rating, 6, "Min rating for equal blobs");
extern double_VAR_H(textord_repch_width_variance, 0.2,
"Max width change of gap/blob for repeated chars");
extern double_VAR_H (textord_wordstats_smooth_factor, 0.05,
"Smoothing gap stats");
extern double_VAR_H (textord_width_smooth_factor, 0.10,
"Smoothing width stats");
extern double_VAR_H (textord_words_width_ile, 0.4,
"Ile of blob widths for space est");
extern double_VAR_H (textord_words_maxspace, 4.0, "Multiple of xheight");
extern double_VAR_H (textord_words_default_maxspace, 3.5,
"Max believable third space");
extern double_VAR_H (textord_words_default_minspace, 0.6,
"Fraction of xheight");
extern double_VAR_H (textord_words_min_minspace, 0.3, "Fraction of xheight");
extern double_VAR_H (textord_words_default_nonspace, 0.2,
"Fraction of xheight");
extern double_VAR_H (textord_words_initial_lower, 0.25,
"Max inital cluster size");
extern double_VAR_H (textord_words_initial_upper, 0.15,
"Min initial cluster spacing");
extern double_VAR_H (textord_words_minlarge, 0.75,
"Fraction of valid gaps needed");
extern double_VAR_H (textord_words_pitchsd_threshold, 0.025,
"Pitch sync threshold");
extern double_VAR_H (textord_words_def_fixed, 0.01,
"Threshold for definite fixed");
extern double_VAR_H (textord_words_def_prop, 0.06,
"Threshold for definite prop");
extern INT_VAR_H (textord_words_veto_power, 5,
"Rows required to outvote a veto");
extern double_VAR_H (textord_pitch_rowsimilarity, 0.08,
"Fraction of xheight for sameness");
extern BOOL_VAR_H (textord_pitch_scalebigwords, FALSE,
"Scale scores on big words");
extern double_VAR_H (words_initial_lower, 0.5, "Max inital cluster size");
extern double_VAR_H (words_initial_upper, 0.15,
"Min initial cluster spacing");
extern double_VAR_H (words_default_prop_nonspace, 0.25,
"Fraction of xheight");
extern double_VAR_H (words_default_fixed_space, 0.75, "Fraction of xheight");
extern double_VAR_H (words_default_fixed_limit, 0.6, "Allowed size variance");
extern double_VAR_H (textord_words_definite_spread, 0.30,
"Non-fuzzy spacing region");
extern double_VAR_H (textord_spacesize_ratiofp, 2.8,
"Min ratio space/nonspace");
extern double_VAR_H (textord_spacesize_ratioprop, 2.0,
"Min ratio space/nonspace");
extern double_VAR_H (textord_fpiqr_ratio, 1.5, "Pitch IQR/Gap IQR threshold");
extern double_VAR_H (textord_max_pitch_iqr, 0.20,
"Xh fraction noise in pitch");
extern double_VAR_H (textord_fp_min_width, 0.5, "Min width of decent blobs");
#endif
