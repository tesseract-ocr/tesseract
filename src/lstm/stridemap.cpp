///////////////////////////////////////////////////////////////////////
// File:        stridemap.cpp
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

#include "stridemap.h"
#include <cassert> // for assert

namespace tesseract {

// Returns true if *this is a valid index.
bool StrideMap::Index::IsValid() const {
  // Cheap check first.
  for (int index : indices_) {
    if (index < 0) {
      return false;
    }
  }
  for (int d = 0; d < FD_DIMSIZE; ++d) {
    if (indices_[d] > MaxIndexOfDim(static_cast<FlexDimensions>(d))) {
      return false;
    }
  }
  return true;
}

// Returns true if the index of the given dimension is the last.
bool StrideMap::Index::IsLast(FlexDimensions dimension) const {
  return MaxIndexOfDim(dimension) == indices_[dimension];
}

// Given that the dimensions up to and including dim-1 are valid, returns the
// maximum index for dimension dim.
int StrideMap::Index::MaxIndexOfDim(FlexDimensions dim) const {
  int max_index = stride_map_->shape_[dim] - 1;
  if (dim == FD_BATCH) {
    return max_index;
  }
  assert(0 <= indices_[FD_BATCH]);
  const size_t batch = indices_[FD_BATCH];
  if (dim == FD_HEIGHT) {
    if (batch >= stride_map_->heights_.size() || stride_map_->heights_[batch] > max_index) {
      return max_index;
    }
    return stride_map_->heights_[batch] - 1;
  }
  if (batch >= stride_map_->widths_.size() || stride_map_->widths_[batch] > max_index) {
    return max_index;
  }
  return stride_map_->widths_[batch] - 1;
}

// Adds the given offset to the given dimension. Returns true if the result
// makes a valid index.
bool StrideMap::Index::AddOffset(int offset, FlexDimensions dimension) {
  indices_[dimension] += offset;
  SetTFromIndices();
  return IsValid();
}

// Increments the index in some encapsulated way that guarantees to remain
// valid until it returns false, meaning that the iteration is complete.
bool StrideMap::Index::Increment() {
  for (int d = FD_DIMSIZE - 1; d >= 0; --d) {
    if (!IsLast(static_cast<FlexDimensions>(d))) {
      t_ += stride_map_->t_increments_[d];
      ++indices_[d];
      return true;
    }
    t_ -= stride_map_->t_increments_[d] * indices_[d];
    indices_[d] = 0;
    // Now carry to the next dimension.
  }
  return false;
}

// Decrements the index in some encapsulated way that guarantees to remain
// valid until it returns false, meaning that the iteration (that started
// with InitToLast()) is complete.
bool StrideMap::Index::Decrement() {
  for (int d = FD_DIMSIZE - 1; d >= 0; --d) {
    if (indices_[d] > 0) {
      --indices_[d];
      if (d == FD_BATCH) {
        // The upper limits of the other dimensions may have changed as a result
        // of a different batch index, so they have to be reset.
        InitToLastOfBatch(indices_[FD_BATCH]);
      } else {
        t_ -= stride_map_->t_increments_[d];
      }
      return true;
    }
    indices_[d] = MaxIndexOfDim(static_cast<FlexDimensions>(d));
    t_ += stride_map_->t_increments_[d] * indices_[d];
    // Now borrow from the next dimension.
  }
  return false;
}

// Initializes the indices to the last valid location in the given batch
// index.
void StrideMap::Index::InitToLastOfBatch(int batch) {
  indices_[FD_BATCH] = batch;
  for (int d = FD_BATCH + 1; d < FD_DIMSIZE; ++d) {
    indices_[d] = MaxIndexOfDim(static_cast<FlexDimensions>(d));
  }
  SetTFromIndices();
}

// Computes and sets t_ from the current indices_.
void StrideMap::Index::SetTFromIndices() {
  t_ = 0;
  for (int d = 0; d < FD_DIMSIZE; ++d) {
    t_ += stride_map_->t_increments_[d] * indices_[d];
  }
}

// Sets up the stride for the given array of height, width pairs.
void StrideMap::SetStride(const std::vector<std::pair<int, int>> &h_w_pairs) {
  int max_height = 0;
  int max_width = 0;
  for (const std::pair<int, int> &hw : h_w_pairs) {
    int height = hw.first;
    int width = hw.second;
    heights_.push_back(height);
    widths_.push_back(width);
    if (height > max_height) {
      max_height = height;
    }
    if (width > max_width) {
      max_width = width;
    }
  }
  shape_[FD_BATCH] = heights_.size();
  shape_[FD_HEIGHT] = max_height;
  shape_[FD_WIDTH] = max_width;
  ComputeTIncrements();
}

// Scales width and height dimensions by the given factors.
void StrideMap::ScaleXY(int x_factor, int y_factor) {
  for (int &height : heights_) {
    height /= y_factor;
  }
  for (int &width : widths_) {
    width /= x_factor;
  }
  shape_[FD_HEIGHT] /= y_factor;
  shape_[FD_WIDTH] /= x_factor;
  ComputeTIncrements();
}

// Reduces width to 1, across the batch, whatever the input size.
void StrideMap::ReduceWidthTo1() {
  widths_.assign(widths_.size(), 1);
  shape_[FD_WIDTH] = 1;
  ComputeTIncrements();
}

// Transposes the width and height dimensions.
void StrideMap::TransposeXY() {
  std::swap(shape_[FD_HEIGHT], shape_[FD_WIDTH]);
  std::swap(heights_, widths_);
  ComputeTIncrements();
}

// Computes t_increments_ from shape_.
void StrideMap::ComputeTIncrements() {
  t_increments_[FD_DIMSIZE - 1] = 1;
  for (int d = FD_DIMSIZE - 2; d >= 0; --d) {
    t_increments_[d] = t_increments_[d + 1] * shape_[d + 1];
  }
}

} // namespace tesseract
