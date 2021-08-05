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

namespace tesseract {

// Number of outputs held in each register. (Actually, we use a
// pair of 4x32 registers, so 8 x 32 bit ints).
constexpr int kNumOutputsPerRegister = 8;
// Maximum number of registers that we will use.
constexpr int kMaxOutputRegisters = 1;
// Number of inputs in the inputs register.
constexpr int kNumInputsPerRegister = 8;
// Number of inputs in each weight group.
constexpr int kNumInputsPerGroup = 8;

// Function to compute part of a matrix.vector multiplication. The weights
// are in a very specific order (see above) in w, which is multiplied by
// u of length num_in, to produce output v after scaling the integer results
// by the corresponding member of scales.
// The amount of w and scales consumed is fixed and not available to the
// caller.

// Computes part of matrix.vector v = Wu. Computes N=8 results.
// The weights *must* be arranged so that consecutive reads from wi
// provides (num_in/kNumInputsPerGroup groups of (N output dim groups of
// (kNumInputsPerGroup inputs))). After that there must be N consecutive
// bias weights, before continuing with any more weights.
// u must be padded out with zeros to
// kNumInputsPerGroup*ceil(num_in/kNumInputsPerGroup) elements.
static inline void PartialMatrixDotVector8(const int8_t *__restrict wi,
                                           const TFloat *__restrict scales,
                                           const int8_t *__restrict u, int num_in,
                                           TFloat *__restrict v, int num_out) {
  // Initialize all the results to 0.
  int32x4_t result0123 = {0, 0, 0, 0};
  int32x4_t result4567 = {0, 0, 0, 0};
  int8x8_t bias_scale = {127, 127, 127, 127, 127, 127, 127, 127};
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in; j += 8) {
    int8x8_t vu = vld1_s8(u);              // vu     = u0  u1  u2  u3  u4  u5  u6  u7
    int8x16_t vw01 = vld1q_s8(wi);         // vw0    = w00 w01 w02 w03 w04 w05 w06 w07
                                           // w10 w11 w12 w13 w14 w15 w16 w17
    int8x16_t vw23 = vld1q_s8(wi + 8 * 2); // vw2    = w20 w21 w22 w23 w24 w25 w26 w27 w30
                                           // w31 w32 w33 w34 w35 w36 w37
    int8x16_t vw45 = vld1q_s8(wi + 8 * 4); // vw4    = w40 w41 w42 w43 w44 w45 w46 w47 w50
                                           // w51 w52 w53 w54 w55 w56 w57
    int8x16_t vw67 = vld1q_s8(wi + 8 * 6); // vw6    = w60 w61 w62 w63 w64 w65 w66 w67 w70
                                           // w71 w72 w73 w74 w75 w76 w77

    int16x8_t vrow0q = vmull_s8(vget_low_s8(vw01), vu); // vrow0q = vw00.u0 w01.u1 w02.u2
                                                        // w03.u3 vw04.u4 w05.u5 w06.u6 w07.u7
    int16x8_t vrow1q = vmull_s8(vget_high_s8(vw01),
                                vu);                    // vrow1q = vw10.u0 w11.u1 w12.u2 w13.u3
                                                        // vw14.u4 w15.u5 w16.u6 w17.u7
    int16x8_t vrow2q = vmull_s8(vget_low_s8(vw23), vu); // vrow2q = vw20.u0 w21.u1 w22.u2
                                                        // w23.u3 vw24.u4 w25.u5 w26.u6 w27.u7
    int16x8_t vrow3q = vmull_s8(vget_high_s8(vw23),
                                vu);                    // vrow3q = vw30.u0 w31.u1 w32.u2 w33.u3
                                                        // vw34.u4 w35.u5 w36.u6 w37.u7
    int16x8_t vrow4q = vmull_s8(vget_low_s8(vw45), vu); // vrow4q = vw40.u0 w41.u1 w42.u2
                                                        // w43.u3 vw44.u4 w45.u5 w46.u6 w47.u7
    int16x8_t vrow5q = vmull_s8(vget_high_s8(vw45),
                                vu);                    // vrow5q = vw50.u0 w51.u1 w52.u2 w53.u3
                                                        // vw54.u4 w55.u5 w56.u6 w57.u7
    int16x8_t vrow6q = vmull_s8(vget_low_s8(vw67), vu); // vrow6q = vw60.u0 w61.u1 w62.u2
                                                        // w63.u3 vw64.u4 w65.u5 w66.u6 w67.u7
    int16x8_t vrow7q = vmull_s8(vget_high_s8(vw67),
                                vu); // vrow7q = vw70.u0 w71.u1 w72.u2 w73.u3
                                     // vw74.u4 w75.u5 w76.u6 w77.u7

    int32x4_t vrow0q2 = vpaddlq_s16(vrow0q); // vrow0q2 = vw00.u0+w01.u1 w02.u2+w03.u3
                                             // vw04.u4+w05.u5 w06.u6+w07.u7
    int32x4_t vrow1q2 = vpaddlq_s16(vrow1q); // vrow1q2 = vw10.u0+w11.u1 w12.u2+w13.u3
                                             // vw14.u4+w15.u5 w16.u6+w17.u7
    int32x4_t vrow2q2 = vpaddlq_s16(vrow2q); // vrow2q2 = vw20.u0+w21.u1 w22.u2+w23.u3
                                             // vw24.u4+w25.u5 w26.u6+w27.u7
    int32x4_t vrow3q2 = vpaddlq_s16(vrow3q); // vrow3q2 = vw30.u0+w31.u1 w32.u2+w33.u3
                                             // vw34.u4+w35.u5 w36.u6+w37.u7
    int32x4_t vrow4q2 = vpaddlq_s16(vrow4q); // vrow4q2 = vw40.u0+w41.u1 w42.u2+w43.u3
                                             // vw44.u4+w45.u5 w46.u6+w47.u7
    int32x4_t vrow5q2 = vpaddlq_s16(vrow5q); // vrow5q2 = vw50.u0+w51.u1 w52.u2+w53.u3
                                             // vw54.u4+w55.u5 w56.u6+w57.u7
    int32x4_t vrow6q2 = vpaddlq_s16(vrow6q); // vrow6q2 = vw60.u0+w61.u1 w62.u2+w63.u3
                                             // vw64.u4+w65.u5 w66.u6+w67.u7
    int32x4_t vrow7q2 = vpaddlq_s16(vrow7q); // vrow7q2 = vw70.u0+w71.u1 w72.u2+w73.u3
                                             // vw74.u4+w75.u5 w76.u6+w77.u7

    vrow0q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow0q2), vget_high_s32(vrow0q2)),
                           vpadd_s32(vget_low_s32(vrow1q2), vget_high_s32(vrow1q2)));
    // vrow0q2 = vw00.u0+...+w03.u3 vw04.u4+...+w07.u7 vw10.u0+...+w13.u3
    // vw14.u4+...+w17.u7
    vrow2q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow2q2), vget_high_s32(vrow2q2)),
                           vpadd_s32(vget_low_s32(vrow3q2), vget_high_s32(vrow3q2)));
    // vrow0q2 = vw20.u0+...+w23.u3 vw24.u4+...+w27.u7 vw30.u0+...+w33.u3
    // vw34.u4+...+w37.u7
    vrow4q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow4q2), vget_high_s32(vrow4q2)),
                           vpadd_s32(vget_low_s32(vrow5q2), vget_high_s32(vrow5q2)));
    // vrow0q2 = vw40.u0+...+w43.u3 vw44.u4+...+w47.u7 vw50.u0+...+w53.u3
    // vw54.u4+...+w57.u7
    vrow6q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow6q2), vget_high_s32(vrow6q2)),
                           vpadd_s32(vget_low_s32(vrow7q2), vget_high_s32(vrow7q2)));
    // vrow0q2 = vw60.u0+...+w63.u3 vw64.u4+...+w67.u7 vw70.u0+...+w73.u3
    // vw74.u4+...+w77.u7

    vrow0q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow0q2), vget_high_s32(vrow0q2)),
                           vpadd_s32(vget_low_s32(vrow2q2), vget_high_s32(vrow2q2)));
    // vrow0q2 = vw00.u0+...+w07.u7 vw10.u0+...+w17.u7 vw20.u0+...+w27.u7
    // vw30.u0+...+w37.u7
    vrow4q2 = vcombine_s32(vpadd_s32(vget_low_s32(vrow4q2), vget_high_s32(vrow4q2)),
                           vpadd_s32(vget_low_s32(vrow6q2), vget_high_s32(vrow6q2)));
    // vrow0q2 = vw40.u0+...+w47.u7 vw50.u0+...+w57.u7 vw60.u0+...+w67.u7
    // vw70.u0+...+w77.u7

    result0123 = vaddq_s32(result0123, vrow0q2);
    result4567 = vaddq_s32(result4567, vrow4q2);
    u += 8;
    wi += 64;
  }
  {
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
}

static void matrixDotVector(int dim1, int dim2, const int8_t *wi, const TFloat *scales,
                            const int8_t *u, TFloat *v) {
  const int num_out = dim1;
  const int num_in = dim2 - 1;
  // Each call to a partial_func_ produces group_size outputs, except the
  // last one, which can produce less.
  const int rounded_num_in = IntSimdMatrix::Roundup(num_in, kNumInputsPerGroup);
  int group_size = kNumOutputsPerRegister * kMaxOutputRegisters;
  int output = 0;

  int w_step = (rounded_num_in + 1) * group_size;

  for (; output + group_size <= num_out; output += group_size) {
    PartialMatrixDotVector8(wi, scales, u, rounded_num_in, v, kNumOutputsPerRegister);
    wi += w_step;
    scales += group_size;
    v += group_size;
  }
  if (output < num_out)
    PartialMatrixDotVector8(wi, scales, u, rounded_num_in, v,
                            num_out & (kNumOutputsPerRegister - 1));
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

} // namespace tesseract.

#endif /* __ARM_NEON */
