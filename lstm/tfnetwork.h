///////////////////////////////////////////////////////////////////////
// File:        tfnetwork.h
// Description: Encapsulation of an entire tensorflow graph as a
//              Tesseract Network.
// Author:      Ray Smith
// Created:     Fri Feb 26 09:35:29 PST 2016
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
#include "tfnetwork.proto.h"
#include "third_party/tensorflow/core/framework/graph.pb.h"
#include "third_party/tensorflow/core/public/session.h"

namespace tesseract {

class TFNetwork : public Network {
 public:
  explicit TFNetwork(const STRING& name);
  virtual ~TFNetwork();

  // Returns the required shape input to the network.
  virtual StaticShape InputShape() const { return input_shape_; }
  // Returns the shape output from the network given an input shape (which may
  // be partially unknown ie zero).
  virtual StaticShape OutputShape(const StaticShape& input_shape) const {
    return output_shape_;
  }

  virtual STRING spec() const { return spec_.c_str(); }

  // Deserializes *this from a serialized TFNetwork proto. Returns 0 if failed,
  // otherwise the global step of the serialized graph.
  int InitFromProtoStr(const string& proto_str);
  // The number of classes in this network should be equal to those in the
  // recoder_ in LSTMRecognizer.
  int num_classes() const { return output_shape_.depth(); }

  // Writes to the given file. Returns false in case of error.
  // Should be overridden by subclasses, but called by their Serialize.
  virtual bool Serialize(TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  // Should be overridden by subclasses, but NOT called by their DeSerialize.
  virtual bool DeSerialize(bool swap, TFile* fp);

  // Runs forward propagation of activations on the input line.
  // See Network for a detailed discussion of the arguments.
  virtual void Forward(bool debug, const NetworkIO& input,
                       const TransposedArray* input_transpose,
                       NetworkScratch* scratch, NetworkIO* output);

 private:
  int InitFromProto();

  // The original network definition for reference.
  string spec_;
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
