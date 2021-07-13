///////////////////////////////////////////////////////////////////////
// File:        simddetect.cpp
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // for HAVE_AVX, ...
#endif
#include <numeric> // for std::inner_product
#include "dotproduct.h"
#include "intsimdmatrix.h" // for IntSimdMatrix
#include "params.h"        // for STRING_VAR
#include "simddetect.h"
#include "tprintf.h" // for tprintf

#if defined(HAVE_FRAMEWORK_ACCELERATE)

// Use Apple Accelerate framework.
// https://developer.apple.com/documentation/accelerate/simd

// Comparison of execution time with different dot product implementations.
// time DOTPRODUCT=accelerate lstm_squashed_test
// Results for Intel Core i5 2.4 MHz:
// DotProductGeneric      108 s
// DotProduct (default)    60 s
// DotProductAccelerate    78 s
// DotProductNative        65 s
// Results for Apple M1:
// DotProductGeneric       64 s
// DotProduct (default)    60 s
// DotProductAccelerate    33 s
// DotProductNative        30 s

#include <Accelerate/Accelerate.h>

#endif

#if defined(HAVE_AVX) || defined(HAVE_AVX2) || defined(HAVE_FMA) || defined(HAVE_SSE4_1)
#  define HAS_CPUID
#endif

#if defined(HAS_CPUID)
#  if defined(__GNUC__)
#    include <cpuid.h>
#  elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#    include <intrin.h>
#  endif
#endif

#if defined(HAVE_NEON) && !defined(__aarch64__)
#  ifdef ANDROID
#    include <cpu-features.h>
#  elif defined(HAVE_HWCAP_BASED_NEON_RUNTIME_DETECTION)
#    include <sys/auxv.h>
#    include <asm/hwcap.h>
#  else
/* Assume linux */
#    include <asm/hwcap.h>
#    include <sys/auxv.h>
#  endif
#endif

namespace tesseract {

// Computes and returns the dot product of the two n-vectors u and v.
// Note: because the order of addition is different among the different dot
// product functions, the results can (and do) vary slightly (although they
// agree to within about 4e-15). This produces different results when running
// training, despite all random inputs being precisely equal.
// To get consistent results, use just one of these dot product functions.
// On a test multi-layer network, serial is 57% slower than SSE, and AVX
// is about 8% faster than SSE. This suggests that the time is memory
// bandwidth constrained and could benefit from holding the reused vector
// in AVX registers.
DotProductFunction DotProduct;

static STRING_VAR(dotproduct, "auto", "Function used for calculation of dot product");

SIMDDetect SIMDDetect::detector;

#if defined(__aarch64__)
// ARMv8 always has NEON.
bool SIMDDetect::neon_available_ = true;
#else
// If true, then Neon has been detected.
bool SIMDDetect::neon_available_ = false;
#endif
// If true, then AVX has been detected.
bool SIMDDetect::avx_available_ = false;
bool SIMDDetect::avx2_available_ = false;
bool SIMDDetect::avx512F_available_ = false;
bool SIMDDetect::avx512BW_available_ = false;
// If true, then FMA has been detected.
bool SIMDDetect::fma_available_ = false;
// If true, then SSE4.1 has been detected.
bool SIMDDetect::sse_available_ = false;

#if defined(HAVE_FRAMEWORK_ACCELERATE)
TFloat DotProductAccelerate(const TFloat* u, const TFloat* v, int n) {
  TFloat total = 0;
  const int stride = 1;
#if defined(FAST_FLOAT)
  vDSP_dotpr(u, stride, v, stride, &total, n);
#else
  vDSP_dotprD(u, stride, v, stride, &total, n);
#endif
  return total;
}
#endif

// Computes and returns the dot product of the two n-vectors u and v.
static TFloat DotProductGeneric(const TFloat *u, const TFloat *v, int n) {
  TFloat total = 0.0;
  for (int k = 0; k < n; ++k) {
    total += u[k] * v[k];
  }
  return total;
}

// Compute dot product using std::inner_product.
static TFloat DotProductStdInnerProduct(const TFloat *u, const TFloat *v, int n) {
  return std::inner_product(u, u + n, v, static_cast<TFloat>(0));
}

static void SetDotProduct(DotProductFunction f, const IntSimdMatrix *m = nullptr) {
  DotProduct = f;
  IntSimdMatrix::intSimdMatrix = m;
}

// Constructor.
// Tests the architecture in a system-dependent way to detect AVX, SSE and
// any other available SIMD equipment.
// __GNUC__ is also defined by compilers that include GNU extensions such as
// clang.
SIMDDetect::SIMDDetect() {
  // The fallback is a generic dot product calculation.
  SetDotProduct(DotProductGeneric);
  const char* dotproduct_env = getenv("DOTPRODUCT");
  if (dotproduct_env != nullptr) {
    dotproduct = dotproduct_env;
    Update();
    if (strcmp(dotproduct_env, "native") == 0) {
      SetDotProduct(DotProductNative);
#if defined(HAVE_FRAMEWORK_ACCELERATE)
    } else if (strcmp(dotproduct_env, "accelerate") == 0) {
      SetDotProduct(DotProductAccelerate);
#endif
    }
    return;
  }

#if defined(HAS_CPUID)
#  if defined(__GNUC__)
  unsigned int eax, ebx, ecx, edx;
  if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) != 0) {
    // Note that these tests all use hex because the older compilers don't have
    // the newer flags.
#    if defined(HAVE_SSE4_1)
    sse_available_ = (ecx & 0x00080000) != 0;
#    endif
#    if defined(HAVE_AVX) || defined(HAVE_AVX2) || defined(HAVE_FMA)
    auto xgetbv = []() {
      uint32_t xcr0;
      __asm__("xgetbv" : "=a"(xcr0) : "c"(0) : "%edx");
      return xcr0;
    };
    if ((ecx & 0x08000000) && ((xgetbv() & 6) == 6)) {
      // OSXSAVE bit is set, XMM state and YMM state are fine.
#      if defined(HAVE_FMA)
      fma_available_ = (ecx & 0x00001000) != 0;
#      endif
#      if defined(HAVE_AVX)
      avx_available_ = (ecx & 0x10000000) != 0;
      if (avx_available_) {
        // There is supposed to be a __get_cpuid_count function, but this is all
        // there is in my cpuid.h. It is a macro for an asm statement and cannot
        // be used inside an if.
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
        avx2_available_ = (ebx & 0x00000020) != 0;
        avx512F_available_ = (ebx & 0x00010000) != 0;
        avx512BW_available_ = (ebx & 0x40000000) != 0;
      }
#      endif
    }
#    endif
  }
