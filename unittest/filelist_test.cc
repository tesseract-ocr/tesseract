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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <tesseract/baseapi.h>
#include "include_gunit.h"

namespace tesseract {

namespace {

class CinRedirect {
public:
  explicit CinRedirect(std::istream &in) : old_(std::cin.rdbuf(in.rdbuf())) {}
  ~CinRedirect() {
    std::cin.rdbuf(old_);
  }

private:
  std::streambuf *old_;
};

class CwdGuard {
public:
  explicit CwdGuard(const std::filesystem::path &dir)
      : old_(std::filesystem::current_path()) {
    std::filesystem::current_path(dir);
  }
  ~CwdGuard() {
    std::filesystem::current_path(old_);
  }

private:
  std::filesystem::path old_;
};

bool ProcessStdin(TessBaseAPI *api, const std::string &input) {
  std::istringstream in(input);
  CinRedirect redirect(in);
  return api->ProcessPages("-", nullptr, 0, nullptr);
}

} // namespace

// Filelists whose first filename starts with the TIFF byte-order markers
// "MM" or "II" must still be treated as filelists when streamed on stdin.
// Leptonica's findFileFormatBuffer() only looks at those two bytes, so
// without a version check Tesseract misdetects the list as a TIFF (issue 4619).

TEST(FileListStdinTest, FilenamesStartingWithMMAreNotTreatedAsTiff) {
  TessBaseAPI api;
  api.InitForAnalysePage();
  // Missing listed image: filelist path returns false; TIFF path returns true.
  EXPECT_FALSE(ProcessStdin(&api, "MM11_missing_image.pnm\n"));
}

TEST(FileListStdinTest, FilenamesStartingWithIIAreNotTreatedAsTiff) {
  TessBaseAPI api;
  api.InitForAnalysePage();
  EXPECT_FALSE(ProcessStdin(&api, "II11_missing_image.pnm\n"));
}

TEST(FileListStdinTest, FileListStartingWithMMReadsTheListedImage) {
  TessBaseAPI api;
  api.InitForAnalysePage();
  api.SetPageSegMode(PSM_AUTO_ONLY);

  const auto tmp = std::filesystem::temp_directory_path() / "tesseract_mm_filelist_test";
  std::filesystem::create_directories(tmp);
  {
    // Binary PGM: leptonica rejects the minimal ASCII P1 we tried first.
    std::ofstream out(tmp / "MMpage.pgm", std::ios::binary);
    ASSERT_TRUE(out.good());
    out << "P5\n1 1\n255\n" << char(128);
  }

  {
    CwdGuard cwd(tmp);
    // 1x1 image may fail layout analysis; we only need the filelist path to run.
    ProcessStdin(&api, "MMpage.pgm\n");
  }

  // ProcessPage records the listed filename. The TIFF misdetect never does.
  EXPECT_STREQ("MMpage.pgm", api.GetInputName());
}

} // namespace tesseract
