///////////////////////////////////////////////////////////////////////
// File:        networkio.h
// Description: Network input/output data, allowing float/int implementations.
// Author:      Ray Smith
// Created:     Tue Jun 17 08:43:11 PST 2014
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

#ifndef TESSERACT_LSTM_NETWORKIO_H_
#define TESSERACT_LSTM_NETWORKIO_H_

#include <cmath>
#include <cstdio>
#include <vector>

#include <tesseract/genericvector.h>
#include <tesseract/helpers.h>
#include "static_shape.h"
#include "stridemap.h"
#include "weightmatrix.h"

struct Pix;

namespace tesseract {

// Class to contain all the input/output of a network, allowing for fixed or
// variable-strided 2d to 1d mapping, and float or int8_t values. Provides
// enough calculating functions to hide the detail of the implementation.
class NetworkIO {
 public:
  NetworkIO() : int_mode_(false) {}
  // Resizes the array (and stride), avoiding realloc if possible, to the given
  // size from various size specs:
  // Same stride size, but given number of features.
  void Resize(const NetworkIO& src, int num_features) {
    ResizeToMap(src.int_mode(), src.stride_map(), num_features);
  }
  // Resizes to a specific size as a 2-d temp buffer. No batches, no y-dim.
  void Resize2d(bool int_mode, int width, int num_features);
  // Resizes forcing a float representation with the stridemap of src and the
  // given number of features.
  void ResizeFloat(const NetworkIO& src, int num_features) {
    ResizeToMap(false, src.stride_map(), num_features);
  }
  // Resizes to a specific stride_map.
  void ResizeToMap(bool int_mode, const StrideMap& stride_map,
                   int num_features);
  // Shrinks image size by x_scale,y_scale, and use given number of features.
  void ResizeScaled(const NetworkIO& src, int x_scale, int y_scale,
                    int num_features);
  // Resizes to just 1 x-coord, whatever the input.
  void ResizeXTo1(const NetworkIO& src, int num_features);
  // Initialize all the array to zero.
  void Zero();
  // Initializes to zero all elements of the array that do not correspond to
  // valid image positions. (If a batch of different-sized images are packed
  // together, then there will be padding pixels.)
  void ZeroInvalidElements();
  // Sets up the array from the given image, using the currently set int_mode_.
  // If the image width doesn't match the shape, the image is truncated or
  // padded with noise to match.
  void FromPix(const StaticShape& shape, const Pix* pix, TRand* randomizer);
  // Sets up the array from the given set of images, using the currently set
  // int_mode_. If the image width doesn't match the shape, the images are
  // truncated or padded with noise to match.
  void FromPixes(const StaticShape& shape, const std::vector<const Pix*>& pixes,
                 TRand* randomizer);
  // Copies the given pix to *this at the given batch index, stretching and
  // clipping the pixel values so that [black, black + 2*contrast] maps to the
  // dynamic range of *this, ie [-1,1] for a float and (-127,127) for int.
  // This is a 2-d operation in the sense that the output depth is the number
  // of input channels, the height is the height of the image, and the width
  // is the width of the image, or truncated/padded with noise if the width
  // is a fixed size.
  void Copy2DImage(int batch, Pix* pix, float black, float contrast,
                   TRand* randomizer);
  // Copies the given pix to *this at the given batch index, as Copy2DImage
  // above, except that the output depth is the height of the input image, the
  // output height is 1, and the output width as for Copy2DImage.
  // The image is thus treated as a 1-d set of vertical pixel strips.
  void Copy1DGreyImage(int batch, Pix* pix, float black, float contrast,
                       TRand* randomizer);
  // Helper stores the pixel value in i_ or f_ according to int_mode_.
  // t: is the index from the StrideMap corresponding to the current
  //   [batch,y,x] position
  // f: is the index into the depth/channel
  // pixel: the value of the pixel from the image (in one channel)
  // black: the pixel value to map to the lowest of the range of *this
  // contrast: the range of pixel values to stretch to half the range of *this.
  void SetPixel(int t, int f, int pixel, float black, float contrast);
  // Converts the array to a Pix. Must be pixDestroyed after use.
  Pix* ToPix() const;
  // Prints the first and last num timesteps of the array for each feature.
  void Print(int num) const;

  // Returns the timestep width.
  int Width() const {
    return int_mode_ ? i_.dim1() : f_.dim1();
  }
  // Returns the number of features.
  int NumFeatures() const {
    return int_mode_ ? i_.dim2() : f_.dim2();
  }
  // Accessor to a timestep of the float matrix.
  float* f(int t) {
    ASSERT_HOST(!int_mode_);
    return f_[t];
  }
  const float* f(int t) const {
    ASSERT_HOST(!int_mode_);
    return f_[t];
  }
  const int8_t* i(int t) const {
    ASSERT_HOST(int_mode_);
    return i_[t];
  }
  bool int_mode() const {
    return int_mode_;
  }
  void set_int_mode(bool is_quantized) {
    int_mode_ = is_quantized;
  }
  const StrideMap& stride_map() const {
    return stride_map_;
  }
  void set_stride_map(const StrideMap& map) {
    stride_map_ = map;
  }
  const GENERIC_2D_ARRAY<float>& float_array() const { return f_; }
  GENERIC_2D_ARRAY<float>* mutable_float_array() { return &f_; }

