/////////////////////////////////////////////////////////////////////////
// File:        parallel.cpp
// Description: Runs networks in parallel on the same input.
// Author:      Ray Smith
// Created:     Thu May 02 08:06:06 PST 2013
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

#include "parallel.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include "functions.h"  // For conditional undef of _OPENMP.
#include "networkscratch.h"

namespace tesseract {

// ni_ and no_ will be set by AddToStack.
Parallel::Parallel(const STRING& name, NetworkType type) : Plumbing(name) {
  type_ = type;
}

// Returns the shape output from the network given an input shape (which may
// be partially unknown ie zero).
StaticShape Parallel::OutputShape(const StaticShape& input_shape) const {
  StaticShape result = stack_[0]->OutputShape(input_shape);
  int stack_size = stack_.size();
  for (int i = 1; i < stack_size; ++i) {
    StaticShape shape = stack_[i]->OutputShape(input_shape);
    result.set_depth(result.depth() + shape.depth());
  }
  return result;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void Parallel::Forward(bool debug, const NetworkIO& input,
                       const TransposedArray* input_transpose,
                       NetworkScratch* scratch, NetworkIO* output) {
  bool parallel_debug = false;
  // If this parallel is a replicator of convolvers, or holds a 1-d LSTM pair,
  // or a 2-d LSTM quad, do debug locally, and don't pass the flag on.
  if (debug && type_ != NT_PARALLEL) {
    parallel_debug = true;
    debug = false;
  }
  int stack_size = stack_.size();
  if (type_ == NT_PAR_2D_LSTM) {
    // Special case, run parallel in parallel.
    GenericVector<NetworkScratch::IO> results;
    results.init_to_size(stack_size, NetworkScratch::IO());
    for (int i = 0; i < stack_size; ++i) {
      results[i].Resize(input, stack_[i]->NumOutputs(), scratch);
    }
#ifdef _OPENMP
#pragma omp parallel for num_threads(stack_size)
#endif
    for (int i = 0; i < stack_size; ++i) {
      stack_[i]->Forward(debug, input, nullptr, scratch, results[i]);
    }
    // Now pack all the results (serially) into the output.
    int out_offset = 0;
    output->Resize(*results[0], NumOutputs());
    for (int i = 0; i < stack_size; ++i) {
      out_offset = output->CopyPacking(*results[i], out_offset);
    }
  } else {
    // Revolving intermediate result.
    NetworkScratch::IO result(input, scratch);
    // Source for divided replicated.
    NetworkScratch::IO source_part;
    TransposedArray* src_transpose = nullptr;
    if (IsTraining() && type_ == NT_REPLICATED) {
      // Make a transposed copy of the input.
      input.Transpose(&transposed_input_);
      src_transpose = &transposed_input_;
    }
    // Run each network, putting the outputs into result.
    int out_offset = 0;
    for (int i = 0; i < stack_size; ++i) {
      stack_[i]->Forward(debug, input, src_transpose, scratch, result);
      // All networks must have the same output width
      if (i == 0) {
        output->Resize(*result, NumOutputs());
      } else {
        ASSERT_HOST(result->Width() == output->Width());
      }
      out_offset = output->CopyPacking(*result, out_offset);
    }
  }
  if (parallel_debug) {
    DisplayForward(*output);
  }
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Parallel::Backward(bool debug, const NetworkIO& fwd_deltas,
                        NetworkScratch* scratch,
                        NetworkIO* back_deltas) {
  // If this parallel is a replicator of convolvers, or holds a 1-d LSTM pair,
  // or a 2-d LSTM quad, do debug locally, and don't pass the flag on.
  if (debug && type_ != NT_PARALLEL) {
    DisplayBackward(fwd_deltas);
    debug = false;
  }
  int stack_size = stack_.size();
  if (type_ == NT_PAR_2D_LSTM) {
    // Special case, run parallel in parallel.
    GenericVector<NetworkScratch::IO> in_deltas, out_deltas;
    in_deltas.init_to_size(stack_size, NetworkScratch::IO());
    out_deltas.init_to_size(stack_size, NetworkScratch::IO());
    // Split the forward deltas for each stack element.
    int feature_offset = 0;
    for (int i = 0; i < stack_.size(); ++i) {
      int num_features = stack_[i]->NumOutputs();
      in_deltas[i].Resize(fwd_deltas, num_features, scratch);
      out_deltas[i].Resize(fwd_deltas, stack_[i]->NumInputs(), scratch);
      in_deltas[i]->CopyUnpacking(fwd_deltas, feature_offset, num_features);
      feature_offset += num_features;
    }
#ifdef _OPENMP
#pragma omp parallel for num_threads(stack_size)
#endif
    for (int i = 0; i < stack_size; ++i) {
      stack_[i]->Backward(debug, *in_deltas[i], scratch,
                          i == 0 ? back_deltas : out_deltas[i]);
    }
    if (needs_to_backprop_) {
      for (int i = 1; i < stack_size; ++i) {
        back_deltas->AddAllToFloat(*out_deltas[i]);
      }
    }
  } else {
    // Revolving partial deltas.
    NetworkScratch::IO in_deltas(fwd_deltas, scratch);
    // The sum of deltas from different sources, which will eventually go into
    // back_deltas.
    NetworkScratch::IO out_deltas;
    int feature_offset = 0;
    for (int i = 0; i < stack_.size(); ++i) {
      int num_features = stack_[i]->NumOutputs();
      in_deltas->CopyUnpacking(fwd_deltas, feature_offset, num_features);
      feature_offset += num_features;
      if (stack_[i]->Backward(debug, *in_deltas, scratch, back_deltas)) {
        if (i == 0) {
          out_deltas.ResizeFloat(*back_deltas, back_deltas->NumFeatures(),
                                 scratch);
          out_deltas->CopyAll(*back_deltas);
        } else if (back_deltas->NumFeatures() == out_deltas->NumFeatures()) {
          // Widths are allowed to be different going back, as we may have
          // input nets, so only accumulate the deltas if the widths are the
          // same.
          out_deltas->AddAllToFloat(*back_deltas);
        }
      }
    }
    if (needs_to_backprop_) back_deltas->CopyAll(*out_deltas);
  }
  if (needs_to_backprop_) back_deltas->ScaleFloatBy(1.0f / stack_size);
  return needs_to_backprop_;
}

}  // namespace tesseract.
