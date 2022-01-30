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

#include "fileio.h"
#include "include_gunit.h"

namespace tesseract {

TEST(FileTest, JoinPath) {
  EXPECT_EQ("/abc/def", File::JoinPath("/abc", "def"));
  EXPECT_EQ("/abc/def", File::JoinPath("/abc/", "def"));
  EXPECT_EQ("def", File::JoinPath("", "def"));
}

TEST(OutputBufferTest, WriteString) {
  const int kMaxBufSize = 128;
  char buffer[kMaxBufSize];
  for (char &i : buffer) {
    i = '\0';
  }
  FILE *fp = tmpfile();
  CHECK(fp != nullptr);

  auto output = std::make_unique<OutputBuffer>(fp);
  output->WriteString("Hello ");
  output->WriteString("world!");

  rewind(fp);
  auto s = "Hello world!";
  fread(buffer, strlen(s), 1, fp);
  EXPECT_STREQ(s, buffer);
}

TEST(InputBufferTest, Read) {
  const int kMaxBufSize = 128;
  char buffer[kMaxBufSize];
  auto s = "Hello\n world!";
  strncpy(buffer, s, kMaxBufSize);
  EXPECT_STREQ(s, buffer);
  FILE *fp = tmpfile();
  CHECK(fp != nullptr);
  fwrite(buffer, strlen(s), 1, fp);
  rewind(fp);

  std::string str;
  auto input = std::make_unique<InputBuffer>(fp);
  EXPECT_TRUE(input->Read(&str));
  std::vector<std::string> lines = split(str, '\n');
  EXPECT_EQ(2, lines.size());
  EXPECT_EQ("Hello", lines[0]);
  EXPECT_EQ(" world!", lines[1]);
}

} // namespace tesseract
