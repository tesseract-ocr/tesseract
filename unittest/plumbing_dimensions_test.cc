///////////////////////////////////////////////////////////////////////
// File:        plumbing_dimensions_test.cc
// Description: Serialized child-layer dimension consistency tests.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "parallel.h"
#include "reconfig.h"
#include "series.h"

#include <limits>
#include <memory>
#include <vector>

namespace tesseract {
namespace {

// Each child is valid independently. The parent must check their dimensions.
bool ReadChildren(Plumbing *parent, const Network &first,
                  const Network &second) {
  std::vector<char> data;
  TFile writer;
  writer.OpenWrite(&data);
  const uint32_t count = 2;
  EXPECT_TRUE(writer.Serialize(&count));
  EXPECT_TRUE(first.Serialize(&writer));
  EXPECT_TRUE(second.Serialize(&writer));
  TFile reader;
  EXPECT_TRUE(reader.Open(data.data(), data.size()));
  return parent->DeSerialize(&reader);
}

TEST(PlumbingDimensionsTest, RejectsSeriesDimensionMismatch) {
  Reconfig first("first", 2, 3, 1);
  Reconfig second("second", 3, 1, 1);
  Series parent("series");
  EXPECT_FALSE(ReadChildren(&parent, first, second));
}

TEST(PlumbingDimensionsTest, RejectsParallelDimensionMismatch) {
  Reconfig first("first", 2, 1, 1);
  Reconfig second("second", 3, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  EXPECT_FALSE(ReadChildren(&parent, first, second));
}

TEST(PlumbingDimensionsTest, RejectsParallelOutputSumBeyondIntRange) {
  Reconfig first("first", 1, std::numeric_limits<int>::max(), 1);
  Reconfig second("second", 1, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  EXPECT_FALSE(ReadChildren(&parent, first, second));
}

TEST(PlumbingDimensionsTest, AcceptsParallelOutputSumAtIntLimit) {
  Reconfig first("first", 1, std::numeric_limits<int>::max() - 1, 1);
  Reconfig second("second", 1, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  ASSERT_TRUE(ReadChildren(&parent, first, second));
  EXPECT_EQ(1, parent.NumInputs());
  EXPECT_EQ(std::numeric_limits<int>::max(), parent.NumOutputs());
  EXPECT_EQ(2U, parent.stack().size());
}

TEST(PlumbingDimensionsTest, AcceptsMatchingSeriesDimensions) {
  Reconfig first("first", 2, 3, 1);
  Reconfig second("second", 6, 2, 1);
  Series parent("series");
  ASSERT_TRUE(ReadChildren(&parent, first, second));
  EXPECT_EQ(2, parent.NumInputs());
  EXPECT_EQ(12, parent.NumOutputs());
  EXPECT_EQ(2U, parent.stack().size());
}

TEST(PlumbingDimensionsTest, RoundTripsValidParallelLayers) {
  Parallel original("parallel", NT_PARALLEL);
  original.AddToStack(new Reconfig("first", 2, 3, 1));
  original.AddToStack(new Reconfig("second", 2, 2, 1));
  std::vector<char> data;
  TFile writer;
  writer.OpenWrite(&data);
  ASSERT_TRUE(original.Serialize(&writer));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  std::unique_ptr<Network> restored(Network::CreateFromFile(&reader));
  ASSERT_NE(nullptr, restored);
  EXPECT_EQ(original.NumInputs(), restored->NumInputs());
  EXPECT_EQ(original.NumOutputs(), restored->NumOutputs());
  EXPECT_EQ(0U, reader.RemainingBytes());
}

} // namespace
} // namespace tesseract
