/******************************************************************************
 * File:         matrix.h
 * Description:  Generic 2-d array/matrix and banded triangular matrix class.
 * Author:       Ray Smith
 * TODO(rays) Separate from ratings matrix, which it also contains:
 *
 * Description:  Ratings matrix class (specialization of banded matrix).
 *               Segmentation search matrix of lists of BLOB_CHOICE.
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

#ifndef TESSERACT_CCSTRUCT_MATRIX_H_
#define TESSERACT_CCSTRUCT_MATRIX_H_

#include "errcode.h" // for ASSERT_HOST
#include "helpers.h" // for ReverseN, ClipToRange
#include "kdpair.h"  // for KDPairInc
#include "points.h"  // for ICOORD

#include "serialis.h" // for TFile

#include <algorithm> // for max, min
#include <cmath>     // for sqrt, fabs, isfinite
#include <cstdint>   // for int32_t
#include <cstdio>    // for FILE
#include <cstring>   // for memcpy

namespace tesseract {

class BLOB_CHOICE_LIST;
class UNICHARSET;

#define NOT_CLASSIFIED static_cast<BLOB_CHOICE_LIST *>(nullptr)

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
  GENERIC_2D_ARRAY(int dim1, int dim2, const T &empty, T *array)
      : empty_(empty), dim1_(dim1), dim2_(dim2), array_(array) {
    size_allocated_ = dim1 * dim2;
  }
  // Original constructor for a full rectangular matrix DOES allocate memory
  // and initialize it to empty.
  GENERIC_2D_ARRAY(int dim1, int dim2, const T &empty) : empty_(empty), dim1_(dim1), dim2_(dim2) {
    int new_size = dim1 * dim2;
    array_ = new T[new_size];
    size_allocated_ = new_size;
    for (int i = 0; i < size_allocated_; ++i) {
      array_[i] = empty_;
    }
  }
  // Default constructor for array allocation. Use Resize to set the size.
  GENERIC_2D_ARRAY()
      : array_(nullptr), empty_(static_cast<T>(0)), dim1_(0), dim2_(0), size_allocated_(0) {}
  GENERIC_2D_ARRAY(const GENERIC_2D_ARRAY<T> &src)
      : array_(nullptr), empty_(static_cast<T>(0)), dim1_(0), dim2_(0), size_allocated_(0) {
    *this = src;
  }
  virtual ~GENERIC_2D_ARRAY() {
    delete[] array_;
  }

  void operator=(const GENERIC_2D_ARRAY<T> &src) {
    ResizeNoInit(src.dim1(), src.dim2());
    int size = num_elements();
    if (size > 0) {
      memcpy(array_, src.array_, size * sizeof(array_[0]));
    }
  }

  // Reallocates the array to the given size. Does not keep old data, but does
  // not initialize the array either.
  // The allocated memory is expanded on the end by pad, allowing deliberate
  // access beyond the bounds of the array.
  void ResizeNoInit(int size1, int size2, int pad = 0) {
    int new_size = size1 * size2 + pad;
    if (new_size > size_allocated_) {
      delete[] array_;
      array_ = new T[new_size];
      size_allocated_ = new_size;
    }
    dim1_ = size1;
    dim2_ = size2;
    // Fill the padding data so it isn't uninitialized.
    for (int i = size1 * size2; i < new_size; ++i) {
      array_[i] = empty_;
    }
  }

  // Reallocate the array to the given size. Does not keep old data.
  void Resize(int size1, int size2, const T &empty) {
    empty_ = empty;
    ResizeNoInit(size1, size2);
    Clear();
  }

  // Reallocate the array to the given size, keeping old data.
  void ResizeWithCopy(int size1, int size2) {
    if (size1 != dim1_ || size2 != dim2_) {
      int new_size = size1 * size2;
      T *new_array = new T[new_size];
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
      size_allocated_ = new_size;
    }
  }

  // Sets all the elements of the array to the empty value.
  void Clear() {
    int total_size = num_elements();
    for (int i = 0; i < total_size; ++i) {
      array_[i] = empty_;
    }
  }

  // Writes to the given file. Returns false in case of error.
  // Only works with bitwise-serializeable types!
  bool Serialize(FILE *fp) const {
    if (!SerializeSize(fp)) {
      return false;
    }
    if (!tesseract::Serialize(fp, &empty_)) {
      return false;
    }
    int size = num_elements();
    return tesseract::Serialize(fp, &array_[0], size);
  }

  bool Serialize(TFile *fp) const {
    if (!SerializeSize(fp)) {
      return false;
    }
    if (!fp->Serialize(&empty_)) {
      return false;
    }
    int size = num_elements();
    return fp->Serialize(&array_[0], size);
  }

  // Reads from the given file. Returns false in case of error.
  // Only works with bitwise-serializeable types!
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE *fp) {
    if (!DeSerializeSize(swap, fp)) {
      return false;
    }
    if (!tesseract::DeSerialize(fp, &empty_)) {
      return false;
    }
    if (swap) {
      ReverseN(&empty_, sizeof(empty_));
    }
    int size = num_elements();
    if (!tesseract::DeSerialize(fp, &array_[0], size)) {
      return false;
    }
    if (swap) {
      for (int i = 0; i < size; ++i) {
        ReverseN(&array_[i], sizeof(array_[i]));
      }
    }
    return true;
  }

  bool DeSerialize(TFile *fp) {
    return DeSerializeSize(fp) && fp->DeSerialize(&empty_) &&
           fp->DeSerialize(&array_[0], num_elements());
  }

  // Writes to the given file. Returns false in case of error.
  // Assumes a T::Serialize(FILE*) const function.
  bool SerializeClasses(FILE *fp) const {
    if (!SerializeSize(fp)) {
      return false;
    }
    if (!empty_.Serialize(fp)) {
      return false;
    }
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      if (!array_[i].Serialize(fp)) {
        return false;
      }
    }
    return true;
  }

  // Reads from the given file. Returns false in case of error.
  // Assumes a T::DeSerialize(bool swap, FILE*) function.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerializeClasses(bool swap, FILE *fp) {
    if (!DeSerializeSize(swap, fp)) {
      return false;
    }
    if (!empty_.DeSerialize(swap, fp)) {
      return false;
    }
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      if (!array_[i].DeSerialize(swap, fp)) {
        return false;
      }
    }
    return true;
  }

  // Provide the dimensions of this rectangular matrix.
  int dim1() const {
    return dim1_;
  }
  int dim2() const {
    return dim2_;
  }
  // Returns the number of elements in the array.
  // Banded/triangular matrices may override.
  virtual int num_elements() const {
    return dim1_ * dim2_;
  }

  // Expression to select a specific location in the matrix. The matrix is
  // stored COLUMN-major, so the left-most index is the most significant.
  // This allows [][] access to use indices in the same order as (,).
  virtual int index(int column, int row) const {
    return (column * dim2_ + row);
  }

  // Put a list element into the matrix at a specific location.
  void put(ICOORD pos, const T &thing) {
    array_[this->index(pos.x(), pos.y())] = thing;
  }
  void put(int column, int row, const T &thing) {
    array_[this->index(column, row)] = thing;
  }

  // Get the item at a specified location from the matrix.
  T get(ICOORD pos) const {
    return array_[this->index(pos.x(), pos.y())];
  }
  T get(int column, int row) const {
    return array_[this->index(column, row)];
  }
  // Return a reference to the element at the specified location.
  const T &operator()(int column, int row) const {
    return array_[this->index(column, row)];
  }
  T &operator()(int column, int row) {
    return array_[this->index(column, row)];
  }
  // Allow access using array[column][row]. NOTE that the indices are
  // in the same left-to-right order as the () indexing.
  T *operator[](int column) {
    return &array_[this->index(column, 0)];
  }
  const T *operator[](int column) const {
    return &array_[this->index(column, 0)];
  }

  // Adds addend to *this, element-by-element.
  void operator+=(const GENERIC_2D_ARRAY<T> &addend) {
    if (dim2_ == addend.dim2_) {
      // Faster if equal size in the major dimension.
      int size = std::min(num_elements(), addend.num_elements());
      for (int i = 0; i < size; ++i) {
        array_[i] += addend.array_[i];
      }
    } else {
      for (int x = 0; x < dim1_; x++) {
        for (int y = 0; y < dim2_; y++) {
          (*this)(x, y) += addend(x, y);
        }
      }
    }
  }
  // Subtracts minuend from *this, element-by-element.
  void operator-=(const GENERIC_2D_ARRAY<T> &minuend) {
    if (dim2_ == minuend.dim2_) {
      // Faster if equal size in the major dimension.
      int size = std::min(num_elements(), minuend.num_elements());
      for (int i = 0; i < size; ++i) {
        array_[i] -= minuend.array_[i];
      }
    } else {
      for (int x = 0; x < dim1_; x++) {
        for (int y = 0; y < dim2_; y++) {
          (*this)(x, y) -= minuend(x, y);
        }
      }
    }
  }
  // Adds addend to all elements.
  void operator+=(const T &addend) {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      array_[i] += addend;
    }
  }
  // Multiplies *this by factor, element-by-element.
  void operator*=(const T &factor) {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      array_[i] *= factor;
    }
  }
  // Clips *this to the given range.
  void Clip(const T &rangemin, const T &rangemax) {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      array_[i] = ClipToRange(array_[i], rangemin, rangemax);
    }
  }
  // Returns true if all elements of *this are within the given range.
  // Only uses operator<
  bool WithinBounds(const T &rangemin, const T &rangemax) const {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      const T &value = array_[i];
      if (value < rangemin || rangemax < value) {
        return false;
      }
    }
    return true;
  }
  // Normalize the whole array.
  double Normalize() {
    int size = num_elements();
    if (size <= 0) {
      return 0.0;
    }
    // Compute the mean.
    double mean = 0.0;
    for (int i = 0; i < size; ++i) {
      mean += array_[i];
    }
    mean /= size;
    // Subtract the mean and compute the standard deviation.
    double sd = 0.0;
    for (int i = 0; i < size; ++i) {
      double normed = array_[i] - mean;
      array_[i] = normed;
      sd += normed * normed;
    }
    sd = sqrt(sd / size);
    if (sd > 0.0) {
      // Divide by the sd.
      for (int i = 0; i < size; ++i) {
        array_[i] /= sd;
      }
    }
    return sd;
  }

  // Returns the maximum value of the array.
  T Max() const {
    int size = num_elements();
    if (size <= 0) {
      return empty_;
    }
    // Compute the max.
    T max_value = array_[0];
    for (int i = 1; i < size; ++i) {
      const T &value = array_[i];
      if (value > max_value) {
        max_value = value;
      }
    }
    return max_value;
  }

  // Returns the maximum absolute value of the array.
  T MaxAbs() const {
    int size = num_elements();
    if (size <= 0) {
      return empty_;
    }
    // Compute the max.
    T max_abs = static_cast<T>(0);
    for (int i = 0; i < size; ++i) {
      T value = static_cast<T>(fabs(array_[i]));
      if (value > max_abs) {
        max_abs = value;
      }
    }
    return max_abs;
  }

  // Accumulates the element-wise sums of squares of src into *this.
  void SumSquares(const GENERIC_2D_ARRAY<T> &src, const T &decay_factor) {
    T update_factor = 1 - decay_factor;
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      array_[i] = array_[i] * decay_factor + update_factor * src.array_[i] * src.array_[i];
    }
  }

  // Scales each element using the adam algorithm, ie array_[i] by
  // sqrt(sqsum[i] + epsilon)).
  void AdamUpdate(const GENERIC_2D_ARRAY<T> &sum, const GENERIC_2D_ARRAY<T> &sqsum,
                  const T &epsilon) {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      array_[i] += sum.array_[i] / (sqrt(sqsum.array_[i]) + epsilon);
    }
  }

  void AssertFinite() const {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      ASSERT_HOST(isfinite(array_[i]));
    }
  }

  // REGARDLESS OF THE CURRENT DIMENSIONS, treats the data as a
  // num_dims-dimensional array/tensor with dimensions given by dims, (ordered
  // from most significant to least significant, the same as standard C arrays)
  // and moves src_dim to dest_dim, with the initial dest_dim and any dimensions
  // in between shifted towards the hole left by src_dim. Example:
  // Current data content: array_=[0, 1, 2, ....119]
  //   perhaps *this may be of dim[40, 3], with values [[0, 1, 2][3, 4, 5]...
  //   but the current dimensions are irrelevant.
  // num_dims = 4, dims=[5, 4, 3, 2]
  // src_dim=3, dest_dim=1
  // tensor=[[[[0, 1][2, 3][4, 5]]
  //          [[6, 7][8, 9][10, 11]]
  //          [[12, 13][14, 15][16, 17]]
  //          [[18, 19][20, 21][22, 23]]]
  //         [[[24, 25]...
  // output dims =[5, 2, 4, 3]
  // output tensor=[[[[0, 2, 4][6, 8, 10][12, 14, 16][18, 20, 22]]
  //                 [[1, 3, 5][7, 9, 11][13, 15, 17][19, 21, 23]]]
  //                [[[24, 26, 28]...
  // which is stored in the array_ as:
  //   [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 1, 3, 5, 7, 9, 11, 13...]
  // NOTE: the 2 stored matrix dimensions are simply copied from *this. To
  // change the dimensions after the transpose, use ResizeNoInit.
  // Higher dimensions above 2 are strictly the responsibility of the caller.
  void RotatingTranspose(const int *dims, int num_dims, int src_dim, int dest_dim,
                         GENERIC_2D_ARRAY<T> *result) const {
    int max_d = std::max(src_dim, dest_dim);
    int min_d = std::min(src_dim, dest_dim);
    // In a tensor of shape [d0, d1... min_d, ... max_d, ... dn-2, dn-1], the
    // ends outside of min_d and max_d are unaffected, with [max_d +1, dn-1]
    // being contiguous blocks of data that will move together, and
    // [d0, min_d -1] being replicas of the transpose operation.
    // num_replicas represents the large dimensions unchanged by the operation.
    // move_size represents the small dimensions unchanged by the operation.
    // src_step represents the stride in the src between each adjacent group
    // in the destination.
    int num_replicas = 1, move_size = 1, src_step = 1;
    for (int d = 0; d < min_d; ++d) {
      num_replicas *= dims[d];
    }
    for (int d = max_d + 1; d < num_dims; ++d) {
      move_size *= dims[d];
    }
    for (int d = src_dim + 1; d < num_dims; ++d) {
      src_step *= dims[d];
    }
    if (src_dim > dest_dim) {
      src_step *= dims[src_dim];
    }
    // wrap_size is the size of a single replica, being the amount that is
    // handled num_replicas times.
    int wrap_size = move_size;
    for (int d = min_d; d <= max_d; ++d) {
      wrap_size *= dims[d];
    }
    result->ResizeNoInit(dim1_, dim2_);
    result->empty_ = empty_;
    const T *src = array_;
    T *dest = result->array_;
    for (int replica = 0; replica < num_replicas; ++replica) {
      for (int start = 0; start < src_step; start += move_size) {
        for (int pos = start; pos < wrap_size; pos += src_step) {
          memcpy(dest, src + pos, sizeof(*dest) * move_size);
          dest += move_size;
        }
      }
      src += wrap_size;
    }
  }

  // Delete objects pointed to by array_[i].
  void delete_matrix_pointers() {
    int size = num_elements();
    for (int i = 0; i < size; ++i) {
      T matrix_cell = array_[i];
      if (matrix_cell != empty_) {
        delete matrix_cell;
      }
    }
  }

protected:
  // Factored helper to serialize the size.
  bool SerializeSize(FILE *fp) const {
    uint32_t size = dim1_;
    if (!tesseract::Serialize(fp, &size)) {
      return false;
    }
    size = dim2_;
    return tesseract::Serialize(fp, &size);
  }
  bool SerializeSize(TFile *fp) const {
    uint32_t size = dim1_;
    if (!fp->Serialize(&size)) {
      return false;
    }
    size = dim2_;
    return fp->Serialize(&size);
  }
  // Factored helper to deserialize the size.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerializeSize(bool swap, FILE *fp) {
    uint32_t size1, size2;
    if (!tesseract::DeSerialize(fp, &size1)) {
      return false;
    }
    if (!tesseract::DeSerialize(fp, &size2)) {
      return false;
    }
    if (swap) {
      ReverseN(&size1, sizeof(size1));
      ReverseN(&size2, sizeof(size2));
    }
    // Arbitrarily limit the number of elements to protect against bad data.
    if (size1 > UINT16_MAX) {
      return false;
    }
    if (size2 > UINT16_MAX) {
      return false;
    }
    Resize(size1, size2, empty_);
    return true;
  }
  bool DeSerializeSize(TFile *fp) {
    int32_t size1, size2;
    if (!fp->DeSerialize(&size1)) {
      return false;
    }
    if (!fp->DeSerialize(&size2)) {
      return false;
    }
    // Arbitrarily limit the number of elements to protect against bad data.
    if (size1 > UINT16_MAX) {
      return false;
    }
    if (size2 > UINT16_MAX) {
      return false;
    }
    Resize(size1, size2, empty_);
    return true;
  }

  T *array_;
  T empty_;  // The unused cell.
  int dim1_; // Size of the 1st dimension in indexing functions.
  int dim2_; // Size of the 2nd dimension in indexing functions.
  // The total size to which the array can be expanded before a realloc is
  // needed. If Resize is used, memory is retained so it can be re-expanded
  // without a further alloc, and this stores the allocated size.
  int size_allocated_;
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
  BandTriMatrix(int dim1, int dim2, const T &empty) : GENERIC_2D_ARRAY<T>(dim1, dim2, empty) {}
  // The default destructor will do.

  // Provide the dimensions of this matrix.
  // dimension is the size of the nominally square matrix.
  int dimension() const {
    return this->dim1_;
  }
  // bandwidth is the number of bands in the matrix, INCLUDING the diagonal.
  int bandwidth() const {
    return this->dim2_;
  }

  // Expression to select a specific location in the matrix. The matrix is
  // stored COLUMN-major, so the left-most index is the most significant.
  // This allows [][] access to use indices in the same order as (,).
  int index(int column, int row) const override {
    ASSERT_HOST(row >= column);
    ASSERT_HOST(row - column < this->dim2_);
    return column * this->dim2_ + row - column;
  }

  // Appends array2 corner-to-corner to *this, making an array of dimension
  // equal to the sum of the individual dimensions.
  // array2 is not destroyed, but is left empty, as all elements are moved
  // to *this.
  void AttachOnCorner(BandTriMatrix<T> *array2) {
    int new_dim1 = this->dim1_ + array2->dim1_;
    int new_dim2 = std::max(this->dim2_, array2->dim2_);
    T *new_array = new T[new_dim1 * new_dim2];
    for (int col = 0; col < new_dim1; ++col) {
      for (int j = 0; j < new_dim2; ++j) {
        int new_index = col * new_dim2 + j;
        if (col < this->dim1_ && j < this->dim2_) {
          new_array[new_index] = this->get(col, col + j);
        } else if (col >= this->dim1_ && j < array2->dim2_) {
          new_array[new_index] = array2->get(col - this->dim1_, col - this->dim1_ + j);
          array2->put(col - this->dim1_, col - this->dim1_ + j, nullptr);
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

  ~MATRIX() override;

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
  MATRIX *ConsumeAndMakeBigger(int ind);

  // Makes and returns a deep copy of *this, including all the BLOB_CHOICEs
  // on the lists, but not any LanguageModelState that may be attached to the
  // BLOB_CHOICEs.
  MATRIX *DeepCopy() const;

  // Print a shortened version of the contents of the matrix.
  void print(const UNICHARSET &unicharset) const;
};

struct MATRIX_COORD {
  static void Delete(void *arg) {
    auto *c = static_cast<MATRIX_COORD *>(arg);
    delete c;
  }
  // Default constructor required by GenericHeap.
  MATRIX_COORD() : col(0), row(0) {}
  MATRIX_COORD(int c, int r) : col(c), row(r) {}
  ~MATRIX_COORD() = default;

  bool Valid(const MATRIX &m) const {
    return 0 <= col && col < m.dimension() && col <= row && row < col + m.bandwidth() &&
           row < m.dimension();
  }

  // Remaps the col,row pair to split the blob at the given (ind,ind) diagonal
  // location.
  // Entries at (i,j) for i in [0,ind] and j in [ind,dim) move to (i,j+1),
  // making a new row at ind.
  // Entries at (i,j) for i in [ind+1,dim) and j in [i,dim) move to (i+i,j+1),
  // making a new column at ind+1.
  void MapForSplit(int ind) {
    ASSERT_HOST(row >= col);
    if (col > ind) {
      ++col;
    }
    if (row >= ind) {
      ++row;
    }
    ASSERT_HOST(row >= col);
  }

  int col;
  int row;
};

// The MatrixCoordPair contains a MATRIX_COORD and its priority.
using MatrixCoordPair = KDPairInc<float, MATRIX_COORD>;

} // namespace tesseract

#endif // TESSERACT_CCSTRUCT_MATRIX_H_
