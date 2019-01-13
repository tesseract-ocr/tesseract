///////////////////////////////////////////////////////////////////////
// File:        parallel.h
// Description: Runs networks in parallel on the same input.
// Author:      Ray Smith
// Created:     Thu May 02 08:02:06 PST 2013
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

#ifndef TESSERACT_LSTM_PARALLEL_H_
#define TESSERACT_LSTM_PARALLEL_H_

#include "plumbing.h"

namespace tesseract {

// Runs multiple networks in parallel, interlacing their outputs.
class Parallel : public Plumbing {
 public:
  // ni_ and no_ will be set by AddToStack.
  Parallel(const STRING& name, NetworkType type);
  virtual ~Parallel() = default;

  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  StaticShape OutputShape(const StaticShape& input_shape) const override;

  STRING spec() const override {
    STRING spec;
    if (type_ == NT_PAR_2D_LSTM) {
      // We have 4 LSTMs operating in parallel here, so the size of each is
      // the number of outputs/4.
      spec.add_str_int("L2xy", no_ / 4);
    } else if (type_ == NT_PAR_RL_LSTM) {
      // We have 2 LSTMs operating in parallel here, so the size of each is
      // the number of outputs/2.
      if (stack_[0]->type() == NT_LSTM_SUMMARY)
        spec.add_str_int("Lbxs", no_ / 2);
      else
        spec.add_str_int("Lbx", no_ / 2);
    } else {
      if (type_ == NT_REPLICATED) {
        spec.add_str_int("R", stack_.size());
        spec += "(";
        spec += stack_[0]->spec();
      } else {
        spec = "(";
        for (int i = 0; i < stack_.size(); ++i) spec += stack_[i]->spec();
      }
      spec += ")";
    }
    return spec;
  }

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
  // If *this is a NT_REPLICATED, then it feeds a replicated network with
  // identical inputs, and it would be extremely wasteful for them to each
  // calculate and store the same transpose of the inputs, so Parallel does it
  // and passes a pointer to the replicated network, allowing it to use the
  // transpose on the next call to Backward.
  TransposedArray transposed_input_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_PARALLEL_H_
