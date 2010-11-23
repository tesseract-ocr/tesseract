// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// neural_net.h: Declarations of a class for an object that
// represents an arbitrary network of neurons
//

#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <string>
#include <vector>
#include "neuron.h"
#include "input_file_buffer.h"

namespace tesseract {

// Minimum input range below which we set the input weight to zero
static const float kMinInputRange = 1e-6f;

class NeuralNet {
  public:
    NeuralNet();
    virtual ~NeuralNet();
    // create a net object from a file. Uses stdio
    static NeuralNet *FromFile(const string file_name);
    // create a net object from an input buffer
    static NeuralNet *FromInputBuffer(InputFileBuffer *ib);
    // Different flavors of feed forward function
    template <typename Type> bool FeedForward(const Type *inputs,
                                              Type *outputs);
    // Compute the output of a specific output node.
    // This function is useful for application that are interested in a single
    // output of the net and do not want to waste time on the rest
    template <typename Type> bool GetNetOutput(const Type *inputs,
                                               int output_id,
                                               Type *output);
    // Accessor functions
    int in_cnt() const { return in_cnt_; }
    int out_cnt() const { return out_cnt_; }

  protected:
    struct Node;
    // A node-weight pair
    struct WeightedNode {
      Node *input_node;
      float input_weight;
    };
    // node struct used for fast feedforward in
    // Read only nets
    struct Node {
      float out;
      float bias;
      int fan_in_cnt;
      WeightedNode *inputs;
    };
    // Read-Only flag (no training: On by default)
    // will presumeably be set to false by
    // the inherting TrainableNeuralNet class
    bool read_only_;
    // input count
    int in_cnt_;
    // output count
    int out_cnt_;
    // Total neuron count (including inputs)
    int neuron_cnt_;
    // count of unique weights
    int  wts_cnt_;
    // Neuron vector
    Neuron *neurons_;
    // size of allocated weight chunk (in weights)
    // This is basically the size of the biggest network
    // that I have trained. However, the class will allow
    // a bigger sized net if desired
    static const int kWgtChunkSize = 0x10000;
    // Magic number expected at the beginning of the NN
    // binary file
    static const unsigned int kNetSignature = 0xFEFEABD0;
    // count of allocated wgts in the last chunk
    int alloc_wgt_cnt_;
    // vector of weights buffers
    vector<vector<float> *>wts_vec_;
    // Is the net an auto-encoder type
    bool auto_encoder_;
    // vector of input max values
    vector<float> inputs_max_;
    // vector of input min values
    vector<float> inputs_min_;
    // vector of input mean values
    vector<float> inputs_mean_;
    // vector of input standard deviation values
    vector<float> inputs_std_dev_;
    // vector of input offsets used by fast read-only
    // feedforward function
    vector<Node> fast_nodes_;
    // Network Initialization function
    void Init();
    // Clears all neurons
    void Clear() {
      for (int node = 0; node < neuron_cnt_; node++) {
        neurons_[node].Clear();
      }
    }
    // Reads the net from an input buffer
    template<class ReadBuffType> bool ReadBinary(ReadBuffType *input_buff) {
      // Init vars
      Init();
      // is this an autoencoder
      unsigned int read_val;
      unsigned int auto_encode;
      // read and verify signature
      if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
        return false;
      }
      if (read_val != kNetSignature) {
        return false;
      }
      if (input_buff->Read(&auto_encode, sizeof(auto_encode)) !=
          sizeof(auto_encode)) {
        return false;
      }
      auto_encoder_ = auto_encode;
      // read and validate total # of nodes
      if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
        return false;
      }
      neuron_cnt_ = read_val;
      if (neuron_cnt_ <= 0) {
        return false;
      }
      // set the size of the neurons vector
      neurons_ = new Neuron[neuron_cnt_];
      if (neurons_ == NULL) {
        return false;
      }
      // read & validate inputs
      if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
        return false;
      }
      in_cnt_ = read_val;
      if (in_cnt_ <= 0) {
        return false;
      }
      // read outputs
      if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
        return false;
      }
      out_cnt_ = read_val;
      if (out_cnt_ <= 0) {
        return false;
      }
      // set neuron ids and types
      for (int idx = 0; idx < neuron_cnt_; idx++) {
        neurons_[idx].set_id(idx);
        // input type
        if (idx < in_cnt_) {
          neurons_[idx].set_node_type(Neuron::Input);
        } else if (idx >= (neuron_cnt_ - out_cnt_)) {
          neurons_[idx].set_node_type(Neuron::Output);
        } else {
          neurons_[idx].set_node_type(Neuron::Hidden);
        }
      }
      // read the connections
      for (int node_idx = 0; node_idx < neuron_cnt_; node_idx++) {
        // read fanout
        if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
          return false;
        }
        // read the neuron's info
        int fan_out_cnt = read_val;
        for (int fan_out_idx = 0; fan_out_idx < fan_out_cnt; fan_out_idx++) {
          // read the neuron id
          if (input_buff->Read(&read_val, sizeof(read_val)) != sizeof(read_val)) {
            return false;
          }
          // create the connection
          if (!SetConnection(node_idx, read_val)) {
            return false;
          }
        }
      }
      // read all the neurons' fan-in connections
      for (int node_idx = 0; node_idx < neuron_cnt_; node_idx++) {
        // read
        if (!neurons_[node_idx].ReadBinary(input_buff)) {
          return false;
        }
      }
      // size input stats vector to expected input size
      inputs_mean_.resize(in_cnt_);
      inputs_std_dev_.resize(in_cnt_);
      inputs_min_.resize(in_cnt_);
      inputs_max_.resize(in_cnt_);
      // read stats
      if (input_buff->Read(&(inputs_mean_.front()),
          sizeof(inputs_mean_[0]) * in_cnt_) !=
          sizeof(inputs_mean_[0]) * in_cnt_) {
        return false;
      }
      if (input_buff->Read(&(inputs_std_dev_.front()),
          sizeof(inputs_std_dev_[0]) * in_cnt_) !=
          sizeof(inputs_std_dev_[0]) * in_cnt_) {
        return false;
      }
      if (input_buff->Read(&(inputs_min_.front()),
          sizeof(inputs_min_[0]) * in_cnt_) !=
          sizeof(inputs_min_[0]) * in_cnt_) {
        return false;
      }
      if (input_buff->Read(&(inputs_max_.front()),
          sizeof(inputs_max_[0]) * in_cnt_) !=
          sizeof(inputs_max_[0]) * in_cnt_) {
        return false;
      }
      // create a readonly version for fast feedforward
      if (read_only_) {
        return CreateFastNet();
      }
      return true;
    }

    // creates a connection between two nodes
    bool SetConnection(int from, int to);
    // Create a read only version of the net that
    // has faster feedforward performance
    bool CreateFastNet();
    // internal function to allocate a new set of weights
    // Centralized weight allocation attempts to increase
    // weights locality of reference making it more cache friendly
    float *AllocWgt(int wgt_cnt);
    // different flavors read-only feedforward function
    template <typename Type> bool FastFeedForward(const Type *inputs,
                                                  Type *outputs);
    // Compute the output of a specific output node.
    // This function is useful for application that are interested in a single
    // output of the net and do not want to waste time on the rest
    // This is the fast-read-only version of this function
    template <typename Type> bool FastGetNetOutput(const Type *inputs,
                                                   int output_id,
                                                   Type *output);
};
}

#endif  // NEURAL_NET_H__
