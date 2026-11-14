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

// This standalone program (ARM64 only) compares variants of the core loop
// of PartialMatrixDotVector8 in src/arch/intsimdmatrixneon.cpp, which is
// used for the recognition with integer (fast) models:
//
//   old   8 rows per pass, groups of 8 inputs, widening multiplication and
//         a tree of pairwise additions (the code until November 2026)
//   mul4  4 rows per pass, groups of 16 inputs, widening multiplication and
//         pairwise accumulation
//   mul8  the same with 8 rows per pass (implemented as intSimdMatrixNEON)
//   dot4  4 rows per pass, groups of 16 inputs, dot product instruction SDOT
//   dot8  the same with 8 rows per pass (implemented as
//         intSimdMatrixNEONDotProd)
//
// SDOT needs a CPU with the feature "asimddp" (see /proc/cpuinfo), an
// optional part of ARMv8.2. The scaling of the results and the bias are not
// included. The weights and the inputs use all values of int8_t, and the
// results of all variants are compared.
//
//   c++ -std=c++17 -O3 -o neon-int-kernel-variants neon-int-kernel-variants.cpp
//   ./neon-int-kernel-variants

#include <arm_neon.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>
#define NOINLINE __attribute__((noinline))
#if defined(__clang__)
#  define TARGET_DOTPROD __attribute__((target("dotprod")))
#else
#  define TARGET_DOTPROD __attribute__((target("+dotprod")))
#endif

