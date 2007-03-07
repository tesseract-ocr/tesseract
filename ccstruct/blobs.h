/* -*-C-*-
 ********************************************************************************
 *
 * File:        blobs.h  (Formerly blobs.h)
 * Description:  Blob definition
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 27 15:39:52 1989
 * Modified:     Thu Mar 28 15:33:38 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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

#ifndef BLOBS_H
#define BLOBS_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include                   "vecfuncs.h"
#include  "tessclas.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct
{                                /* Widths of pieces */
  int num_chars;
  int widths[1];
} WIDTH_RECORD;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * free_widths
 *
 * Free the memory taken up by a width array.
 **********************************************************************/
#define free_widths(w)  \
if (w) memfree (w)

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void blob_origin(TBLOB *blob,      /*blob to compute on */
                 TPOINT *origin);  /*return value */

                                 /*blob to compute on */
void blob_bounding_box(TBLOB *blob,
                       register TPOINT *topleft,  /*bounding box */
                       register TPOINT *botright);

void blobs_bounding_box(TBLOB *blobs, TPOINT *topleft, TPOINT *botright); 

void blobs_origin(TBLOB *blobs,     /*blob to compute on */
                  TPOINT *origin);  /*return value */

                                 /*blob to compute on */
WIDTH_RECORD *blobs_widths(TBLOB *blobs); 

int count_blobs(TBLOB *blobs); 

void delete_word(TWERD *word); 

void delete_edgepts(register EDGEPT *edgepts); 

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* blobs.c
void blob_origin
  _ARGS((BLOB *blob,
  TPOINT *origin));

void blob_bounding_box
  _ARGS((BLOB *blob,
  TPOINT *topleft,
  TPOINT *botright));

void blobs_bounding_box
  _ARGS((BLOB *blobs,
  TPOINT *topleft,
  TPOINT *botright));

void blobs_origin
  _ARGS((BLOB *blobs,
  TPOINT *origin));

WIDTH_RECORD *blobs_widths
  _ARGS((BLOB *blobs));

int count_blobs
  _ARGS((BLOB *blobs));

void delete_word
  _ARGS((TWERD *word));

void delete_edgepts
  _ARGS((EDGEPT *edgepts));
#undef _ARGS
*/
#endif
