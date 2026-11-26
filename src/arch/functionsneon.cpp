///////////////////////////////////////////////////////////////////////
// File:        functionsneon.cpp
// Description: Non-linearity functions and conversion of floats to
//              int8_t for whole vectors with NEON.
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

#if !defined(__ARM_NEON)
#  if defined(__aarch64__) || defined(__arm__)
#    error Implementation only for NEON capable architectures
#  endif
#else

#  include <arm_neon.h>
#  include <cstdint>
#  include "dotproduct.h"
#  include "functions.h" // for Tanh, Logistic, TanhTable, LogisticTable

namespace tesseract {

// The functions give exactly the same results as the scalar functions Tanh,
// Logistic and IntCastRounded, because they do the same calculations in the
// same order. The linear interpolation is written as the same expression in
// the NEON code as in the scalar code, t0 + (t1 - t0) * fraction, so that the
// compiler applies the same FMA contraction to both and the results stay
// identical, whatever the compiler decides to do. This is only used for
// ARM64.
#  if defined(FAST_FLOAT) && defined(__ARM_ARCH_ISA_A64)

// Returns the interpolated values of the table (TanhTable or LogisticTable)
// for the 4 values of a, which must not be negative, like the scalar
// functions do for x >= 0.
static inline float32x4_t Lookup(const float *table, float32x4_t a) {
  // Values which are too large for the table (the result is 1 for them) are
  // limited first, so the indices are always valid and nothing overflows,
  // also for huge values and for NaN (vminq_f32 returns the second value
  // then).
  const float32x4_t limit = vdupq_n_f32((kTableSize - 1) / kScaleFactor);
  float32x4_t clamped = vminq_f32(a, limit);
  float32x4_t scaled = vmulq_f32(clamped, vdupq_n_f32(kScaleFactor));
  int32x4_t index =
      vminq_s32(vcvtq_s32_f32(scaled), vdupq_n_s32(kTableSize - 2));
  // Each value gives table[i] and table[i + 1]. Loading the table entries one
  // by one is faster than the gather instruction.
  int32_t i[4];
  vst1q_s32(i, index);
  float t0[4] = {table[i[0]], table[i[1]], table[i[2]], table[i[3]]};
  float t1[4] = {table[i[0] + 1], table[i[1] + 1], table[i[2] + 1],
                 table[i[3] + 1]};
  float32x4_t t0_vec = vld1q_f32(t0);
  float32x4_t t1_vec = vld1q_f32(t1);
  float32x4_t fraction = vsubq_f32(scaled, vcvtq_f32_s32(index));
  // Linear interpolation. This is written as the same expression as the
  // scalar code, t0 + (t1 - t0) * fraction, instead of using a NEON
  // intrinsic, so that the compiler applies the same FMA contraction to it as
  // to the scalar code and the results stay identical.
  float32x4_t result = t0_vec + (t1_vec - t0_vec) * fraction;
  // The result is 1 for values which are too large for the table.
  uint32x4_t too_large = reinterpret_cast<uint32x4_t>(vcgeq_f32(a, limit));
  return vbslq_f32(too_large, vdupq_n_f32(1.0f), result);
}

// Computes out[i] = Tanh(in[i]) for n values. in and out can be the same.
void TanhVectorNEON(int n, const float *in, float *out) {
  const uint32x4_t abs_mask = vdupq_n_u32(0x7fffffff);
  const float32x4_t zero = vdupq_n_f32(0.0f);
  int i = 0;
  for (; i + 4 <= n; i += 4) {
    float32x4_t x = vld1q_f32(in + i);
    // The sign bits are removed to use the table for the absolute value.
    float32x4_t a =
        vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(x), abs_mask));
    float32x4_t result = Lookup(TanhTable, a);
    // Tanh(x) = -Tanh(-x) for x < 0.
    uint32x4_t negative = reinterpret_cast<uint32x4_t>(vcltq_f32(x, zero));
    result = vbslq_f32(negative, vsubq_f32(zero, result), result);
    vst1q_f32(out + i, result);
  }
  for (; i < n; i++) {
    out[i] = Tanh(in[i]);
  }
}

// Computes out[i] = Logistic(in[i]) for n values. in and out can be the same.
void LogisticVectorNEON(int n, const float *in, float *out) {
  const uint32x4_t abs_mask = vdupq_n_u32(0x7fffffff);
  const float32x4_t one = vdupq_n_f32(1.0f);
  const float32x4_t zero = vdupq_n_f32(0.0f);
  int i = 0;
  for (; i + 4 <= n; i += 4) {
    float32x4_t x = vld1q_f32(in + i);
    // The sign bits are removed to use the table for the absolute value.
    float32x4_t a =
        vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(x), abs_mask));
    float32x4_t result = Lookup(LogisticTable, a);
    // Logistic(x) = 1 - Logistic(-x) for x < 0.
    uint32x4_t negative = reinterpret_cast<uint32x4_t>(vcltq_f32(x, zero));
    result = vbslq_f32(negative, vsubq_f32(one, result), result);
    vst1q_f32(out + i, result);
  }
  for (; i < n; i++) {
    out[i] = Logistic(in[i]);
  }
}

// Computes out[i] = IntCastRounded(ClipToRange(in[i] * INT8_MAX, -INT8_MAX,
// INT8_MAX)) for n values.
void FloatToInt8VectorNEON(int n, const float *in, int8_t *out) {
  const float32x4_t scale = vdupq_n_f32(INT8_MAX);
  const float32x4_t lower = vdupq_n_f32(-INT8_MAX);
  const float32x4_t half = vdupq_n_f32(0.5f);
  const float32x4_t zero = vdupq_n_f32(0.0f);
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    float32x4_t y0 = vmulq_f32(vld1q_f32(in + i), scale);
    float32x4_t y1 = vmulq_f32(vld1q_f32(in + i + 4), scale);
    // NaN gives INT8_MAX.
    y0 = vmaxq_f32(vminq_f32(y0, scale), lower);
    y1 = vmaxq_f32(vminq_f32(y1, scale), lower);
    // Round half away from zero: truncate |y| + 0.5 and keep the sign.
    // (vcvtq_s32_f32 truncates toward zero, like the cast to int in the
    // scalar function.)
    int32x4_t r0pos = vcvtq_s32_f32(vaddq_f32(y0, half));
    int32x4_t r0neg = vcvtq_s32_f32(vaddq_f32(vnegq_f32(y0), half));
    uint32x4_t m0 = reinterpret_cast<uint32x4_t>(vcgeq_f32(y0, zero));
    int32x4_t s0 = vbslq_s32(m0, r0pos, vnegq_s32(r0neg));
    int32x4_t r1pos = vcvtq_s32_f32(vaddq_f32(y1, half));
    int32x4_t r1neg = vcvtq_s32_f32(vaddq_f32(vnegq_f32(y1), half));
    uint32x4_t m1 = reinterpret_cast<uint32x4_t>(vcgeq_f32(y1, zero));
    int32x4_t s1 = vbslq_s32(m1, r1pos, vnegq_s32(r1neg));
    // Values are in [-127, 127], so the narrowing conversions do not
    // saturate.
    vst1_s8(out + i, vqmovn_s16(vcombine_s16(vqmovn_s32(s0), vqmovn_s32(s1))));
  }
  for (; i < n; i++) {
    out[i] = IntCastRounded(
        ClipToRange<float>(in[i] * INT8_MAX, -INT8_MAX, INT8_MAX));
  }
}

#  endif // FAST_FLOAT && __ARM_ARCH_ISA_A64

} // namespace tesseract.

#endif
