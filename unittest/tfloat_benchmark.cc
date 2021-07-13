///////////////////////////////////////////////////////////////////////
// File:        tfloat_benchmark.cc
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

#include <memory>
#include <vector>
#include <numeric> // for std::inner_product
#include "dotproduct.h"
#include "include_gunit.h"
#include "matrix.h"
#include "simddetect.h"
#include "tprintf.h"
#include "intsimdmatrix.h" // for IntSimdMatrix
#include "simddetect.h" // for DotProduct
#include "tfloat.h"
#include "intsimdmatrix.h"

// Tests various implementations of costly DotProduct and Matrix calls, using
// AVX/FMA/SSE/NEON/OPENMP/native in both TFloat=float and TFloat=double
// incantations.

#if defined(__AVX__)

#  include <immintrin.h>
#  include <cstdint>

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
// Uses Intel AVX intrinsics to access the SIMD instruction set.
float DotProductAVX_8(const float *u, const float *v, int n) {
  const unsigned quot = n / 8;
  const unsigned rem = n % 8;
  __m256 t0 = _mm256_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    __m256 f0 = _mm256_loadu_ps(u);
    __m256 f1 = _mm256_loadu_ps(v);
    f0 = _mm256_mul_ps(f0, f1);
    t0 = _mm256_add_ps(t0, f0);
    u += 8;
    v += 8;
  }
  alignas(32) float tmp[8];
  _mm256_store_ps(tmp, t0);
  float result =
      tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

