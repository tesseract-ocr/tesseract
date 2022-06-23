///////////////////////////////////////////////////////////////////////
// File:        weightmatrix.cpp
// Description: Hides distinction between float/int implementations.
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

#include "weightmatrix.h"

#include <cassert> // for assert
#include "intsimdmatrix.h"
#include "simddetect.h" // for DotProduct
#include "statistc.h"
#include "tprintf.h"    // forTFloat

namespace tesseract {

#if defined(ANDROID)
static inline TFloat log2(TFloat n) {
  return log(n) / log(2.0);
}
#endif // ANDROID

// Number of iterations after which the correction effectively becomes unity.
const int kAdamCorrectionIterations = 200000;
// Epsilon in Adam to prevent division by zero.
const TFloat kAdamEpsilon = 1e-8;

// Utility functions convert between double and float arrays.
#ifdef FAST_FLOAT
static void DoubleToFloat(const GENERIC_2D_ARRAY<double> &src, GENERIC_2D_ARRAY<float> &dst) {
  const auto dim1 = src.dim1();
  const auto dim2 = src.dim2();
  dst.ResizeNoInit(dim1, dim2);
  for (int i = 0; i < dim1; ++i) {
    const auto *src_i = src[i];
    auto *dst_i = dst[i];
    for (int j = 0; j < dim2; ++j) {
      dst_i[j] = static_cast<float>(src_i[j]);
    }
  }
}
#endif

static void FloatToDouble(const GENERIC_2D_ARRAY<float> &src, GENERIC_2D_ARRAY<double> &dst) {
  const auto dim1 = src.dim1();
  const auto dim2 = src.dim2();
  dst.ResizeNoInit(dim1, dim2);
  for (int i = 0; i < dim1; ++i) {
    const auto *src_i = src[i];
    auto *dst_i = dst[i];
    for (int j = 0; j < dim2; ++j) {
      dst_i[j] = static_cast<double>(src_i[j]);
    }
  }
}

static bool DeSerialize(TFile *fp, GENERIC_2D_ARRAY<TFloat> &tfloat_array) {
#ifdef FAST_FLOAT
  GENERIC_2D_ARRAY<double> double_array;
  if (!double_array.DeSerialize(fp)) {
    return false;
  }
  DoubleToFloat(double_array, tfloat_array);
  return true;
#else
  return tfloat_array.DeSerialize(fp);
#endif
}

static bool Serialize(TFile *fp, const GENERIC_2D_ARRAY<TFloat> &tfloat_array) {
#ifdef FAST_FLOAT
  GENERIC_2D_ARRAY<double> double_array;
  FloatToDouble(tfloat_array, double_array);
  return double_array.Serialize(fp);
#else
  return tfloat_array.Serialize(fp);
#endif
}

// Computes matrix.vector v = Wu.
// u is of size W.dim2() - add_bias_fwd and the output v is of size
// W.dim1() - skip_bias_back.
// If add_bias_fwd, u is imagined to have an extra element at the end with value
// 1, to implement the bias, weight.
// If skip_bias_back, we are actually performing the backwards product on a
// transposed matrix, so we need to drop the v output corresponding to the last
// element in dim1.
static inline void MatrixDotVectorInternal(const GENERIC_2D_ARRAY<TFloat> &w, bool add_bias_fwd,
                                           bool skip_bias_back, const TFloat *u, TFloat *v) {
  int num_results = w.dim1() - skip_bias_back;
  int extent = w.dim2() - add_bias_fwd;
  for (int i = 0; i < num_results; ++i) {
    const TFloat *wi = w[i];
    TFloat total = DotProduct(wi, u, extent);
    if (add_bias_fwd) {
      total += wi[extent]; // The bias value.
    }
    v[i] = total;
  }
}

// Copies the whole input transposed, converted to TFloat, into *this.
void TransposedArray::Transpose(const GENERIC_2D_ARRAY<TFloat> &input) {
  int width = input.dim1();
  int num_features = input.dim2();
  ResizeNoInit(num_features, width);
  for (int t = 0; t < width; ++t) {
    WriteStrided(t, input[t]);
  }
}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
TransposedArray::~TransposedArray() = default;

// Sets up the network for training. Initializes weights using weights of
// scale `range` picked according to the random number generator `randomizer`.
int WeightMatrix::InitWeightsFloat(int no, int ni, bool use_adam, float weight_range,
                                   TRand *randomizer) {
  int_mode_ = false;
  wf_.Resize(no, ni, 0.0);
  if (randomizer != nullptr) {
    for (int i = 0; i < no; ++i) {
      for (int j = 0; j < ni; ++j) {
        wf_[i][j] = randomizer->SignedRand(weight_range);
      }
    }
  }
  use_adam_ = use_adam;
  InitBackward();
  return ni * no;
}

// Changes the number of outputs to the size of the given code_map, copying
// the old weight matrix entries for each output from code_map[output] where
// non-negative, and uses the mean (over all outputs) of the existing weights
// for all outputs with negative code_map entries. Returns the new number of
// weights.
int WeightMatrix::RemapOutputs(const std::vector<int> &code_map) {
  GENERIC_2D_ARRAY<TFloat> old_wf(wf_);
  int old_no = wf_.dim1();
  int new_no = code_map.size();
  int ni = wf_.dim2();
  std::vector<TFloat> means(ni, 0.0);
  for (int c = 0; c < old_no; ++c) {
    const TFloat *weights = wf_[c];
    for (int i = 0; i < ni; ++i) {
      means[i] += weights[i];
    }
  }
  for (auto &mean : means) {
    mean /= old_no;
  }
  wf_.Resize(new_no, ni, 0.0);
  InitBackward();
  for (int dest = 0; dest < new_no; ++dest) {
    int src = code_map[dest];
    const TFloat *src_data = src >= 0 ? old_wf[src] : means.data();
    memcpy(wf_[dest], src_data, ni * sizeof(*src_data));
  }
  return ni * new_no;
}

// Converts a float network to an int network. Each set of input weights that
// corresponds to a single output weight is converted independently:
// Compute the max absolute value of the weight set.
// Scale so the max absolute value becomes INT8_MAX.
// Round to integer.
// Store a multiplicative scale factor (as a TFloat) that will reproduce
// the original value, subject to rounding errors.
void WeightMatrix::ConvertToInt() {
  wi_.ResizeNoInit(wf_.dim1(), wf_.dim2());
  scales_.reserve(wi_.dim1());
  int dim2 = wi_.dim2();
  for (int t = 0; t < wi_.dim1(); ++t) {
    TFloat *f_line = wf_[t];
    int8_t *i_line = wi_[t];
    TFloat max_abs = 0;
    for (int f = 0; f < dim2; ++f) {
      TFloat abs_val = fabs(f_line[f]);
      if (abs_val > max_abs) {
        max_abs = abs_val;
      }
    }
    TFloat scale = max_abs / INT8_MAX;
    scales_.push_back(scale / INT8_MAX);
    if (scale == 0.0) {
      scale = 1.0;
    }
    for (int f = 0; f < dim2; ++f) {
      i_line[f] = IntCastRounded(f_line[f] / scale);
    }
  }
  wf_.Resize(1, 1, 0.0);
  int_mode_ = true;
  if (IntSimdMatrix::intSimdMatrix) {
    int32_t rounded_num_out;
    IntSimdMatrix::intSimdMatrix->Init(wi_, shaped_w_, rounded_num_out);
    scales_.resize(rounded_num_out);
  }
}

// Allocates any needed memory for running Backward, and zeroes the deltas,
// thus eliminating any existing momentum.
void WeightMatrix::InitBackward() {
  int no = int_mode_ ? wi_.dim1() : wf_.dim1();
  int ni = int_mode_ ? wi_.dim2() : wf_.dim2();
  dw_.Resize(no, ni, 0.0);
  updates_.Resize(no, ni, 0.0);
  wf_t_.Transpose(wf_);
  if (use_adam_) {
    dw_sq_sum_.Resize(no, ni, 0.0);
  }
}

// Flag on mode to indicate that this weightmatrix uses int8_t.
const int kInt8Flag = 1;
// Flag on mode to indicate that this weightmatrix uses adam.
const int kAdamFlag = 4;
// Flag on mode to indicate that this weightmatrix uses double. Set
// independently of kInt8Flag as even in int mode the scales can
// be float or double.
const int kDoubleFlag = 128;

// Writes to the given file. Returns false in case of error.
bool WeightMatrix::Serialize(bool training, TFile *fp) const {
  // For backward compatibility, add kDoubleFlag to mode to indicate the doubles
  // format, without errs, so we can detect and read old format weight matrices.
  uint8_t mode = (int_mode_ ? kInt8Flag : 0) | (use_adam_ ? kAdamFlag : 0) | kDoubleFlag;
  if (!fp->Serialize(&mode)) {
    return false;
  }
  if (int_mode_) {
    if (!wi_.Serialize(fp)) {
      return false;
    }
    uint32_t size = scales_.size();
    if (!fp->Serialize(&size)) {
      return false;
    }
    for (auto scale : scales_) {
      // The scales stored in memory have an extra factor applied to them
      // to allow faster operation. We have to remove that factor here
      // before writing to disc.
      double value = scale * INT8_MAX;
      if (!fp->Serialize(&value)) {
        return false;
      }
    }
  } else {
    if (!tesseract::Serialize(fp, wf_)) {
      return false;
    }
    if (training) {
      if (!tesseract::Serialize(fp, updates_)) {
        return false;
      }
      if (use_adam_ && !tesseract::Serialize(fp, dw_sq_sum_)) {
        return false;
      }
    }
  }
  return true;
}

// Reads from the given file. Returns false in case of error.

bool WeightMatrix::DeSerialize(bool training, TFile *fp) {
  uint8_t mode;
  if (!fp->DeSerialize(&mode)) {
    return false;
  }
  int_mode_ = (mode & kInt8Flag) != 0;
  use_adam_ = (mode & kAdamFlag) != 0;
  if ((mode & kDoubleFlag) == 0) {
    return DeSerializeOld(training, fp);
  }
  if (int_mode_) {
    if (!wi_.DeSerialize(fp)) {
      return false;
    }
    uint32_t size;
    if (!fp->DeSerialize(&size)) {
      return false;
    }
#ifdef FAST_FLOAT
    scales_.reserve(size);
    for (auto n = size; n > 0; n--) {
      double val;
      if (!fp->DeSerialize(&val)) {
        return false;
      }
      scales_.push_back(val / INT8_MAX);
    }
#else
    scales_.resize(size);
    if (!fp->DeSerialize(&scales_[0], size)) {
      return false;
    }
    for (auto &scale : scales_) {
      scale /= INT8_MAX;
    }
#endif
    if (IntSimdMatrix::intSimdMatrix) {
      int32_t rounded_num_out;
      IntSimdMatrix::intSimdMatrix->Init(wi_, shaped_w_, rounded_num_out);
      scales_.resize(rounded_num_out);
    }
  } else {
    if (!tesseract::DeSerialize(fp, wf_)) {
      return false;
    }
    if (training) {
      InitBackward();
      if (!tesseract::DeSerialize(fp, updates_)) {
        return false;
      }
      if (use_adam_) {
        if (!tesseract::DeSerialize(fp, dw_sq_sum_)) {
          return false;
        }
      }
    }
  }
  return true;
}

// As DeSerialize, but reads an old (float) format WeightMatrix for
// backward compatibility.
bool WeightMatrix::DeSerializeOld(bool training, TFile *fp) {
#ifdef FAST_FLOAT
  // Not implemented.
  ASSERT_HOST(!"not implemented");
  return false;
#else
  if (int_mode_) {
    if (!wi_.DeSerialize(fp)) {
      return false;
    }
    std::vector<float> old_scales;
    if (!fp->DeSerialize(old_scales)) {
      return false;
    }
    scales_.reserve(old_scales.size());
    for (float old_scale : old_scales) {
      scales_.push_back(old_scale);
    }
  } else {
    GENERIC_2D_ARRAY<float> float_array;
    if (!float_array.DeSerialize(fp)) {
      return false;
    }
    FloatToDouble(float_array, wf_);
  }
  if (training) {
    InitBackward();
    GENERIC_2D_ARRAY<float> float_array;
    if (!float_array.DeSerialize(fp)) {
      return false;
    }
    FloatToDouble(float_array, updates_);
    // Errs was only used in int training, which is now dead.
    if (!float_array.DeSerialize(fp)) {
      return false;
    }
  }
  return true;
#endif
}

// Computes matrix.vector v = Wu.
// u is of size W.dim2() - 1 and the output v is of size W.dim1().
// u is imagined to have an extra element at the end with value 1, to
// implement the bias, but it doesn't actually have it.
// Asserts that the call matches what we have.
void WeightMatrix::MatrixDotVector(const TFloat *u, TFloat *v) const {
  assert(!int_mode_);
  MatrixDotVectorInternal(wf_, true, false, u, v);
}

void WeightMatrix::MatrixDotVector(const int8_t *u, TFloat *v) const {
  assert(int_mode_);
  if (IntSimdMatrix::intSimdMatrix) {
    IntSimdMatrix::intSimdMatrix->matrixDotVectorFunction(wi_.dim1(), wi_.dim2(), &shaped_w_[0],
                                                          &scales_[0], u, v);
  } else {
    IntSimdMatrix::MatrixDotVector(wi_, scales_, u, v);
  }
}

// MatrixDotVector for peep weights, MultiplyAccumulate adds the
// component-wise products of *this[0] and v to inout.
void WeightMatrix::MultiplyAccumulate(const TFloat *v, TFloat *inout) {
  assert(!int_mode_);
  assert(wf_.dim1() == 1);
  int n = wf_.dim2();
  const TFloat *u = wf_[0];
  for (int i = 0; i < n; ++i) {
    inout[i] += u[i] * v[i];
  }
}

// Computes vector.matrix v = uW.
// u is of size W.dim1() and the output v is of size W.dim2() - 1.
// The last result is discarded, as v is assumed to have an imaginary
// last value of 1, as with MatrixDotVector.
void WeightMatrix::VectorDotMatrix(const TFloat *u, TFloat *v) const {
  assert(!int_mode_);
  MatrixDotVectorInternal(wf_t_, false, true, u, v);
}

// Fills dw_[i][j] with the dot product u[i][] . v[j][], using elements from
// u and v. In terms of the neural network, u is the gradients and v is the
// inputs.
// Note that (matching MatrixDotVector) v[last][] is missing, presumed 1.0.
// Runs parallel if requested. Note that u and v must be transposed.
void WeightMatrix::SumOuterTransposed(const TransposedArray &u, const TransposedArray &v,
                                      bool in_parallel) {
  assert(!int_mode_);
  int num_outputs = dw_.dim1();
  assert(u.dim1() == num_outputs);
  assert(u.dim2() == v.dim2());
  int num_inputs = dw_.dim2() - 1;
  int num_samples = u.dim2();
  // v is missing the last element in dim1.
  assert(v.dim1() == num_inputs);
#ifdef _OPENMP
#  pragma omp parallel for num_threads(4) if (in_parallel)
#endif
  for (int i = 0; i < num_outputs; ++i) {
    TFloat *dwi = dw_[i];
    const TFloat *ui = u[i];
    for (int j = 0; j < num_inputs; ++j) {
      dwi[j] = DotProduct(ui, v[j], num_samples);
    }
    // The last element of v is missing, presumed 1.0f.
    TFloat total = 0;
    for (int k = 0; k < num_samples; ++k) {
      total += ui[k];
    }
    dwi[num_inputs] = total;
  }
}

// Updates the weights using the given learning rate and momentum.
// num_samples is the quotient to be used in the adam computation iff
// use_adam_ is true.
void WeightMatrix::Update(float learning_rate, float momentum, float adam_beta, int num_samples) {
  assert(!int_mode_);
  if (use_adam_ && momentum > 0.0f && num_samples > 0 && num_samples < kAdamCorrectionIterations) {
    learning_rate *= sqrt(1.0f - pow(adam_beta, num_samples));
    learning_rate /= 1.0f - pow(momentum, num_samples);
  }
  if (use_adam_ && num_samples > 0 && momentum > 0.0f) {
    dw_sq_sum_.SumSquares(dw_, adam_beta);
    dw_ *= learning_rate * (1.0f - momentum);
    updates_ *= momentum;
    updates_ += dw_;
    wf_.AdamUpdate(updates_, dw_sq_sum_, learning_rate * kAdamEpsilon);
  } else {
    dw_ *= learning_rate;
    updates_ += dw_;
    if (momentum > 0.0f) {
      wf_ += updates_;
    }
    if (momentum >= 0.0f) {
      updates_ *= momentum;
    }
  }
  wf_t_.Transpose(wf_);
}

// Adds the dw_ in other to the dw_ is *this.
void WeightMatrix::AddDeltas(const WeightMatrix &other) {
  assert(dw_.dim1() == other.dw_.dim1());
  assert(dw_.dim2() == other.dw_.dim2());
  dw_ += other.dw_;
}

// Sums the products of weight updates in *this and other, splitting into
// positive (same direction) in *same and negative (different direction) in
// *changed.
void WeightMatrix::CountAlternators(const WeightMatrix &other, TFloat *same,
                                    TFloat *changed) const {
  int num_outputs = updates_.dim1();
  int num_inputs = updates_.dim2();
  assert(num_outputs == other.updates_.dim1());
  assert(num_inputs == other.updates_.dim2());
  for (int i = 0; i < num_outputs; ++i) {
    const TFloat *this_i = updates_[i];
    const TFloat *other_i = other.updates_[i];
    for (int j = 0; j < num_inputs; ++j) {
      TFloat product = this_i[j] * other_i[j];
      if (product < 0.0) {
        *changed -= product;
      } else {
        *same += product;
      }
    }
  }
}

// Helper computes an integer histogram bucket for a weight and adds it
// to the histogram.
const int kHistogramBuckets = 16;
static void HistogramWeight(TFloat weight, STATS *histogram) {
  int bucket = kHistogramBuckets - 1;
  if (weight != 0.0) {
    TFloat logval = -log2(fabs(weight));
    bucket = ClipToRange(IntCastRounded(logval), 0, kHistogramBuckets - 1);
  }
  histogram->add(bucket, 1);
}

void WeightMatrix::Debug2D(const char *msg) {
  STATS histogram(0, kHistogramBuckets - 1);
  if (int_mode_) {
    for (int i = 0; i < wi_.dim1(); ++i) {
      for (int j = 0; j < wi_.dim2(); ++j) {
        HistogramWeight(wi_[i][j] * scales_[i], &histogram);
      }
    }
  } else {
    for (int i = 0; i < wf_.dim1(); ++i) {
      for (int j = 0; j < wf_.dim2(); ++j) {
        HistogramWeight(wf_[i][j], &histogram);
      }
    }
  }
  tprintf("%s\n", msg);
  histogram.print();
}

} // namespace tesseract.
