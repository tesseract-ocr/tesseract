///////////////////////////////////////////////////////////////////////
// File:        intsimdmatrixavx2.cpp
// Description: matrix-vector product for 8-bit data on avx2.
// Author:      Ray Smith
//
// (C) Copyright 2017, Google Inc.
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

#include "intsimdmatrix.h"

#if !defined(__AVX2__)
#  if defined(__i686__) || defined(__x86_64__)
#    error Implementation only for AVX2 capable architectures
#  endif
#else
#  include <immintrin.h>
#  include <algorithm>
#  include <cstdint>
#  include <vector>

#  if defined(_MSC_VER) && _MSC_VER >= 1925 && _MSC_VER <= 1929 && \
      defined(_WIN32) && !defined(_WIN64)
// Optimize for size (/Os) instead of using the default optimization for some
// versions of the 32 bit Visual Studio compiler which generate buggy code.
#    pragma optimize("", off)
#    pragma optimize("s", on)
#  endif

namespace tesseract {

// Number of outputs held in each register. 8 x 32 bit ints.
constexpr int kNumOutputsPerRegister = 8;
// Maximum number of registers that we will use.
constexpr int kMaxOutputRegisters = 8;
// Number of inputs in the inputs register.
constexpr int kNumInputsPerRegister = 32;
// Number of inputs in each weight group.
constexpr int kNumInputsPerGroup = 4;
// Number of groups of inputs to be broadcast.
constexpr int kNumInputGroups = kNumInputsPerRegister / kNumInputsPerGroup;

// Functions to compute part of a matrix.vector multiplication. The weights
// are in a very specific order (see above) in w, which is multiplied by
// u of length num_in, to produce output v after scaling the integer results
// by the corresponding member of scales.
// The amount of w and scales consumed is fixed and not available to the
// caller. The number of outputs written to v will be at most num_out.

// Computes one set of 4x8 products of inputs and weights, adding to result.
// Horizontally adds 4 adjacent results, making 8x32-bit results.
// rep_input is assumed to be an 8x replicated set of 4x8-bit signed integers.
// Note that wi must previously have been re-organized with blocks of 4x8
// weights in contiguous memory.
// ones is a register of 16x16-bit values all equal to 1.
// Note: wi is incremented by the amount of data read.
// weights and reps are scratch registers.
// This function must be inlined with references in order for the compiler to
// correctly use the registers declared in the caller.
static inline void MultiplyGroup(const __m256i &rep_input, const __m256i &ones, const int8_t *&wi,
                                 __m256i &weights, __m256i &reps, __m256i &result) {
  // Load a 4x8 block of weights.
  weights = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(wi));
  wi += kNumInputsPerRegister;
  // Normalize the signs on rep_input, weights, so weights is always +ve.
  reps = _mm256_sign_epi8(rep_input, weights);
  weights = _mm256_sign_epi8(weights, weights);
  // Multiply 32x8-bit reps by 32x8-bit weights to make 16x16-bit results,
  // with adjacent pairs added.
  weights = _mm256_maddubs_epi16(weights, reps);
  // Multiply 16x16-bit result by 16x16-bit ones to make 8x32-bit results,
  // with  adjacent pairs added. What we really want is a horizontal add of
  // 16+16=32 bit result, but there is no such instruction, so multiply by
  // 16-bit ones instead. It is probably faster than all the sign-extending,
  // permuting and adding that would otherwise be required.
  weights = _mm256_madd_epi16(weights, ones);
  result = _mm256_add_epi32(result, weights);
}

// Load 64 bits into the bottom of a 128bit register.
// We don't actually care what the top 64bits are, but this ends
// up with them being zero.
static inline __m128i load64_to_128(const int8_t *wi_) {
  const auto *wi = reinterpret_cast<const int64_t *>(wi_);
  return _mm_set_epi64x(0, wi[0]);
}

#if defined(FAST_FLOAT)

static inline void ExtractResults8(__m256i result, const int8_t *wi,
                                   const float *scales, float *v) {
  __m128i w128 = load64_to_128(wi); // 8x8bit vals in bottom of 128bit reg
  __m256i w256 = _mm256_cvtepi8_epi32(w128); // 8x32bit vals in 256bit reg
  __m256i bias_scale = _mm256_set_epi32(127, 127, 127, 127, 127, 127, 127, 127);
  __m256 scale01234567 = _mm256_loadu_ps(scales);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result = _mm256_add_epi32(result, w256);     // result += bias * 127
  __m256 res01234567 = _mm256_cvtepi32_ps(result);
  result = _mm256_permute4x64_epi64(result, 2 + (3 << 2));
  res01234567 = _mm256_mul_ps(res01234567, scale01234567);
  _mm256_storeu_ps(v, res01234567);
}

