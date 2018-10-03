
#include <vector>

#include "tesseract/wordrec/params_model.h"

namespace {

// Test some basic I/O of params model files (automated learning of language
// model weights).
class ParamsModelTest : public testing::Test {
 protected:
  string TestDataNameToPath(const string& name) const {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
  string OutputNameToPath(const string& name) const {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  // Test that we are able to load a params model, save it, reload it,
  // and verify that the re-serialized version is the same as the original.
  void TestParamsModelRoundTrip(const string& params_model_filename) const {
    tesseract::ParamsModel orig_model;
    tesseract::ParamsModel duplicate_model;
    string orig_file = TestDataNameToPath(params_model_filename);
    string out_file = OutputNameToPath(params_model_filename);

    EXPECT_TRUE(orig_model.LoadFromFile("eng", orig_file.c_str()));
    EXPECT_TRUE(orig_model.SaveToFile(out_file.c_str()));

    EXPECT_TRUE(duplicate_model.LoadFromFile("eng", out_file.c_str()));
    EXPECT_TRUE(orig_model.Equivalent(duplicate_model));
  }
};

TEST_F(ParamsModelTest, TestEngParamsModelIO) {
  TestParamsModelRoundTrip("eng.params_model");
}

}  // namespace
