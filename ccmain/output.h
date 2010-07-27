/******************************************************************
 * File:        output.h  (Formerly output.h)
 * Description: Output pass
 * Author:		Phil Cheatle
 * Created:		Thu Aug  4 10:56:08 BST 1994
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
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

#ifndef           OUTPUT_H
#define           OUTPUT_H

#include          "varable.h"
//#include                                      "epapconv.h"
#include          "pageres.h"
#include          "notdll.h"

extern BOOL_EVAR_H (tessedit_write_block_separators, TRUE,
"Write block separators in output");
extern BOOL_VAR_H (tessedit_write_raw_output, FALSE,
"Write raw stuff to name.raw");
extern BOOL_EVAR_H (tessedit_write_output, TRUE, "Write text to name.txt");
extern BOOL_EVAR_H (tessedit_write_txt_map, TRUE,
"Write .txt to .etx map file");
extern BOOL_EVAR_H (tessedit_write_rep_codes, TRUE,
"Write repetition char code");
extern BOOL_EVAR_H (tessedit_write_unlv, FALSE, "Write .unlv output file");
extern STRING_EVAR_H (unrecognised_char, "|",
"Output char for unidentified blobs");
extern INT_EVAR_H (suspect_level, 99, "Suspect marker level");
extern INT_VAR_H (suspect_space_level, 100,
"Min suspect level for rejecting spaces");
extern INT_VAR_H (suspect_short_words, 2,
"Dont Suspect dict wds longer than this");
extern BOOL_VAR_H (suspect_constrain_1Il, FALSE,
"UNLV keep 1Il chars rejected");
extern double_VAR_H (suspect_rating_per_ch, 999.9,
"Dont touch bad rating limit");
extern double_VAR_H (suspect_accept_rating, -999.9,
"Accept good rating limit");
extern BOOL_EVAR_H (tessedit_minimal_rejection, FALSE,
"Only reject tess failures");
extern BOOL_VAR_H (tessedit_zero_rejection, FALSE, "Dont reject ANYTHING");
extern BOOL_VAR_H (tessedit_word_for_word, FALSE,
"Make output have exactly one word per WERD");
extern BOOL_VAR_H (tessedit_consistent_reps, TRUE,
"Force all rep chars the same");

/** output a word */
void write_results(
                   PAGE_RES_IT &page_res_it,  ///< full info
                   char newline_type,         ///< type of newline
                   BOOL8 force_eol,           ///< override tilde crunch?
                   BOOL8 write_to_shm         ///< send to api
                  );

/** convert one word */
WERD_CHOICE *make_epaper_choice(
                                WERD_RES *word,    ///< word to do
                                char newline_type  ///< type of newline
                               );
/** make reject code */
inT16 make_reject (
TBOX * inset_box,                ///< bounding box
inT16 prevright,                 ///< previous char
inT16 nextleft,                  ///< next char
DENORM * denorm,                 ///< de-normalizer
char word_string[]               ///< output string
);

/** test line ends */
char determine_newline_type(WERD *word,        ///< word to do
                            BLOCK *block,      ///< current block
                            WERD *next_word,   ///< next word
                            BLOCK *next_block  ///< block of next word
                           );
/** write output */
void write_cooked_text(WERD *word,          ///< word to do
                       const STRING &text,  ///< text to write
                       BOOL8 acceptable,    ///< good stuff
                       BOOL8 pass2,         ///< done on pass2
                       FILE *fp             ///< file to write
                      );
/** write output */
void write_shm_text(WERD_RES *word,     ///< word to do
                    BLOCK *block,       ///< block it is from
                    ROW_RES *row,       ///< row it is from
                    const STRING &text, ///< text to write
                    const STRING &text_lengths
                   );
/** output a map file */
void write_map(
               FILE *mapfile,  ///< mapfile to write to
               WERD_RES *word  ///< word
              );
/*FILE *open_outfile(  //open .map & .unlv file
                   const char *extension);*/
void write_unlv_text(WERD_RES *word);
void ensure_rep_chars_are_consistent(WERD_RES *word);
#endif
