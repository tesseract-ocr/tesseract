///////////////////////////////////////////////////////////////////////
// File:        dotproductfma.cpp
// Description: Architecture-specific dot-product function.
// Author:      Stefan Weil
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

#if !defined(__FMA__)
#  if defined(__i686__) || defined(__x86_64__)
#    error Implementation only for FMA capable architectures
#  endif
#else

#  include <immintrin.h>
#  include <cstdint>
#  include "dotproduct.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
// Uses Intel FMA intrinsics to access the SIMD instruction set.
#if defined(FAST_FLOAT)
float DotProductFMA(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m256 t0 = _mm256_setzero_ps();
  __m256 t1 = _mm256_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    __m256 f0 = _mm256_loadu_ps(u);
    __m256 f1 = _mm256_loadu_ps(v);
    t0 = _mm256_fmadd_ps(f0, f1, t0);
    u += 8;
    v += 8;
    __m256 f2 = _mm256_loadu_ps(u);
    __m256 f3 = _mm256_loadu_ps(v);
    t1 = _mm256_fmadd_ps(f2, f3, t1);
    u += 8;
    v += 8;
  }
  t0 = _mm256_hadd_ps(t0, t1);
  alignas(32) float tmp[8];
  _mm256_store_ps(tmp, t0);
  float result = tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}
#else
double DotProductFMA(const double *u, const double *v, int n) {
  const unsigned quot = n / 8;
  const unsigned rem = n % 8;
  __m256d t0 = _mm256_setzero_pd();
  __m256d t1 = _mm256_setzero_pd();
  for (unsigned k = 0; k < quot; k++) {
    __m256d f0 = _mm256_loadu_pd(u);
    __m256d f1 = _mm256_loadu_pd(v);
    t0 = _mm256_fmadd_pd(f0, f1, t0);
    u += 4;
    v += 4;
    __m256d f2 = _mm256_loadu_pd(u);
    __m256d f3 = _mm256_loadu_pd(v);
    t1 = _mm256_fmadd_pd(f2, f3, t1);
    u += 4;
    v += 4;
  }
  t0 = _mm256_hadd_pd(t0, t1);
  alignas(32) double tmp[4];
  _mm256_store_pd(tmp, t0);
  double result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}
#endif

} // namespace tesseract.

#endif
