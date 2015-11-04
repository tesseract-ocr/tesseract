/**********************************************************************
 * File:        tovars.cpp  (Formerly to_vars.c)
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

#include "tovars.h"
#include "params.h"

#define EXTERN

EXTERN BOOL_VAR (textord_show_initial_words, FALSE, "Display separate words");
EXTERN BOOL_VAR (textord_show_new_words, FALSE, "Display separate words");
EXTERN BOOL_VAR (textord_show_fixed_words, FALSE,
"Display forced fixed pitch words");
EXTERN BOOL_VAR (textord_blocksall_fixed, FALSE, "Moan about prop blocks");
EXTERN BOOL_VAR (textord_blocksall_prop, FALSE,
"Moan about fixed pitch blocks");
EXTERN BOOL_VAR (textord_blocksall_testing, FALSE, "Dump stats when moaning");
EXTERN BOOL_VAR (textord_test_mode, FALSE, "Do current test");
EXTERN INT_VAR (textord_dotmatrix_gap, 3,
"Max pixel gap for broken pixed pitch");
EXTERN INT_VAR (textord_debug_block, 0, "Block to do debug on");
EXTERN INT_VAR (textord_pitch_range, 2, "Max range test on pitch");
EXTERN double_VAR (textord_wordstats_smooth_factor, 0.05,
"Smoothing gap stats");
EXTERN double_VAR (textord_width_smooth_factor, 0.10,
"Smoothing width stats");
EXTERN double_VAR (textord_words_width_ile, 0.4,
"Ile of blob widths for space est");
EXTERN double_VAR (textord_words_maxspace, 4.0, "Multiple of xheight");
EXTERN double_VAR (textord_words_default_maxspace, 3.5,
"Max believable third space");
EXTERN double_VAR (textord_words_default_minspace, 0.6,
"Fraction of xheight");
EXTERN double_VAR (textord_words_min_minspace, 0.3, "Fraction of xheight");
EXTERN double_VAR (textord_words_default_nonspace, 0.2,
"Fraction of xheight");
EXTERN double_VAR (textord_words_initial_lower, 0.25,
"Max initial cluster size");
EXTERN double_VAR (textord_words_initial_upper, 0.15,
"Min initial cluster spacing");
EXTERN double_VAR (textord_words_minlarge, 0.75,
"Fraction of valid gaps needed");
EXTERN double_VAR (textord_words_pitchsd_threshold, 0.040,
"Pitch sync threshold");
EXTERN double_VAR (textord_words_def_fixed, 0.016,
"Threshold for definite fixed");
EXTERN double_VAR (textord_words_def_prop, 0.090,
"Threshold for definite prop");
EXTERN INT_VAR (textord_words_veto_power, 5,
"Rows required to outvote a veto");
EXTERN double_VAR (textord_pitch_rowsimilarity, 0.08,
"Fraction of xheight for sameness");
EXTERN BOOL_VAR (textord_pitch_scalebigwords, FALSE,
"Scale scores on big words");
EXTERN double_VAR (words_initial_lower, 0.5, "Max initial cluster size");
EXTERN double_VAR (words_initial_upper, 0.15, "Min initial cluster spacing");
EXTERN double_VAR (words_default_prop_nonspace, 0.25, "Fraction of xheight");
EXTERN double_VAR (words_default_fixed_space, 0.75, "Fraction of xheight");
EXTERN double_VAR (words_default_fixed_limit, 0.6, "Allowed size variance");
EXTERN double_VAR (textord_words_definite_spread, 0.30,
"Non-fuzzy spacing region");
EXTERN double_VAR (textord_spacesize_ratiofp, 2.8,
"Min ratio space/nonspace");
EXTERN double_VAR (textord_spacesize_ratioprop, 2.0,
"Min ratio space/nonspace");
EXTERN double_VAR (textord_fpiqr_ratio, 1.5, "Pitch IQR/Gap IQR threshold");
EXTERN double_VAR (textord_max_pitch_iqr, 0.20, "Xh fraction noise in pitch");
EXTERN double_VAR (textord_fp_min_width, 0.5, "Min width of decent blobs");
