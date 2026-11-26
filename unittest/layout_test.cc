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

#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include "include_gunit.h"

#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>
#include "coutln.h"
#include "image.h"           // for Image
#include "log.h"             // for LOG
#include "mutableiterator.h"
#include "ocrblock.h"        // for class BLOCK
#include "pageres.h"
#include "polyblk.h"
#include "stepblob.h"

namespace tesseract {

/** String name for each block type. Keep in sync with PolyBlockType. */
static const char *kPolyBlockNames[] = {
    "Unknown",
    "Flowing Text",
    "Heading Text",
    "Pullout Text",
    "Equation",
    "Inline Equation",
    "Table",
    "Vertical Text",
    "Caption Text",
    "Flowing Image",
    "Heading Image",
    "Pullout Image",
    "Horizontal Line",
    "Vertical Line",
    "Noise",
    "" // End marker for testing that sizes match.
};

const char *kStrings8087_054[] = {"dat", "Dalmatian", "", "DAMAGED DURING", "margarine,", nullptr};
const PolyBlockType kBlocks8087_054[] = {PT_HEADING_TEXT, PT_FLOWING_TEXT, PT_PULLOUT_IMAGE,
                                         PT_CAPTION_TEXT, PT_FLOWING_TEXT};

// Size of the synthetic pages, about A5 at 300 dpi.
const int kPageWidth = 2000;
const int kPageHeight = 2800;
// Left end, baseline and size of the page number of the synthetic pages.
const int kPageNumberLeft = 300;
const int kPageNumberBaseline = 2580;
const int kPageNumberHeight = 60;
// Width of the dark border of a scan.
const int kScanBorder = 60;
// Words for the text of the synthetic pages.
const char *kPageWords[] = {"layout", "analysis", "finds", "the",     "text",
                            "lines",  "of",       "a",     "printed", "page",
                            "and",    "puts",     "them",  "into",    "blocks",
                            "which",  "are",      "read",  "in",      "order"};

// The fixture for testing Tesseract.
class LayoutTest : public testing::Test {
protected:
  std::string TestDataNameToPath(const std::string &name) {
    return file::JoinPath(TESTING_DIR, "/" + name);
  }
  std::string TessdataPath() {
    return file::JoinPath(TESSDATA_DIR, "");
  }

  LayoutTest() {
    src_pix_ = nullptr;
  }
  ~LayoutTest() override {
    src_pix_.destroy();
  }

  void SetImage(const char *filename, const char *lang) {
    src_pix_.destroy();
    src_pix_ = pixRead(TestDataNameToPath(filename).c_str());
    api_.Init(TessdataPath().c_str(), lang, tesseract::OEM_TESSERACT_ONLY);
    api_.SetPageSegMode(tesseract::PSM_AUTO);
    api_.SetImage(src_pix_);
  }

  // Tests reading order and block finding (very roughly) by iterating
  // over the blocks, expecting that they contain the strings in order,
  // allowing for other blocks in between.
  // An empty string should match an image block, and a nullptr string
  // indicates the end of the array.
  void VerifyBlockTextOrder(const char *strings[], const PolyBlockType *blocks,
                            ResultIterator *it) {
    it->Begin();
    int string_index = 0;
    int block_index = 0;
    do {
      char *block_text = it->GetUTF8Text(tesseract::RIL_BLOCK);
      if (block_text != nullptr && it->BlockType() == blocks[string_index] &&
          strstr(block_text, strings[string_index]) != nullptr) {
        LOG(INFO) << "Found string " << strings[string_index] << " in block " << block_index
                  << " of type " << kPolyBlockNames[blocks[string_index]] << "\n";
        // Found this one.
        ++string_index;
      } else if (it->BlockType() == blocks[string_index] && block_text == nullptr &&
                 strings[string_index][0] == '\0') {
        LOG(INFO) << "Found block of type " << kPolyBlockNames[blocks[string_index]] << " at block "
                  << block_index << "\n";
        // Found this one.
        ++string_index;
      } else {
        LOG(INFO) << "No match found in block with text:\n" << block_text;
      }
      delete[] block_text;
      ++block_index;
      if (strings[string_index] == nullptr) {
        break;
      }
    } while (it->Next(tesseract::RIL_BLOCK));
    EXPECT_TRUE(strings[string_index] == nullptr);
  }

