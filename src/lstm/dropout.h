///////////////////////////////////////////////////////////////////////
// File:        dropout.h
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

#ifndef TESSERACT_LSTM_DROPOUT_H_
#define TESSERACT_LSTM_DROPOUT_H_

#include "network.h"

#include <cstdio> // for std::snprintf
#include <string>

namespace tesseract {

// Placeholder for a dropout layer.
//
// A dropout layer only changes its input while a network is trained. When a
// model is used for recognition, the layer passes its input through
// unchanged. This class implements only that pass-through, so models which
// contain a dropout layer can be loaded and used for recognition. This
// version of Tesseract cannot train with dropout.
//
// The layer stores two values in a model file: the dropout rate (float,
// 0 <= rate < 1) and the dropout dimension (32 bit unsigned integer,
// 0 = single elements, 1 = whole timesteps, 2 = whole features).
class Dropout : public Network {
public:
  TESS_API
  Dropout(const std::string &name, int ni, float dropout_rate, unsigned dropout_dim = 0);
  ~Dropout() override = default;

  // Accessors.
  std::string spec() const override {
    // %g gives a compact representation (eg "0.5") instead of the six-decimal
    // padding of std::to_string (eg "0.500000").
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Do%g", static_cast<double>(dropout_rate_));
    std::string s = buf;
    if (dropout_dim_ != 0) {
      s += "," + std::to_string(dropout_dim_);
    }
    return s;
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

  // Returns true if the rate and the dimension are valid.
  static bool IsValid(float dropout_rate, unsigned dropout_dim) {
    // The comparison is written so that it is false for NaN.
    return dropout_rate >= 0 && dropout_rate < 1 && dropout_dim <= 2;
  }

  float dropout_rate_;
  unsigned dropout_dim_; // 0=elementwise, 1=temporal, 2=feature/channel
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_DROPOUT_H_
