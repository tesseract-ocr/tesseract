///////////////////////////////////////////////////////////////////////
// File:        reconfig.h
// Description: Network layer that reconfigures the scaling vs feature
//              depth.
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

#ifndef TESSERACT_LSTM_RECONFIG_H_
#define TESSERACT_LSTM_RECONFIG_H_

#include "matrix.h"
#include "network.h"

namespace tesseract {

// Reconfigures (Shrinks) the inputs by concatenating an x_scale by y_scale tile
// of inputs together, producing a single, deeper output per tile.
// Note that fractional parts are truncated for efficiency, so make sure the
// input stride is a multiple of the y_scale factor!
class Reconfig : public Network {
public:
  TESS_API
  Reconfig(const std::string &name, int ni, int x_scale, int y_scale);
  ~Reconfig() override = default;

  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  StaticShape OutputShape(const StaticShape &input_shape) const override;

  std::string spec() const override {
    return "S" + std::to_string(y_scale_) + "," + std::to_string(x_scale_);
  }

  // Returns an integer reduction factor that the network applies to the
  // time sequence. Assumes that any 2-d is already eliminated. Used for
  // scaling bounding boxes of truth data.
  // WARNING: if GlobalMinimax is used to vary the scale, this will return
  // the last used scale factor. Call it before any forward, and it will return
  // the minimum scale factor of the paths through the GlobalMinimax.
  int XScaleFactor() const override;

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
  // Non-serialized data used to store parameters between forward and back.
  StrideMap back_map_;
  // Serialized data.
  int32_t x_scale_;
  int32_t y_scale_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_SUBSAMPLE_H_
