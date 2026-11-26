// (C) Copyright 2026, Tesseract contributors.
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

#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <tesseract/resultiterator.h>

#include <tesseract/ocrclass.h> // for ETEXT_DESC

#include "image.h"          // for Image
#include "tesseractclass.h" // for Tesseract::auto_columns_used

#include <algorithm> // for std::max, std::min
#include <climits>   // for INT_MAX, INT_MIN
#include <memory>
#include <string>

namespace tesseract {

// Test the automatic multi-column (newspaper) recognition path
// (textord_auto_columns). It detects the vertical gutters between columns,
// recognises each column independently and merges the per-column results back
// into a single page. When no gutter is found it must fall back to the normal
// single-page path and produce exactly the same output.
//
// These tests double as the AddressSanitizer/UndefinedBehaviorSanitizer
// coverage for that path: Recognizing a multi-column page moves BLOCK /
// BLOCK_RES / ROW / WERD / C_BLOB nodes between BLOCK_LIST and PAGE_RES
// containers and restores the full-page thresholded image, which is exactly
// the kind of intrusive-list / pix ownership handling the sanitizer build
// wants to exercise.
//
// 8071_093.3B.tif is a genuine two-column newspaper page that the gutter
// detector triggers on (it splits into two columns), which drives the full
// split + recognise + merge path. phototest.tif is a single-column page that
// yields no gutter, which drives the safe no-split fallback.
class FriendlyTessBaseAPI : public tesseract::TessBaseAPI {
 public:
  // Expose the number of columns the automatic multi-column path used for
  // the last recognised page, without adding anything to the public API: the
  // count is kept in the internal Tesseract class.
  int auto_columns_used() const {
    return tesseract_->auto_columns_used();
  }
};

class MultiColumnTest : public testing::Test {
 protected:
  static std::string TestDataNameToPath(const std::string &name) {
    return file::JoinPath(TESTING_DIR, name);
  }
  static std::string TessdataPath() {
    return TESSDATA_DIR;
  }

  void SetUp() override {
    REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
    if (api_.Init(TessdataPath().c_str(), "eng", tesseract::OEM_LSTM_ONLY) == -1) {
      // eng.traineddata not found.
      GTEST_SKIP();
    }
    api_.SetPageSegMode(tesseract::PSM_AUTO);
  }

  // Reads the named test image.
  static Image ReadImage(const std::string &image) {
    return pixRead(TestDataNameToPath(image).c_str());
  }

  // Gives the image to the API, with textord_auto_columns set to |enable|.
  // Takes the image.
  void SetPage(Image pix, const std::string &name, const std::string &enable) {
    ASSERT_TRUE(pix) << "Could not read " << name;
    ASSERT_TRUE(api_.SetVariable("textord_auto_columns", enable.c_str()));
    api_.SetInputName(name.c_str());
    api_.SetImage(pix);
    pix.destroy(); // SetImage clones, so the caller's copy is no longer needed.
  }

  // Recognise the image with textord_auto_columns set to |enable|. Takes the
  // image. On success (assertion failures abort the test), the UTF-8 text and
  // the number of columns the automatic path used (0 => it fell back to the
  // normal single-page path) are stored in last_text_ and last_columns_.
  void Recognise(Image pix, const std::string &name,
                 const std::string &enable) {
    SetPage(pix, name, enable);
    ASSERT_EQ(api_.Recognize(nullptr), 0) << "Recognize failed on " << name;
    last_columns_ = api_.auto_columns_used();
    char *utf8 = api_.GetUTF8Text();
    ASSERT_TRUE(utf8 != nullptr);
    last_text_ = utf8;
    delete[] utf8;
  }

  // The same for the named test image.
  void Recognise(const std::string &image, const std::string &enable) {
    Recognise(ReadImage(image), image, enable);
  }

