///////////////////////////////////////////////////////////////////////
// File:        reversed.h
// Description: Runs a single network on time-reversed input, reversing output.
// Author:      Ray Smith
// Created:     Thu May 02 08:38:06 PST 2013
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

#ifndef TESSERACT_LSTM_REVERSED_H_
#define TESSERACT_LSTM_REVERSED_H_

#include "matrix.h"
#include "plumbing.h"

namespace tesseract {

// C++ Implementation of the Reversed class from lstm.py.
class Reversed : public Plumbing {
 public:
  explicit Reversed(const STRING& name, NetworkType type);
  virtual ~Reversed() = default;

  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  StaticShape OutputShape(const StaticShape& input_shape) const override;

  STRING spec() const override {
    STRING spec(type_ == NT_XREVERSED ? "Rx"
                                      : (type_ == NT_YREVERSED ? "Ry" : "Txy"));
    // For most simple cases, we will output Rx<net> or Ry<net> where <net> is
    // the network in stack_[0], but in the special case that <net> is an
    // LSTM, we will just output the LSTM's spec modified to take the reversal
    // into account. This is because when the user specified Lfy64, we actually
    // generated TxyLfx64, and if the user specified Lrx64 we actually
    // generated RxLfx64, and we want to display what the user asked for.
    STRING net_spec = stack_[0]->spec();
    if (net_spec[0] == 'L') {
      // Setup a from and to character according to the type of the reversal
      // such that the LSTM spec gets modified to the spec that the user
      // asked for
      char from = 'f';
      char to = 'r';
      if (type_ == NT_XYTRANSPOSE) {
        from = 'x';
        to = 'y';
      }
      // Change the from char to the to char.
      for (int i = 0; i < net_spec.length(); ++i) {
        if (net_spec[i] == from) net_spec[i] = to;
      }
      return net_spec;
    }
    spec += net_spec;
    return spec;
  }

  // Takes ownership of the given network to make it the reversed one.
  void SetNetwork(Network* network);

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
  // Copies src to *dest with the reversal according to type_.
  void ReverseData(const NetworkIO& src, NetworkIO* dest) const;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_REVERSED_H_
