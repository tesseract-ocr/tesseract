///////////////////////////////////////////////////////////////////////
// File:        apiexample_test.cc
// Description: Api Test for Tesseract using text fixtures and parameters.
// Tests for Devanagari, Latin and Arabic scripts are disabled by default.
// Disabled tests can be run when required by using the
// --gtest_also_run_disabled_tests argument.
//                 ./unittest/apiexample_test --gtest_also_run_disabled_tests
//
// Author:      ShreeDevi Kumar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

// expects clone of tessdata_fast repo in ../../tessdata_fast

//#include "log.h"
#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory> // std::unique_ptr
#include <string>
#include "include_gunit.h"
#include "image.h"

namespace tesseract {

class QuickTest : public testing::Test {
protected:
  void SetUp() override {
    start_time_ = time(nullptr);
  }
  void TearDown() override {
#ifndef NDEBUG
    // Debug builds can be very slow, so allow 4 min for OCR of a test image.
    // apitest_example including disabled tests takes about 18 min on ARMv7.
    const time_t MAX_SECONDS_FOR_TEST = 240;
#else
    // Release builds typically need less than 10 s for OCR of a test image,
    // apitest_example including disabled tests takes about 90 s on ARMv7.
    const time_t MAX_SECONDS_FOR_TEST = 55;
#endif
    const time_t end_time = time(nullptr);
    EXPECT_TRUE(end_time - start_time_ <= MAX_SECONDS_FOR_TEST)
        << "The test took too long - " << ::testing::PrintToString(end_time - start_time_);
  }
  time_t start_time_;
};

void OCRTester(const char *imgname, const char *groundtruth, const char *tessdatadir,
               const char *lang) {
  // log.info() << tessdatadir << " for language: " << lang << std::endl;
  char *outText;
  std::locale loc("C"); // You can also use "" for the default system locale
  std::ifstream file(groundtruth);
  file.imbue(loc); // Use it for file input
  std::string gtText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  auto api = std::make_unique<tesseract::TessBaseAPI>();
  ASSERT_FALSE(api->Init(tessdatadir, lang)) << "Could not initialize tesseract.";
  Image image = pixRead(imgname);
  ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
  api->SetImage(image);
  outText = api->GetUTF8Text();
  EXPECT_EQ(gtText, outText) << "Phototest.tif OCR does not match ground truth for "
                             << ::testing::PrintToString(lang);
  api->End();
  api->ClearPersistentCache();
  delete[] outText;
  image.destroy();
}

class MatchGroundTruth : public QuickTest, public ::testing::WithParamInterface<const char *> {};

TEST_P(MatchGroundTruth, FastPhototestOCR) {
  OCRTester(TESTING_DIR "/phototest.tif", TESTING_DIR "/phototest.txt", TESSDATA_DIR "_fast",
            GetParam());
}

TEST_P(MatchGroundTruth, BestPhototestOCR) {
  OCRTester(TESTING_DIR "/phototest.tif", TESTING_DIR "/phototest.txt", TESSDATA_DIR "_best",
            GetParam());
}

TEST_P(MatchGroundTruth, TessPhototestOCR) {
  OCRTester(TESTING_DIR "/phototest.tif", TESTING_DIR "/phototest.txt", TESSDATA_DIR, GetParam());
}

INSTANTIATE_TEST_SUITE_P(Eng, MatchGroundTruth, ::testing::Values("eng"));
INSTANTIATE_TEST_SUITE_P(DISABLED_Latin, MatchGroundTruth, ::testing::Values("script/Latin"));
INSTANTIATE_TEST_SUITE_P(DISABLED_Deva, MatchGroundTruth, ::testing::Values("script/Devanagari"));
INSTANTIATE_TEST_SUITE_P(DISABLED_Arabic, MatchGroundTruth, ::testing::Values("script/Arabic"));

class EuroText : public QuickTest {};

TEST_F(EuroText, FastLatinOCR) {
  OCRTester(TESTING_DIR "/eurotext.tif", TESTING_DIR "/eurotext.txt", TESSDATA_DIR "_fast",
            "script/Latin");
}

// script/Latin for eurotext.tif does not match groundtruth
// for tessdata & tessdata_best.
// so do not test these here.

} // namespace tesseract
