///////////////////////////////////////////////////////////////////////
// File:        functions.h
// Description: Collection of function-objects used by the network layers.
// Author:      Ray Smith
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

#include "helpers.h"
#include "tesstypes.h"

// Setting this to 1 or more causes massive dumps of debug data: weights,
// updates, internal calculations etc, and reduces the number of test iterations
// to a small number, so outputs can be diffed.
#define DEBUG_DETAIL 0
#if DEBUG_DETAIL > 0
#  undef _OPENMP // Disable open mp to get the outputs in sync.
#endif

namespace tesseract {

// Size of static tables.
constexpr int kTableSize = 4096;
// Scale factor for float arg to int index.
constexpr TFloat kScaleFactor = 256.0;

// Generated lookup tables.
extern const TFloat TanhTable[];
extern const TFloat LogisticTable[];

// Non-linearity (sigmoid) functions with cache tables and clipping.
inline TFloat Tanh(TFloat x) {
  if (x < 0) {
    return -Tanh(-x);
  }
  x *= kScaleFactor;
  auto index = static_cast<unsigned>(x);
  if (index >= (kTableSize - 1)) {
    return 1;
  }
  TFloat tanh_i0 = TanhTable[index];
  TFloat tanh_i1 = TanhTable[index + 1];
  // Linear interpolation.
  return tanh_i0 + (tanh_i1 - tanh_i0) * (x - index);
}

inline TFloat Logistic(TFloat x) {
  if (x < 0) {
    return 1 - Logistic(-x);
  }
  x *= kScaleFactor;
  auto index = static_cast<unsigned>(x);
  if (index >= (kTableSize - 1)) {
    return 1;
  }
  TFloat l0 = LogisticTable[index];
  TFloat l1 = LogisticTable[index + 1];
  // Linear interpolation.
  return l0 + (l1 - l0) * (x - index);
}

// Non-linearity (sigmoid) functions and their derivatives.
struct FFunc {
  inline TFloat operator()(TFloat x) const {
    return Logistic(x);
  }
};
struct FPrime {
  inline TFloat operator()(TFloat y) const {
    return y * (1 - y);
  }
};
struct ClipFFunc {
  inline TFloat operator()(TFloat x) const {
    if (x <= 0) {
      return 0;
    }
    if (x >= 1) {
      return 1;
    }
    return x;
  }
};
struct ClipFPrime {
  inline TFloat operator()(TFloat y) const {
    return 0 < y && y < 1 ? 1 : 0;
  }
};
struct Relu {
  inline TFloat operator()(TFloat x) const {
    if (x <= 0) {
      return 0;
    }
    return x;
  }
};
struct ReluPrime {
  inline TFloat operator()(TFloat y) const {
    return 0 < y ? 1 : 0;
  }
};
struct GFunc {
  inline TFloat operator()(TFloat x) const {
    return Tanh(x);
  }
};
struct GPrime {
  inline TFloat operator()(TFloat y) const {
    return 1 - y * y;
  }
};
struct ClipGFunc {
  inline TFloat operator()(TFloat x) const {
    if (x <= -1) {
      return -1;
    }
    if (x >= 1) {
      return 1;
    }
    return x;
  }
};
struct ClipGPrime {
  inline TFloat operator()(TFloat y) const {
    return -1 < y && y < 1 ? 1 : 0;
  }
};
struct HFunc {
  inline TFloat operator()(TFloat x) const {
    return Tanh(x);
  }
};
struct HPrime {
  inline TFloat operator()(TFloat y) const {
    TFloat u = Tanh(y);
    return 1 - u * u;
  }
};
struct UnityFunc {
  inline TFloat operator()(TFloat /*x*/) const {
    return 1.0;
  }
};
struct IdentityFunc {
  inline TFloat operator()(TFloat x) const {
    return x;
  }
};

// Applies Func in-place to inout, of size n.
template <class Func>
inline void FuncInplace(int n, TFloat *inout) {
  Func f;
  for (int i = 0; i < n; ++i) {
    inout[i] = f(inout[i]);
  }
}
// Applies Func to u and multiplies the result by v component-wise,
// putting the product in out, all of size n.
template <class Func>
inline void FuncMultiply(const TFloat *u, const TFloat *v, int n, TFloat *out) {
  Func f;
  for (int i = 0; i < n; ++i) {
    out[i] = f(u[i]) * v[i];
  }
}
// Applies the Softmax function in-place to inout, of size n.
template <typename T>
inline void SoftmaxInPlace(int n, T *inout) {
  if (n <= 0) {
    return;
  }
  // A limit on the negative range input to exp to guarantee non-zero output.
  const T kMaxSoftmaxActivation = 86;

  T max_output = inout[0];
  for (int i = 1; i < n; i++) {
    T output = inout[i];
    if (output > max_output) {
      max_output = output;
    }
  }
  T prob_total = 0;
  for (int i = 0; i < n; i++) {
    T prob = inout[i] - max_output;
    prob = std::exp(ClipToRange(prob, -kMaxSoftmaxActivation, static_cast<T>(0)));
    prob_total += prob;
    inout[i] = prob;
  }
  if (prob_total > 0) {
    for (int i = 0; i < n; i++) {
      inout[i] /= prob_total;
    }
  }
}

// Copies n values of the given src vector to dest.
inline void CopyVector(unsigned n, const TFloat *src, TFloat *dest) {
  memcpy(dest, src, n * sizeof(dest[0]));
}

// Adds n values of the given src vector to dest.
inline void AccumulateVector(int n, const TFloat *src, TFloat *dest) {
  for (int i = 0; i < n; ++i) {
    dest[i] += src[i];
  }
}

// Multiplies n values of inout in-place element-wise by the given src vector.
inline void MultiplyVectorsInPlace(int n, const TFloat *src, TFloat *inout) {
  for (int i = 0; i < n; ++i) {
    inout[i] *= src[i];
  }
}

// Multiplies n values of u by v, element-wise, accumulating to out.
inline void MultiplyAccumulate(int n, const TFloat *u, const TFloat *v, TFloat *out) {
  for (int i = 0; i < n; i++) {
    out[i] += u[i] * v[i];
  }
}

// Sums the given 5 n-vectors putting the result into sum.
inline void SumVectors(int n, const TFloat *v1, const TFloat *v2, const TFloat *v3,
                       const TFloat *v4, const TFloat *v5, TFloat *sum) {
  for (int i = 0; i < n; ++i) {
    sum[i] = v1[i] + v2[i] + v3[i] + v4[i] + v5[i];
  }
}

// Sets the given n-vector vec to 0.
template <typename T>
inline void ZeroVector(unsigned n, T *vec) {
  memset(vec, 0, n * sizeof(*vec));
}

// Clips the given vector vec, of size n to [lower, upper].
template <typename T>
inline void ClipVector(int n, T lower, T upper, T *vec) {
  for (int i = 0; i < n; ++i) {
    vec[i] = ClipToRange(vec[i], lower, upper);
  }
}

// Converts the given n-vector to a binary encoding of the maximum value,
// encoded as vector of nf binary values.
inline void CodeInBinary(int n, int nf, TFloat *vec) {
  if (nf <= 0 || n < nf) {
    return;
  }
  int index = 0;
  TFloat best_score = vec[0];
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

} // namespace tesseract.

#endif // TESSERACT_LSTM_FUNCTIONS_H_
