///////////////////////////////////////////////////////////////////////
// File:        stridemap.h
// Description: Indexing into a 4-d tensor held in a 2-d Array.
// Author:      Ray Smith
//
// (C) Copyright 2016, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////
#ifndef TESSERACT_LSTM_STRIDEMAP_H_
#define TESSERACT_LSTM_STRIDEMAP_H_

#include <cstring>
#include <vector>

namespace tesseract {

// Enum describing the dimensions of the 'Tensor' in a NetworkIO.
// A NetworkIO is analogous to a TF Tensor, except that the number of dimensions
// is fixed (4), and they always have the same meaning. The underlying
// representation is a 2-D array, for which the product batch*height*width
// is always dim1 and depth is always dim2. FlexDimensions is used only for
// batch, height, width with the StrideMap, and therefore represents the runtime
// shape. The build-time shape is defined by StaticShape.
enum FlexDimensions {
  FD_BATCH,    // Index of multiple images.
  FD_HEIGHT,   // y-coordinate in image.
  FD_WIDTH,    // x-coordinate in image.
  FD_DIMSIZE,  // Number of flexible non-depth dimensions.
};

// Encapsulation of information relating to the mapping from [batch][y][x] to
// the first index into the 2-d array underlying a NetworkIO.
class StrideMap {
 public:
  // Class holding the non-depth indices.
  class Index {
   public:
    explicit Index(const StrideMap& stride_map) : stride_map_(&stride_map) {
      InitToFirst();
    }
    Index(const StrideMap& stride_map, int batch, int y, int x)
        : stride_map_(&stride_map) {
      indices_[FD_BATCH] = batch;
      indices_[FD_HEIGHT] = y;
      indices_[FD_WIDTH] = x;
      SetTFromIndices();
    }
    // Accesses the index to the underlying array.
    int t() const { return t_; }
    int index(FlexDimensions dimension) const { return indices_[dimension]; }
    // Initializes the indices to the first valid location.
    void InitToFirst() {
      memset(indices_, 0, sizeof(indices_));
      t_ = 0;
    }
    // Initializes the indices to the last valid location.
    void InitToLast() { InitToLastOfBatch(MaxIndexOfDim(FD_BATCH)); }
    // Returns true if *this is a valid index.
    bool IsValid() const;
    // Returns true if the index of the given dimension is the last.
    bool IsLast(FlexDimensions dimension) const;
    // Given that the dimensions up to and including dim-1 are valid, returns
    // the maximum index for dimension dim.
    int MaxIndexOfDim(FlexDimensions dim) const;
    // Adds the given offset to the given dimension. Returns true if the result
    // makes a valid index.
    bool AddOffset(int offset, FlexDimensions dimension);
    // Increments the index in some encapsulated way that guarantees to remain
    // valid until it returns false, meaning that the iteration is complete.
    bool Increment();
    // Decrements the index in some encapsulated way that guarantees to remain
    // valid until it returns false, meaning that the iteration (that started
    // with InitToLast()) is complete.
    bool Decrement();

   private:
    // Initializes the indices to the last valid location in the given batch
    // index.
    void InitToLastOfBatch(int batch);
    // Computes and sets t_ from the current indices_.
    void SetTFromIndices();

    // Map into which *this is an index.
    const StrideMap* stride_map_;
    // Index to the first dimension of the underlying array.
    int t_;
    // Indices into the individual dimensions.
    int indices_[FD_DIMSIZE];
  };

  StrideMap() {
    memset(shape_, 0, sizeof(shape_));
    memset(t_increments_, 0, sizeof(t_increments_));
  }
  // Default copy constructor and operator= are OK to use here!

  // Sets up the stride for the given array of height, width pairs.
  void SetStride(const std::vector<std::pair<int, int>>& h_w_pairs);
  // Scales width and height dimensions by the given factors.
  void ScaleXY(int x_factor, int y_factor);
  // Reduces width to 1, across the batch, whatever the input size.
  void ReduceWidthTo1();
  // Transposes the width and height dimensions.
  void TransposeXY();
  // Returns the size of the given dimension.
  int Size(FlexDimensions dimension) const { return shape_[dimension]; }
  // Returns the total width required.
  int Width() const { return t_increments_[FD_BATCH] * shape_[FD_BATCH]; }

 private:
  // Computes t_increments_ from shape_.
  void ComputeTIncrements();

  // The size of each non-depth dimension.
  int shape_[FD_DIMSIZE];
  // Precomputed 't' increments for each dimension. This is the value of
  // the given dimension in the packed 3-d array that the shape_ represents.
  int t_increments_[FD_DIMSIZE];
  // Vector of size shape_[FD_BATCH] holds the height of each image in a batch.
  std::vector<int> heights_;
  // Vector of size shape_[FD_BATCH] holds the width of each image in a batch.
  std::vector<int> widths_;
};

}  // namespace tesseract

#endif  // TESSERACT_LSTM_STRIDEMAP_H_
