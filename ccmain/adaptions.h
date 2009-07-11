/**********************************************************************
 * File:        adaptions.h  (Formerly adaptions.h)
 * Description: Functions used to adapt to blobs already confidently
 *					identified
 * Author:		Chris Newton
 * Created:		Thu Oct  7 10:17:28 BST 1993
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

#ifndef           ADAPTIONS_H
#define           ADAPTIONS_H

#include          "charsample.h"
#include          "charcut.h"
#include          "notdll.h"

extern BOOL_VAR_H (tessedit_reject_ems, FALSE, "Reject all m's");
extern BOOL_VAR_H (tessedit_reject_suspect_ems, FALSE, "Reject suspect m's");
extern double_VAR_H (tessedit_cluster_t1, 0.20,
"t1 threshold for clustering samples");
extern double_VAR_H (tessedit_cluster_t2, 0.40,
"t2 threshold for clustering samples");
extern double_VAR_H (tessedit_cluster_t3, 0.12,
"Extra threshold for clustering samples, only keep a new sample if best score greater than this value");
extern double_VAR_H (tessedit_cluster_accept_fraction, 0.80,
"Largest fraction of characters in cluster for it to be used for adaption");
extern INT_VAR_H (tessedit_cluster_min_size, 3,
"Smallest number of samples in a cluster for it to be used for adaption");
extern BOOL_VAR_H (tessedit_cluster_debug, FALSE,
"Generate and print debug information for adaption by clustering");
extern BOOL_VAR_H (tessedit_use_best_sample, FALSE,
"Use best sample from cluster when adapting");
extern BOOL_VAR_H (tessedit_test_cluster_input, FALSE,
"Set reject map to enable cluster input to be measured");
extern BOOL_VAR_H (tessedit_matrix_match, TRUE, "Use matrix matcher");
extern BOOL_VAR_H (tessedit_old_matrix_match, FALSE, "Use matrix matcher");
extern BOOL_VAR_H (tessedit_mm_use_non_adaption_set, FALSE,
"Don't try to adapt to characters on this list");
extern STRING_VAR_H (tessedit_non_adaption_set, ",.;:'~@*",
"Characters to be avoided when adapting");
extern BOOL_VAR_H (tessedit_mm_adapt_using_prototypes, TRUE,
"Use prototypes when adapting");
extern BOOL_VAR_H (tessedit_mm_use_prototypes, TRUE,
"Use prototypes as clusters are built");
extern BOOL_VAR_H (tessedit_mm_use_rejmap, FALSE,
"Adapt to characters using reject map");
extern BOOL_VAR_H (tessedit_mm_all_rejects, FALSE,
"Adapt to all characters using, matrix matcher");
extern BOOL_VAR_H (tessedit_mm_only_match_same_char, FALSE,
"Only match samples against clusters for the same character");
extern BOOL_VAR_H (tessedit_process_rns, FALSE, "Handle m - rn ambigs");
extern BOOL_VAR_H (tessedit_demo_adaption, FALSE,
"Display cut images and matrix match for demo purposes");
extern INT_VAR_H (tessedit_demo_word1, 62,
"Word number of first word to display");
extern INT_VAR_H (tessedit_demo_word2, 64,
"Word number of second word to display");
extern STRING_VAR_H (tessedit_demo_file, "academe",
"Name of document containing demo words");
extern BOOL_VAR_H(tessedit_adapt_to_char_fragments, TRUE,
                  "Adapt to words that contain "
                  " a character composed form fragments");

void print_em_stats(CHAR_SAMPLES_LIST *char_clusters,
                    CHAR_SAMPLE_LIST *chars_waiting);
                                 //lines of the image
CHAR_SAMPLE *clip_sample(PIXROW *pixrow,
                         IMAGELINE *imlines,
                         TBOX pix_box,  //box of imlines extent
                         BOOL8 white_on_black,
                         char c);
void display_cluster_prototypes(CHAR_SAMPLES_LIST *char_clusters);
void reject_all_ems(WERD_RES *word);
void reject_all_fullstops(WERD_RES *word);
void reject_suspect_fullstops(WERD_RES *word);
BOOL8 suspect_em(WERD_RES *word, inT16 index);
BOOL8 suspect_fullstop(WERD_RES *word, inT16 i);
#endif
