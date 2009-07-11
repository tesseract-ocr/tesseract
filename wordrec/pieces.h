/* -*-C-*-
 ********************************************************************************
 *
 * File:        pieces.h  (Formerly pieces.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Apr 30 11:49:11 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef PIECES_H
#define PIECES_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "matrix.h"
#include "seam.h"
#include "states.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct
{                                /*  BOUNDS  */
  TPOINT topleft;
  TPOINT botright;
} BOUNDS;

typedef BOUNDS *BOUNDS_LIST;     /*  BOUNDS_LIST  */

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void break_pieces(TBLOB *blobs, SEAMS seams, inT16 start, inT16 end);

void join_pieces(TBLOB *piece_blobs, SEAMS seams, inT16 start, inT16 end);

void hide_seam(SEAM *seam);

void hide_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

void reveal_seam(SEAM *seam);

void reveal_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

void bounds_of_piece(BOUNDS_LIST bounds,
                     inT16 start,
                     inT16 end,
                     TPOINT *extreme_tl,
                     TPOINT *extreme_br);

BOUNDS_LIST record_blob_bounds(TBLOB *blobs);

MATRIX *record_piece_ratings(TBLOB *blobs);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* pieces.c *
void break_pieces
  _ARGS((BLOB *blobs,
  SEAMS seams));

void join_pieces
  _ARGS((BLOB *piece_blobs,
  SEAMS seams,
  inT16 start,
  inT16 end));

void hide_seam
  _ARGS((SEAM *seam));

void reveal_seam
  _ARGS((SEAM *seam));

void bounds_of_piece
  _ARGS((BOUNDS_LIST bounds,
  inT16 start,
  inT16 end,
  TPOINT *extreme_tl,
  TPOINT *extreme_br));

CHOICES classify_piece
  _ARGS((BLOB *pieces,
  SEAMS seams,
  inT16 start,
  inT16 end,
  inT32 fx,
  STATE* this_state,
  STATE* best_state,
  inT32 pass,
  inT32 blob_index));

CHOICES get_piece_rating
  _ARGS((MATRIX ratings,
  BLOB *blobs,
  SEAMS seams,
  inT16 start,
  inT16 end,
  inT32 fx,
  STATE* this_state,
  STATE* best_state,
  inT32 pass,
  inT32 blob_index));

BOUNDS_LIST record_blob_bounds
  _ARGS((BLOB *blobs));

MATRIX record_piece_ratings
  _ARGS((BLOB *blobs));

#undef _ARGS
*/
#endif
