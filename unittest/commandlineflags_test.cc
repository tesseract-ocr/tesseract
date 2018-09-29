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

#include "commandlineflags.h"

#include "include_gunit.h"

// Flags used for testing parser.
INT_PARAM_FLAG(foo_int, 0, "Integer flag for testing");
INT_PARAM_FLAG(bar_int, 0, "Integer flag for testing");
DOUBLE_PARAM_FLAG(foo_double, 0.1, "Double flag for testing");
DOUBLE_PARAM_FLAG(bar_double, 0.2, "Double flag for testing");
STRING_PARAM_FLAG(foo_string, "foo", "String flag for testing");
STRING_PARAM_FLAG(bar_string, "bar", "String flag for testing");
BOOL_PARAM_FLAG(foo_bool, false, "Bool flag for testing");
BOOL_PARAM_FLAG(bar_bool, false, "Bool flag for testing");
// A flag whose name is a single character, tested for backward
// compatability. This should be selected to not conflict with existing flags
// in commontraining.cpp.
STRING_PARAM_FLAG(q, "", "Single character name");

namespace {

class CommandlineflagsTest : public ::testing::Test {
 protected:
  void TestParser(int argc, const char** const_argv) {
    TestParser("", argc, const_argv);
  }
  void TestParser(const char* usage, int argc, const char** const_argv) {
    // Make a copy of the pointer since it can be altered by the function.
    char** argv = const_cast<char**>(const_argv);
    tesseract::ParseCommandLineFlags(usage, &argc, &argv, true);
  }
};

TEST_F(CommandlineflagsTest, RemoveFlags) {
  const char* const_argv[] = {"Progname", "--foo_int", "3", "file1.h",
                              "file2.h"};
  int argc = ARRAYSIZE(const_argv);
  char** argv = const_cast<char**>(const_argv);
  tesseract::ParseCommandLineFlags(argv[0], &argc, &argv, true);

  // argv should be rearranged to look like { "Progname", "file1.h", "file2.h" }
  EXPECT_EQ(3, argc);
  EXPECT_STREQ("Progname", argv[0]);
  EXPECT_STREQ("file1.h", argv[1]);
  EXPECT_STREQ("file2.h", argv[2]);
}

#if 0  // TODO: this test needs an update (it currently fails).
TEST_F(CommandlineflagsTest, PrintUsageAndExit) {
  const char* argv[] = { "Progname", "--help" };
  EXPECT_EXIT(TestParser("Progname [flags]", ARRAYSIZE(argv), argv),
              ::testing::ExitedWithCode(0),
              "USAGE: Progname \\[flags\\]");
}
#endif

TEST_F(CommandlineflagsTest, ExitsWithErrorOnInvalidFlag) {
  const char* argv[] = {"", "--test_nonexistent_flag"};
  EXPECT_EXIT(TestParser(ARRAYSIZE(argv), argv), ::testing::ExitedWithCode(1),
              "ERROR: Non-existent flag");
}

TEST_F(CommandlineflagsTest, ParseIntegerFlags) {
  const char* argv[] = {"", "--foo_int=3", "--bar_int", "-4"};
  TestParser(ARRAYSIZE(argv), argv);
  EXPECT_EQ(3, FLAGS_foo_int);
  EXPECT_EQ(-4, FLAGS_bar_int);

  const char* arg_no_value[] = {"", "--bar_int"};
  EXPECT_EXIT(TestParser(ARRAYSIZE(arg_no_value), arg_no_value),
              ::testing::ExitedWithCode(1), "ERROR");

  const char* arg_invalid_value[] = {"", "--bar_int", "--foo_int=3"};
  EXPECT_EXIT(TestParser(ARRAYSIZE(arg_invalid_value), arg_invalid_value),
              ::testing::ExitedWithCode(1), "ERROR");

  const char* arg_bad_format[] = {"", "--bar_int="};
  EXPECT_EXIT(TestParser(ARRAYSIZE(arg_bad_format), arg_bad_format),
              ::testing::ExitedWithCode(1), "ERROR");
}

TEST_F(CommandlineflagsTest, ParseDoubleFlags) {
  const char* argv[] = {"", "--foo_double=3.14", "--bar_double", "1.2"};
  TestParser(ARRAYSIZE(argv), argv);

  EXPECT_EQ(3.14, FLAGS_foo_double);
  EXPECT_EQ(1.2, FLAGS_bar_double);

  const char* arg_no_value[] = {"", "--bar_double"};
  EXPECT_EXIT(TestParser(2, arg_no_value), ::testing::ExitedWithCode(1),
              "ERROR");

  const char* arg_bad_format[] = {"", "--bar_double="};
  EXPECT_EXIT(TestParser(2, arg_bad_format), ::testing::ExitedWithCode(1),
              "ERROR");
}

TEST_F(CommandlineflagsTest, ParseStringFlags) {
  const char* argv[] = {"", "--foo_string=abc", "--bar_string", "def"};
  TestParser(ARRAYSIZE(argv), argv);

  EXPECT_STREQ("abc", FLAGS_foo_string.c_str());
  EXPECT_STREQ("def", FLAGS_bar_string.c_str());

  const char* arg_no_value[] = {"", "--bar_string"};
  EXPECT_EXIT(TestParser(2, arg_no_value), ::testing::ExitedWithCode(1),
              "ERROR");

  FLAGS_bar_string.set_value("bar");
  const char* arg_empty_string[] = {"", "--bar_string="};
  TestParser(2, arg_empty_string);
  EXPECT_STREQ("", FLAGS_bar_string.c_str());
}

TEST_F(CommandlineflagsTest, ParseBoolFlags) {
  const char* argv[] = {"", "--foo_bool=true", "--bar_bool=1"};
  FLAGS_foo_bool.set_value(false);
  FLAGS_bar_bool.set_value(false);
  TestParser(ARRAYSIZE(argv), argv);
  // Verify changed value
  EXPECT_TRUE(FLAGS_foo_bool);
  EXPECT_TRUE(FLAGS_bar_bool);

  const char* inv_argv[] = {"", "--foo_bool=false", "--bar_bool=0"};
  FLAGS_foo_bool.set_value(true);
  FLAGS_bar_bool.set_value(true);
  TestParser(3, inv_argv);
  // Verify changed value
  EXPECT_FALSE(FLAGS_foo_bool);
  EXPECT_FALSE(FLAGS_bar_bool);

  const char* arg_implied_true[] = {"", "--bar_bool"};
  FLAGS_bar_bool.set_value(false);
  TestParser(2, arg_implied_true);
  EXPECT_TRUE(FLAGS_bar_bool);

  const char* arg_missing_val[] = {"", "--bar_bool="};
  EXPECT_EXIT(TestParser(2, arg_missing_val), ::testing::ExitedWithCode(1),
              "ERROR");
}

TEST_F(CommandlineflagsTest, ParseOldFlags) {
  EXPECT_STREQ("", FLAGS_q.c_str());
  const char* argv[] = {"", "-q", "text"};
  TestParser(ARRAYSIZE(argv), argv);
  EXPECT_STREQ("text", FLAGS_q.c_str());
}
}  // namespace
