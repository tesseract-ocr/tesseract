///////////////////////////////////////////////////////////////////////
// File:        intsimdmatrixneon.cpp
// Description: matrix-vector product for 8-bit data on neon.
// Author:      Robin Watts (from the AVX2 original by Ray Smith)
//
// (C) Copyright 2017, Google Inc.
// (C) Copyright 2020, Artifex Software Inc.
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

#  include "intsimdmatrix.h"
#  include "tesstypes.h"

#  include <algorithm>
#  include <cstdint>
#  include <vector>
#  include "arm_neon.h"

// The dot product instruction SDOT is an optional feature of ARMv8.2.
// A function with a target attribute can use it even if the rest of the code
// is built for a CPU without it. Such a function must only be called if the
// CPU supports the instruction (see SIMDDetect).
#  if defined(__ARM_FEATURE_DOTPROD)
// The compiler may use the instruction anywhere (for example on macOS).
#    define HAVE_NEON_DOTPROD
#    define TARGET_DOTPROD
#  elif defined(__aarch64__) && defined(__clang__)
#    if __clang_major__ >= 17
#      define HAVE_NEON_DOTPROD
#      define TARGET_DOTPROD __attribute__((target("+dotprod")))
#    endif
#  elif defined(__aarch64__) && defined(__GNUC__)
#    if __GNUC__ >= 10
#      define HAVE_NEON_DOTPROD
#      define TARGET_DOTPROD __attribute__((target("+dotprod")))
#    endif
#  endif

