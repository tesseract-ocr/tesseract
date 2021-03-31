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

#include "include_gunit.h"

#include "colpartitiongrid.h"
#include "equationdetect.h"
#include "tesseractclass.h"

#include <allheaders.h>

#include <memory>
#include <string>
#include <utility>

#define ENABLE_IdentifySpecialText_TEST 0
#if ENABLE_IdentifySpecialText_TEST
#  define EQU_TRAINEDDATA_NAME "equ"
#else
#  define EQU_TRAINEDDATA_NAME "equINTENTIONALLY_MISSING_FILE"
#endif

namespace tesseract {

class TestableEquationDetect : public EquationDetect {
public:
  TestableEquationDetect(const char *tessdata, Tesseract *lang_tesseract)
      : EquationDetect(tessdata, EQU_TRAINEDDATA_NAME) {
    SetLangTesseract(lang_tesseract);
  }

  // Insert a certain math and digit blobs into part.
  void AddMathDigitBlobs(const int math_blobs, const int digit_blobs, const int total_blobs,
                         ColPartition *part) {
    CHECK(part != nullptr);
    CHECK_LE(math_blobs + digit_blobs, total_blobs);
    int count = 0;
    for (int i = 0; i < math_blobs; i++, count++) {
      auto *blob = new BLOBNBOX();
      blob->set_special_text_type(BSTT_MATH);
      part->AddBox(blob);
    }
    for (int i = 0; i < digit_blobs; i++, count++) {
      auto *blob = new BLOBNBOX();
      blob->set_special_text_type(BSTT_DIGIT);
      part->AddBox(blob);
    }
    for (int i = count; i < total_blobs; i++) {
      auto *blob = new BLOBNBOX();
      blob->set_special_text_type(BSTT_NONE);
      part->AddBox(blob);
    }
  }

  // Set up pix_binary for lang_tesseract_.
  void SetPixBinary(Image pix) {
    CHECK_EQ(1, pixGetDepth(pix));
    *(lang_tesseract_->mutable_pix_binary()) = pix;
  }

  void RunIdentifySpecialText(BLOBNBOX *blob, const int height_th) {
    IdentifySpecialText(blob, height_th);
  }

  BlobSpecialTextType RunEstimateTypeForUnichar(const char *val) {
    const UNICHARSET &unicharset = lang_tesseract_->unicharset;
    return EstimateTypeForUnichar(unicharset, unicharset.unichar_to_id(val));
  }

  EquationDetect::IndentType RunIsIndented(ColPartitionGrid *part_grid, ColPartition *part) {
    this->part_grid_ = part_grid;
    return IsIndented(part);
  }

  bool RunIsNearSmallNeighbor(const TBOX &seed_box, const TBOX &part_box) {
    return IsNearSmallNeighbor(seed_box, part_box);
  }

  bool RunCheckSeedBlobsCount(ColPartition *part) {
    return CheckSeedBlobsCount(part);
  }

  float RunComputeForegroundDensity(const TBOX &tbox) {
    return ComputeForegroundDensity(tbox);
  }

  int RunCountAlignment(const std::vector<int> &sorted_vec, const int val) {
    return CountAlignment(sorted_vec, val);
  }

  void RunSplitCPHorLite(ColPartition *part, std::vector<TBOX> *splitted_boxes) {
    SplitCPHorLite(part, splitted_boxes);
  }

  void RunSplitCPHor(ColPartition *part, std::vector<ColPartition *> *parts_splitted) {
    SplitCPHor(part, parts_splitted);
  }

  void TestComputeCPsSuperBBox(const TBOX &box, ColPartitionGrid *part_grid) {
    CHECK(part_grid != nullptr);
    part_grid_ = part_grid;
    ComputeCPsSuperBBox();
    EXPECT_TRUE(*cps_super_bbox_ == box);
  }
};

class EquationFinderTest : public testing::Test {
protected:
  std::unique_ptr<TestableEquationDetect> equation_det_;
  std::unique_ptr<Tesseract> tesseract_;

  // The directory for testdata;
  std::string testdata_dir_;

  void SetUp() override {
    std::locale::global(std::locale(""));
    tesseract_ = std::make_unique<Tesseract>();
    tesseract_->init_tesseract(TESSDATA_DIR, "eng", OEM_TESSERACT_ONLY);
    tesseract_->set_source_resolution(300);
    equation_det_ = std::make_unique<TestableEquationDetect>(TESSDATA_DIR, tesseract_.get());
    equation_det_->SetResolution(300);

    testdata_dir_ = TESTDATA_DIR;
  }