  // Checks that the geometry of the recognised page is in the coordinates of
  // the page, at all levels: a symbol lies in its word, a word in its text
  // line and a text line in its paragraph (horizontally, as a column is
  // shifted horizontally), and the baseline of a text line and the polygon
  // of a block are where the text line and the block are.
  // Only text blocks are checked. For a page which is not upright the symbols
  // are left out (the boxes of the symbols which the LSTM recognizer returns
  // for rotated text are not in the coordinates of the image, with or without
  // columns), and a few words may lie outside of their text line (noise at
  // the edge of a column which the recognizer takes for a word).
  void CheckGeometry(bool upright) {
    std::unique_ptr<ResultIterator> it(api_.GetIterator());
    ASSERT_NE(it, nullptr);
    ASSERT_NE(api_.GetInputImage(), nullptr);
    const int page_width = pixGetWidth(api_.GetInputImage());
    int para_left = 0, para_right = 0;
    int line_left = 0, line_right = 0;
    int word_left = 0, word_right = 0;
    int top, bottom;
    int num_blocks = 0, num_lines = 0, num_words = 0, num_symbols = 0;
    int bad_polygons = 0, bad_lines = 0, bad_baselines = 0, bad_words = 0;
    int bad_symbols = 0;
    int words_in_right_half = 0;
    do {
      if (!PTIsTextType(it->BlockType())) {
        continue;
      }
      if (it->IsAtBeginningOf(RIL_BLOCK)) {
        int block_left, block_right;
        ASSERT_TRUE(it->BoundingBox(RIL_BLOCK, &block_left, &top, &block_right,
                                    &bottom));
        Pta *polygon = it->BlockPolygon();
        if (polygon != nullptr) {
          int min_x = INT_MAX;
          int max_x = INT_MIN;
          for (int i = 0; i < ptaGetCount(polygon); ++i) {
            int x, y;
            ptaGetIPt(polygon, i, &x, &y);
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
          }
          ptaDestroy(&polygon);
          ++num_blocks;
          // The text of a block lies in its polygon.
          if (block_left < min_x || block_right > max_x) {
            ++bad_polygons;
          }
        }
      }
      if (it->IsAtBeginningOf(RIL_PARA)) {
        ASSERT_TRUE(
            it->BoundingBox(RIL_PARA, &para_left, &top, &para_right, &bottom));
      }
      if (it->IsAtBeginningOf(RIL_TEXTLINE)) {
        ASSERT_TRUE(it->BoundingBox(RIL_TEXTLINE, &line_left, &top, &line_right,
                                    &bottom));
        ++num_lines;
        if (line_left < para_left || line_right > para_right) {
          ++bad_lines;
        }
        int x1, y1, x2, y2;
        ASSERT_TRUE(it->Baseline(RIL_TEXTLINE, &x1, &y1, &x2, &y2));
        // The baseline and the text line overlap horizontally.
        if (std::max(x1, x2) <= line_left || std::min(x1, x2) >= line_right) {
          ++bad_baselines;
        }
      }
      if (it->IsAtBeginningOf(RIL_WORD)) {
        ASSERT_TRUE(
            it->BoundingBox(RIL_WORD, &word_left, &top, &word_right, &bottom));
        ++num_words;
        if (word_left < line_left || word_right > line_right) {
          ++bad_words;
        }
        if (word_left > page_width / 2) {
          ++words_in_right_half;
        }
      }
      int symbol_left, symbol_right;
      if (upright && it->BoundingBox(RIL_SYMBOL, &symbol_left, &top,
                                     &symbol_right, &bottom)) {
        ++num_symbols;
        if (symbol_left < word_left || symbol_right > word_right) {
          ++bad_symbols;
        }
      }
    } while (it->Next(RIL_SYMBOL));
    EXPECT_GT(num_blocks, 1);
    EXPECT_GT(num_lines, 100);
    EXPECT_GT(num_words, 500);
    if (upright) {
      EXPECT_GT(num_symbols, 2000);
    }
    // The page has four columns of text, two of them in its right half.
    EXPECT_GT(words_in_right_half, num_words / 4);
    EXPECT_EQ(bad_polygons, 0) << "of " << num_blocks << " blocks";
    EXPECT_EQ(bad_lines, 0) << "of " << num_lines << " text lines";
    EXPECT_EQ(bad_baselines, 0) << "of " << num_lines << " text lines";
    EXPECT_LE(bad_words, upright ? 0 : num_words / 100)
        << "of " << num_words << " words";
    EXPECT_EQ(bad_symbols, 0) << "of " << num_symbols << " symbols";
  }

