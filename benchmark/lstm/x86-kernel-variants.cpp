///////////////////////////////////////////////////////////////////////
// File:        x86-kernel-variants.cpp
// Description: Microbenchmark for variants of the FMA and AVX512F
//              functions which calculate several dot products at once.
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

// This standalone program (x86_64 only) compares the time of single dot
// products, called by a function pointer like in the generic code, with
// variants which handle several rows per pass, with registers of 256 bits
// (AVX and FMA) and of 512 bits (AVX512F). The sizes are those of the layers
// of the default network of tesstrain. It was used to choose the
// implementations of RowDotProductsFMA, CrossDotProductsFMA (src/arch/
// dotproductfma.cpp), RowDotProductsAVX512F and CrossDotProductsAVX512F
// (src/arch/dotproductavx512.cpp) and can be used to try other variants on
// a new CPU or with a new compiler:
//
//   c++ -std=c++17 -O3 -mavx2 -mfma -mavx512f -o x86-kernel-variants
//     x86-kernel-variants.cpp        (one command line)
//   ./x86-kernel-variants
//
// It needs a CPU with AVX512F. Remove the variants for 512 bits (V16) from
// the tables in main() to run it on a CPU without AVX512F.
//
// A variant RxA handles R rows per pass with A sums for each of them
// (matrix times vector), a variant RxC calculates a block of R x C dot
// products per pass (sum of outer products).

#include <immintrin.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

#define NOINLINE __attribute__((noinline))
#define INLINE inline __attribute__((always_inline))

// 256 bits: AVX and FMA.
struct V8 {
  using R = __m256;
  static constexpr int W = 8;
  static INLINE R Zero() {
    return _mm256_setzero_ps();
  }
  static INLINE R Load(const float *p) {
    return _mm256_loadu_ps(p);
  }
  // Loads the first rem (< W) elements, the others are 0 and are not read.
  static INLINE R LoadTail(const float *p, int rem) {
    alignas(32) static const int32_t kMask[16] = {-1, -1, -1, -1, -1, -1, -1, -1,
                                                  0,  0,  0,  0,  0,  0,  0,  0};
    return _mm256_maskload_ps(
        p, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(kMask + 8 - rem)));
  }
  static INLINE R Fma(R a, R b, R c) {
    return _mm256_fmadd_ps(a, b, c);
  }
  static INLINE R Add(R a, R b) {
    return _mm256_add_ps(a, b);
  }
  static INLINE float Sum(R a) {
    __m128 s = _mm_add_ps(_mm256_castps256_ps128(a), _mm256_extractf128_ps(a, 1));
    s = _mm_add_ps(s, _mm_movehl_ps(s, s));
    s = _mm_add_ss(s, _mm_shuffle_ps(s, s, 1));
    return _mm_cvtss_f32(s);
  }
};

// 512 bits: AVX512F.
struct V16 {
  using R = __m512;
  static constexpr int W = 16;
  static INLINE R Zero() {
    return _mm512_setzero_ps();
  }
  static INLINE R Load(const float *p) {
    return _mm512_loadu_ps(p);
  }
  static INLINE R LoadTail(const float *p, int rem) {
    return _mm512_maskz_loadu_ps(static_cast<__mmask16>((1u << rem) - 1), p);
  }
  static INLINE R Fma(R a, R b, R c) {
    return _mm512_fmadd_ps(a, b, c);
  }
  static INLINE R Add(R a, R b) {
    return _mm512_add_ps(a, b);
  }
  static INLINE float Sum(R a) {
    return _mm512_reduce_add_ps(a);
  }
};

// Dot product like DotProductFMA for FAST_FLOAT.
NOINLINE float DotFMA(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m256 t0 = _mm256_setzero_ps();
  __m256 t1 = _mm256_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    t0 = _mm256_fmadd_ps(_mm256_loadu_ps(u), _mm256_loadu_ps(v), t0);
    t1 = _mm256_fmadd_ps(_mm256_loadu_ps(u + 8), _mm256_loadu_ps(v + 8), t1);
    u += 16;
    v += 16;
  }
  t0 = _mm256_hadd_ps(t0, t1);
  alignas(32) float tmp[8];
  _mm256_store_ps(tmp, t0);
  float result =
      tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

