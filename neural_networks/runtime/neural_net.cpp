// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// neural_net.cpp: Declarations of a class for an object that
// represents an arbitrary network of neurons
//
#include <vector>
#include <string>
#include "neural_net.h"
#include "input_file_buffer.h"

namespace tesseract {

NeuralNet::NeuralNet() {
  Init();
}

NeuralNet::~NeuralNet() {
  // clean up the wts chunks vector
  for (int vec = 0; vec < static_cast<int>(wts_vec_.size()); vec++) {
    delete wts_vec_[vec];
  }
  // clean up neurons
  delete []neurons_;
  // clean up nodes
  for (int node_idx = 0; node_idx < neuron_cnt_; node_idx++) {
    delete []fast_nodes_[node_idx].inputs;
  }

}

// Initiaization function
void NeuralNet::Init() {
  read_only_ = true;
  auto_encoder_ = false;
  alloc_wgt_cnt_ = 0;
  wts_cnt_ = 0;
  neuron_cnt_ = 0;
  in_cnt_ = 0;
  out_cnt_ = 0;
  wts_vec_.clear();
  neurons_ = NULL;
  inputs_mean_.clear();
  inputs_std_dev_.clear();
  inputs_min_.clear();
  inputs_max_.clear();
}

// Does a fast feedforward for read_only nets
// Templatized for float and double Types
template <typename Type> bool NeuralNet::FastFeedForward(const Type *inputs,
                                                         Type *outputs) {
  int node_idx = 0;
  Node *node = &fast_nodes_[0];
  // feed inputs in and offset them by the pre-computed bias
  for (node_idx = 0; node_idx < in_cnt_; node_idx++, node++) {
    node->out = inputs[node_idx] - node->bias;
  }
  // compute nodes activations and outputs
  for (;node_idx < neuron_cnt_; node_idx++, node++) {
    double activation = -node->bias;
    for (int fan_in_idx = 0; fan_in_idx < node->fan_in_cnt; fan_in_idx++) {
      activation += (node->inputs[fan_in_idx].input_weight *
                     node->inputs[fan_in_idx].input_node->out);
    }
    node->out = Neuron::Sigmoid(activation);
  }
  // copy the outputs to the output buffers
  node = &fast_nodes_[neuron_cnt_ - out_cnt_];
  for (node_idx = 0; node_idx < out_cnt_; node_idx++, node++) {
    outputs[node_idx] = node->out;
  }
  return true;
}

// Performs a feedforward for general nets. Used mainly in training mode
// Templatized for float and double Types
template <typename Type> bool NeuralNet::FeedForward(const Type *inputs,
                                                     Type *outputs) {
  // call the fast version in case of readonly nets
  if (read_only_) {
    return FastFeedForward(inputs, outputs);
  }
  // clear all neurons
  Clear();
  // for auto encoders, apply no input normalization
  if (auto_encoder_) {
    for (int in = 0; in < in_cnt_; in++) {
      neurons_[in].set_output(inputs[in]);
    }
  } else {
    // Input normalization : subtract mean and divide by stddev
    for (int in = 0; in < in_cnt_; in++) {
      neurons_[in].set_output((inputs[in] - inputs_min_[in]) /
                              (inputs_max_[in] - inputs_min_[in]));
      neurons_[in].set_output((neurons_[in].output() - inputs_mean_[in]) /
                              inputs_std_dev_[in]);
    }
  }
  // compute the net outputs: follow a pull model each output pulls the
  // outputs of its input nodes and so on
  for (int out = neuron_cnt_ - out_cnt_; out < neuron_cnt_; out++) {
    neurons_[out].FeedForward();
    // copy the values to the output buffer
    outputs[out] = neurons_[out].output();
  }
  return true;
}

// Sets a connection between two neurons
bool NeuralNet::SetConnection(int from, int to) {
  // allocate the wgt
  float *wts  =  AllocWgt(1);
  if (wts == NULL) {
    return false;
  }
  // register the connection
  neurons_[to].AddFromConnection(neurons_ + from, wts, 1);
  return true;
}

// Create a fast readonly version of the net
bool NeuralNet::CreateFastNet() {
  fast_nodes_.resize(neuron_cnt_);
  // build the node structures
  int wts_cnt = 0;
  for (int node_idx = 0; node_idx < neuron_cnt_; node_idx++) {
    Node *node = &fast_nodes_[node_idx];
    if (neurons_[node_idx].node_type() == Neuron::Input) {
      // Input neurons have no fan-in
      node->fan_in_cnt = 0;
      node->inputs = NULL;
      // Input bias is the normalization offset computed from
      // training input stats
      if (fabs(inputs_max_[node_idx] - inputs_min_[node_idx]) <
          kMinInputRange) {
        // if the range approaches zero, the stdev is not defined,
        // this indicates that this input does not change.
        // Set the bias to zero
        node->bias = 0.0f;
      } else {
        node->bias = inputs_min_[node_idx] + (inputs_mean_[node_idx] *
            (inputs_max_[node_idx] - inputs_min_[node_idx]));
      }
    } else {
      node->bias = neurons_[node_idx].bias();
      node->fan_in_cnt = neurons_[node_idx].fan_in_cnt();
      // allocate memory for fan-in nodes
      node->inputs = new WeightedNode[node->fan_in_cnt];
      if (node->inputs == NULL) {
        return false;
      }
      for (int fan_in = 0; fan_in < node->fan_in_cnt; fan_in++) {
        // identify fan-in neuron
        const int id = neurons_[node_idx].fan_in(fan_in)->id();
        // Feedback connections are not allowed and should never happen
        if (id >= node_idx) {
          return false;
        }
        // add the the fan-in neuron and its wgt
        node->inputs[fan_in].input_node = &fast_nodes_[id];
        float wgt_val = neurons_[node_idx].fan_in_wts(fan_in);
        // for input neurons normalize the wgt by the input scaling
        // values to save time during feedforward
        if (neurons_[node_idx].fan_in(fan_in)->node_type() == Neuron::Input) {
          // if the range approaches zero, the stdev is not defined,
          // this indicates that this input does not change.
          // Set the weight to zero
          if (fabs(inputs_max_[id] - inputs_min_[id]) < kMinInputRange) {
            wgt_val = 0.0f;
          } else {
            wgt_val /= ((inputs_max_[id] - inputs_min_[id]) *
                inputs_std_dev_[id]);
          }
        }
        node->inputs[fan_in].input_weight = wgt_val;
      }
      // incr wgt count to validate against at the end
      wts_cnt += node->fan_in_cnt;
    }
  }
  // sanity check
  return wts_cnt_ == wts_cnt;
}

// returns a pointer to the requested set of weights
// Allocates in chunks
float * NeuralNet::AllocWgt(int wgt_cnt) {
  // see if need to allocate a new chunk of wts
  if (wts_vec_.size() == 0 || (alloc_wgt_cnt_ + wgt_cnt) > kWgtChunkSize) {
    // add the new chunck to the wts_chunks vector
    wts_vec_.push_back(new vector<float> (kWgtChunkSize));
    alloc_wgt_cnt_ = 0;
  }
  float *ret_ptr = &((*wts_vec_.back())[alloc_wgt_cnt_]);
  // incr usage counts
  alloc_wgt_cnt_ += wgt_cnt;
  wts_cnt_ += wgt_cnt;
  return ret_ptr;
}

// create a new net object using an input file as a source
NeuralNet *NeuralNet::FromFile(const string file_name) {
  // open the file
  InputFileBuffer   input_buff(file_name);
  // create a new net object using input buffer
  NeuralNet *net_obj = FromInputBuffer(&input_buff);
  return net_obj;
}

// create a net object from an input buffer
NeuralNet *NeuralNet::FromInputBuffer(InputFileBuffer *ib) {
      // create a new net object
  NeuralNet *net_obj = new NeuralNet();
  if (net_obj == NULL) {
    return NULL;
  }
      // load the net
  if (!net_obj->ReadBinary(ib)) {
    delete net_obj;
    net_obj = NULL;
  }
  return net_obj;
}

// Compute the output of a specific output node.
// This function is useful for application that are interested in a single
// output of the net and do not want to waste time on the rest
// This is the fast-read-only version of this function
template <typename Type> bool NeuralNet::FastGetNetOutput(const Type *inputs,
                                                          int output_id,
                                                          Type *output) {
  // feed inputs in and offset them by the pre-computed bias
  int node_idx = 0;
  Node *node = &fast_nodes_[0];
  for (node_idx = 0; node_idx < in_cnt_; node_idx++, node++) {
    node->out = inputs[node_idx] - node->bias;
  }

  // compute nodes' activations and outputs for hidden nodes if any
  int hidden_node_cnt = neuron_cnt_ - out_cnt_;
  for (;node_idx < hidden_node_cnt; node_idx++, node++) {
    double activation = -node->bias;
    for (int fan_in_idx = 0; fan_in_idx < node->fan_in_cnt; fan_in_idx++) {
      activation += (node->inputs[fan_in_idx].input_weight *
                     node->inputs[fan_in_idx].input_node->out);
    }
    node->out = Neuron::Sigmoid(activation);
  }

  // compute the output of the required output node
  node += output_id;
  double activation = -node->bias;
  for (int fan_in_idx = 0; fan_in_idx < node->fan_in_cnt; fan_in_idx++) {
    activation += (node->inputs[fan_in_idx].input_weight *
                   node->inputs[fan_in_idx].input_node->out);
  }
  (*output) = Neuron::Sigmoid(activation);
  return true;
}

// Performs a feedforward for general nets. Used mainly in training mode
// Templatized for float and double Types
template <typename Type> bool NeuralNet::GetNetOutput(const Type *inputs,
                                                      int output_id,
                                                      Type *output) {
  // validate output id
  if (output_id < 0 || output_id >= out_cnt_) {
    return false;
  }

  // call the fast version in case of readonly nets
  if (read_only_) {
    return FastGetNetOutput(inputs, output_id, output);
  }

  // For the slow version, we'll just call FeedForward and return the
  // appropriate output
  vector<Type> outputs(out_cnt_);
  if (!FeedForward(inputs, &outputs[0])) {
    return false;
  }
  (*output) = outputs[output_id];

  return true;
}

// Instantiate all supported templates now that the functions have been defined.
template bool NeuralNet::FeedForward(const float *inputs, float *outputs);
template bool NeuralNet::FeedForward(const double *inputs, double *outputs);
template bool NeuralNet::FastFeedForward(const float *inputs, float *outputs);
template bool NeuralNet::FastFeedForward(const double *inputs,
                                         double *outputs);
template bool NeuralNet::GetNetOutput(const float *inputs, int output_id,
                                      float *output);
template bool NeuralNet::GetNetOutput(const double *inputs, int output_id,
                                      double *output);
template bool NeuralNet::FastGetNetOutput(const float *inputs, int output_id,
                                          float *output);
template bool NeuralNet::FastGetNetOutput(const double *inputs, int output_id,
                                          double *output);
template bool NeuralNet::ReadBinary(InputFileBuffer *input_buffer);

}
