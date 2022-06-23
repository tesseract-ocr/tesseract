///////////////////////////////////////////////////////////////////////
// File:        plumbing.h
// Description: Base class for networks that organize other networks
//              eg series or parallel.
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

#ifndef TESSERACT_LSTM_PLUMBING_H_
#define TESSERACT_LSTM_PLUMBING_H_

#include "matrix.h"
#include "network.h"

namespace tesseract {

// Holds a collection of other networks and forwards calls to each of them.
class TESS_API Plumbing : public Network {
public:
  // ni_ and no_ will be set by AddToStack.
  explicit Plumbing(const std::string &name);
  ~Plumbing() override {
    for (auto data : stack_) {
      delete data;
    }
  }

  // Returns the required shape input to the network.
  StaticShape InputShape() const override {
    return stack_[0]->InputShape();
  }
  std::string spec() const override {
    return "Sub-classes of Plumbing must implement spec()!";
  }

  // Returns true if the given type is derived from Plumbing, and thus contains
  // multiple sub-networks that can have their own learning rate.
  bool IsPlumbingType() const override {
    return true;
  }

  // Suspends/Enables training by setting the training_ flag. Serialize and
  // DeSerialize only operate on the run-time data if state is false.
  void SetEnableTraining(TrainingState state) override;

  // Sets flags that control the action of the network. See NetworkFlags enum
  // for bit values.
  void SetNetworkFlags(uint32_t flags) override;

  // Sets up the network for training. Initializes weights using weights of
  // scale `range` picked according to the random number generator `randomizer`.
  // Note that randomizer is a borrowed pointer that should outlive the network
  // and should not be deleted by any of the networks.
  // Returns the number of weights initialized.
  int InitWeights(float range, TRand *randomizer) override;
  // Recursively searches the network for softmaxes with old_no outputs,
  // and remaps their outputs according to code_map. See network.h for details.
  int RemapOutputs(int old_no, const std::vector<int> &code_map) override;

  // Converts a float network to an int network.
  void ConvertToInt() override;

  // Provides a pointer to a TRand for any networks that care to use it.
  // Note that randomizer is a borrowed pointer that should outlive the network
  // and should not be deleted by any of the networks.
  void SetRandomizer(TRand *randomizer) override;

  // Adds the given network to the stack.
  virtual void AddToStack(Network *network);

  // Sets needs_to_backprop_ to needs_backprop and returns true if
  // needs_backprop || any weights in this network so the next layer forward
  // can be told to produce backprop for this layer if needed.
  bool SetupNeedsBackprop(bool needs_backprop) override;

  // Returns an integer reduction factor that the network applies to the
  // time sequence. Assumes that any 2-d is already eliminated. Used for
  // scaling bounding boxes of truth data.
  // WARNING: if GlobalMinimax is used to vary the scale, this will return
  // the last used scale factor. Call it before any forward, and it will return
  // the minimum scale factor of the paths through the GlobalMinimax.
  int XScaleFactor() const override;

  // Provides the (minimum) x scale factor to the network (of interest only to
  // input units) so they can determine how to scale bounding boxes.
  void CacheXScaleFactor(int factor) override;

  // Provides debug output on the weights.
  void DebugWeights() override;

  // Returns the current stack.
  const std::vector<Network *> &stack() const {
    return stack_;
  }
  // Returns a set of strings representing the layer-ids of all layers below.
  void EnumerateLayers(const std::string *prefix, std::vector<std::string> &layers) const;
  // Returns a pointer to the network layer corresponding to the given id.
  Network *GetLayer(const char *id) const;
  // Returns the learning rate for a specific layer of the stack.
  float LayerLearningRate(const char *id) {
    const float *lr_ptr = LayerLearningRatePtr(id);
    ASSERT_HOST(lr_ptr != nullptr);
    return *lr_ptr;
  }
  // Scales the learning rate for a specific layer of the stack.
  void ScaleLayerLearningRate(const char *id, double factor) {
    float *lr_ptr = LayerLearningRatePtr(id);
    ASSERT_HOST(lr_ptr != nullptr);
    *lr_ptr *= factor;
  }

  // Set the learning rate for a specific layer of the stack to the given value.
  void SetLayerLearningRate(const char *id, float learning_rate) {
    float *lr_ptr = LayerLearningRatePtr(id);
    ASSERT_HOST(lr_ptr != nullptr);
    *lr_ptr = learning_rate;
  }

  // Returns a pointer to the learning rate for the given layer id.
  float *LayerLearningRatePtr(const char *id);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(TFile *fp) const override;
  // Reads from the given file. Returns false in case of error.
  bool DeSerialize(TFile *fp) override;

  // Updates the weights using the given learning rate, momentum and adam_beta.
  // num_samples is used in the adam computation iff use_adam_ is true.
  void Update(float learning_rate, float momentum, float adam_beta, int num_samples) override;
  // Sums the products of weight updates in *this and other, splitting into
  // positive (same direction) in *same and negative (different direction) in
  // *changed.
  void CountAlternators(const Network &other, TFloat *same, TFloat *changed) const override;

protected:
  // The networks.
  std::vector<Network *> stack_;
  // Layer-specific learning rate iff network_flags_ & NF_LAYER_SPECIFIC_LR.
  // One element for each element of stack_.
  std::vector<float> learning_rates_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_PLUMBING_H_
