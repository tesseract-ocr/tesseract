///////////////////////////////////////////////////////////////////////
// File:        intsimdmatrix.h
// Description: Base class for 8-bit int SIMD matrix multipliers.
// Author:      Ray Smith
//
// (C) Copyright 2017, Google Inc.
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

#ifndef TESSERACT_ARCH_INTSIMDMATRIX_H_
#define TESSERACT_ARCH_INTSIMDMATRIX_H_

#include <tesseract/export.h>

#include <cstdint>
#include <vector>

#include "tesstypes.h"

namespace tesseract {

template <class T>
class GENERIC_2D_ARRAY;

// Base class for a SIMD function to multiply a matrix by a vector, with sources
// of 8-bit signed integer, and result in a double, after appropriate scaling.
// Assumes a specific method of multiplication that can be applied to any size
// and number of SIMD registers as follows:
// int32_t results are computed with num_outputs_per_register_ in each of
// max_output_registers_ result registers, repeatedly until it would make too
// many results, then the number of registers is halved, and so-on down to a
// single result register. The last calculation only outputs the required number
// of results instead of writing beyond the bounds. Eg: matrix has 75 outputs,
//  num_outputs_per_register_ = 4, and max_output_registers_ = 8,
// Step 1: 8x4=32 results are computed,
// Step 2: 8x4=32 again, total 64,
// Step 3: 2x4=8 (since 8x4 is too many, so is 4x4), total 72,
// Step 4: 1x3, total 75.
// Each step above is computed using a PartialFunc, which runs over the input
// vector once. The input is read one registerful of num_inputs_per_register_
// at a time (presumably 4x num_outputs_per_register_ since they are int8_t)
// so the inputs MUST BE PADDED to a multiple of num_inputs_per_register_.
// Since it is slow (on Intel at least) to horizontally add in a register,
// provision is made to process num_inputs_per_group_ inputs at a time, with
// the group being replicated num_input_groups_ times and multiplied by a
// num_inputs_per_group_ by num_input_groups_ rectangle of the weights matrix.
// This is most convenient if num_inputs_per_group_ is 4, and the product
// sign-extends and sums 8x8=16 bit results to 32 bits, adding 4 adjacent
// results in the process, but it doesn't have to be implemented that way.
// The weights are re-ordered by Init() to be used sequentially by the above
// algorithm, followed by the biases, so they can be added at the end.
// The base class computes the base C++ implementation.
// NOTE that, although the subclasses execute on different SIMD hardware, no
// virtual methods are needed, as the constructor sets up everything that
// is required to allow the base class implementation to do all the work.
struct TESS_API IntSimdMatrix {
  // Computes a reshaped copy of the weight matrix w.
  void Init(const GENERIC_2D_ARRAY<int8_t> &w, std::vector<int8_t> &shaped_w,
            int32_t &rounded_num_out) const;

  // Rounds the size up to a multiple of the input register size (in int8_t).
  int RoundInputs(int size) const {
    return Roundup(size, num_inputs_per_register_);
  }
  // Rounds the size up to a multiple of the output register size (in int32_t).
  int RoundOutputs(int size) const {
    return Roundup(size, num_outputs_per_register_);
  }

  // Computes matrix.vector v = Wu.
  // u is of size W.dim2() - 1 and the output v is of size W.dim1().
  // u is imagined to have an extra element at the end with value 1, to
  // implement the bias, but it doesn't actually have it.
  // Computes the base C++ implementation.
  static void MatrixDotVector(const GENERIC_2D_ARRAY<int8_t> &w, const std::vector<TFloat> &scales,
                              const int8_t *u, TFloat *v);

  // Rounds the input up to a multiple of the given factor.
  static int Roundup(int input, int factor) {
    return (input + factor - 1) / factor * factor;
  }

  // Computes matrix.vector v = Wu.
  // u is of size W.dim2() - 1 and the output v is of size W.dim1().
  // u is imagined to have an extra element at the end with value 1, to
  // implement the bias, but it doesn't actually have it.
  // Uses an optimized implementation with partial funcs.
  // NOTE: The size of the input vector (u) must be padded using
  // RoundInputs above.
  // The input will be over-read to the extent of the padding. There are no
  // alignment requirements.
  using MatrixDotVectorFunction = void (*)(int, int, const int8_t *, const TFloat *, const int8_t *,
                                           TFloat *);
  MatrixDotVectorFunction matrixDotVectorFunction;

  // Number of 32 bit outputs held in each register.
  int num_outputs_per_register_;
  // Maximum number of registers that we will use to hold outputs.
  int max_output_registers_;
  // Number of 8 bit inputs in the inputs register.
  int num_inputs_per_register_;
  // Number of inputs in each weight group.
  int num_inputs_per_group_;
  // Number of groups of inputs to be broadcast.
  // num_input_groups_ = num_inputs_per_register_ / num_inputs_per_group_

  static const IntSimdMatrix *intSimdMatrix;
  // Only available with NEON.
  static const IntSimdMatrix intSimdMatrixNEON;
  // Only available with AVX2 / AVX / FMA / SSE.
  static const IntSimdMatrix intSimdMatrixAVX2;
  static const IntSimdMatrix intSimdMatrixSSE;
};

} // namespace tesseract

#endif // TESSERACT_ARCH_INTSIMDMATRIX_H_
