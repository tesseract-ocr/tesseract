///////////////////////////////////////////////////////////////////////
// File:        weightmatrix.h
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

#ifndef TESSERACT_LSTM_WEIGHTMATRIX_H_
#define TESSERACT_LSTM_WEIGHTMATRIX_H_

#include <memory>
#include <tesseract/genericvector.h>
#include "intsimdmatrix.h"
#include "matrix.h"
#include "tprintf.h"

namespace tesseract {

// Convenience instantiation of GENERIC_2D_ARRAY<double> with additional
// operations to write a strided vector, so the transposed form of the input
// is memory-contiguous.
class TransposedArray : public GENERIC_2D_ARRAY<double> {
 public:
  // Copies the whole input transposed, converted to double, into *this.
  void Transpose(const GENERIC_2D_ARRAY<double>& input);
  // Writes a vector of data representing a timestep (gradients or sources).
  // The data is assumed to be of size1 in size (the strided dimension).
  ~TransposedArray() override;
  void WriteStrided(int t, const float* data) {
    int size1 = dim1();
    for (int i = 0; i < size1; ++i) put(i, t, data[i]);
  }
  void WriteStrided(int t, const double* data) {
    int size1 = dim1();
    for (int i = 0; i < size1; ++i) put(i, t, data[i]);
  }
  // Prints the first and last num elements of the un-transposed array.
  void PrintUnTransposed(int num) {
    int num_features = dim1();
    int width = dim2();
    for (int y = 0; y < num_features; ++y) {
      for (int t = 0; t < width; ++t) {
        if (num == 0 || t < num || t + num >= width) {
          tprintf(" %g", (*this)(y, t));
        }
      }
      tprintf("\n");
    }
  }
};  // class TransposedArray

// Generic weight matrix for network layers. Can store the matrix as either
// an array of floats or int8_t. Provides functions to compute the forward and
// backward steps with the matrix and updates to the weights.
class WeightMatrix {
 public:
  WeightMatrix() : int_mode_(false), use_adam_(false) {}
  // Sets up the network for training. Initializes weights using weights of
  // scale `range` picked according to the random number generator `randomizer`.
  // Note the order is outputs, inputs, as this is the order of indices to
  // the matrix, so the adjacent elements are multiplied by the input during
  // a forward operation.
  int InitWeightsFloat(int no, int ni, bool use_adam, float weight_range,
                       TRand* randomizer);
  // Changes the number of outputs to the size of the given code_map, copying
  // the old weight matrix entries for each output from code_map[output] where
  // non-negative, and uses the mean (over all outputs) of the existing weights
  // for all outputs with negative code_map entries. Returns the new number of
  // weights.
  int RemapOutputs(const std::vector<int>& code_map);

  // Converts a float network to an int network. Each set of input weights that
  // corresponds to a single output weight is converted independently:
  // Compute the max absolute value of the weight set.
  // Scale so the max absolute value becomes INT8_MAX.
  // Round to integer.
  // Store a multiplicative scale factor (as a float) that will reproduce
  // the original value, subject to rounding errors.
  void ConvertToInt();
  // Returns the size rounded up to an internal factor used by the SIMD
  // implementation for its input.
  int RoundInputs(int size) const {
    if (!int_mode_ || !IntSimdMatrix::intSimdMatrix) return size;
    return IntSimdMatrix::intSimdMatrix->RoundInputs(size);
  }

  // Accessors.
  bool is_int_mode() const {
    return int_mode_;
  }
  int NumOutputs() const { return int_mode_ ? wi_.dim1() : wf_.dim1(); }
  // Provides one set of weights. Only used by peep weight maxpool.
  const double* GetWeights(int index) const { return wf_[index]; }
  // Provides access to the deltas (dw_).
  double GetDW(int i, int j) const { return dw_(i, j); }

  // Allocates any needed memory for running Backward, and zeroes the deltas,
  // thus eliminating any existing momentum.
  void InitBackward();

  // Writes to the given file. Returns false in case of error.
  bool Serialize(bool training, TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  bool DeSerialize(bool training, TFile* fp);
  // As DeSerialize, but reads an old (float) format WeightMatrix for
  // backward compatibility.
  bool DeSerializeOld(bool training, TFile* fp);

  // Computes matrix.vector v = Wu.
  // u is of size W.dim2() - 1 and the output v is of size W.dim1().
  // u is imagined to have an extra element at the end with value 1, to
  // implement the bias, but it doesn't actually have it.
  // Asserts that the call matches what we have.
  void MatrixDotVector(const double* u, double* v) const;
  void MatrixDotVector(const int8_t* u, double* v) const;
  // MatrixDotVector for peep weights, MultiplyAccumulate adds the
  // component-wise products of *this[0] and v to inout.
  void MultiplyAccumulate(const double* v, double* inout);
  // Computes vector.matrix v = uW.
  // u is of size W.dim1() and the output v is of size W.dim2() - 1.
  // The last result is discarded, as v is assumed to have an imaginary
  // last value of 1, as with MatrixDotVector.
  void VectorDotMatrix(const double* u, double* v) const;
  // Fills dw_[i][j] with the dot product u[i][] . v[j][], using elements
  // from u and v, starting with u[i][offset] and v[j][offset].
  // Note that (matching MatrixDotVector) v[last][] is missing, presumed 1.0.
  // Runs parallel if requested. Note that inputs must be transposed.
  void SumOuterTransposed(const TransposedArray& u, const TransposedArray& v,
                          bool parallel);
  // Updates the weights using the given learning rate, momentum and adam_beta.
  // num_samples is used in the Adam correction factor.
  void Update(double learning_rate, double momentum, double adam_beta,
              int num_samples);
  // Adds the dw_ in other to the dw_ is *this.
  void AddDeltas(const WeightMatrix& other);
  // Sums the products of weight updates in *this and other, splitting into
  // positive (same direction) in *same and negative (different direction) in
  // *changed.
  void CountAlternators(const WeightMatrix& other, double* same,
                        double* changed) const;

  void Debug2D(const char* msg);

  // Utility function converts an array of float to the corresponding array
  // of double.
  static void FloatToDouble(const GENERIC_2D_ARRAY<float>& wf,
                            GENERIC_2D_ARRAY<double>* wd);

 private:
  // Choice between float and 8 bit int implementations.
  GENERIC_2D_ARRAY<double> wf_;
  GENERIC_2D_ARRAY<int8_t> wi_;
  // Transposed copy of wf_, used only for Backward, and set with each Update.
  TransposedArray wf_t_;
  // Which of wf_ and wi_ are we actually using.
  bool int_mode_;
  // True if we are running adam in this weight matrix.
  bool use_adam_;
  // If we are using wi_, then scales_ is a factor to restore the row product
  // with a vector to the correct range.
  GenericVector<double> scales_;
  // Weight deltas. dw_ is the new delta, and updates_ the momentum-decaying
  // amount to be added to wf_/wi_.
  GENERIC_2D_ARRAY<double> dw_;
  GENERIC_2D_ARRAY<double> updates_;
  // Iff use_adam_, the sum of squares of dw_. The number of samples is
  // given to Update(). Serialized iff use_adam_.
  GENERIC_2D_ARRAY<double> dw_sq_sum_;
  // The weights matrix reorganized in whatever way suits this instance.
  std::vector<int8_t> shaped_w_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_WEIGHTMATRIX_H_