static inline void ExtractResults16(__m256i result0, __m256i result1,
                                    const int8_t *&wi, const float *&scales,
                                    float *&v) {
  __m128i w8 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(wi));
  // 8x8bit vals in bottom of 128bit reg
  const __m256i bias_scale =
      _mm256_set_epi32(127, 127, 127, 127, 127, 127, 127, 127);
  __m256i w256 = _mm256_cvtepi8_epi32(w8); // 8x32bit vals in 256bit reg
  __m256 scale01234567 = _mm256_loadu_ps(scales);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result0 = _mm256_add_epi32(result0, w256);   // result += bias * 127
  __m256 res01234567 = _mm256_cvtepi32_ps(result0);
  result0 = _mm256_permute4x64_epi64(result0, 2 + (3 << 2));
  res01234567 = _mm256_mul_ps(res01234567, scale01234567);
  _mm256_storeu_ps(v, res01234567);
  w8 = _mm_shuffle_epi32(w8, 2 + (3 << 2));
  w256 = _mm256_cvtepi8_epi32(w8); // 8x32bit vals in 256bit reg
  scale01234567 = _mm256_loadu_ps(scales + 8);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result1 = _mm256_add_epi32(result1, w256);   // result += bias * 127
  res01234567 = _mm256_cvtepi32_ps(result1);
  result1 = _mm256_permute4x64_epi64(result1, 2 + (3 << 2));
  res01234567 = _mm256_mul_ps(res01234567, scale01234567);
  _mm256_storeu_ps(v + 8, res01234567);
  wi += 16;
  scales += 16;
  v += 16;
}

// Computes part of matrix.vector v = Wu. Computes N=64 results.
// The weights *must* be arranged so that consecutive reads from wi
// provides (num_in/kNumInputsPerGroup groups of (N output dim groups of
// (kNumInputsPerGroup inputs))). After that there must be N consecutive
// bias weights, before continuing with any more weights.
// u must be padded out with zeros to
// kNumInputsPerGroup*ceil(num_in/kNumInputsPerGroup) elements.
static void PartialMatrixDotVector64(const int8_t *wi, const float *scales, const int8_t *u,
                                     int num_in, float *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  __m256i result2 = _mm256_setzero_si256();
  __m256i result3 = _mm256_setzero_si256();
  __m256i result4 = _mm256_setzero_si256();
  __m256i result5 = _mm256_setzero_si256();
  __m256i result6 = _mm256_setzero_si256();
  __m256i result7 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result2);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result3);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result4);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result5);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result6);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result7);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
  ExtractResults16(result2, result3, wi, scales, v);
  ExtractResults16(result4, result5, wi, scales, v);
  ExtractResults16(result6, result7, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=32 results.
// For details see PartialMatrixDotVector64 with N=32.
static void PartialMatrixDotVector32(const int8_t *wi, const float *scales, const int8_t *u,
                                     int num_in, float *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  __m256i result2 = _mm256_setzero_si256();
  __m256i result3 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result2);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result3);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
  ExtractResults16(result2, result3, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=16 results.
// For details see PartialMatrixDotVector64 with N=16.
static void PartialMatrixDotVector16(const int8_t *wi, const float *scales, const int8_t *u,
                                     int num_in, float *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=8 results.
// For details see PartialMatrixDotVector64 with N=8.
static inline void PartialMatrixDotVector8(const int8_t *wi, const float *scales, const int8_t *u,
                                           int num_in, float *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
    }
  }
  ExtractResults8(result0, wi, scales, v);
}

