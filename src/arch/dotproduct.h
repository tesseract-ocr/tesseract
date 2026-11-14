///////////////////////////////////////////////////////////////////////
// File:        dotproduct.h
// Description: Native dot product function.
//
// (C) Copyright 2018, Google Inc.
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

#ifndef TESSERACT_ARCH_DOTPRODUCT_H_
#define TESSERACT_ARCH_DOTPRODUCT_H_

#include "tesstypes.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
TFloat DotProductNative(const TFloat *u, const TFloat *v, int n);

// Uses Intel AVX intrinsics to access the SIMD instruction set.
TFloat DotProductAVX(const TFloat *u, const TFloat *v, int n);

// Uses Intel AVX512F intrinsics to access the SIMD instruction set.
TFloat DotProductAVX512F(const TFloat *u, const TFloat *v, int n);

// Use Intel FMA.
TFloat DotProductFMA(const TFloat *u, const TFloat *v, int n);

// Uses Intel SSE intrinsics to access the SIMD instruction set.
TFloat DotProductSSE(const TFloat *u, const TFloat *v, int n);

// Use NEON intrinsics.
TFloat DotProductNEON(const TFloat *u, const TFloat *v, int n);

#if defined(FAST_FLOAT) && defined(__ARM_ARCH_ISA_A64)
// Computes v[i] = w[i] . u for num_rows rows of w with n used elements each.
// Row i of w starts at w + i * w_stride.
void RowDotProductsNEON(const float *w, int w_stride, int num_rows,
                        const float *u, int n, float *v);

// Computes out[i * out_stride + j] = u[i] . v[j] for the num_u rows of u and
// the num_v rows of v, which all have n elements and are stored without gaps.
void CrossDotProductsNEON(const float *u, int num_u, const float *v, int num_v,
                          int n, float *out, int out_stride);
#endif

} // namespace tesseract.

#endif // TESSERACT_ARCH_DOTPRODUCT_H_
