///////////////////////////////////////////////////////////////////////
// File:        tfnetwork.h
// Description: Encapsulation of an entire tensorflow graph as a
//              Tesseract Network.
// Author:      Ray Smith
//
// (C) Copyright 2016, Google Inc.
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

#ifndef TESSERACT_LSTM_TFNETWORK_H_
#define TESSERACT_LSTM_TFNETWORK_H_

#ifdef INCLUDE_TENSORFLOW

#include <memory>
#include <string>

#include "network.h"
#include "static_shape.h"
#include "tfnetwork.pb.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/public/session.h"

namespace tesseract {

class TFNetwork : public Network {
 public:
  explicit TFNetwork(const STRING& name);
  virtual ~TFNetwork() = default;

  // Returns the required shape input to the network.
  StaticShape InputShape() const override { return input_shape_; }
  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  StaticShape OutputShape(const StaticShape& input_shape) const override {
    return output_shape_;
  }

  STRING spec() const override { return spec_.c_str(); }

  // Deserializes *this from a serialized TFNetwork proto. Returns 0 if failed,
  // otherwise the global step of the serialized graph.
  int InitFromProtoStr(const std::string& proto_str);
  // The number of classes in this network should be equal to those in the
  // recoder_ in LSTMRecognizer.
  int num_classes() const { return output_shape_.depth(); }

  // Writes to the given file. Returns false in case of error.
  // Should be overridden by subclasses, but called by their Serialize.
  bool Serialize(TFile* fp) const override;
  // Reads from the given file. Returns false in case of error.
  // Should be overridden by subclasses, but NOT called by their DeSerialize.
  bool DeSerialize(TFile* fp) override;

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  void Forward(bool debug, const NetworkIO& input,
               const TransposedArray* input_transpose,
               NetworkScratch* scratch, NetworkIO* output) override;

 private:
  // Runs backward propagation of errors on the deltas line.
  // See Network for a detailed discussion of the arguments.
  bool Backward(bool debug, const NetworkIO& fwd_deltas,
                NetworkScratch* scratch,
                NetworkIO* back_deltas) override {
    tprintf("Must override Network::Backward for type %d\n", type_);
    return false;
  }

  void DebugWeights() override {
    tprintf("Must override Network::DebugWeights for type %d\n", type_);
  }

  int InitFromProto();

  // The original network definition for reference.
  std::string spec_;
  // Input tensor parameters.
  StaticShape input_shape_;
  // Output tensor parameters.
  StaticShape output_shape_;
  // The tensor flow graph is contained in here.
  std::unique_ptr<tensorflow::Session> session_;
  // The serialized graph is also contained in here.
  TFNetworkModel model_proto_;
};

}  // namespace tesseract.

#endif  // ifdef INCLUDE_TENSORFLOW

#endif  // TESSERACT_TENSORFLOW_TFNETWORK_H_
