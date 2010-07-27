/* -*-C-*-
 ********************************************************************************
 *
 * File:        associate.h  (Formerly associate.h)
 * Description:  Associate the outlines and classify them
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon Feb  5 11:42:51 1990
 * Modified:     Tue May 21 15:34:56 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 ********************************************************************************
 */

#ifndef ASSOCIATE_H
#define ASSOCIATE_H

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include "matrix.h"
#include "states.h"
#include "blobs.h"
#include "split.h"
#include "seam.h"

/*
----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------
*/

/** List of (BLOB*) */
typedef LIST BLOBS;

/** List of (TESSLINE*) */
typedef LIST OUTLINES;

/** List of (EDGEPT*) */
typedef LIST EDGEPTS;

typedef inT16 BLOB_WEIGHTS[MAX_NUM_CHUNKS];

/** Each char evaluated */
typedef struct
{
  float match;
  float certainty;
  char character;
  int width;
  int gap;
} EVALUATION_RECORD;

/** Classification info for chunks */
struct CHUNKS_RECORD
{
  MATRIX *ratings;
  TBLOB *chunks;
  SEAMS splits;
  TEXTROW *row;
  int fx;
  int x_height;
  WIDTH_RECORD *chunk_widths;
  WIDTH_RECORD *char_widths;
  inT16 *weights;
};

/** Each segmentation */
typedef EVALUATION_RECORD EVALUATION_ARRAY[MAX_NUM_CHUNKS];

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern EVALUATION_ARRAY last_segmentation;
extern WIDTH_RECORD *char_widths;
extern BOOL_VAR_H(wordrec_enable_assoc, 1, "Associator Enable");
extern BOOL_VAR_H(force_word_assoc, FALSE,
       "always force associator to run, independent of what enable_assoc is."
       "This is used for CJK where component grouping is necessary.");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void init_associate_vars();
void print_eval_record(const char* label, EVALUATION_RECORD *eval_rec);
#endif
