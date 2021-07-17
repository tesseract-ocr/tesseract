///////////////////////////////////////////////////////////////////////
// File:        convolve.h
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

#ifndef TESSERACT_LSTM_CONVOLVE_H_
#define TESSERACT_LSTM_CONVOLVE_H_

#include "matrix.h"
#include "network.h"

namespace tesseract {

// Makes each time-step deeper by stacking inputs over its rectangle. Does not
// affect the size of its input. Achieves this by bringing in random values in
// out-of-input areas.
class Convolve : public Network {
public:
  // The area of convolution is 2*half_x + 1 by 2*half_y + 1, forcing it to
  // always be odd, so the center is the current pixel.
  TESS_API
  Convolve(const std::string &name, int ni, int half_x, int half_y);
  ~Convolve() override = default;

  std::string spec() const override {
    return "C" + std::to_string(half_y_ * 2 + 1) + "," + std::to_string(half_x_ * 2 + 1);
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
  void DebugWeights() override {
    tprintf("Must override Network::DebugWeights for type %d\n", type_);
  }

protected:
  // Serialized data.
  int32_t half_x_;
  int32_t half_y_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_SUBSAMPLE_H_