  void TearDown() override {
    tesseract_.reset(nullptr);
    equation_det_.reset(nullptr);
  }

  // Add a BLOCK covering the whole page.
  void AddPageBlock(Image pix, BLOCK_LIST *blocks) {
    CHECK(pix != nullptr);
    CHECK(blocks != nullptr);
    BLOCK_IT block_it(blocks);
    auto *block = new BLOCK("", true, 0, 0, 0, 0, pixGetWidth(pix), pixGetHeight(pix));
    block_it.add_to_end(block);
  }

  // Create col partitions, add into part_grid, and put them into all_parts.
  void CreateColParts(const int rows, const int cols, ColPartitionGrid *part_grid,
                      std::vector<ColPartition *> *all_parts) {
    const int kWidth = 10, kHeight = 10;
    ClearParts(all_parts);
    for (int y = 0; y < rows; ++y) {
      for (int x = 0; x < cols; ++x) {
        int left = x * kWidth * 2, bottom = y * kHeight * 2;
        TBOX box(left, bottom, left + kWidth, bottom + kHeight);
        ColPartition *part = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
        part_grid->InsertBBox(true, true, part);
        all_parts->push_back(part);
      }
    }
  }

  void ClearParts(std::vector<ColPartition *> *all_parts) {
    for (auto &all_part : *all_parts) {
      all_part->DeleteBoxes();
      delete all_part;
    }
  }

  // Create a BLOBNBOX object with bounding box tbox, and add it into part.
  void AddBlobIntoPart(const TBOX &tbox, ColPartition *part) {
    CHECK(part != nullptr);
    auto *blob = new BLOBNBOX();
    blob->set_bounding_box(tbox);
    part->AddBox(blob);
  }
};

TEST_F(EquationFinderTest, IdentifySpecialText) {
#if !ENABLE_IdentifySpecialText_TEST
  GTEST_SKIP();
#else // TODO: missing equ_gt1.tif
  // Load Image.
  std::string imagefile = file::JoinPath(testdata_dir_, "equ_gt1.tif");
  Image pix_binary = pixRead(imagefile.c_str());
  CHECK(pix_binary != nullptr && pixGetDepth(pix_binary) == 1);

  // Get components.
  BLOCK_LIST blocks;
  TO_BLOCK_LIST to_blocks;
  AddPageBlock(pix_binary, &blocks);
  Textord *textord = tesseract_->mutable_textord();
  textord->find_components(pix_binary, &blocks, &to_blocks);

  // Identify special texts from to_blocks.
  TO_BLOCK_IT to_block_it(&to_blocks);
  std::map<int, int> stt_count;
  for (to_block_it.mark_cycle_pt(); !to_block_it.cycled_list(); to_block_it.forward()) {
    TO_BLOCK *to_block = to_block_it.data();
    BLOBNBOX_IT blob_it(&(to_block->blobs));
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      BLOBNBOX *blob = blob_it.data();
      // blob->set_special_text_type(BSTT_NONE);
      equation_det_->RunIdentifySpecialText(blob, 0);
      tensorflow::gtl::InsertIfNotPresent(&stt_count, blob->special_text_type(), 0);
      stt_count[blob->special_text_type()]++;
    }
  }

  // Verify the number, but allow a range of +/- kCountRange before squealing.
  const int kCountRange = 3;
  EXPECT_GE(39 + kCountRange, stt_count[BSTT_NONE]);
  EXPECT_LE(39 - kCountRange, stt_count[BSTT_NONE]);

  // if you count all the subscripts etc, there are ~45 italic chars.
  EXPECT_GE(45 + kCountRange, stt_count[BSTT_ITALIC]);
  EXPECT_LE(45 - kCountRange, stt_count[BSTT_ITALIC]);
  EXPECT_GE(41 + kCountRange, stt_count[BSTT_DIGIT]);
  EXPECT_LE(41 - kCountRange, stt_count[BSTT_DIGIT]);
  EXPECT_GE(50 + kCountRange, stt_count[BSTT_MATH]);
  EXPECT_LE(50 - kCountRange, stt_count[BSTT_MATH]);
  EXPECT_GE(10 + kCountRange, stt_count[BSTT_UNCLEAR]);
  EXPECT_LE(10 - kCountRange, stt_count[BSTT_UNCLEAR]);

  // Release memory.
  pix_binary.destroy();
#endif
}

