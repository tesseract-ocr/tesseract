///////////////////////////////////////////////////////////////////////
// File:        dotproductneon.cpp
// Description: Dot product function for ARM NEON.
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

#if defined(__ARM_NEON)

#include <arm_neon.h>
#include <cstddef> // for size_t
#include "dotproduct.h"

namespace tesseract {

// Documentation:
// https://developer.arm.com/architectures/instruction-sets/intrinsics/

#if defined(FAST_FLOAT) && defined(__ARM_ARCH_ISA_A64)

float DotProductNEON(const float *u, const float *v, int n) {
  float32x4_t result0123 = vdupq_n_f32(0.0f);
  float32x4_t result4567 = vdupq_n_f32(0.0f);
  while (n > 7) {
    // Calculate 8 dot products per iteration.
    float32x4_t u0 = vld1q_f32(u);
    float32x4_t v0 = vld1q_f32(v);
    float32x4_t u4 = vld1q_f32(u + 4);
    float32x4_t v4 = vld1q_f32(v + 4);
    result0123 = vfmaq_f32(result0123, u0, v0);
    result4567 = vfmaq_f32(result4567, u4, v4);
    u += 8;
    v += 8;
    n -= 8;
  }
  float total = vaddvq_f32(result0123);
  total += vaddvq_f32(result4567);
  while (n > 0) {
    total += *u++ * *v++;
    n--;
  }
  return total;
}

// Computes v[i] = w[i] . u for num_rows rows of w with n used elements each.
// Four rows are handled per pass, so u is loaded only once for them and
// eight independent sums are calculated at the same time.
void RowDotProductsNEON(const float *w, int w_stride, int num_rows,
                        const float *u, int n, float *v) {
  int i = 0;
  for (; i + 4 <= num_rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * w_stride;
    const float *w1 = w0 + w_stride;
    const float *w2 = w1 + w_stride;
    const float *w3 = w2 + w_stride;
    float32x4_t a0 = vdupq_n_f32(0.0f);
    float32x4_t a1 = a0, a2 = a0, a3 = a0;
    float32x4_t b0 = a0, b1 = a0, b2 = a0, b3 = a0;
    int j = 0;
    for (; j + 8 <= n; j += 8) {
      float32x4_t x = vld1q_f32(u + j);
      float32x4_t y = vld1q_f32(u + j + 4);
      a0 = vfmaq_f32(a0, vld1q_f32(w0 + j), x);
      b0 = vfmaq_f32(b0, vld1q_f32(w0 + j + 4), y);
      a1 = vfmaq_f32(a1, vld1q_f32(w1 + j), x);
      b1 = vfmaq_f32(b1, vld1q_f32(w1 + j + 4), y);
      a2 = vfmaq_f32(a2, vld1q_f32(w2 + j), x);
      b2 = vfmaq_f32(b2, vld1q_f32(w2 + j + 4), y);
      a3 = vfmaq_f32(a3, vld1q_f32(w3 + j), x);
      b3 = vfmaq_f32(b3, vld1q_f32(w3 + j + 4), y);
    }
    if (j + 4 <= n) {
      float32x4_t x = vld1q_f32(u + j);
      a0 = vfmaq_f32(a0, vld1q_f32(w0 + j), x);
      a1 = vfmaq_f32(a1, vld1q_f32(w1 + j), x);
      a2 = vfmaq_f32(a2, vld1q_f32(w2 + j), x);
      a3 = vfmaq_f32(a3, vld1q_f32(w3 + j), x);
      j += 4;
    }
    float total0 = vaddvq_f32(vaddq_f32(a0, b0));
    float total1 = vaddvq_f32(vaddq_f32(a1, b1));
    float total2 = vaddvq_f32(vaddq_f32(a2, b2));
    float total3 = vaddvq_f32(vaddq_f32(a3, b3));
    for (; j < n; j++) {
      total0 += w0[j] * u[j];
      total1 += w1[j] * u[j];
      total2 += w2[j] * u[j];
      total3 += w3[j] * u[j];
    }
    v[i] = total0;
    v[i + 1] = total1;
    v[i + 2] = total2;
    v[i + 3] = total3;
  }
  for (; i < num_rows; i++) {
    v[i] = DotProductNEON(w + static_cast<size_t>(i) * w_stride, u, n);
  }
}

// Computes out[i * out_stride + j] = u[i] . v[j] for the num_u rows of u and
// the num_v rows of v, which all have n elements and are stored without gaps.
// Four rows of u and four rows of v are handled per pass, so each of them is
// loaded only once for 16 sums which are calculated at the same time.
// The accumulators are named variables instead of an array, because some
// compilers do not keep an array of vectors in registers.
void CrossDotProductsNEON(const float *u, int num_u, const float *v, int num_v,
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
    for (; j + 4 <= num_v; j += 4) {
      const float *v0 = v + static_cast<size_t>(j) * n;
      const float *v1 = v0 + n;
      const float *v2 = v1 + n;
      const float *v3 = v2 + n;
      float32x4_t a0 = vdupq_n_f32(0.0f);
      float32x4_t a1 = a0, a2 = a0, a3 = a0;
      float32x4_t b0 = a0, b1 = a0, b2 = a0, b3 = a0;
      float32x4_t c0 = a0, c1 = a0, c2 = a0, c3 = a0;
      float32x4_t d0 = a0, d1 = a0, d2 = a0, d3 = a0;
      int k = 0;
      for (; k + 4 <= n; k += 4) {
        float32x4_t y0 = vld1q_f32(v0 + k);
        float32x4_t y1 = vld1q_f32(v1 + k);
        float32x4_t y2 = vld1q_f32(v2 + k);
        float32x4_t y3 = vld1q_f32(v3 + k);
        float32x4_t x = vld1q_f32(u0 + k);
        a0 = vfmaq_f32(a0, x, y0);
        a1 = vfmaq_f32(a1, x, y1);
        a2 = vfmaq_f32(a2, x, y2);
        a3 = vfmaq_f32(a3, x, y3);
        x = vld1q_f32(u1 + k);
        b0 = vfmaq_f32(b0, x, y0);
        b1 = vfmaq_f32(b1, x, y1);
        b2 = vfmaq_f32(b2, x, y2);
        b3 = vfmaq_f32(b3, x, y3);
        x = vld1q_f32(u2 + k);
        c0 = vfmaq_f32(c0, x, y0);
        c1 = vfmaq_f32(c1, x, y1);
        c2 = vfmaq_f32(c2, x, y2);
        c3 = vfmaq_f32(c3, x, y3);
        x = vld1q_f32(u3 + k);
        d0 = vfmaq_f32(d0, x, y0);
        d1 = vfmaq_f32(d1, x, y1);
        d2 = vfmaq_f32(d2, x, y2);
        d3 = vfmaq_f32(d3, x, y3);
      }
      float total[4][4] = {
          {vaddvq_f32(a0), vaddvq_f32(a1), vaddvq_f32(a2), vaddvq_f32(a3)},
          {vaddvq_f32(b0), vaddvq_f32(b1), vaddvq_f32(b2), vaddvq_f32(b3)},
          {vaddvq_f32(c0), vaddvq_f32(c1), vaddvq_f32(c2), vaddvq_f32(c3)},
          {vaddvq_f32(d0), vaddvq_f32(d1), vaddvq_f32(d2), vaddvq_f32(d3)}};
      for (; k < n; k++) {
        const float x[4] = {u0[k], u1[k], u2[k], u3[k]};
        const float y[4] = {v0[k], v1[k], v2[k], v3[k]};
        for (int r = 0; r < 4; r++) {
          for (int c = 0; c < 4; c++) {
            total[r][c] += x[r] * y[c];
          }
        }
      }
      for (int c = 0; c < 4; c++) {
        out0[j + c] = total[0][c];
        out1[j + c] = total[1][c];
        out2[j + c] = total[2][c];
        out3[j + c] = total[3][c];
      }
    }
    for (; j < num_v; j++) {
      const float *vj = v + static_cast<size_t>(j) * n;
      out0[j] = DotProductNEON(u0, vj, n);
      out1[j] = DotProductNEON(u1, vj, n);
      out2[j] = DotProductNEON(u2, vj, n);
      out3[j] = DotProductNEON(u3, vj, n);
    }
  }
  // The remaining rows of u are single vectors for the rows of v.
  for (; i < num_u; i++) {
    RowDotProductsNEON(v, n, num_v, u + static_cast<size_t>(i) * n, n,
                       out + static_cast<size_t>(i) * out_stride);
  }
}

#else

// Computes and returns the dot product of the two n-vectors u and v.
TFloat DotProductNEON(const TFloat *u, const TFloat *v, int n) {
  TFloat total = 0;
#if defined(OPENMP_SIMD) || defined(_OPENMP)
#pragma omp simd reduction(+:total)
#endif
  for (int k = 0; k < n; k++) {
    total += u[k] * v[k];
  }
  return total;
}

#endif

} // namespace tesseract

#endif /* __ARM_NEON */
