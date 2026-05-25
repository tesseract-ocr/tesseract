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
#include "tesserrstream.h" // for tesserr

namespace tesseract {

Dropout::Dropout(const std::string &name, int ni, float dropout_rate)
    : Network(NT_DROPOUT, name, ni, ni), dropout_mask_(), dropout_rate_(dropout_rate) {
  if (dropout_rate_ < 0 || dropout_rate_ >= 1) {
    throw std::invalid_argument("Invalid dropout rate. Must be in [0, 1).");
  }
}

void Dropout::DebugWeights() {
  tesserr << "Dropout layer '" << name_ << "': rate=" << dropout_rate_ << '\n';
}

// Writes to the given file. Returns false in case of error.
bool Dropout::Serialize(TFile *fp) const {
  // dropout_mask_ is runtime data and is not serialized.
  return Network::Serialize(fp) && fp->Serialize(&dropout_rate_);
}

// Reads from the given file. Returns false in case of error.
bool Dropout::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(&dropout_rate_)) {
    return false;
  }
  no_ = ni_;
  return true;
}

// Runs forward propagation of activations on the input line.
// See Network for a detailed discussion of the arguments.
void Dropout::Forward(bool debug, const NetworkIO &input,
                      const TransposedArray *input_transpose, NetworkScratch *scratch,
                      NetworkIO *output) {
  // Output has the same shape as input (ni_ == no_).
  output->Resize(input, no_);

  int width = input.Width();
  int num_features = input.NumFeatures();

  if (IsTraining() && dropout_rate_ > 0.0f) {
    // Inverted dropout: scale retained activations by 1/keep_prob so that
    // inference can pass the network output through unchanged.
    float keep_prob = 1.0f - dropout_rate_;
    float scale = 1.0f / keep_prob;

    int num_elements = width * num_features;
    dropout_mask_.resize(num_elements);

    for (int t = 0; t < width; ++t) {
      const float *in = input.f(t);
      float *out = output->f(t);
      char *mask = dropout_mask_.data() + t * num_features;
      for (int i = 0; i < num_features; ++i) {
        // UnsignedRand(1.0) returns a value in [0, 1].
        float r = static_cast<float>(randomizer_->UnsignedRand(1.0));
        if (r < keep_prob) {
          mask[i] = 1;
          out[i] = in[i] * scale;
        } else {
          mask[i] = 0;
          out[i] = 0.0f;
        }
      }
    }
  } else {
    // Inference mode (or dropout_rate_ == 0): pass input through unchanged.
    output->CopyAll(input);
  }

#ifndef GRAPHICS_DISABLED
  if (debug) {
    DisplayForward(*output);
  }
#endif
}

// Runs backward propagation of errors on the deltas line.
// See Network for a detailed discussion of the arguments.
bool Dropout::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                       NetworkIO *back_deltas) {
  back_deltas->Resize(fwd_deltas, ni_);

  int width = fwd_deltas.Width();
  int num_features = fwd_deltas.NumFeatures();

  if (IsTraining() && dropout_rate_ > 0.0f) {
    // Apply the same inverted-dropout mask that was used in Forward.
    float keep_prob = 1.0f - dropout_rate_;
    float scale = 1.0f / keep_prob;

    for (int t = 0; t < width; ++t) {
      const float *in = fwd_deltas.f(t);
      float *out = back_deltas->f(t);
      const char *mask = dropout_mask_.data() + t * num_features;
      for (int i = 0; i < num_features; ++i) {
        out[i] = mask[i] ? in[i] * scale : 0.0f;
      }
    }
  } else {
    // Inference mode: pass gradients through unchanged.
    back_deltas->CopyAll(fwd_deltas);
  }

#ifndef GRAPHICS_DISABLED
  if (debug) {
    DisplayBackward(*back_deltas);
  }
#endif
  return needs_to_backprop_;
}

} // namespace tesseract.
