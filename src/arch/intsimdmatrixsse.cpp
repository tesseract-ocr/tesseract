///////////////////////////////////////////////////////////////////////
// File:        intsindmatrixsse.cpp
// Description: SSE implementation of 8-bit int SIMD matrix multiply.
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

#if !defined(__SSE4_1__)
#error Implementation only for SSE 4.1 capable architectures
#endif

#include "intsimdmatrix.h"

#include <cstdint>
#include "dotproductsse.h"

namespace tesseract {

// Computes part of matrix.vector v = Wu. Computes 1 result.
static void PartialMatrixDotVector1(const int8_t* wi, const double* scales,
                                    const int8_t* u, int num_in, int num_out,
                                    double* v) {
  double total = IntDotProductSSE(u, wi, num_in);
  // Add in the bias and correct for integer values.
  *v = (total / INT8_MAX + wi[num_in]) * *scales;
}

static void matrixDotVector(int dim1, int dim2, const int8_t* wi,
                            const double* scales, const int8_t* u, double* v) {
  const int num_out = dim1;
  const int num_in = dim2 - 1;
  int output = 0;

  for (; output + 1 <= num_out; output += 1) {
    PartialMatrixDotVector1(wi, scales, u, num_in, num_out - output, v);
    wi += dim2;
    scales += 1;
    v += 1;
  }
}

const IntSimdMatrix IntSimdMatrix::intSimdMatrixSSE = {
  // Number of 32 bit outputs held in each register.
  1,
  // Maximum number of registers that we will use to hold outputs.
  1,
  // Number of 8 bit inputs in the inputs register.
  1,
  // Number of inputs in each weight group.
  1,
  // Number of groups of inputs to be broadcast.
  1,
  matrixDotVector
};

}  // namespace tesseract.
