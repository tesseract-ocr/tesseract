///////////////////////////////////////////////////////////////////////
// File:        lstm.h
// Description: Long-term-short-term-memory Recurrent neural network.
// Author:      Ray Smith
// Created:     Wed May 01 17:33:06 PST 2013
//
// (C) Copyright 2013, Google Inc.
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

#ifndef TESSERACT_LSTM_LSTM_H_
#define TESSERACT_LSTM_LSTM_H_

#include "network.h"
#include "fullyconnected.h"

namespace tesseract {

// C++ Implementation of the LSTM class from lstm.py.
class LSTM : public Network {
 public:
  // Enum for the different weights in LSTM, to reduce some of the I/O and
  // setup code to loops. The elements of the enum correspond to elements of an
  // array of WeightMatrix or a corresponding array of NetworkIO.
  enum WeightType {
    CI,   // Cell Inputs.
    GI,   // Gate at the input.
    GF1,  // Forget gate at the memory (1-d or looking back 1 timestep).
    GO,   // Gate at the output.
    GFS,  // Forget gate at the memory, looking back in the other dimension.

    WT_COUNT  // Number of WeightTypes.
  };

  // Constructor for NT_LSTM (regular 1 or 2-d LSTM), NT_LSTM_SOFTMAX (LSTM with
  // additional softmax layer included and fed back into the input at the next
  // timestep), or NT_LSTM_SOFTMAX_ENCODED (as LSTM_SOFTMAX, but the feedback
  // is binary encoded instead of categorical) only.
  // 2-d and bidi softmax LSTMs are not rejected, but are impossible to build
  // in the conventional way because the output feedback both forwards and
  // backwards in time does become impossible.
  LSTM(const STRING& name, int num_inputs, int num_states, int num_outputs,
       bool two_dimensional, NetworkType type);
  virtual ~LSTM();

  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  virtual StaticShape OutputShape(const StaticShape& input_shape) const;

  virtual STRING spec() const {
    STRING spec;
    if (type_ == NT_LSTM)
      spec.add_str_int("Lfx", ns_);
    else if (type_ == NT_LSTM_SUMMARY)
      spec.add_str_int("Lfxs", ns_);
    else if (type_ == NT_LSTM_SOFTMAX)
      spec.add_str_int("LS", ns_);
    else if (type_ == NT_LSTM_SOFTMAX_ENCODED)
      spec.add_str_int("LE", ns_);
    if (softmax_ != NULL) spec += softmax_->spec();
    return spec;
  }

  // Suspends/Enables training by setting the training_ flag. Serialize and
  // DeSerialize only operate on the run-time data if state is false.
  virtual void SetEnableTraining(TrainingState state);

  // Sets up the network for training. Initializes weights using weights of
  // scale `range` picked according to the random number generator `randomizer`.
  virtual int InitWeights(float range, TRand* randomizer);

  // Converts a float network to an int network.
  virtual void ConvertToInt();

  // Provides debug output on the weights.
  virtual void DebugWeights();

  // Writes to the given file. Returns false in case of error.
  virtual bool Serialize(TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  virtual bool DeSerialize(bool swap, TFile* fp);

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  virtual void Forward(bool debug, const NetworkIO& input,
                       const TransposedArray* input_transpose,
                       NetworkScratch* scratch, NetworkIO* output);

  // Runs backward propagation of errors on the deltas line.
  // See Network for a detailed discussion of the arguments.
  virtual bool Backward(bool debug, const NetworkIO& fwd_deltas,
                        NetworkScratch* scratch,
                        NetworkIO* back_deltas);
  // Updates the weights using the given learning rate and momentum.
  // num_samples is the quotient to be used in the adagrad computation iff
  // use_ada_grad_ is true.
  virtual void Update(float learning_rate, float momentum, int num_samples);
  // Sums the products of weight updates in *this and other, splitting into
  // positive (same direction) in *same and negative (different direction) in
  // *changed.
  virtual void CountAlternators(const Network& other, double* same,
                                double* changed) const;
  // Prints the weights for debug purposes.
  void PrintW();
  // Prints the weight deltas for debug purposes.
  void PrintDW();

  // Returns true of this is a 2-d lstm.
  bool Is2D() const {
    return is_2d_;
  }

 private:
  // Resizes forward data to cope with an input image of the given width.
  void ResizeForward(const NetworkIO& input);

 private:
  // Size of padded input to weight matrices = ni_ + no_ for 1-D operation
  // and ni_ + 2 * no_ for 2-D operation. Note that there is a phantom 1 input
  // for the bias that makes the weight matrices of size [na + 1][no].
  inT32 na_;
  // Number of internal states. Equal to no_ except for a softmax LSTM.
  // ns_ is NOT serialized, but is calculated from gate_weights_.
  inT32 ns_;
  // Number of additional feedback states. The softmax types feed back
  // additional output information on top of the ns_ internal states.
  // In the case of a binary-coded (EMBEDDED) softmax, nf_ < no_.
  inT32 nf_;
  // Flag indicating 2-D operation.
  bool is_2d_;

  // Gate weight arrays of size [na + 1, no].
  WeightMatrix gate_weights_[WT_COUNT];
  // Used only if this is a softmax LSTM.
  FullyConnected* softmax_;
  // Input padded with previous output of size [width, na].
  NetworkIO source_;
  // Internal state used during forward operation, of size [width, ns].
  NetworkIO state_;
  // State of the 2-d maxpool, generated during forward, used during backward.
  GENERIC_2D_ARRAY<inT8> which_fg_;
  // Internal state saved from forward, but used only during backward.
  NetworkIO node_values_[WT_COUNT];
  // Preserved input stride_map used for Backward when NT_LSTM_SQUASHED.
  StrideMap input_map_;
  int input_width_;
};

}  // namespace tesseract.


#endif  // TESSERACT_LSTM_LSTM_H_
