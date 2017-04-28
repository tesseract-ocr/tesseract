///////////////////////////////////////////////////////////////////////
// File:        functions.h
// Description: Collection of function-objects used by the network layers.
// Author:      Ray Smith
// Created:     Fri Jun 20 10:45:37 PST 2014
//
// (C) Copyright 2014, Google Inc.
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

#ifndef TESSERACT_LSTM_FUNCTIONS_H_
#define TESSERACT_LSTM_FUNCTIONS_H_

#include <cmath>
#include "helpers.h"
#include "tprintf.h"

// Setting this to 1 or more causes massive dumps of debug data: weights,
// updates, internal calculations etc, and reduces the number of test iterations
// to a small number, so outputs can be diffed.
#define DEBUG_DETAIL 0
#if DEBUG_DETAIL > 0
#undef _OPENMP  // Disable open mp to get the outputs in sync.
#endif

namespace tesseract {

// Size of static tables.
const int kTableSize = 4096;
// Scale factor for float arg to int index.
const double kScaleFactor = 256.0;

extern double TanhTable[];
extern double LogisticTable[];

// Non-linearity (sigmoid) functions with cache tables and clipping.
inline double Tanh(double x) {
  if (x < 0.0) return -Tanh(-x);
  if (x >= (kTableSize - 1) / kScaleFactor) return 1.0;
  x *= kScaleFactor;
  int index = static_cast<int>(floor(x));
  if (TanhTable[index] == 0.0 && index > 0) {
    // Generate the entry.
    TanhTable[index] = tanh(index / kScaleFactor);
  }
  if (index == kTableSize - 1) return TanhTable[kTableSize - 1];
  if (TanhTable[index + 1] == 0.0) {
    // Generate the entry.
    TanhTable[index + 1] = tanh((index + 1) / kScaleFactor);
  }
  double offset = x - index;
  return TanhTable[index] * (1.0 - offset) + TanhTable[index + 1] * offset;
}

inline double Logistic(double x) {
  if (x < 0.0) return 1.0 - Logistic(-x);
  if (x >= (kTableSize - 1) / kScaleFactor) return 1.0;
  x *= kScaleFactor;
  int index = static_cast<int>(floor(x));
  if (LogisticTable[index] == 0.0) {
    // Generate the entry.
    LogisticTable[index] = 1.0 / (1.0 + exp(-index / kScaleFactor));
  }
  if (index == kTableSize - 1) return LogisticTable[kTableSize - 1];
  if (LogisticTable[index + 1] == 0.0) {
    // Generate the entry.
    LogisticTable[index + 1] = 1.0 / (1.0 + exp(-(index + 1) / kScaleFactor));
  }
  double offset = x - index;
  return LogisticTable[index] * (1.0 - offset) +
         LogisticTable[index + 1] * offset;
}

// Non-linearity (sigmoid) functions and their derivatives.
struct FFunc {
  inline double operator()(double x) const { return Logistic(x); }
};
struct FPrime {
  inline double operator()(double y) const { return y * (1.0 - y); }
};
struct ClipFFunc {
  inline double operator()(double x) const {
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;
    return x;
  }
};
struct ClipFPrime {
  inline double operator()(double y) const {
    return 0.0 < y && y < 1.0 ? 1.0 : 0.0;
  }
};
struct Relu {
  inline double operator()(double x) const {
    if (x <= 0.0) return 0.0;
    return x;
  }
};
struct ReluPrime {
  inline double operator()(double y) const { return 0.0 < y ? 1.0 : 0.0; }
};
struct GFunc {
  inline double operator()(double x) const { return Tanh(x); }
};
struct GPrime {
  inline double operator()(double y) const { return 1.0 - y * y; }
};
struct ClipGFunc {
  inline double operator()(double x) const {
    if (x <= -1.0) return -1.0;
    if (x >= 1.0) return 1.0;
    return x;
  }
};
struct ClipGPrime {
  inline double operator()(double y) const {
    return -1.0 < y && y < 1.0 ? 1.0 : 0.0;
  }
};
struct HFunc {
  inline double operator()(double x) const { return Tanh(x); }
};
struct HPrime {
  inline double operator()(double y) const {
    double u = Tanh(y);
    return 1.0 - u * u;
  }
};
struct UnityFunc {
  inline double operator()(double x) const { return 1.0; }
};
struct IdentityFunc {
  inline double operator()(double x) const { return x; }
};

// Applies Func in-place to inout, of size n.
template <class Func>
inline void FuncInplace(int n, double* inout) {
  Func f;
  for (int i = 0; i < n; ++i) {
    inout[i] = f(inout[i]);
  }
}
// Applies Func to u and multiplies the result by v component-wise,
// putting the product in out, all of size n.
template <class Func>
inline void FuncMultiply(const double* u, const double* v, int n, double* out) {
  Func f;
  for (int i = 0; i < n; ++i) {
    out[i] = f(u[i]) * v[i];
  }
}
// Applies the Softmax function in-place to inout, of size n.
template <typename T>
inline void SoftmaxInPlace(int n, T* inout) {
  if (n <= 0) return;
  // A limit on the negative range input to exp to guarantee non-zero output.
  const T kMaxSoftmaxActivation = 86.0f;

  T max_output = inout[0];
  for (int i = 1; i < n; i++) {
    T output = inout[i];
    if (output > max_output) max_output = output;
  }
  T prob_total = 0.0;
  for (int i = 0; i < n; i++) {
    T prob = inout[i] - max_output;
    prob = exp(ClipToRange(prob, -kMaxSoftmaxActivation, static_cast<T>(0)));
    prob_total += prob;
    inout[i] = prob;
  }
  if (prob_total > 0.0) {
    for (int i = 0; i < n; i++) inout[i] /= prob_total;
  }
}

// Copies n values of the given src vector to dest.
inline void CopyVector(int n, const double* src, double* dest) {
  memcpy(dest, src, n * sizeof(dest[0]));
}

// Adds n values of the given src vector to dest.
inline void AccumulateVector(int n, const double* src, double* dest) {
  for (int i = 0; i < n; ++i) dest[i] += src[i];
}

// Multiplies n values of inout in-place element-wise by the given src vector.
inline void MultiplyVectorsInPlace(int n, const double* src, double* inout) {
  for (int i = 0; i < n; ++i) inout[i] *= src[i];
}

// Multiplies n values of u by v, element-wise, accumulating to out.
inline void MultiplyAccumulate(int n, const double* u, const double* v,
                               double* out) {
  for (int i = 0; i < n; i++) {
    out[i] += u[i] * v[i];
  }
}

// Sums the given 5 n-vectors putting the result into sum.
inline void SumVectors(int n, const double* v1, const double* v2,
                       const double* v3, const double* v4, const double* v5,
                       double* sum) {
  for (int i = 0; i < n; ++i) {
    sum[i] = v1[i] + v2[i] + v3[i] + v4[i] + v5[i];
  }
}

// Sets the given n-vector vec to 0.
template <typename T>
inline void ZeroVector(int n, T* vec) {
  memset(vec, 0, n * sizeof(*vec));
}

// Clips the given vector vec, of size n to [lower, upper].
template <typename T>
inline void ClipVector(int n, T lower, T upper, T* vec) {
  for (int i = 0; i < n; ++i) vec[i] = ClipToRange(vec[i], lower, upper);
}

// Converts the given n-vector to a binary encoding of the maximum value,
// encoded as vector of nf binary values.
inline void CodeInBinary(int n, int nf, double* vec) {
  if (nf <= 0 || n < nf) return;
  int index = 0;
  double best_score = vec[0];
  for (int i = 1; i < n; ++i) {
    if (vec[i] > best_score) {
      best_score = vec[i];
      index = i;
    }
  }
  int mask = 1;
  for (int i = 0; i < nf; ++i, mask *= 2) {
    vec[i] = (index & mask) ? 1.0 : 0.0;
  }
}

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_FUNCTIONS_H_
