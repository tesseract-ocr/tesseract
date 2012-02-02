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
#ifndef TESSERACT_CCSTRUCT_MATRIX_H__
#define TESSERACT_CCSTRUCT_MATRIX_H__

#include "ratngs.h"
#include "unicharset.h"

#define NOT_CLASSIFIED reinterpret_cast<BLOB_CHOICE_LIST*>(NULL)

// A generic class to store a matrix with entries of type T.
template <class T>
class GENERIC_2D_ARRAY {
 public:
  // Allocate a piece of memory to hold a 2d-array of the given dimension.
  // Initialize all the elements of the array to empty instead of assuming
  // that a default constructor can be used.
  GENERIC_2D_ARRAY(int dim1, int dim2, const T& empty)
    : empty_(empty), dim1_(dim1), dim2_(dim2)  {
    array_ = new T[dim1_ * dim2_];
    for (int x = 0; x < dim1_; x++)
      for (int y = 0; y < dim2_; y++)
        this->put(x, y, empty_);
  }
  ~GENERIC_2D_ARRAY() { delete[] array_; }

  // Writes to the given file. Returns false in case of error.
  // Only works with bitwise-serializeable types!
  bool Serialize(FILE* fp) const {
    if (!SerializeSize(fp)) return false;
    if (fwrite(&empty_, sizeof(empty_), 1, fp) != 1) return false;
    int size = dim1_ * dim2_;
    if (fwrite(array_, sizeof(*array_), size, fp) != size) return false;
    return true;
  }

  // Reads from the given file. Returns false in case of error.
  // Only works with bitwise-serializeable types!
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp) {
    if (!DeSerializeSize(swap, fp)) return false;
    if (fread(&empty_, sizeof(empty_), 1, fp) != 1) return false;
    if (swap) ReverseN(&empty_, sizeof(empty_));
    int size = dim1_ * dim2_;
    if (fread(array_, sizeof(*array_), size, fp) != size) return false;
    if (swap) {
      for (int i = 0; i < size; ++i)
        ReverseN(&array_[i], sizeof(array_[i]));
    }
    return true;
  }

  // Writes to the given file. Returns false in case of error.
  // Assumes a T::Serialize(FILE*) const function.
  bool SerializeClasses(FILE* fp) const {
    if (!SerializeSize(fp)) return false;
    if (!empty_.Serialize(fp)) return false;
    int size = dim1_ * dim2_;
    for (int i = 0; i < size; ++i) {
      if (!array_[i].Serialize(fp)) return false;
    }
    return true;
  }

  // Reads from the given file. Returns false in case of error.
  // Assumes a T::DeSerialize(bool swap, FILE*) function.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerializeClasses(bool swap, FILE* fp) {
    if (!DeSerializeSize(swap, fp)) return false;
    if (!empty_.DeSerialize(swap, fp)) return false;
    int size = dim1_ * dim2_;
    for (int i = 0; i < size; ++i) {
      if (!array_[i].DeSerialize(swap, fp)) return false;
    }
    return true;
  }

  // Provide the dimensions of this rectangular matrix.
  int dim1() const { return dim1_; }
  int dim2() const { return dim2_; }

  // Expression to select a specific location in the matrix. The matrix is
  // stored COLUMN-major, so the left-most index is the most significant.
  // This allows [][] access to use indices in the same order as (,).
  int index(int column, int row) const {
    return (column * dim2_ + row);
  }

  // Put a list element into the matrix at a specific location.
  void put(int column, int row, const T& thing) {
    array_[this->index(column, row)] = thing;
  }

  // Get the item at a specified location from the matrix.
  T get(int column, int row) const {
    return array_[this->index(column, row)];
  }
  // Return a reference to the element at the specified location.
  const T& operator()(int column, int row) const {
    return array_[this->index(column, row)];
  }
  T& operator()(int column, int row) {
    return array_[this->index(column, row)];
  }
  // Allow access using array[column][row]. NOTE that the indices are
  // in the same left-to-right order as the () indexing.
  T* operator[](int column) {
    return &array_[this->index(column, 0)];
  }

  // Delete objects pointed to by array_[i].
  void delete_matrix_pointers() {
    for (int x = 0; x < dim1_; x++) {
      for (int y = 0; y < dim2_; y++) {
        T matrix_cell = this->get(x, y);
        if (matrix_cell != empty_)
          delete matrix_cell;
      }
    }
  }

 private:
  // Factored helper to serialize the size.
  bool SerializeSize(FILE* fp) const {
    inT32 size = dim1_;
    if (fwrite(&size, sizeof(size), 1, fp) != 1) return false;
    size = dim2_;
    if (fwrite(&size, sizeof(size), 1, fp) != 1) return false;
    return true;
  }
  // Factored helper to deserialize the size.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerializeSize(bool swap, FILE* fp) {
    inT32 size1, size2;
    if (fread(&size1, sizeof(size1), 1, fp) != 1) return false;
    if (fread(&size2, sizeof(size2), 1, fp) != 1) return false;
    if (swap) {
      ReverseN(&size1, sizeof(size1));
      ReverseN(&size2, sizeof(size2));
    }
    if (size1 != dim1_ || size2 != dim2_) {
      dim1_ = size1;
      dim2_ = size2;
      delete [] array_;
      array_ = new T[dim1_ * dim2_];
    }
    return true;
  }

  T* array_;
  T empty_;   // The unused cell.
  int dim1_;  // Size of the 1st dimension in indexing functions.
  int dim2_;  // Size of the 2nd dimension in indexing functions.
};

// A generic class to store a square matrix with entries of type T.
template <class T>
class GENERIC_MATRIX : public GENERIC_2D_ARRAY<T> {
 public:
  // Allocate a piece of memory to hold a matrix of the given dimension.
  // Initialize all the elements of the matrix to empty instead of assuming
  // that a default constructor can be used.
  GENERIC_MATRIX(int dimension, const T& empty)
    : GENERIC_2D_ARRAY<T>(dimension, dimension, empty) {
  }

  // Provide the dimension of this square matrix.
  int dimension() const { return this->dim1(); }
};

class MATRIX : public GENERIC_MATRIX<BLOB_CHOICE_LIST *> {
 public:
  MATRIX(int dimension) : GENERIC_MATRIX<BLOB_CHOICE_LIST *>(dimension,
                                                             NOT_CLASSIFIED) {}
  // Print a shortened version of the contents of the matrix.
  void print(const UNICHARSET &unicharset) const;
};

struct MATRIX_COORD {
  static void Delete(void *arg) {
    MATRIX_COORD *c = static_cast<MATRIX_COORD *>(arg);
    delete c;
  }
  MATRIX_COORD(int c, int r): col(c), row(r) {}
  ~MATRIX_COORD() {}
  bool Valid(const MATRIX &m) const {
    return (col >= 0 && row >= 0 &&
            col < m.dimension() && row < m.dimension());
  }
  int col;
  int row;
};

#endif  // TESSERACT_CCSTRUCT_MATRIX_H__
