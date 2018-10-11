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

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "allheaders.h"

#include "include_gunit.h"
#include "gmock/gmock-matchers.h"

#include "baseapi.h"
#include "cycletimer.h" // for CycleTimer
#include "log.h"        // for LOG
#include "ocrblock.h"   // for class BLOCK
#include "pageres.h"

namespace {

using ::testing::ContainsRegex;
using ::testing::HasSubstr;

static const char* langs[] = {"eng", "vie", "hin", "ara", nullptr};
static const char* image_files[] = {"HelloGoogle.tif", "viet.tif", "raaj.tif",
                             "arabic.tif", nullptr};
static const char* gt_text[] = {"Hello Google", "\x74\x69\xe1\xba\xbf\x6e\x67",
                         "\xe0\xa4\xb0\xe0\xa4\xbe\xe0\xa4\x9c",
                         "\xd8\xa7\xd9\x84\xd8\xb9\xd8\xb1\xd8\xa8\xd9\x8a",
                         nullptr};

class FriendlyTessBaseAPI : public tesseract::TessBaseAPI {
  FRIEND_TEST(TesseractTest, LSTMGeometryTest);
};

std::string GetCleanedTextResult(tesseract::TessBaseAPI* tess, Pix* pix) {
  tess->SetImage(pix);
  char* result = tess->GetUTF8Text();
  std::string ocr_result = result;
  delete[] result;
  absl::StripAsciiWhitespace(&ocr_result);
  return ocr_result;
}

// The fixture for testing Tesseract.
class TesseractTest : public testing::Test {
 protected:
  static std::string TestDataNameToPath(const std::string& name) {
    return file::JoinPath(TESTING_DIR, name);
  }
  static std::string TessdataPath() {
    return TESSDATA_DIR;
  }
};

// Tests that array sizes match their intended size.
TEST_F(TesseractTest, ArraySizeTest) {
  int size = 0;
  for (size = 0; kPolyBlockNames[size][0] != '\0'; ++size)
    ;
  EXPECT_EQ(size, PT_COUNT);
}

// Tests that Tesseract gets exactly the right answer on phototest.
TEST_F(TesseractTest, BasicTesseractTest) {
  tesseract::TessBaseAPI api;
  std::string truth_text;
  std::string ocr_text;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
  Pix* src_pix = pixRead(TestDataNameToPath("phototest.tif").c_str());
  CHECK(src_pix);
  ocr_text = GetCleanedTextResult(&api, src_pix);
  CHECK_OK(file::GetContents(TestDataNameToPath("phototest.gold.txt"),
                             &truth_text, file::Defaults()));
  absl::StripAsciiWhitespace(&truth_text);
  EXPECT_STREQ(truth_text.c_str(), ocr_text.c_str());
  pixDestroy(&src_pix);
}

// Test that api.GetComponentImages() will return a set of images for
// paragraphs even if text recognition was not run.
TEST_F(TesseractTest, IteratesParagraphsEvenIfNotDetected) {
  tesseract::TessBaseAPI api;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
  api.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
  api.SetVariable("paragraph_debug_level", "3");
#if 0 // TODO: b622.png is missing
  Pix* src_pix = pixRead(TestDataNameToPath("b622.png").c_str());
  CHECK(src_pix);
  api.SetImage(src_pix);
  Boxa* para_boxes =
      api.GetComponentImages(tesseract::RIL_PARA, true, nullptr, nullptr);
  EXPECT_TRUE(para_boxes != nullptr);
  Boxa* block_boxes =
      api.GetComponentImages(tesseract::RIL_BLOCK, true, nullptr, nullptr);
  EXPECT_TRUE(block_boxes != nullptr);
  // TODO(eger): Get paragraphs out of this page pre-text.
  EXPECT_GE(boxaGetCount(para_boxes), boxaGetCount(block_boxes));
  boxaDestroy(&block_boxes);
  boxaDestroy(&para_boxes);
  pixDestroy(&src_pix);
#endif
}

// We should get hOCR output and not seg fault, even if the api caller doesn't
// call SetInputName().
TEST_F(TesseractTest, HOCRWorksWithoutSetInputName) {
  tesseract::TessBaseAPI api;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
  Pix* src_pix = pixRead(TestDataNameToPath("HelloGoogle.tif").c_str());
  CHECK(src_pix);
  api.SetImage(src_pix);
  char* result = api.GetHOCRText(0);
  EXPECT_TRUE(result != nullptr);
  EXPECT_THAT(result, HasSubstr("Hello"));
  EXPECT_THAT(result, HasSubstr("<div class='ocr_page'"));
  delete[] result;
  pixDestroy(&src_pix);
}

// hOCR output should contain baseline info for upright textlines.
TEST_F(TesseractTest, HOCRContainsBaseline) {
  tesseract::TessBaseAPI api;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
  Pix* src_pix = pixRead(TestDataNameToPath("HelloGoogle.tif").c_str());
  CHECK(src_pix);
  api.SetInputName("HelloGoogle.tif");
  api.SetImage(src_pix);
  char* result = api.GetHOCRText(0);
  EXPECT_TRUE(result != nullptr);
  EXPECT_THAT(result, HasSubstr("Hello"));
  EXPECT_THAT(result, ContainsRegex("<span class='ocr_line'[^>]* "
                                    "baseline [-.0-9]+ [-.0-9]+"));
  delete[] result;
  pixDestroy(&src_pix);
}

// A provided document we once misread "RICK SNYDER" as "FUCK SNYDER"
// causing a bit of an embarrassment.  This was due to bad baseline fitting
// which has been addressed by both better baseline finding and by
// better algorithms to deal with baseline and xheight consistency.
TEST_F(TesseractTest, RickSnyderNotFuckSnyder) {
  tesseract::TessBaseAPI api;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
#if 0 // TODO: rick_snyder.jpeg is missing
  Pix* src_pix = pixRead(TestDataNameToPath("rick_snyder.jpeg").c_str());
  CHECK(src_pix);
  api.SetImage(src_pix);
  char* result = api.GetHOCRText(0);
  EXPECT_TRUE(result != nullptr);
  EXPECT_THAT(result, Not(HasSubstr("FUCK")));
  delete[] result;
  pixDestroy(&src_pix);
#endif
}

// Tests that Tesseract gets exactly the right answer on some page numbers.
TEST_F(TesseractTest, AdaptToWordStrTest) {
  static const char* kTrainingPages[] = {
      "136.tif", "256.tif", "410.tif", "432.tif", "540.tif",
      "692.tif", "779.tif", "793.tif", "808.tif", "815.tif",
      "12.tif",  "12.tif",  nullptr};
  static const char* kTrainingText[] = {
      "1 3 6", "2 5 6", "4 1 0", "4 3 2", "5 4 0", "6 9 2", "7 7 9",
      "7 9 3", "8 0 8", "8 1 5", "1 2",   "1 2",   nullptr};
  static const char* kTestPages[] = {"324.tif", "433.tif", "12.tif", nullptr};
  static const char* kTestText[] = {"324", "433", "12", nullptr};
  tesseract::TessBaseAPI api;
  std::string truth_text;
  std::string ocr_text;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
  api.SetVariable("matcher_sufficient_examples_for_prototyping", "1");
  api.SetVariable("classify_class_pruner_threshold", "220");
  // Train on the training text.
  for (int i = 0; kTrainingPages[i] != nullptr; ++i) {
    std::string image_file = TestDataNameToPath(kTrainingPages[i]);
    Pix* src_pix = pixRead(image_file.c_str());
    CHECK(src_pix);
    api.SetImage(src_pix);
    EXPECT_TRUE(
        api.AdaptToWordStr(tesseract::PSM_SINGLE_WORD, kTrainingText[i]))
        << "Failed to adapt to text \"" << kTrainingText[i] << "\" on image "
        << image_file;
    pixDestroy(&src_pix);
  }
  // Test the test text.
  api.SetVariable("tess_bn_matching", "1");
  api.SetPageSegMode(tesseract::PSM_SINGLE_WORD);
  for (int i = 0; kTestPages[i] != nullptr; ++i) {
    Pix* src_pix = pixRead(TestDataNameToPath(kTestPages[i]).c_str());
    CHECK(src_pix);
    ocr_text = GetCleanedTextResult(&api, src_pix);
    absl::StripAsciiWhitespace(&truth_text);
    EXPECT_STREQ(kTestText[i], ocr_text.c_str());
    pixDestroy(&src_pix);
  }
}

// Tests that LSTM gets exactly the right answer on phototest.
TEST_F(TesseractTest, BasicLSTMTest) {
  tesseract::TessBaseAPI api;
  std::string truth_text;
  std::string ocr_text;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_LSTM_ONLY);
  Pix* src_pix = pixRead(TestDataNameToPath("phototest_2.tif").c_str());
  CHECK(src_pix);
  ocr_text = GetCleanedTextResult(&api, src_pix);
  CHECK_OK(file::GetContents(TestDataNameToPath("phototest.gold.txt"),
                             &truth_text, file::Defaults()));
  absl::StripAsciiWhitespace(&truth_text);
  EXPECT_STREQ(truth_text.c_str(), ocr_text.c_str());
  pixDestroy(&src_pix);
}

