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
// Portability include to match the Google test environment.

#ifndef TESSERACT_UNITTEST_INCLUDE_GUNIT_H_
#define TESSERACT_UNITTEST_INCLUDE_GUNIT_H_

#include "errcode.h" // for ASSERT_HOST
#include "fileio.h"  // for tesseract::File
#include "gtest/gtest.h"
#include "log.h" // for LOG

static const char *FLAGS_test_tmpdir = "./tmp";

namespace tesseract {

static inline void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

} // namespace tesseract

class file : public tesseract::File {
public:
  static void MakeTmpdir() {
#if defined(_WIN32)
    _mkdir(FLAGS_test_tmpdir);
#else
    mkdir(FLAGS_test_tmpdir, S_IRWXU | S_IRWXG);
#endif
  }

  // Create a file and write a string to it.
  static bool WriteStringToFile(const std::string &contents, const std::string &filename) {
    File::WriteStringToFileOrDie(contents, filename);
    return true;
  }

  static bool GetContents(const std::string &filename, std::string *out, int) {
    return File::ReadFileToString(filename, out);
  }

  static bool SetContents(const std::string &name, const std::string &contents,
                          bool /*is_default*/) {
    return WriteStringToFile(contents, name);
  }

  static int Defaults() {
    return 0;
  }

  static std::string JoinPath(const std::string &s1, const std::string &s2) {
    return tesseract::File::JoinPath(s1, s2);
  }

  static std::string JoinPath(const std::string &s1, const std::string &s2, const std::string &s3) {
    return JoinPath(JoinPath(s1, s2), s3);
  }
};

// /usr/include/tensorflow/core/platform/default/logging.h defines the CHECK* macros.
#if !defined(CHECK)
#  define CHECK(condition) \
    if (!(condition))      \
    LOG(FATAL) << "Check failed: " #condition " "
#  define CHECK_EQ(test, value) CHECK((test) == (value))
#  define CHECK_GE(test, value) CHECK((test) >= (value))
#  define CHECK_GT(test, value) CHECK((test) > (value))
#  define CHECK_LT(test, value) CHECK((test) < (value))
#  define CHECK_LE(test, value) CHECK((test) <= (value))
#  define CHECK_OK(test) CHECK(test)
#endif

#endif // TESSERACT_UNITTEST_INCLUDE_GUNIT_H_
