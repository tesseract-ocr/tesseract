///////////////////////////////////////////////////////////////////////
// File:        dropout.cpp
// Description: Placeholder for a dropout layer.
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
#include "networkio.h"
#include "serialis.h"
#include "tprintf.h"

#include <cstdint>   // for uint32_t
#include <stdexcept> // for std::invalid_argument

namespace tesseract {

Dropout::Dropout(const std::string &name, int ni, float dropout_rate, unsigned dropout_dim)
    : Network(NT_DROPOUT, name, ni, ni), dropout_rate_(dropout_rate), dropout_dim_(dropout_dim) {
  if (!IsValid(dropout_rate, dropout_dim)) {
    throw std::invalid_argument(
        "Invalid dropout layer. The rate must be in [0, 1), the dimension 0, 1 or 2.");
  }
}

void Dropout::DebugWeights() {
  tprintf("Dropout layer '%s': rate=%g, dimension=%u\n", name_.c_str(),
          static_cast<double>(dropout_rate_), dropout_dim_);
}

// Writes to the given file. Returns false in case of error.
bool Dropout::Serialize(TFile *fp) const {
  uint32_t dropout_dim = dropout_dim_;
  return Network::Serialize(fp) && fp->Serialize(&dropout_rate_) && fp->Serialize(&dropout_dim);
}

// Reads from the given file. Returns false in case of error.
bool Dropout::DeSerialize(TFile *fp) {
  float dropout_rate;
  uint32_t dropout_dim;
  if (!fp->DeSerialize(&dropout_rate) || !fp->DeSerialize(&dropout_dim)) {
    return false;
  }
  if (!IsValid(dropout_rate, dropout_dim)) {
    // Reject a corrupt model.
    return false;
  }
  dropout_rate_ = dropout_rate;
  dropout_dim_ = dropout_dim;
  no_ = ni_;
  return true;
}

// Runs forward propagation of activations on the input line.
// See Network for a detailed discussion of the arguments.
void Dropout::Forward([[maybe_unused]] bool debug, const NetworkIO &input,
                      [[maybe_unused]] const TransposedArray *input_transpose,
                      [[maybe_unused]] NetworkScratch *scratch, NetworkIO *output) {
  // The output has the same shape as the input (ni_ == no_).
  output->Resize(input, no_);
  // Pass the input through unchanged. CopyTimeStepFrom is mode-aware, so
  // this also works for a network which was converted to integer mode.
  const int width = input.Width();
  for (int t = 0; t < width; ++t) {
    output->CopyTimeStepFrom(t, input, t);
  }
}

// Runs backward propagation of errors on the deltas line.
// See Network for a detailed discussion of the arguments.
bool Dropout::Backward([[maybe_unused]] bool debug, const NetworkIO &fwd_deltas,
                       [[maybe_unused]] NetworkScratch *scratch, NetworkIO *back_deltas) {
  back_deltas->Resize(fwd_deltas, ni_);
  // Pass the deltas through unchanged.
  const int width = fwd_deltas.Width();
  for (int t = 0; t < width; ++t) {
    back_deltas->CopyTimeStepFrom(t, fwd_deltas, t);
  }
  return needs_to_backprop_;
}

} // namespace tesseract.
