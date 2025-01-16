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

#include <random>

#include "dropout.h"
#include "networkscratch.h"
#include "serialis.h"
#include "tesserrstream.h"  // for tesserr

namespace tesseract {

Dropout::Dropout(const std::string &name, int ni, float dropout_rate, uint8_t dimensions)
    : Network(NT_DROPOUT, name, ni, 0),
      dropout_rate_(dropout_rate),
      dimensions_(dimensions)
{
  if (dropout_rate_ < 0 || dropout_rate_ >= 1) {
    throw std::invalid_argument("Invalid dropout rate. Must be in [0, 1).");
  }
}

void Dropout::DebugWeights() {
  tesserr << "Must override Network::DebugWeights for type " << type_ << '\n';
}

// Writes to the given file. Returns false in case of error.
bool Dropout::Serialize(TFile *fp) const {
  return Network::Serialize(fp) && fp->Serialize(&dropout_rate_) && fp->Serialize(&dimensions_);
}

// Reads from the given file. Returns false in case of error.
bool Dropout::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(&dropout_rate_)) {
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
  // Resize output to match input dimensions
  output->Resize(input.NumFeatures(), input.TimeSize(), input.BatchSize());

  if (IsTraining()) {
    // Initialize the dropout mask
    dropout_mask_.Resize(input.NumFeatures(), input.TimeSize(), input.BatchSize());

    float retain_prob = 1.0f - dropout_rate_;

    // Random number generator setup
    std::mt19937 generator;  // You may need to use Tesseract's RNG
    std::bernoulli_distribution distribution(retain_prob);

    // Apply dropout mask
    for (int b = 0; b < input.BatchSize(); ++b) {
      for (int t = 0; t < input.TimeSize(); ++t) {
        const float* input_features = input.f(b, t);
        float* output_features = output->f(b, t);
        float* mask_features = dropout_mask_.f(b, t);
        for (int i = 0; i < input.NumFeatures(); ++i) {
          bool retain = distribution(generator);
          mask_features[i] = retain ? 1.0f : 0.0f;
          // Scale the activations to maintain expected value
          output_features[i] = input_features[i] * mask_features[i] / retain_prob;
        }
      }
    }
  } else {
    // During inference, pass input to output unchanged
    output->CopyAll(input);
  }

  if (debug) {
    tprintf("Dropout Forward Pass Complete.\n");
  }
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
  int size = deltas.Size();
  input_deltas->Resize(size);

  if (IsTraining()) {
    for (int i = 0; i < size; ++i) {
      (*input_deltas)(i) = deltas(i) * dropout_mask_[i];
    }
  } else {
    for (int i = 0; i < size; ++i) {
      (*input_deltas)(i) = deltas(i) * (1.0f - dropout_rate_);
    }
  }

#if 0
  tesserr << __FUNCTION__ << ": missing implementation\n";

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  back_deltas->Resize(fwd_deltas, ni_);
  for (unsigned i = 0; i < ni_; i++) {
    if (dist(gen) >= dropout_rate_) {
      // Keep the neuron
      output->push_back(input[i]);
    } else {
      // Drop the neuron
      output->push_back(0.0f);
    }
  }
#endif

#if 0
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
