///////////////////////////////////////////////////////////////////////
// File:        tfnetwork.cpp
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
#ifdef INCLUDE_TENSORFLOW

#  include "tfnetwork.h"

#  include <allheaders.h>
#  include "input.h"
#  include "networkscratch.h"

using tensorflow::Status;
using tensorflow::Tensor;
using tensorflow::TensorShape;

namespace tesseract {

TFNetwork::TFNetwork(const std::string &name) : Network(NT_TENSORFLOW, name, 0, 0) {}

int TFNetwork::InitFromProtoStr(const std::string &proto_str) {
  if (!model_proto_.ParseFromString(proto_str))
    return 0;
  return InitFromProto();
}

// Writes to the given file. Returns false in case of error.
// Should be overridden by subclasses, but called by their Serialize.
bool TFNetwork::Serialize(TFile *fp) const {
  if (!Network::Serialize(fp))
    return false;
  std::string proto_str;
  model_proto_.SerializeToString(&proto_str);
  // TODO: optimize and avoid copy from proto_str to data.
  std::vector<char> data(proto_str.size());
  memcpy(&data[0], proto_str.data(), proto_str.size());
  return fp->Serialize(data);
}

// Reads from the given file. Returns false in case of error.
// Should be overridden by subclasses, but NOT called by their DeSerialize.
bool TFNetwork::DeSerialize(TFile *fp) {
  std::vector<char> data;
  if (!fp->DeSerialize(data))
    return false;
  if (!model_proto_.ParseFromArray(&data[0], data.size())) {
    return false;
  }
  return InitFromProto();
}

// Runs forward propagation of activations on the input line.
// See Network for a detailed discussion of the arguments.
void TFNetwork::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                        NetworkScratch *scratch, NetworkIO *output) {
  std::vector<std::pair<std::string, Tensor>> tf_inputs;
  int depth = input_shape_.depth();
  ASSERT_HOST(depth == input.NumFeatures());
  // TODO(rays) Allow batching. For now batch_size = 1.
  const StrideMap &stride_map = input.stride_map();
  // TF requires a tensor of shape float[batch, height, width, depth].
  TensorShape shape{1, stride_map.Size(FD_HEIGHT), stride_map.Size(FD_WIDTH), depth};
  Tensor input_tensor(tensorflow::DT_FLOAT, shape);
  // The flat() member gives a 1d array, with a data() member to get the data.
  auto eigen_tensor = input_tensor.flat<float>();
  memcpy(eigen_tensor.data(), input.f(0), input.Width() * depth * sizeof(input.f(0)[0]));
  // Add the tensor to the vector of inputs.
  tf_inputs.emplace_back(model_proto_.image_input(), input_tensor);

  // Provide tensors giving the width and/or height of the image if they are
  // required. Some tf ops require a separate tensor with knowledge of the
  // size of the input as they cannot obtain it from the input tensor. This is
  // usually true in the case of ops that process a batch of variable-sized
  // objects.
  if (!model_proto_.image_widths().empty()) {
    TensorShape size_shape{1};
    Tensor width_tensor(tensorflow::DT_INT64, size_shape);
    auto eigen_wtensor = width_tensor.flat<tensorflow::int64>();
    *eigen_wtensor.data() = stride_map.Size(FD_WIDTH);
    tf_inputs.emplace_back(model_proto_.image_widths(), width_tensor);
  }
  if (!model_proto_.image_heights().empty()) {
    TensorShape size_shape{1};
    Tensor height_tensor(tensorflow::DT_INT64, size_shape);
    auto eigen_htensor = height_tensor.flat<tensorflow::int64>();
    *eigen_htensor.data() = stride_map.Size(FD_HEIGHT);
    tf_inputs.emplace_back(model_proto_.image_heights(), height_tensor);
  }
  std::vector<std::string> target_layers = {model_proto_.output_layer()};
  std::vector<Tensor> outputs;
  Status s = session_->Run(tf_inputs, target_layers, {}, &outputs);
  if (!s.ok())
    tprintf("session->Run failed:%s\n", s.error_message().c_str());
  ASSERT_HOST(s.ok());
  ASSERT_HOST(outputs.size() == 1);
  const Tensor &output_tensor = outputs[0];
  // Check the dimensions of the output.
  ASSERT_HOST(output_tensor.shape().dims() == 3);
  int output_batch = output_tensor.shape().dim_size(0);
  int output_steps = output_tensor.shape().dim_size(1);
  int output_depth = output_tensor.shape().dim_size(2);
  ASSERT_HOST(output_batch == 1);
  ASSERT_HOST(output_depth == output_shape_.depth());
  output->Resize2d(false, output_steps, output_depth);
  auto eigen_output = output_tensor.flat<float>();
  memcpy(output->f(0), eigen_output.data(), output_steps * output_depth * sizeof(output->f(0)[0]));
}

int TFNetwork::InitFromProto() {
  spec_ = model_proto_.spec();
  input_shape_.SetShape(model_proto_.batch_size(), std::max(0, model_proto_.y_size()),
                        std::max(0, model_proto_.x_size()), model_proto_.depth());
  output_shape_.SetShape(model_proto_.batch_size(), 1, 0, model_proto_.num_classes());
  output_shape_.set_loss_type(model_proto_.using_ctc() ? LT_CTC : LT_SOFTMAX);
  ni_ = input_shape_.height();
  no_ = output_shape_.depth();
  // Initialize the session_ with the graph. Since we can't get the graph
  // back from the session_, we have to keep the proto as well
  tensorflow::SessionOptions options;
  session_.reset(NewSession(options));
  Status s = session_->Create(model_proto_.graph());
  if (s.ok())
    return model_proto_.global_step();
  tprintf("Session_->Create returned '%s'\n", s.error_message().c_str());
  return 0;
}

} // namespace tesseract

#endif // ifdef INCLUDE_TENSORFLOW
