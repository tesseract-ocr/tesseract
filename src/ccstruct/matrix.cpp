/******************************************************************************
 *
 * File:         matrix.cpp  (Formerly matrix.c)
 * Description:  Ratings matrix code. (Used by associator)
 * Author:       Mark Seaman, OCR Technology
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
 *****************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "matrix.h"

#include "ratngs.h"
#include "tprintf.h"
#include "unicharset.h"

namespace tesseract {

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
MATRIX::~MATRIX() = default;

// Returns true if there are any real classification results.
bool MATRIX::Classified(int col, int row, int wildcard_id) const {
  if (get(col, row) == NOT_CLASSIFIED) {
    return false;
  }
  BLOB_CHOICE_IT b_it(get(col, row));
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    BLOB_CHOICE *choice = b_it.data();
    if (choice->IsClassified()) {
      return true;
    }
  }
  return false;
}

// Expands the existing matrix in-place to make the band wider, without
// losing any existing data.
void MATRIX::IncreaseBandSize(int bandwidth) {
  ResizeWithCopy(dimension(), bandwidth);
}

// Returns a bigger MATRIX with a new column and row in the matrix in order
// to split the blob at the given (ind,ind) diagonal location.
// Entries are relocated to the new MATRIX using the transformation defined
// by MATRIX_COORD::MapForSplit.
// Transfers the pointer data to the new MATRIX and deletes *this.
MATRIX *MATRIX::ConsumeAndMakeBigger(int ind) {
  int dim = dimension();
  int band_width = bandwidth();
  // Check to see if bandwidth needs expanding.
  for (int col = ind; col >= 0 && col > ind - band_width; --col) {
    if (array_[col * band_width + band_width - 1] != empty_) {
      ++band_width;
      break;
    }
  }
  auto *result = new MATRIX(dim + 1, band_width);

  for (int col = 0; col < dim; ++col) {
    for (int row = col; row < dim && row < col + bandwidth(); ++row) {
      MATRIX_COORD coord(col, row);
      coord.MapForSplit(ind);
      BLOB_CHOICE_LIST *choices = get(col, row);
      if (choices != nullptr) {
        // Correct matrix location on each choice.
        BLOB_CHOICE_IT bc_it(choices);
        for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
          BLOB_CHOICE *choice = bc_it.data();
          choice->set_matrix_cell(coord.col, coord.row);
        }
        ASSERT_HOST(coord.Valid(*result));
        result->put(coord.col, coord.row, choices);
      }
    }
  }
  delete this;
  return result;
}

// Makes and returns a deep copy of *this, including all the BLOB_CHOICEs
// on the lists, but not any LanguageModelState that may be attached to the
// BLOB_CHOICEs.
MATRIX *MATRIX::DeepCopy() const {
  int dim = dimension();
  int band_width = bandwidth();
  auto *result = new MATRIX(dim, band_width);
  for (int col = 0; col < dim; ++col) {
    for (int row = col; row < dim && row < col + band_width; ++row) {
      BLOB_CHOICE_LIST *choices = get(col, row);
      if (choices != nullptr) {
        auto *copy_choices = new BLOB_CHOICE_LIST;
        copy_choices->deep_copy(choices, &BLOB_CHOICE::deep_copy);
        result->put(col, row, copy_choices);
      }
    }
  }
  return result;
}

// Print the best guesses out of the match rating matrix.
void MATRIX::print(const UNICHARSET &unicharset) const {
  tprintf("Ratings Matrix (top 3 choices)\n");
  int dim = dimension();
  int band_width = bandwidth();
  int row, col;
  for (col = 0; col < dim; ++col) {
    for (row = col; row < dim && row < col + band_width; ++row) {
      BLOB_CHOICE_LIST *rating = this->get(col, row);
      if (rating == NOT_CLASSIFIED) {
        continue;
      }
      BLOB_CHOICE_IT b_it(rating);
      tprintf("col=%d row=%d ", col, row);
      for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
        tprintf("%s rat=%g cert=%g ", unicharset.id_to_unichar(b_it.data()->unichar_id()),
                b_it.data()->rating(), b_it.data()->certainty());
      }
      tprintf("\n");
    }
    tprintf("\n");
  }
  tprintf("\n");
  for (col = 0; col < dim; ++col) {
    tprintf("\t%d", col);
  }
  tprintf("\n");
  for (row = 0; row < dim; ++row) {
    for (col = 0; col <= row; ++col) {
      if (col == 0) {
        tprintf("%d\t", row);
      }
      if (row >= col + band_width) {
        tprintf(" \t");
        continue;
      }
      BLOB_CHOICE_LIST *rating = this->get(col, row);
      if (rating != NOT_CLASSIFIED) {
        BLOB_CHOICE_IT b_it(rating);
        int counter = 0;
        for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
          tprintf("%s ", unicharset.id_to_unichar(b_it.data()->unichar_id()));
          ++counter;
          if (counter == 3) {
            break;
          }
        }
        tprintf("\t");
      } else {
        tprintf(" \t");
      }
    }
    tprintf("\n");
  }
}

} // namespace tesseract