  // Copies a single time step from src.
  void CopyTimeStepFrom(int dest_t, const NetworkIO& src, int src_t);
  // Copies a part of single time step from src.
  void CopyTimeStepGeneral(int dest_t, int dest_offset, int num_features,
                           const NetworkIO& src, int src_t, int src_offset);
  // Zeroes a single time step.
  void ZeroTimeStep(int t) { ZeroTimeStepGeneral(t, 0, NumFeatures()); }
  void ZeroTimeStepGeneral(int t, int offset, int num_features);
  // Sets the given range to random values.
  void Randomize(int t, int offset, int num_features, TRand* randomizer);

  // Helper returns the label and score of the best choice over a range.
  int BestChoiceOverRange(int t_start, int t_end, int not_this, int null_ch,
                          float* rating, float* certainty) const;
  // Helper returns the rating and certainty of the choice over a range in t.
  void ScoresOverRange(int t_start, int t_end, int choice, int null_ch,
                       float* rating, float* certainty) const;
  // Returns the index (label) of the best value at the given timestep,
  // and if not null, sets the score to the log of the corresponding value.
  int BestLabel(int t, float* score) const {
    return BestLabel(t, -1, -1, score);
  }
  // Returns the index (label) of the best value at the given timestep,
  // excluding not_this and not_that, and if not null, sets the score to the
  // log of the corresponding value.
  int BestLabel(int t, int not_this, int not_that, float* score) const;
  // Returns the best start position out of range (into which both start and end
  // must fit) to obtain the highest cumulative score for the given labels.
  int PositionOfBestMatch(const GenericVector<int>& labels, int start,
                          int end) const;
  // Returns the cumulative score of the given labels starting at start, and
  // using one label per time-step.
  double ScoreOfLabels(const GenericVector<int>& labels, int start) const;
  // Helper function sets all the outputs for a single timestep, such that
  // label has value ok_score, and the other labels share 1 - ok_score.
  // Assumes float mode.
  void SetActivations(int t, int label, float ok_score);
  // Modifies the values, only if needed, so that the given label is
  // the winner at the given time step t.
  // Assumes float mode.
  void EnsureBestLabel(int t, int label);
  // Helper function converts prob to certainty taking the minimum into account.
  static float ProbToCertainty(float prob);
  // Returns true if there is any bad value that is suspiciously like a GT
  // error. Assuming that *this is the difference(gradient) between target
  // and forward output, returns true if there is a large negative value
  // (correcting a very confident output) for which there is no corresponding
  // positive value in an adjacent timestep for the same feature index. This
  // allows the box-truthed samples to make fine adjustments to position while
  // stopping other disagreements of confident output with ground truth.
  bool AnySuspiciousTruth(float confidence_thr) const;

  // Reads a single timestep to floats in the range [-1, 1].
  void ReadTimeStep(int t, double* output) const;
  // Adds a single timestep to floats.
  void AddTimeStep(int t, double* inout) const;
  // Adds part of a single timestep to floats.
  void AddTimeStepPart(int t, int offset, int num_features, float* inout) const;
  // Writes a single timestep from floats in the range [-1, 1].
  void WriteTimeStep(int t, const double* input);
  // Writes a single timestep from floats in the range [-1, 1] writing only
  // num_features elements of input to (*this)[t], starting at offset.
  void WriteTimeStepPart(int t, int offset, int num_features,
                         const double* input);
  // Maxpools a single time step from src.
  void MaxpoolTimeStep(int dest_t, const NetworkIO& src, int src_t,
                       int* max_line);
  // Runs maxpool backward, using maxes to index timesteps in *this.
  void MaxpoolBackward(const NetworkIO& fwd,
                       const GENERIC_2D_ARRAY<int>& maxes);
  // Returns the min over time of the maxes over features of the outputs.
  float MinOfMaxes() const;
  // Returns the min over time.
  float Max() const { return int_mode_ ? i_.Max() : f_.Max(); }
  // Computes combined results for a combiner that chooses between an existing
  // input and itself, with an additional output to indicate the choice.
  void CombineOutputs(const NetworkIO& base_output,
                      const NetworkIO& combiner_output);
  // Computes deltas for a combiner that chooses between 2 sets of inputs.
  void ComputeCombinerDeltas(const NetworkIO& fwd_deltas,
                             const NetworkIO& base_output);

