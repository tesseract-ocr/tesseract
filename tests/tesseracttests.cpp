// File:        tesseracttests.cpp
// Description: OSD Tests for Tesseract.
// Author:      Stefan Weil
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
//based on https://gist.github.com/amitdo/7c7a522004dd79b398340c9595b377e1 

#include "log.h"
#include "include_gunit.h"
#include "baseapi.h"
#include "leptonica/allheaders.h"
#include <iostream>
#include <string>

namespace {

class TestClass : public testing::Test {
 protected:
  };
  
  void OSDTester( const char* imgname, const char* tessdatadir) {
    log.info() << tessdatadir << " for image: " << imgname << std::endl;
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    ASSERT_FALSE(api->Init(tessdatadir, "osd")) << "Could not initialize tesseract.";
    Pix *image = pixRead(imgname);
    ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
    api->SetImage(image);
    int orient_deg;
    float orient_conf;
    const char* script_name;
    float script_conf;
    bool detected = api->DetectOrientationScript(&orient_deg, &orient_conf, &script_name, &script_conf);
    ASSERT_FALSE(!detected) << "Failed to detect OSD.";
    printf("Orientation in degrees: %d\n"
         "Orientation confidence: %.2f\n"
         "Script: %s\n"
         "Script confidence: %.2f\n",
         orient_deg, orient_conf,
         script_name, script_conf);
    api->End();
    pixDestroy(&image);
  }

  class OSDTest : public TestClass ,
      public ::testing::WithParamInterface<std::tuple<const char*, const char*>> {};
  
  TEST_P(OSDTest, EngEuroHebrew) {
  OSDTester(std::get<0>(GetParam()), std::get<1>(GetParam()));
  }

  INSTANTIATE_TEST_CASE_P( OSDTestCase, OSDTest, 
                        ::testing::Combine(
                        ::testing::Values("../testing/phototest.tif" , "../testing/eurotext.tif" , "../testing/hebrew.png"),
                        ::testing::Values("../../tessdata_best/" , "../../tessdata_fast/" ,"../../tessdata/")));
 
}  // namespace
