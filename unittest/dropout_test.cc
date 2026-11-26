///////////////////////////////////////////////////////////////////////
// File:        dropout_test.cc
// Description: Unit tests for the placeholder of the dropout layer:
//              the pass-through in float and integer mode, the
//              serialization, and the rejection of "Do" in a network
//              specification.
//
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

#include "include_gunit.h"

#include "dropout.h"
#include "network.h"
#include "networkbuilder.h" // for NetworkBuilder (spec parsing)
#include "networkio.h"
#include "networkscratch.h"
#include "serialis.h" // for TFile
#include "stridemap.h"

#include <algorithm> // for std::copy
#include <cstdint>   // for uint32_t
#include <stdexcept> // for std::invalid_argument
#include <vector>

namespace tesseract {
namespace {

// A NetworkIO with `width` timesteps and `ni` float features. Each element
// gets a different value, so a wrong copy is detected.
NetworkIO MakeFloatInput(int width, int ni) {
  StrideMap stride_map;
  stride_map.SetStride({{1, width}});
  NetworkIO input;
  input.ResizeToMap(false, stride_map, ni);
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      input.f(t)[i] = 0.1f * (t + 1) - 0.05f * i;
    }
  }
  return input;
}

// Serializes the layer and returns the data.
std::vector<char> SerializeLayer(const Dropout &net) {
  std::vector<char> data;
  TFile fp;
  fp.OpenWrite(&data);
  EXPECT_TRUE(net.Serialize(&fp));
  return data;
}

// Replaces the bytes at the given offset by the serialized value.
template <typename T>
void OverwriteValue(std::vector<char> &data, size_t offset, const T &value) {
  std::vector<char> bytes;
  TFile fp;
  fp.OpenWrite(&bytes);
  ASSERT_TRUE(fp.Serialize(&value));
  ASSERT_LE(offset + bytes.size(), data.size());
  std::copy(bytes.begin(), bytes.end(), data.begin() + offset);
}

// The rate must lie in [0, 1).
TEST(DropoutLayerTest, RejectsOutOfRangeRate) {
  EXPECT_THROW(Dropout("Dropout", 4, 1.0f), std::invalid_argument);
  EXPECT_THROW(Dropout("Dropout", 4, 1.5f), std::invalid_argument);
  EXPECT_THROW(Dropout("Dropout", 4, -0.1f), std::invalid_argument);
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.0f));
}

// The dimension must be 0, 1 or 2.
TEST(DropoutLayerTest, RejectsOutOfRangeDim) {
  EXPECT_THROW(Dropout("Dropout", 4, 0.5f, 3), std::invalid_argument);
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 0));
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 1));
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 2));
}

// A dropout layer has no weights, and it has as many outputs as inputs.
TEST(DropoutLayerTest, ShapeAndWeights) {
  Dropout net("Dropout", 7, 0.5f);
  EXPECT_EQ(7, net.NumInputs());
  EXPECT_EQ(7, net.NumOutputs());
  EXPECT_EQ(NT_DROPOUT, net.type());
  EXPECT_EQ(0, net.num_weights());
}

// spec() is used for logging and for the description of a network.
TEST(DropoutLayerTest, Spec) {
  EXPECT_EQ("Do0.5", Dropout("Dropout", 4, 0.5f).spec());
  EXPECT_EQ("Do0.25", Dropout("Dropout", 4, 0.25f).spec());
  EXPECT_EQ("Do0.5,1", Dropout("Dropout", 4, 0.5f, 1).spec());
  EXPECT_EQ("Do0.5,2", Dropout("Dropout", 4, 0.5f, 2).spec());
}

// The layer passes its input through unchanged, whether training is enabled
// for the layer or not.
TEST(DropoutLayerTest, ForwardIdentity) {
  const int width = 5;
  const int ni = 3;
  for (auto state : {TS_ENABLED, TS_DISABLED}) {
    Dropout net("Dropout", ni, 0.5f);
    net.SetEnableTraining(state);
    NetworkIO input = MakeFloatInput(width, ni);
    NetworkScratch scratch;
    NetworkIO output;
    net.Forward(false, input, nullptr, &scratch, &output);
    ASSERT_EQ(width, output.Width());
    ASSERT_EQ(ni, output.NumFeatures());
    for (int t = 0; t < width; ++t) {
      for (int i = 0; i < ni; ++i) {
        EXPECT_FLOAT_EQ(input.f(t)[i], output.f(t)[i]) << "t=" << t << " i=" << i;
      }
    }
  }
}

