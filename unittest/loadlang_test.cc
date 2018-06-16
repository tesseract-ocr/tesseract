///////////////////////////////////////////////////////////////////////
// File:        loadlang_test.cc
// Description: Load All languages for Tesseract using text fixtures and parameters.
// Author:      Shree Devi Kumar
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
#include "include_gunit.h"
#include "baseapi.h"
#include "leptonica/allheaders.h"
#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <limits.h>
#include <time.h>

namespace {

class QuickTest : public testing::Test {
 protected:
  virtual void SetUp() {
    start_time_ = time(nullptr);
  }
  virtual void TearDown() {
    const time_t end_time = time(nullptr);
    EXPECT_TRUE(end_time - start_time_ <=25) << "The test took too long - " << ::testing::PrintToString(end_time - start_time_);
  }
  time_t start_time_;
  };

  void LangLoader(const char* tessdatadir, const char* lang) {
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    ASSERT_FALSE(api->Init(tessdatadir, lang)) << "Could not initialize tesseract for Language $lang.";
    api->End();
  }

  class LoadLanguage : public QuickTest ,
      public ::testing::WithParamInterface<const char*> {
  };
  
  TEST_P(LoadLanguage, LoadFast) {
    LangLoader(TESSDATA_DIR "_fast", GetParam());
  }
  TEST_P(LoadLanguage, LoadBest) {
    LangLoader(TESSDATA_DIR "_best", GetParam());
  } 
  TEST_P(LoadLanguage, LoadTess) {
    LangLoader(TESSDATA_DIR , GetParam());
  }   

  INSTANTIATE_TEST_CASE_P( AllScripts, LoadLanguage, 
                        ::testing::Values("script/Latin", "script/Devanagari", "script/Arabic") );

  class EuroText : public QuickTest {
  };
  
  TEST_F(EuroText, FastData) {
    LangLoader(TESSDATA_DIR "_fast", "script/Latin");
  }
  TEST_F(EuroText, BestData) {
    LangLoader(TESSDATA_DIR "_best", "script/Latin");
  }
  TEST_F(EuroText, TessData) {
    LangLoader(TESSDATA_DIR , "script/Latin");
  }  
}  // namespace
