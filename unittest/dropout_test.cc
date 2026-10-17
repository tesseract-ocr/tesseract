///////////////////////////////////////////////////////////////////////
// File:        dropout_test.cc
// Description: Unit tests for the Dropout network layer: the "Do" spec,
//              the inverted-dropout forward/backward passes in all three
//              mask modes, the int-mode recognition pass-through, and
//              serialization.
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
#include "networkio.h"
#include "networkscratch.h"
#include "networkbuilder.h" // for NetworkBuilder (spec parsing)
#include "serialis.h" // for TFile
#include "stridemap.h"

#include <stdexcept> // for std::invalid_argument
#include <vector>

namespace tesseract {
namespace {

// A NetworkIO with `width` timesteps and `ni` float features, filled with a
// known value so assertions below are not sensitive to uninitialized memory.
NetworkIO MakeFloatInput(int width, int ni, float fill) {
  StrideMap stride_map;
  stride_map.SetStride({{1, width}});
  NetworkIO input;
  input.ResizeToMap(false, stride_map, ni);
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      input.f(t)[i] = fill;
    }
  }
  return input;
}

// The Dropout layer is in libtesseract; the "Do" spec parser lives in
// libtesseract_training (NetworkBuilder). Both are available to this test.
class DropoutLayerTest : public testing::Test {
 protected:
  DropoutLayerTest() { rand_.set_seed(42); }

  // Training-mode Forward/Backward dereference randomizer_ (set by
  // InitWeights in production), so wire one up.
  void InitRandomizer(Dropout &net) { net.InitWeights(0.0f, &rand_); }

  // Runs Forward on the Dropout (training enabled, the default) and returns
  // the output.
  NetworkIO Forward(Dropout &net, const NetworkIO &input) {
    NetworkScratch scratch;
    NetworkIO output;
    net.Forward(false, input, nullptr, &scratch, &output);
    return output;
  }

  TRand rand_;
};

// rate must lie in [0, 1). The boundary cases that are easy to get wrong are
// 1.0 (all units dropped -> 1/keep_prob is a division by zero) and negatives.
TEST_F(DropoutLayerTest, RejectsOutOfRangeRate) {
  EXPECT_THROW(Dropout("Dropout", 4, 1.0f), std::invalid_argument);
  EXPECT_THROW(Dropout("Dropout", 4, 1.5f), std::invalid_argument);
  EXPECT_THROW(Dropout("Dropout", 4, -0.1f), std::invalid_argument);
}

// dim selects the mask granularity: 0=element-wise, 1=temporal, 2=feature.
TEST_F(DropoutLayerTest, RejectsOutOfRangeDim) {
  EXPECT_THROW(Dropout("Dropout", 4, 0.5f, 3), std::invalid_argument);
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 0));
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 1));
  EXPECT_NO_THROW(Dropout("Dropout", 4, 0.5f, 2));
}

// A dropout layer is a pure activation function: it owns no learnable weights,
// and no_ is always ni_.
TEST_F(DropoutLayerTest, ShapeAndWeights) {
  Dropout net("Dropout", 7, 0.5f);
  EXPECT_EQ(7, net.NumInputs());
  EXPECT_EQ(7, net.NumOutputs());
  EXPECT_EQ(NT_DROPOUT, net.type());
  EXPECT_EQ(0, net.num_weights());
}

// spec() is used for logging and to build the .traineddata version string.
TEST_F(DropoutLayerTest, Spec) {
  EXPECT_EQ("Do0.5", Dropout("Dropout", 4, 0.5f).spec());
  EXPECT_EQ("Do0.25", Dropout("Dropout", 4, 0.25f).spec());
  EXPECT_EQ("Do0.5,1", Dropout("Dropout", 4, 0.5f, 1).spec());
  EXPECT_EQ("Do0.5,2", Dropout("Dropout", 4, 0.5f, 2).spec());
}

// In training mode each retained unit is scaled by 1/keep_prob (inverted
// dropout), so the expected output for a fully-retained input of value `fill`
// is fill / (1 - rate). Here rate=0.5 -> scale 2.
TEST_F(DropoutLayerTest, ForwardInvertedScaling) {
  const float fill = 0.25f;
  const float rate = 0.5f;
  const float scale = 1.0f / (1.0f - rate); // 2.0
  const int width = 5;
  const int ni = 3;
  Dropout net("Dropout", ni, rate); // element-wise (dim 0)
  InitRandomizer(net);
  NetworkIO input = MakeFloatInput(width, ni, fill);
  NetworkIO output = Forward(net, input);
  EXPECT_EQ(width, output.Width());
  EXPECT_EQ(ni, output.NumFeatures());
  for (int t = 0; t < width; ++t) {
    for (int i = 0; i < ni; ++i) {
      float v = output.f(t)[i];
      // Each element is either dropped (0) or kept and scaled.
      ASSERT_TRUE(v == 0.0f || v == fill * scale) << "t=" << t << " i=" << i << " v=" << v;
    }
  }
  // With rate 0.5 and only 15 elements it is possible (astronomically
  // unlikely) for every element to be dropped. Make sure at least one was
  // retained so the test is actually exercising the scale.
  int kept = 0;
  for (int t = 0; t < width; ++t)
    for (int i = 0; i < ni; ++i)
      if (output.f(t)[i] != 0.0f)
        ++kept;
  EXPECT_GT(kept, 0);
}

