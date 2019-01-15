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

#include <cassert>              // for assert
#include "intsimdmatrix.h"
#include "simddetect.h"         // for DotProduct
#include "statistc.h"
#include "tprintf.h"

namespace tesseract {

#if defined(ANDROID)
static inline double log2(double n) {
  return log(n) / log(2.0);
}
#endif // ANDROID

// Number of iterations after which the correction effectively becomes unity.
const int kAdamCorrectionIterations = 200000;
// Epsilon in Adam to prevent division by zero.
const double kAdamEpsilon = 1e-8;

// Computes matrix.vector v = Wu.
// u is of size W.dim2() - add_bias_fwd and the output v is of size
// W.dim1() - skip_bias_back.
// If add_bias_fwd, u is imagined to have an extra element at the end with value
// 1, to implement the bias, weight.
// If skip_bias_back, we are actullay performing the backwards product on a
// transposed matrix, so we need to drop the v output corresponding to the last
// element in dim1.
static inline void MatrixDotVectorInternal(const GENERIC_2D_ARRAY<double>& w,
                                           bool add_bias_fwd,
                                           bool skip_bias_back, const double* u,
                                           double* v) {
  int num_results = w.dim1() - skip_bias_back;
  int extent = w.dim2() - add_bias_fwd;
  for (int i = 0; i < num_results; ++i) {
    const double* wi = w[i];
    double total = DotProduct(wi, u, extent);
    if (add_bias_fwd) total += wi[extent];  // The bias value.
    v[i] = total;
  }
}

// Copies the whole input transposed, converted to double, into *this.
void TransposedArray::Transpose(const GENERIC_2D_ARRAY<double>& input) {
  int width = input.dim1();
  int num_features = input.dim2();
  ResizeNoInit(num_features, width);
  for (int t = 0; t < width; ++t) WriteStrided(t, input[t]);
}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
TransposedArray::~TransposedArray() = default;

// Sets up the network for training. Initializes weights using weights of
// scale `range` picked according to the random number generator `randomizer`.
int WeightMatrix::InitWeightsFloat(int no, int ni, bool use_adam,
                                   float weight_range, TRand* randomizer) {
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
int WeightMatrix::RemapOutputs(const std::vector<int>& code_map) {
  GENERIC_2D_ARRAY<double> old_wf(wf_);
  int old_no = wf_.dim1();
  int new_no = code_map.size();
  int ni = wf_.dim2();
  std::vector<double> means(ni, 0.0);
  for (int c = 0; c < old_no; ++c) {
    const double* weights = wf_[c];
    for (int i = 0; i < ni; ++i) means[i] += weights[i];
  }
  for (double& mean : means) mean /= old_no;
  wf_.ResizeNoInit(new_no, ni);
  InitBackward();
  for (int dest = 0; dest < new_no; ++dest) {
    int src = code_map[dest];
    const double* src_data = src >= 0 ? old_wf[src] : means.data();
    memcpy(wf_[dest], src_data, ni * sizeof(*src_data));
  }
  return ni * new_no;
}

// Converts a float network to an int network. Each set of input weights that
// corresponds to a single output weight is converted independently:
// Compute the max absolute value of the weight set.
// Scale so the max absolute value becomes INT8_MAX.
// Round to integer.
// Store a multiplicative scale factor (as a double) that will reproduce
// the original value, subject to rounding errors.
void WeightMatrix::ConvertToInt() {
  wi_.ResizeNoInit(wf_.dim1(), wf_.dim2());
  scales_.init_to_size(wi_.dim1(), 0.0);
  int dim2 = wi_.dim2();
  for (int t = 0; t < wi_.dim1(); ++t) {
    double* f_line = wf_[t];
    int8_t* i_line = wi_[t];
    double max_abs = 0.0;
    for (int f = 0; f < dim2; ++f) {
      double abs_val = fabs(f_line[f]);
      if (abs_val > max_abs) max_abs = abs_val;
    }
    double scale = max_abs / INT8_MAX;
    scales_[t] = scale;
    if (scale == 0.0) scale = 1.0;
    for (int f = 0; f < dim2; ++f) {
      i_line[f] = IntCastRounded(f_line[f] / scale);
    }
  }
  wf_.Resize(1, 1, 0.0);
  int_mode_ = true;
  if (IntSimdMatrix::intSimdMatrix) {
    IntSimdMatrix::intSimdMatrix->Init(wi_, shaped_w_);
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
  if (use_adam_) dw_sq_sum_.Resize(no, ni, 0.0);
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
bool WeightMatrix::Serialize(bool training, TFile* fp) const {
  // For backward compatibility, add kDoubleFlag to mode to indicate the doubles
  // format, without errs, so we can detect and read old format weight matrices.
  uint8_t mode =
      (int_mode_ ? kInt8Flag : 0) | (use_adam_ ? kAdamFlag : 0) | kDoubleFlag;
  if (!fp->Serialize(&mode)) return false;
  if (int_mode_) {
    if (!wi_.Serialize(fp)) return false;
    if (!scales_.Serialize(fp)) return false;
  } else {
    if (!wf_.Serialize(fp)) return false;
    if (training && !updates_.Serialize(fp)) return false;
    if (training && use_adam_ && !dw_sq_sum_.Serialize(fp)) return false;
  }
  return true;
}

// Reads from the given file. Returns false in case of error.

bool WeightMatrix::DeSerialize(bool training, TFile* fp) {
  uint8_t mode;
  if (!fp->DeSerialize(&mode)) return false;
  int_mode_ = (mode & kInt8Flag) != 0;
  use_adam_ = (mode & kAdamFlag) != 0;
  if ((mode & kDoubleFlag) == 0) return DeSerializeOld(training, fp);
  if (int_mode_) {
    if (!wi_.DeSerialize(fp)) return false;
    if (!scales_.DeSerialize(fp)) return false;
    if (IntSimdMatrix::intSimdMatrix) {
      IntSimdMatrix::intSimdMatrix->Init(wi_, shaped_w_);
    }
  } else {
    if (!wf_.DeSerialize(fp)) return false;
    if (training) {
      InitBackward();
      if (!updates_.DeSerialize(fp)) return false;
      if (use_adam_ && !dw_sq_sum_.DeSerialize(fp)) return false;
    }
  }
  return true;
}

// As DeSerialize, but reads an old (float) format WeightMatrix for
// backward compatibility.
bool WeightMatrix::DeSerializeOld(bool training, TFile* fp) {
  GENERIC_2D_ARRAY<float> float_array;
  if (int_mode_) {
    if (!wi_.DeSerialize(fp)) return false;
    GenericVector<float> old_scales;
    if (!old_scales.DeSerialize(fp)) return false;
    scales_.resize_no_init(old_scales.size());
    for (int i = 0; i < old_scales.size(); ++i) scales_[i] = old_scales[i];
  } else {
    if (!float_array.DeSerialize(fp)) return false;
    FloatToDouble(float_array, &wf_);
  }
  if (training) {
    InitBackward();
    if (!float_array.DeSerialize(fp)) return false;
    FloatToDouble(float_array, &updates_);
    // Errs was only used in int training, which is now dead.
    if (!float_array.DeSerialize(fp)) return false;
  }
  return true;
}

// Computes matrix.vector v = Wu.
// u is of size W.dim2() - 1 and the output v is of size W.dim1().
// u is imagined to have an extra element at the end with value 1, to
// implement the bias, but it doesn't actually have it.
// Asserts that the call matches what we have.
void WeightMatrix::MatrixDotVector(const double* u, double* v) const {
  assert(!int_mode_);
  MatrixDotVectorInternal(wf_, true, false, u, v);
}

void WeightMatrix::MatrixDotVector(const int8_t* u, double* v) const {
  assert(int_mode_);
  if (IntSimdMatrix::intSimdMatrix) {
    IntSimdMatrix::intSimdMatrix->matrixDotVectorFunction(
      wi_.dim1(), wi_.dim2(), &shaped_w_[0], &scales_[0], u, v);
  } else {
    IntSimdMatrix::MatrixDotVector(wi_, scales_, u, v);
  }
}

// MatrixDotVector for peep weights, MultiplyAccumulate adds the
// component-wise products of *this[0] and v to inout.
void WeightMatrix::MultiplyAccumulate(const double* v, double* inout) {
  assert(!int_mode_);
  assert(wf_.dim1() == 1);
  int n = wf_.dim2();
  const double* u = wf_[0];
  for (int i = 0; i < n; ++i) {
    inout[i] += u[i] * v[i];
  }
}

// Computes vector.matrix v = uW.
// u is of size W.dim1() and the output v is of size W.dim2() - 1.
// The last result is discarded, as v is assumed to have an imaginary
// last value of 1, as with MatrixDotVector.
void WeightMatrix::VectorDotMatrix(const double* u, double* v) const {
  assert(!int_mode_);
  MatrixDotVectorInternal(wf_t_, false, true, u, v);
}

// Fills dw_[i][j] with the dot product u[i][] . v[j][], using elements from
// u and v. In terms of the neural network, u is the gradients and v is the
// inputs.
// Note that (matching MatrixDotVector) v[last][] is missing, presumed 1.0.
// Runs parallel if requested. Note that u and v must be transposed.
void WeightMatrix::SumOuterTransposed(const TransposedArray& u,
                                      const TransposedArray& v,
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
#pragma omp parallel for num_threads(4) if (in_parallel)
#endif
  for (int i = 0; i < num_outputs; ++i) {
    double* dwi = dw_[i];
    const double* ui = u[i];
    for (int j = 0; j < num_inputs; ++j) {
      dwi[j] = DotProduct(ui, v[j], num_samples);
    }
    // The last element of v is missing, presumed 1.0f.
    double total = 0.0;
    for (int k = 0; k < num_samples; ++k) total += ui[k];
    dwi[num_inputs] = total;
  }
}

// Updates the weights using the given learning rate and momentum.
// num_samples is the quotient to be used in the adam computation iff
// use_adam_ is true.
void WeightMatrix::Update(double learning_rate, double momentum,
                          double adam_beta, int num_samples) {
  assert(!int_mode_);
  if (use_adam_ && num_samples > 0 && num_samples < kAdamCorrectionIterations) {
    learning_rate *= sqrt(1.0 - pow(adam_beta, num_samples));
    learning_rate /= 1.0 - pow(momentum, num_samples);
  }
  if (use_adam_ && num_samples > 0 && momentum > 0.0) {
    dw_sq_sum_.SumSquares(dw_, adam_beta);
    dw_ *= learning_rate * (1.0 - momentum);
    updates_ *= momentum;
    updates_ += dw_;
    wf_.AdamUpdate(updates_, dw_sq_sum_, learning_rate * kAdamEpsilon);
  } else {
    dw_ *= learning_rate;
    updates_ += dw_;
    if (momentum > 0.0) wf_ += updates_;
    if (momentum >= 0.0) updates_ *= momentum;
  }
  wf_t_.Transpose(wf_);
}

// Adds the dw_ in other to the dw_ is *this.
void WeightMatrix::AddDeltas(const WeightMatrix& other) {
  assert(dw_.dim1() == other.dw_.dim1());
  assert(dw_.dim2() == other.dw_.dim2());
  dw_ += other.dw_;
}

// Sums the products of weight updates in *this and other, splitting into
// positive (same direction) in *same and negative (different direction) in
// *changed.
void WeightMatrix::CountAlternators(const WeightMatrix& other, double* same,
                                    double* changed) const {
  int num_outputs = updates_.dim1();
  int num_inputs = updates_.dim2();
  assert(num_outputs == other.updates_.dim1());
  assert(num_inputs == other.updates_.dim2());
  for (int i = 0; i < num_outputs; ++i) {
    const double* this_i = updates_[i];
    const double* other_i = other.updates_[i];
    for (int j = 0; j < num_inputs; ++j) {
      double product = this_i[j] * other_i[j];
      if (product < 0.0)
        *changed -= product;
      else
        *same += product;
    }
  }
}

// Helper computes an integer histogram bucket for a weight and adds it
// to the histogram.
const int kHistogramBuckets = 16;
static void HistogramWeight(double weight, STATS* histogram) {
  int bucket = kHistogramBuckets - 1;
  if (weight != 0.0) {
    double logval = -log2(fabs(weight));
    bucket = ClipToRange(IntCastRounded(logval), 0, kHistogramBuckets - 1);
  }
  histogram->add(bucket, 1);
}

void WeightMatrix::Debug2D(const char* msg) {
  STATS histogram(0, kHistogramBuckets);
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

// Utility function converts an array of float to the corresponding array
// of double.
/* static */
void WeightMatrix::FloatToDouble(const GENERIC_2D_ARRAY<float>& wf,
                                 GENERIC_2D_ARRAY<double>* wd) {
  int dim1 = wf.dim1();
  int dim2 = wf.dim2();
  wd->ResizeNoInit(dim1, dim2);
  for (int i = 0; i < dim1; ++i) {
    const float* wfi = wf[i];
    double* wdi = (*wd)[i];
    for (int j = 0; j < dim2; ++j) wdi[j] = static_cast<double>(wfi[j]);
  }
}

}  // namespace tesseract.