  // Tests that approximate order of the biggest text blocks is correct.
  // Correctness is tested by the following simple rules:
  // If a block overlaps its predecessor in x, then it must be below it.
  // otherwise, if the block is not below its predecessor, then it must
  // be to the left of it if right_to_left is true, or to the right otherwise.
  void VerifyRoughBlockOrder(bool right_to_left, ResultIterator *it) {
    int prev_left = 0;
    int prev_right = 0;
    int prev_bottom = 0;
    it->Begin();
    do {
      int left, top, right, bottom;
      if (it->BoundingBox(tesseract::RIL_BLOCK, &left, &top, &right, &bottom) &&
          PTIsTextType(it->BlockType()) && right - left > 800 && bottom - top > 200) {
        if (prev_right > prev_left) {
          if (std::min(right, prev_right) > std::max(left, prev_left)) {
            EXPECT_GE(top, prev_bottom) << "Overlapping block should be below";
          } else if (top < prev_bottom) {
            if (right_to_left) {
              EXPECT_GE(prev_left, right) << "Block should be to the left";
            } else {
              EXPECT_GE(left, prev_right) << "Block should be to the right";
            }
          }
        }
        prev_left = left;
        prev_right = right;
        prev_bottom = bottom;
      }
    } while (it->Next(tesseract::RIL_BLOCK));
  }