// Temporal dropout (dim 1) keeps or drops an entire timestep, so every
// feature of a timestep is either all-zero or all `fill*scale`.
TEST_F(DropoutLayerTest, ForwardTemporal) {
  const float fill = 0.25f;
  const float rate = 0.5f;
  const float scale = 1.0f / (1.0f - rate);
  const int width = 6;
  const int ni = 3;
  Dropout net("Dropout", ni, rate, 1);
  InitRandomizer(net);
  NetworkIO input = MakeFloatInput(width, ni, fill);
  NetworkIO output = Forward(net, input);
  for (int t = 0; t < width; ++t) {
    float first = output.f(t)[0];
    for (int i = 0; i < ni; ++i) {
      EXPECT_EQ(first, output.f(t)[i]) << "t=" << t << " feature i=" << i << " is not uniform";
      ASSERT_TRUE(first == 0.0f || first == fill * scale);
    }
  }
}

// Feature dropout (dim 2) keeps or drops an entire feature column, so every
// timestep of a feature is either all-zero or all `fill*scale`.
TEST_F(DropoutLayerTest, ForwardFeature) {
  const float fill = 0.25f;
  const float rate = 0.5f;
  const float scale = 1.0f / (1.0f - rate);
  const int width = 6;
  const int ni = 3;
  Dropout net("Dropout", ni, rate, 2);
  InitRandomizer(net);
  NetworkIO input = MakeFloatInput(width, ni, fill);
  NetworkIO output = Forward(net, input);
  for (int i = 0; i < ni; ++i) {
    float first = output.f(0)[i];
    for (int t = 0; t < width; ++t) {
      EXPECT_EQ(first, output.f(t)[i]) << "feature i=" << i << " t=" << t << " is not uniform";
      ASSERT_TRUE(first == 0.0f || first == fill * scale);
    }
  }
}

// In recognition mode (training disabled) the layer is an identity: the input
// passes through unchanged, with no masking and no rescaling.
TEST_F(DropoutLayerTest, ForwardRecognitionIdentity) {
  const float fill = 0.3f;
  const int width = 4;
  const int ni = 2;
  Dropout net("Dropout", ni, 0.5f);
  net.SetEnableTraining(TS_DISABLED);
  ASSERT_FALSE(net.IsTraining());
  NetworkIO input = MakeFloatInput(width, ni, fill);
  NetworkIO output = Forward(net, input);
  for (int t = 0; t < width; ++t)
    for (int i = 0; i < ni; ++i)
      EXPECT_FLOAT_EQ(fill, output.f(t)[i]) << "t=" << t << " i=" << i;
}

// A converted (int8) recognition network also runs the identity pass-through.
// Regression: the pass-through previously used CopyAll, which copies only the
// float buffer, so an int-mode Dropout emitted uninitialized data.
TEST_F(DropoutLayerTest, ForwardIntModeIdentity) {
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
  // The identity pass-through must reproduce the (quantized) input exactly.
  for (int t = 0; t < width; ++t)
    for (int i = 0; i < ni; ++i)
      EXPECT_EQ(input.i(t)[i], output.i(t)[i]) << "t=" << t << " i=" << i;
}

// The backward pass reuses the mask generated by the forward pass (inverted
// dropout must use the same mask in both directions), so Forward must run
// first, exactly as in the trainer. Each gradient is then either 0 or
// fwd*1/(1-rate).
TEST_F(DropoutLayerTest, BackwardInvertedScaling) {
  const float d = 0.2f;
  const float rate = 0.5f;
  const float scale = 1.0f / (1.0f - rate);
  const int width = 5;
  const int ni = 3;
  Dropout net("Dropout", ni, rate);
  InitRandomizer(net);
  // Forward populates the dropout mask that Backward must re-use.
  {
    NetworkScratch fs;
    NetworkIO fo;
    net.Forward(false, MakeFloatInput(width, ni, 0.25f), nullptr, &fs, &fo);
  }
  NetworkIO fwd = MakeFloatInput(width, ni, d);
  NetworkScratch scratch;
  NetworkIO back;
  net.Backward(false, fwd, &scratch, &back);
  EXPECT_EQ(width, back.Width());
  EXPECT_EQ(ni, back.NumFeatures());
  int kept = 0;
  for (int t = 0; t < width; ++t)
    for (int i = 0; i < ni; ++i) {
      float v = back.f(t)[i];
      ASSERT_TRUE(v == 0.0f || v == d * scale) << "t=" << t << " i=" << i << " v=" << v;
      if (v != 0.0f)
        ++kept;
    }
  EXPECT_GT(kept, 0);
}

