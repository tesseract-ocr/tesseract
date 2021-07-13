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

#include "tfloat.h"

namespace tesseract {

// Computes and returns the dot product of the n-vectors u and v.
template <class TFloat>
TFloat DotProductNative(const TFloat *u, const TFloat *v, int n);

// ------------ FAST FLOAT specializations -----------------

// Uses Intel AVX intrinsics to access the SIMD instruction set.
float DotProductAVX(const float *u, const float *v, int n);
float DotProductAVX1(const float *u, const float *v, int n);
float DotProductAVX2(const float *u, const float *v, int n);
float DotProductAVX3(const float *u, const float *v, int n);
float DotProductAVX4(const float *u, const float *v, int n);

// Use Intel FMA.
float DotProductFMA(const float *u, const float *v, int n);

// Uses Intel SSE intrinsics to access the SIMD instruction set.
float DotProductSSE(const float *u, const float *v, int n);

float DotProductAccelerate(const float *u, const float *v, int n);

// ------------ HIGH PRECISION DOUBLE specializations -----------------

// Uses Intel AVX intrinsics to access the SIMD instruction set.
double DotProductAVX(const double *u, const double *v, int n);
double DotProductAVX1(const double *u, const double *v, int n);
double DotProductAVX2(const double *u, const double *v, int n);
double DotProductAVX3(const double *u, const double *v, int n);
double DotProductAVX4(const double *u, const double *v, int n);

// Use Intel FMA.
double DotProductFMA(const double *u, const double *v, int n);

// Uses Intel SSE intrinsics to access the SIMD instruction set.
double DotProductSSE(const double *u, const double *v, int n);

double DotProductAccelerate(const double *u, const double *v, int n);

} // namespace tesseract.

#endif // TESSERACT_ARCH_DOTPRODUCT_H_
