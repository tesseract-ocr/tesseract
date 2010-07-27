/******************************************************************
 * File:        fixspace.h  (Formerly fixspace.h)
 * Description: Implements a pass over the page res, exploring the alternative
 *					spacing possibilities, trying to use context to improve the
          word spacing
* Author:		Phil Cheatle
* Created:		Thu Oct 21 11:38:43 BST 1993
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

#ifndef           FIXSPACE_H
#define           FIXSPACE_H

#include          "pageres.h"
#include          "varable.h"
#include          "ocrclass.h"
#include          "notdll.h"

extern BOOL_VAR_H (fixsp_check_for_fp_noise_space, TRUE,
"Try turning noise to space in fixed pitch");
extern BOOL_VAR_H (fixsp_fp_eval, TRUE, "Use alternate evaluation for fp");
extern BOOL_VAR_H (fixsp_noise_score_fixing, TRUE, "More sophisticated?");
extern INT_VAR_H (fixsp_non_noise_limit, 1,
"How many non-noise blbs either side?");
extern double_VAR_H (fixsp_small_outlines_size, 0.28,
"Small if lt xht x this");
extern BOOL_VAR_H (fixsp_ignore_punct, TRUE, "In uniform spacing calc");
extern BOOL_VAR_H (fixsp_numeric_fix, TRUE, "Try to deal with numeric punct");
extern BOOL_VAR_H (fixsp_prefer_joined_1s, TRUE, "Arbitrary boost");
extern BOOL_VAR_H (tessedit_test_uniform_wd_spacing, FALSE,
"Limit context word spacing");
extern BOOL_VAR_H (tessedit_prefer_joined_punct, FALSE,
"Reward punctation joins");
extern INT_VAR_H (fixsp_done_mode, 1, "What constitutes done for spacing");
extern INT_VAR_H (debug_fix_space_level, 0, "Contextual fixspace debug");
extern STRING_VAR_H (numeric_punctuation, ".,",
"Punct. chs expected WITHIN numbers");
void initialise_search(WERD_RES_LIST &src_list, WERD_RES_LIST &new_list);
void transform_to_next_perm(WERD_RES_LIST &words);
void dump_words(WERD_RES_LIST &perm, inT16 score, inT16 mode, BOOL8 improved);
BOOL8 uniformly_spaced(  //sensible word
                       WERD_RES *word);
BOOL8 fixspace_thinks_word_done(WERD_RES *word);
void break_noisiest_blob_word(WERD_RES_LIST &words);
inT16 worst_noise_blob(WERD_RES *word_res, float *worst_noise_score);
float blob_noise_score(PBLOB *blob);
void fixspace_dbg(WERD_RES *word);
#endif