// In recognition mode the backward pass is also an identity.
TEST_F(DropoutLayerTest, BackwardRecognitionIdentity) {
  const float d = 0.2f;
  const int width = 4;
  const int ni = 2;
  Dropout net("Dropout", ni, 0.5f);
  net.SetEnableTraining(TS_DISABLED);
  NetworkIO fwd = MakeFloatInput(width, ni, d);
  NetworkScratch scratch;
  NetworkIO back;
  net.Backward(false, fwd, &scratch, &back);
  for (int t = 0; t < width; ++t)
    for (int i = 0; i < ni; ++i)
      EXPECT_FLOAT_EQ(d, back.f(t)[i]) << "t=" << t << " i=" << i;
}

// Serializing a Dropout and reading it back must reproduce the rate, dim and
// layer type (the mask is runtime state and is intentionally not serialized).
TEST_F(DropoutLayerTest, SerializeRoundTrip) {
  Dropout net("Dropout", 6, 0.4f, 2);
  std::vector<char> data;
  {
    TFile fpw;
    fpw.OpenWrite(&data);
    ASSERT_TRUE(net.Serialize(&fpw));
  }
  TFile fpr;
  ASSERT_TRUE(fpr.Open(data.data(), data.size()));
  Network *net2 = Network::CreateFromFile(&fpr);
  ASSERT_NE(net2, nullptr);
  ASSERT_EQ(NT_DROPOUT, net2->type());
  Dropout *d2 = dynamic_cast<Dropout *>(net2);
  ASSERT_NE(d2, nullptr);
  EXPECT_EQ(6, d2->NumInputs());
  EXPECT_EQ(6, d2->NumOutputs());
  EXPECT_EQ("Do0.4,2", d2->spec());
  delete net2;
}

// ===== "Do" network-spec parsing (NetworkBuilder, libtesseract_training) =====
//
// BuildFromString is invoked through NetworkBuilder. The specs below are
// truncated after the dropout token; for a trailing element the parser consumes
// exactly the dropout token, so the next character must be a valid separator.

TEST(DropoutSpecTest, ParsesDefaultDim) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do0.25 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  ASSERT_NE(net, nullptr);
  ASSERT_EQ(NT_DROPOUT, net->type());
  Dropout *d = dynamic_cast<Dropout *>(net);
  ASSERT_NE(d, nullptr);
  EXPECT_EQ(4, d->NumInputs());
  EXPECT_EQ(4, d->NumOutputs());
  EXPECT_EQ("Do0.25", d->spec());
  // The parser must consume exactly "Do0.25", stopping before the space.
  EXPECT_EQ(' ', *s);
  delete net;
}

TEST(DropoutSpecTest, ParsesTemporalDim) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do0.5,1 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  ASSERT_NE(net, nullptr);
  Dropout *d = dynamic_cast<Dropout *>(net);
  ASSERT_NE(d, nullptr);
  EXPECT_EQ("Do0.5,1", d->spec());
  EXPECT_EQ(' ', *s);
  delete net;
}

TEST(DropoutSpecTest, ParsesFeatureDim) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do0.5,2 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  ASSERT_NE(net, nullptr);
  Dropout *d = dynamic_cast<Dropout *>(net);
  ASSERT_NE(d, nullptr);
  EXPECT_EQ("Do0.5,2", d->spec());
  EXPECT_EQ(' ', *s);
  delete net;
}

TEST(DropoutSpecTest, RejectsRateOfOne) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do1.0 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  EXPECT_EQ(nullptr, net);
}

TEST(DropoutSpecTest, RejectsNegativeRate) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do-0.5 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  EXPECT_EQ(nullptr, net);
}

TEST(DropoutSpecTest, RejectsInvalidDim) {
  StaticShape shape;
  shape.SetShape(1, 1, 1, 4);
  const char spec[] = "Do0.5,3 ";
  const char *s = spec;
  Network *net = NetworkBuilder(1).BuildFromString(shape, &s);
  EXPECT_EQ(nullptr, net);
}

} // namespace
} // namespace tesseract
