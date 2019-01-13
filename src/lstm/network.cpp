///////////////////////////////////////////////////////////////////////
// File:        network.cpp
// Description: Base class for neural network implementations.
// Author:      Ray Smith
// Created:     Wed May 01 17:25:06 PST 2013
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

// String names corresponding to the NetworkType enum. Keep in sync.
// Names used in Serialization to allow re-ordering/addition/deletion of
// layer types in NetworkType without invalidating existing network files.
char const* const Network::kTypeNames[NT_COUNT] = {
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

// Reads from the given file. Returns false in case of error.
// Should be overridden by subclasses, but NOT called by their DeSerialize.
bool Network::DeSerialize(TFile* fp) {
  int8_t data;
  if (!fp->DeSerialize(&data)) return false;
  if (data == NT_NONE) {
    STRING type_name;
    if (!type_name.DeSerialize(fp)) return false;
    for (data = 0; data < NT_COUNT && type_name != kTypeNames[data]; ++data) {
    }
    if (data == NT_COUNT) {
      tprintf("Invalid network layer type:%s\n", type_name.string());
      return false;
    }
  }
  type_ = static_cast<NetworkType>(data);
  if (!fp->DeSerialize(&data)) return false;
  training_ = data == TS_ENABLED ? TS_ENABLED : TS_DISABLED;
  if (!fp->DeSerialize(&data)) return false;
  needs_to_backprop_ = data != 0;
  if (!fp->DeSerialize(&network_flags_)) return false;
  if (!fp->DeSerialize(&ni_)) return false;
  if (!fp->DeSerialize(&no_)) return false;
  if (!fp->DeSerialize(&num_weights_)) return false;
  if (!name_.DeSerialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns nullptr in case of error.
// Determines the type of the serialized class and calls its DeSerialize
// on a new object of the appropriate type, which is returned.
Network* Network::CreateFromFile(TFile* fp) {
  Network stub;
  if (!stub.DeSerialize(fp)) return nullptr;
  Network* network = nullptr;
  switch (stub.type_) {
    case NT_CONVOLVE:
      network = new Convolve(stub.name_, stub.ni_, 0, 0);
      break;
    case NT_INPUT:
      network = new Input(stub.name_, stub.ni_, stub.no_);
      break;
    case NT_LSTM:
    case NT_LSTM_SOFTMAX:
    case NT_LSTM_SOFTMAX_ENCODED:
    case NT_LSTM_SUMMARY:
      network =
          new LSTM(stub.name_, stub.ni_, stub.no_, stub.no_, false, stub.type_);
      break;
    case NT_MAXPOOL:
      network = new Maxpool(stub.name_, stub.ni_, 0, 0);
      break;
    // All variants of Parallel.
    case NT_PARALLEL:
    case NT_REPLICATED:
    case NT_PAR_RL_LSTM:
    case NT_PAR_UD_LSTM:
    case NT_PAR_2D_LSTM:
      network = new Parallel(stub.name_, stub.type_);
      break;
    case NT_RECONFIG:
      network = new Reconfig(stub.name_, stub.ni_, 0, 0);
      break;
    // All variants of reversed.
    case NT_XREVERSED:
    case NT_YREVERSED:
    case NT_XYTRANSPOSE:
      network = new Reversed(stub.name_, stub.type_);
      break;
    case NT_SERIES:
      network = new Series(stub.name_);
      break;
    case NT_TENSORFLOW:
#ifdef INCLUDE_TENSORFLOW
      network = new TFNetwork(stub.name_);
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
      network = new FullyConnected(stub.name_, stub.ni_, stub.no_, stub.type_);
      break;
    default:
      break;
  }
  if (network) {
    network->training_ = stub.training_;
    network->needs_to_backprop_ = stub.needs_to_backprop_;
    network->network_flags_ = stub.network_flags_;
    network->num_weights_ = stub.num_weights_;
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

// === Debug image display methods. ===
// Displays the image of the matrix to the forward window.
void Network::DisplayForward(const NetworkIO& matrix) {
#ifndef GRAPHICS_DISABLED  // do nothing if there's no graphics
  Pix* image = matrix.ToPix();
  ClearWindow(false, name_.string(), pixGetWidth(image),
              pixGetHeight(image), &forward_win_);
  DisplayImage(image, forward_win_);
  forward_win_->Update();
#endif  // GRAPHICS_DISABLED
}

// Displays the image of the matrix to the backward window.
void Network::DisplayBackward(const NetworkIO& matrix) {
#ifndef GRAPHICS_DISABLED  // do nothing if there's no graphics
  Pix* image = matrix.ToPix();
  STRING window_name = name_ + "-back";
  ClearWindow(false, window_name.string(), pixGetWidth(image),
              pixGetHeight(image), &backward_win_);
  DisplayImage(image, backward_win_);
  backward_win_->Update();
#endif  // GRAPHICS_DISABLED
}

#ifndef GRAPHICS_DISABLED
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
#endif  // GRAPHICS_DISABLED

}  // namespace tesseract.