namespace tesseract {

// Number of outputs which are computed in one pass over the inputs. Each of
// them uses one register for four partial sums.
constexpr int kNumOutputsPerRegister = 8;
// Maximum number of registers that we will use.
constexpr int kMaxOutputRegisters = 1;
// Number of inputs in the inputs register.
constexpr int kNumInputsPerRegister = 16;
// Number of inputs in each weight group.
constexpr int kNumInputsPerGroup = 16;

// Function to compute part of a matrix.vector multiplication. The weights
// are in a very specific order (see above) in w, which is multiplied by
// u of length num_in, to produce output v after scaling the integer results
// by the corresponding member of scales.
// The amount of w and scales consumed is fixed and not available to the
// caller.
using PartialFunction = void (*)(const int8_t *wi, const TFloat *scales,
                                 const int8_t *u, int num_in, TFloat *v,
                                 int num_out);

// Returns the sums of the four elements of each of the four arguments.
static inline int32x4_t HorizontalSums(int32x4_t a, int32x4_t b, int32x4_t c,
                                       int32x4_t d) {
  int32x2_t a2 = vpadd_s32(vget_low_s32(a), vget_high_s32(a)); // a0+a1 a2+a3
  int32x2_t b2 = vpadd_s32(vget_low_s32(b), vget_high_s32(b));
  int32x2_t c2 = vpadd_s32(vget_low_s32(c), vget_high_s32(c));
  int32x2_t d2 = vpadd_s32(vget_low_s32(d), vget_high_s32(d));
  return vcombine_s32(vpadd_s32(a2, b2), vpadd_s32(c2, d2));
}

// Adds the bias weights (which follow the other weights in wi) to the 8
// integer results, scales them and writes num_out of them to v.
static inline void WriteResults(int32x4_t result0123, int32x4_t result4567,
                                const int8_t *wi, const TFloat *scales,
                                TFloat *v, int num_out) {
  int8x8_t bias_scale = {127, 127, 127, 127, 127, 127, 127, 127};
  int8x8_t bias = vld1_s8(wi); // vw0    = b0  b1  b2  b3  b4  b5  b6  b7
  int16x8_t scaled_bias = vmull_s8(bias, bias_scale);
  result0123 = vaddw_s16(result0123, vget_low_s16(scaled_bias));
  result4567 = vaddw_s16(result4567, vget_high_s16(scaled_bias));
  *v++ = vget_lane_s32(vget_low_s32(result0123), 0) * *scales++;
  if (num_out > 1)
    *v++ = vget_lane_s32(vget_low_s32(result0123), 1) * *scales++;
  if (num_out > 2)
    *v++ = vget_lane_s32(vget_high_s32(result0123), 0) * *scales++;
  if (num_out > 3)
    *v++ = vget_lane_s32(vget_high_s32(result0123), 1) * *scales++;
  if (num_out > 4)
    *v++ = vget_lane_s32(vget_low_s32(result4567), 0) * *scales++;
  if (num_out > 5)
    *v++ = vget_lane_s32(vget_low_s32(result4567), 1) * *scales++;
  if (num_out > 6)
    *v++ = vget_lane_s32(vget_high_s32(result4567), 0) * *scales++;
  if (num_out > 7)
    *v = vget_lane_s32(vget_high_s32(result4567), 1) * *scales;
}

// Adds the 16 products of the elements of w and u to the four partial sums
// in result. The products are widened to 32 bits when they are added, so the
// result is exact for all values of w and u.
static inline int32x4_t AddProducts(int32x4_t result, int8x16_t w,
                                    int8x8_t u_low, int8x8_t u_high) {
  result = vpadalq_s16(result, vmull_s8(vget_low_s8(w), u_low));
  return vpadalq_s16(result, vmull_s8(vget_high_s8(w), u_high));
}

// Computes part of matrix.vector v = Wu. Computes N=8 results.
// The weights *must* be arranged so that consecutive reads from wi
// provides (num_in/kNumInputsPerGroup groups of (N output dim groups of
// (kNumInputsPerGroup inputs))). After that there must be N consecutive
// bias weights, before continuing with any more weights.
// u must be padded out to
// kNumInputsPerGroup*ceil(num_in/kNumInputsPerGroup) elements. The weights
// for the padding are 0, so the values of the padding do not matter.
static void PartialMatrixDotVector8(const int8_t *__restrict wi,
                                    const TFloat *__restrict scales,
                                    const int8_t *__restrict u, int num_in,
                                    TFloat *__restrict v, int num_out) {
  // Initialize all the results to 0.
  int32x4_t result0 = vdupq_n_s32(0);
  int32x4_t result1 = result0;
  int32x4_t result2 = result0;
  int32x4_t result3 = result0;
  int32x4_t result4 = result0;
  int32x4_t result5 = result0;
  int32x4_t result6 = result0;
  int32x4_t result7 = result0;
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in; j += kNumInputsPerGroup) {
    int8x16_t vu = vld1q_s8(u);
    int8x8_t vu_low = vget_low_s8(vu);
    int8x8_t vu_high = vget_high_s8(vu);
    result0 = AddProducts(result0, vld1q_s8(wi), vu_low, vu_high);
    result1 = AddProducts(result1, vld1q_s8(wi + 16 * 1), vu_low, vu_high);
    result2 = AddProducts(result2, vld1q_s8(wi + 16 * 2), vu_low, vu_high);
    result3 = AddProducts(result3, vld1q_s8(wi + 16 * 3), vu_low, vu_high);
    result4 = AddProducts(result4, vld1q_s8(wi + 16 * 4), vu_low, vu_high);
    result5 = AddProducts(result5, vld1q_s8(wi + 16 * 5), vu_low, vu_high);
    result6 = AddProducts(result6, vld1q_s8(wi + 16 * 6), vu_low, vu_high);
    result7 = AddProducts(result7, vld1q_s8(wi + 16 * 7), vu_low, vu_high);
    u += kNumInputsPerGroup;
    wi += kNumInputsPerGroup * kNumOutputsPerRegister;
  }
  WriteResults(HorizontalSums(result0, result1, result2, result3),
               HorizontalSums(result4, result5, result6, result7), wi, scales,
               v, num_out);
}

#  if defined(HAVE_NEON_DOTPROD)

