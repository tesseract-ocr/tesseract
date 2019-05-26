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

#include <string>
#include "allheaders.h"
#include "baseapi.h"
#include "boxread.h"
#include "rect.h"
#include "resultiterator.h"

#include "include_gunit.h"

namespace {

using tesseract::ResultIterator;

const char* kTruthTextWords = "To simple burn running of goods lately.\n";
const char* kTruthTextLine = "Tosimpleburnrunningofgoodslately.\n";

// The fixture for testing Tesseract.
class ApplyBoxTest : public testing::Test {
 protected:
  std::string TestDataNameToPath(const std::string& name) {
    return file::JoinPath(TESTING_DIR, name);
  }
  std::string TessdataPath() { return TESSDATA_DIR; }

  ApplyBoxTest() { src_pix_ = nullptr; }
  ~ApplyBoxTest() { pixDestroy(&src_pix_); }

  bool SetImage(const char* filename) {
    bool found = false;
    pixDestroy(&src_pix_);
    src_pix_ = pixRead(TestDataNameToPath(filename).c_str());
    if (api_.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY) != -1) {
      api_.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
      api_.SetImage(src_pix_);
      api_.SetVariable("tessedit_make_boxes_from_boxes", "1");
      api_.SetInputName(TestDataNameToPath(filename).c_str());
      found = true;
    }
    return found;
  }

  // Runs ApplyBoxes (via setting the appropriate variables and Recognize)
  // and checks that the output ocr text matches the truth_str, and that
  // the boxes match the given box file well enough.
  // If line_mode is true, ApplyBoxes is run in line segmentation mode,
  // otherwise the input box file is assumed to have character-level boxes.
  void VerifyBoxesAndText(const char* imagefile, const char* truth_str,
                          const char* target_box_file, bool line_mode) {
    if (!SetImage(imagefile)) {
      // eng.traineddata not found or other problem during Init.
      GTEST_SKIP();
      return;
    }
    if (line_mode)
      api_.SetVariable("tessedit_resegment_from_line_boxes", "1");
    else
      api_.SetVariable("tessedit_resegment_from_boxes", "1");
    api_.Recognize(nullptr);
    char* ocr_text = api_.GetUTF8Text();
    EXPECT_STREQ(truth_str, ocr_text);
    delete[] ocr_text;
    // Test the boxes by reading the target box file in parallel with the
    // bounding boxes in the ocr output.
    std::string box_filename = TestDataNameToPath(target_box_file);
    FILE* box_file = OpenBoxFile(STRING(box_filename.c_str()));
    ASSERT_TRUE(box_file != nullptr);
    int height = pixGetHeight(src_pix_);
    ResultIterator* it = api_.GetIterator();
    do {
      int left, top, right, bottom;
      EXPECT_TRUE(
          it->BoundingBox(tesseract::RIL_SYMBOL, &left, &top, &right, &bottom));
      TBOX ocr_box(ICOORD(left, height - bottom), ICOORD(right, height - top));
      int line_number = 0;
      TBOX truth_box;
      STRING box_text;
      EXPECT_TRUE(
          ReadNextBox(0, &line_number, box_file, &box_text, &truth_box));
      // Testing for major overlap is a bit weak, but if they all
      // major overlap successfully, then it has to be fairly close.
      EXPECT_TRUE(ocr_box.major_overlap(truth_box));
      // Also check that the symbol text matches the box text.
      char* symbol_text = it->GetUTF8Text(tesseract::RIL_SYMBOL);
      EXPECT_STREQ(box_text.string(), symbol_text);
      delete[] symbol_text;
    } while (it->Next(tesseract::RIL_SYMBOL));
    delete it;
  }

  Pix* src_pix_;
  std::string ocr_text_;
  tesseract::TessBaseAPI api_;
};

// Tests character-level applyboxes on normal Times New Roman.
TEST_F(ApplyBoxTest, TimesCharLevel) {
  VerifyBoxesAndText("trainingtimes.tif", kTruthTextWords, "trainingtimes.box",
                     false);
}

// Tests character-level applyboxes on italic Times New Roman.
TEST_F(ApplyBoxTest, ItalicCharLevel) {
  VerifyBoxesAndText("trainingital.tif", kTruthTextWords, "trainingital.box",
                     false);
}

// Tests line-level applyboxes on normal Times New Roman.
TEST_F(ApplyBoxTest, TimesLineLevel) {
  VerifyBoxesAndText("trainingtimesline.tif", kTruthTextLine,
                     "trainingtimes.box", true);
}

// Tests line-level applyboxes on italic Times New Roman.
TEST_F(ApplyBoxTest, ItalLineLevel) {
  VerifyBoxesAndText("trainingitalline.tif", kTruthTextLine, "trainingital.box",
                     true);
}

}  // namespace