#  elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
  int cpuInfo[4];
  int max_function_id;
  __cpuid(cpuInfo, 0);
  max_function_id = cpuInfo[0];
  if (max_function_id >= 1) {
    __cpuid(cpuInfo, 1);
#    if defined(HAVE_SSE4_1)
    sse_available_ = (cpuInfo[2] & 0x00080000) != 0;
#    endif
#    if defined(HAVE_AVX) || defined(HAVE_AVX2) || defined(HAVE_FMA)
    if ((cpuInfo[2] & 0x08000000) && ((_xgetbv(0) & 6) == 6)) {
      // OSXSAVE bit is set, XMM state and YMM state are fine.
#      if defined(HAVE_FMA)
      fma_available_ = (cpuInfo[2] & 0x00001000) != 0;
#      endif
#      if defined(HAVE_AVX)
      avx_available_ = (cpuInfo[2] & 0x10000000) != 0;
#      endif
#      if defined(HAVE_AVX2)
      if (max_function_id >= 7) {
        __cpuid(cpuInfo, 7);
        avx2_available_ = (cpuInfo[1] & 0x00000020) != 0;
        avx512F_available_ = (cpuInfo[1] & 0x00010000) != 0;
        avx512BW_available_ = (cpuInfo[1] & 0x40000000) != 0;
      }
#      endif
    }
#    endif
  }
#  else
#    error "I don't know how to test for SIMD with this compiler"
#  endif
#endif

#if defined(HAVE_NEON) && !defined(__aarch64__)
#  ifdef ANDROID
  {
    AndroidCpuFamily family = android_getCpuFamily();
    if (family == ANDROID_CPU_FAMILY_ARM)
      neon_available_ = (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON);
  }
#  elif defined(HAVE_HWCAP_BASED_NEON_RUNTIME_DETECTION)
#    if defined(NEON_ALWAYS_AVAILABLE)
  neon_available_ = 1;
#    else
  neon_available_ = getauxval(AT_HWCAP) & HWCAP_NEON;
#    endif
#  else
  /* Assume linux */
  neon_available_ = getauxval(AT_HWCAP) & HWCAP_NEON;
