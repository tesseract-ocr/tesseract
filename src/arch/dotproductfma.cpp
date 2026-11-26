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
#  include <cstddef> // for size_t
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

// Loads the first rem (< 8) elements of p. The other elements of the result
// are 0, and the memory behind the first rem elements is not accessed.
static inline __m256 LoadTail(const float *p, int rem) {
  alignas(32) static const int32_t kMask[16] = {-1, -1, -1, -1, -1, -1, -1, -1,
                                                0,  0,  0,  0,  0,  0,  0,  0};
  return _mm256_maskload_ps(
      p, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(kMask + 8 - rem)));
}

// Returns the sums of the elements of a, b, c and d. This is faster than
// four single sums.
static inline __m128 Sum4(__m256 a, __m256 b, __m256 c, __m256 d) {
  auto half = [](__m256 x) {
    return _mm_add_ps(_mm256_castps256_ps128(x), _mm256_extractf128_ps(x, 1));
  };
  __m128 ab = _mm_hadd_ps(half(a), half(b));
  __m128 cd = _mm_hadd_ps(half(c), half(d));
  return _mm_hadd_ps(ab, cd);
}

// Computes v[i] = w[i] . u for num_rows rows of w with n used elements each.
// Four rows are handled per pass, so u is loaded only once for them and
// eight independent sums are calculated at the same time.
void RowDotProductsFMA(const float *w, int w_stride, int num_rows,
                       const float *u, int n, float *v) {
  int i = 0;
  for (; i + 4 <= num_rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    __m256 a0 = _mm256_setzero_ps();
    __m256 a1 = a0, a2 = a0, a3 = a0;
    __m256 b0 = a0, b1 = a0, b2 = a0, b3 = a0;
    int j = 0;
    for (; j + 16 <= n; j += 16) {
      __m256 x = _mm256_loadu_ps(u + j);
      __m256 y = _mm256_loadu_ps(u + j + 8);
      a0 = _mm256_fmadd_ps(_mm256_loadu_ps(w0 + j), x, a0);
      b0 = _mm256_fmadd_ps(_mm256_loadu_ps(w0 + j + 8), y, b0);
      a1 = _mm256_fmadd_ps(_mm256_loadu_ps(w1 + j), x, a1);
      b1 = _mm256_fmadd_ps(_mm256_loadu_ps(w1 + j + 8), y, b1);
      a2 = _mm256_fmadd_ps(_mm256_loadu_ps(w2 + j), x, a2);
      b2 = _mm256_fmadd_ps(_mm256_loadu_ps(w2 + j + 8), y, b2);
      a3 = _mm256_fmadd_ps(_mm256_loadu_ps(w3 + j), x, a3);
      b3 = _mm256_fmadd_ps(_mm256_loadu_ps(w3 + j + 8), y, b3);
    }
    if (j + 8 <= n) {
      __m256 x = _mm256_loadu_ps(u + j);
      a0 = _mm256_fmadd_ps(_mm256_loadu_ps(w0 + j), x, a0);
      a1 = _mm256_fmadd_ps(_mm256_loadu_ps(w1 + j), x, a1);
      a2 = _mm256_fmadd_ps(_mm256_loadu_ps(w2 + j), x, a2);
      a3 = _mm256_fmadd_ps(_mm256_loadu_ps(w3 + j), x, a3);
      j += 8;
    }
    if (j < n) {
      const int rem = n - j;
      __m256 x = LoadTail(u + j, rem);
      b0 = _mm256_fmadd_ps(LoadTail(w0 + j, rem), x, b0);
      b1 = _mm256_fmadd_ps(LoadTail(w1 + j, rem), x, b1);
      b2 = _mm256_fmadd_ps(LoadTail(w2 + j, rem), x, b2);
      b3 = _mm256_fmadd_ps(LoadTail(w3 + j, rem), x, b3);
    }
    _mm_storeu_ps(v + i, Sum4(_mm256_add_ps(a0, b0), _mm256_add_ps(a1, b1),
                              _mm256_add_ps(a2, b2), _mm256_add_ps(a3, b3)));
  }
  for (; i < num_rows; i++) {
    v[i] = DotProductFMA(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

// Computes out[i * out_stride + j] = u[i] . v[j] for the num_u rows of u and
// the num_v rows of v, which all have n elements and are stored without gaps.
// Four rows of u and two rows of v are handled per pass, so each of them is
// loaded only once for 8 sums which are calculated at the same time. Larger
// blocks need more than the 16 registers which exist and are slower.
void CrossDotProductsFMA(const float *u, int num_u, const float *v, int num_v,
                         int n, float *out, int out_stride) {
  int i = 0;
  for (; i + 4 <= num_u; i += 4) {
    const float *u0 = u + static_cast<size_t>(i) * n;
    const float *u1 = u0 + n;
    const float *u2 = u1 + n;
    const float *u3 = u2 + n;
    float *out0 = out + static_cast<size_t>(i) * out_stride;
    float *out1 = out0 + out_stride;
    float *out2 = out1 + out_stride;
    float *out3 = out2 + out_stride;
    int j = 0;
    for (; j + 2 <= num_v; j += 2) {
      const float *v0 = v + static_cast<size_t>(j) * n;
      const float *v1 = v0 + n;
      __m256 a0 = _mm256_setzero_ps();
      __m256 a1 = a0, b0 = a0, b1 = a0;
      __m256 c0 = a0, c1 = a0, d0 = a0, d1 = a0;
      int k = 0;
      for (; k + 8 <= n; k += 8) {
        __m256 y0 = _mm256_loadu_ps(v0 + k);
        __m256 y1 = _mm256_loadu_ps(v1 + k);
        __m256 x = _mm256_loadu_ps(u0 + k);
        a0 = _mm256_fmadd_ps(x, y0, a0);
        a1 = _mm256_fmadd_ps(x, y1, a1);
        x = _mm256_loadu_ps(u1 + k);
        b0 = _mm256_fmadd_ps(x, y0, b0);
        b1 = _mm256_fmadd_ps(x, y1, b1);
        x = _mm256_loadu_ps(u2 + k);
        c0 = _mm256_fmadd_ps(x, y0, c0);
        c1 = _mm256_fmadd_ps(x, y1, c1);
        x = _mm256_loadu_ps(u3 + k);
        d0 = _mm256_fmadd_ps(x, y0, d0);
        d1 = _mm256_fmadd_ps(x, y1, d1);
      }
      if (k < n) {
        const int rem = n - k;
        __m256 y0 = LoadTail(v0 + k, rem);
        __m256 y1 = LoadTail(v1 + k, rem);
        __m256 x = LoadTail(u0 + k, rem);
        a0 = _mm256_fmadd_ps(x, y0, a0);
        a1 = _mm256_fmadd_ps(x, y1, a1);
        x = LoadTail(u1 + k, rem);
        b0 = _mm256_fmadd_ps(x, y0, b0);
        b1 = _mm256_fmadd_ps(x, y1, b1);
        x = LoadTail(u2 + k, rem);
        c0 = _mm256_fmadd_ps(x, y0, c0);
        c1 = _mm256_fmadd_ps(x, y1, c1);
        x = LoadTail(u3 + k, rem);
        d0 = _mm256_fmadd_ps(x, y0, d0);
        d1 = _mm256_fmadd_ps(x, y1, d1);
      }
      // The sums for two rows of u are stored with one call.
      __m128 ab = Sum4(a0, a1, b0, b1);
      __m128 cd = Sum4(c0, c1, d0, d1);
      _mm_storel_pi(reinterpret_cast<__m64 *>(out0 + j), ab);
      _mm_storeh_pi(reinterpret_cast<__m64 *>(out1 + j), ab);
      _mm_storel_pi(reinterpret_cast<__m64 *>(out2 + j), cd);
      _mm_storeh_pi(reinterpret_cast<__m64 *>(out3 + j), cd);
    }
    if (j < num_v) {
      // The last row of v is a single vector for the four rows of u.
      float last[4];
      RowDotProductsFMA(u0, n, 4, v + static_cast<size_t>(j) * n, n, last);
      out0[j] = last[0];
      out1[j] = last[1];
      out2[j] = last[2];
      out3[j] = last[3];
    }
  }
  // The remaining rows of u are single vectors for the rows of v.
  for (; i < num_u; i++) {
    RowDotProductsFMA(v, n, num_v, u + static_cast<size_t>(i) * n, n,
                      out + static_cast<size_t>(i) * out_stride);
  }
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
