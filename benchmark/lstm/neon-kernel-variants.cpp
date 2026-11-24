///////////////////////////////////////////////////////////////////////
// File:        neon-kernel-variants.cpp
// Description: Microbenchmark for variants of the NEON functions which
//              calculate several dot products at once.
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

// This standalone program (ARM64 only) compares the time of single dot
// products, called by a function pointer like in the generic code, with
// variants which handle several rows per pass. The sizes are those of the
// layers of the default network of tesstrain. It was used to choose the
// implementations of RowDotProductsNEON (Rows4x2) and CrossDotProductsNEON
// (Cross4x4) in src/arch/dotproductneon.cpp and can be used to try other
// variants on a new CPU or with a new compiler:
//
//   c++ -std=c++17 -O3 -o neon-kernel-variants neon-kernel-variants.cpp
//   ./neon-kernel-variants
//
// The accumulators are named variables. Variants with arrays of vectors were
// much slower with g++ 13, which did not keep them in registers.

#include <arm_neon.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#define NOINLINE __attribute__((noinline))

// Dot product like DotProductNEON for FAST_FLOAT.
NOINLINE float Dot2(const float *u, const float *v, int n) {
  float32x4_t r0 = vdupq_n_f32(0.0f), r1 = vdupq_n_f32(0.0f);
  while (n > 7) {
    r0 = vfmaq_f32(r0, vld1q_f32(u), vld1q_f32(v));
    r1 = vfmaq_f32(r1, vld1q_f32(u + 4), vld1q_f32(v + 4));
    u += 8;
    v += 8;
    n -= 8;
  }
  float total = vaddvq_f32(r0);
  total += vaddvq_f32(r1);
  while (n > 0) {
    total += *u++ * *v++;
    n--;
  }
  return total;
}
typedef float (*DotFn)(const float *, const float *, int);
volatile DotFn g_dot = Dot2;

// Single dot products for each row, like the generic code.
NOINLINE void RowsNow(const float *w, int stride, int rows, const float *u,
                      int n, float *v) {
  DotFn dot = g_dot;
  for (int i = 0; i < rows; i++)
    v[i] = dot(w + i * stride, u, n);
}

