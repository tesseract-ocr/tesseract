///////////////////////////////////////////////////////////////////////
// File:        dotproductavx.cpp
// Description: Architecture-specific dot-product function.
// Author:      Ray Smith
// Created:     Wed Jul 22 10:48:05 PDT 2015
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

#if !defined(__AVX__)
// Implementation for non-avx archs.

#include "dotproductavx.h"
#include <stdio.h>
#include <stdlib.h>

namespace tesseract {
double DotProductAVX(const double* u, const double* v, int n) {
  fprintf(stderr, "DotProductAVX can't be used on Android\n");
  abort();
}
}  // namespace tesseract

#else  // !defined(__AVX__)
// Implementation for avx capable archs.
#include <immintrin.h>
#include <stdint.h>
#include "dotproductavx.h"
#include "host.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
// Uses Intel AVX intrinsics to access the SIMD instruction set.
double DotProductAVX(const double* u, const double* v, int n) {
  int max_offset = n - 7;
  int offset = 0;
  // Accumulate a set of 4 sums in sum, by loading pairs of 4 values from u and
  // v, and multiplying them together in parallel.
  __m256d sum = _mm256_setzero_pd();
  if (max_offset > 0) {
    // Aligned load is reputedly faster but requires 32 byte aligned input.
    if ((reinterpret_cast<const uintptr_t>(u) & 31) == 0 &&
        (reinterpret_cast<const uintptr_t>(v) & 31) == 0) {
      do {
        // Use aligned load.
        __m256d floats1 = _mm256_load_pd(u + offset);
        __m256d floats2 = _mm256_load_pd(v + offset);
        // Multiply.
        __m256d product = _mm256_mul_pd(floats1, floats2);
        sum = _mm256_add_pd(sum, product);
        offset += 4;
        floats1 = _mm256_load_pd(u + offset);
        floats2 = _mm256_load_pd(v + offset);
        product = _mm256_mul_pd(floats1, floats2);
        sum = _mm256_add_pd(sum, product);
        offset += 4;
      } while (offset < max_offset);
    } else {
      do {
        // Use unaligned load.
        __m256d floats1 = _mm256_loadu_pd(u + offset);
        __m256d floats2 = _mm256_loadu_pd(v + offset);
        // Multiply.
        __m256d product = _mm256_mul_pd(floats1, floats2);
        sum = _mm256_add_pd(sum, product);
        offset += 4;
        floats1 = _mm256_loadu_pd(u + offset);
        floats2 = _mm256_loadu_pd(v + offset);
        product = _mm256_mul_pd(floats1, floats2);
        sum = _mm256_add_pd(sum, product);
        offset += 4;
      } while (offset < max_offset);
    }
  }
  double tmp[4];
  _mm256_store_pd(&tmp[0], sum);
  double result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
  while (offset < n) {
    result += u[offset] * v[offset];
    ++offset;
  }
  return result;
}

}  // namespace tesseract.

#endif  // ANDROID_BUILD