// A network which was converted to integer mode also gets its input passed
// through unchanged.
TEST(DropoutLayerTest, ForwardIntModeIdentity) {
  const int width = 4;
  const int ni = 2;
  Dropout net("Dropout", ni, 0.5f);
  net.SetEnableTraining(TS_DISABLED);

  StrideMap stride_map;
  stride_map.SetStride({{1, width}});
  NetworkIO input;
  input.ResizeToMap(true, stride_map, ni);
  // WriteTimeStepPart expects values in [-1, 1] and quantizes them to int8.
  const float kValues[2] = {0.5f, -0.5f};
  TFloat value;
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      value = kValues[i];
      input.WriteTimeStepPart(t, i, 1, &value);
    }
  }
  NetworkScratch scratch;
  NetworkIO output;
  net.Forward(false, input, nullptr, &scratch, &output);
  ASSERT_TRUE(output.int_mode());
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      EXPECT_EQ(input.i(t)[i], output.i(t)[i]) << "t=" << t << " i=" << i;
    }
  }
}

// The backward pass also passes the deltas through unchanged.
TEST(DropoutLayerTest, BackwardIdentity) {
  const int width = 4;
  const int ni = 2;
  Dropout net("Dropout", ni, 0.5f);
  NetworkIO fwd = MakeFloatInput(width, ni);
  NetworkScratch scratch;
  NetworkIO back;
  net.Backward(false, fwd, &scratch, &back);
  ASSERT_EQ(width, back.Width());
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      EXPECT_FLOAT_EQ(fwd.f(t)[i], back.f(t)[i]) << "t=" << t << " i=" << i;
    }
  }
}

// Serializing a layer and reading it back must reproduce the layer type, the
// rate and the dimension.
TEST(DropoutLayerTest, SerializeRoundTrip) {
  Dropout net("Dropout", 6, 0.4f, 2);
  std::vector<char> data = SerializeLayer(net);
  TFile fp;
  ASSERT_TRUE(fp.Open(data.data(), data.size()));
  Network *net2 = Network::CreateFromFile(&fp);
  ASSERT_NE(net2, nullptr);
  ASSERT_EQ(NT_DROPOUT, net2->type());
  Dropout *d2 = dynamic_cast<Dropout *>(net2);
  ASSERT_NE(d2, nullptr);
  EXPECT_EQ(6, d2->NumInputs());
  EXPECT_EQ(6, d2->NumOutputs());
  EXPECT_EQ("Do0.4,2", d2->spec());
  delete net2;
}

// The serialized layer ends with the rate (float) and the dimension (32 bit
// unsigned integer). This is the format which other versions write, so it
// must not change.
TEST(DropoutLayerTest, SerializedFormat) {
  Dropout net("Dropout", 6, 0.25f, 1);
  std::vector<char> data = SerializeLayer(net);
  const size_t tail_size = sizeof(float) + sizeof(uint32_t);
  ASSERT_GE(data.size(), tail_size);
  float rate = 0;
  uint32_t dim = 0;
  TFile fp;
  ASSERT_TRUE(fp.Open(&data[data.size() - tail_size], tail_size));
  ASSERT_TRUE(fp.DeSerialize(&rate));
  ASSERT_TRUE(fp.DeSerialize(&dim));
  EXPECT_FLOAT_EQ(0.25f, rate);
  EXPECT_EQ(1u, dim);
}

// A model file with an invalid rate or dimension is rejected.
TEST(DropoutLayerTest, DeSerializeRejectsInvalidValues) {
  Dropout net("Dropout", 6, 0.25f, 1);
  const std::vector<char> good = SerializeLayer(net);
  const size_t rate_offset = good.size() - sizeof(float) - sizeof(uint32_t);
  const size_t dim_offset = good.size() - sizeof(uint32_t);
  for (float bad_rate : {1.0f, 2.5f, -0.5f}) {
    std::vector<char> data = good;
    OverwriteValue(data, rate_offset, bad_rate);
    TFile fp;
    ASSERT_TRUE(fp.Open(data.data(), data.size()));
    EXPECT_EQ(nullptr, Network::CreateFromFile(&fp)) << "rate=" << bad_rate;
  }
  {
    std::vector<char> data = good;
    const uint32_t bad_dim = 3;
    OverwriteValue(data, dim_offset, bad_dim);
    TFile fp;
    ASSERT_TRUE(fp.Open(data.data(), data.size()));
    EXPECT_EQ(nullptr, Network::CreateFromFile(&fp));
  }
  {
    // Truncated data.
    TFile fp;
    ASSERT_TRUE(fp.Open(good.data(), good.size() - 1));
    EXPECT_EQ(nullptr, Network::CreateFromFile(&fp));
  }
}

// This version cannot train with dropout, so "Do" in a network specification
// is rejected.
TEST(DropoutSpecTest, RejectedInSpec) {
  for (const char *spec : {"Do0.25 ", "Do0.5,1 ", "Do0.5,2 ", "Do "}) {
    StaticShape shape;
    shape.SetShape(1, 1, 1, 4);
    const char *s = spec;
    Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
    EXPECT_EQ(nullptr, net) << spec;
    delete net;
  }
}

} // namespace
} // namespace tesseract
