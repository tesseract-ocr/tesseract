///////////////////////////////////////////////////////////////////////
// File:        neon-int-kernel-variants.cpp
// Description: Microbenchmark for variants of the NEON function which
//              multiplies an int8_t matrix with a vector.
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

// This standalone program (ARM64 only) compares the core loop of
// PartialMatrixDotVector8 in src/arch/intsimdmatrixneon.cpp, which is used
// for the recognition with integer (fast) models, with two variants:
// one for any ARM64 CPU and one with the dot product instruction SDOT,
// which needs ARMv8.2 with the feature "asimddp" (see /proc/cpuinfo).
// The scaling of the results and the bias are not included. All variants
// give the same results, because the calculation with integers is exact.
//
//   c++ -std=c++17 -O3 -o neon-int-kernel-variants neon-int-kernel-variants.cpp
//   ./neon-int-kernel-variants

#include <arm_neon.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>
#define NOINLINE __attribute__((noinline))

// Core loop of PartialMatrixDotVector8 in src/arch/intsimdmatrixneon.cpp
// (8 outputs, weights interleaved in groups of 8 rows x 8 inputs).
NOINLINE void Now8(const int8_t *wi, const int8_t *u, int num_in, int32_t *v) {
  int32x4_t result0123 = {0, 0, 0, 0};
  int32x4_t result4567 = {0, 0, 0, 0};
  for (int j = 0; j < num_in; j += 8) {
    int8x8_t vu = vld1_s8(u);
    int8x16_t vw01 = vld1q_s8(wi);
    int8x16_t vw23 = vld1q_s8(wi + 8 * 2);
    int8x16_t vw45 = vld1q_s8(wi + 8 * 4);
    int8x16_t vw67 = vld1q_s8(wi + 8 * 6);
    int32x4_t r0 = vpaddlq_s16(vmull_s8(vget_low_s8(vw01), vu));
    int32x4_t r1 = vpaddlq_s16(vmull_s8(vget_high_s8(vw01), vu));
    int32x4_t r2 = vpaddlq_s16(vmull_s8(vget_low_s8(vw23), vu));
    int32x4_t r3 = vpaddlq_s16(vmull_s8(vget_high_s8(vw23), vu));
    int32x4_t r4 = vpaddlq_s16(vmull_s8(vget_low_s8(vw45), vu));
    int32x4_t r5 = vpaddlq_s16(vmull_s8(vget_high_s8(vw45), vu));
    int32x4_t r6 = vpaddlq_s16(vmull_s8(vget_low_s8(vw67), vu));
    int32x4_t r7 = vpaddlq_s16(vmull_s8(vget_high_s8(vw67), vu));
    r0 = vcombine_s32(vpadd_s32(vget_low_s32(r0), vget_high_s32(r0)),
                      vpadd_s32(vget_low_s32(r1), vget_high_s32(r1)));
    r2 = vcombine_s32(vpadd_s32(vget_low_s32(r2), vget_high_s32(r2)),
                      vpadd_s32(vget_low_s32(r3), vget_high_s32(r3)));
    r4 = vcombine_s32(vpadd_s32(vget_low_s32(r4), vget_high_s32(r4)),
                      vpadd_s32(vget_low_s32(r5), vget_high_s32(r5)));
    r6 = vcombine_s32(vpadd_s32(vget_low_s32(r6), vget_high_s32(r6)),
                      vpadd_s32(vget_low_s32(r7), vget_high_s32(r7)));
    r0 = vcombine_s32(vpadd_s32(vget_low_s32(r0), vget_high_s32(r0)),
                      vpadd_s32(vget_low_s32(r2), vget_high_s32(r2)));
    r4 = vcombine_s32(vpadd_s32(vget_low_s32(r4), vget_high_s32(r4)),
                      vpadd_s32(vget_low_s32(r6), vget_high_s32(r6)));
    result0123 = vaddq_s32(result0123, r0);
    result4567 = vaddq_s32(result4567, r4);
    u += 8;
    wi += 64;
  }
  vst1q_s32(v, result0123);
  vst1q_s32(v + 4, result4567);
}
NOINLINE void NowMatrix(const int8_t *wi, int rows, int num_in, const int8_t *u,
                        int32_t *v) {
  for (int i = 0; i < rows; i += 8)
    Now8(wi + static_cast<size_t>(i / 8) * num_in * 8, u, num_in, v + i);
}

