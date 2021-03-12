///////////////////////////////////////////////////////////////////////
// File:        networkbuilder.h
// Description: Class to parse the network description language and
//              build a corresponding network.
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

#ifndef TESSERACT_LSTM_NETWORKBUILDER_H_
#define TESSERACT_LSTM_NETWORKBUILDER_H_

#include "export.h"
#include "static_shape.h"
#include "stridemap.h"

class UNICHARSET;

namespace tesseract {

class Input;
class Network;
class Parallel;
class TRand;

class TESS_COMMON_TRAINING_API NetworkBuilder {
public:
  explicit NetworkBuilder(int num_softmax_outputs) : num_softmax_outputs_(num_softmax_outputs) {}

  // Builds a network with a network_spec in the network description
  // language, to recognize a character set of num_outputs size.
  // If append_index is non-negative, then *network must be non-null and the
  // given network_spec will be appended to *network AFTER append_index, with
  // the top of the input *network discarded.
  // Note that network_spec is call by value to allow a non-const char* pointer
  // into the string for BuildFromString.
  // net_flags control network behavior according to the NetworkFlags enum.
  // The resulting network is returned via **network.
  // Returns false if something failed.
  static bool InitNetwork(int num_outputs, const char *network_spec, int append_index,
                          int net_flags, float weight_range, TRand *randomizer, Network **network);

  // Parses the given string and returns a network according to the following
  // language:
  //  ============ Syntax of description below: ============
  // <d> represents a number.
  // <net> represents any single network element, including (recursively) a
  //   [...] series or (...) parallel construct.
  // (s|t|r|l|m) (regex notation) represents a single required letter.
  // NOTE THAT THROUGHOUT, x and y are REVERSED from conventional mathematics,
  // to use the same convention as Tensor Flow. The reason TF adopts this
  // convention is to eliminate the need to transpose images on input, since
  // adjacent memory locations in images increase x and then y, while adjacent
  // memory locations in tensors in TF, and NetworkIO in tesseract increase the
  // rightmost index first, then the next-left and so-on, like C arrays.
  // ============ INPUTS ============
  // <b>,<h>,<w>,<d> A batch of b images with height h, width w, and depth d.
  //   b, h and/or w may be zero, to indicate variable size. Some network layer
  //   (summarizing LSTM) must be used to make a variable h known.
  //   d may be 1 for greyscale, 3 for color.
  // NOTE that throughout the constructed network, the inputs/outputs are all of
  // the same [batch,height,width,depth] dimensions, even if a different size.
  // ============ PLUMBING ============
  // [...] Execute ... networks in series (layers).
  // (...) Execute ... networks in parallel, with their output depths added.
  // R<d><net> Execute d replicas of net in parallel, with their output depths
  //   added.
  // Rx<net> Execute <net> with x-dimension reversal.
  // Ry<net> Execute <net> with y-dimension reversal.
  // S<y>,<x> Rescale 2-D input by shrink factor x,y, rearranging the data by
  //   increasing the depth of the input by factor xy.
  // Mp<y>,<x> Maxpool the input, reducing the size by an (x,y) rectangle.
  // ============ FUNCTIONAL UNITS ============
  // C(s|t|r|l|m)<y>,<x>,<d> Convolves using a (x,y) window, with no shrinkage,
  //   random infill, producing d outputs, then applies a non-linearity:
  //   s: Sigmoid, t: Tanh, r: Relu, l: Linear, m: Softmax.
  // F(s|t|r|l|m)<d> Truly fully-connected with s|t|r|l|m non-linearity and d
  //   outputs. Connects to every x,y,depth position of the input, reducing
  //   height, width to 1, producing a single <d> vector as the output.
  //   Input height and width must be constant.
  //   For a sliding-window linear or non-linear map that connects just to the
  //   input depth, and leaves the input image size as-is, use a 1x1 convolution
  //   eg. Cr1,1,64 instead of Fr64.
  // L(f|r|b)(x|y)[s]<n> LSTM cell with n states/outputs.
  //   The LSTM must have one of:
  //    f runs the LSTM forward only.
  //    r runs the LSTM reversed only.
  //    b runs the LSTM bidirectionally.
  //   It will operate on either the x- or y-dimension, treating the other
  //     dimension independently (as if part of the batch).
  //   s (optional) summarizes the output in the requested dimension,
  //     outputting only the final step, collapsing the dimension to a
  //     single element.
  // LS<n> Forward-only LSTM cell in the x-direction, with built-in Softmax.
  // LE<n> Forward-only LSTM cell in the x-direction, with built-in softmax,
  //       with binary Encoding.
  // L2xy<n> Full 2-d LSTM operating in quad-directions (bidi in x and y) and
  //   all the output depths added.
  // ============ OUTPUTS ============
  // The network description must finish with an output specification:
  // O(2|1|0)(l|s|c)<n> output layer with n classes
  //  2 (heatmap) Output is a 2-d vector map of the input (possibly at
  //    different scale).
  //  1 (sequence) Output is a 1-d sequence of vector values.
  //  0 (category) Output is a 0-d single vector value.
  //  l uses a logistic non-linearity on the output, allowing multiple
  //    hot elements in any output vector value.
  //  s uses a softmax non-linearity, with one-hot output in each value.
  //  c uses a softmax with CTC. Can only be used with s (sequence).
  //  NOTE1: Only O1s and O1c are currently supported.
  //  NOTE2: n is totally ignored, and for compatibility purposes only. The
  //         output number of classes is obtained automatically from the
  //         unicharset.
  Network *BuildFromString(const StaticShape &input_shape, const char **str);

private:
  // Parses an input specification and returns the result, which may include a
  // series.
  Network *ParseInput(const char **str);
  // Parses a sequential series of networks, defined by [<net><net>...].
  Network *ParseSeries(const StaticShape &input_shape, Input *input_layer, const char **str);
  // Parses a parallel set of networks, defined by (<net><net>...).
  Network *ParseParallel(const StaticShape &input_shape, const char **str);
  // Parses a network that begins with 'R'.
  Network *ParseR(const StaticShape &input_shape, const char **str);
  // Parses a network that begins with 'S'.
  Network *ParseS(const StaticShape &input_shape, const char **str);
  // Parses a network that begins with 'C'.
  Network *ParseC(const StaticShape &input_shape, const char **str);
  // Parses a network that begins with 'M'.
  Network *ParseM(const StaticShape &input_shape, const char **str);
  // Parses an LSTM network, either individual, bi- or quad-directional.
  Network *ParseLSTM(const StaticShape &input_shape, const char **str);
  // Builds a set of 4 lstms with t and y reversal, running in true parallel.
  static Network *BuildLSTMXYQuad(int num_inputs, int num_states);
  // Parses a Fully connected network.
  Network *ParseFullyConnected(const StaticShape &input_shape, const char **str);
  // Parses an Output spec.
  Network *ParseOutput(const StaticShape &input_shape, const char **str);

private:
  int num_softmax_outputs_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_NETWORKBUILDER_H_