TEST_F(EquationFinderTest, EstimateTypeForUnichar) {
  // Test abc characters.
  EXPECT_EQ(BSTT_NONE, equation_det_->RunEstimateTypeForUnichar("a"));
  EXPECT_EQ(BSTT_NONE, equation_det_->RunEstimateTypeForUnichar("c"));

  // Test punctuation characters.
  EXPECT_EQ(BSTT_NONE, equation_det_->RunEstimateTypeForUnichar("'"));
  EXPECT_EQ(BSTT_NONE, equation_det_->RunEstimateTypeForUnichar(","));

  // Test digits.
  EXPECT_EQ(BSTT_DIGIT, equation_det_->RunEstimateTypeForUnichar("1"));
  EXPECT_EQ(BSTT_DIGIT, equation_det_->RunEstimateTypeForUnichar("4"));
  EXPECT_EQ(BSTT_DIGIT, equation_det_->RunEstimateTypeForUnichar("|"));

  // Test math symbols.
  EXPECT_EQ(BSTT_MATH, equation_det_->RunEstimateTypeForUnichar("("));
  EXPECT_EQ(BSTT_MATH, equation_det_->RunEstimateTypeForUnichar("+"));
}

TEST_F(EquationFinderTest, IsIndented) {
  ColPartitionGrid part_grid(10, ICOORD(0, 0), ICOORD(1000, 1000));

  // Create five ColPartitions:
  // part 1: ************
  // part 2:   *********
  // part 3: *******
  // part 4:   *****
  //
  // part 5:   ********
  TBOX box1(0, 950, 999, 999);
  ColPartition *part1 = ColPartition::FakePartition(box1, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part_grid.InsertBBox(true, true, part1);
  TBOX box2(300, 920, 900, 940);
  ColPartition *part2 = ColPartition::FakePartition(box2, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part_grid.InsertBBox(true, true, part2);
  TBOX box3(0, 900, 600, 910);
  ColPartition *part3 = ColPartition::FakePartition(box3, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part_grid.InsertBBox(true, true, part3);
  TBOX box4(300, 890, 600, 899);
  ColPartition *part4 = ColPartition::FakePartition(box4, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part_grid.InsertBBox(true, true, part4);
  TBOX box5(300, 500, 900, 510);
  ColPartition *part5 = ColPartition::FakePartition(box5, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part_grid.InsertBBox(true, true, part5);

  // Test
  // part1 should be no indent.
  EXPECT_EQ(EquationDetect::NO_INDENT, equation_det_->RunIsIndented(&part_grid, part1));
  // part2 should be left indent in terms of part1.
  EXPECT_EQ(EquationDetect::LEFT_INDENT, equation_det_->RunIsIndented(&part_grid, part2));
  // part3 should be right indent.
  EXPECT_EQ(EquationDetect::RIGHT_INDENT, equation_det_->RunIsIndented(&part_grid, part3));
  // part4 should be both indented.
  EXPECT_EQ(EquationDetect::BOTH_INDENT, equation_det_->RunIsIndented(&part_grid, part4));
  // part5 should be no indent because it is too far from part1.
  EXPECT_EQ(EquationDetect::NO_INDENT, equation_det_->RunIsIndented(&part_grid, part5));

  // Release memory.
  part1->DeleteBoxes();
  delete (part1);
  part2->DeleteBoxes();
  delete (part2);
  part3->DeleteBoxes();
  delete (part3);
  part4->DeleteBoxes();
  delete (part4);
  part5->DeleteBoxes();
  delete (part5);
}

TEST_F(EquationFinderTest, IsNearSmallNeighbor) {
  // Create four tboxes:
  //          part 1, part 2
  //           *****   *****
  // part 3:   *****
  //
  // part 4: *****************
  TBOX box1(0, 950, 499, 999);
  TBOX box2(500, 950, 999, 998);
  TBOX box3(0, 900, 499, 949);
  TBOX box4(0, 550, 499, 590);

  // Test
  // box2 should be box1's near neighbor but not vice versa.
  EXPECT_TRUE(equation_det_->RunIsNearSmallNeighbor(box1, box2));
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box2, box1));
  // box1 and box3 should be near neighbors of each other.
  EXPECT_TRUE(equation_det_->RunIsNearSmallNeighbor(box1, box3));
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box2, box3));
  // box2 and box3 should not be near neighbors of each other.
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box2, box3));
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box3, box2));

  // box4 should not be the near neighbor of any one.
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box1, box4));
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box2, box4));
  EXPECT_FALSE(equation_det_->RunIsNearSmallNeighbor(box3, box4));
}

