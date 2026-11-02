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

// Serializes the two children into *data.
bool SerializeChildren(std::vector<char> *data, const Network &first,
                       const Network &second) {
  TFile writer;
  writer.OpenWrite(data);
  const uint32_t count = 2;
  return writer.Serialize(&count) && first.Serialize(&writer) &&
         second.Serialize(&writer);
}

TEST(PlumbingDimensionsTest, RejectsSeriesDimensionMismatch) {
  Reconfig first("first", 2, 3, 1);
  Reconfig second("second", 3, 1, 1);
  Series parent("series");
  std::vector<char> data;
  ASSERT_TRUE(SerializeChildren(&data, first, second));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  EXPECT_FALSE(parent.DeSerialize(&reader));
}

TEST(PlumbingDimensionsTest, RejectsParallelDimensionMismatch) {
  Reconfig first("first", 2, 1, 1);
  Reconfig second("second", 3, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  std::vector<char> data;
  ASSERT_TRUE(SerializeChildren(&data, first, second));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  EXPECT_FALSE(parent.DeSerialize(&reader));
}

TEST(PlumbingDimensionsTest, RejectsParallelOutputSumOverflow) {
  Reconfig first("first", 1, std::numeric_limits<int>::max(), 1);
  Reconfig second("second", 1, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  std::vector<char> data;
  ASSERT_TRUE(SerializeChildren(&data, first, second));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  EXPECT_FALSE(parent.DeSerialize(&reader));
}

TEST(PlumbingDimensionsTest, AcceptsParallelOutputSumAtIntLimit) {
  Reconfig first("first", 1, std::numeric_limits<int>::max() - 1, 1);
  Reconfig second("second", 1, 1, 1);
  Parallel parent("parallel", NT_PARALLEL);
  std::vector<char> data;
  ASSERT_TRUE(SerializeChildren(&data, first, second));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  ASSERT_TRUE(parent.DeSerialize(&reader));
  EXPECT_EQ(1, parent.NumInputs());
  EXPECT_EQ(std::numeric_limits<int>::max(), parent.NumOutputs());
  EXPECT_EQ(2U, parent.stack().size());
}

TEST(PlumbingDimensionsTest, AcceptsMatchingSeriesDimensions) {
  Reconfig first("first", 2, 3, 1);
  Reconfig second("second", 6, 2, 1);
  Series parent("series");
  std::vector<char> data;
  ASSERT_TRUE(SerializeChildren(&data, first, second));
  TFile reader;
  ASSERT_TRUE(reader.Open(data.data(), data.size()));
  ASSERT_TRUE(parent.DeSerialize(&reader));
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
