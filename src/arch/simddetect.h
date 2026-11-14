///////////////////////////////////////////////////////////////////////
// File:        simddetect.h
// Description: Architecture detector.
// Author:      Stefan Weil (based on code from Ray Smith)
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
#ifndef TESSERACT_ARCH_SIMDDETECT_H_
#define TESSERACT_ARCH_SIMDDETECT_H_

#include <tesseract/export.h>
#include "tesstypes.h"

namespace tesseract {

// Function pointer for best calculation of dot product.
using DotProductFunction = TFloat (*)(const TFloat *, const TFloat *, int);
extern DotProductFunction DotProduct;

// Function pointer for best calculation of the dot products of the rows of a
// matrix with a vector: v[i] = w[i] . u for the num_rows rows of w, where row
// i starts at w + i * w_stride and u and the used part of each row have n
// elements.
using RowDotProductsFunction = void (*)(const TFloat *w, int w_stride,
                                        int num_rows, const TFloat *u, int n,
                                        TFloat *v);
extern RowDotProductsFunction RowDotProducts;

// Function pointer for best calculation of the dot products of all rows of
// one matrix with all rows of another one: out[i * out_stride + j] =
// u[i] . v[j] for the num_u rows of u and the num_v rows of v. All rows have
// n elements and are stored without gaps.
using CrossDotProductsFunction = void (*)(const TFloat *u, int num_u,
                                          const TFloat *v, int num_v, int n,
                                          TFloat *out, int out_stride);
extern CrossDotProductsFunction CrossDotProducts;

// Function pointers for the non-linearity functions for whole vectors:
// out[i] = Tanh(in[i]) or out[i] = Logistic(in[i]) (see functions.h) for n
// values. in and out can be the same. All implementations give exactly the
// same results.
using VectorFunction = void (*)(int n, const TFloat *in, TFloat *out);
extern TESS_API VectorFunction TanhVector;
extern TESS_API VectorFunction LogisticVector;

// Function pointer for the conversion of n floats in the range [-1, 1] to
// int8_t: out[i] = IntCastRounded(ClipToRange(in[i] * INT8_MAX, -INT8_MAX,
// INT8_MAX)). All implementations give the same results (unless a value
// is NaN, which gives an undefined result).
using FloatToInt8Function = void (*)(int n, const TFloat *in, int8_t *out);
extern TESS_API FloatToInt8Function FloatToInt8Vector;

// Architecture detector. Add code here to detect any other architectures for
// SIMD-based faster dot product functions. Intended to be a single static
// object, but it does no real harm to have more than one.
class SIMDDetect {
public:
  // Returns true if AVX is available on this system.
  static inline bool IsAVXAvailable() {
    return GetDetector().avx_available_;
  }
  // Returns true if AVX2 (integer support) is available on this system.
  static inline bool IsAVX2Available() {
    return GetDetector().avx2_available_;
  }
  // Returns true if AVX512 Foundation (float) is available on this system.
  static inline bool IsAVX512FAvailable() {
    return GetDetector().avx512F_available_;
  }
  // Returns true if AVX512 integer is available on this system.
  static inline bool IsAVX512BWAvailable() {
    return GetDetector().avx512BW_available_;
  }
  // Returns true if AVX512 Vector Neural Network Instructions are available.
  static inline bool IsAVX512VNNIAvailable() {
    return GetDetector().avx512VNNI_available_;
  }
  // Returns true if FMA is available on this system.
  static inline bool IsFMAAvailable() {
    return GetDetector().fma_available_;
  }
  // Returns true if SSE4.1 is available on this system.
  static inline bool IsSSEAvailable() {
    return GetDetector().sse_available_;
  }
  // Returns true if NEON is available on this system.
  static inline bool IsNEONAvailable() {
    return GetDetector().neon_available_;
  }
  // Returns true if the NEON dot product instructions (an optional feature
  // of ARMv8.2) are available on this system.
  static inline bool IsNEONDotProdAvailable() {
    return GetDetector().neon_dotprod_available_;
  }
  // Returns true if RVV is available on this system.
  static inline bool IsRVVAvailable() {
    return GetDetector().rvv_available_;
  }

  // Update settings after config variable was set.
  static TESS_API void Update();
  // Returns the singleton detector instance.
  // The detector is constructed on first use (a function local static
  // variable), which avoids an unspecified order of static initialization
  // with the static members of other classes (like IntSimdMatrix).
  static TESS_API const SIMDDetect &GetDetector();

private:
  // Constructor, must set all static member variables.
  SIMDDetect();

private:
  // If true, then AVX has been detected.
  static TESS_API bool avx_available_;
  static TESS_API bool avx2_available_;
  static TESS_API bool avx512F_available_;
  static TESS_API bool avx512BW_available_;
  static TESS_API bool avx512VNNI_available_;
  // If true, then FMA has been detected.
  static TESS_API bool fma_available_;
  // If true, then SSe4.1 has been detected.
  static TESS_API bool sse_available_;
  // If true, then NEON has been detected.
  static TESS_API bool neon_available_;
  // If true, then the NEON dot product instructions have been detected.
  static TESS_API bool neon_dotprod_available_;
  // If true, then RVV has been detected.
  static TESS_API bool rvv_available_;
};

} // namespace tesseract

#endif // TESSERACT_ARCH_SIMDDETECT_H_
