/* -*-C-*-
 ******************************************************************************
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

#include "kdpair.h"
#include "unicharset.h"

class BLOB_CHOICE_LIST;

#define NOT_CLASSIFIED reinterpret_cast<BLOB_CHOICE_LIST*>(NULL)

// A generic class to hold a 2-D matrix with entries of type T, but can also
// act as a base class for other implementations, such as a triangular or
// banded matrix.
template <class T>
class GENERIC_2D_ARRAY {
 public:
  // Initializes the array size, and empty element, but cannot allocate memory
  // for the subclasses or initialize because calls to the num_elements
  // member will be routed to the base class implementation. Subclasses can
  // either pass the memory in, or allocate after by calling Resize().
  GENERIC_2D_ARRAY(int dim1, int dim2, const T& empty, T* array)
    : empty_(empty), dim1_(dim1), dim2_(dim2), array_(array)  {
  }
  // Original constructor for a full rectangular matrix DOES allocate memory
  // and initialize it to empty.
  GENERIC_2D_ARRAY(int dim1, int dim2, const T& empty)
    : empty_(empty), dim1_(dim1), dim2_(dim2)  {
    array_ = new T[dim1_ * dim2_];
    for (int x = 0; x < dim1_; x++)
      for (int y = 0; y < dim2_; y++)
        this->put(x, y, empty_);
  }
  virtual ~GENERIC_2D_ARRAY() { delete[] array_; }

  // Reallocate the array to the given size. Does not keep old data.
  void Resize(int size1, int size2, const T& empty) {
    empty_ = empty;
    if (size1 != dim1_ || size2 != dim2_) {
      dim1_ = size1;
      dim2_ = size2;
      delete [] array_;
      array_ = new T[dim1_ * dim2_];
    }
    Clear();
  }

  // Reallocate the array to the given size, keeping old data.
  void ResizeWithCopy(int size1, int size2) {
    if (size1 != dim1_ || size2 != dim2_) {
      T* new_array = new T[size1 * size2];
      for (int col = 0; col < size1; ++col) {
        for (int row = 0; row < size2; ++row) {
          int old_index = col * dim2() + row;
          int new_index = col * size2 + row;
          if (col < dim1_ && row < dim2_) {
            new_array[new_index] = array_[old_index];
          } else {
            new_array[new_index] = empty_;
          }
        }
      }
      delete[] array_;
      array_ = new_array;
      dim1_ = size1;
      dim2_ = size2;
    }
  }

  // Sets all the elements of the array to the empty value.
  void Clear() {
    int total_size = num_elements();
    for (int i = 0; i < total_size; ++i)
      array_[i] = empty_;
  }

  // Writes to the given file. Returns false in case of error.
  // Only works with bitwise-serializeable types!
  bool Serialize(FILE* fp) const {
    if (!SerializeSize(fp)) return false;
    if (fwrite(&empty_, sizeof(empty_), 1, fp) != 1) return false;
    int size = num_elements();
    if (fwrite(array_, sizeof(*array_), size, fp) != size) return false;
    return true;
  }

  // Reads from the given file. Returns false in case of error.
  // Only works with bitwise-serializeable typ
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp) {
    if (!DeSerializeSize(swap, fp)) return false;
    if (fread(&empty_, sizeof(empty_), 1, fp) != 1) return false;
    if (swap) ReverseN(&empty_, sizeof(empty_));
    int size = num_elements();
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
    int size = num_elements();
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
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      if (!array_[i].DeSerialize(swap, fp)) return false;
    }
    return true;
  }

  // Provide the dimensions of this rectangular matrix.
  int dim1() const { return dim1_; }
  int dim2() const { return dim2_; }
  // Returns the number of elements in the array.
  // Banded/triangular matrices may override.
  virtual int num_elements() const { return dim1_ * dim2_; }

  // Expression to select a specific location in the matrix. The matrix is
  // stored COLUMN-major, so the left-most index is the most significant.
  // This allows [][] access to use indices in the same order as (,).
  virtual int index(int column, int row) const {
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
  const T* operator[](int column) const {
    return &array_[this->index(column, 0)];
  }

  // Delete objects pointed to by array_[i].
  void delete_matrix_pointers() {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      T matrix_cell = array_[i];
      if (matrix_cell != empty_)
        delete matrix_cell;
    }
  }

 protected:
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
    Resize(size1, size2, empty_);
    return true;
  }

  T* array_;
  T empty_;   // The unused cell.
  int dim1_;  // Size of the 1st dimension in indexing functions.
  int dim2_;  // Size of the 2nd dimension in indexing functions.
};

// A generic class to store a banded triangular matrix with entries of type T.
// In this array, the nominally square matrix is dim1_ x dim1_, and dim2_ is
// the number of bands, INCLUDING the diagonal. The storage is thus of size
// dim1_ * dim2_ and index(col, row) = col * dim2_ + row - col, and an
// assert will fail if row < col or row - col >= dim2.
template <class T>
class BandTriMatrix : public GENERIC_2D_ARRAY<T> {
 public:
  // Allocate a piece of memory to hold a 2d-array of the given dimension.
  // Initialize all the elements of the array to empty instead of assuming
  // that a default constructor can be used.
  BandTriMatrix(int dim1, int dim2, const T& empty)
    : GENERIC_2D_ARRAY<T>(dim1, dim2, empty)  {
  }
  // The default destructor will do.

  // Provide the dimensions of this matrix.
  // dimension is the size of the nominally square matrix.
  int dimension() const { return this->dim1_; }
  // bandwidth is the number of bands in the matrix, INCLUDING the diagonal.
  int bandwidth() const { return this->dim2_; }

  // Expression to select a specific location in the matrix. The matrix is
  // stored COLUMN-major, so the left-most index is the most significant.
  // This allows [][] access to use indices in the same order as (,).
  virtual int index(int column, int row) const {
    ASSERT_HOST(row >= column);
    ASSERT_HOST(row - column < this->dim2_);
    return column * this->dim2_ + row - column;
  }

  // Appends array2 corner-to-corner to *this, making an array of dimension
  // equal to the sum of the individual dimensions.
  // array2 is not destroyed, but is left empty, as all elements are moved
  // to *this.
  void AttachOnCorner(BandTriMatrix<T>* array2) {
    int new_dim1 = this->dim1_ + array2->dim1_;
    int new_dim2 = MAX(this->dim2_, array2->dim2_);
    T* new_array = new T[new_dim1 * new_dim2];
    for (int col = 0; col < new_dim1; ++col) {
      for (int j = 0; j < new_dim2; ++j) {
        int new_index = col * new_dim2 + j;
        if (col < this->dim1_ && j < this->dim2_) {
          new_array[new_index] = this->get(col, col + j);
        } else if (col >= this->dim1_ && j < array2->dim2_) {
          new_array[new_index] = array2->get(col - this->dim1_,
                                             col - this->dim1_ + j);
          array2->put(col - this->dim1_, col - this->dim1_ + j, NULL);
        } else {
          new_array[new_index] = this->empty_;
        }
      }
    }
    delete[] this->array_;
    this->array_ = new_array;
    this->dim1_ = new_dim1;
    this->dim2_ = new_dim2;
  }
};

class MATRIX : public BandTriMatrix<BLOB_CHOICE_LIST *> {
 public:
  MATRIX(int dimension, int bandwidth)
    : BandTriMatrix<BLOB_CHOICE_LIST *>(dimension, bandwidth, NOT_CLASSIFIED) {}

  // Returns true if there are any real classification results.
  bool Classified(int col, int row, int wildcard_id) const;

  // Expands the existing matrix in-place to make the band wider, without
  // losing any existing data.
  void IncreaseBandSize(int bandwidth);

  // Returns a bigger MATRIX with a new column and row in the matrix in order
  // to split the blob at the given (ind,ind) diagonal location.
  // Entries are relocated to the new MATRIX using the transformation defined
  // by MATRIX_COORD::MapForSplit.
  // Transfers the pointer data to the new MATRIX and deletes *this.
  MATRIX* ConsumeAndMakeBigger(int ind);

  // Makes and returns a deep copy of *this, including all the BLOB_CHOICEs
  // on the lists, but not any LanguageModelState that may be attached to the
  // BLOB_CHOICEs.
  MATRIX* DeepCopy() const;

  // Print a shortened version of the contents of the matrix.
  void print(const UNICHARSET &unicharset) const;
};

struct MATRIX_COORD {
  static void Delete(void *arg) {
    MATRIX_COORD *c = static_cast<MATRIX_COORD *>(arg);
    delete c;
  }
  // Default constructor required by GenericHeap.
  MATRIX_COORD() : col(0), row(0) {}
  MATRIX_COORD(int c, int r): col(c), row(r) {}
  ~MATRIX_COORD() {}

  bool Valid(const MATRIX &m) const {
    return 0 <= col && col < m.dimension() &&
           col <= row && row < col + m.bandwidth() && row < m.dimension();
  }

  // Remaps the col,row pair to split the blob at the given (ind,ind) diagonal
  // location.
  // Entries at (i,j) for i in [0,ind] and j in [ind,dim) move to (i,j+1),
  // making a new row at ind.
  // Entries at (i,j) for i in [ind+1,dim) and j in [i,dim) move to (i+i,j+1),
  // making a new column at ind+1.
  void MapForSplit(int ind) {
    ASSERT_HOST(row >= col);
    if (col > ind) ++col;
    if (row >= ind) ++row;
    ASSERT_HOST(row >= col);
  }

  int col;
  int row;
};

// The MatrixCoordPair contains a MATRIX_COORD and its priority.
typedef tesseract::KDPairInc<float, MATRIX_COORD> MatrixCoordPair;

#endif  // TESSERACT_CCSTRUCT_MATRIX_H__