// Test that LSTM's character bounding boxes are properly converted to
// Tesseract structures. Note that we can't guarantee that LSTM's
// character boxes fall completely within Tesseract's word box because
// the baseline denormalization/normalization transforms may introduce
// errors due to float/int conversions (e.g., see OUTLINE::move() in
// ccstruct/poutline.h) Instead, we do a loose check.
TEST_F(TesseractTest, LSTMGeometryTest) {
  Pix* src_pix = pixRead(TestDataNameToPath("deslant.tif").c_str());
  FriendlyTessBaseAPI api;
  api.Init(TessdataPath().c_str(), "eng", tesseract::OEM_LSTM_ONLY);
  api.SetImage(src_pix);
  ASSERT_EQ(api.Recognize(nullptr), 0);

  const PAGE_RES* page_res = api.GetPageRes();
  PAGE_RES_IT page_res_it(const_cast<PAGE_RES*>(page_res));
  page_res_it.restart_page();
  BLOCK* block = page_res_it.block()->block;
  CHECK(block);

  // extract word and character boxes for each word
  for (page_res_it.restart_page(); page_res_it.word() != nullptr;
       page_res_it.forward()) {
    WERD_RES* word = page_res_it.word();
    CHECK(word);
    CHECK(word->best_choice);
    CHECK_GT(word->best_choice->length(), 0);
    CHECK(word->word);
    CHECK(word->box_word);
    // tesseract's word box
    TBOX tess_blob_box;
    tess_blob_box = word->word->bounding_box();
    tess_blob_box.rotate(block->re_rotation());
    // verify that each of LSTM's character boxes lies close to within
    // tesseract's word box
    for (int i = 0; i < word->box_word->length(); ++i) {
      TBOX lstm_blob_box = word->box_word->BlobBox(i);
      // LSTM character box should not spill out of tesseract word box
      // by more than a few pixels in any direction
      EXPECT_LT(tess_blob_box.left() - lstm_blob_box.left(), 5);
      EXPECT_LT(lstm_blob_box.right() - tess_blob_box.right(), 5);
      EXPECT_LT(tess_blob_box.bottom() - lstm_blob_box.bottom(), 5);
      EXPECT_LT(lstm_blob_box.top() - tess_blob_box.top(), 5);
    }
  }
  pixDestroy(&src_pix);
}