TEST_F(EquationFinderTest, CheckSeedBlobsCount) {
  TBOX box(0, 950, 999, 999);
  ColPartition *part1 = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  ColPartition *part2 = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  ColPartition *part3 = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  ColPartition *part4 = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);

  // Part 1: 8 math, 0 digit, 20 total.
  equation_det_->AddMathDigitBlobs(8, 0, 20, part1);
  EXPECT_TRUE(equation_det_->RunCheckSeedBlobsCount(part1));

  // Part 2: 1 math, 8 digit, 20 total.
  equation_det_->AddMathDigitBlobs(1, 8, 20, part2);
  EXPECT_FALSE(equation_det_->RunCheckSeedBlobsCount(part2));

  // Part 3: 3 math, 8 digit, 8 total.
  equation_det_->AddMathDigitBlobs(3, 8, 20, part3);
  EXPECT_TRUE(equation_det_->RunCheckSeedBlobsCount(part3));

  // Part 4: 8 math, 0 digit, 8 total.
  equation_det_->AddMathDigitBlobs(0, 0, 8, part4);
  EXPECT_FALSE(equation_det_->RunCheckSeedBlobsCount(part4));

  // Release memory.
  part1->DeleteBoxes();
  delete (part1);
  part2->DeleteBoxes();
  delete (part2);
  part3->DeleteBoxes();
  delete (part3);
  part4->DeleteBoxes();
  delete (part4);
}

TEST_F(EquationFinderTest, ComputeForegroundDensity) {
  // Create the pix with top half foreground, bottom half background.
  int width = 1024, height = 768;
  Image pix = pixCreate(width, height, 1);
  pixRasterop(pix, 0, 0, width, height / 2, PIX_SET, nullptr, 0, 0);
  TBOX box1(100, 0, 140, 140), box2(100, height / 2 - 20, 140, height / 2 + 20),
      box3(100, height - 40, 140, height);
  equation_det_->SetPixBinary(pix);

  // Verify
  EXPECT_NEAR(0.0, equation_det_->RunComputeForegroundDensity(box1), 0.0001f);
  EXPECT_NEAR(0.5, equation_det_->RunComputeForegroundDensity(box2), 0.0001f);
  EXPECT_NEAR(1.0, equation_det_->RunComputeForegroundDensity(box3), 0.0001f);
}

TEST_F(EquationFinderTest, CountAlignment) {
  std::vector<int> vec;
  vec.push_back(1);
  vec.push_back(1);
  vec.push_back(1);
  vec.push_back(100);
  vec.push_back(200);
  vec.push_back(200);

  // Test the right point.
  EXPECT_EQ(3, equation_det_->RunCountAlignment(vec, 1));
  EXPECT_EQ(1, equation_det_->RunCountAlignment(vec, 100));
  EXPECT_EQ(2, equation_det_->RunCountAlignment(vec, 200));

  // Test the near neighbors.
  EXPECT_EQ(3, equation_det_->RunCountAlignment(vec, 3));
  EXPECT_EQ(1, equation_det_->RunCountAlignment(vec, 99));
  EXPECT_EQ(2, equation_det_->RunCountAlignment(vec, 202));

  // Test the far neighbors.
  EXPECT_EQ(0, equation_det_->RunCountAlignment(vec, 150));
  EXPECT_EQ(0, equation_det_->RunCountAlignment(vec, 50));
  EXPECT_EQ(0, equation_det_->RunCountAlignment(vec, 250));
}