  // Copies the array checking that the types match.
  void CopyAll(const NetworkIO& src);
  // Adds the array to a float array, with scaling to [-1, 1] if the src is int.
  void AddAllToFloat(const NetworkIO& src);
  // Subtracts the array from a float array. src must also be float.
  void SubtractAllFromFloat(const NetworkIO& src);

  // Copies src to *this, with maxabs normalization to match scale.
  void CopyWithNormalization(const NetworkIO& src, const NetworkIO& scale);
  // Multiplies the float data by the given factor.
  void ScaleFloatBy(float factor) { f_ *= factor; }
  // Copies src to *this with independent reversal of the y dimension.
  void CopyWithYReversal(const NetworkIO& src);
  // Copies src to *this with independent reversal of the x dimension.
  void CopyWithXReversal(const NetworkIO& src);
  // Copies src to *this with independent transpose of the x and y dimensions.
  void CopyWithXYTranspose(const NetworkIO& src);
  // Copies src to *this, at the given feature_offset, returning the total
  // feature offset after the copy. Multiple calls will stack outputs from
  // multiple sources in feature space.
  int CopyPacking(const NetworkIO& src, int feature_offset);
  // Opposite of CopyPacking, fills *this with a part of src, starting at
  // feature_offset, and picking num_features. Resizes *this to match.
  void CopyUnpacking(const NetworkIO& src, int feature_offset,
                     int num_features);
  // Transposes the float part of *this into dest.
  void Transpose(TransposedArray* dest) const;

  // Clips the content of a single time-step to +/-range.
  void ClipVector(int t, float range);

  // Applies Func to timestep t of *this (u) and multiplies the result by v
  // component-wise, putting the product in *product.
  // *this and v may be int or float, but must match. The outputs are double.
  template <class Func>
  void FuncMultiply(const NetworkIO& v_io, int t, double* product) {
    Func f;
    ASSERT_HOST(!int_mode_);
    ASSERT_HOST(!v_io.int_mode_);
    int dim = f_.dim2();
    if (int_mode_) {
      const int8_t* u = i_[t];
      const int8_t* v = v_io.i_[t];
      for (int i = 0; i < dim; ++i) {
        product[i] = f(u[i] / static_cast<double>(INT8_MAX)) * v[i] /
                     static_cast<double>(INT8_MAX);
      }
    } else {
      const float* u = f_[t];
      const float* v = v_io.f_[t];
      for (int i = 0; i < dim; ++i) {
        product[i] = f(u[i]) * v[i];
      }
    }
  }
  // Applies Func to *this (u) at u_t, and multiplies the result by v[v_t] * w,
  // component-wise, putting the product in *product.
  // All NetworkIOs are assumed to be float.
  template <class Func>
  void FuncMultiply3(int u_t, const NetworkIO& v_io, int v_t, const double* w,
                     double* product) const {
    ASSERT_HOST(!int_mode_);
    ASSERT_HOST(!v_io.int_mode_);
    Func f;
    const float* u = f_[u_t];
    const float* v = v_io.f_[v_t];
    int dim = f_.dim2();
    for (int i = 0; i < dim; ++i) {
      product[i] = f(u[i]) * v[i] * w[i];
    }
  }
  // Applies Func to *this (u) at u_t, and multiplies the result by v[v_t] * w,
  // component-wise, adding the product to *product.
  // All NetworkIOs are assumed to be float.
  template <class Func>
  void FuncMultiply3Add(const NetworkIO& v_io, int t, const double* w,
                        double* product) const {
    ASSERT_HOST(!int_mode_);
    ASSERT_HOST(!v_io.int_mode_);
    Func f;
    const float* u = f_[t];
    const float* v = v_io.f_[t];
    int dim = f_.dim2();
    for (int i = 0; i < dim; ++i) {
      product[i] += f(u[i]) * v[i] * w[i];
    }
  }
  // Applies Func1 to *this (u), Func2 to v, and multiplies the result by w,
  // component-wise, putting the product in product, all at timestep t, except
  // w, which is a simple array. All NetworkIOs are assumed to be float.
  template <class Func1, class Func2>
  void Func2Multiply3(const NetworkIO& v_io, int t, const double* w,
                      double* product) const {
    ASSERT_HOST(!int_mode_);
    ASSERT_HOST(!v_io.int_mode_);
    Func1 f;
    Func2 g;
    const float* u = f_[t];
    const float* v = v_io.f_[t];
    int dim = f_.dim2();
    for (int i = 0; i < dim; ++i) {
      product[i] = f(u[i]) * g(v[i]) * w[i];
    }
  }

 private:
  // Returns the padding required for the given number of features in order
  // for the SIMD operations to be safe.
  static int GetPadding(int num_features);

  // Choice of float vs 8 bit int for data.
  GENERIC_2D_ARRAY<float> f_;
  GENERIC_2D_ARRAY<int8_t> i_;
  // Which of f_ and i_ are we actually using.
  bool int_mode_;
  // Stride for 2d input data.
  StrideMap stride_map_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_NETWORKIO_H_