// Without SDOT: widening multiply and pairwise accumulate, 4 rows per pass.
NOINLINE void Padal4(const int8_t *w, int rows, int num_in, const int8_t *u,
                     int32_t *v) {
  for (int i = 0; i < rows; i += 4) {
    const int8_t *w0 = w + static_cast<size_t>(i) * num_in, *w1 = w0 + num_in,
                 *w2 = w1 + num_in, *w3 = w2 + num_in;
    int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0;
    for (int j = 0; j < num_in; j += 16) {
      int8x16_t x = vld1q_s8(u + j);
      int8x16_t y;
      y = vld1q_s8(w0 + j);
      a0 = vpadalq_s16(
          a0, vmlal_high_s8(vmull_s8(vget_low_s8(y), vget_low_s8(x)), y, x));
      y = vld1q_s8(w1 + j);
      a1 = vpadalq_s16(
          a1, vmlal_high_s8(vmull_s8(vget_low_s8(y), vget_low_s8(x)), y, x));
      y = vld1q_s8(w2 + j);
      a2 = vpadalq_s16(
          a2, vmlal_high_s8(vmull_s8(vget_low_s8(y), vget_low_s8(x)), y, x));
      y = vld1q_s8(w3 + j);
      a3 = vpadalq_s16(
          a3, vmlal_high_s8(vmull_s8(vget_low_s8(y), vget_low_s8(x)), y, x));
    }
    v[i] = vaddvq_s32(a0);
    v[i + 1] = vaddvq_s32(a1);
    v[i + 2] = vaddvq_s32(a2);
    v[i + 3] = vaddvq_s32(a3);
  }
}

// With SDOT (ARMv8.2 dot product), 4 rows per pass, 16 inputs per instruction.
__attribute__((target("arch=armv8.2-a+dotprod"))) NOINLINE void Sdot4(
    const int8_t *w, int rows, int num_in, const int8_t *u, int32_t *v) {
  for (int i = 0; i < rows; i += 4) {
    const int8_t *w0 = w + static_cast<size_t>(i) * num_in, *w1 = w0 + num_in,
                 *w2 = w1 + num_in, *w3 = w2 + num_in;
    int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0;
    for (int j = 0; j < num_in; j += 16) {
      int8x16_t x = vld1q_s8(u + j);
      a0 = vdotq_s32(a0, vld1q_s8(w0 + j), x);
      a1 = vdotq_s32(a1, vld1q_s8(w1 + j), x);
      a2 = vdotq_s32(a2, vld1q_s8(w2 + j), x);
      a3 = vdotq_s32(a3, vld1q_s8(w3 + j), x);
    }
    v[i] = vaddvq_s32(a0);
    v[i + 1] = vaddvq_s32(a1);
    v[i + 2] = vaddvq_s32(a2);
    v[i + 3] = vaddvq_s32(a3);
  }
}

template <class F>
static double BestNs(F f, int reps) {
  double best = 1e30;
  for (int r = 0; r < 9; r++) {
    auto t0 = std::chrono::steady_clock::now();
    for (int k = 0; k < reps; k++)
      f();
    double ns = std::chrono::duration<double, std::nano>(
                    std::chrono::steady_clock::now() - t0)
                    .count() /
                reps;
    if (ns < best)
      best = ns;
  }
  return best;
}

// Returns deterministic pseudo-random numbers with 24 bits for the test data.
// A linear congruential generator gives the same data on all platforms.
static uint32_t Random() {
  static uint32_t state = 1;
  state = state * 1664525u + 1013904223u;
  return state >> 8;
}

int main() {
  struct {
    const char *name;
    int ns, na;
  } layers[] = {{"Lfys48", 48, 64},
                {"Lfx96", 96, 144},
                {"Lrx96", 96, 192},
                {"Lfx192", 192, 288},
                {"Lfx512", 512, 608}};
  printf(
      "int8 W*u (4 gate matrices), us per timestep\n"
      " layer      now  padal   sdot   now/sdot\n");
  for (auto &L : layers) {
    int rows = L.ns, cols = (L.na + 15) / 16 * 16;
    std::vector<int8_t> w(4 * rows * cols), wi(4 * rows * cols), u(cols);
    for (auto &e : w)
      e = Random() % 255 - 127;
    for (auto &e : u)
      e = Random() % 255 - 127;
    // Interleaved copy for the current code: blocks of 8 rows, groups of 8
    // inputs.
    for (int g = 0; g < 4; g++)
      for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
          wi[g * rows * cols + (i / 8) * cols * 8 + (j / 8) * 64 + (i % 8) * 8 +
             j % 8] = w[g * rows * cols + i * cols + j];
    std::vector<int32_t> v0(4 * rows), v1(4 * rows), v2(4 * rows);
    double t0 = BestNs(
        [&] {
          for (int g = 0; g < 4; g++)
            NowMatrix(wi.data() + g * rows * cols, rows, cols, u.data(),
                      v0.data() + g * rows);
        },
        5000);
    double t1 = BestNs(
        [&] {
          for (int g = 0; g < 4; g++)
            Padal4(w.data() + g * rows * cols, rows, cols, u.data(),
                   v1.data() + g * rows);
        },
        5000);
    double t2 = BestNs(
        [&] {
          for (int g = 0; g < 4; g++)
            Sdot4(w.data() + g * rows * cols, rows, cols, u.data(),
                  v2.data() + g * rows);
        },
        5000);
    printf(" %-7s %6.2f %6.2f %6.2f   %.1fx %s\n", L.name, t0 / 1e3, t1 / 1e3,
           t2 / 1e3, t0 / t2,
           (v0 == v1 && v0 == v2) ? "(identical results)" : "(RESULTS DIFFER)");
  }
  return 0;
}
