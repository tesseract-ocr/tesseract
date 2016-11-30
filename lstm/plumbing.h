///////////////////////////////////////////////////////////////////////
// File:        plumbing.h
// Description: Base class for networks that organize other networks
//              eg series or parallel.
// Author:      Ray Smith
// Created:     Mon May 12 08:11:36 PST 2014
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

#ifndef TESSERACT_LSTM_PLUMBING_H_
#define TESSERACT_LSTM_PLUMBING_H_

#include "genericvector.h"
#include "matrix.h"
#include "network.h"

namespace tesseract {

// Holds a collection of other networks and forwards calls to each of them.
class Plumbing : public Network {
 public:
  // ni_ and no_ will be set by AddToStack.
  explicit Plumbing(const STRING& name);
  virtual ~Plumbing();

  // Returns the required shape input to the network.
  virtual StaticShape InputShape() const { return stack_[0]->InputShape(); }
  virtual STRING spec() const {
    return "Sub-classes of Plumbing must implement spec()!";
  }

  // Returns true if the given type is derived from Plumbing, and thus contains
  // multiple sub-networks that can have their own learning rate.
  virtual bool IsPlumbingType() const { return true; }

  // Suspends/Enables training by setting the training_ flag. Serialize and
  // DeSerialize only operate on the run-time data if state is false.
  virtual void SetEnableTraining(TrainingState state);

  // Sets flags that control the action of the network. See NetworkFlags enum
  // for bit values.
  virtual void SetNetworkFlags(uinT32 flags);

  // Sets up the network for training. Initializes weights using weights of
  // scale `range` picked according to the random number generator `randomizer`.
  // Note that randomizer is a borrowed pointer that should outlive the network
  // and should not be deleted by any of the networks.
  // Returns the number of weights initialized.
  virtual int InitWeights(float range, TRand* randomizer);

  // Converts a float network to an int network.
  virtual void ConvertToInt();

  // Provides a pointer to a TRand for any networks that care to use it.
  // Note that randomizer is a borrowed pointer that should outlive the network
  // and should not be deleted by any of the networks.
  virtual void SetRandomizer(TRand* randomizer);

  // Adds the given network to the stack.
  virtual void AddToStack(Network* network);

  // Sets needs_to_backprop_ to needs_backprop and returns true if
  // needs_backprop || any weights in this network so the next layer forward
  // can be told to produce backprop for this layer if needed.
  virtual bool SetupNeedsBackprop(bool needs_backprop);

  // Returns an integer reduction factor that the network applies to the
  // time sequence. Assumes that any 2-d is already eliminated. Used for
  // scaling bounding boxes of truth data.
  // WARNING: if GlobalMinimax is used to vary the scale, this will return
  // the last used scale factor. Call it before any forward, and it will return
  // the minimum scale factor of the paths through the GlobalMinimax.
  virtual int XScaleFactor() const;

  // Provides the (minimum) x scale factor to the network (of interest only to
  // input units) so they can determine how to scale bounding boxes.
  virtual void CacheXScaleFactor(int factor);

  // Provides debug output on the weights.
  virtual void DebugWeights();

  // Returns the current stack.
  const PointerVector<Network>& stack() const {
    return stack_;
  }
  // Returns a set of strings representing the layer-ids of all layers below.
  void EnumerateLayers(const STRING* prefix,
                       GenericVector<STRING>* layers) const;
  // Returns a pointer to the network layer corresponding to the given id.
  Network* GetLayer(const char* id) const;
  // Returns the learning rate for a specific layer of the stack.
  float LayerLearningRate(const char* id) const {
    const float* lr_ptr = LayerLearningRatePtr(id);
    ASSERT_HOST(lr_ptr != NULL);
    return *lr_ptr;
  }
  // Scales the learning rate for a specific layer of the stack.
  void ScaleLayerLearningRate(const char* id, double factor) {
    float* lr_ptr = LayerLearningRatePtr(id);
    ASSERT_HOST(lr_ptr != NULL);
    *lr_ptr *= factor;
  }
  // Returns a pointer to the learning rate for the given layer id.
  float* LayerLearningRatePtr(const char* id) const;

  // Writes to the given file. Returns false in case of error.
  virtual bool Serialize(TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  virtual bool DeSerialize(bool swap, TFile* fp);

  // Updates the weights using the given learning rate and momentum.
  // num_samples is the quotient to be used in the adagrad computation iff
  // use_ada_grad_ is true.
  virtual void Update(float learning_rate, float momentum, int num_samples);
  // Sums the products of weight updates in *this and other, splitting into
  // positive (same direction) in *same and negative (different direction) in
  // *changed.
  virtual void CountAlternators(const Network& other, double* same,
                                double* changed) const;

 protected:
  // The networks.
  PointerVector<Network> stack_;
  // Layer-specific learning rate iff network_flags_ & NF_LAYER_SPECIFIC_LR.
  // One element for each element of stack_.
  GenericVector<float> learning_rates_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_PLUMBING_H_

