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

// Unit test to run Tesseract instances in parallel threads and verify
// the OCR result.

// Note that success of running this test as-is does NOT verify
// thread-safety. For that, you need to run this binary under TSAN using the
// associated baseapi_thread_test_with_tsan.sh script.
//
// The tests are partitioned by instance to allow running Tesseract/Cube/both
// and by stage to run initialization/recognition/both. See flag descriptions
// for details.

#include <functional>
#include <memory>
#include <string>
#include <tensorflow/core/lib/core/threadpool.h>
#include "absl/strings/ascii.h"         // for absl::StripAsciiWhitespace
#include "allheaders.h"
#include "include_gunit.h"
#include <tesseract/baseapi.h>
#include "commandlineflags.h"
#include "log.h"

// Run with Tesseract instances.
BOOL_PARAM_FLAG(test_tesseract, true, "Test tesseract instances");
// Run with Cube instances.
// Note that with TSAN, Cube typically takes much longer to test. Ignoring
// std::string operations using the associated tess_tsan.ignore file when
// testing Cube significantly reduces testing time.
BOOL_PARAM_FLAG(test_cube, true, "Test Cube instances");

// When used with TSAN, having more repetitions can help in finding hidden
// thread-safety violations at the expense of increased testing time.
INT_PARAM_FLAG(reps, 1, "Num of parallel test repetitions to run.");

INT_PARAM_FLAG(max_concurrent_instances, 0,
             "Maximum number of instances to run in parallel at any given "
             "instant. The number of concurrent instances cannot exceed "
             "reps * number_of_langs_tested, which is also the default value.");

using tesseract::TessBaseAPI;

namespace {

static const char* kTessLangs[] = {"eng", "vie", nullptr};
static const char* kTessImages[] = {"HelloGoogle.tif", "viet.tif", nullptr};
static const char* kTessTruthText[] = {"Hello Google", "\x74\x69\xe1\xba\xbf\x6e\x67",
                                nullptr};

static const char* kCubeLangs[] = {"hin", "ara", nullptr};
static const char* kCubeImages[] = {"raaj.tif", "arabic.tif", nullptr};
static const char* kCubeTruthText[] = {
    "\xe0\xa4\xb0\xe0\xa4\xbe\xe0\xa4\x9c",
    "\xd8\xa7\xd9\x84\xd8\xb9\xd8\xb1\xd8\xa8\xd9\x8a", nullptr};

class BaseapiThreadTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    CHECK(FLAGS_test_tesseract || FLAGS_test_cube)
        << "Need to test at least one of Tesseract/Cube!";
    // Form a list of langs/gt_text/image_files we will work with.
    std::vector<std::string> image_files;
    if (FLAGS_test_tesseract) {
      int i = 0;
      while (kTessLangs[i] && kTessTruthText[i] && kTessImages[i]) {
        langs_.push_back(kTessLangs[i]);
        gt_text_.push_back(kTessTruthText[i]);
        image_files.push_back(kTessImages[i]);
        ++i;
      }
      LOG(INFO) << "Testing Tesseract on " << i << " languages.";
    }
    if (FLAGS_test_cube) {
      int i = 0;
      while (kCubeLangs[i] && kCubeTruthText[i] && kCubeImages[i]) {
        langs_.push_back(kCubeLangs[i]);
        gt_text_.push_back(kCubeTruthText[i]);
        image_files.push_back(kCubeImages[i]);
        ++i;
      }
      LOG(INFO) << "Testing Cube on " << i << " languages.";
    }
    num_langs_ = langs_.size();

    // Pre-load the images into an array. We will be making multiple copies of
    // an image here if FLAGS_reps > 1 and that is intentional. In this test, we
    // wish to not make any assumptions about the thread-safety of Pix objects,
    // and so entirely disallow concurrent access of a Pix instance.
    const int n = num_langs_ * FLAGS_reps;
    for (int i = 0; i < n; ++i) {
      std::string path = TESTING_DIR "/" + image_files[i % num_langs_];
      Pix* new_pix = pixRead(path.c_str());
      QCHECK(new_pix != nullptr) << "Could not read " << path;
      pix_.push_back(new_pix);
    }

