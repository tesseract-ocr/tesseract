/**********************************************************************
 * File:        paircmp.cpp  (Formerly paircmp.c)
 * Description: Code to compare two blobs using the adaptive matcher
 * Author:		Ray Smith
 * Created:		Wed Apr 21 09:31:02 BST 1993
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include          "blobcmp.h"
#include          "tfacep.h"
#include          "paircmp.h"
#include          "tesseractclass.h"

#define EXTERN

/**********************************************************************
 * compare_blob_pairs
 *
 * A blob processor to compare pairs of selected blobs.
 **********************************************************************/

namespace tesseract {
BOOL8 Tesseract::compare_blob_pairs(             //blob processor
                                    BLOCK *,
                                    ROW *row,    //row it came from
                                    WERD *,
                                    PBLOB *blob  //blob to compare
                                   ) {
  static ROW *prev_row = NULL;   //other in pair
  static PBLOB *prev_blob = NULL;
  float rating;                  //from matcher

  if (prev_row == NULL || prev_blob == NULL) {
    prev_row = row;
    prev_blob = blob;
  }
  else {
    rating = compare_blobs (prev_blob, prev_row, blob, row);
    tprintf ("Rating=%g\n", rating);
    prev_row = NULL;
    prev_blob = NULL;
  }
  return TRUE;
}


/**********************************************************************
 * compare_blobs
 *
 * Compare 2 blobs and return the rating.
 **********************************************************************/

float Tesseract::compare_blobs(               //match 2 blobs
                               PBLOB *blob1,  //first blob
                               ROW *row1,     //row it came from
                               PBLOB *blob2,  //other blob
                               ROW *row2) {
  PBLOB *bn_blob1;               //baseline norm
  PBLOB *bn_blob2;
  DENORM denorm1, denorm2;
  float rating;                  //match result

  bn_blob1 = blob1->baseline_normalise (row1, &denorm1);
  bn_blob2 = blob2->baseline_normalise (row2, &denorm2);
  rating = compare_bln_blobs (bn_blob1, &denorm1, bn_blob2, &denorm2);
  delete bn_blob1;
  delete bn_blob2;
  return rating;
}


/**********************************************************************
 * compare_bln_blobs
 *
 * Compare 2 baseline normalised blobs and return the rating.
 **********************************************************************/
float Tesseract::compare_bln_blobs(               //match 2 blobs
                                   PBLOB *blob1,  //first blob
                                   DENORM *denorm1,
                                   PBLOB *blob2,  //other blob
                                   DENORM *denorm2) {
  TBLOB *tblob1;                 //tessblobs
  TBLOB *tblob2;
  TEXTROW tessrow1, tessrow2;    //tess rows
  float rating;                  //match result

  tblob1 = make_tess_blob (blob1, TRUE);
  make_tess_row(denorm1, &tessrow1); 
  tblob2 = make_tess_blob (blob2, TRUE);
  make_tess_row(denorm2, &tessrow2); 
  rating = compare_tess_blobs (tblob1, &tessrow1, tblob2, &tessrow2);
  free_blob(tblob1); 
  free_blob(tblob2); 

  return rating;
}
}  // namespace tesseract
