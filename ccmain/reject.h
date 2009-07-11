/**********************************************************************
 * File:        reject.h  (Formerly reject.h)
 * Description: Rejection functions used in tessedit
 * Author:		Phil Cheatle
 * Created:		Wed Sep 23 16:50:21 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef           REJECT_H
#define           REJECT_H

#include          "varable.h"
#include          "pageres.h"
#include          "notdll.h"

extern INT_VAR_H (tessedit_reject_mode, 5, "Rejection algorithm");
extern INT_VAR_H (tessedit_ok_mode, 5, "Acceptance decision algorithm");
extern BOOL_VAR_H (tessedit_use_nn, TRUE, "");
extern BOOL_VAR_H (tessedit_rejection_debug, FALSE, "Adaption debug");
extern BOOL_VAR_H (tessedit_rejection_stats, FALSE, "Show NN stats");
extern BOOL_VAR_H (tessedit_flip_0O, TRUE, "Contextual 0O O0 flips");
extern double_VAR_H (tessedit_lower_flip_hyphen, 1.5,
"Aspect ratio dot/hyphen test");
extern double_VAR_H (tessedit_upper_flip_hyphen, 1.8,
"Aspect ratio dot/hyphen test");
extern BOOL_VAR_H (rej_trust_doc_dawg, FALSE,
"Use DOC dawg in 11l conf. detector");
extern BOOL_VAR_H (rej_1Il_use_dict_word, FALSE, "Use dictword test");
extern BOOL_VAR_H (rej_1Il_trust_permuter_type, TRUE, "Dont double check");
extern BOOL_VAR_H (one_ell_conflict_default, TRUE,
"one_ell_conflict default");
extern BOOL_VAR_H (show_char_clipping, FALSE, "Show clip image window?");
extern BOOL_VAR_H (nn_debug, FALSE, "NN DEBUGGING?");
extern BOOL_VAR_H (nn_reject_debug, FALSE, "NN DEBUG each char?");
extern BOOL_VAR_H (nn_lax, FALSE, "Use 2nd rate matches");
extern BOOL_VAR_H (nn_double_check_dict, FALSE, "Double check");
extern BOOL_VAR_H (nn_conf_double_check_dict, TRUE,
"Double check for confusions");
extern BOOL_VAR_H (nn_conf_1Il, TRUE, "NN use 1Il conflicts");
extern BOOL_VAR_H (nn_conf_Ss, TRUE, "NN use Ss conflicts");
extern BOOL_VAR_H (nn_conf_hyphen, TRUE, "NN hyphen conflicts");
extern BOOL_VAR_H (nn_conf_test_good_qual, FALSE, "NN dodgy 1Il cross check");
extern BOOL_VAR_H (nn_conf_test_dict, TRUE, "NN dodgy 1Il cross check");
extern BOOL_VAR_H (nn_conf_test_sensible, TRUE, "NN dodgy 1Il cross check");
extern BOOL_VAR_H (nn_conf_strict_on_dodgy_chs, TRUE,
"Require stronger NN match");
extern double_VAR_H (nn_dodgy_char_threshold, 0.99, "min accept score");
extern INT_VAR_H (nn_conf_accept_level, 4, "NN accept dodgy 1Il matches? ");
extern INT_VAR_H (nn_conf_initial_i_level, 3,
"NN accept initial Ii match level ");
extern BOOL_VAR_H (no_unrej_dubious_chars, TRUE,
"Dubious chars next to reject?");
extern BOOL_VAR_H (no_unrej_no_alphanum_wds, TRUE,
"Stop unrej of non A/N wds?");
extern BOOL_VAR_H (no_unrej_1Il, FALSE, "Stop unrej of 1Ilchars?");
extern BOOL_VAR_H (rej_use_tess_accepted, TRUE,
"Individual rejection control");
extern BOOL_VAR_H (rej_use_tess_blanks, TRUE, "Individual rejection control");
extern BOOL_VAR_H (rej_use_good_perm, TRUE, "Individual rejection control");
extern BOOL_VAR_H (rej_use_sensible_wd, FALSE, "Extend permuter check");
extern BOOL_VAR_H (rej_alphas_in_number_perm, FALSE, "Extend permuter check");
extern double_VAR_H (rej_whole_of_mostly_reject_word_fract, 0.85,
"if >this fract");
extern INT_VAR_H (rej_mostly_reject_mode, 1,
"0-never, 1-afterNN, 2-after new xht");
extern double_VAR_H (tessed_fullstop_aspect_ratio, 1.2,
"if >this fract then reject");
extern INT_VAR_H (net_image_width, 40, "NN input image width");
extern INT_VAR_H (net_image_height, 36, "NN input image height");
extern INT_VAR_H (net_image_x_height, 22, "NN input image x_height");
extern INT_VAR_H (tessedit_image_border, 2, "Rej blbs near image edge limit");
extern INT_VAR_H (net_bl_nodes, 20, "Number of baseline nodes");
extern double_VAR_H (nn_reject_threshold, 0.5, "NN min accept score");
extern double_VAR_H (nn_reject_head_and_shoulders, 0.6,
"top scores sep factor");
extern STRING_VAR_H (ok_single_ch_non_alphanum_wds, "-?\075",
"Allow NN to unrej");
extern STRING_VAR_H (ok_repeated_ch_non_alphanum_wds, "-?*\075",
"Allow NN to unrej");
extern STRING_VAR_H (conflict_set_I_l_1, "Il1[]", "Il1 conflict set");
extern STRING_VAR_H (conflict_set_S_s, "Ss$", "Ss conflict set");
extern STRING_VAR_H (conflict_set_hyphen, "-_~", "hyphen conflict set");
extern STRING_VAR_H (dubious_chars_left_of_reject, "!'+`()-./\\<>;:^_,~\"",
"Unreliable chars");
extern STRING_VAR_H (dubious_chars_right_of_reject, "!'+`()-./\\<>;:^_,~\"",
"Unreliable chars");
extern INT_VAR_H (min_sane_x_ht_pixels, 8,
"Reject any x-ht lt or eq than this");
void reject_blanks(WERD_RES *word);
void reject_I_1_L(WERD_RES *word);
                                 //detailed results
void reject_poor_matches(WERD_RES *word, BLOB_CHOICE_LIST_CLIST *blob_choices);
float compute_reject_threshold(  //compute threshold //detailed results
                               BLOB_CHOICE_LIST_CLIST *blob_choices);
int sort_floats(                   //qsort function
                const void *arg1,  //ptrs to floats
                const void *arg2);
void reject_edge_blobs(WERD_RES *word);
BOOL8 word_contains_non_1_digit(const char *word,
                                const char *word_lengths);
                                 //of character
inT16 nn_match_char(IMAGE &scaled_image,
                    float baseline_pos,       //rel to scaled_image
                    BOOL8 dict_word,          //part of dict wd?
                    BOOL8 checked_dict_word,  //part of dict wd?
                    BOOL8 sensible_word,      //part acceptable str?
                    BOOL8 centre,             //not at word ends?
                    BOOL8 good_quality_word,  //initial segmentation
                    char tess_ch              //confirm this?
                   );
inT16 evaluate_net_match(char top,
                         float top_score,
                         char next,
                         float next_score,
                         char tess_ch,
                         BOOL8 dict_word,
                         BOOL8 checked_dict_word,
                         BOOL8 sensible_word,
                         BOOL8 centre,
                         BOOL8 good_quality_word);
void dont_allow_dubious_chars(WERD_RES *word);

void dont_allow_1Il(WERD_RES *word);

void reject_mostly_rejects(  //rej all if most rejectd
                           WERD_RES *word);
void flip_hyphens(WERD_RES *word);
void flip_0O(WERD_RES *word);
BOOL8 non_0_digit(const char* str, int length);
#endif
