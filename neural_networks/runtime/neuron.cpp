// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// neuron.cpp: The implementation of a class for an object
// that represents a single neuron in a neural network

#include "neuron.h"
#include "input_file_buffer.h"

namespace tesseract {

// Instantiate all supported templates
template bool Neuron::ReadBinary(InputFileBuffer *input_buffer);

// default and only constructor
Neuron::Neuron() {
  Init();
}

// virtual destructor
Neuron::~Neuron() {
}

// Initializer
void Neuron::Init() {
  id_ = -1;
  frwd_dirty_ = false;
  fan_in_.clear();
  fan_in_weights_.clear();
  activation_ = 0.0f;
  output_ = 0.0f;
  bias_ = 0.0f;
  node_type_ = Unknown;
}

// Computes the activation and output of the neuron if not fresh
// by pulling the outputs of all fan-in neurons
void Neuron::FeedForward() {
  if (!frwd_dirty_ ) {
    return;
  }
  // nothing to do for input nodes: just pass the input to the o/p
  // otherwise, pull the output of all fan-in neurons
  if (node_type_ != Input) {
    int fan_in_cnt = fan_in_.size();
    // sum out the activation
    activation_ = -bias_;
    for (int in = 0; in < fan_in_cnt; in++) {
      if (fan_in_[in]->frwd_dirty_) {
        fan_in_[in]->FeedForward();
      }
      activation_ += ((*(fan_in_weights_[in])) * fan_in_[in]->output_);
    }
    // sigmoid it
    output_ = Sigmoid(activation_);
  }
  frwd_dirty_ = false;
}

// set the type of the neuron
void Neuron::set_node_type(NeuronTypes Type) {
  node_type_ = Type;
}

// Adds new connections *to* this neuron *From*
// a target neuron using specfied params
// Note that what is actually copied in this function are pointers to the
// specified Neurons and weights and not the actualt values. This is by
// design to centralize the alloction of neurons and weights and so
// increase the locality of reference and improve cache-hits resulting
// in a faster net. This technique resulted in a 2X-10X speedup
// (depending on network size and processor)
void Neuron::AddFromConnection(Neuron *neurons,
                               float *wts_offset,
                               int from_cnt) {
  for (int in = 0; in < from_cnt; in++) {
    fan_in_.push_back(neurons + in);
    fan_in_weights_.push_back(wts_offset + in);
  }
}

// fast computation of sigmoid function using a lookup table
// defined in sigmoid_table.cpp
float Neuron::Sigmoid(float activation) {
  if (activation <= -10.0f) {
    return 0.0f;
  } else if (activation >= 10.0f) {
    return 1.0f;
  } else {
    return kSigmoidTable[static_cast<int>(100 * (activation + 10.0))];
  }
}
}
