///////////////////////////////////////////////////////////////////////
// File:        dotproductavx512.cpp
// Description: Architecture-specific dot-product function.
// Author:      Stefan Weil
//
// (C) Copyright 2022
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
#  if defined(__i686__) || defined(__x86_64__)
#    error Implementation only for AVX capable architectures
#  endif
#else

#  include <immintrin.h>
#  include <cstddef> // for size_t
#  include <cstdint>
#  include "dotproduct.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
// Uses Intel AVX intrinsics to access the SIMD instruction set.
#  if defined(FAST_FLOAT)
float DotProductAVX512F(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m512 t0 = _mm512_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    __m512 f0 = _mm512_loadu_ps(u);
    __m512 f1 = _mm512_loadu_ps(v);
    t0 = _mm512_fmadd_ps(f0, f1, t0);
    u += 16;
    v += 16;
  }
  float result = _mm512_reduce_add_ps(t0);
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

// Loads the first rem (< 16) elements of p. The other elements of the result
// are 0, and the memory behind the first rem elements is not accessed.
static inline __m512 LoadTail(const float *p, int rem) {
  return _mm512_maskz_loadu_ps(static_cast<__mmask16>((1u << rem) - 1), p);
}

// Returns the sums of the elements of a, b, c and d. This is much faster
// than four calls of _mm512_reduce_add_ps.
static inline __m128 Sum4(__m512 a, __m512 b, __m512 c, __m512 d) {
  auto half = [](__m512 x) {
    __m256 y = _mm256_add_ps(
        _mm512_castps512_ps256(x),
        _mm256_castpd_ps(_mm512_extractf64x4_pd(_mm512_castps_pd(x), 1)));
    return _mm_add_ps(_mm256_castps256_ps128(y), _mm256_extractf128_ps(y, 1));
  };
  __m128 ab = _mm_hadd_ps(half(a), half(b));
  __m128 cd = _mm_hadd_ps(half(c), half(d));
  return _mm_hadd_ps(ab, cd);
}