// Old code: 8 rows, groups of 8 inputs.
NOINLINE void Old8(const int8_t *wi, const int8_t *u, int num_in, int32_t *v) {
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

// Returns the sum of the four elements of a.
static inline int32_t Sum(int32x4_t a) {
  int32x2_t s = vadd_s32(vget_low_s32(a), vget_high_s32(a));
  return vget_lane_s32(s, 0) + vget_lane_s32(s, 1);
}

// Adds the products of 16 weights of row k and 16 inputs to acc.
#define MUL(acc, k)                                            \
  {                                                            \
    int8x16_t w = vld1q_s8(wi + 16 * (k));                     \
    acc = vpadalq_s16(acc, vmull_s8(vget_low_s8(w), x_low));   \
    acc = vpadalq_s16(acc, vmull_s8(vget_high_s8(w), x_high)); \
  }
#define DOT(acc, k) acc = vdotq_s32(acc, vld1q_s8(wi + 16 * (k)), x);

NOINLINE void Mul4(const int8_t *wi, const int8_t *u, int num_in, int32_t *v) {
  int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0;
  for (int j = 0; j < num_in; j += 16) {
    int8x16_t x = vld1q_s8(u + j);
    int8x8_t x_low = vget_low_s8(x), x_high = vget_high_s8(x);
    MUL(a0, 0) MUL(a1, 1) MUL(a2, 2) MUL(a3, 3) wi += 64;
  }
  v[0] = Sum(a0);
  v[1] = Sum(a1);
  v[2] = Sum(a2);
  v[3] = Sum(a3);
}
NOINLINE void Mul8(const int8_t *wi, const int8_t *u, int num_in, int32_t *v) {
  int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0, a4 = a0, a5 = a0,
            a6 = a0, a7 = a0;
  for (int j = 0; j < num_in; j += 16) {
    int8x16_t x = vld1q_s8(u + j);
    int8x8_t x_low = vget_low_s8(x), x_high = vget_high_s8(x);
    MUL(a0, 0)
    MUL(a1, 1) MUL(a2, 2) MUL(a3, 3) MUL(a4, 4) MUL(a5, 5) MUL(a6, 6) MUL(a7, 7)
        wi += 128;
  }
  v[0] = Sum(a0);
  v[1] = Sum(a1);
  v[2] = Sum(a2);
  v[3] = Sum(a3);
  v[4] = Sum(a4);
  v[5] = Sum(a5);
  v[6] = Sum(a6);
  v[7] = Sum(a7);
}
TARGET_DOTPROD NOINLINE void Dot4(const int8_t *wi, const int8_t *u, int num_in,
                                  int32_t *v) {
  int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0;
  for (int j = 0; j < num_in; j += 16) {
    int8x16_t x = vld1q_s8(u + j);
    DOT(a0, 0) DOT(a1, 1) DOT(a2, 2) DOT(a3, 3) wi += 64;
  }
  v[0] = Sum(a0);
  v[1] = Sum(a1);
  v[2] = Sum(a2);
  v[3] = Sum(a3);
}
TARGET_DOTPROD NOINLINE void Dot8(const int8_t *wi, const int8_t *u, int num_in,
                                  int32_t *v) {
  int32x4_t a0 = vdupq_n_s32(0), a1 = a0, a2 = a0, a3 = a0, a4 = a0, a5 = a0,
            a6 = a0, a7 = a0;
  for (int j = 0; j < num_in; j += 16) {
    int8x16_t x = vld1q_s8(u + j);
    DOT(a0, 0)
    DOT(a1, 1) DOT(a2, 2) DOT(a3, 3) DOT(a4, 4) DOT(a5, 5) DOT(a6, 6) DOT(a7, 7)
        wi += 128;
  }
  v[0] = Sum(a0);
  v[1] = Sum(a1);
  v[2] = Sum(a2);
  v[3] = Sum(a3);
  v[4] = Sum(a4);
  v[5] = Sum(a5);
  v[6] = Sum(a6);
  v[7] = Sum(a7);
}

typedef void (*Fn)(const int8_t *, const int8_t *, int, int32_t *);
// Interleaves w (rows x cols) for sets of n rows and groups of g inputs.
static std::vector<int8_t> Shape(const std::vector<int8_t> &w, int rows,
                                 int cols, int n, int g) {
  std::vector<int8_t> s(w.size());
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      s[(i / n) * cols * n + (j / g) * n * g + (i % n) * g + j % g] =
          w[i * cols + j];
  return s;
}
// Returns the best time per call in ns of several measurements.
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
  } layers[] = {{"Conv16", 16, 16}, {"Lfys48", 48, 64},   {"Lfx96", 96, 144},
                {"Lrx96", 96, 192}, {"Lfx192", 192, 288}, {"Lfx512", 512, 608}};
  struct {
    const char *name;
    Fn f;
    int n, g;
  } variants[] = {{"old", Old8, 8, 8},
                  {"mul4", Mul4, 4, 16},
                  {"mul8", Mul8, 8, 16},
                  {"dot4", Dot4, 4, 16},
                  {"dot8", Dot8, 8, 16}};
  printf("int8 W*u (4 gate matrices), us per timestep\n layer  ");
  for (auto &V : variants) {
    printf(" %6s", V.name);
  }
  printf("\n");
  for (auto &L : layers) {
    int rows = L.ns, cols = (L.na + 15) / 16 * 16;
    std::vector<int8_t> w(rows * cols), u(cols);
    for (auto &e : w)
      e = Random() % 256 - 128;
    for (auto &e : u)
      e = Random() % 256 - 128;
    std::vector<int32_t> ref;
    printf(" %-7s", L.name);
    for (auto &V : variants) {
      std::vector<int8_t> s = Shape(w, rows, cols, V.n, V.g);
      std::vector<int8_t> s4;
      for (int g = 0; g < 4; g++)
        s4.insert(s4.end(), s.begin(), s.end());
      std::vector<int32_t> v(4 * rows);
      double t = BestNs(
          [&] {
            for (int g = 0; g < 4; g++)
              for (int i = 0; i < rows; i += V.n)
                V.f(s4.data() + (size_t)g * rows * cols +
                        (size_t)(i / V.n) * cols * V.n,
                    u.data(), cols, v.data() + g * rows + i);
          },
          5000);
      printf(" %6.2f", t / 1e3);
      if (ref.empty()) {
        ref = v;
      } else if (ref != v) {
        printf("(RESULTS DIFFER)");
      }
    }
    printf("\n");
  }
  return 0;
}
