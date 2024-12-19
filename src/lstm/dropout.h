///////////////////////////////////////////////////////////////////////
// File:        dropout.h
// Description: Standard Dropout layer.
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

#ifndef TESSERACT_LSTM_DROPOUT_H_
#define TESSERACT_LSTM_DROPOUT_H_

#include "network.h"

namespace tesseract {

// Dropout.
class Dropout : public Network {
public:
  TESS_API
  Dropout(const std::string &name, int ni, float dropout_rate, uint8_t dimensions);
  ~Dropout() override = default;

  // Accessors.
  std::string spec() const override {
    return "Do" + std::to_string(dropout_rate_) + "," + std::to_string(dimensions_);
  }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(TFile *fp) const override;
  // Reads from the given file. Returns false in case of error.
  bool DeSerialize(TFile *fp) override;

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  void Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
               NetworkScratch *scratch, NetworkIO *output) override;

  // Runs backward propagation of errors on the deltas line.
  // See Network for a detailed discussion of the arguments.
  bool Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                NetworkIO *back_deltas) override;

private:
  void DebugWeights() override;

  float dropout_rate_;
  uint8_t dimensions_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_DROPOUT_H_
