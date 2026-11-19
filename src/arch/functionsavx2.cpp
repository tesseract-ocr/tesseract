///////////////////////////////////////////////////////////////////////
// File:        functionsavx2.cpp
// Description: Non-linearity functions and conversion of floats to
//              int8_t for whole vectors with AVX2.
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

#if !defined(__AVX2__)
#  if defined(__i686__) || defined(__x86_64__)
#    error Implementation only for AVX2 capable architectures
#  endif
#else

#  include <immintrin.h>
#  include <cstdint>
#  include "dotproduct.h"
#  include "functions.h" // for Tanh, Logistic, TanhTable, LogisticTable

namespace tesseract {

#  if defined(FAST_FLOAT)

// The functions give exactly the same results as the scalar functions Tanh,
// Logistic and IntCastRounded, because they do the same calculations in the
// same order. This file must not be built with FMA, which would change that.

// Returns the interpolated values of the table (TanhTable or LogisticTable)
// for the 8 values of a, which must not be negative, like the scalar
// functions do for x >= 0.
static inline __m256 Lookup(const float *table, __m256 a) {
  // Values which are too large for the table (the result is 1 for them) are
  // limited first, so the indices are always valid and nothing overflows,
  // also for huge values and for NaN (_mm256_min_ps returns the second value
  // then).
  const __m256 limit = _mm256_set1_ps((kTableSize - 1) / kScaleFactor);
  __m256 scaled =
      _mm256_mul_ps(_mm256_min_ps(a, limit), _mm256_set1_ps(kScaleFactor));
  __m256i index = _mm256_min_epi32(_mm256_cvttps_epi32(scaled),
                                   _mm256_set1_epi32(kTableSize - 2));
  // Each load gets table[i] and table[i + 1]. This is faster than the gather
  // instructions (much faster on CPUs with a microcode mitigation for them).
  alignas(32) int32_t i[8];
  _mm256_store_si256(reinterpret_cast<__m256i *>(i), index);
  auto pair = [table](int32_t k) {
    return _mm_castsi128_ps(
        _mm_loadl_epi64(reinterpret_cast<const __m128i *>(table + k)));
  };
  __m128 p01 = _mm_unpacklo_ps(pair(i[0]), pair(i[1]));
  __m128 p23 = _mm_unpacklo_ps(pair(i[2]), pair(i[3]));
  __m128 p45 = _mm_unpacklo_ps(pair(i[4]), pair(i[5]));
  __m128 p67 = _mm_unpacklo_ps(pair(i[6]), pair(i[7]));
  __m256 lo = _mm256_set_m128(p45, p01);
  __m256 hi = _mm256_set_m128(p67, p23);
  __m256 t0 = _mm256_shuffle_ps(lo, hi, 0x44); // table[i]
  __m256 t1 = _mm256_shuffle_ps(lo, hi, 0xee); // table[i + 1]
  // Linear interpolation.
  __m256 fraction = _mm256_sub_ps(scaled, _mm256_cvtepi32_ps(index));
  __m256 result =
      _mm256_add_ps(t0, _mm256_mul_ps(_mm256_sub_ps(t1, t0), fraction));
  // The result is 1 for values which are too large for the table.
  __m256 too_large = _mm256_cmp_ps(a, limit, _CMP_GE_OQ);
  return _mm256_blendv_ps(result, _mm256_set1_ps(1.0f), too_large);
}

// Computes out[i] = Tanh(in[i]) for n values. in and out can be the same.
void TanhVectorAVX2(int n, const float *in, float *out) {
  const __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff));
  const __m256 zero = _mm256_setzero_ps();
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m256 x = _mm256_loadu_ps(in + i);
    __m256 result = Lookup(TanhTable, _mm256_and_ps(x, abs_mask));
    // Tanh(x) = -Tanh(-x) for x < 0.
    __m256 negative = _mm256_cmp_ps(x, zero, _CMP_LT_OQ);
    result = _mm256_blendv_ps(result, _mm256_sub_ps(zero, result), negative);
    _mm256_storeu_ps(out + i, result);
  }
  for (; i < n; i++) {
    out[i] = Tanh(in[i]);
  }
}

// Computes out[i] = Logistic(in[i]) for n values. in and out can be the same.
void LogisticVectorAVX2(int n, const float *in, float *out) {
  const __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff));
  const __m256 one = _mm256_set1_ps(1.0f);
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m256 x = _mm256_loadu_ps(in + i);
    __m256 result = Lookup(LogisticTable, _mm256_and_ps(x, abs_mask));
    // Logistic(x) = 1 - Logistic(-x) for x < 0.
    __m256 negative = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_LT_OQ);
    result = _mm256_blendv_ps(result, _mm256_sub_ps(one, result), negative);
    _mm256_storeu_ps(out + i, result);
  }
  for (; i < n; i++) {
    out[i] = Logistic(in[i]);
  }
}

// Computes out[i] = IntCastRounded(ClipToRange(in[i] * INT8_MAX, -INT8_MAX,
// INT8_MAX)) for n values.
void FloatToInt8VectorAVX2(int n, const float *in, int8_t *out) {
  const __m256 scale = _mm256_set1_ps(INT8_MAX);
  const __m256 lower = _mm256_set1_ps(-INT8_MAX);
  const __m256 sign_mask = _mm256_castsi256_ps(_mm256_set1_epi32(INT32_MIN));
  const __m256 half = _mm256_set1_ps(0.5f);
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m256 x = _mm256_mul_ps(_mm256_loadu_ps(in + i), scale);
    // NaN gives INT8_MAX.
    x = _mm256_max_ps(_mm256_min_ps(x, scale), lower);
    // Round half away from zero: add 0.5 with the sign of x and truncate.
    x = _mm256_add_ps(x, _mm256_or_ps(half, _mm256_and_ps(x, sign_mask)));
    __m256i value = _mm256_cvttps_epi32(x);
    __m128i value16 = _mm_packs_epi32(_mm256_castsi256_si128(value),
                                      _mm256_extracti128_si256(value, 1));
    _mm_storel_epi64(reinterpret_cast<__m128i *>(out + i),
                     _mm_packs_epi16(value16, value16));
  }
  for (; i < n; i++) {
    out[i] = IntCastRounded(
        ClipToRange<float>(in[i] * INT8_MAX, -INT8_MAX, INT8_MAX));
  }
}

#  endif // FAST_FLOAT

} // namespace tesseract.

#endif
