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


#include <stdio.h>
#include <memory>

#include "absl/strings/str_split.h"

#include "fileio.h"
#include "include_gunit.h"

namespace {

using tesseract::File;
using tesseract::InputBuffer;
using tesseract::OutputBuffer;

TEST(FileTest, JoinPath) {
  EXPECT_EQ("/abc/def", File::JoinPath("/abc", "def"));
  EXPECT_EQ("/abc/def", File::JoinPath("/abc/", "def"));
  EXPECT_EQ("def", File::JoinPath("", "def"));
}

TEST(OutputBufferTest, WriteString) {
  const int kMaxBufSize = 128;
  char buffer[kMaxBufSize];
  for (int i = 0; i < kMaxBufSize; ++i) buffer[i] = '\0';
  FILE* fp = fmemopen(buffer, kMaxBufSize, "w");
  CHECK(fp != nullptr);

  {
    std::unique_ptr<OutputBuffer> output(new OutputBuffer(fp));
    output->WriteString("Hello ");
    output->WriteString("world!");
  }
  EXPECT_STREQ("Hello world!", buffer);
}

TEST(InputBufferTest, Read) {
  const int kMaxBufSize = 128;
  char buffer[kMaxBufSize];
  snprintf(buffer, kMaxBufSize, "Hello\n world!");
  EXPECT_STREQ("Hello\n world!", buffer);
  FILE* fp = fmemopen(buffer, kMaxBufSize, "r");
  CHECK(fp != nullptr);

  std::string str;
  std::unique_ptr<InputBuffer> input(new InputBuffer(fp));
  EXPECT_TRUE(input->Read(&str));
  std::vector<std::string> lines = absl::StrSplit(str, '\n', absl::SkipEmpty());
  EXPECT_EQ(2, lines.size());
  EXPECT_EQ("Hello", lines[0]);
  EXPECT_EQ(" world!", lines[1]);
}

}  // namespace
