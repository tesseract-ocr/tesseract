///////////////////////////////////////////////////////////////////////
// File:        reversed.cpp
// Description: Runs a single network on time-reversed input, reversing output.
// Author:      Ray Smith
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

#include "reversed.h"

#include <cstdio>

#include "networkscratch.h"

namespace tesseract {

Reversed::Reversed(const std::string &name, NetworkType type) : Plumbing(name) {
  type_ = type;
}

// Returns the shape output from the network given an input shape (which may
// be partially unknown ie zero).
StaticShape Reversed::OutputShape(const StaticShape &input_shape) const {
  if (type_ == NT_XYTRANSPOSE) {
    StaticShape x_shape(input_shape);
    x_shape.set_width(input_shape.height());
    x_shape.set_height(input_shape.width());
    x_shape = stack_[0]->OutputShape(x_shape);
    x_shape.SetShape(x_shape.batch(), x_shape.width(), x_shape.height(), x_shape.depth());
    return x_shape;
  }
  return stack_[0]->OutputShape(input_shape);
}

// Takes ownership of the given network to make it the reversed one.
void Reversed::SetNetwork(Network *network) {
  stack_.clear();
  AddToStack(network);
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Reversed::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                       NetworkScratch *scratch, NetworkIO *output) {
  NetworkScratch::IO rev_input(input, scratch);
  ReverseData(input, rev_input);
  NetworkScratch::IO rev_output(input, scratch);
  stack_[0]->Forward(debug, *rev_input, nullptr, scratch, rev_output);
  ReverseData(*rev_output, output);
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Reversed::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                        NetworkIO *back_deltas) {
  NetworkScratch::IO rev_input(fwd_deltas, scratch);
  ReverseData(fwd_deltas, rev_input);
  NetworkScratch::IO rev_output(fwd_deltas, scratch);
  if (stack_[0]->Backward(debug, *rev_input, scratch, rev_output)) {
    ReverseData(*rev_output, back_deltas);
    return true;
  }
  return false;
}

// Copies src to *dest with the reversal according to type_.
void Reversed::ReverseData(const NetworkIO &src, NetworkIO *dest) const {
  if (type_ == NT_XREVERSED) {
    dest->CopyWithXReversal(src);
  } else if (type_ == NT_YREVERSED) {
    dest->CopyWithYReversal(src);
  } else {
    dest->CopyWithXYTranspose(src);
  }
}

} // namespace tesseract.