  // Tests that every blob assigned to the biggest text blocks is contained
  // fully within its block by testing that the block polygon winds around
  // the center of the bounding boxes of the outlines in the blob.
  void VerifyTotalContainment(int winding_target, MutableIterator *it) {
    it->Begin();
    do {
      int left, top, right, bottom;
      if (it->BoundingBox(tesseract::RIL_BLOCK, &left, &top, &right, &bottom) &&
          PTIsTextType(it->BlockType()) && right - left > 800 && bottom - top > 200) {
        const PAGE_RES_IT *pr_it = it->PageResIt();
        POLY_BLOCK *pb = pr_it->block()->block->pdblk.poly_block();
        CHECK(pb != nullptr);
        FCOORD skew = pr_it->block()->block->skew();
        EXPECT_GT(skew.x(), 0.0f);
        EXPECT_GT(skew.y(), 0.0f);
        // Iterate the words in the block.
        MutableIterator word_it = *it;
        do {
          const PAGE_RES_IT *w_it = word_it.PageResIt();
          // Iterate the blobs in the word.
          C_BLOB_IT b_it(w_it->word()->word->cblob_list());
          for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
            C_BLOB *blob = b_it.data();
            // Iterate the outlines in the blob.
            C_OUTLINE_IT ol_it(blob->out_list());
            for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
              C_OUTLINE *ol = ol_it.data();
              TBOX box = ol->bounding_box();
              ICOORD middle((box.left() + box.right()) / 2, (box.top() + box.bottom()) / 2);
              EXPECT_EQ(winding_target, pb->winding_number(middle));
            }
          }
        } while (word_it.Next(tesseract::RIL_WORD) &&
                 !word_it.IsAtBeginningOf(tesseract::RIL_BLOCK));
      }
    } while (it->Next(tesseract::RIL_BLOCK));
  }

  // Writes text with its baseline at y from x to the right on pix, with a
  // font of the given size (an even number from 4 to 20) which is built in
  // Leptonica. Returns the width of the text.
  static int DrawText(Pix *pix, const char *text, int x, int y, int size = 10) {
    static L_BMF *fonts[21];
    CHECK(size >= 4 && size <= 20);
    if (fonts[size] == nullptr) {
      fonts[size] = bmfCreate(nullptr, size);
    }
    CHECK(fonts[size] != nullptr);
    int width = 0;
    int overflow = 0;
    pixSetTextline(pix, fonts[size], text, 0, x, y, &width, &overflow);
    return width;
  }

  // A rectangle in the coordinates of the image.
  struct Rect {
    int left;
    int top;
    int right;
    int bottom;
  };

  // Writes text with the largest built in font, enlarged by the factor
  // scale, with its centre at x and its baseline at y on pix. Returns the
  // rectangle of its pixels.
  static Rect DrawCentredText(Pix *pix, const char *text, int x, int y,
                              int scale) {
    const int kBaseline = 60;
    Pix *line = pixCreate(1200, 100, 8);
    pixSetAll(line);
    DrawText(line, text, 10, kBaseline, 20);
    Pix *binary = pixThresholdToBinary(line, 128);
    Box *box = nullptr;
    pixClipToForeground(binary, nullptr, &box);
    CHECK(box != nullptr);
    l_int32 left, top, width, height;
    boxGetGeometry(box, &left, &top, &width, &height);
    Pix *word = pixClipRectangle(line, box, nullptr);
    Pix *scaled = pixScaleBySampling(word, scale, scale);
    Rect rect;
    rect.left = x - width * scale / 2;
    rect.top = y - (kBaseline - top) * scale;
    rect.right = rect.left + width * scale;
    rect.bottom = rect.top + height * scale;
    pixRasterop(pix, rect.left, rect.top, width * scale, height * scale,
                PIX_SRC, scaled, 0, 0);
    pixDestroy(&scaled);
    pixDestroy(&word);
    boxDestroy(&box);
    pixDestroy(&binary);
    pixDestroy(&line);
    return rect;
  }

  // Makes a page of text with a title of two lines of very large text and a
  // small word between them, like the title of a newspaper. The letters of
  // the small word are as high as the lines of the text, and they are so
  // close to the title that the layout analysis takes them for diacritics
  // of its letters. Returns the rectangle of the small word in small_word.
  static Pix *MakeTitlePage(Rect *small_word) {
    Pix *page = MakeTextPage();
    // Make room at the top of the text.
    FillRect(page, 200, 300, 1600, 660, false);
    DrawCentredText(page, "German Gazette", kPageWidth / 2, 400, 3);
    *small_word = DrawCentredText(page, "and", kPageWidth / 2, 500, 1);
    DrawCentredText(page, "Prussian Herald", kPageWidth / 2, 720, 3);
    return page;
  }

  // Makes a white 8 bit page with 32 lines of text.
  static Pix *MakeTextPage() {
    Pix *pix = pixCreate(kPageWidth, kPageHeight, 8);
    pixSetAll(pix);
    int word = 0;
    for (int y = 400; y <= 2300; y += 60) {
      for (int x = 300; x < 1450;) {
        x += DrawText(pix, kPageWords[word++ % std::size(kPageWords)], x, y) +
             16;
      }
    }
    return pix;
  }

  // Paints the given rectangle black, or white if not black.
  static void FillRect(Pix *pix, int left, int top, int width, int height,
                       bool black) {
    Box *box = boxCreate(left, top, width, height);
    if (black) {
      pixClearInRect(pix, box);
    } else {
      pixSetInRect(pix, box);
    }
    boxDestroy(&box);
  }

  // Adds a dark border on all sides of the page, like the scan of a page
  // which is smaller than the scanned area.
  static void AddScanBorder(Pix *pix) {
    FillRect(pix, 0, 0, kPageWidth, kScanBorder, true);
    FillRect(pix, 0, kPageHeight - kScanBorder, kPageWidth, kScanBorder, true);
    FillRect(pix, 0, 0, kScanBorder, kPageHeight, true);
    FillRect(pix, kPageWidth - kScanBorder, 0, kScanBorder, kPageHeight, true);
  }

  // Takes a synthetic page, which is destroyed here, and runs the layout
  // analysis on it. A resolution of 0 leaves it to Tesseract.
  void AnalysePage(Pix *page, int resolution = 0,
                   tesseract::PageSegMode mode = tesseract::PSM_AUTO,
                   const char *lang = "eng") {
    src_pix_.destroy();
    src_pix_ = pixThresholdToBinary(page, 128);
    pixDestroy(&page);
    ASSERT_EQ(api_.Init(TessdataPath().c_str(), lang), 0);
    api_.SetPageSegMode(mode);
    api_.SetImage(src_pix_);
    if (resolution > 0) {
      api_.SetSourceResolution(resolution);
    }
    page_it_.reset(api_.AnalyseLayout());
  }

  // Returns the number of text lines which the last AnalysePage found
  // completely inside the given rectangle.
  int CountTextLinesIn(int left, int top, int right, int bottom) {
    int count = 0;
    if (page_it_ == nullptr) {
      return count;
    }
    page_it_->Begin();
    do {
      int line_left, line_top, line_right, line_bottom;
      if (PTIsTextType(page_it_->BlockType()) &&
          page_it_->BoundingBox(tesseract::RIL_TEXTLINE, &line_left, &line_top,
                                &line_right, &line_bottom) &&
          line_left >= left && line_right <= right && line_top >= top &&
          line_bottom <= bottom) {
        ++count;
      }
    } while (page_it_->Next(tesseract::RIL_TEXTLINE));
    return count;
  }

  // Finds the text line which the last AnalysePage found completely inside
  // area, not more than margin pixels from its sides. Sets line to its box
  // and block to the bounding box of the polygon of its block. Returns false
  // if there is no such line or more than one.
  bool FindTextLineIn(const Rect &area, int margin, Rect *line, Rect *block) {
    int count = 0;
    if (page_it_ == nullptr) {
      return false;
    }
    page_it_->Begin();
    do {
      Rect box;
      if (!PTIsTextType(page_it_->BlockType()) ||
          !page_it_->BoundingBox(tesseract::RIL_TEXTLINE, &box.left, &box.top,
                                 &box.right, &box.bottom) ||
          box.left < area.left - margin || box.right > area.right + margin ||
          box.top < area.top - margin || box.bottom > area.bottom + margin) {
        continue;
      }
      ++count;
      *line = box;
      *block = {0, 0, 0, 0};
      Pta *polygon = page_it_->BlockPolygon();
      if (polygon != nullptr) {
        l_float32 left, right, top, bottom;
        ptaGetRange(polygon, &left, &right, &top, &bottom);
        *block = {static_cast<int>(left), static_cast<int>(top),
                  static_cast<int>(right), static_cast<int>(bottom)};
        ptaDestroy(&polygon);
      }
    } while (page_it_->Next(tesseract::RIL_TEXTLINE));
    return count == 1;
  }

  // Expects that the two rectangles differ by not more than tolerance pixels
  // on each side.
  static void ExpectNear(const Rect &a, const Rect &b, int tolerance) {
    EXPECT_NEAR(a.left, b.left, tolerance);
    EXPECT_NEAR(a.top, b.top, tolerance);
    EXPECT_NEAR(a.right, b.right, tolerance);
    EXPECT_NEAR(a.bottom, b.bottom, tolerance);
  }

  // Expects that the last AnalysePage found the small word of a page from
  // MakeTitlePage as a text line with a block of its own, which is where
  // the line is. The word is at small_word, not more than margin pixels
  // away, and its line and its block fit with the given tolerance.
  void ExpectSmallWordBlock(const Rect &small_word, int margin,
                            int tolerance) {
    Rect line;
    Rect block;
    ASSERT_TRUE(FindTextLineIn(small_word, margin, &line, &block));
    // All of the word, not a part of it.
    EXPECT_NEAR(line.right - line.left, small_word.right - small_word.left,
                tolerance);
    EXPECT_NEAR(line.bottom - line.top, small_word.bottom - small_word.top,
                tolerance);
    ExpectNear(block, line, tolerance);
  }

  // Returns the number of text lines found where the page number is.
  int CountPageNumberLines() {
    return CountTextLinesIn(kPageNumberLeft - 50,
                            kPageNumberBaseline - kPageNumberHeight,
                            kPageNumberLeft + 250, kPageNumberBaseline + 30);
  }

  Image src_pix_;
  std::string ocr_text_;
  tesseract::TessBaseAPI api_;
  std::unique_ptr<tesseract::PageIterator> page_it_;
};