TEST_F(TesseractTest, InitConfigOnlyTest) {
  // Languages for testing initialization.
  const char* langs[] = {"eng", "chi_tra", "jpn", "vie"};
  std::unique_ptr<tesseract::TessBaseAPI> api;
  CycleTimer timer;
  for (int i = 0; i < ARRAYSIZE(langs); ++i) {
    api.reset(new tesseract::TessBaseAPI);
    timer.Restart();
    EXPECT_EQ(0, api->Init(TessdataPath().c_str(), langs[i],
                           tesseract::OEM_TESSERACT_ONLY));
    timer.Stop();
    LOG(INFO) << "Lang " << langs[i] << " took " << timer.GetInMs()
              << "ms in regular init";
  }
  // Init variables to set for config-only initialization.
  GenericVector<STRING> vars_vec, vars_values;
  vars_vec.push_back(STRING("tessedit_init_config_only"));
  vars_values.push_back(STRING("1"));
  LOG(INFO) << "Switching to config only initialization:";
  for (int i = 0; i < ARRAYSIZE(langs); ++i) {
    api.reset(new tesseract::TessBaseAPI);
    timer.Restart();
    EXPECT_EQ(0, api->Init(TessdataPath().c_str(), langs[i],
                           tesseract::OEM_TESSERACT_ONLY, nullptr, 0, &vars_vec,
                           &vars_values, false));
    timer.Stop();
    LOG(INFO) << "Lang " << langs[i] << " took " << timer.GetInMs()
              << "ms in config-only init";
  }
}

