///////////////////////////////////////////////////////////////////////
// File:        dotproductsse.cpp
// Description: Architecture-specific dot-product function.
// Author:      Ray Smith
//
// (C) Copyright 2015, Google Inc.
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
#  if defined(__i686__) || defined(__x86_64__)
#    error Implementation only for SSE 4.1 capable architectures
#  endif
#else

#  include <emmintrin.h>
#  include <smmintrin.h>
#  include <cstdint>
#  include "dotproduct.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
// Uses Intel SSE intrinsics to access the SIMD instruction set.
#if defined(FAST_FLOAT)
float DotProductSSE(const float *u, const float *v, int n) {
  int max_offset = n - 4;
  int offset = 0;
  // Accumulate a set of 4 sums in sum, by loading pairs of 4 values from u and
  // v, and multiplying them together in parallel.
  __m128 sum = _mm_setzero_ps();
  if (offset <= max_offset) {
    offset = 4;
    // Aligned load is reputedly faster but requires 16 byte aligned input.
    if ((reinterpret_cast<uintptr_t>(u) & 15) == 0 &&
        (reinterpret_cast<uintptr_t>(v) & 15) == 0) {
      // Use aligned load.
      sum = _mm_load_ps(u);
      __m128 floats2 = _mm_load_ps(v);
      // Multiply.
      sum = _mm_mul_ps(sum, floats2);
      while (offset <= max_offset) {
        __m128 floats1 = _mm_load_ps(u + offset);
        floats2 = _mm_load_ps(v + offset);
        floats1 = _mm_mul_ps(floats1, floats2);
        sum = _mm_add_ps(sum, floats1);
        offset += 4;
      }
    } else {
      // Use unaligned load.
      sum = _mm_loadu_ps(u);
      __m128 floats2 = _mm_loadu_ps(v);
      // Multiply.
      sum = _mm_mul_ps(sum, floats2);
      while (offset <= max_offset) {
        __m128 floats1 = _mm_loadu_ps(u + offset);
        floats2 = _mm_loadu_ps(v + offset);
        floats1 = _mm_mul_ps(floats1, floats2);
        sum = _mm_add_ps(sum, floats1);
        offset += 4;
      }
    }
  }
  // Add the 4 sums in sum horizontally.
#if 0
  alignas(32) float tmp[4];
  _mm_store_ps(tmp, sum);
  float result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
#else
  __m128 zero = _mm_setzero_ps();
  // https://www.felixcloutier.com/x86/haddps
  sum = _mm_hadd_ps(sum, zero);
  sum = _mm_hadd_ps(sum, zero);
  // Extract the low result.
  float result = _mm_cvtss_f32(sum);
#endif
  // Add on any left-over products.
  while (offset < n) {
    result += u[offset] * v[offset];
    ++offset;
  }
  return result;
}
#else
double DotProductSSE(const double *u, const double *v, int n) {
  int max_offset = n - 2;
  int offset = 0;
  // Accumulate a set of 2 sums in sum, by loading pairs of 2 values from u and
  // v, and multiplying them together in parallel.
  __m128d sum = _mm_setzero_pd();
  if (offset <= max_offset) {
    offset = 2;
    // Aligned load is reputedly faster but requires 16 byte aligned input.
    if ((reinterpret_cast<uintptr_t>(u) & 15) == 0 &&
        (reinterpret_cast<uintptr_t>(v) & 15) == 0) {
      // Use aligned load.
      sum = _mm_load_pd(u);
      __m128d floats2 = _mm_load_pd(v);
      // Multiply.
      sum = _mm_mul_pd(sum, floats2);
      while (offset <= max_offset) {
        __m128d floats1 = _mm_load_pd(u + offset);
        floats2 = _mm_load_pd(v + offset);
        offset += 2;
        floats1 = _mm_mul_pd(floats1, floats2);
        sum = _mm_add_pd(sum, floats1);
      }
    } else {
      // Use unaligned load.
      sum = _mm_loadu_pd(u);
      __m128d floats2 = _mm_loadu_pd(v);
      // Multiply.
      sum = _mm_mul_pd(sum, floats2);
      while (offset <= max_offset) {
        __m128d floats1 = _mm_loadu_pd(u + offset);
        floats2 = _mm_loadu_pd(v + offset);
        offset += 2;
        floats1 = _mm_mul_pd(floats1, floats2);
        sum = _mm_add_pd(sum, floats1);
      }
    }
  }
  // Add the 2 sums in sum horizontally.
  sum = _mm_hadd_pd(sum, sum);
  // Extract the low result.
  double result = _mm_cvtsd_f64(sum);
  // Add on any left-over products.
  while (offset < n) {
    result += u[offset] * v[offset];
    ++offset;
  }
  return result;
}
#endif

} // namespace tesseract.

#endif
