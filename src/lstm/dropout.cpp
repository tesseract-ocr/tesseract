///////////////////////////////////////////////////////////////////////
// File:        dropout.cpp
// Description: Dropout layer.
// Author:      Stefan Weil
//
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

#include "dropout.h"

#include "networkscratch.h"
#include "serialis.h"

namespace tesseract {

Dropout::Dropout(const std::string &name, int ni, float probability, uint8_t dimensions)
    : Network(NT_DROPOUT, name, ni, 0),
      probability_(probability),
      dimensions_(dimensions)
{
}

// Writes to the given file. Returns false in case of error.
bool Dropout::Serialize(TFile *fp) const {
  return Network::Serialize(fp) && fp->Serialize(&probability_) && fp->Serialize(&dimensions_);
}

// Reads from the given file. Returns false in case of error.
bool Dropout::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(&probability_)) {
    return false;
  }
  if (!fp->DeSerialize(&dimensions_)) {
    return false;
  }
  no_ = ni_;
  return true;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Dropout::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                       NetworkScratch *scratch, NetworkIO *output) {
#if 0
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
#endif
#ifndef GRAPHICS_DISABLED
  if (debug) {
    DisplayForward(*output);
  }
#endif
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Dropout::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                        NetworkIO *back_deltas) {
#if 0
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
#endif
  return true;
}

} // namespace tesseract.
