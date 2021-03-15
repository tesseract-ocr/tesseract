// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string> // std::string
#include <vector>

#include "include_gunit.h"
#include "params_model.h"
#include "serialis.h" // TFile
#include "tprintf.h"  // tprintf

namespace tesseract {

// Test some basic I/O of params model files (automated learning of language
// model weights).
#ifndef DISABLED_LEGACY_ENGINE
static bool LoadFromFile(tesseract::ParamsModel &model, const char *lang, const char *full_path) {
  tesseract::TFile fp;
  if (!fp.Open(full_path, nullptr)) {
    tprintf("Error opening file %s\n", full_path);
    return false;
  }
  return model.LoadFromFp(lang, &fp);
}
#endif

class ParamsModelTest : public testing::Test {
#ifndef DISABLED_LEGACY_ENGINE
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

  std::string TestDataNameToPath(const std::string &name) const {
    return file::JoinPath(TESTDATA_DIR, name);
  }
  std::string OutputNameToPath(const std::string &name) const {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  // Test that we are able to load a params model, save it, reload it,
  // and verify that the re-serialized version is the same as the original.
  void TestParamsModelRoundTrip(const std::string &params_model_filename) const {
    tesseract::ParamsModel orig_model;
    tesseract::ParamsModel duplicate_model;
    file::MakeTmpdir();
    std::string orig_file = TestDataNameToPath(params_model_filename);
    std::string out_file = OutputNameToPath(params_model_filename);

    EXPECT_TRUE(LoadFromFile(orig_model, "eng", orig_file.c_str()));
    EXPECT_TRUE(orig_model.SaveToFile(out_file.c_str()));

    EXPECT_TRUE(LoadFromFile(duplicate_model, "eng", out_file.c_str()));
    EXPECT_TRUE(orig_model.Equivalent(duplicate_model));
  }
#endif
};

TEST_F(ParamsModelTest, TestEngParamsModelIO) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because ParamsModel::LoadFromFp is missing.
  GTEST_SKIP();
#else
  TestParamsModelRoundTrip("eng.params_model");
#endif
}

} // namespace tesseract
