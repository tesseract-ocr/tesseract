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

#include "errcode.h"  // for ASSERT_HOST
#include "fileio.h"   // for tesseract::File
#include "gtest/gtest.h"

const char* FLAGS_test_tmpdir = "./tmp";

class file : public tesseract::File {
public:

// Create a file and write a string to it.
  static bool WriteStringToFile(const std::string& contents, const std::string& filename) {
    File::WriteStringToFileOrDie(contents, filename);
    return true;
  }

  static bool GetContents(const std::string& filename, std::string* out, int) {
    return File::ReadFileToString(filename, out);
  }

  static bool SetContents(const std::string& name, const std::string& contents, bool /*is_default*/) {
    return WriteStringToFile(contents, name);
  }

  static int Defaults() {
    return 0;
  }

  static std::string JoinPath(const std::string& s1, const std::string& s2) {
    return tesseract::File::JoinPath(s1, s2);
  }

  static std::string JoinPath(const std::string& s1, const std::string& s2,
                              const std::string& s3) {
    return JoinPath(JoinPath(s1, s2), s3);
  }
};

#define ARRAYSIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define CHECK(test) ASSERT_HOST(test)
#define CHECK_GT(test, value) ASSERT_HOST((test) > (value))
#define CHECK_LT(test, value) ASSERT_HOST((test) < (value))
#define CHECK_LE(test, value) ASSERT_HOST((test) <= (value))
#define CHECK_OK(test) ASSERT_HOST(test)

#endif  // TESSERACT_UNITTEST_INCLUDE_GUNIT_H_