    pool_size_ = (FLAGS_max_concurrent_instances < 1)
                     ? num_langs_ * FLAGS_reps
                     : FLAGS_max_concurrent_instances;
  }

  static void TearDownTestCase() {
    for (auto& pix : pix_) {
      pixDestroy(&pix);
    }
  }

  void ResetPool() {
    pool_.reset(new tensorflow::thread::ThreadPool(tensorflow::Env::Default(), "tessthread", pool_size_));
  }

  void WaitForPoolWorkers() { pool_.reset(nullptr); }

  std::unique_ptr<tensorflow::thread::ThreadPool> pool_;
  static int pool_size_;
  static std::vector<Pix*> pix_;
  static std::vector<std::string> langs_;
  static std::vector<std::string> gt_text_;
  static int num_langs_;
};

// static member variable declarations.
int BaseapiThreadTest::pool_size_;
std::vector<Pix*> BaseapiThreadTest::pix_;
std::vector<std::string> BaseapiThreadTest::langs_;
std::vector<std::string> BaseapiThreadTest::gt_text_;
int BaseapiThreadTest::num_langs_;

static void InitTessInstance(TessBaseAPI* tess, const std::string& lang) {
  CHECK(tess != nullptr);
  EXPECT_EQ(0, tess->Init(TESSDATA_DIR, lang.c_str()));
}

static void GetCleanedText(TessBaseAPI* tess, Pix* pix, std::string* ocr_text) {
  tess->SetImage(pix);
  char* result = tess->GetUTF8Text();
  *ocr_text = result;
  delete[] result;
  absl::StripAsciiWhitespace(ocr_text);
}

static void VerifyTextResult(TessBaseAPI* tess, Pix* pix, const std::string& lang,
                             const std::string& expected_text) {
  TessBaseAPI* tess_local = nullptr;
  if (tess) {
    tess_local = tess;
  } else {
    tess_local = new TessBaseAPI;
    InitTessInstance(tess_local, lang);
  }
  std::string ocr_text;
  GetCleanedText(tess_local, pix, &ocr_text);
  EXPECT_STREQ(expected_text.c_str(), ocr_text.c_str());
  if (tess_local != tess) delete tess_local;
}

// Check that Tesseract/Cube produce the correct results in single-threaded
// operation. If not, it is pointless to run the real multi-threaded tests.
TEST_F(BaseapiThreadTest, TestBasicSanity) {
  for (int i = 0; i < num_langs_; ++i) {
    TessBaseAPI tess;
    InitTessInstance(&tess, langs_[i]);
    std::string ocr_text;
    GetCleanedText(&tess, pix_[i], &ocr_text);
    CHECK(strcmp(gt_text_[i].c_str(), ocr_text.c_str()) == 0)
        << "Failed with lang = " << langs_[i];
  }
}

// Test concurrent instance initialization.
TEST_F(BaseapiThreadTest, TestInit) {
  const int n = num_langs_ * FLAGS_reps;
  ResetPool();
  std::vector<TessBaseAPI> tess(n);
  for (int i = 0; i < n; ++i) {
    pool_->Schedule(std::bind(InitTessInstance, &tess[i], langs_[i % num_langs_]));
  }
  WaitForPoolWorkers();
}

// Test concurrent recognition.
TEST_F(BaseapiThreadTest, TestRecognition) {
  const int n = num_langs_ * FLAGS_reps;
  std::vector<TessBaseAPI> tess(n);
  // Initialize api instances in a single thread.
  for (int i = 0; i < n; ++i) {
    InitTessInstance(&tess[i], langs_[i % num_langs_]);
  }

  ResetPool();
  for (int i = 0; i < n; ++i) {
    pool_->Schedule(std::bind(VerifyTextResult, &tess[i], pix_[i],
      langs_[i % num_langs_], gt_text_[i % num_langs_]));
  }
  WaitForPoolWorkers();
}

TEST_F(BaseapiThreadTest, TestAll) {
  const int n = num_langs_ * FLAGS_reps;
  ResetPool();
  for (int i = 0; i < n; ++i) {
    pool_->Schedule(std::bind(VerifyTextResult, nullptr, pix_[i],
      langs_[i % num_langs_], gt_text_[i % num_langs_]));
  }
  WaitForPoolWorkers();
}
}  // namespace
