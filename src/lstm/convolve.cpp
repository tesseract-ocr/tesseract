///////////////////////////////////////////////////////////////////////
// File:        convolve.cpp
// Description: Convolutional layer that stacks the inputs over its rectangle
//              and pulls in random data to fill out-of-input inputs.
//              Output is therefore same size as its input, but deeper.
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "convolve.h"

#include "networkscratch.h"
#include "serialis.h"

namespace tesseract {

Convolve::Convolve(const std::string &name, int ni, int half_x, int half_y)
    : Network(NT_CONVOLVE, name, ni, ni * (2 * half_x + 1) * (2 * half_y + 1))
    , half_x_(half_x)
    , half_y_(half_y) {}

// Writes to the given file. Returns false in case of error.
bool Convolve::Serialize(TFile *fp) const {
  return Network::Serialize(fp) && fp->Serialize(&half_x_) && fp->Serialize(&half_y_);
}

// Reads from the given file. Returns false in case of error.
bool Convolve::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(&half_x_)) {
    return false;
  }
  if (!fp->DeSerialize(&half_y_)) {
    return false;
  }
  no_ = ni_ * (2 * half_x_ + 1) * (2 * half_y_ + 1);
  return true;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Convolve::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                       NetworkScratch *scratch, NetworkIO *output) {
  output->Resize(input, no_);
  int y_scale = 2 * half_y_ + 1;
  StrideMap::Index dest_index(output->stride_map());
  do {
    // Stack x_scale groups of y_scale * ni_ inputs together.
    int t = dest_index.t();
    int out_ix = 0;
    for (int x = -half_x_; x <= half_x_; ++x, out_ix += y_scale * ni_) {
      StrideMap::Index x_index(dest_index);
      if (!x_index.AddOffset(x, FD_WIDTH)) {
        // This x is outside the image.
        output->Randomize(t, out_ix, y_scale * ni_, randomizer_);
      } else {
        int out_iy = out_ix;
        for (int y = -half_y_; y <= half_y_; ++y, out_iy += ni_) {
          StrideMap::Index y_index(x_index);
          if (!y_index.AddOffset(y, FD_HEIGHT)) {
            // This y is outside the image.
            output->Randomize(t, out_iy, ni_, randomizer_);
          } else {
            output->CopyTimeStepGeneral(t, out_iy, ni_, input, y_index.t(), 0);
          }
        }
      }
    }
  } while (dest_index.Increment());
#ifndef GRAPHICS_DISABLED
  if (debug) {
    DisplayForward(*output);
  }
#endif
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Convolve::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                        NetworkIO *back_deltas) {
  back_deltas->Resize(fwd_deltas, ni_);
  NetworkScratch::IO delta_sum;
  delta_sum.ResizeFloat(fwd_deltas, ni_, scratch);
  delta_sum->Zero();
  int y_scale = 2 * half_y_ + 1;
  StrideMap::Index src_index(fwd_deltas.stride_map());
  do {
    // Stack x_scale groups of y_scale * ni_ inputs together.
    int t = src_index.t();
    int out_ix = 0;
    for (int x = -half_x_; x <= half_x_; ++x, out_ix += y_scale * ni_) {
      StrideMap::Index x_index(src_index);
      if (x_index.AddOffset(x, FD_WIDTH)) {
        int out_iy = out_ix;
        for (int y = -half_y_; y <= half_y_; ++y, out_iy += ni_) {
          StrideMap::Index y_index(x_index);
          if (y_index.AddOffset(y, FD_HEIGHT)) {
            fwd_deltas.AddTimeStepPart(t, out_iy, ni_, delta_sum->f(y_index.t()));
          }
        }
      }
    }
  } while (src_index.Increment());
  back_deltas->CopyAll(*delta_sum);
  return true;
}

} // namespace tesseract.