double DotProductAVX_8(const double *u, const double *v, int n) {
  const unsigned quot = n / 4;
  const unsigned rem = n % 4;
  __m256d t0 = _mm256_setzero_pd();
  for (unsigned k = 0; k < quot; k++) {
    __m256d f0 = _mm256_loadu_pd(u);
    __m256d f1 = _mm256_loadu_pd(v);
    f0 = _mm256_mul_pd(f0, f1);
    t0 = _mm256_add_pd(t0, f0);
    u += 4;
    v += 4;
  }
  alignas(32) double tmp[4];
  _mm256_store_pd(tmp, t0);
  double result =
      tmp[0] + tmp[1] + tmp[2] + tmp[3];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

float DotProductAVX_A(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m256 t0 = _mm256_setzero_ps();
  __m256 t1 = _mm256_setzero_ps();
  // Aligned load is reputedly faster but requires 16 byte aligned input.
  if ((reinterpret_cast<uintptr_t>(u) & 15) == 0 &&
      (reinterpret_cast<uintptr_t>(v) & 15) == 0) {
    for (unsigned k = 0; k < quot; k++) {
      __m256 f0 = _mm256_load_ps(u);
      __m256 f1 = _mm256_load_ps(v);
      f0 = _mm256_mul_ps(f0, f1);
      t0 = _mm256_add_ps(t0, f0);
      u += 8;
      v += 8;
      f0 = _mm256_load_ps(u);
      f1 = _mm256_load_ps(v);
      f0 = _mm256_mul_ps(f0, f1);
      t1 = _mm256_add_ps(t1, f0);
      u += 8;
      v += 8;
    }
  } else {
    for (unsigned k = 0; k < quot; k++) {
      __m256 f0 = _mm256_loadu_ps(u);
      __m256 f1 = _mm256_loadu_ps(v);
      f0 = _mm256_mul_ps(f0, f1);
      t0 = _mm256_add_ps(t0, f0);
      u += 8;
      v += 8;
      __m256 f2 = _mm256_loadu_ps(u);
      __m256 f3 = _mm256_loadu_ps(v);
      f2 = _mm256_mul_ps(f2, f3);
      t1 = _mm256_add_ps(t1, f2);
      u += 8;
      v += 8;
    }
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

double DotProductAVX_A(const double *u, const double *v, int n) {
	// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  return DotProductAVX(u, v, n);
}

float DotProductAVX_512(const float *u, const float *v, int n) {
  const unsigned quot = n / 16;
  const unsigned rem = n % 16;
  __m512 t0 = _mm512_setzero_ps();
  for (unsigned k = 0; k < quot; k++) {
    __m512 f0 = _mm512_loadu_ps(u);
    __m512 f1 = _mm512_loadu_ps(v);
    f0 = _mm512_mul_ps(f0, f1);
    t0 = _mm512_add_ps(t0, f0);
    u += 16;
    v += 16;
  }
  // https://stackoverflow.com/questions/26896432/horizontal-add-with-m512-avx512
  __m512 tmpf =
      _mm512_add_ps(t0, _mm512_shuffle_f32x4(t0, t0, _MM_SHUFFLE(0, 0, 3, 2)));
  __m128 r = _mm512_castps512_ps128(_mm512_add_ps(
      tmpf, _mm512_shuffle_f32x4(tmpf, tmpf, _MM_SHUFFLE(0, 0, 0, 1))));
  r = _mm_hadd_ps(r, r);
  float result = _mm_cvtss_f32(_mm_hadd_ps(r, r));

  alignas(32) float tmp[16];
  _mm512_store_ps(tmp, t0);
  result = tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] +
           tmp[7] + tmp[8] + tmp[9] + tmp[10] + tmp[11] + tmp[12] + tmp[13] +
           tmp[14] + tmp[15];
  for (unsigned k = 0; k < rem; k++) {
    result += *u++ * *v++;
  }
  return result;
}

} // namespace tesseract

#endif // __AVX__


namespace tesseract {

// Computes and returns the dot product of the two n-vectors u and v.
template <class TFloat>
TFloat DotProductDBL(const TFloat *u, const TFloat *v, int n) {
  double total = 0.0;
  for (int k = 0; k < n; ++k) {
    total += (double)u[k] * (double)v[k];
  }
  return total;
}

// Computes and returns the dot product of the two n-vectors u and v.
template <class TFloat>
TFloat DotProductNativeTF(const TFloat *u, const TFloat *v, int n) {
  float total = 0.0;
  for (int k = 0; k < n; ++k) {
    total += (float)u[k] * (float)v[k];
  }
  return total;
}

template <class TFloat>
TFloat DotProductGeneric(const TFloat *u, const TFloat *v, int n) {
  TFloat total = 0;
#pragma omp simd reduction(+ : total)
  for (int k = 0; k < n; ++k) {
    total += u[k] * v[k];
  }
  return total;
}


bool approx_eq(double a, double b) {
  auto diff = a - b;
  if (diff == 0.0)
    return true;
  // take the log of both, as we know all incoming values will be positive,
  // so we can check the precision easily, i.e. at which significant digit
  // did the difference occur? ::
  a = log(a);
  b = log(b);
  diff = a - b;
  return (diff >= -1e-5 && diff <= 1e-5);
}

template <class T>
void run_tfloat_benchmark(void) {
#define AMOUNT 655360
  // offset jump between tests; determines (indirectly) the number of
  // memory-aligned tests vs. UNaligned tests executed. (STEP=4 would be
  // 16-byte step @ float, i.e. each test aligned)
#define STEP 1

  static T arr1[AMOUNT];
  static T arr2[AMOUNT];

  for (int i = 0; i < AMOUNT; i++) {
    arr1[i] = 2.0 + i * 0.005;
    arr2[i] = 7.0 + i * 0.005;
  }

  T total[20];
  const int size = 32768; // size of vector(s) to feed the dot-product functions

  // start with aligned storage, then step through, including unaligned storage
  // accesses
  int round = 0;
  for (int step = 0; step + size < AMOUNT; (step += STEP), round++) {
    T *p1 = arr1 + step;
    T *p2 = arr2 + step;

    total[0] = DotProductDBL(p1, p2, size);
    const double sollwert = total[0];
    // total[1] = DotProduct(p1, p2, size);
    total[1] = DotProductNative(p1, p2, size);
    total[2] = DotProductNativeTF(p1, p2, size);
    total[3] = DotProductAVX(p1, p2, size);
    total[4] = DotProductFMA(p1, p2, size);
    total[5] = DotProductSSE(p1, p2, size);
    total[6] = DotProductAVX_8(p1, p2, size);
    total[7] = DotProductAVX_A(p1, p2, size);
    total[8] = sollwert; // regrettably, my i7 doesn't have AVX512 support.
    // total[8] = DotProductAVX_512(p1, p2, size);
    total[9] = DotProductGeneric(p1, p2, size);
    total[10] = DotProductAVX1(p1, p2, size);

    // check calculations: this is always active to ensure we haven't got any
    // obvious bugs in there (we got a few! -- fixed!)
    for (int i = 1; i <= 10; i++) {
      if (!approx_eq(total[i], sollwert))
        fprintf(stderr, "step %d: total %d = %lf %s\n", step, i,
                (double)total[i],
                approx_eq(total[i], sollwert) ? "(PASS)" : "*****FAIL!*****");
    }
    // only load the console output for information once in a while: we're
    // running a benchmark here...
    if (round % 20000 == 0) {
      for (int i = 1; i <= 10; i++) {
        fprintf(stderr, "step %d: total %d = %lf %s\n", step, i,
                (double)total[i],
                approx_eq(total[i], sollwert) ? "(PASS)" : "*****FAIL!*****");
      }
    }
  }
}

// The benchmark runner:
void run_tfloat_benchmark(void) {
  run_tfloat_benchmark<float>();
  run_tfloat_benchmark<double>();
}

} // namespace tesseract