// Tests that array sizes match their intended size.
TEST_F(LayoutTest, ArraySizeTest) {
  int size = 0;
  for (size = 0; kPolyBlockNames[size][0] != '\0'; ++size) {
    ;
  }
  EXPECT_EQ(size, PT_COUNT);
}

// Tests that Tesseract gets the important blocks and in the right order
// on a UNLV page numbered 8087_054.3B.tif. (Dubrovnik)
TEST_F(LayoutTest, UNLV8087_054) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  SetImage("8087_054.3B.tif", "eng");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  // Check iterator position.
  tesseract::ResultIterator *it = api_.GetIterator();
  VerifyBlockTextOrder(kStrings8087_054, kBlocks8087_054, it);
  delete it;
}

// Tests that Tesseract gets the important blocks and in the right order
// on GOOGLE:13510798882202548:74:84.sj-79.tif (Hebrew image)
// TODO: replace hebrew.png by Google image referred above
TEST_F(LayoutTest, HebrewOrderingAndSkew) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "heb");
  SetImage("hebrew.png", "eng");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  tesseract::MutableIterator *it = api_.GetMutableIterator();
  // In eng mode, block order should not be RTL.
  VerifyRoughBlockOrder(false, it);
  VerifyTotalContainment(1, it);
  delete it;
  // Now try again using Hebrew.
  SetImage("hebrew.png", "heb");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  it = api_.GetMutableIterator();
  // In heb mode, block order should be RTL.
  VerifyRoughBlockOrder(true, it);
  // And blobs should still be fully contained.
  VerifyTotalContainment(-1, it);
  delete it;
}

