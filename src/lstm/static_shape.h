///////////////////////////////////////////////////////////////////////
// File:        static_shape.h
// Description: Defines the size of the 4-d tensor input/output from a network.
// Author:      Ray Smith
// Created:     Fri Oct 14 09:07:31 PST 2016
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

#ifndef TESSERACT_LSTM_STATIC_SHAPE_H_
#define TESSERACT_LSTM_STATIC_SHAPE_H_

#include <tesseract/serialis.h>   // for TFile
#include "tprintf.h"    // for tprintf

namespace tesseract {

// Enum describing the loss function to apply during training and/or the
// decoding method to apply at runtime.
enum LossType {
  LT_NONE,      // Undefined.
  LT_CTC,       // Softmax with standard CTC for training/decoding.
  LT_SOFTMAX,   // Outputs sum to 1 in fixed positions.
  LT_LOGISTIC,  // Logistic outputs with independent values.
};

// Simple class to hold the tensor shape that is known at network build time
// and the LossType of the loss function.
class StaticShape {
 public:
  StaticShape()
      : batch_(0), height_(0), width_(0), depth_(0), loss_type_(LT_NONE) {}
  int batch() const { return batch_; }
  void set_batch(int value) { batch_ = value; }
  int height() const { return height_; }
  void set_height(int value) { height_ = value; }
  int width() const { return width_; }
  void set_width(int value) { width_ = value; }
  int depth() const { return depth_; }
  void set_depth(int value) { depth_ = value; }
  LossType loss_type() const { return loss_type_; }
  void set_loss_type(LossType value) { loss_type_ = value; }
  void SetShape(int batch, int height, int width, int depth) {
    batch_ = batch;
    height_ = height;
    width_ = width;
    depth_ = depth;
  }

  void Print() const {
    tprintf("Batch=%d, Height=%d, Width=%d, Depth=%d, loss=%d\n", batch_,
            height_, width_, depth_, loss_type_);
  }

  bool DeSerialize(TFile *fp) {
    int32_t tmp = LT_NONE;
    bool result =
      fp->DeSerialize(&batch_) &&
      fp->DeSerialize(&height_) &&
      fp->DeSerialize(&width_) &&
      fp->DeSerialize(&depth_) &&
      fp->DeSerialize(&tmp);
    loss_type_ = static_cast<LossType>(tmp);
    return result;
  }

  bool Serialize(TFile *fp) const {
    int32_t tmp = loss_type_;
    return
      fp->Serialize(&batch_) &&
      fp->Serialize(&height_) &&
      fp->Serialize(&width_) &&
      fp->Serialize(&depth_) &&
      fp->Serialize(&tmp);
  }

 private:
  // Size of the 4-D tensor input/output to a network. A value of zero is
  // allowed for all except depth_ and means to be determined at runtime, and
  // regarded as variable.
  // Number of elements in a batch, or number of frames in a video stream.
  int32_t batch_;
  // Height of the image.
  int32_t height_;
  // Width of the image.
  int32_t width_;
  // Depth of the image. (Number of "nodes").
  int32_t depth_;
  // How to train/interpret the output.
  LossType loss_type_;
};

}  // namespace tesseract

#endif  // TESSERACT_LSTM_STATIC_SHAPE_H_
