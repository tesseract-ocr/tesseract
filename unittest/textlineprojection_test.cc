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

#include <allheaders.h>
#include <string>                       // for std::string

#include "absl/strings/str_format.h"    // for absl::StrFormat
#include "include_gunit.h"

#include <tesseract/baseapi.h>
#include "colfind.h"
#include "log.h"                        // for LOG
#include "mutableiterator.h"
#include <tesseract/osdetect.h>
#include "pageres.h"
#include "tesseractclass.h"
#include "textlineprojection.h"

namespace {

using tesseract::ColumnFinder;
using tesseract::MutableIterator;
using tesseract::Tesseract;
using tesseract::TextlineProjection;

// Minimum score for a STRONG_CHAIN textline.
// NOTE: Keep in sync with textlineprojection.cc.
const int kMinStrongTextValue = 6;

// The fixture for testing Tesseract.
class TextlineProjectionTest : public testing::Test {
 protected:
  std::string OutputNameToPath(const std::string& name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }

  TextlineProjectionTest() {
    src_pix_ = nullptr;
    bin_pix_ = nullptr;
    tesseract_ = nullptr;
    finder_ = nullptr;
    denorm_ = nullptr;
    projection_ = nullptr;
  }
  virtual ~TextlineProjectionTest() {
    pixDestroy(&src_pix_);
    pixDestroy(&bin_pix_);
    delete finder_;
    delete tesseract_;
  }

  void SetImage(const char* filename) {
    pixDestroy(&src_pix_);
    src_pix_ = pixRead(file::JoinPath(TESTING_DIR, filename).c_str());
    api_.Init(TESSDATA_DIR, "eng", tesseract::OEM_TESSERACT_ONLY);
    api_.SetPageSegMode(tesseract::PSM_AUTO_OSD);
    api_.SetImage(src_pix_);
  }

  // Ugly hacked-together function sets up projection_ and denorm_ by setting
  // up for auto pagelayout, setting up a ColumnFinder, running it, and
  // using accessors to get at the internal denorm and projection.
  // If the coordinates have been rotated, the denorm should match
  // correctly and transform coordinates back to the projection.
  // We throw away all the blocks, blobs etc, and test the projection with
  // the resultiterator from a separate BaseAPI run.
  void SetupProjection() {
    tesseract::TessdataManager mgr;
    Tesseract* osd_tess = new Tesseract;
    OSResults osr;
    EXPECT_EQ(osd_tess->init_tesseract(TESSDATA_DIR, nullptr, "osd",
                                       tesseract::OEM_TESSERACT_ONLY, nullptr, 0,
                                       nullptr, nullptr, false, &mgr),
              0);
    tesseract_ = new Tesseract;
    EXPECT_EQ(tesseract_->init_tesseract(TESSDATA_DIR, nullptr, "eng",
                                         tesseract::OEM_TESSERACT_ONLY, nullptr, 0,
                                         nullptr, nullptr, false, &mgr),
              0);
    bin_pix_ = api_.GetThresholdedImage();
    *tesseract_->mutable_pix_binary() = pixClone(bin_pix_);
    osd_tess->set_source_resolution(api_.tesseract()->source_resolution());
    tesseract_->set_source_resolution(api_.tesseract()->source_resolution());
    int width = pixGetWidth(bin_pix_);
    int height = pixGetHeight(bin_pix_);
    // First make a single block covering the whole image.
    BLOCK* block = new BLOCK("", true, 0, 0, 0, 0, width, height);
    block->set_right_to_left(false);
    BLOCK_LIST src_blocks;
    BLOCK_IT block_it(&src_blocks);
    block_it.add_to_end(block);
    Pix* photomask_pix = nullptr;
    // The blocks made by the ColumnFinder. Moved to blocks before return.
    BLOCK_LIST found_blocks;
    TO_BLOCK_LIST temp_blocks;
    finder_ = tesseract_->SetupPageSegAndDetectOrientation(
        tesseract::PSM_AUTO_OSD, &src_blocks, osd_tess, &osr, &temp_blocks,
        &photomask_pix, nullptr);
    TO_BLOCK_IT to_block_it(&temp_blocks);
    TO_BLOCK* to_block = to_block_it.data();
    denorm_ = finder_->denorm();
    TO_BLOCK_LIST to_blocks;
    BLOBNBOX_LIST diacritic_blobs;
    EXPECT_GE(finder_->FindBlocks(tesseract::PSM_AUTO, nullptr, 1, to_block,
                                  photomask_pix, nullptr, nullptr, nullptr,
                                  &found_blocks, &diacritic_blobs, &to_blocks),
              0);
    projection_ = finder_->projection();
    pixDestroy(&photomask_pix);
    delete osd_tess;
  }

  // Helper evaluates the given box, expects the result to be greater_than
  // or !greater_than the target_value and provides diagnostics if not.
  void EvaluateBox(const TBOX& box, bool greater_or_equal, int target_value,
                   const char* text, const char* message) {
    int value = projection_->EvaluateBox(box, denorm_, false);
    if (greater_or_equal != (value > target_value)) {
      LOG(INFO) << absl::StrFormat(
          "EvaluateBox too %s:%d vs %d for %s word '%s' at:",
          greater_or_equal ? "low" : "high", value, target_value, message,
          text);
      box.print();
      value = projection_->EvaluateBox(box, denorm_, true);
    } else {
      LOG(INFO) << absl::StrFormat("EvaluateBox OK(%d) for %s word '%s'",
                                   value, message, text);
    }
    if (greater_or_equal) {
      EXPECT_GE(value, target_value);
    } else {
      EXPECT_LT(value, target_value);
    }
  }

