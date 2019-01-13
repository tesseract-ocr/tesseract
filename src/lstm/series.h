///////////////////////////////////////////////////////////////////////
// File:        series.h
// Description: Runs networks in series on the same input.
// Author:      Ray Smith
// Created:     Thu May 02 08:20:06 PST 2013
//
// (C) Copyright 2013, Google Inc.
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

#ifndef TESSERACT_LSTM_SERIES_H_
#define TESSERACT_LSTM_SERIES_H_

#include "plumbing.h"

namespace tesseract {

// Runs two or more networks in series (layers) on the same input.
class Series : public Plumbing {
 public:
  // ni_ and no_ will be set by AddToStack.
  explicit Series(const STRING& name);
  virtual ~Series() = default;

  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  StaticShape OutputShape(const StaticShape& input_shape) const override;

  STRING spec() const override {
    STRING spec("[");
    for (int i = 0; i < stack_.size(); ++i)
      spec += stack_[i]->spec();
    spec += "]";
    return spec;
  }

  // Sets up the network for training. Initializes weights using weights of
  // scale `range` picked according to the random number generator `randomizer`.
  // Returns the number of weights initialized.
  int InitWeights(float range, TRand* randomizer) override;
  // Recursively searches the network for softmaxes with old_no outputs,
  // and remaps their outputs according to code_map. See network.h for details.
  int RemapOutputs(int old_no, const std::vector<int>& code_map) override;

  // Sets needs_to_backprop_ to needs_backprop and returns true if
  // needs_backprop || any weights in this network so the next layer forward
  // can be told to produce backprop for this layer if needed.
  bool SetupNeedsBackprop(bool needs_backprop) override;

  // Returns an integer reduction factor that the network applies to the
  // time sequence. Assumes that any 2-d is already eliminated. Used for
  // scaling bounding boxes of truth data.
  // WARNING: if GlobalMinimax is used to vary the scale, this will return
  // the last used scale factor. Call it before any forward, and it will return
  // the minimum scale factor of the paths through the GlobalMinimax.
  int XScaleFactor() const override;

  // Provides the (minimum) x scale factor to the network (of interest only to
  // input units) so they can determine how to scale bounding boxes.
  void CacheXScaleFactor(int factor) override;

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  void Forward(bool debug, const NetworkIO& input,
               const TransposedArray* input_transpose, NetworkScratch* scratch,
               NetworkIO* output) override;

  // Runs backward propagation of errors on the deltas line.
  // See Network for a detailed discussion of the arguments.
  bool Backward(bool debug, const NetworkIO& fwd_deltas,
                NetworkScratch* scratch, NetworkIO* back_deltas) override;

  // Splits the series after the given index, returning the two parts and
  // deletes itself. The first part, up to network with index last_start, goes
  // into start, and the rest goes into end.
  void SplitAt(int last_start, Series** start, Series** end);

  // Appends the elements of the src series to this, removing from src and
  // deleting it.
  void AppendSeries(Network* src);
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_SERIES_H_
