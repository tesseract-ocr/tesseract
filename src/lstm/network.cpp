///////////////////////////////////////////////////////////////////////
// File:        network.cpp
// Description: Base class for neural network implementations.
// Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "network.h"

#include <cstdlib>

// This base class needs to know about all its sub-classes because of the
// factory deserializing method: CreateFromFile.
#include "allheaders.h"
#include "convolve.h"
#include "fullyconnected.h"
#include "input.h"
#include "lstm.h"
#include "maxpool.h"
#include "parallel.h"
#include "reconfig.h"
#include "reversed.h"
#include "scrollview.h"
#include "series.h"
#include "statistc.h"
#ifdef INCLUDE_TENSORFLOW
#include "tfnetwork.h"
#endif
#include "tprintf.h"

namespace tesseract {

// Min and max window sizes.
const int kMinWinSize = 500;
const int kMaxWinSize = 2000;
// Window frame sizes need adding on to make the content fit.
const int kXWinFrameSize = 30;
const int kYWinFrameSize = 80;

// String names corresponding to the NetworkType enum.
// Keep in sync with NetworkType.
// Names used in Serialization to allow re-ordering/addition/deletion of
// layer types in NetworkType without invalidating existing network files.
static char const* const kTypeNames[NT_COUNT] = {
    "Invalid",     "Input",
    "Convolve",    "Maxpool",
    "Parallel",    "Replicated",
    "ParBidiLSTM", "DepParUDLSTM",
    "Par2dLSTM",   "Series",
    "Reconfig",    "RTLReversed",
    "TTBReversed", "XYTranspose",
    "LSTM",        "SummLSTM",
    "Logistic",    "LinLogistic",
    "LinTanh",     "Tanh",
    "Relu",        "Linear",
    "Softmax",     "SoftmaxNoCTC",
    "LSTMSoftmax", "LSTMBinarySoftmax",
    "TensorFlow",
};

Network::Network()
    : type_(NT_NONE),
      training_(TS_ENABLED),
      needs_to_backprop_(true),
      network_flags_(0),
      ni_(0),
      no_(0),
      num_weights_(0),
      forward_win_(nullptr),
      backward_win_(nullptr),
      randomizer_(nullptr) {}
Network::Network(NetworkType type, const STRING& name, int ni, int no)
    : type_(type),
      training_(TS_ENABLED),
      needs_to_backprop_(true),
      network_flags_(0),
      ni_(ni),
      no_(no),
      num_weights_(0),
      name_(name),
      forward_win_(nullptr),
      backward_win_(nullptr),
      randomizer_(nullptr) {}


// Suspends/Enables/Permanently disables training by setting the training_
// flag. Serialize and DeSerialize only operate on the run-time data if state
// is TS_DISABLED or TS_TEMP_DISABLE. Specifying TS_TEMP_DISABLE will
// temporarily disable layers in state TS_ENABLED, allowing a trainer to
// serialize as if it were a recognizer.
// TS_RE_ENABLE will re-enable layers that were previously in any disabled
// state. If in TS_TEMP_DISABLE then the flag is just changed, but if in
// TS_DISABLED, the deltas in the weight matrices are reinitialized so that a
// recognizer can be converted back to a trainer.
void Network::SetEnableTraining(TrainingState state) {
  if (state == TS_RE_ENABLE) {
    // Enable only from temp disabled.
    if (training_ == TS_TEMP_DISABLE) training_ = TS_ENABLED;
  } else if (state == TS_TEMP_DISABLE) {
    // Temp disable only from enabled.
    if (training_ == TS_ENABLED) training_ = state;
  } else {
    training_ = state;
  }
}

// Sets flags that control the action of the network. See NetworkFlags enum
// for bit values.
void Network::SetNetworkFlags(uint32_t flags) {
  network_flags_ = flags;
}

// Sets up the network for training. Initializes weights using weights of
// scale `range` picked according to the random number generator `randomizer`.
int Network::InitWeights(float range, TRand* randomizer) {
  randomizer_ = randomizer;
  return 0;
}

// Provides a pointer to a TRand for any networks that care to use it.
// Note that randomizer is a borrowed pointer that should outlive the network
// and should not be deleted by any of the networks.
void Network::SetRandomizer(TRand* randomizer) {
  randomizer_ = randomizer;
}

// Sets needs_to_backprop_ to needs_backprop and returns true if
// needs_backprop || any weights in this network so the next layer forward
// can be told to produce backprop for this layer if needed.
bool Network::SetupNeedsBackprop(bool needs_backprop) {
  needs_to_backprop_ = needs_backprop;
  return needs_backprop || num_weights_ > 0;
}

// Writes to the given file. Returns false in case of error.
bool Network::Serialize(TFile* fp) const {
  int8_t data = NT_NONE;
  if (!fp->Serialize(&data)) return false;
  STRING type_name = kTypeNames[type_];
  if (!type_name.Serialize(fp)) return false;
  data = training_;
  if (!fp->Serialize(&data)) return false;
  data = needs_to_backprop_;
  if (!fp->Serialize(&data)) return false;
  if (!fp->Serialize(&network_flags_)) return false;
  if (!fp->Serialize(&ni_)) return false;
  if (!fp->Serialize(&no_)) return false;
  if (!fp->Serialize(&num_weights_)) return false;
  if (!name_.Serialize(fp)) return false;
  return true;
}

static NetworkType getNetworkType(TFile* fp) {
  int8_t data;
  if (!fp->DeSerialize(&data)) return NT_NONE;
  if (data == NT_NONE) {
    STRING type_name;
    if (!type_name.DeSerialize(fp)) return NT_NONE;
    for (data = 0; data < NT_COUNT && type_name != kTypeNames[data]; ++data) {
    }
    if (data == NT_COUNT) {
      tprintf("Invalid network layer type:%s\n", type_name.c_str());
      return NT_NONE;
    }
  }
  return static_cast<NetworkType>(data);
}

// Reads from the given file. Returns nullptr in case of error.
// Determines the type of the serialized class and calls its DeSerialize
// on a new object of the appropriate type, which is returned.
Network* Network::CreateFromFile(TFile* fp) {
  NetworkType type;          // Type of the derived network class.
  TrainingState training;    // Are we currently training?
  bool needs_to_backprop;    // This network needs to output back_deltas.
  int32_t network_flags;     // Behavior control flags in NetworkFlags.
  int32_t ni;                // Number of input values.
  int32_t no;                // Number of output values.
  int32_t num_weights;       // Number of weights in this and sub-network.
  STRING name;               // A unique name for this layer.
  int8_t data;
  Network* network = nullptr;
  type = getNetworkType(fp);
  if (!fp->DeSerialize(&data)) return nullptr;
  training = data == TS_ENABLED ? TS_ENABLED : TS_DISABLED;
  if (!fp->DeSerialize(&data)) return nullptr;
  needs_to_backprop = data != 0;
  if (!fp->DeSerialize(&network_flags)) return nullptr;
  if (!fp->DeSerialize(&ni)) return nullptr;
  if (!fp->DeSerialize(&no)) return nullptr;
  if (!fp->DeSerialize(&num_weights)) return nullptr;
  if (!name.DeSerialize(fp)) return nullptr;

  switch (type) {
    case NT_CONVOLVE:
      network = new Convolve(name, ni, 0, 0);
      break;
    case NT_INPUT:
      network = new Input(name, ni, no);
      break;
    case NT_LSTM:
    case NT_LSTM_SOFTMAX:
    case NT_LSTM_SOFTMAX_ENCODED:
    case NT_LSTM_SUMMARY:
      network =
          new LSTM(name, ni, no, no, false, type);
      break;
    case NT_MAXPOOL:
      network = new Maxpool(name, ni, 0, 0);
      break;
    // All variants of Parallel.
    case NT_PARALLEL:
    case NT_REPLICATED:
    case NT_PAR_RL_LSTM:
    case NT_PAR_UD_LSTM:
    case NT_PAR_2D_LSTM:
      network = new Parallel(name, type);
      break;
    case NT_RECONFIG:
      network = new Reconfig(name, ni, 0, 0);
      break;
    // All variants of reversed.
    case NT_XREVERSED:
    case NT_YREVERSED:
    case NT_XYTRANSPOSE:
      network = new Reversed(name, type);
      break;
    case NT_SERIES:
      network = new Series(name);
      break;
    case NT_TENSORFLOW:
#ifdef INCLUDE_TENSORFLOW
      network = new TFNetwork(name);
#else
      tprintf("TensorFlow not compiled in! -DINCLUDE_TENSORFLOW\n");
#endif
      break;
    // All variants of FullyConnected.
    case NT_SOFTMAX:
    case NT_SOFTMAX_NO_CTC:
    case NT_RELU:
    case NT_TANH:
    case NT_LINEAR:
    case NT_LOGISTIC:
    case NT_POSCLIP:
    case NT_SYMCLIP:
      network = new FullyConnected(name, ni, no, type);
      break;
    default:
      break;
  }
  if (network) {
    network->training_ = training;
    network->needs_to_backprop_ = needs_to_backprop;
    network->network_flags_ = network_flags;
    network->num_weights_ = num_weights;
    if (!network->DeSerialize(fp)) {
      delete network;
      network = nullptr;
    }
  }
  return network;
}

// Returns a random number in [-range, range].
double Network::Random(double range) {
  ASSERT_HOST(randomizer_ != nullptr);
  return randomizer_->SignedRand(range);
}

#ifndef GRAPHICS_DISABLED

// === Debug image display methods. ===
// Displays the image of the matrix to the forward window.
void Network::DisplayForward(const NetworkIO& matrix) {
  Pix* image = matrix.ToPix();
  ClearWindow(false, name_.c_str(), pixGetWidth(image),
              pixGetHeight(image), &forward_win_);
  DisplayImage(image, forward_win_);
  forward_win_->Update();
}

// Displays the image of the matrix to the backward window.
void Network::DisplayBackward(const NetworkIO& matrix) {
  Pix* image = matrix.ToPix();
  STRING window_name = name_ + "-back";
  ClearWindow(false, window_name.c_str(), pixGetWidth(image),
              pixGetHeight(image), &backward_win_);
  DisplayImage(image, backward_win_);
  backward_win_->Update();
}

// Creates the window if needed, otherwise clears it.
void Network::ClearWindow(bool tess_coords, const char* window_name,
                          int width, int height, ScrollView** window) {
  if (*window == nullptr) {
    int min_size = std::min(width, height);
    if (min_size < kMinWinSize) {
      if (min_size < 1) min_size = 1;
      width = width * kMinWinSize / min_size;
      height = height * kMinWinSize / min_size;
    }
    width += kXWinFrameSize;
    height += kYWinFrameSize;
    if (width > kMaxWinSize) width = kMaxWinSize;
    if (height > kMaxWinSize) height = kMaxWinSize;
    *window = new ScrollView(window_name, 80, 100, width, height, width, height,
                             tess_coords);
    tprintf("Created window %s of size %d, %d\n", window_name, width, height);
  } else {
    (*window)->Clear();
  }
}

// Displays the pix in the given window. and returns the height of the pix.
// The pix is pixDestroyed.
int Network::DisplayImage(Pix* pix, ScrollView* window) {
  int height = pixGetHeight(pix);
  window->Image(pix, 0, 0);
  pixDestroy(&pix);
  return height;
}
#endif // !GRAPHICS_DISABLED

}  // namespace tesseract.
