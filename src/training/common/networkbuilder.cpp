///////////////////////////////////////////////////////////////////////
// File:        networkbuilder.cpp
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

#include "networkbuilder.h"

#include "convolve.h"
#include "fullyconnected.h"
#include "input.h"
#include "lstm.h"
#include "maxpool.h"
#include "network.h"
#include "parallel.h"
#include "reconfig.h"
#include "reversed.h"
#include "series.h"
#include "unicharset.h"

namespace tesseract {

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
bool NetworkBuilder::InitNetwork(int num_outputs, const char *network_spec, int append_index,
                                 int net_flags, float weight_range, TRand *randomizer,
                                 Network **network) {
  NetworkBuilder builder(num_outputs);
  Series *bottom_series = nullptr;
  StaticShape input_shape;
  if (append_index >= 0) {
    // Split the current network after the given append_index.
    ASSERT_HOST(*network != nullptr && (*network)->type() == NT_SERIES);
    auto *series = static_cast<Series *>(*network);
    Series *top_series = nullptr;
    series->SplitAt(append_index, &bottom_series, &top_series);
    if (bottom_series == nullptr || top_series == nullptr) {
      tprintf("Yikes! Splitting current network failed!!\n");
      return false;
    }
    input_shape = bottom_series->OutputShape(input_shape);
    delete top_series;
  }
  *network = builder.BuildFromString(input_shape, &network_spec);
  if (*network == nullptr) {
    return false;
  }
  (*network)->SetNetworkFlags(net_flags);
  (*network)->InitWeights(weight_range, randomizer);
  (*network)->SetupNeedsBackprop(false);
  if (bottom_series != nullptr) {
    bottom_series->AppendSeries(*network);
    *network = bottom_series;
  }
  (*network)->CacheXScaleFactor((*network)->XScaleFactor());
  return true;
}

// Helper skips whitespace.
static void SkipWhitespace(const char **str) {
  while (**str == ' ' || **str == '\t' || **str == '\n') {
    ++*str;
  }
}

// Parses the given string and returns a network according to the network
// description language in networkbuilder.h
Network *NetworkBuilder::BuildFromString(const StaticShape &input_shape, const char **str) {
  SkipWhitespace(str);
  char code_ch = **str;
  if (code_ch == '[') {
    return ParseSeries(input_shape, nullptr, str);
  }
  if (input_shape.depth() == 0) {
    // There must be an input at this point.
    return ParseInput(str);
  }
  switch (code_ch) {
    case '(':
      return ParseParallel(input_shape, str);
    case 'R':
      return ParseR(input_shape, str);
    case 'S':
      return ParseS(input_shape, str);
    case 'C':
      return ParseC(input_shape, str);
    case 'M':
      return ParseM(input_shape, str);
    case 'L':
      return ParseLSTM(input_shape, str);
    case 'F':
      return ParseFullyConnected(input_shape, str);
    case 'O':
      return ParseOutput(input_shape, str);
    default:
      tprintf("Invalid network spec:%s\n", *str);
      return nullptr;
  }
  return nullptr;
}

// Parses an input specification and returns the result, which may include a
// series.
Network *NetworkBuilder::ParseInput(const char **str) {
  // There must be an input at this point.
  int length = 0;
  int batch, height, width, depth;
  int num_converted = sscanf(*str, "%d,%d,%d,%d%n", &batch, &height, &width, &depth, &length);
  StaticShape shape;
  shape.SetShape(batch, height, width, depth);
  // num_converted may or may not include the length.
  if (num_converted != 4 && num_converted != 5) {
    tprintf("Must specify an input layer as the first layer, not %s!!\n", *str);
    return nullptr;
  }
  *str += length;
  auto *input = new Input("Input", shape);
  // We want to allow [<input>rest of net... or <input>[rest of net... so we
  // have to check explicitly for '[' here.
  SkipWhitespace(str);
  if (**str == '[') {
    return ParseSeries(shape, input, str);
  }
  return input;
}

// Parses a sequential series of networks, defined by [<net><net>...].
Network *NetworkBuilder::ParseSeries(const StaticShape &input_shape, Input *input_layer,
                                     const char **str) {
  StaticShape shape = input_shape;
  auto *series = new Series("Series");
  ++*str;
  if (input_layer != nullptr) {
    series->AddToStack(input_layer);
    shape = input_layer->OutputShape(shape);
  }
  Network *network = nullptr;
  while (**str != '\0' && **str != ']' && (network = BuildFromString(shape, str)) != nullptr) {
    shape = network->OutputShape(shape);
    series->AddToStack(network);
  }
  if (**str != ']') {
    tprintf("Missing ] at end of [Series]!\n");
    delete series;
    return nullptr;
  }
  ++*str;
  return series;
}

// Parses a parallel set of networks, defined by (<net><net>...).
Network *NetworkBuilder::ParseParallel(const StaticShape &input_shape, const char **str) {
  auto *parallel = new Parallel("Parallel", NT_PARALLEL);
  ++*str;
  Network *network = nullptr;
  while (**str != '\0' && **str != ')' &&
         (network = BuildFromString(input_shape, str)) != nullptr) {
    parallel->AddToStack(network);
  }
  if (**str != ')') {
    tprintf("Missing ) at end of (Parallel)!\n");
    delete parallel;
    return nullptr;
  }
  ++*str;
  return parallel;
}

// Parses a network that begins with 'R'.
Network *NetworkBuilder::ParseR(const StaticShape &input_shape, const char **str) {
  char dir = (*str)[1];
  if (dir == 'x' || dir == 'y') {
    std::string name = "Reverse";
    name += dir;
    *str += 2;
    Network *network = BuildFromString(input_shape, str);
    if (network == nullptr) {
      return nullptr;
    }
    auto *rev = new Reversed(name, dir == 'y' ? NT_YREVERSED : NT_XREVERSED);
    rev->SetNetwork(network);
    return rev;
  }
  char *end;
  int replicas = strtol(*str + 1, &end, 10);
  *str = end;
  if (replicas <= 0) {
    tprintf("Invalid R spec!:%s\n", end);
    return nullptr;
  }
  auto *parallel = new Parallel("Replicated", NT_REPLICATED);
  const char *str_copy = *str;
  for (int i = 0; i < replicas; ++i) {
    str_copy = *str;
    Network *network = BuildFromString(input_shape, &str_copy);
    if (network == nullptr) {
      tprintf("Invalid replicated network!\n");
      delete parallel;
      return nullptr;
    }
    parallel->AddToStack(network);
  }
  *str = str_copy;
  return parallel;
}

// Parses a network that begins with 'S'.
Network *NetworkBuilder::ParseS(const StaticShape &input_shape, const char **str) {
  char *end;
  int y = strtol(*str + 1, &end, 10);
  *str = end;
  if (**str == ',') {
    int x = strtol(*str + 1, &end, 10);
    *str = end;
    if (y <= 0 || x <= 0) {
      tprintf("Invalid S spec!:%s\n", *str);
      return nullptr;
    }
    return new Reconfig("Reconfig", input_shape.depth(), x, y);
  } else if (**str == '(') {
    // TODO(rays) Add Generic reshape.
    tprintf("Generic reshape not yet implemented!!\n");
    return nullptr;
  }
  tprintf("Invalid S spec!:%s\n", *str);
  return nullptr;
}

// Helper returns the fully-connected type for the character code.
static NetworkType NonLinearity(char func) {
  switch (func) {
    case 's':
      return NT_LOGISTIC;
    case 't':
      return NT_TANH;
    case 'r':
      return NT_RELU;
    case 'l':
      return NT_LINEAR;
    case 'm':
      return NT_SOFTMAX;
    case 'p':
      return NT_POSCLIP;
    case 'n':
      return NT_SYMCLIP;
    default:
      return NT_NONE;
  }
}

// Parses a network that begins with 'C'.
Network *NetworkBuilder::ParseC(const StaticShape &input_shape, const char **str) {
  NetworkType type = NonLinearity((*str)[1]);
  if (type == NT_NONE) {
    tprintf("Invalid nonlinearity on C-spec!: %s\n", *str);
    return nullptr;
  }
  int y = 0, x = 0, d = 0;
  char *end;
  if ((y = strtol(*str + 2, &end, 10)) <= 0 || *end != ',' ||
      (x = strtol(end + 1, &end, 10)) <= 0 || *end != ',' || (d = strtol(end + 1, &end, 10)) <= 0) {
    tprintf("Invalid C spec!:%s\n", end);
    return nullptr;
  }
  *str = end;
  if (x == 1 && y == 1) {
    // No actual convolution. Just a FullyConnected on the current depth, to
    // be slid over all batch,y,x.
    return new FullyConnected("Conv1x1", input_shape.depth(), d, type);
  }
  auto *series = new Series("ConvSeries");
  auto *convolve = new Convolve("Convolve", input_shape.depth(), x / 2, y / 2);
  series->AddToStack(convolve);
  StaticShape fc_input = convolve->OutputShape(input_shape);
  series->AddToStack(new FullyConnected("ConvNL", fc_input.depth(), d, type));
  return series;
}

// Parses a network that begins with 'M'.
Network *NetworkBuilder::ParseM(const StaticShape &input_shape, const char **str) {
  int y = 0, x = 0;
  char *end;
  if ((*str)[1] != 'p' || (y = strtol(*str + 2, &end, 10)) <= 0 || *end != ',' ||
      (x = strtol(end + 1, &end, 10)) <= 0) {
    tprintf("Invalid Mp spec!:%s\n", *str);
    return nullptr;
  }
  *str = end;
  return new Maxpool("Maxpool", input_shape.depth(), x, y);
}

// Parses an LSTM network, either individual, bi- or quad-directional.
Network *NetworkBuilder::ParseLSTM(const StaticShape &input_shape, const char **str) {
  bool two_d = false;
  NetworkType type = NT_LSTM;
  const char *spec_start = *str;
  int chars_consumed = 1;
  int num_outputs = 0;
  char key = (*str)[chars_consumed], dir = 'f', dim = 'x';
  if (key == 'S') {
    type = NT_LSTM_SOFTMAX;
    num_outputs = num_softmax_outputs_;
    ++chars_consumed;
  } else if (key == 'E') {
    type = NT_LSTM_SOFTMAX_ENCODED;
    num_outputs = num_softmax_outputs_;
    ++chars_consumed;
  } else if (key == '2' &&
             (((*str)[2] == 'x' && (*str)[3] == 'y') || ((*str)[2] == 'y' && (*str)[3] == 'x'))) {
    chars_consumed = 4;
    dim = (*str)[3];
    two_d = true;
  } else if (key == 'f' || key == 'r' || key == 'b') {
    dir = key;
    dim = (*str)[2];
    if (dim != 'x' && dim != 'y') {
      tprintf("Invalid dimension (x|y) in L Spec!:%s\n", *str);
      return nullptr;
    }
    chars_consumed = 3;
    if ((*str)[chars_consumed] == 's') {
      ++chars_consumed;
      type = NT_LSTM_SUMMARY;
    }
  } else {
    tprintf("Invalid direction (f|r|b) in L Spec!:%s\n", *str);
    return nullptr;
  }
  char *end;
  int num_states = strtol(*str + chars_consumed, &end, 10);
  if (num_states <= 0) {
    tprintf("Invalid number of states in L Spec!:%s\n", *str);
    return nullptr;
  }
  *str = end;
  Network *lstm = nullptr;
  if (two_d) {
    lstm = BuildLSTMXYQuad(input_shape.depth(), num_states);
  } else {
    if (num_outputs == 0) {
      num_outputs = num_states;
    }
    std::string name(spec_start, *str - spec_start);
    lstm = new LSTM(name, input_shape.depth(), num_states, num_outputs, false, type);
    if (dir != 'f') {
      auto *rev = new Reversed("RevLSTM", NT_XREVERSED);
      rev->SetNetwork(lstm);
      lstm = rev;
    }
    if (dir == 'b') {
      name += "LTR";
      auto *parallel = new Parallel("BidiLSTM", NT_PAR_RL_LSTM);
      parallel->AddToStack(
          new LSTM(name, input_shape.depth(), num_states, num_outputs, false, type));
      parallel->AddToStack(lstm);
      lstm = parallel;
    }
  }
  if (dim == 'y') {
    auto *rev = new Reversed("XYTransLSTM", NT_XYTRANSPOSE);
    rev->SetNetwork(lstm);
    lstm = rev;
  }
  return lstm;
}

// Builds a set of 4 lstms with x and y reversal, running in true parallel.
Network *NetworkBuilder::BuildLSTMXYQuad(int num_inputs, int num_states) {
  auto *parallel = new Parallel("2DLSTMQuad", NT_PAR_2D_LSTM);
  parallel->AddToStack(new LSTM("L2DLTRDown", num_inputs, num_states, num_states, true, NT_LSTM));
  auto *rev = new Reversed("L2DLTRXRev", NT_XREVERSED);
  rev->SetNetwork(new LSTM("L2DRTLDown", num_inputs, num_states, num_states, true, NT_LSTM));
  parallel->AddToStack(rev);
  rev = new Reversed("L2DRTLYRev", NT_YREVERSED);
  rev->SetNetwork(new LSTM("L2DRTLUp", num_inputs, num_states, num_states, true, NT_LSTM));
  auto *rev2 = new Reversed("L2DXRevU", NT_XREVERSED);
  rev2->SetNetwork(rev);
  parallel->AddToStack(rev2);
  rev = new Reversed("L2DXRevY", NT_YREVERSED);
  rev->SetNetwork(new LSTM("L2DLTRDown", num_inputs, num_states, num_states, true, NT_LSTM));
  parallel->AddToStack(rev);
  return parallel;
}

// Helper builds a truly (0-d) fully connected layer of the given type.
static Network *BuildFullyConnected(const StaticShape &input_shape, NetworkType type,
                                    const std::string &name, int depth) {
  if (input_shape.height() == 0 || input_shape.width() == 0) {
    tprintf("Fully connected requires positive height and width, had %d,%d\n", input_shape.height(),
            input_shape.width());
    return nullptr;
  }
  int input_size = input_shape.height() * input_shape.width();
  int input_depth = input_size * input_shape.depth();
  Network *fc = new FullyConnected(name, input_depth, depth, type);
  if (input_size > 1) {
    auto *series = new Series("FCSeries");
    series->AddToStack(
        new Reconfig("FCReconfig", input_shape.depth(), input_shape.width(), input_shape.height()));
    series->AddToStack(fc);
    fc = series;
  }
  return fc;
}

// Parses a Fully connected network.
Network *NetworkBuilder::ParseFullyConnected(const StaticShape &input_shape, const char **str) {
  const char *spec_start = *str;
  NetworkType type = NonLinearity((*str)[1]);
  if (type == NT_NONE) {
    tprintf("Invalid nonlinearity on F-spec!: %s\n", *str);
    return nullptr;
  }
  char *end;
  int depth = strtol(*str + 2, &end, 10);
  if (depth <= 0) {
    tprintf("Invalid F spec!:%s\n", *str);
    return nullptr;
  }
  *str = end;
  std::string name(spec_start, *str - spec_start);
  return BuildFullyConnected(input_shape, type, name, depth);
}

// Parses an Output spec.
Network *NetworkBuilder::ParseOutput(const StaticShape &input_shape, const char **str) {
  char dims_ch = (*str)[1];
  if (dims_ch != '0' && dims_ch != '1' && dims_ch != '2') {
    tprintf("Invalid dims (2|1|0) in output spec!:%s\n", *str);
    return nullptr;
  }
  char type_ch = (*str)[2];
  if (type_ch != 'l' && type_ch != 's' && type_ch != 'c') {
    tprintf("Invalid output type (l|s|c) in output spec!:%s\n", *str);
    return nullptr;
  }
  char *end;
  int depth = strtol(*str + 3, &end, 10);
  if (depth != num_softmax_outputs_) {
    tprintf("Warning: given outputs %d not equal to unicharset of %d.\n", depth,
            num_softmax_outputs_);
    depth = num_softmax_outputs_;
  }
  *str = end;
  NetworkType type = NT_SOFTMAX;
  if (type_ch == 'l') {
    type = NT_LOGISTIC;
  } else if (type_ch == 's') {
    type = NT_SOFTMAX_NO_CTC;
  }
  if (dims_ch == '0') {
    // Same as standard fully connected.
    return BuildFullyConnected(input_shape, type, "Output", depth);
  } else if (dims_ch == '2') {
    // We don't care if x and/or y are variable.
    return new FullyConnected("Output2d", input_shape.depth(), depth, type);
  }
  // For 1-d y has to be fixed, and if not 1, moved to depth.
  if (input_shape.height() == 0) {
    tprintf("Fully connected requires fixed height!\n");
    return nullptr;
  }
  int input_size = input_shape.height();
  int input_depth = input_size * input_shape.depth();
  Network *fc = new FullyConnected("Output", input_depth, depth, type);
  if (input_size > 1) {
    auto *series = new Series("FCSeries");
    series->AddToStack(new Reconfig("FCReconfig", input_shape.depth(), 1, input_shape.height()));
    series->AddToStack(fc);
    fc = series;
  }
  return fc;
}

} // namespace tesseract.
