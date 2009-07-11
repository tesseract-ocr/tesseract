/**********************************************************************
 * File:        fixxht.h  (Formerly fixxht.h)
 * Description: Improve x_ht and look out for case inconsistencies
 * Author:		Phil Cheatle
 * Created:		Thu Aug  5 14:11:08 BST 1993
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

#ifndef           FIXXHT_H
#define           FIXXHT_H

#include          "varable.h"
#include          "statistc.h"
#include          "pageres.h"
#include          "notdll.h"

extern double_VAR_H (x_ht_fraction_of_caps_ht, 0.7,
"Fract of cps ht est of xht");
extern double_VAR_H (x_ht_variation, 0.35,
"Err band as fract of caps/xht dist");
extern double_VAR_H (x_ht_sub_variation, 0.5,
"Err band as fract of caps/xht dist");
extern BOOL_VAR_H (rej_trial_ambigs, TRUE,
"reject x-ht ambigs when under trial");
extern BOOL_VAR_H (x_ht_conservative_ambigs, FALSE,
"Dont rely on ambigs + maxht");
extern BOOL_VAR_H (x_ht_check_est, TRUE, "Cross check estimates");
extern BOOL_VAR_H (x_ht_case_flip, FALSE, "Flip or reject suspect case");
extern BOOL_VAR_H (x_ht_include_dodgy_blobs, TRUE,
"Include blobs with possible noise?");
extern BOOL_VAR_H (x_ht_limit_flip_trials, TRUE,
"Dont do trial flips when ambigs are close to xht?");
extern BOOL_VAR_H (rej_use_check_block_occ, TRUE,
"Analyse rejection behaviour");
extern STRING_VAR_H (chs_non_ambig_caps_ht,
"!#$%&()/12346789?ABDEFGHIKLNQRT[]\\bdfhkl",
"Reliable ascenders");
extern STRING_VAR_H (chs_x_ht, "acegmnopqrsuvwxyz", "X height chars");
extern STRING_VAR_H (chs_non_ambig_x_ht, "aenqr", "reliable X height chars");
extern STRING_VAR_H (chs_ambig_caps_x, "cCmMoO05sSuUvVwWxXzZ",
"X ht or caps ht chars");
extern STRING_VAR_H (chs_bl_ambig_caps_x, "pPyY",
" Caps or descender ambigs");
extern STRING_VAR_H (chs_caps_ht,
"!#$%&()/0123456789?ABCDEFGHIJKLMNOPQRSTUVWXYZ[]\\bdfhkl{|}",
"Ascender chars");
extern STRING_VAR_H (chs_desc, "gjpqy", "Descender chars");
extern STRING_VAR_H (chs_non_ambig_bl,
"!#$%&01246789?ABCDEFGHIKLMNORSTUVWXYZabcdehiklmnorstuvwxz",
"Reliable baseline chars");
extern STRING_VAR_H (chs_odd_top, "ijt", "Chars with funny ascender region");
extern STRING_VAR_H (chs_odd_bot, "()35JQ[]\\/{}|", "Chars with funny base");
extern STRING_VAR_H (chs_bl,
"!#$%&()/01246789?ABCDEFGHIJKLMNOPRSTUVWXYZ[]\\abcdefhiklmnorstuvwxz{}",
"Baseline chars");
extern STRING_VAR_H (chs_non_ambig_desc, "gq", "Reliable descender chars");
void re_estimate_x_ht(                     //improve for 1 word
                      WERD_RES *word_res,  //word to do
                      float *trial_x_ht    //new match value
                     );
void check_blob_occ(char *proposed_char,
                    inT16 blob_ht_above_baseline,
                    float x_ht,
                    float caps_ht,
                    char *confirmed_char);
float estimate_from_stats(STATS &stats);
void improve_estimate(WERD_RES *word_res,
                      float &est_x_ht,
                      float &est_caps_ht,
                      STATS &x_ht,
                      STATS &caps_ht);
void reject_ambigs(  //rej any accepted xht ambig chars
                   WERD_RES *word);
                                 //xht ambig ht stats
void est_ambigs(WERD_RES *word_res,
                STATS &stats,
                float *ambig_lc_x_est,    //xht est
                float *ambig_uc_caps_est  //caps est
               );
BOOL8 dodgy_blob(PBLOB *blob);
#endif