// Dot product like DotProductAVX512F for FAST_FLOAT.
NOINLINE float DotAVX512F(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m512 t0 = _mm512_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    t0 = _mm512_fmadd_ps(_mm512_loadu_ps(u), _mm512_loadu_ps(v), t0);
    u += 16;
    v += 16;
  }
  float result = _mm512_reduce_add_ps(t0);
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

typedef float (*DotFn)(const float *, const float *, int);
volatile DotFn g_dot;

// Single dot products like the generic code.
NOINLINE void RowsSingle(const float *w, int w_stride, int num_rows,
                         const float *u, int n, float *v) {
  DotFn dot = g_dot;
  for (int i = 0; i < num_rows; i++) {
    v[i] = dot(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

NOINLINE void CrossSingle(const float *u, int num_u, const float *v, int num_v,
                          int n, float *out, int out_stride) {
  DotFn dot = g_dot;
  for (int i = 0; i < num_u; i++) {
    for (int j = 0; j < num_v; j++) {
      out[static_cast<size_t>(i) * out_stride + j] =
          dot(u + static_cast<size_t>(i) * n, v + static_cast<size_t>(j) * n, n);
    }
  }
}

// One row with two sums, for the rows which are left.
template <class V>
static INLINE float Dot1x2(const float *w, const float *u, int n) {
  typename V::R a = V::Zero(), b = a;
  int j = 0;
  for (; j + 2 * V::W <= n; j += 2 * V::W) {
    a = V::Fma(V::Load(w + j), V::Load(u + j), a);
    b = V::Fma(V::Load(w + j + V::W), V::Load(u + j + V::W), b);
  }
  if (j + V::W <= n) {
    a = V::Fma(V::Load(w + j), V::Load(u + j), a);
    j += V::W;
  }
  if (j < n) {
    b = V::Fma(V::LoadTail(w + j, n - j), V::LoadTail(u + j, n - j), b);
  }
  return V::Sum(V::Add(a, b));
}

// Four rows per pass, one sum for each row.
template <class V>
static INLINE void Rows4x1(const float *w, int w_stride, int num_rows,
                           const float *u, int n, float *v) {
  int i = 0;
  for (; i + 4 <= num_rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    typename V::R a0 = V::Zero(), a1 = a0, a2 = a0, a3 = a0;
    int j = 0;
    for (; j + V::W <= n; j += V::W) {
      typename V::R x = V::Load(u + j);
      a0 = V::Fma(V::Load(w0 + j), x, a0);
      a1 = V::Fma(V::Load(w1 + j), x, a1);
      a2 = V::Fma(V::Load(w2 + j), x, a2);
      a3 = V::Fma(V::Load(w3 + j), x, a3);
    }
    if (j < n) {
      int rem = n - j;
      typename V::R x = V::LoadTail(u + j, rem);
      a0 = V::Fma(V::LoadTail(w0 + j, rem), x, a0);
      a1 = V::Fma(V::LoadTail(w1 + j, rem), x, a1);
      a2 = V::Fma(V::LoadTail(w2 + j, rem), x, a2);
      a3 = V::Fma(V::LoadTail(w3 + j, rem), x, a3);
    }
    v[i] = V::Sum(a0);
    v[i + 1] = V::Sum(a1);
    v[i + 2] = V::Sum(a2);
    v[i + 3] = V::Sum(a3);
  }
  for (; i < num_rows; i++) {
    v[i] = Dot1x2<V>(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

// Four rows per pass, two sums for each row.
template <class V>
static INLINE void Rows4x2(const float *w, int w_stride, int num_rows,
                           const float *u, int n, float *v) {
  int i = 0;
  for (; i + 4 <= num_rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    typename V::R a0 = V::Zero(), a1 = a0, a2 = a0, a3 = a0;
    typename V::R b0 = a0, b1 = a0, b2 = a0, b3 = a0;
    int j = 0;
    for (; j + 2 * V::W <= n; j += 2 * V::W) {
      typename V::R x = V::Load(u + j);
      typename V::R y = V::Load(u + j + V::W);
      a0 = V::Fma(V::Load(w0 + j), x, a0);
      b0 = V::Fma(V::Load(w0 + j + V::W), y, b0);
      a1 = V::Fma(V::Load(w1 + j), x, a1);
      b1 = V::Fma(V::Load(w1 + j + V::W), y, b1);
      a2 = V::Fma(V::Load(w2 + j), x, a2);
      b2 = V::Fma(V::Load(w2 + j + V::W), y, b2);
      a3 = V::Fma(V::Load(w3 + j), x, a3);
      b3 = V::Fma(V::Load(w3 + j + V::W), y, b3);
    }
    if (j + V::W <= n) {
      typename V::R x = V::Load(u + j);
      a0 = V::Fma(V::Load(w0 + j), x, a0);
      a1 = V::Fma(V::Load(w1 + j), x, a1);
      a2 = V::Fma(V::Load(w2 + j), x, a2);
      a3 = V::Fma(V::Load(w3 + j), x, a3);
      j += V::W;
    }
    if (j < n) {
      int rem = n - j;
      typename V::R x = V::LoadTail(u + j, rem);
      b0 = V::Fma(V::LoadTail(w0 + j, rem), x, b0);
      b1 = V::Fma(V::LoadTail(w1 + j, rem), x, b1);
      b2 = V::Fma(V::LoadTail(w2 + j, rem), x, b2);
      b3 = V::Fma(V::LoadTail(w3 + j, rem), x, b3);
    }
    v[i] = V::Sum(V::Add(a0, b0));
    v[i + 1] = V::Sum(V::Add(a1, b1));
    v[i + 2] = V::Sum(V::Add(a2, b2));
    v[i + 3] = V::Sum(V::Add(a3, b3));
  }
  for (; i < num_rows; i++) {
    v[i] = Dot1x2<V>(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

// Eight rows per pass, one sum for each row.
template <class V>
static INLINE void Rows8x1(const float *w, int w_stride, int num_rows,
                           const float *u, int n, float *v) {
  int i = 0;
  for (; i + 8 <= num_rows; i += 8) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    const float *w4 = w3 + w_stride;
    const float *w5 = w4 + w_stride;
    const float *w6 = w5 + w_stride;
    const float *w7 = w6 + w_stride;
    typename V::R a0 = V::Zero(), a1 = a0, a2 = a0, a3 = a0;
    typename V::R a4 = a0, a5 = a0, a6 = a0, a7 = a0;
    int j = 0;
    for (; j + V::W <= n; j += V::W) {
      typename V::R x = V::Load(u + j);
      a0 = V::Fma(V::Load(w0 + j), x, a0);
      a1 = V::Fma(V::Load(w1 + j), x, a1);
      a2 = V::Fma(V::Load(w2 + j), x, a2);
      a3 = V::Fma(V::Load(w3 + j), x, a3);
      a4 = V::Fma(V::Load(w4 + j), x, a4);
      a5 = V::Fma(V::Load(w5 + j), x, a5);
      a6 = V::Fma(V::Load(w6 + j), x, a6);
      a7 = V::Fma(V::Load(w7 + j), x, a7);
    }
    if (j < n) {
      int rem = n - j;
      typename V::R x = V::LoadTail(u + j, rem);
      a0 = V::Fma(V::LoadTail(w0 + j, rem), x, a0);
      a1 = V::Fma(V::LoadTail(w1 + j, rem), x, a1);
      a2 = V::Fma(V::LoadTail(w2 + j, rem), x, a2);
      a3 = V::Fma(V::LoadTail(w3 + j, rem), x, a3);
      a4 = V::Fma(V::LoadTail(w4 + j, rem), x, a4);
      a5 = V::Fma(V::LoadTail(w5 + j, rem), x, a5);
      a6 = V::Fma(V::LoadTail(w6 + j, rem), x, a6);
      a7 = V::Fma(V::LoadTail(w7 + j, rem), x, a7);
    }
    v[i] = V::Sum(a0);
    v[i + 1] = V::Sum(a1);
    v[i + 2] = V::Sum(a2);
    v[i + 3] = V::Sum(a3);
    v[i + 4] = V::Sum(a4);
    v[i + 5] = V::Sum(a5);
    v[i + 6] = V::Sum(a6);
    v[i + 7] = V::Sum(a7);
  }
  Rows4x1<V>(w + static_cast<size_t>(i) * w_stride, w_stride, num_rows - i, u,
             n, v + i);
}

// Blocks of 4 x 2 dot products.
template <class V>
static INLINE void Cross4x2(const float *u, int num_u, const float *v,
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
    for (; j + 2 <= num_v; j += 2) {
      const float *v0 = v + static_cast<size_t>(j) * n;
      const float *v1 = v0 + n;
      typename V::R a0 = V::Zero(), a1 = a0, b0 = a0, b1 = a0;
      typename V::R c0 = a0, c1 = a0, d0 = a0, d1 = a0;
      int k = 0;
      for (; k + V::W <= n; k += V::W) {
        typename V::R y0 = V::Load(v0 + k);
        typename V::R y1 = V::Load(v1 + k);
        typename V::R x = V::Load(u0 + k);
        a0 = V::Fma(x, y0, a0);
        a1 = V::Fma(x, y1, a1);
        x = V::Load(u1 + k);
        b0 = V::Fma(x, y0, b0);
        b1 = V::Fma(x, y1, b1);
        x = V::Load(u2 + k);
        c0 = V::Fma(x, y0, c0);
        c1 = V::Fma(x, y1, c1);
        x = V::Load(u3 + k);
        d0 = V::Fma(x, y0, d0);
        d1 = V::Fma(x, y1, d1);
      }
      if (k < n) {
        int rem = n - k;
        typename V::R y0 = V::LoadTail(v0 + k, rem);
        typename V::R y1 = V::LoadTail(v1 + k, rem);
        typename V::R x = V::LoadTail(u0 + k, rem);
        a0 = V::Fma(x, y0, a0);
        a1 = V::Fma(x, y1, a1);
        x = V::LoadTail(u1 + k, rem);
        b0 = V::Fma(x, y0, b0);
        b1 = V::Fma(x, y1, b1);
        x = V::LoadTail(u2 + k, rem);
        c0 = V::Fma(x, y0, c0);
        c1 = V::Fma(x, y1, c1);
        x = V::LoadTail(u3 + k, rem);
        d0 = V::Fma(x, y0, d0);
        d1 = V::Fma(x, y1, d1);
      }
      out0[j] = V::Sum(a0);
      out0[j + 1] = V::Sum(a1);
      out1[j] = V::Sum(b0);
      out1[j + 1] = V::Sum(b1);
      out2[j] = V::Sum(c0);
      out2[j + 1] = V::Sum(c1);
      out3[j] = V::Sum(d0);
      out3[j + 1] = V::Sum(d1);
    }
    if (j < num_v) {
      // The last row of v is a single vector for four rows of u.
      float last[4];
      Rows4x1<V>(u0, n, 4, v + static_cast<size_t>(j) * n, n, last);
      out0[j] = last[0];
      out1[j] = last[1];
      out2[j] = last[2];
      out3[j] = last[3];
    }
  }
  // The remaining rows of u are single vectors for the rows of v.
  for (; i < num_u; i++) {
    Rows4x1<V>(v, n, num_v, u + static_cast<size_t>(i) * n, n,
               out + static_cast<size_t>(i) * out_stride);
  }
}

// Blocks of 4 x 4 dot products.
template <class V>
static INLINE void Cross4x4(const float *u, int num_u, const float *v,
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
      typename V::R a0 = V::Zero(), a1 = a0, a2 = a0, a3 = a0;
      typename V::R b0 = a0, b1 = a0, b2 = a0, b3 = a0;
      typename V::R c0 = a0, c1 = a0, c2 = a0, c3 = a0;
      typename V::R d0 = a0, d1 = a0, d2 = a0, d3 = a0;
#define CROSS4X4_STEP(LOAD)   \
  {                           \
    typename V::R y0 = LOAD(v0); \
    typename V::R y1 = LOAD(v1); \
    typename V::R y2 = LOAD(v2); \
    typename V::R y3 = LOAD(v3); \
    typename V::R x = LOAD(u0);  \
    a0 = V::Fma(x, y0, a0);   \
    a1 = V::Fma(x, y1, a1);   \
    a2 = V::Fma(x, y2, a2);   \
    a3 = V::Fma(x, y3, a3);   \
    x = LOAD(u1);             \
    b0 = V::Fma(x, y0, b0);   \
    b1 = V::Fma(x, y1, b1);   \
    b2 = V::Fma(x, y2, b2);   \
    b3 = V::Fma(x, y3, b3);   \
    x = LOAD(u2);             \
    c0 = V::Fma(x, y0, c0);   \
    c1 = V::Fma(x, y1, c1);   \
    c2 = V::Fma(x, y2, c2);   \
    c3 = V::Fma(x, y3, c3);   \
    x = LOAD(u3);             \
    d0 = V::Fma(x, y0, d0);   \
    d1 = V::Fma(x, y1, d1);   \
    d2 = V::Fma(x, y2, d2);   \
    d3 = V::Fma(x, y3, d3);   \
  }
      int k = 0;
#define FULL(p) V::Load(p + k)
      for (; k + V::W <= n; k += V::W) {
        CROSS4X4_STEP(FULL)
      }
      if (k < n) {
        int rem = n - k;
#define TAIL(p) V::LoadTail(p + k, rem)
        CROSS4X4_STEP(TAIL)
      }
      out0[j] = V::Sum(a0);
      out0[j + 1] = V::Sum(a1);
      out0[j + 2] = V::Sum(a2);
      out0[j + 3] = V::Sum(a3);
      out1[j] = V::Sum(b0);
      out1[j + 1] = V::Sum(b1);
      out1[j + 2] = V::Sum(b2);
      out1[j + 3] = V::Sum(b3);
      out2[j] = V::Sum(c0);
      out2[j + 1] = V::Sum(c1);
      out2[j + 2] = V::Sum(c2);
      out2[j + 3] = V::Sum(c3);
      out3[j] = V::Sum(d0);
      out3[j + 1] = V::Sum(d1);
      out3[j + 2] = V::Sum(d2);
      out3[j + 3] = V::Sum(d3);
    }
    for (; j < num_v; j++) {
      float last[4];
      Rows4x1<V>(u0, n, 4, v + static_cast<size_t>(j) * n, n, last);
      out0[j] = last[0];
      out1[j] = last[1];
      out2[j] = last[2];
      out3[j] = last[3];
    }
  }
  for (; i < num_u; i++) {
    Rows4x1<V>(v, n, num_v, u + static_cast<size_t>(i) * n, n,
               out + static_cast<size_t>(i) * out_stride);
  }
}

#define ROWS(NAME, IMPL, V)                                          \
  NOINLINE void NAME(const float *w, int w_stride, int num_rows,     \
                     const float *u, int n, float *v) {              \
    IMPL<V>(w, w_stride, num_rows, u, n, v);                         \
  }
ROWS(Rows4x1_256, Rows4x1, V8)
ROWS(Rows4x2_256, Rows4x2, V8)
ROWS(Rows8x1_256, Rows8x1, V8)
ROWS(Rows4x1_512, Rows4x1, V16)
ROWS(Rows4x2_512, Rows4x2, V16)
ROWS(Rows8x1_512, Rows8x1, V16)

#define CROSS(NAME, IMPL, V)                                           \
  NOINLINE void NAME(const float *u, int num_u, const float *v,        \
                     int num_v, int n, float *out, int out_stride) {   \
    IMPL<V>(u, num_u, v, num_v, n, out, out_stride);                   \
  }
CROSS(Cross4x2_256, Cross4x2, V8)
CROSS(Cross4x4_256, Cross4x4, V8)
CROSS(Cross4x2_512, Cross4x2, V16)
CROSS(Cross4x4_512, Cross4x4, V16)

// Returns the best time per call in ns of several measurements.
template <class F>
static double BestNs(F f, int reps) {
  double best = 1e30;
  for (int r = 0; r < 7; r++) {
    auto t0 = std::chrono::steady_clock::now();
    for (int k = 0; k < reps; k++) {
      f();
    }
    double ns = std::chrono::duration<double, std::nano>(
                    std::chrono::steady_clock::now() - t0)
                    .count() /
                reps;
    if (ns < best) {
      best = ns;
    }
  }
  return best;
}

// Returns the largest difference of two results.
static double MaxDiff(const std::vector<float> &a,
                      const std::vector<float> &b) {
  double m = 0;
  for (size_t i = 0; i < a.size(); i++) {
    m = std::fmax(m, std::fabs(a[i] - b[i]));
  }
  return m;
}

// Returns deterministic pseudo-random numbers with 24 bits for the test data.
// A linear congruential generator gives the same data on all platforms.
static uint32_t Random() {
  static uint32_t state = 1;
  state = state * 1664525u + 1013904223u;
  return state >> 8;
}

int main() {
  auto fill = [](std::vector<float> &x) {
    for (auto &e : x) {
      e = Random() / (float)0x1000000 - 0.5f;
    }
  };
  struct {
    const char *name;
    int ns, na;
  } layers[] = {{"Conv16", 16, 9},  {"Lfys48", 48, 64},   {"Lfx96", 96, 144},
                {"Lrx96", 96, 192}, {"Lfx192", 192, 288}, {"Out111", 111, 192},
                {"odd", 113, 195}};
  typedef void (*RowFn)(const float *, int, int, const float *, int, float *);
  struct {
    const char *name;
    RowFn f;
    DotFn dot;
  } rf[] = {{"fma", RowsSingle, DotFMA},       {"4x1", Rows4x1_256, nullptr},
            {"4x2", Rows4x2_256, nullptr},     {"8x1", Rows8x1_256, nullptr},
            {"avx512", RowsSingle, DotAVX512F}, {"4x1", Rows4x1_512, nullptr},
            {"4x2", Rows4x2_512, nullptr},     {"8x1", Rows8x1_512, nullptr}};
  for (int backward = 0; backward < 2; backward++) {
    printf(backward
               ? "u*W backward (4 transposed gate matrices), us per timestep\n"
               : "W*u forward (4 gate matrices in turn), us per timestep\n");
    printf(" %-7s", "layer");
    for (auto &f : rf) {
      printf(" %6s", f.name);
    }
    printf("\n");
    for (auto &L : layers) {
      // Forward: ns rows with na inputs and a bias. Backward: na rows with
      // ns elements.
      int rows = backward ? L.na : L.ns;
      int n = backward ? L.ns : L.na;
      int stride = backward ? L.ns : L.na + 1;
      std::vector<float> w(4 * (rows + 1) * stride), u(n), ref(4 * rows),
          v(4 * rows);
      fill(w);
      fill(u);
      printf(" %-7s", L.name);
      bool first = true;
      for (auto &f : rf) {
        auto &dst = first ? ref : v;
        if (f.dot) {
          g_dot = f.dot;
        }
        double t = BestNs(
            [&] {
              for (int g = 0; g < 4; g++) {
                f.f(w.data() + g * (rows + 1) * stride, stride, rows, u.data(),
                    n, dst.data() + g * rows);
              }
            },
            3000);
        printf(" %6.2f", t / 1e3);
        if (!first && MaxDiff(ref, v) > 1e-3) {
          printf("(DIFF %g)", MaxDiff(ref, v));
        }
        first = false;
      }
      printf("\n");
    }
  }
  typedef void (*CrossFn)(const float *, int, const float *, int, int, float *,
                          int);
  struct {
    const char *name;
    CrossFn f;
    DotFn dot;
  } cf[] = {{"fma", CrossSingle, DotFMA},
            {"4x2", Cross4x2_256, nullptr},
            {"4x4", Cross4x4_256, nullptr},
            {"avx512", CrossSingle, DotAVX512F},
            {"4x2", Cross4x2_512, nullptr},
            {"4x4", Cross4x4_512, nullptr}};
  printf("gradient sum (one gate) for T timesteps, ms per call\n");
  printf(" %-7s %4s", "layer", "T");
  for (auto &f : cf) {
    printf(" %6s", f.name);
  }
  printf("\n");
  for (int T : {37, 100, 300, 1000}) {
    for (auto &L : layers) {
      int os = L.na + 1;
      std::vector<float> u(L.ns * T), vv(L.na * T), ref(L.ns * os),
          d(L.ns * os);
      fill(u);
      fill(vv);
      printf(" %-7s %4d", L.name, T);
      bool first = true;
      for (auto &f : cf) {
        auto &dst = first ? ref : d;
        if (f.dot) {
          g_dot = f.dot;
        }
        double t = BestNs(
            [&] { f.f(u.data(), L.ns, vv.data(), L.na, T, dst.data(), os); },
            20);
        printf(" %6.3f", t / 1e6);
        if (!first && MaxDiff(ref, d) > 2e-3) {
          printf("(DIFF %g)", MaxDiff(ref, d));
        }
        first = false;
      }
      printf("\n");
    }
  }
  return 0;
}