// Same as PartialMatrixDotVector8, but uses the dot product instruction SDOT,
// which multiplies 16 pairs of values and adds the products to four partial
// sums at once.
TARGET_DOTPROD static void PartialMatrixDotVector8DotProd(
    const int8_t *__restrict wi, const TFloat *__restrict scales,
    const int8_t *__restrict u, int num_in, TFloat *__restrict v, int num_out) {
  // Initialize all the results to 0.
  int32x4_t result0 = vdupq_n_s32(0);
  int32x4_t result1 = result0;
  int32x4_t result2 = result0;
  int32x4_t result3 = result0;
  int32x4_t result4 = result0;
  int32x4_t result5 = result0;
  int32x4_t result6 = result0;
  int32x4_t result7 = result0;
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in; j += kNumInputsPerGroup) {
    int8x16_t vu = vld1q_s8(u);
    result0 = vdotq_s32(result0, vld1q_s8(wi), vu);
    result1 = vdotq_s32(result1, vld1q_s8(wi + 16 * 1), vu);
    result2 = vdotq_s32(result2, vld1q_s8(wi + 16 * 2), vu);
    result3 = vdotq_s32(result3, vld1q_s8(wi + 16 * 3), vu);
    result4 = vdotq_s32(result4, vld1q_s8(wi + 16 * 4), vu);
    result5 = vdotq_s32(result5, vld1q_s8(wi + 16 * 5), vu);
    result6 = vdotq_s32(result6, vld1q_s8(wi + 16 * 6), vu);
    result7 = vdotq_s32(result7, vld1q_s8(wi + 16 * 7), vu);
    u += kNumInputsPerGroup;
    wi += kNumInputsPerGroup * kNumOutputsPerRegister;
  }
  WriteResults(HorizontalSums(result0, result1, result2, result3),
               HorizontalSums(result4, result5, result6, result7), wi, scales,
               v, num_out);
}

#  endif // HAVE_NEON_DOTPROD

// Computes matrix.vector v = Wu with the given function for the parts.
static inline void MatrixDotVectorPartial(PartialFunction partial_func,
                                          int dim1, int dim2, const int8_t *wi,
                                          const TFloat *scales, const int8_t *u,
                                          TFloat *v) {
  const int num_out = dim1;
  const int num_in = dim2 - 1;
  // Each call to a partial_func_ produces group_size outputs, except the
  // last one, which can produce less.
  const int rounded_num_in = IntSimdMatrix::Roundup(num_in, kNumInputsPerGroup);
  int group_size = kNumOutputsPerRegister * kMaxOutputRegisters;
  int output = 0;

  int w_step = (rounded_num_in + 1) * group_size;

  for (; output + group_size <= num_out; output += group_size) {
    partial_func(wi, scales, u, rounded_num_in, v, kNumOutputsPerRegister);
    wi += w_step;
    scales += group_size;
    v += group_size;
  }
  if (output < num_out)
    partial_func(wi, scales, u, rounded_num_in, v,
                 num_out & (kNumOutputsPerRegister - 1));
}

static void matrixDotVector(int dim1, int dim2, const int8_t *wi,
                            const TFloat *scales, const int8_t *u, TFloat *v) {
  MatrixDotVectorPartial(PartialMatrixDotVector8, dim1, dim2, wi, scales, u, v);
}

const IntSimdMatrix IntSimdMatrix::intSimdMatrixNEON = {
    // Function.
    matrixDotVector,
    // Number of 32 bit outputs held in each register.
    kNumOutputsPerRegister,
    // Maximum number of registers that we will use to hold outputs.
    kMaxOutputRegisters,
    // Number of 8 bit inputs in the inputs register.
    kNumInputsPerRegister,
    // Number of inputs in each weight group.
    kNumInputsPerGroup
};

#  if defined(HAVE_NEON_DOTPROD)
static void matrixDotVectorDotProd(int dim1, int dim2, const int8_t *wi,
                                   const TFloat *scales, const int8_t *u,
                                   TFloat *v) {
  MatrixDotVectorPartial(PartialMatrixDotVector8DotProd, dim1, dim2, wi, scales,
                         u, v);
}
#  endif

// The weights are in the same order as for intSimdMatrixNEON.
const IntSimdMatrix IntSimdMatrix::intSimdMatrixNEONDotProd = {
    // Function.
#  if defined(HAVE_NEON_DOTPROD)
    matrixDotVectorDotProd,
#  else
    // The compiler cannot build the code with SDOT.
    matrixDotVector,
#  endif
    // Number of 32 bit outputs held in each register.
    kNumOutputsPerRegister,
    // Maximum number of registers that we will use to hold outputs.
    kMaxOutputRegisters,
    // Number of 8 bit inputs in the inputs register.
    kNumInputsPerRegister,
    // Number of inputs in each weight group.
    kNumInputsPerGroup
};

} // namespace tesseract.

#endif /* __ARM_NEON */