static void matrixDotVector(int dim1, int dim2, const int8_t *wi, const float *scales,
                            const int8_t *u, float *v) {
  const int num_out = dim1;
  const int num_in = dim2 - 1;
  // Each call to a partial_func_ produces group_size outputs, except the
  // last one, which can produce less.
  const int rounded_num_in = IntSimdMatrix::Roundup(num_in, kNumInputsPerGroup);
  const int rounded_num_out = IntSimdMatrix::Roundup(num_out, kNumOutputsPerRegister);
  int group_size = kNumOutputsPerRegister * kMaxOutputRegisters;
  int output = 0;

  int w_step = (rounded_num_in + 1) * group_size;

  // Run with this group size, until it would produce too much output, then
  // switch to a smaller size.
  for (; output + group_size <= rounded_num_out; output += group_size) {
    PartialMatrixDotVector64(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector32(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
    output += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector16(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
    output += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector8(wi, scales, u, rounded_num_in, v);
  }
}
#else
static inline void ExtractResults8(__m256i result, const int8_t *wi, const double *scales,
                                   double *v) {
  __m128i w128 = load64_to_128(wi);          // 8x8bit vals in bottom of 128bit reg
  __m256i w256 = _mm256_cvtepi8_epi32(w128); // 8x32bit vals in 256bit reg
  __m256i bias_scale = _mm256_set_epi32(127, 127, 127, 127, 127, 127, 127, 127);
  __m256d scale0123 = _mm256_loadu_pd(scales);
  __m256d scale4567 = _mm256_loadu_pd(scales + 4);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result = _mm256_add_epi32(result, w256);     // result += bias * 127
  __m256d res0123 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result));
  result = _mm256_permute4x64_epi64(result, 2 + (3 << 2));
  __m256d res4567 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result));
  res0123 = _mm256_mul_pd(res0123, scale0123);
  res4567 = _mm256_mul_pd(res4567, scale4567);
  _mm256_storeu_pd(v, res0123);
  _mm256_storeu_pd(v + 4, res4567);
}

static inline void ExtractResults16(__m256i result0, __m256i result1, const int8_t *&wi,
                                    const double *&scales, double *&v) {
  __m128i w8 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(wi));
  // 8x8bit vals in bottom of 128bit reg
  const __m256i bias_scale = _mm256_set_epi32(127, 127, 127, 127, 127, 127, 127, 127);
  __m256i w256 = _mm256_cvtepi8_epi32(w8); // 8x32bit vals in 256bit reg
  __m256d scale0123 = _mm256_loadu_pd(scales);
  __m256d scale4567 = _mm256_loadu_pd(scales + 4);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result0 = _mm256_add_epi32(result0, w256);   // result += bias * 127
  __m256d res0123 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result0));
  result0 = _mm256_permute4x64_epi64(result0, 2 + (3 << 2));
  __m256d res4567 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result0));
  res0123 = _mm256_mul_pd(res0123, scale0123);
  res4567 = _mm256_mul_pd(res4567, scale4567);
  _mm256_storeu_pd(v, res0123);
  _mm256_storeu_pd(v + 4, res4567);
  w8 = _mm_shuffle_epi32(w8, 2 + (3 << 2));
  w256 = _mm256_cvtepi8_epi32(w8); // 8x32bit vals in 256bit reg
  scale0123 = _mm256_loadu_pd(scales + 8);
  scale4567 = _mm256_loadu_pd(scales + 12);
  w256 = _mm256_mullo_epi32(w256, bias_scale); // 8x32 <bias * 127>
  result1 = _mm256_add_epi32(result1, w256);   // result += bias * 127
  res0123 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result1));
  result1 = _mm256_permute4x64_epi64(result1, 2 + (3 << 2));
  res4567 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(result1));
  res0123 = _mm256_mul_pd(res0123, scale0123);
  res4567 = _mm256_mul_pd(res4567, scale4567);
  _mm256_storeu_pd(v + 8, res0123);
  _mm256_storeu_pd(v + 12, res4567);
  wi += 16;
  scales += 16;
  v += 16;
}

