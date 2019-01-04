///////////////////////////////////////////////////////////////////////
// File:        maxpool.h
// Description: Standard Max-Pooling layer.
// Author:      Ray Smith
// Created:     Tue Mar 18 16:28:18 PST 2014
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

#ifndef TESSERACT_LSTM_MAXPOOL_H_
#define TESSERACT_LSTM_MAXPOOL_H_

#include "reconfig.h"

namespace tesseract {

// Maxpooling reduction. Independently for each input, selects the location
// in the rectangle that contains the max value.
// Backprop propagates only to the position that was the max.
class Maxpool : public Reconfig {
 public:
  Maxpool(const STRING& name, int ni, int x_scale, int y_scale);
  virtual ~Maxpool() = default;

  // Accessors.
  STRING spec() const override {
    STRING spec;
    spec.add_str_int("Mp", y_scale_);
    spec.add_str_int(",", x_scale_);
    return spec;
  }

  // Reads from the given file. Returns false in case of error.
  bool DeSerialize(TFile* fp) override;

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  void Forward(bool debug, const NetworkIO& input,
               const TransposedArray* input_transpose,
               NetworkScratch* scratch, NetworkIO* output) override;

  // Runs backward propagation of errors on the deltas line.
  // See Network for a detailed discussion of the arguments.
  bool Backward(bool debug, const NetworkIO& fwd_deltas,
                NetworkScratch* scratch,
                NetworkIO* back_deltas) override;

 private:
  // Memory of which input was the max.
  GENERIC_2D_ARRAY<int> maxes_;
};


}  // namespace tesseract.





#endif  // TESSERACT_LSTM_MAXPOOL_H_