// Tests if two instances of Tesseract/LSTM can co-exist in the same thread.
// NOTE: This is not an exhaustive test and current support for multiple
// instances in Tesseract is fragile. This test is intended largely as a means
// of detecting and guarding against the existing support being possibly broken
// by future CLs. TessBaseAPI instances are initialized using the default
// OEM_DEFAULT mode.
TEST(TesseractInstanceTest, TestMultipleTessInstances) {
  int num_langs = 0;
  while (langs[num_langs] != nullptr) ++num_langs;

  const std::string kTessdataPath = TESSDATA_DIR;

  // Preload images and verify that OCR is correct on them individually.
  std::vector<Pix*> pix(num_langs);
  for (int i = 0; i < num_langs; ++i) {
    SCOPED_TRACE(absl::StrCat("Single instance test with lang = ", langs[i]));
    std::string path = file::JoinPath(TESTING_DIR, image_files[i]);
    pix[i] = pixRead(path.c_str());
    QCHECK(pix[i] != nullptr) << "Could not read " << path;

    tesseract::TessBaseAPI tess;
    EXPECT_EQ(0, tess.Init(kTessdataPath.c_str(), langs[i]));
    std::string ocr_result = GetCleanedTextResult(&tess, pix[i]);
    EXPECT_STREQ(gt_text[i], ocr_result.c_str());
  }

  // Process the images in all pairwise combinations of associated languages.
  std::string ocr_result[2];
  for (int i = 0; i < num_langs; ++i) {
    for (int j = i + 1; j < num_langs; ++j) {
      tesseract::TessBaseAPI tess1, tess2;
      tess1.Init(kTessdataPath.c_str(), langs[i]);
      tess2.Init(kTessdataPath.c_str(), langs[j]);

      ocr_result[0] = GetCleanedTextResult(&tess1, pix[i]);
      ocr_result[1] = GetCleanedTextResult(&tess2, pix[j]);

      EXPECT_FALSE(strcmp(gt_text[i], ocr_result[0].c_str()) ||
                   strcmp(gt_text[j], ocr_result[1].c_str()))
          << "OCR failed on language pair " << langs[i] << "-" << langs[j];
    }
  }

  for (int i = 0; i < num_langs; ++i) pixDestroy(&pix[i]);
}

// Tests whether Tesseract parameters are correctly set for the two instances.
TEST(TesseractInstanceTest, TestMultipleTessInstanceVariables) {
  std::string illegal_name = "an_illegal_name";
  std::string langs[2] = {"eng", "hin"};
  std::string int_param_name = "tessedit_pageseg_mode";
  int int_param[2] = {1, 2};
  std::string int_param_str[2] = {"1", "2"};
  std::string bool_param_name = "tessedit_ambigs_training";
  bool bool_param[2] = {false, true};
  std::string bool_param_str[2] = {"F", "T"};
  std::string str_param_name = "tessedit_char_blacklist";
  std::string str_param[2] = {"abc", "def"};
  std::string double_param_name = "segment_penalty_dict_frequent_word";
  std::string double_param_str[2] = {"0.01", "2"};
  double double_param[2] = {0.01, 2};

  const std::string kTessdataPath = TESSDATA_DIR;

  tesseract::TessBaseAPI tess1, tess2;
  for (int i = 0; i < 2; ++i) {
    tesseract::TessBaseAPI* api = (i == 0) ? &tess1 : &tess2;
    api->Init(kTessdataPath.c_str(), langs[i].c_str());
    api->SetVariable(illegal_name.c_str(), "none");
    api->SetVariable(int_param_name.c_str(), int_param_str[i].c_str());
    api->SetVariable(bool_param_name.c_str(), bool_param_str[i].c_str());
    api->SetVariable(str_param_name.c_str(), str_param[i].c_str());
    api->SetVariable(double_param_name.c_str(), double_param_str[i].c_str());
  }
  for (int i = 0; i < 2; ++i) {
    tesseract::TessBaseAPI* api = (i == 0) ? &tess1 : &tess2;
    EXPECT_FALSE(api->GetStringVariable(illegal_name.c_str()));
    int intvar;
    EXPECT_TRUE(api->GetIntVariable(int_param_name.c_str(), &intvar));
    EXPECT_EQ(int_param[i], intvar);
    bool boolvar;
    EXPECT_TRUE(api->GetBoolVariable(bool_param_name.c_str(), &boolvar));
    EXPECT_EQ(bool_param[i], boolvar);
    EXPECT_STREQ(str_param[i].c_str(),
                 api->GetStringVariable(str_param_name.c_str()));
    double doublevar;
    EXPECT_TRUE(api->GetDoubleVariable(double_param_name.c_str(), &doublevar));
    EXPECT_EQ(double_param[i], doublevar);
  }
}

}  // namespace