// Computes v[i] = w[i] . u for num_rows rows of w with n used elements each.
// Four rows are handled per pass, so u is loaded only once for them and
// eight independent sums are calculated at the same time.
void RowDotProductsAVX512F(const float *w, int w_stride, int num_rows,
                           const float *u, int n, float *v) {
  int i = 0;
  for (; i + 4 <= num_rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    __m512 a0 = _mm512_setzero_ps();
    __m512 a1 = a0, a2 = a0, a3 = a0;
    __m512 b0 = a0, b1 = a0, b2 = a0, b3 = a0;
    int j = 0;
    for (; j + 32 <= n; j += 32) {
      __m512 x = _mm512_loadu_ps(u + j);
      __m512 y = _mm512_loadu_ps(u + j + 16);
      a0 = _mm512_fmadd_ps(_mm512_loadu_ps(w0 + j), x, a0);
      b0 = _mm512_fmadd_ps(_mm512_loadu_ps(w0 + j + 16), y, b0);
      a1 = _mm512_fmadd_ps(_mm512_loadu_ps(w1 + j), x, a1);
      b1 = _mm512_fmadd_ps(_mm512_loadu_ps(w1 + j + 16), y, b1);
      a2 = _mm512_fmadd_ps(_mm512_loadu_ps(w2 + j), x, a2);
      b2 = _mm512_fmadd_ps(_mm512_loadu_ps(w2 + j + 16), y, b2);
      a3 = _mm512_fmadd_ps(_mm512_loadu_ps(w3 + j), x, a3);
      b3 = _mm512_fmadd_ps(_mm512_loadu_ps(w3 + j + 16), y, b3);
    }
    if (j + 16 <= n) {
      __m512 x = _mm512_loadu_ps(u + j);
      a0 = _mm512_fmadd_ps(_mm512_loadu_ps(w0 + j), x, a0);
      a1 = _mm512_fmadd_ps(_mm512_loadu_ps(w1 + j), x, a1);
      a2 = _mm512_fmadd_ps(_mm512_loadu_ps(w2 + j), x, a2);
      a3 = _mm512_fmadd_ps(_mm512_loadu_ps(w3 + j), x, a3);
      j += 16;
    }
    if (j < n) {
      const int rem = n - j;
      __m512 x = LoadTail(u + j, rem);
      b0 = _mm512_fmadd_ps(LoadTail(w0 + j, rem), x, b0);
      b1 = _mm512_fmadd_ps(LoadTail(w1 + j, rem), x, b1);
      b2 = _mm512_fmadd_ps(LoadTail(w2 + j, rem), x, b2);
      b3 = _mm512_fmadd_ps(LoadTail(w3 + j, rem), x, b3);
    }
    _mm_storeu_ps(v + i, Sum4(_mm512_add_ps(a0, b0), _mm512_add_ps(a1, b1),
                              _mm512_add_ps(a2, b2), _mm512_add_ps(a3, b3)));
  }
  for (; i < num_rows; i++) {
    v[i] = DotProductAVX512F(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

// Adds the products of four vectors x (loaded from four rows of u) with the
// four vectors y0 ... y3 to 16 sums.
#    define CROSS_STEP(LOAD)            \
      {                                 \
        __m512 y0 = LOAD(v0);           \
        __m512 y1 = LOAD(v1);           \
        __m512 y2 = LOAD(v2);           \
        __m512 y3 = LOAD(v3);           \
        __m512 x = LOAD(u0);            \
        a0 = _mm512_fmadd_ps(x, y0, a0); \
        a1 = _mm512_fmadd_ps(x, y1, a1); \
        a2 = _mm512_fmadd_ps(x, y2, a2); \
        a3 = _mm512_fmadd_ps(x, y3, a3); \
        x = LOAD(u1);                   \
        b0 = _mm512_fmadd_ps(x, y0, b0); \
        b1 = _mm512_fmadd_ps(x, y1, b1); \
        b2 = _mm512_fmadd_ps(x, y2, b2); \
        b3 = _mm512_fmadd_ps(x, y3, b3); \
        x = LOAD(u2);                   \
        c0 = _mm512_fmadd_ps(x, y0, c0); \
        c1 = _mm512_fmadd_ps(x, y1, c1); \
        c2 = _mm512_fmadd_ps(x, y2, c2); \
        c3 = _mm512_fmadd_ps(x, y3, c3); \
        x = LOAD(u3);                   \
        d0 = _mm512_fmadd_ps(x, y0, d0); \
        d1 = _mm512_fmadd_ps(x, y1, d1); \
        d2 = _mm512_fmadd_ps(x, y2, d2); \
        d3 = _mm512_fmadd_ps(x, y3, d3); \
      }
#    define LOAD_FULL(p) _mm512_loadu_ps((p) + k)
#    define LOAD_TAIL(p) LoadTail((p) + k, rem)

// Computes out[i * out_stride + j] = u[i] . v[j] for the num_u rows of u and
// the num_v rows of v, which all have n elements and are stored without gaps.
// Four rows of u and four rows of v are handled per pass, so each of them is
// loaded only once for 16 sums which are calculated at the same time.
void CrossDotProductsAVX512F(const float *u, int num_u, const float *v,
                             int num_v, int n, float *out, int out_stride) {
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
    for (; j + 4 <= num_v; j += 4) {
      const float *v0 = v + static_cast<size_t>(j) * n;
      const float *v1 = v0 + n;
      const float *v2 = v1 + n;
      const float *v3 = v2 + n;
      __m512 a0 = _mm512_setzero_ps();
      __m512 a1 = a0, a2 = a0, a3 = a0;
      __m512 b0 = a0, b1 = a0, b2 = a0, b3 = a0;
      __m512 c0 = a0, c1 = a0, c2 = a0, c3 = a0;
      __m512 d0 = a0, d1 = a0, d2 = a0, d3 = a0;
      int k = 0;
      for (; k + 16 <= n; k += 16) {
        CROSS_STEP(LOAD_FULL)
      }
      if (k < n) {
        const int rem = n - k;
        CROSS_STEP(LOAD_TAIL)
      }
      _mm_storeu_ps(out0 + j, Sum4(a0, a1, a2, a3));
      _mm_storeu_ps(out1 + j, Sum4(b0, b1, b2, b3));
      _mm_storeu_ps(out2 + j, Sum4(c0, c1, c2, c3));
      _mm_storeu_ps(out3 + j, Sum4(d0, d1, d2, d3));
    }
    for (; j < num_v; j++) {
      // A remaining row of v is a single vector for the four rows of u.
      float last[4];
      RowDotProductsAVX512F(u0, n, 4, v + static_cast<size_t>(j) * n, n, last);
      out0[j] = last[0];
      out1[j] = last[1];
      out2[j] = last[2];
      out3[j] = last[3];
    }
  }
  // The remaining rows of u are single vectors for the rows of v.
  for (; i < num_u; i++) {
    RowDotProductsAVX512F(v, n, num_v, u + static_cast<size_t>(i) * n, n,
                          out + static_cast<size_t>(i) * out_stride);
  }
}

#    undef CROSS_STEP
#    undef LOAD_FULL
#    undef LOAD_TAIL
#  else
double DotProductAVX512F(const double *u, const double *v, int n) {
  const unsigned quot = n / 8;
  const unsigned rem = n % 8;
  __m512d t0 = _mm512_setzero_pd();
  for (unsigned k = 0; k < quot; k++) {
    t0 = _mm512_fmadd_pd(_mm512_loadu_pd(u), _mm512_loadu_pd(v), t0);
    u += 8;
    v += 8;
  }
  double result = _mm512_reduce_add_pd(t0);
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}
#  endif

} // namespace tesseract.

#endif
