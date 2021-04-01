///////////////////////////////////////////////////////////////////////
// File:        input.cpp
// Description: Input layer class for neural network implementations.
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

#include "input.h"

#include <allheaders.h>
#include "imagedata.h"
#include "pageres.h"
#include "scrollview.h"

namespace tesseract {

// Max height for variable height inputs before scaling anyway.
const int kMaxInputHeight = 48;

Input::Input(const std::string &name, int ni, int no)
    : Network(NT_INPUT, name, ni, no), cached_x_scale_(1) {}
Input::Input(const std::string &name, const StaticShape &shape)
    : Network(NT_INPUT, name, shape.height(), shape.depth()), shape_(shape), cached_x_scale_(1) {
  if (shape.height() == 1) {
    ni_ = shape.depth();
  }
}

// Writes to the given file. Returns false in case of error.
bool Input::Serialize(TFile *fp) const {
  return Network::Serialize(fp) && shape_.Serialize(fp);
}

// Reads from the given file. Returns false in case of error.
bool Input::DeSerialize(TFile *fp) {
  return shape_.DeSerialize(fp);
}

// Returns an integer reduction factor that the network applies to the
// time sequence. Assumes that any 2-d is already eliminated. Used for
// scaling bounding boxes of truth data.
int Input::XScaleFactor() const {
  return 1;
}

// Provides the (minimum) x scale factor to the network (of interest only to
// input units) so they can determine how to scale bounding boxes.
void Input::CacheXScaleFactor(int factor) {
  cached_x_scale_ = factor;
}

// Runs forward propagation of activations on the input line.
// See Network for a detailed discussion of the arguments.
void Input::Forward(bool debug, const NetworkIO &input, const TransposedArray *input_transpose,
                    NetworkScratch *scratch, NetworkIO *output) {
  *output = input;
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool Input::Backward(bool debug, const NetworkIO &fwd_deltas, NetworkScratch *scratch,
                     NetworkIO *back_deltas) {
  tprintf("Input::Backward should not be called!!\n");
  return false;
}

// Creates and returns a Pix of appropriate size for the network from the
// image_data. If non-null, *image_scale returns the image scale factor used.
// Returns nullptr on error.
/* static */
Image Input::PrepareLSTMInputs(const ImageData &image_data, const Network *network, int min_width,
                              TRand *randomizer, float *image_scale) {
  // Note that NumInputs() is defined as input image height.
  int target_height = network->NumInputs();
  int width, height;
  Image pix =
      image_data.PreScale(target_height, kMaxInputHeight, image_scale, &width, &height, nullptr);
  if (pix == nullptr) {
    tprintf("Bad pix from ImageData!\n");
    return nullptr;
  }
  if (width < min_width || height < min_width) {
    tprintf("Image too small to scale!! (%dx%d vs min width of %d)\n", width, height, min_width);
    pix.destroy();
    return nullptr;
  }
  return pix;
}

// Converts the given pix to a NetworkIO of height and depth appropriate to the
// given StaticShape:
// If depth == 3, convert to 24 bit color, otherwise normalized grey.
// Scale to target height, if the shape's height is > 1, or its depth if the
// height == 1. If height == 0 then no scaling.
// NOTE: It isn't safe for multiple threads to call this on the same pix.
/* static */
void Input::PreparePixInput(const StaticShape &shape, const Image pix, TRand *randomizer,
                            NetworkIO *input) {
  bool color = shape.depth() == 3;
  Image var_pix = pix;
  int depth = pixGetDepth(var_pix);
  Image normed_pix = nullptr;
  // On input to BaseAPI, an image is forced to be 1, 8 or 24 bit, without
  // colormap, so we just have to deal with depth conversion here.
  if (color) {
    // Force RGB.
    if (depth == 32) {
      normed_pix = var_pix.clone();
    } else {
      normed_pix = pixConvertTo32(var_pix);
    }
  } else {
    // Convert non-8-bit images to 8 bit.
    if (depth == 8) {
      normed_pix = var_pix.clone();
    } else {
      normed_pix = pixConvertTo8(var_pix, false);
    }
  }
  int height = pixGetHeight(normed_pix);
  int target_height = shape.height();
  if (target_height == 1) {
    target_height = shape.depth();
  }
  if (target_height != 0 && target_height != height) {
    // Get the scaled image.
    float im_factor = static_cast<float>(target_height) / height;
    Image scaled_pix = pixScale(normed_pix, im_factor, im_factor);
    normed_pix.destroy();
    normed_pix = scaled_pix;
  }
  input->FromPix(shape, normed_pix, randomizer);
  normed_pix.destroy();
}

} // namespace tesseract.