// 4 rows per pass, 2 accumulators per row (8 columns per iteration).
NOINLINE void Rows4x2(const float *w, int stride, int rows, const float *u,
                      int n, float *v) {
  int i = 0;
  for (; i + 4 <= rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * stride, *w1 = w0 + stride,
                *w2 = w1 + stride, *w3 = w2 + stride;
    float32x4_t a0 = vdupq_n_f32(0), a1 = a0, a2 = a0, a3 = a0, b0 = a0,
                b1 = a0, b2 = a0, b3 = a0;
    int j = 0;
    for (; j + 8 <= n; j += 8) {
      float32x4_t x = vld1q_f32(u + j), y = vld1q_f32(u + j + 4);
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
    float t0 = vaddvq_f32(vaddq_f32(a0, b0)),
          t1 = vaddvq_f32(vaddq_f32(a1, b1));
    float t2 = vaddvq_f32(vaddq_f32(a2, b2)),
          t3 = vaddvq_f32(vaddq_f32(a3, b3));
    for (; j < n; j++) {
      t0 += w0[j] * u[j];
      t1 += w1[j] * u[j];
      t2 += w2[j] * u[j];
      t3 += w3[j] * u[j];
    }
    v[i] = t0;
    v[i + 1] = t1;
    v[i + 2] = t2;
    v[i + 3] = t3;
  }
  for (; i < rows; i++)
    v[i] = Dot2(w + static_cast<size_t>(i) * stride, u, n);
}

// 4 rows per pass, 4 accumulators per row (16 columns per iteration).
NOINLINE void Rows4x4(const float *w, int stride, int rows, const float *u,
                      int n, float *v) {
  int i = 0;
  for (; i + 4 <= rows; i += 4) {
    const float *w0 = w + static_cast<size_t>(i) * stride, *w1 = w0 + stride,
                *w2 = w1 + stride, *w3 = w2 + stride;
    float32x4_t z = vdupq_n_f32(0);
    float32x4_t a0 = z, a1 = z, a2 = z, a3 = z, b0 = z, b1 = z, b2 = z, b3 = z;
    float32x4_t c0 = z, c1 = z, c2 = z, c3 = z, d0 = z, d1 = z, d2 = z, d3 = z;
    int j = 0;
    for (; j + 16 <= n; j += 16) {
      float32x4_t x = vld1q_f32(u + j), y = vld1q_f32(u + j + 4),
                  p = vld1q_f32(u + j + 8), q = vld1q_f32(u + j + 12);
      a0 = vfmaq_f32(a0, vld1q_f32(w0 + j), x);
      b0 = vfmaq_f32(b0, vld1q_f32(w0 + j + 4), y);
      c0 = vfmaq_f32(c0, vld1q_f32(w0 + j + 8), p);
      d0 = vfmaq_f32(d0, vld1q_f32(w0 + j + 12), q);
      a1 = vfmaq_f32(a1, vld1q_f32(w1 + j), x);
      b1 = vfmaq_f32(b1, vld1q_f32(w1 + j + 4), y);
      c1 = vfmaq_f32(c1, vld1q_f32(w1 + j + 8), p);
      d1 = vfmaq_f32(d1, vld1q_f32(w1 + j + 12), q);
      a2 = vfmaq_f32(a2, vld1q_f32(w2 + j), x);
      b2 = vfmaq_f32(b2, vld1q_f32(w2 + j + 4), y);
      c2 = vfmaq_f32(c2, vld1q_f32(w2 + j + 8), p);
      d2 = vfmaq_f32(d2, vld1q_f32(w2 + j + 12), q);
      a3 = vfmaq_f32(a3, vld1q_f32(w3 + j), x);
      b3 = vfmaq_f32(b3, vld1q_f32(w3 + j + 4), y);
      c3 = vfmaq_f32(c3, vld1q_f32(w3 + j + 8), p);
      d3 = vfmaq_f32(d3, vld1q_f32(w3 + j + 12), q);
    }
    for (; j + 4 <= n; j += 4) {
      float32x4_t x = vld1q_f32(u + j);
      a0 = vfmaq_f32(a0, vld1q_f32(w0 + j), x);
      a1 = vfmaq_f32(a1, vld1q_f32(w1 + j), x);
      a2 = vfmaq_f32(a2, vld1q_f32(w2 + j), x);
      a3 = vfmaq_f32(a3, vld1q_f32(w3 + j), x);
    }
    float t0 = vaddvq_f32(vaddq_f32(vaddq_f32(a0, b0), vaddq_f32(c0, d0))),
          t1 = vaddvq_f32(vaddq_f32(vaddq_f32(a1, b1), vaddq_f32(c1, d1)));
    float t2 = vaddvq_f32(vaddq_f32(vaddq_f32(a2, b2), vaddq_f32(c2, d2))),
          t3 = vaddvq_f32(vaddq_f32(vaddq_f32(a3, b3), vaddq_f32(c3, d3)));
    for (; j < n; j++) {
      t0 += w0[j] * u[j];
      t1 += w1[j] * u[j];
      t2 += w2[j] * u[j];
      t3 += w3[j] * u[j];
    }
    v[i] = t0;
    v[i + 1] = t1;
    v[i + 2] = t2;
    v[i + 3] = t3;
  }
  for (; i < rows; i++)
    v[i] = Dot2(w + static_cast<size_t>(i) * stride, u, n);
}

// Single dot products for each pair of rows, like the generic code.
NOINLINE void CrossNow(const float *u, int nu, const float *v, int nv, int n,
                       float *out, int os) {
  DotFn dot = g_dot;
  for (int i = 0; i < nu; i++)
    for (int j = 0; j < nv; j++)
      out[static_cast<size_t>(i) * os + j] = dot(
          u + static_cast<size_t>(i) * n, v + static_cast<size_t>(j) * n, n);
}

// 2 rows of u and 4 rows of v per pass.
NOINLINE void Cross2x4(const float *u, int nu, const float *v, int nv, int n,
                       float *out, int os) {
  int i = 0;
  for (; i + 2 <= nu; i += 2) {
    const float *u0 = u + static_cast<size_t>(i) * n, *u1 = u0 + n;
    float *o0 = out + static_cast<size_t>(i) * os, *o1 = o0 + os;
    int j = 0;
    for (; j + 4 <= nv; j += 4) {
      const float *v0 = v + static_cast<size_t>(j) * n, *v1 = v0 + n,
                  *v2 = v1 + n, *v3 = v2 + n;
      float32x4_t z = vdupq_n_f32(0);
      float32x4_t a0 = z, a1 = z, a2 = z, a3 = z, b0 = z, b1 = z, b2 = z,
                  b3 = z;
      int t = 0;
      for (; t + 4 <= n; t += 4) {
        float32x4_t x0 = vld1q_f32(u0 + t), x1 = vld1q_f32(u1 + t);
        float32x4_t y0 = vld1q_f32(v0 + t), y1 = vld1q_f32(v1 + t),
                    y2 = vld1q_f32(v2 + t), y3 = vld1q_f32(v3 + t);
        a0 = vfmaq_f32(a0, x0, y0);
        a1 = vfmaq_f32(a1, x0, y1);
        a2 = vfmaq_f32(a2, x0, y2);
        a3 = vfmaq_f32(a3, x0, y3);
        b0 = vfmaq_f32(b0, x1, y0);
        b1 = vfmaq_f32(b1, x1, y1);
        b2 = vfmaq_f32(b2, x1, y2);
        b3 = vfmaq_f32(b3, x1, y3);
      }
      float r00 = vaddvq_f32(a0), r01 = vaddvq_f32(a1), r02 = vaddvq_f32(a2),
            r03 = vaddvq_f32(a3);
      float r10 = vaddvq_f32(b0), r11 = vaddvq_f32(b1), r12 = vaddvq_f32(b2),
            r13 = vaddvq_f32(b3);
      for (; t < n; t++) {
        r00 += u0[t] * v0[t];
        r01 += u0[t] * v1[t];
        r02 += u0[t] * v2[t];
        r03 += u0[t] * v3[t];
        r10 += u1[t] * v0[t];
        r11 += u1[t] * v1[t];
        r12 += u1[t] * v2[t];
        r13 += u1[t] * v3[t];
      }
      o0[j] = r00;
      o0[j + 1] = r01;
      o0[j + 2] = r02;
      o0[j + 3] = r03;
      o1[j] = r10;
      o1[j + 1] = r11;
      o1[j + 2] = r12;
      o1[j + 3] = r13;
    }
    for (; j < nv; j++) {
      o0[j] = Dot2(u0, v + static_cast<size_t>(j) * n, n);
      o1[j] = Dot2(u1, v + static_cast<size_t>(j) * n, n);
    }
  }
  for (; i < nu; i++)
    Rows4x2(v, n, nv, u + static_cast<size_t>(i) * n, n,
            out + static_cast<size_t>(i) * os);
}

// 4 rows of u and 4 rows of v per pass.
NOINLINE void Cross4x4(const float *u, int nu, const float *v, int nv, int n,
                       float *out, int os) {
  int i = 0;
  for (; i + 4 <= nu; i += 4) {
    const float *u0 = u + static_cast<size_t>(i) * n, *u1 = u0 + n,
                *u2 = u1 + n, *u3 = u2 + n;
    float *o0 = out + static_cast<size_t>(i) * os, *o1 = o0 + os, *o2 = o1 + os,
          *o3 = o2 + os;
    int j = 0;
    for (; j + 4 <= nv; j += 4) {
      const float *v0 = v + static_cast<size_t>(j) * n, *v1 = v0 + n,
                  *v2 = v1 + n, *v3 = v2 + n;
      float32x4_t z = vdupq_n_f32(0);
      float32x4_t a0 = z, a1 = z, a2 = z, a3 = z, b0 = z, b1 = z, b2 = z,
                  b3 = z;
      float32x4_t c0 = z, c1 = z, c2 = z, c3 = z, d0 = z, d1 = z, d2 = z,
                  d3 = z;
      int t = 0;
      for (; t + 4 <= n; t += 4) {
        float32x4_t y0 = vld1q_f32(v0 + t), y1 = vld1q_f32(v1 + t),
                    y2 = vld1q_f32(v2 + t), y3 = vld1q_f32(v3 + t);
        float32x4_t x = vld1q_f32(u0 + t);
        a0 = vfmaq_f32(a0, x, y0);
        a1 = vfmaq_f32(a1, x, y1);
        a2 = vfmaq_f32(a2, x, y2);
        a3 = vfmaq_f32(a3, x, y3);
        x = vld1q_f32(u1 + t);
        b0 = vfmaq_f32(b0, x, y0);
        b1 = vfmaq_f32(b1, x, y1);
        b2 = vfmaq_f32(b2, x, y2);
        b3 = vfmaq_f32(b3, x, y3);
        x = vld1q_f32(u2 + t);
        c0 = vfmaq_f32(c0, x, y0);
        c1 = vfmaq_f32(c1, x, y1);
        c2 = vfmaq_f32(c2, x, y2);
        c3 = vfmaq_f32(c3, x, y3);
        x = vld1q_f32(u3 + t);
        d0 = vfmaq_f32(d0, x, y0);
        d1 = vfmaq_f32(d1, x, y1);
        d2 = vfmaq_f32(d2, x, y2);
        d3 = vfmaq_f32(d3, x, y3);
      }
      float r[4][4] = {
          {vaddvq_f32(a0), vaddvq_f32(a1), vaddvq_f32(a2), vaddvq_f32(a3)},
          {vaddvq_f32(b0), vaddvq_f32(b1), vaddvq_f32(b2), vaddvq_f32(b3)},
          {vaddvq_f32(c0), vaddvq_f32(c1), vaddvq_f32(c2), vaddvq_f32(c3)},
          {vaddvq_f32(d0), vaddvq_f32(d1), vaddvq_f32(d2), vaddvq_f32(d3)}};
      for (; t < n; t++) {
        const float x[4] = {u0[t], u1[t], u2[t], u3[t]},
                    y[4] = {v0[t], v1[t], v2[t], v3[t]};
        for (int a = 0; a < 4; a++)
          for (int b = 0; b < 4; b++)
            r[a][b] += x[a] * y[b];
      }
      for (int b = 0; b < 4; b++) {
        o0[j + b] = r[0][b];
        o1[j + b] = r[1][b];
        o2[j + b] = r[2][b];
        o3[j + b] = r[3][b];
      }
    }
    for (; j < nv; j++) {
      const float *vj = v + static_cast<size_t>(j) * n;
      o0[j] = Dot2(u0, vj, n);
      o1[j] = Dot2(u1, vj, n);
      o2[j] = Dot2(u2, vj, n);
      o3[j] = Dot2(u3, vj, n);
    }
  }
  for (; i < nu; i++)
    Rows4x2(v, n, nv, u + static_cast<size_t>(i) * n, n,
            out + static_cast<size_t>(i) * os);
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
// Returns the largest difference of two results.
static double MaxDiff(const std::vector<float> &a,
                      const std::vector<float> &b) {
  double m = 0;
  for (size_t i = 0; i < a.size(); i++)
    m = std::fmax(m, std::fabs(a[i] - b[i]));
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
    for (auto &e : x)
      e = Random() / (float)0x1000000 - 0.5f;
  };
  struct {
    const char *name;
    int ns, na;
  } layers[] = {{"Conv16", 16, 9},  {"Lfys48", 48, 64},   {"Lfx96", 96, 144},
                {"Lrx96", 96, 192}, {"Lfx192", 192, 288}, {"Out111", 111, 192},
                {"odd", 113, 195}};
  typedef void (*RowFn)(const float *, int, int, const float *, int, float *);
  RowFn rf[] = {RowsNow, Rows4x2, Rows4x4};
  printf(
      "W*u forward (4 gate matrices in turn), us per timestep\n"
      " layer   single    4x2    4x4\n");
  for (auto &L : layers) {
    int stride = L.na + 1;
    std::vector<float> w(4 * L.ns * stride), u(L.na), ref(4 * L.ns),
        v(4 * L.ns);
    fill(w);
    fill(u);
    printf(" %-7s", L.name);
    for (int k = 0; k < 3; k++) {
      auto &dst = k == 0 ? ref : v;
      double t = BestNs(
          [&] {
            for (int g = 0; g < 4; g++)
              rf[k](w.data() + g * L.ns * stride, stride, L.ns, u.data(), L.na,
                    dst.data() + g * L.ns);
          },
          5000);
      printf(" %6.2f", t / 1e3);
      if (k && MaxDiff(ref, v) > 1e-3)
        printf("(DIFF %g)", MaxDiff(ref, v));
    }
    printf("\n");
  }
  printf(
      "u*W backward (4 transposed gate matrices), us per timestep\n"
      " layer   single    4x2    4x4\n");
  for (auto &L : layers) {
    int stride = L.ns;
    std::vector<float> w(4 * (L.na + 1) * stride), u(L.ns), ref(4 * L.na),
        v(4 * L.na);
    fill(w);
    fill(u);
    printf(" %-7s", L.name);
    for (int k = 0; k < 3; k++) {
      auto &dst = k == 0 ? ref : v;
      double t = BestNs(
          [&] {
            for (int g = 0; g < 4; g++)
              rf[k](w.data() + g * (L.na + 1) * stride, stride, L.na, u.data(),
                    L.ns, dst.data() + g * L.na);
          },
          5000);
      printf(" %6.2f", t / 1e3);
      if (k && MaxDiff(ref, v) > 1e-3)
        printf("(DIFF %g)", MaxDiff(ref, v));
    }
    printf("\n");
  }
  typedef void (*CrossFn)(const float *, int, const float *, int, int, float *,
                          int);
  CrossFn cf[] = {CrossNow, Cross2x4, Cross4x4};
  printf(
      "gradient sum (one gate) for T timesteps, ms per call\n"
      " layer     T  single    2x4    4x4\n");
  for (int T : {37, 100, 300, 1000}) {
    for (auto &L : layers) {
      int os = L.na + 1;
      std::vector<float> u(L.ns * T), vv(L.na * T), ref(L.ns * os),
          d(L.ns * os);
      fill(u);
      fill(vv);
      printf(" %-7s %4d", L.name, T);
      for (int k = 0; k < 3; k++) {
        auto &dst = k == 0 ? ref : d;
        double t = BestNs(
            [&] { cf[k](u.data(), L.ns, vv.data(), L.na, T, dst.data(), os); },
            40);
        printf(" %6.3f", t / 1e6);
        if (k && MaxDiff(ref, d) > 2e-3)
          printf("(DIFF %g)", MaxDiff(ref, d));
      }
      printf("\n");
    }
  }
  return 0;
}
