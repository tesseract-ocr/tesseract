///////////////////////////////////////////////////////////////////////
// File:        series.cpp
// Description: Runs networks in series on the same input.
// Author:      Ray Smith
// Created:     Thu May 02 08:26:06 PST 2013
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

#include "series.h"

#include "fullyconnected.h"
#include "networkscratch.h"
#include "scrollview.h"
#include "tprintf.h"

namespace tesseract {

// ni_ and no_ will be set by AddToStack.
Series::Series(const STRING& name) : Plumbing(name) {
  type_ = NT_SERIES;
}

// Returns the shape output from the network given an input shape (which may
// be partially unknown ie zero).
StaticShape Series::OutputShape(const StaticShape& input_shape) const {
  StaticShape result(input_shape);
  int stack_size = stack_.size();
  for (int i = 0; i < stack_size; ++i) {
    result = stack_[i]->OutputShape(result);
  }
  return result;
}

// Sets up the network for training. Initializes weights using weights of
// scale `range` picked according to the random number generator `randomizer`.
// Note that series has its own implementation just for debug purposes.
int Series::InitWeights(float range, TRand* randomizer) {
  num_weights_ = 0;
  tprintf("Num outputs,weights in Series:\n");
  for (int i = 0; i < stack_.size(); ++i) {
    int weights = stack_[i]->InitWeights(range, randomizer);
    tprintf("  %s:%d, %d\n",
            stack_[i]->spec().string(), stack_[i]->NumOutputs(), weights);
    num_weights_ += weights;
  }
  tprintf("Total weights = %d\n", num_weights_);
  return num_weights_;
}

// Recursively searches the network for softmaxes with old_no outputs,
// and remaps their outputs according to code_map. See network.h for details.
int Series::RemapOutputs(int old_no, const std::vector<int>& code_map) {
  num_weights_ = 0;
  tprintf("Num (Extended) outputs,weights in Series:\n");
  for (int i = 0; i < stack_.size(); ++i) {
    int weights = stack_[i]->RemapOutputs(old_no, code_map);
    tprintf("  %s:%d, %d\n", stack_[i]->spec().string(),
            stack_[i]->NumOutputs(), weights);
    num_weights_ += weights;
  }
  tprintf("Total weights = %d\n", num_weights_);
  no_ = stack_.back()->NumOutputs();
  return num_weights_;
}

// Sets needs_to_backprop_ to needs_backprop and returns true if
// needs_backprop || any weights in this network so the next layer forward
// can be told to produce backprop for this layer if needed.
bool Series::SetupNeedsBackprop(bool needs_backprop) {
  needs_to_backprop_ = needs_backprop;
  for (int i = 0; i < stack_.size(); ++i)
    needs_backprop = stack_[i]->SetupNeedsBackprop(needs_backprop);
  return needs_backprop;
}

// Returns an integer reduction factor that the network applies to the
// time sequence. Assumes that any 2-d is already eliminated. Used for
// scaling bounding boxes of truth data.
// WARNING: if GlobalMinimax is used to vary the scale, this will return
// the last used scale factor. Call it before any forward, and it will return
// the minimum scale factor of the paths through the GlobalMinimax.
int Series::XScaleFactor() const {
  int factor = 1;
  for (int i = 0; i < stack_.size(); ++i)
      factor *= stack_[i]->XScaleFactor();
  return factor;
}

// Provides the (minimum) x scale factor to the network (of interest only to
// input units) so they can determine how to scale bounding boxes.
void Series::CacheXScaleFactor(int factor) {
  stack_[0]->CacheXScaleFactor(factor);
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Series::Forward(bool debug, const NetworkIO& input,
                     const TransposedArray* input_transpose,
                     NetworkScratch* scratch, NetworkIO* output) {
  int stack_size = stack_.size();
  ASSERT_HOST(stack_size > 1);
  // Revolving intermediate buffers.
  NetworkScratch::IO buffer1(input, scratch);
  NetworkScratch::IO buffer2(input, scratch);
  // Run each network in turn, giving the output of n as the input to n + 1,
  // with the final network providing the real output.
  stack_[0]->Forward(debug, input, input_transpose, scratch, buffer1);
  for (int i = 1; i < stack_size; i += 2) {
    stack_[i]->Forward(debug, *buffer1, nullptr, scratch,
                       i + 1 < stack_size ? buffer2 : output);
    if (i + 1 == stack_size) return;
    stack_[i + 1]->Forward(debug, *buffer2, nullptr, scratch,
                           i + 2 < stack_size ? buffer1 : output);
  }
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Series::Backward(bool debug, const NetworkIO& fwd_deltas,
                      NetworkScratch* scratch,
                      NetworkIO* back_deltas) {
  if (!IsTraining()) return false;
  int stack_size = stack_.size();
  ASSERT_HOST(stack_size > 1);
  // Revolving intermediate buffers.
  NetworkScratch::IO buffer1(fwd_deltas, scratch);
  NetworkScratch::IO buffer2(fwd_deltas, scratch);
  // Run each network in reverse order, giving the back_deltas output of n as
  // the fwd_deltas input to n-1, with the 0 network providing the real output.
  if (!stack_.back()->IsTraining() ||
      !stack_.back()->Backward(debug, fwd_deltas, scratch, buffer1))
    return false;
  for (int i = stack_size - 2; i >= 0; i -= 2) {
    if (!stack_[i]->IsTraining() ||
        !stack_[i]->Backward(debug, *buffer1, scratch,
                             i > 0 ? buffer2 : back_deltas))
      return false;
    if (i == 0) return needs_to_backprop_;
    if (!stack_[i - 1]->IsTraining() ||
        !stack_[i - 1]->Backward(debug, *buffer2, scratch,
                                 i > 1 ? buffer1 : back_deltas))
      return false;
  }
  return needs_to_backprop_;
}

// Splits the series after the given index, returning the two parts and
// deletes itself. The first part, up to network with index last_start, goes
// into start, and the rest goes into end.
void Series::SplitAt(int last_start, Series** start, Series** end) {
  *start = nullptr;
  *end = nullptr;
  if (last_start < 0 || last_start >= stack_.size()) {
    tprintf("Invalid split index %d must be in range [0,%d]!\n",
            last_start, stack_.size() - 1);
    return;
  }
  Series* master_series = new Series("MasterSeries");
  Series* boosted_series = new Series("BoostedSeries");
  for (int s = 0; s <= last_start; ++s) {
    if (s + 1 == stack_.size() && stack_[s]->type() == NT_SOFTMAX) {
      // Change the softmax to a tanh.
      auto* fc = static_cast<FullyConnected*>(stack_[s]);
      fc->ChangeType(NT_TANH);
    }
    master_series->AddToStack(stack_[s]);
    stack_[s] = nullptr;
  }
  for (int s = last_start + 1; s < stack_.size(); ++s) {
    boosted_series->AddToStack(stack_[s]);
    stack_[s] = nullptr;
  }
  *start = master_series;
  *end = boosted_series;
  delete this;
}

// Appends the elements of the src series to this, removing from src and
// deleting it.
void Series::AppendSeries(Network* src) {
  ASSERT_HOST(src->type() == NT_SERIES);
  auto* src_series = static_cast<Series*>(src);
  for (int s = 0; s < src_series->stack_.size(); ++s) {
    AddToStack(src_series->stack_[s]);
    src_series->stack_[s] = nullptr;
  }
  delete src;
}


}  // namespace tesseract.