// Tests that a page number is found on a clean page. The following tests
// need this to mean anything.
TEST_F(LayoutTest, PageNumber) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  Pix *page = MakeTextPage();
  DrawText(page, "184", kPageNumberLeft, kPageNumberBaseline);
  AnalysePage(page);
  EXPECT_EQ(CountTextLinesIn(250, 340, 1750, 2330), 32);
  EXPECT_EQ(CountPageNumberLines(), 1);
}

// Tests that the dark border of a scan, which is a frame around the page,
// does not take the page number and a running title as part of an image.
TEST_F(LayoutTest, PageNumberInsideScanBorder) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  Pix *page = MakeTextPage();
  DrawText(page, "CURATOR", 300, 240);
  DrawText(page, "184", kPageNumberLeft, kPageNumberBaseline);
  AddScanBorder(page);
  AnalysePage(page);
  EXPECT_EQ(CountTextLinesIn(250, 340, 1750, 2330), 32);
  EXPECT_EQ(CountTextLinesIn(250, 180, 750, 270), 1);
  EXPECT_EQ(CountPageNumberLines(), 1);
}

// Tests that a line of text between two dashed lines is found.
TEST_F(LayoutTest, TextLineBetweenLeaders) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  Pix *page = MakeTextPage();
  // Make room in the middle of the text.
  FillRect(page, 200, 1090, 1600, 250, false);
  std::string dashes;
  for (int i = 0; i < 30; ++i) {
    dashes += "- ";
  }
  DrawText(page, dashes.c_str(), 500, 1160);
  DrawText(page, "Insert Table 6 about here", 700, 1230);
  DrawText(page, dashes.c_str(), 500, 1300);
  AnalysePage(page);
  EXPECT_EQ(CountTextLinesIn(600, 1175, 1400, 1260), 1);
}

// Tests that a word is found in an image which has only a few pixels of
// margin around it, like the images of single words or lines. The digits
// all have the same height, so the word has straight edges.
TEST_F(LayoutTest, WordWithLittleMargin) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  const int kMargin = 4;
  Pix *page = pixCreate(600, 200, 8);
  pixSetAll(page);
  DrawText(page, "6881", 100, 120);
  // Cut out the word with its margin.
  Pix *binary = pixThresholdToBinary(page, 128);
  Box *word_box = nullptr;
  pixClipToForeground(binary, nullptr, &word_box);
  ASSERT_NE(word_box, nullptr);
  Pix *word = pixClipRectangle(page, word_box, nullptr);
  Pix *image = pixAddBorder(word, kMargin, 255);
  const int width = pixGetWidth(image);
  const int height = pixGetHeight(image);
  pixDestroy(&binary);
  boxDestroy(&word_box);
  pixDestroy(&word);
  pixDestroy(&page);
  AnalysePage(image, 300);
  EXPECT_EQ(CountTextLinesIn(0, 0, width, height), 1);
}

