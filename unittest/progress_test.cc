///////////////////////////////////////////////////////////////////////
// File:        apiexample_test.cc
// Description: Progress reporting API Test for Tesseract.
// Author:      Jaroslaw Kubik
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

#include <limits.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include "baseapi.h"
#include "gmock/gmock.h"
#include "include_gunit.h"
#include "leptonica/allheaders.h"
#include "ocrclass.h"

namespace {

class QuickTest : public testing::Test {
 protected:
  virtual void SetUp() { start_time_ = time(nullptr); }
  virtual void TearDown() {
    const time_t end_time = time(nullptr);
    EXPECT_TRUE(end_time - start_time_ <= 25)
        << "The test took too long - "
        << ::testing::PrintToString(end_time - start_time_);
  }
  time_t start_time_;
};

class ClassicMockProgressSink {
 public:
  MOCK_METHOD1(classicProgress, bool(int));
  MOCK_METHOD1(cancel, bool(int));

  ETEXT_DESC monitor;

  ClassicMockProgressSink() {
    monitor.progress_callback = [](int progress, int, int, int, int) -> bool {
      return instance->classicProgress(progress);
    };
    monitor.cancel = [](void* ths, int words) -> bool {
      return ((ClassicMockProgressSink*)ths)->cancel(words);
    };
    monitor.cancel_this = this;
    instance = this;
  }

  static ClassicMockProgressSink* instance;
};

ClassicMockProgressSink* ClassicMockProgressSink::instance = nullptr;

class NewMockProgressSink : public ClassicMockProgressSink {
 public:
  MOCK_METHOD1(progress, bool(int));

  NewMockProgressSink() {
    monitor.progress_callback2 = [](ETEXT_DESC* ths, int, int, int,
                                    int) -> bool {
      return ((NewMockProgressSink*)ths->cancel_this)->progress(ths->progress);
    };
  }
};

void ClassicProgressTester(const char* imgname, const char* tessdatadir,
                           const char* lang) {
  using ::testing::_;
  using ::testing::AllOf;
  using ::testing::AtLeast;
  using ::testing::DoAll;
  using ::testing::Gt;
  using ::testing::Le;
  using ::testing::Return;
  using ::testing::SaveArg;

  tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
  ASSERT_FALSE(api->Init(tessdatadir, lang))
      << "Could not initialize tesseract.";
  Pix* image = pixRead(imgname);
  ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
  api->SetImage(image);

  ClassicMockProgressSink progressSink;

  int currentProgress = -1;
  EXPECT_CALL(progressSink,
              classicProgress(AllOf(Gt<int&>(currentProgress), Le(100))))
      .Times(AtLeast(5))
      .WillRepeatedly(DoAll(SaveArg<0>(&currentProgress), Return(false)));
  EXPECT_CALL(progressSink, cancel(_))
      .Times(AtLeast(5))
      .WillRepeatedly(Return(false));

  EXPECT_EQ(api->Recognize(&progressSink.monitor), false);
  EXPECT_GE(currentProgress, 50) << "The reported progress did not reach 50%";

  api->End();
  pixDestroy(&image);
}

void NewProgressTester(const char* imgname, const char* tessdatadir,
                       const char* lang) {
  using ::testing::_;
  using ::testing::AllOf;
  using ::testing::AtLeast;
  using ::testing::DoAll;
  using ::testing::Gt;
  using ::testing::Le;
  using ::testing::Return;
  using ::testing::SaveArg;

  tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
  ASSERT_FALSE(api->Init(tessdatadir, lang))
      << "Could not initialize tesseract.";
  Pix* image = pixRead(imgname);
  ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
  api->SetImage(image);

  NewMockProgressSink progressSink;

  int currentProgress = -1;
  EXPECT_CALL(progressSink, classicProgress(_)).Times(0);
  EXPECT_CALL(progressSink, progress(AllOf(Gt<int&>(currentProgress), Le(100))))
      .Times(AtLeast(5))
      .WillRepeatedly(DoAll(SaveArg<0>(&currentProgress), Return(false)));
  EXPECT_CALL(progressSink, cancel(_))
      .Times(AtLeast(5))
      .WillRepeatedly(Return(false));

  EXPECT_EQ(api->Recognize(&progressSink.monitor), false);
  EXPECT_GE(currentProgress, 50) << "The reported progress did not reach 50%";

  api->End();
  pixDestroy(&image);
}

TEST(QuickTest, ClassicProgressReporitng) {
  ClassicProgressTester(TESTING_DIR "/phototest.tif", TESSDATA_DIR "_fast",
                        "eng");
}

TEST(QuickTest, NewProgressReporitng) {
  NewProgressTester(TESTING_DIR "/phototest.tif", TESSDATA_DIR "_fast", "eng");
}

}  // namespace
