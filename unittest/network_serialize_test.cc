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

// ASSERT_TRUE needs a function that returns nothing, so the bytes come
// back through the parameter.
void SerializeSeries(TrainingState state, std::vector<char> *data) {
  Series series("test");
  // TS_TEMP_DISABLE is only reachable from TS_ENABLED, which is how a
  // trainer gets there before writing a recognition dump.
  series.SetEnableTraining(TS_ENABLED);
  series.SetEnableTraining(state);
  TFile fp;
  fp.OpenWrite(data);
  ASSERT_TRUE(series.Serialize(&fp));
}

TEST(NetworkSerializeTest, TemporarilyDisabledIsWrittenAsDisabled) {
  std::vector<char> data;
  ASSERT_NO_FATAL_FAILURE(SerializeSeries(TS_TEMP_DISABLE, &data));
  ASSERT_GT(data.size(), TrainingStateOffset("Series"));
  EXPECT_EQ(TS_DISABLED, data[TrainingStateOffset("Series")]);
  // A recognition dump has to look like a network that was never training.
  std::vector<char> disabled;
  ASSERT_NO_FATAL_FAILURE(SerializeSeries(TS_DISABLED, &disabled));
  EXPECT_EQ(disabled, data);
}

TEST(NetworkSerializeTest, EnabledIsWrittenAsEnabled) {
  std::vector<char> data;
  ASSERT_NO_FATAL_FAILURE(SerializeSeries(TS_ENABLED, &data));
  ASSERT_GT(data.size(), TrainingStateOffset("Series"));
  EXPECT_EQ(TS_ENABLED, data[TrainingStateOffset("Series")]);
}

} // namespace
} // namespace tesseract