// Computes part of matrix.vector v = Wu. Computes N=64 results.
// The weights *must* be arranged so that consecutive reads from wi
// provides (num_in/kNumInputsPerGroup groups of (N output dim groups of
// (kNumInputsPerGroup inputs))). After that there must be N consecutive
// bias weights, before continuing with any more weights.
// u must be padded out with zeros to
// kNumInputsPerGroup*ceil(num_in/kNumInputsPerGroup) elements.
static void PartialMatrixDotVector64(const int8_t *wi, const double *scales, const int8_t *u,
                                     int num_in, double *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  __m256i result2 = _mm256_setzero_si256();
  __m256i result3 = _mm256_setzero_si256();
  __m256i result4 = _mm256_setzero_si256();
  __m256i result5 = _mm256_setzero_si256();
  __m256i result6 = _mm256_setzero_si256();
  __m256i result7 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result2);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result3);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result4);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result5);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result6);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result7);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
  ExtractResults16(result2, result3, wi, scales, v);
  ExtractResults16(result4, result5, wi, scales, v);
  ExtractResults16(result6, result7, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=32 results.
// For details see PartialMatrixDotVector64 with N=32.
static void PartialMatrixDotVector32(const int8_t *wi, const double *scales, const int8_t *u,
                                     int num_in, double *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  __m256i result2 = _mm256_setzero_si256();
  __m256i result3 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result2);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result3);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
  ExtractResults16(result2, result3, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=16 results.
// For details see PartialMatrixDotVector64 with N=16.
static void PartialMatrixDotVector16(const int8_t *wi, const double *scales, const int8_t *u,
                                     int num_in, double *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  __m256i result1 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
      MultiplyGroup(rep_input, ones, wi, weights, reps, result1);
    }
  }
  ExtractResults16(result0, result1, wi, scales, v);
}

// Computes part of matrix.vector v = Wu. Computes N=8 results.
// For details see PartialMatrixDotVector64 with N=8.
static inline void PartialMatrixDotVector8(const int8_t *wi, const double *scales, const int8_t *u,
                                           int num_in, double *v) {
  // Register containing 16-bit ones for horizontal add with 16->32 bit
  // conversion.
  __m256i ones = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  __m256i shift_id = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
  // Initialize all the results to 0.
  __m256i result0 = _mm256_setzero_si256();
  // Iterate over the input (u), one registerful at a time.
  for (int j = 0; j < num_in;) {
    __m256i inputs = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(u + j));
    // Inputs are processed in groups of kNumInputsPerGroup, replicated
    // kNumInputGroups times.
    for (int ig = 0; ig < kNumInputGroups && j < num_in; ++ig, j += kNumInputsPerGroup) {
      // Replicate the low 32 bits (4 inputs) 8 times.
      __m256i rep_input = _mm256_broadcastd_epi32(_mm256_castsi256_si128(inputs));
      // Rotate the inputs in groups of 4, so the next 4 inputs are ready.
      inputs = _mm256_permutevar8x32_epi32(inputs, shift_id);
      __m256i weights, reps;
      // Mul-add, with horizontal add of the 4 inputs to each of the results.
      MultiplyGroup(rep_input, ones, wi, weights, reps, result0);
    }
  }
  ExtractResults8(result0, wi, scales, v);
}

static void matrixDotVector(int dim1, int dim2, const int8_t *wi, const double *scales,
                            const int8_t *u, double *v) {
  const int num_out = dim1;
  const int num_in = dim2 - 1;
  // Each call to a partial_func_ produces group_size outputs, except the
  // last one, which can produce less.
  const int rounded_num_in = IntSimdMatrix::Roundup(num_in, kNumInputsPerGroup);
  const int rounded_num_out = IntSimdMatrix::Roundup(num_out, kNumOutputsPerRegister);
  int group_size = kNumOutputsPerRegister * kMaxOutputRegisters;
  int output = 0;

  int w_step = (rounded_num_in + 1) * group_size;

  // Run with this group size, until it would produce too much output, then
  // switch to a smaller size.
  for (; output + group_size <= rounded_num_out; output += group_size) {
    PartialMatrixDotVector64(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector32(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
    output += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector16(wi, scales, u, rounded_num_in, v);
    wi += w_step;
    scales += group_size;
    v += group_size;
    output += group_size;
  }
  group_size /= 2;
  w_step /= 2;

  if (output + group_size <= rounded_num_out) {
    PartialMatrixDotVector8(wi, scales, u, rounded_num_in, v);
  }
}
#endif

const IntSimdMatrix IntSimdMatrix::intSimdMatrixAVX2 = {
    // Function.
    matrixDotVector,
    // Number of 32 bit outputs held in each register.
    kNumOutputsPerRegister,
    // Maximum number of registers that we will use to hold outputs.
    kMaxOutputRegisters,
    // Number of 8 bit inputs in the inputs register.
    kNumInputsPerRegister,
    // Number of inputs in each weight group.
    kNumInputsPerGroup
};

} // namespace tesseract.

#endif
