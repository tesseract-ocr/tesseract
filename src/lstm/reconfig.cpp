///////////////////////////////////////////////////////////////////////
// File:        reconfig.cpp
// Description: Network layer that reconfigures the scaling vs feature
//              depth.
// Author:      Ray Smith
//
// (C) Copyright 2014, Google Inc.
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

#include "reconfig.h"

namespace tesseract {

Reconfig::Reconfig(const std::string &name, int ni, int x_scale, int y_scale)
    : Network(NT_RECONFIG, name, ni, ni * x_scale * y_scale)
    , x_scale_(x_scale)
    , y_scale_(y_scale) {}

// Returns the shape output from the network given an input shape (which may
// be partially unknown ie zero).
StaticShape Reconfig::OutputShape(const StaticShape &input_shape) const {
  StaticShape result = input_shape;
  result.set_height(result.height() / y_scale_);
  result.set_width(result.width() / x_scale_);
  if (type_ != NT_MAXPOOL) {
    result.set_depth(result.depth() * y_scale_ * x_scale_);
  }
  return result;
}

// Returns an integer reduction factor that the network applies to the
// time sequence. Assumes that any 2-d is already eliminated. Used for
// scaling bounding boxes of truth data.
// WARNING: if GlobalMinimax is used to vary the scale, this will return
// the last used scale factor. Call it before any forward, and it will return
// the minimum scale factor of the paths through the GlobalMinimax.
int Reconfig::XScaleFactor() const {
  return x_scale_;
}

// Writes to the given file. Returns false in case of error.
bool Reconfig::Serialize(TFile *fp) const {
  return Network::Serialize(fp) && fp->Serialize(&x_scale_) && fp->Serialize(&y_scale_);
}

// Reads from the given file. Returns false in case of error.
bool Reconfig::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(&x_scale_)) {
    return false;
  }
  if (!fp->DeSerialize(&y_scale_)) {
    return false;
  }
  no_ = ni_ * x_scale_ * y_scale_;
  return true;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Reconfig::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                       NetworkScratch *scratch, NetworkIO *output) {
  output->ResizeScaled(input, x_scale_, y_scale_, no_);
  back_map_ = input.stride_map();
  StrideMap::Index dest_index(output->stride_map());
  do {
    int out_t = dest_index.t();
    StrideMap::Index src_index(input.stride_map(), dest_index.index(FD_BATCH),
                               dest_index.index(FD_HEIGHT) * y_scale_,
                               dest_index.index(FD_WIDTH) * x_scale_);
    // Stack x_scale_ groups of y_scale_ inputs together.
    for (int x = 0; x < x_scale_; ++x) {
      for (int y = 0; y < y_scale_; ++y) {
        StrideMap::Index src_xy(src_index);
        if (src_xy.AddOffset(x, FD_WIDTH) && src_xy.AddOffset(y, FD_HEIGHT)) {
          output->CopyTimeStepGeneral(out_t, (x * y_scale_ + y) * ni_, ni_, input, src_xy.t(), 0);
        }
      }
    }
  } while (dest_index.Increment());
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Reconfig::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                        NetworkIO *back_deltas) {
  back_deltas->ResizeToMap(fwd_deltas.int_mode(), back_map_, ni_);
  StrideMap::Index src_index(fwd_deltas.stride_map());
  do {
    int in_t = src_index.t();
    StrideMap::Index dest_index(back_deltas->stride_map(), src_index.index(FD_BATCH),
                                src_index.index(FD_HEIGHT) * y_scale_,
                                src_index.index(FD_WIDTH) * x_scale_);
    // Unstack x_scale_ groups of y_scale_ inputs that are together.
    for (int x = 0; x < x_scale_; ++x) {
      for (int y = 0; y < y_scale_; ++y) {
        StrideMap::Index dest_xy(dest_index);
        if (dest_xy.AddOffset(x, FD_WIDTH) && dest_xy.AddOffset(y, FD_HEIGHT)) {
          back_deltas->CopyTimeStepGeneral(dest_xy.t(), 0, ni_, fwd_deltas, in_t,
                                           (x * y_scale_ + y) * ni_);
        }
      }
    }
  } while (src_index.Increment());
  return needs_to_backprop_;
}

} // namespace tesseract.
