#include "tesseract/training/fileio.h"

#include <stdio.h>
#include <memory>

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
  for (int i = 0; i < kMaxBufSize; ++i)
    buffer[i] = '\0';
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

  string str;
  std::unique_ptr<InputBuffer> input(new InputBuffer(fp));
  EXPECT_TRUE(input->Read(&str));
  std::vector<string> lines = absl::StrSplit(str, '\n', absl::SkipEmpty());
  EXPECT_EQ(2, lines.size());
  EXPECT_EQ("Hello", lines[0]);
  EXPECT_EQ(" world!", lines[1]);
}

}  // namespace
