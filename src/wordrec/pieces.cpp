/******************************************************************************
 *
 * File:         pieces.cpp
 * Description:
 * Author:       Mark Seaman, OCR Technology
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
 *****************************************************************************/
/*----------------------------------------------------------------------
          I n c l u d e s
----------------------------------------------------------------------*/

#include "blobs.h"
#include "helpers.h"
#include "matrix.h"
#include "ratngs.h"
#include "seam.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

using tesseract::ScoredFont;

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * classify_piece
 *
 * Create a larger piece from a collection of smaller ones.  Classify
 * it and return the results.  Take the large piece apart to leave
 * the collection of small pieces un modified.
 **********************************************************************/
namespace tesseract {
BLOB_CHOICE_LIST *Wordrec::classify_piece(const std::vector<SEAM *> &seams, int16_t start,
                                          int16_t end, const char *description, TWERD *word,
                                          BlamerBundle *blamer_bundle) {
  if (end > start) {
    SEAM::JoinPieces(seams, word->blobs, start, end);
  }
  BLOB_CHOICE_LIST *choices =
      classify_blob(word->blobs[start], description, ScrollView::WHITE, blamer_bundle);
  // Set the matrix_cell_ entries in all the BLOB_CHOICES.
  BLOB_CHOICE_IT bc_it(choices);
  for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
    bc_it.data()->set_matrix_cell(start, end);
  }

  if (end > start) {
    SEAM::BreakPieces(seams, word->blobs, start, end);
  }

  return (choices);
}

template <class BLOB_CHOICE>
int SortByUnicharID(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *static_cast<const BLOB_CHOICE *const *>(void1);
  const BLOB_CHOICE *p2 = *static_cast<const BLOB_CHOICE *const *>(void2);

  return p1->unichar_id() - p2->unichar_id();
}

template <class BLOB_CHOICE>
int SortByRating(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *static_cast<const BLOB_CHOICE *const *>(void1);
  const BLOB_CHOICE *p2 = *static_cast<const BLOB_CHOICE *const *>(void2);

  if (p1->rating() < p2->rating()) {
    return 1;
  }
  return -1;
}

} // namespace tesseract
