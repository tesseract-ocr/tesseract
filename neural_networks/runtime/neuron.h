// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// neuron.h: Declarations of a class for an object that
// represents a single neuron in a neural network
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NEURON_H
#define NEURON_H

#include <math.h>
#include <vector>

#ifdef USE_STD_NAMESPACE
using std::vector;
#endif

namespace tesseract {

// Input Node bias values
static const float kInputNodeBias = 0.0f;

class Neuron {
  public:
    // Types of nodes
    enum NeuronTypes {
      Unknown = 0,
      Input,
      Hidden,
      Output
    };
    Neuron();
    ~Neuron();
    // set the forward dirty flag indicating that the
    // activation of the net is not fresh
    void Clear() {
      frwd_dirty_  =  true;
    }
    // Read a binary representation of the neuron info from
    // an input buffer.
    template <class BuffType> bool ReadBinary(BuffType *input_buff) {
      float val;
      if (input_buff->Read(&val, sizeof(val)) != sizeof(val)) {
        return false;
      }
      // input nodes should have no biases
      if (node_type_ == Input) {
        bias_ = kInputNodeBias;
      } else {
        bias_ = val;
      }
      // read fanin count
      int fan_in_cnt;
      if (input_buff->Read(&fan_in_cnt, sizeof(fan_in_cnt)) !=
          sizeof(fan_in_cnt)) {
        return false;
      }
      // validate fan-in cnt
      if (fan_in_cnt != fan_in_.size()) {
        return false;
      }
      // read the weights
      for (int in = 0; in < fan_in_cnt; in++) {
        if (input_buff->Read(&val, sizeof(val)) != sizeof(val)) {
          return false;
        }
        *(fan_in_weights_[in]) = val;
      }
      return true;
    }

    // Add a new connection from this neuron *From*
    // a target neuron using specfied params
    // Note that what is actually copied in this function are pointers to the
    // specified Neurons and weights and not the actualt values. This is by
    // design to centralize the alloction of neurons and weights and so
    // increase the locality of reference and improve cache-hits resulting
    // in a faster net. This technique resulted in a 2X-10X speedup
    // (depending on network size and processor)
    void AddFromConnection(Neuron *neuron_vec,
                           float *wts_offset,
                           int from_cnt);
    // Set the type of a neuron
    void set_node_type(NeuronTypes type);
    // Computes the output of the node by
    // "pulling" the output of the fan-in nodes
    void FeedForward();
    // fast computation of sigmoid function using a lookup table
    // defined in sigmoid_table.cpp
    static float Sigmoid(float activation);
    // Accessor functions
    float output() const {
      return output_;
    }
    void set_output(float out_val) {
      output_ = out_val;
    }
    int id() const {
      return id_;
    }
    int fan_in_cnt() const {
      return fan_in_.size();
    }
    Neuron * fan_in(int idx) const {
      return fan_in_[idx];
    }
    float fan_in_wts(int idx) const {
      return *(fan_in_weights_[idx]);
    }
    void set_id(int id) {
      id_ = id;
    }
    float bias() const {
      return bias_;
    }
    Neuron::NeuronTypes node_type() const {
      return node_type_;
    }

  protected:
    // Type of Neuron
    NeuronTypes node_type_;
    // unqique id of the neuron
    int id_;
    // node bias
    float bias_;
    // node net activation
    float activation_;
    // node output
    float output_;
    // pointers to fanin nodes
    vector<Neuron *> fan_in_;
    // pointers to fanin weights
    vector<float *> fan_in_weights_;
    // Sigmoid function lookup table used for fast computation
    // of sigmoid function
    static const float kSigmoidTable[];
    // flag determining if the activation of the node
    // is fresh or not (dirty)
    bool frwd_dirty_;
    // Initializer
    void Init();
};
}

#endif  // NEURON_H__
