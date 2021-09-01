///////////////////////////////////////////////////////////////////////
// File:        dotproductneon.cpp
// Description: Dot product function for ARM NEON.
// Author:      Stefan Weil
//
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

#if defined(__ARM_NEON)

#include <arm_neon.h>
#include "dotproduct.h"

namespace tesseract {

// Documentation:
// https://developer.arm.com/architectures/instruction-sets/intrinsics/

#if defined(FAST_FLOAT) && defined(__ARM_ARCH_ISA_A64)

float DotProductNEON(const float *u, const float *v, int n) {
  float32x4_t result0123 = vdupq_n_f32(0.0f);
  float32x4_t result4567 = vdupq_n_f32(0.0f);
  while (n > 7) {
    // Calculate 8 dot products per iteration.
    float32x4_t u0 = vld1q_f32(u);
    float32x4_t v0 = vld1q_f32(v);
    float32x4_t u4 = vld1q_f32(u + 4);
    float32x4_t v4 = vld1q_f32(v + 4);
    result0123 = vfmaq_f32(result0123, u0, v0);
    result4567 = vfmaq_f32(result4567, u4, v4);
    u += 8;
    v += 8;
    n -= 8;
  }
  float total = vaddvq_f32(result0123);
  total += vaddvq_f32(result4567);
  while (n > 0) {
    total += *u++ * *v++;
    n--;
  }
  return total;
}

#else

// Computes and returns the dot product of the two n-vectors u and v.
TFloat DotProductNEON(const TFloat *u, const TFloat *v, int n) {
  TFloat total = 0;
#if defined(OPENMP_SIMD) || defined(_OPENMP)
#pragma omp simd reduction(+:total)
#endif
  for (int k = 0; k < n; k++) {
    total += u[k] * v[k];
  }
  return total;
}

#endif

} // namespace tesseract

#endif /* __ARM_NEON */
