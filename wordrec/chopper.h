/* -*-C-*-
 ********************************************************************************
 *
 * File:        chopper.h  (Formerly chopper.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed May 15 14:24:26 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *********************************************************************************/

#ifndef CHOPPER_H
#define CHOPPER_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "choicearr.h"
#include "seam.h"
#include "matrix.h"
#include "stopper.h"
#include "states.h"
#include "cutil.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void preserve_outline(EDGEPT *start);

void preserve_outline_tree(TESSLINE *srcline);

EDGEPT *restore_outline(EDGEPT *start);

void restore_outline_tree(TESSLINE *srcline);

SEAM *attempt_blob_chop(TWERD *word, inT32 blob_number, SEAMS seam_list);

int any_shared_split_points(SEAMS seam_list, SEAM *seam);

int check_blob(TBLOB *blob);

CHOICES_LIST improve_one_blob(TWERD *word,
                              CHOICES_LIST char_choices,
                              int fx,
                              inT32 *blob_number,
                              SEAMS *seam_list,
                              DANGERR *fixpt,
                              STATE *this_state,
                              STATE *correct_state,
                              inT32 pass);

inT16 check_seam_order(TBLOB *blob, SEAM *seam);

CHOICES_LIST chop_word_main(register TWERD *word,
                            int fx,
                            A_CHOICE *best_choice,
                            A_CHOICE *raw_choice,
                            BOOL8 tester,
                            BOOL8 trainer);

void improve_by_chopping(register TWERD *word,
                         CHOICES_LIST *char_choices,
                         int fx,
                         STATE *best_state,
                         A_CHOICE *best_choice,
                         A_CHOICE *raw_choice,
                         SEAMS *seam_list,
                         DANGERR *fixpt,
                         STATE *chop_states,
                         inT32 *state_count,
                         STATE *correct_state,
                         inT32 pass);

inT16 select_blob_to_split(CHOICES_LIST char_choices, float rating_ceiling);

SEAMS start_seam_list(TBLOB *blobs);

inT16 total_containment(TBLOB *blob1, TBLOB *blob2);

MATRIX word_associator(TBLOB *blobs,
                       SEAMS seams,
                       STATE *state,
                       int fxid,
                       A_CHOICE *best_choice,
                       A_CHOICE *raw_choice,
                       char *correct,
                       DANGERR *fixpt,
                       STATE *best_state,
                       inT32 pass);
#endif