// Tests that a row which is made from the lines of two columns is not taken
// for noise. In the mode for a single block the lines of both columns share
// their rows. Here the left line is lower than the right one, which has the
// larger font, so the row gets the baseline of the right line and an
// x-height which is less than half the height of its characters.
TEST_F(LayoutTest, RowFromTwoColumns) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  const int kBaseline = 1230;
  Pix *page = MakeTextPage();
  // Make room in the middle of the text.
  FillRect(page, 200, 1090, 1600, 250, false);
  DrawText(page, "GAJOVA 13", 300, kBaseline + 14);
  DrawText(page, "No. : 91385763", 900, kBaseline, 12);
  AnalysePage(page, 0, tesseract::PSM_SINGLE_BLOCK);
  EXPECT_EQ(CountTextLinesIn(250, kBaseline - 60, 1750, kBaseline + 60), 1);
}

// Tests that a small word between the two lines of a very large title is
// found. Its letters are shared out as diacritics among the letters of the
// title, so it gets a text line and a block of its own.
TEST_F(LayoutTest, SmallWordInLargeTitle) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  Rect small_word;
  Pix *page = MakeTitlePage(&small_word);
  AnalysePage(page);
  ExpectSmallWordBlock(small_word, 2, 2);
  // The lines of the text are still there.
  EXPECT_EQ(CountTextLinesIn(250, 960, 1750, 2330), 22);
}

// Tests that the block of the small word is where the word is on a skewed
// page. The layout analysis makes the blocks from deskewed boxes and turns
// them back at the end.
TEST_F(LayoutTest, SmallWordInSkewedLargeTitle) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  const float kRadiansPerDegree = 0.0174533f;
  for (float degrees : {1.0f, -1.0f}) {
    SCOPED_TRACE(degrees);
    Rect small_word;
    Pix *page = MakeTitlePage(&small_word);
    Pix *skewed = pixRotate(page, degrees * kRadiansPerDegree,
                            L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    pixDestroy(&page);
    ASSERT_NE(skewed, nullptr);
    AnalysePage(skewed);
    // The skew moves the word by some pixels.
    ExpectSmallWordBlock(small_word, 30, 5);
  }
}

// Tests that the block of the small word is where the word is on a page
// which is read from right to left, where the layout analysis works with
// reflected boxes.
TEST_F(LayoutTest, SmallWordInLargeTitleRightToLeft) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "heb");
  Rect small_word;
  Pix *page = MakeTitlePage(&small_word);
  AnalysePage(page, 0, tesseract::PSM_AUTO, "heb");
  ExpectSmallWordBlock(small_word, 2, 2);
}

// Tests that the block of the small word is where the word is on a page
// which is upside down, and that it comes after the block of the title.
TEST_F(LayoutTest, SmallWordInLargeTitleUpsideDown) {
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "osd");
  Rect small_word;
  Pix *page = MakeTitlePage(&small_word);
  Pix *turned = pixRotate180(nullptr, page);
  pixDestroy(&page);
  AnalysePage(turned, 0, tesseract::PSM_AUTO_OSD);
  const Rect turned_word = {kPageWidth - small_word.right,
                            kPageHeight - small_word.bottom,
                            kPageWidth - small_word.left,
                            kPageHeight - small_word.top};
  ExpectSmallWordBlock(turned_word, 2, 2);
  // The first block is the title, which is much wider than the small word.
  page_it_->Begin();
  int left, top, right, bottom;
  ASSERT_TRUE(page_it_->BoundingBox(tesseract::RIL_BLOCK, &left, &top, &right,
                                    &bottom));
  EXPECT_GT(right - left, 1000);
}

} // namespace tesseract
