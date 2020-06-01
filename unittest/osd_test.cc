///////////////////////////////////////////////////////////////////////
// File:        osd_test.cc
// Description: OSD Tests for Tesseract.
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

// based on https://gist.github.com/amitdo/7c7a522004dd79b398340c9595b377e1

// expects clones of tessdata, tessdata_fast and tessdata_best repos

//#include "log.h"
#include <iostream>
#include <memory>               // std::unique_ptr
#include <string>
#include <tesseract/baseapi.h>
#include "include_gunit.h"
#include "leptonica/allheaders.h"

namespace {

class TestClass : public testing::Test {
 protected:
};

#ifndef DISABLED_LEGACY_ENGINE
static void OSDTester(int expected_deg, const char* imgname, const char* tessdatadir) {
  // log.info() << tessdatadir << " for image: " << imgname << std::endl;
  std::unique_ptr<tesseract::TessBaseAPI> api(new tesseract::TessBaseAPI());
  ASSERT_FALSE(api->Init(tessdatadir, "osd"))
      << "Could not initialize tesseract.";
  Pix* image = pixRead(imgname);
  ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
  api->SetImage(image);
  int orient_deg;
  float orient_conf;
  const char* script_name;
  float script_conf;
  bool detected = api->DetectOrientationScript(&orient_deg, &orient_conf,
                                               &script_name, &script_conf);
  ASSERT_FALSE(!detected) << "Failed to detect OSD.";
  printf(
      "************ Orientation in degrees: %d, Orientation confidence: %.2f\n"
      "             Script: %s, Script confidence: %.2f\n",
      orient_deg, orient_conf, script_name, script_conf);
  EXPECT_EQ(expected_deg, orient_deg);
  api->End();
  pixDestroy(&image);
}
#endif

class OSDTest : public TestClass,
                public ::testing::WithParamInterface<
                    std::tuple<int, const char*, const char*>> {};

TEST_P(OSDTest, MatchOrientationDegrees) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because TessBaseAPI::DetectOrientationScript is missing.
  GTEST_SKIP();
#else
  OSDTester(std::get<0>(GetParam()), std::get<1>(GetParam()),
            std::get<2>(GetParam()));
#endif
}

INSTANTIATE_TEST_CASE_P(
    TessdataEngEuroHebrew, OSDTest,
    ::testing::Combine(::testing::Values(0),
                       ::testing::Values(TESTING_DIR "/phototest.tif",
                                         TESTING_DIR "/eurotext.tif",
                                         TESTING_DIR "/hebrew.png"),
                       ::testing::Values(TESSDATA_DIR)));

INSTANTIATE_TEST_CASE_P(
    TessdataBestEngEuroHebrew, OSDTest,
    ::testing::Combine(::testing::Values(0),
                       ::testing::Values(TESTING_DIR "/phototest.tif",
                                         TESTING_DIR "/eurotext.tif",
                                         TESTING_DIR "/hebrew.png"),
                       ::testing::Values(TESSDATA_DIR "_best")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastEngEuroHebrew, OSDTest,
    ::testing::Combine(::testing::Values(0),
                       ::testing::Values(TESTING_DIR "/phototest.tif",
                                         TESTING_DIR "/eurotext.tif",
                                         TESTING_DIR "/hebrew.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastRotated90, OSDTest,
    ::testing::Combine(::testing::Values(90),
                       ::testing::Values(TESTING_DIR
                                         "/phototest-rotated-R.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastRotated180, OSDTest,
    ::testing::Combine(::testing::Values(180),
                       ::testing::Values(TESTING_DIR
                                         "/phototest-rotated-180.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastRotated270, OSDTest,
    ::testing::Combine(::testing::Values(270),
                       ::testing::Values(TESTING_DIR
                                         "/phototest-rotated-L.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastDevaRotated270, OSDTest,
    ::testing::Combine(::testing::Values(270),
                       ::testing::Values(TESTING_DIR
                                         "/devatest-rotated-270.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_CASE_P(
    TessdataFastDeva, OSDTest,
    ::testing::Combine(::testing::Values(0),
                       ::testing::Values(TESTING_DIR "/devatest.png"),
                       ::testing::Values(TESSDATA_DIR "_fast")));

}  // namespace
