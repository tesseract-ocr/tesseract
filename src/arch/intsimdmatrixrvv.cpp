///////////////////////////////////////////////////////////////////////
// File:        intsimdmatrixrvv.cpp
// Description: matrix-vector product for 8-bit data on rvv.
// Author:      sunyuechi
//
// Copyright (c) 2024 Institute of Software Chinese Academy of Sciences (ISCAS).
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // for HAVE_RVV, ...
#endif

#if HAVE_RVV
#  include "intsimdmatrix.h"
#  include "tesstypes.h"

namespace tesseract {

static int DotProduct(const int8_t *u, const int8_t *v, int num) {
  int total = 0;

  asm __volatile__ (
    "  .option       arch, +v                   \n\t"
    "  vsetvli t0,zero,e32,m8,ta,ma             \n\t"
    "  vmv.v.i v0,0                             \n\t"
    "1:                                         \n\t"
    "  vsetvli t0,%[num],e8,m2,ta,ma            \n\t"
    "  vle8.v v16,0(%[u])                       \n\t"
    "  vle8.v v24,0(%[v])                       \n\t"
    "  sub %[num],%[num],t0                     \n\t"
    "  vwmul.vv v8,v24,v16                      \n\t"
    "  add %[u],%[u],t0                         \n\t"
    "  add %[v],%[v],t0                         \n\t"
    "  vsetvli zero,zero,e16,m4,tu,ma           \n\t"
    "  vwadd.wv v0,v0,v8                        \n\t"
    "  bnez %[num],1b                           \n\t"
    "  vsetvli t0,zero,e32,m8,ta,ma             \n\t"
    "  vmv.s.x v8,zero                          \n\t"
    "  vredsum.vs v0,v0,v8                      \n\t"
    "  vmv.x.s %[total],v0                      \n\t"
    :  [u] "+r" (u),
       [v] "+r" (v),
       [num] "+r" (num),
       [total] "+r" (total)
    :
    :  "cc", "memory"
  );

  return total;
}

static void matrixDotVector(int dim1, int dim2, const int8_t *wi, const TFloat *scales,
                            const int8_t *u, TFloat *v) {
  int num_out = dim1;
  int num_in = dim2 - 1;
  for (int i = 0; i < num_out; ++i) {
    const int8_t *wi_start = wi + i * dim2;
    int total = DotProduct(wi_start, u, num_in);
    // Add in the bias and apply scaling.
    v[i] = (total + wi_start[num_in] * INT8_MAX) * scales[i];
  }
}

const IntSimdMatrix IntSimdMatrix::intSimdMatrixRVV = {
    // Function.
    matrixDotVector,
    // Number of 32 bit outputs held in each register.
    1,
    // Maximum number of registers that we will use to hold outputs.
    1,
    // Number of 8 bit inputs in the inputs register.
    1,
    // Number of inputs in each weight group.
    1
};

} // namespace tesseract.

#endif /* HAVE_RVV */