  // Returns the number of blocks and of paragraphs of the recognised page.
  void CountBlocksAndParagraphs(int *blocks, int *paragraphs) {
    *blocks = 0;
    *paragraphs = 0;
    std::unique_ptr<ResultIterator> it(api_.GetIterator());
    ASSERT_NE(it, nullptr);
    do {
      if (it->IsAtBeginningOf(RIL_BLOCK)) {
        ++*blocks;
      }
      ++*paragraphs;
    } while (it->Next(RIL_PARA));
  }

  FriendlyTessBaseAPI api_;
  std::string last_text_;
  int last_columns_ = 0;
};

// A genuine multi-column page must be split into more than one column, and
// the merged result must contain recognisable text and well-formed layout.
TEST_F(MultiColumnTest, SplitsMultiColumnPage) {
  Recognise("8071_093.3B.tif", "1");
  // The detector must have found a gutter, so the automatic path -- not the
  // fallback -- ran, and it split the page into at least two columns.
  EXPECT_GE(last_columns_, 2) << "Expected the automatic path to split 8071 into "
                                 "columns, but it reported "
                              << last_columns_;

  // Merged text must be non-trivial.
  EXPECT_GE(last_text_.size(), 50u)
      << "Multi-column page produced too little text";

  // hOCR rendering walks every merged line/word box (whose coordinates were
  // shifted into full-page space). It must succeed and emit word boxes.
  ASSERT_TRUE(api_.SetVariable("textord_auto_columns", "1"));
  Image pix = pixRead(TestDataNameToPath("8071_093.3B.tif").c_str());
  ASSERT_TRUE(pix);
  api_.SetInputName("8071_093.3B.tif");
  api_.SetImage(pix);
  pix.destroy();
  ASSERT_EQ(api_.Recognize(nullptr), 0);
  char *hocr = api_.GetHOCRText(0);
  ASSERT_TRUE(hocr != nullptr);
  const std::string hocr_str(hocr);
  EXPECT_NE(hocr_str.find("class='ocr_page'"), std::string::npos)
      << "hOCR output is missing the page element";
  EXPECT_NE(hocr_str.find("ocrx_word"), std::string::npos)
      << "hOCR output is missing word boxes";
  delete[] hocr;
}

// The geometry of all columns must be in the coordinates of the page, at all
// levels: a symbol lies in its word, a word in its text line and a text line
// in its paragraph (horizontally, as a column is shifted horizontally), and
// the baseline of a text line and the polygon of a block are where the text
// line and the block are. The words of a text line which the LSTM recognizer
// returns are not the words of the row, and the row, the block and the
// symbols keep boxes of their own, so each of them can be left behind in the
// coordinates of its column.
TEST_F(MultiColumnTest, GeometryInPageCoordinates) {
  Recognise("8071_093.3B.tif", "1");
  ASSERT_GE(last_columns_, 2);
  CheckGeometry(true);
}

// The same for a page which is upside down. The coordinates of its blocks
// are rotated against the image, so a column which is shifted to the right
// in the coordinates of its blocks moves to the left in the image. And its
// columns are read from right to left.
TEST_F(MultiColumnTest, GeometryOfPageUpsideDown) {
#ifdef DISABLED_LEGACY_ENGINE
  // The orientation detection needs the legacy engine.
  GTEST_SKIP();
#else
  REQUIRE_TRAINEDDATA(TESSDATA_DIR, "osd");
  api_.SetPageSegMode(tesseract::PSM_AUTO_OSD);
  Image upright = ReadImage("8071_093.3B.tif");
  ASSERT_TRUE(upright);
  Image pix = pixRotate180(nullptr, upright);
  upright.destroy();
  Recognise(pix, "8071_093.3B.tif", "1");
  ASSERT_GE(last_columns_, 2);
  CheckGeometry(false);
  // The text begins with the first line of the first column, which is the
  // rightmost one in the image.
  const size_t heading = last_text_.find("sighed");
  ASSERT_NE(heading, std::string::npos);
  EXPECT_LT(heading, last_text_.size() / 10);
#endif
}

// The recognition of a page which the monitor stops must be reported as
// failed, as it is without the columns.
TEST_F(MultiColumnTest, CancelIsReported) {
  for (const char *enable : {"0", "1"}) {
    SetPage(ReadImage("8071_093.3B.tif"), "8071_093.3B.tif", enable);
    ETEXT_DESC monitor;
    monitor.cancel = [](void *, int) { return true; };
    EXPECT_EQ(api_.Recognize(&monitor), -1)
        << "with textord_auto_columns " << enable;
  }
}

// The resolution which the application sets for the page must survive the
// recognition of the columns, which are images of their own.
TEST_F(MultiColumnTest, KeepsSourceResolution) {
  for (const char *enable : {"0", "1"}) {
    SetPage(ReadImage("8071_093.3B.tif"), "8071_093.3B.tif", enable);
    ASSERT_NE(api_.GetSourceYResolution(), 250);
    api_.SetSourceResolution(250);
    ASSERT_EQ(api_.Recognize(nullptr), 0);
    EXPECT_EQ(api_.auto_columns_used() >= 2, enable[0] == '1');
    EXPECT_EQ(api_.GetSourceYResolution(), 250)
        << "with textord_auto_columns " << enable;
  }
}

// The modes of Recognize which do not recognise the page in the normal way
// must not be replaced by the recognition of the columns. Making training
// data for the line recognizer is such a mode: it needs a box file, which the
// page does not have, so it fails.
TEST_F(MultiColumnTest, OtherModesAreNotSplit) {
  ASSERT_TRUE(api_.SetVariable("tessedit_train_line_recognizer", "1"));
  SetPage(ReadImage("8071_093.3B.tif"), "8071_093.3B.tif", "1");
  EXPECT_EQ(api_.Recognize(nullptr), -1);
  EXPECT_EQ(api_.auto_columns_used(), 0);
}

// A page segmentation mode without column finding says that the page is a
// single column, block, line or word, so the page must not be split. Such a
// mode also takes the blocks from a zone file for the whole page if there is
// one next to the image, as for this page.
TEST_F(MultiColumnTest, SingleBlockModeIsNotSplit) {
  api_.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
  SetPage(ReadImage("8071_093.3B.tif"), TestDataNameToPath("8071_093.3B.tif"),
          "1");
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  EXPECT_EQ(api_.auto_columns_used(), 0);
}

// Paragraphs must be found in the columns too when they are detected without
// the text (paragraph_text_based off).
TEST_F(MultiColumnTest, ParagraphsWithoutText) {
  ASSERT_TRUE(api_.SetVariable("paragraph_text_based", "0"));
  Recognise("8071_093.3B.tif", "1");
  ASSERT_GE(last_columns_, 2);
  int blocks, paragraphs;
  CountBlocksAndParagraphs(&blocks, &paragraphs);
  EXPECT_GT(blocks, 1);
  // Without the detection each block is one paragraph.
  EXPECT_GT(paragraphs, blocks);
}

// A block of vertical text in a column is rotated in the coordinates of its
// block, so it must not be shifted to the right in these coordinates.
TEST_F(MultiColumnTest, VerticalTextInColumn) {
  Image page = ReadImage("8071_093.3B.tif");
  ASSERT_TRUE(page);
  ASSERT_EQ(pixGetDepth(page), 1);
  // Replace the upper part of the last column by a copy of the upper part of
  // the first column which is rotated by 90 degrees.
  const int kLeft = 2400, kTop = 140, kWidth = 680, kHeight = 760;
  Box *box = boxCreate(170, 150, 640, 580);
  Image lines = pixClipRectangle(page, box, nullptr);
  boxDestroy(&box);
  ASSERT_TRUE(lines);
  Image vertical = pixRotate90(lines, -1);
  lines.destroy();
  ASSERT_TRUE(vertical);
  pixRasterop(page, kLeft, kTop, kWidth, kHeight, PIX_CLR, nullptr, 0, 0);
  pixRasterop(page, kLeft + 40, kTop + 30, pixGetWidth(vertical),
              pixGetHeight(vertical), PIX_SRC, vertical, 0, 0);
  vertical.destroy();
  Recognise(page, "vertical text", "1");
  ASSERT_GE(last_columns_, 2);
  // Count the text lines in that part of the page which are much higher than
  // wide. The rotated copy has 14 text lines.
  std::unique_ptr<ResultIterator> it(api_.GetIterator());
  ASSERT_NE(it, nullptr);
  int num_vertical_lines = 0;
  do {
    int left, top, right, bottom;
    if (it->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom) &&
        bottom - top > 3 * (right - left) && left >= kLeft && top >= kTop &&
        right <= kLeft + kWidth && bottom <= kTop + kHeight) {
      ++num_vertical_lines;
    }
  } while (it->Next(RIL_TEXTLINE));
  EXPECT_GE(num_vertical_lines, 10);
}