#  endif
#endif

  // Select code for calculation of dot product based on autodetection.
  if (false) {
    // This is a dummy to support conditional compilation.
  } else if (avx2_available_ && IntSimdMatrix::intSimdMatrixAVX2 != nullptr) {
    // AVX2 detected.
    SetDotProduct(DotProductAVX, IntSimdMatrix::intSimdMatrixAVX2);
  } else if (avx_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // AVX detected.
    SetDotProduct(DotProductAVX, IntSimdMatrix::intSimdMatrixSSE);
  } else if (fma_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // FMA detected.
    SetDotProduct(DotProductFMA, IntSimdMatrix::intSimdMatrixSSE);
  } else if (sse_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // SSE detected.
    SetDotProduct(DotProductSSE, IntSimdMatrix::intSimdMatrixSSE);
  } else if (neon_available_ && IntSimdMatrix::intSimdMatrixNEON != nullptr) {
    // NEON detected.
    SetDotProduct(DotProductNative, IntSimdMatrix::intSimdMatrixNEON);
  }
}

void SIMDDetect::Update() {
  // Select code for calculation of dot product based on the
  // value of the config variable if that value is not empty.
  const char *dotproduct_method = "generic";
  if (!strcmp(dotproduct.c_str(), "auto")) {
    // Automatic detection. Nothing to be done.
  } else if (!strcmp(dotproduct.c_str(), "generic")) {
    // Generic code selected by config variable.
    SetDotProduct(DotProductGeneric);
    dotproduct_method = "generic";
  } else if (!strcmp(dotproduct.c_str(), "native")) {
    // Native optimized code selected by config variable.
    SetDotProduct(DotProductNative);
    dotproduct_method = "native";
#if defined(HAVE_FRAMEWORK_ACCELERATE)
  } else if (dotproduct == "accelerate") {
    SetDotProduct(DotProductAccelerate);
    dotproduct_method = "accelerate";
#endif
  } else if (!strcmp(dotproduct.c_str(), "avx2") && avx2_available_ && IntSimdMatrix::intSimdMatrixAVX2 != nullptr) {
    // AVX2 selected by config variable.
    SetDotProduct(DotProductAVX, IntSimdMatrix::intSimdMatrixAVX2);
    dotproduct_method = "avx2";
  } else if (!strcmp(dotproduct.c_str(), "avx-1") && avx_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // AVX2 (Alternative Implementation) selected by config variable.
    SetDotProduct(DotProductAVX1, IntSimdMatrix::intSimdMatrixAVX2);
    dotproduct_method = "avx-1";
  } else if (!strcmp(dotproduct.c_str(), "avx") && avx_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // AVX selected by config variable.
    SetDotProduct(DotProductAVX, IntSimdMatrix::intSimdMatrixSSE);
    dotproduct_method = "avx";
  } else if (!strcmp(dotproduct.c_str(), "fma") && fma_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // FMA selected by config variable.
    SetDotProduct(DotProductFMA, IntSimdMatrix::intSimdMatrixSSE);
    dotproduct_method = "fma";
  } else if (!strcmp(dotproduct.c_str(), "sse") && sse_available_ && IntSimdMatrix::intSimdMatrixSSE != nullptr) {
    // SSE selected by config variable.
    SetDotProduct(DotProductSSE, IntSimdMatrix::intSimdMatrixSSE);
    dotproduct_method = "sse";
  } else if (!strcmp(dotproduct.c_str(), "neon") && neon_available_ && IntSimdMatrix::intSimdMatrixNEON != nullptr) {
    // NEON selected by config variable.
    SetDotProduct(DotProductNative, IntSimdMatrix::intSimdMatrixNEON);
    dotproduct_method = "neon";
  } else if (!strcmp(dotproduct.c_str(), "std::inner_product")) {
    // std::inner_product selected by config variable.
    SetDotProduct(DotProductStdInnerProduct);
    dotproduct_method = "std::inner_product";
  } else {
    // Unsupported value of config variable.
    tprintf("WARNING: Ignoring unsupported config variable value: dotproduct=%s\n",
            dotproduct.c_str());
    tprintf(
        "Support values for dotproduct: auto generic native"
#if defined(HAVE_FRAMEWORK_ACCELERATE)
        " accelerate"
#endif
		"%s%s%s%s%s%s",
		(avx2_available_&& IntSimdMatrix::intSimdMatrixAVX2 != nullptr) ? " avx2" : "",
		(avx_available_&& IntSimdMatrix::intSimdMatrixSSE != nullptr) ? " avx" : "",
		(fma_available_&& IntSimdMatrix::intSimdMatrixSSE != nullptr) ? " fma" : "",
		(sse_available_&& IntSimdMatrix::intSimdMatrixSSE != nullptr) ? " sse" : "",
		(neon_available_&& IntSimdMatrix::intSimdMatrixNEON != nullptr) ? " neon" : "",
		" std::inner_product.\n");
  }

  dotproduct.set_value(dotproduct_method);
}

} // namespace tesseract
