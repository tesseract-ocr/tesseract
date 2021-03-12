///////////////////////////////////////////////////////////////////////
// File:        maxpool.cpp
// Description: Standard Max-Pooling layer.
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

#include "maxpool.h"

namespace tesseract {

Maxpool::Maxpool(const char *name, int ni, int x_scale, int y_scale)
    : Reconfig(name, ni, x_scale, y_scale) {
  type_ = NT_MAXPOOL;
  no_ = ni;
}

// Reads from the given file. Returns false in case of error.
bool Maxpool::DeSerialize(TFile *fp) {
  bool result = Reconfig::DeSerialize(fp);
  no_ = ni_;
  return result;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Maxpool::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                      NetworkScratch *scratch, NetworkIO *output) {
  output->ResizeScaled(input, x_scale_, y_scale_, no_);
  maxes_.ResizeNoInit(output->Width(), ni_);
  back_map_ = input.stride_map();

  StrideMap::Index dest_index(output->stride_map());
  do {
    int out_t = dest_index.t();
    StrideMap::Index src_index(input.stride_map(), dest_index.index(FD_BATCH),
                               dest_index.index(FD_HEIGHT) * y_scale_,
                               dest_index.index(FD_WIDTH) * x_scale_);
    // Find the max input out of x_scale_ groups of y_scale_ inputs.
    // Do it independently for each input dimension.
    int *max_line = maxes_[out_t];
    int in_t = src_index.t();
    output->CopyTimeStepFrom(out_t, input, in_t);
    for (int i = 0; i < ni_; ++i) {
      max_line[i] = in_t;
    }
    for (int x = 0; x < x_scale_; ++x) {
      for (int y = 0; y < y_scale_; ++y) {
        StrideMap::Index src_xy(src_index);
        if (src_xy.AddOffset(x, FD_WIDTH) && src_xy.AddOffset(y, FD_HEIGHT)) {
          output->MaxpoolTimeStep(out_t, input, src_xy.t(), max_line);
        }
      }
    }
  } while (dest_index.Increment());
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Maxpool::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                       NetworkIO *back_deltas) {
  back_deltas->ResizeToMap(fwd_deltas.int_mode(), back_map_, ni_);
  back_deltas->MaxpoolBackward(fwd_deltas, maxes_);
  return true;
}

} // namespace tesseract.