// A gutter of a page which is not quite upright is slanted, so the columns
// are cut out along slanted lines and what lies beyond them is painted
// white. White must be white for an image with a colormap too, where the
// highest pixel value can be any colour, or no colour of the colormap at all.
TEST_F(MultiColumnTest, SkewedPageWithColormap) {
  Image upright = ReadImage("8071_093.3B.tif");
  ASSERT_TRUE(upright);
  // Rotate by one degree.
  Image skewed =
      pixRotate(upright, 0.0175f, L_ROTATE_SHEAR, L_BRING_IN_WHITE, 0, 0);
  upright.destroy();
  ASSERT_TRUE(skewed);
  ASSERT_EQ(pixGetDepth(skewed), 1);
  // The same page with the pixel values 0 (white) and 1 (black) and a
  // colormap with 256 colours. The page does not use the other 254 colours,
  // so the text must not depend on them.
  std::string texts[2];
  for (int i = 0; i < 2; ++i) {
    const int unused_colour = i == 0 ? 255 : 0;
    Image with_colormap = pixConvert1To8(nullptr, skewed, 0, 1);
    ASSERT_TRUE(with_colormap);
    PIXCMAP *colormap = pixcmapCreate(8);
    pixcmapAddColor(colormap, 255, 255, 255);
    pixcmapAddColor(colormap, 0, 0, 0);
    for (int c = 2; c < 256; ++c) {
      pixcmapAddColor(colormap, unused_colour, unused_colour, unused_colour);
    }
    ASSERT_EQ(pixSetColormap(with_colormap, colormap), 0);
    Recognise(with_colormap, "skewed with colormap", "1");
    ASSERT_GE(last_columns_, 2);
    EXPECT_GE(last_text_.size(), 3000u);
    texts[i] = last_text_;
  }
  skewed.destroy();
  EXPECT_EQ(texts[0], texts[1]);
}

// On a single-column page the automatic path must not split at all and must
// fall back to the normal path, yielding output identical to the flag-off run.
TEST_F(MultiColumnTest, NoSplitOnSingleColumnPage) {
  Recognise("phototest.tif", "0");
  const std::string off_text = last_text_;
  const int off_columns = last_columns_;

  Recognise("phototest.tif", "1");

  // No gutter => the automatic path fell back to the normal single-page path.
  EXPECT_EQ(off_columns, 0) << "Expected no column split with the flag off";
  EXPECT_EQ(last_columns_, 0) << "Expected no column split on single-column page";

  // The output must be byte-for-byte identical to the disabled run.
  EXPECT_EQ(off_text, last_text_) << "Flag-on output differs from flag-off output";
}

} // namespace tesseract