TEST_F(EquationFinderTest, ComputeCPsSuperBBox) {
  Image pix = pixCreate(1001, 1001, 1);
  equation_det_->SetPixBinary(pix);
  ColPartitionGrid part_grid(10, ICOORD(0, 0), ICOORD(1000, 1000));

  TBOX box1(0, 0, 999, 99);
  ColPartition *part1 = ColPartition::FakePartition(box1, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  TBOX box2(0, 100, 499, 199);
  ColPartition *part2 = ColPartition::FakePartition(box2, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  TBOX box3(500, 100, 999, 199);
  ColPartition *part3 = ColPartition::FakePartition(box3, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  TBOX box4(0, 200, 999, 299);
  ColPartition *part4 = ColPartition::FakePartition(box4, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  TBOX box5(0, 900, 999, 999);
  ColPartition *part5 = ColPartition::FakePartition(box5, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);

  // Add part1->part3 into part_grid and test.
  part_grid.InsertBBox(true, true, part1);
  part_grid.InsertBBox(true, true, part2);
  part_grid.InsertBBox(true, true, part3);
  TBOX super_box(0, 0, 999, 199);
  equation_det_->TestComputeCPsSuperBBox(super_box, &part_grid);

  // Add part4 and test.
  part_grid.InsertBBox(true, true, part4);
  TBOX super_box2(0, 0, 999, 299);
  equation_det_->TestComputeCPsSuperBBox(super_box2, &part_grid);

  // Add part5 and test.
  part_grid.InsertBBox(true, true, part5);
  TBOX super_box3(0, 0, 999, 999);
  equation_det_->TestComputeCPsSuperBBox(super_box3, &part_grid);

  // Release memory.
  part1->DeleteBoxes();
  delete (part1);
  part2->DeleteBoxes();
  delete (part2);
  part3->DeleteBoxes();
  delete (part3);
  part4->DeleteBoxes();
  delete (part4);
  part5->DeleteBoxes();
  delete (part5);
}

TEST_F(EquationFinderTest, SplitCPHorLite) {
  TBOX box(0, 0, 999, 99);
  ColPartition *part = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part->DeleteBoxes();
  part->set_median_width(10);
  std::vector<TBOX> splitted_boxes;

  // Test an empty part.
  equation_det_->RunSplitCPHorLite(part, &splitted_boxes);
  EXPECT_TRUE(splitted_boxes.empty());

  // Test with one blob.
  AddBlobIntoPart(TBOX(0, 0, 10, 50), part);
  equation_det_->RunSplitCPHorLite(part, &splitted_boxes);
  EXPECT_EQ(1, splitted_boxes.size());
  EXPECT_TRUE(TBOX(0, 0, 10, 50) == splitted_boxes[0]);

  // Add more blob and test.
  AddBlobIntoPart(TBOX(11, 0, 20, 60), part);
  AddBlobIntoPart(TBOX(25, 0, 30, 55), part); // break point.
  AddBlobIntoPart(TBOX(100, 0, 110, 15), part);
  AddBlobIntoPart(TBOX(125, 0, 140, 45), part); // break point.
  AddBlobIntoPart(TBOX(500, 0, 540, 35), part); // break point.
  equation_det_->RunSplitCPHorLite(part, &splitted_boxes);
  // Verify.
  EXPECT_EQ(3, splitted_boxes.size());
  EXPECT_TRUE(TBOX(0, 0, 30, 60) == splitted_boxes[0]);
  EXPECT_TRUE(TBOX(100, 0, 140, 45) == splitted_boxes[1]);
  EXPECT_TRUE(TBOX(500, 0, 540, 35) == splitted_boxes[2]);

  part->DeleteBoxes();
  delete (part);
}

TEST_F(EquationFinderTest, SplitCPHor) {
  TBOX box(0, 0, 999, 99);
  ColPartition *part = ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
  part->DeleteBoxes();
  part->set_median_width(10);
  std::vector<ColPartition *> parts_splitted;

  // Test an empty part.
  equation_det_->RunSplitCPHor(part, &parts_splitted);
  EXPECT_TRUE(parts_splitted.empty());
  // Test with one blob.
  AddBlobIntoPart(TBOX(0, 0, 10, 50), part);

  equation_det_->RunSplitCPHor(part, &parts_splitted);
  EXPECT_EQ(1, parts_splitted.size());
  EXPECT_TRUE(TBOX(0, 0, 10, 50) == parts_splitted[0]->bounding_box());

  // Add more blob and test.
  AddBlobIntoPart(TBOX(11, 0, 20, 60), part);
  AddBlobIntoPart(TBOX(25, 0, 30, 55), part); // break point.
  AddBlobIntoPart(TBOX(100, 0, 110, 15), part);
  AddBlobIntoPart(TBOX(125, 0, 140, 45), part); // break point.
  AddBlobIntoPart(TBOX(500, 0, 540, 35), part); // break point.
  equation_det_->RunSplitCPHor(part, &parts_splitted);

  // Verify.
  EXPECT_EQ(3, parts_splitted.size());
  EXPECT_TRUE(TBOX(0, 0, 30, 60) == parts_splitted[0]->bounding_box());
  EXPECT_TRUE(TBOX(100, 0, 140, 45) == parts_splitted[1]->bounding_box());
  EXPECT_TRUE(TBOX(500, 0, 540, 35) == parts_splitted[2]->bounding_box());

  for (auto part_splitted : parts_splitted) {
    delete part_splitted;
  }
  part->DeleteBoxes();
  delete (part);
}

} // namespace tesseract
