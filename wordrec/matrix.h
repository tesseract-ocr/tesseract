/* -*-C-*-
 ********************************************************************************
 *
 * File:        matrix.h  (Formerly matrix.h)
 * Description:  Ratings matrix code. (Used by associator)
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 13:22:06 1990
 * Modified:     Tue Mar 19 16:00:20 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
#ifndef MATRIX_H
#define MATRIX_H

#include "ratngs.h"
#include "unicharset.h"

static BLOB_CHOICE_LIST* NOT_CLASSIFIED = NULL;

// A generic class to store a matrix with entries of type T.
template <class T>
class GENERIC_MATRIX {
 public:
  // Allocate a piece of memory to hold a matrix of the given dimension.
  // Initialize all the elements of the matrix to empty instead of assuming
  // that a default constructor can be used.
  GENERIC_MATRIX(int dimension, const T& empty) : empty_(empty) {
    matrix_ = new T[dimension * dimension];
    dimension_ = dimension;
    for (int x = 0; x < dimension; x++)
      for (int y = 0; y < dimension; y++)
        this->put(x, y, empty_);
  }
  ~GENERIC_MATRIX() { delete[] matrix_; }

  // Provide the dimension of this square matrix.
  long dimension() const { return dimension_; }

  // Expression to select a specific location in the matrix.
  int index(int column, int row) const {
    return (row * this->dimension() + column);
  }

  // Put a list element into the matrix at a specific location.
  void put(int column, int row, const T& thing) {
    matrix_[this->index(column, row)] = thing;
  }

  // Get the item at a specified location from the matrix.
  T get(int column, int row) const {
    return matrix_[this->index(column, row)];
  }

  // Delete objects pointed to by matrix_[i].
  void delete_matrix_pointers() {
    for (int x = 0; x < this->dimension(); x++) {
      for (int y = 0; y < this->dimension(); y++) {
        T matrix_cell = this->get(x, y);
        if (matrix_cell != empty_)
          delete matrix_cell;
      }
    }
  }

 private:
  T *matrix_;
  T empty_;  // The unused cell.
  int dimension_;
};

class MATRIX : public GENERIC_MATRIX<BLOB_CHOICE_LIST *> {
 public:
  MATRIX(int dimension) : GENERIC_MATRIX<BLOB_CHOICE_LIST *>(dimension,
                                                             NOT_CLASSIFIED) {}
  // Print a shortened version of the contents of the matrix.
  void print(const UNICHARSET &current_unicharset);
};

#endif