  // Helper evaluates the DistanceOfBoxFromBox function by expecting that
  // box should be nearer to true_box than false_box.
  void EvaluateDistance(const TBOX& box, const TBOX& true_box,
                        const TBOX& false_box, const char* text,
                        const char* message) {
    int true_dist =
        projection_->DistanceOfBoxFromBox(box, true_box, true, denorm_, false);
    int false_dist =
        projection_->DistanceOfBoxFromBox(box, false_box, true, denorm_, false);
    if (false_dist <= true_dist) {
      LOG(INFO) << absl::StrFormat(
        "Distance wrong:%d vs %d for %s word '%s' at:",
        false_dist, true_dist, message, text);
      true_box.print();
      projection_->DistanceOfBoxFromBox(box, true_box, true, denorm_, true);
      projection_->DistanceOfBoxFromBox(box, false_box, true, denorm_, true);
    } else {
      LOG(INFO) << absl::StrFormat("Distance OK(%d vs %d) for %s word '%s'",
                                   false_dist, true_dist, message, text);
    }
  }

  // Tests the projection on the word boxes of the given image.
  // line_height is the cap + descender size of the text.
  void VerifyBoxes(const char* imagefile, int line_height) {
    SetImage(imagefile);
    api_.Recognize(nullptr);
    SetupProjection();
    MutableIterator* it = api_.GetMutableIterator();
    do {
      char* text = it->GetUTF8Text(tesseract::RIL_WORD);
      const PAGE_RES_IT* pr_it = it->PageResIt();
      WERD_RES* word = pr_it->word();
      // The word_box refers to the internal, possibly rotated, coords.
      TBOX word_box = word->word->bounding_box();
      bool small_word = word_box.height() * 1.5 < line_height;
      bool tall_word = word_box.height() * 1.125 > line_height;
      // We pad small and tall words differently because ascenders and
      // descenders affect the position and size of the upper/lower boxes.
      int padding;
      if (small_word) {
        padding = word_box.height();
      } else if (tall_word) {
        padding = word_box.height() / 3;
      } else {
        padding = word_box.height() / 2;
      }
      // Test that the word box gets a good score.
      EvaluateBox(word_box, true, kMinStrongTextValue, text, "Real Word");

      // Now test a displaced box, both above and below the word.
      TBOX upper_box(word_box);
      upper_box.set_bottom(word_box.top());
      upper_box.set_top(word_box.top() + padding);
      EvaluateBox(upper_box, false, kMinStrongTextValue, text, "Upper Word");
      EvaluateBox(upper_box, true, -1, text, "Upper Word not vertical");
      TBOX lower_box = word_box;
      lower_box.set_top(word_box.bottom());
      lower_box.set_bottom(word_box.bottom() - padding);
      if (tall_word) lower_box.move(ICOORD(0, padding / 2));
      EvaluateBox(lower_box, false, kMinStrongTextValue, text, "Lower Word");
      EvaluateBox(lower_box, true, -1, text, "Lower Word not vertical");

      // Since some words have no text below and some words have no text above
      // check that at least one of the boxes satisfies BoxOutOfTextline.
      bool upper_or_lower_out_of_textline =
          projection_->BoxOutOfHTextline(upper_box, denorm_, false) ||
          projection_->BoxOutOfHTextline(lower_box, denorm_, false);
      if (!upper_or_lower_out_of_textline) {
        projection_->BoxOutOfHTextline(upper_box, denorm_, true);
        projection_->BoxOutOfHTextline(lower_box, denorm_, true);
      }
      EXPECT_TRUE(upper_or_lower_out_of_textline);

      // Now test DistanceOfBoxFromBox by faking a challenger word, and asking
      // that each pad box be nearer to its true textline than the
      // challenger. Due to the tight spacing of latin text, getting
      // the right position and size of these test boxes is quite fiddly.
      padding = line_height / 4;
      upper_box.set_top(upper_box.bottom() + padding);
      TBOX target_box(word_box);
      if (!small_word) {
        upper_box.move(ICOORD(0, -padding * 3 / 2));
      }
      target_box.set_top(upper_box.bottom());
      TBOX upper_challenger(upper_box);
      upper_challenger.set_bottom(upper_box.top());
      upper_challenger.set_top(upper_box.top() + word_box.height());
      EvaluateDistance(upper_box, target_box, upper_challenger, text,
                       "Upper Word");
      if (tall_word) lower_box.move(ICOORD(0, padding / 2));
      lower_box.set_bottom(lower_box.top() - padding);
      target_box = word_box;
      target_box.set_bottom(lower_box.top());
      TBOX lower_challenger(lower_box);
      lower_challenger.set_top(lower_box.bottom());
      lower_challenger.set_bottom(lower_box.bottom() - word_box.height());
      EvaluateDistance(lower_box, target_box, lower_challenger, text,
                       "Lower Word");

      delete[] text;
    } while (it->Next(tesseract::RIL_WORD));
    delete it;
  }

  Pix* src_pix_;
  Pix* bin_pix_;
  BLOCK_LIST blocks_;
  std::string ocr_text_;
  tesseract::TessBaseAPI api_;
  Tesseract* tesseract_;
  ColumnFinder* finder_;
  const DENORM* denorm_;
  const TextlineProjection* projection_;
};

// Tests all word boxes on an unrotated image.
TEST_F(TextlineProjectionTest, Unrotated) { VerifyBoxes("phototest.tif", 31); }

// Tests character-level applyboxes on italic Times New Roman.
TEST_F(TextlineProjectionTest, Rotated) { VerifyBoxes("phototestrot.tif", 31); }

}  // namespace
