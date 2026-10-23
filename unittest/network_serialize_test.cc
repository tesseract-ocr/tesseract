///////////////////////////////////////////////////////////////////////
// File:        network_serialize_test.cc
// Description: Tests the training state that Network::Serialize puts
//              into the network header. A recognition dump is written
//              while the network is in TS_TEMP_DISABLE, a state that
//              only means something to the writer, so the file has to
//              say TS_DISABLED like a model that never trained.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "network.h"  // for TrainingState
#include "serialis.h" // for TFile
#include "series.h"   // for Series

#include <string>
#include <vector>

namespace tesseract {
namespace {

// Network::Serialize writes an int8 NT_NONE, the type name as a length
// prefixed string and then the int8 training state.
size_t TrainingStateOffset(const std::string &type_name) {
  return 1 + 4 + type_name.size();
}

std::vector<char> SerializeSeries(TrainingState state) {
  Series series("test");
  // TS_TEMP_DISABLE is only reachable from TS_ENABLED, which is how a
  // trainer gets there before writing a recognition dump.
  series.SetEnableTraining(TS_ENABLED);
  series.SetEnableTraining(state);
  std::vector<char> data;
  TFile fp;
  fp.OpenWrite(&data);
  EXPECT_TRUE(series.Serialize(&fp));
  return data;
}

TEST(NetworkSerializeTest, TemporarilyDisabledIsWrittenAsDisabled) {
  const std::vector<char> data = SerializeSeries(TS_TEMP_DISABLE);
  ASSERT_GT(data.size(), TrainingStateOffset("Series"));
  EXPECT_EQ(TS_DISABLED, data[TrainingStateOffset("Series")]);
  // A recognition dump has to look like a network that was never training.
  EXPECT_EQ(SerializeSeries(TS_DISABLED), data);
}

TEST(NetworkSerializeTest, EnabledIsWrittenAsEnabled) {
  const std::vector<char> data = SerializeSeries(TS_ENABLED);
  ASSERT_GT(data.size(), TrainingStateOffset("Series"));
  EXPECT_EQ(TS_ENABLED, data[TrainingStateOffset("Series")]);
}

} // namespace
} // namespace tesseract
