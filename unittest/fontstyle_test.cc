///////////////////////////////////////////////////////////////////////
// File:        fontstyle_test.cc
// Description: Unit tests for the estimation of bold and italic from
//              the image of a word (parameter lstm_font_attributes):
//              the measurement of slant and stroke width, the fonts
//              without a name, and the attributes which the API
//              reports for words recognized by LSTM.
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

#include "include_gunit.h"

#include "fontinfo.h"
#include "fontstyle.h"
#include "image.h"

#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace tesseract {

// Returns a binary image with four vertical bars of the given width. A
// positive slant shifts the upper end of the bars to the right.
static Image MakeBars(int stroke_width, float slant) {
  const int kWidth = 200;
  const int kHeight = 60;
  Image pix = pixCreate(kWidth, kHeight, 1);
  for (int y = 0; y < kHeight; ++y) {
    const int shift = static_cast<int>(std::lround(slant * (kHeight - 1 - y)));
    for (int left = 40; left < 140; left += 25) {
      for (int x = 0; x < stroke_width; ++x) {
        pixSetPixel(pix, left + shift + x, y, 1);
      }
    }
  }
  return pix;
}

TEST(FontStyleTest, MeasuresSlant) {
  for (float slant : {-0.1f, -0.03f, 0.0f, 0.03f, 0.1f, 0.2f, 0.3f, 0.35f}) {
    Image pix = MakeBars(4, slant);
    TextShape shape;
    ASSERT_TRUE(MeasureTextShape(pix, &shape));
    EXPECT_TRUE(shape.has_slant) << "slant " << slant;
    EXPECT_NEAR(shape.slant, slant, 0.015f) << "slant " << slant;
    pix.destroy();
  }
}

// Strokes which are steeper than those of italic text are diagonal strokes
// and give no slant.
TEST(FontStyleTest, DiagonalStrokesHaveNoSlant) {
  for (float slant : {-0.3f, 0.5f}) {
    Image pix = MakeBars(4, slant);
    TextShape shape;
    ASSERT_TRUE(MeasureTextShape(pix, &shape));
    EXPECT_FALSE(shape.has_slant) << "slant " << slant;
    EXPECT_EQ(shape.slant, 0.0f) << "slant " << slant;
    EXPECT_FLOAT_EQ(shape.stroke_width, 4.0f) << "slant " << slant;
    pix.destroy();
  }
}

// The stroke width is measured along the pixel rows, so it does not depend
// on the slant.
TEST(FontStyleTest, MeasuresStrokeWidth) {
  for (int stroke_width : {1, 2, 3, 5, 8}) {
    for (float slant : {0.0f, 0.25f}) {
      Image pix = MakeBars(stroke_width, slant);
      TextShape shape;
      ASSERT_TRUE(MeasureTextShape(pix, &shape));
      EXPECT_FLOAT_EQ(shape.stroke_width, stroke_width)
          << "stroke width " << stroke_width << ", slant " << slant;
      pix.destroy();
    }
  }
}

// Strokes of two widths give a median between the widths, which moves with
// the share of each width.
TEST(FontStyleTest, StrokeWidthBetweenPixels) {
  const int kHeight = 40;
  float last_width = 0.0f;
  for (int wide_rows : {8, 16, 24, 32}) {
    Image pix = pixCreate(40, kHeight, 1);
    for (int y = 0; y < kHeight; ++y) {
      const int stroke_width = y < wide_rows ? 4 : 3;
      for (int x = 0; x < stroke_width; ++x) {
        pixSetPixel(pix, 10 + x, y, 1);
      }
    }
    TextShape shape;
    ASSERT_TRUE(MeasureTextShape(pix, &shape));
    EXPECT_GT(shape.stroke_width, 2.5f);
    EXPECT_LT(shape.stroke_width, 4.5f);
    EXPECT_GT(shape.stroke_width, last_width) << wide_rows << " wide rows";
    last_width = shape.stroke_width;
    pix.destroy();
  }
}

TEST(FontStyleTest, RejectsUnusableImages) {
  TextShape shape;
  shape.slant = 7.0f;
  EXPECT_FALSE(MeasureTextShape(nullptr, &shape));
  Image empty = pixCreate(50, 20, 1);
  EXPECT_FALSE(MeasureTextShape(empty, &shape));
  empty.destroy();
  Image grey = pixCreate(50, 20, 8);
  EXPECT_FALSE(MeasureTextShape(grey, &shape));
  grey.destroy();
  // A failed measurement does not change the shape.
  EXPECT_EQ(shape.slant, 7.0f);
}

TEST(FontStyleTest, NamelessFonts) {
  for (bool bold : {false, true}) {
    for (bool italic : {false, true}) {
      const FontInfo *font = NamelessFont(bold, italic);
      ASSERT_NE(font, nullptr);
      EXPECT_EQ(font->name, nullptr);
      EXPECT_EQ(font->universal_id, -1);
      EXPECT_EQ(font->is_bold(), bold);
      EXPECT_EQ(font->is_italic(), italic);
      EXPECT_FALSE(font->is_fixed_pitch());
      EXPECT_FALSE(font->is_serif());
      EXPECT_FALSE(font->is_fraktur());
      // The same style always gives the same font.
      EXPECT_EQ(font, NamelessFont(bold, italic));
    }
  }
  // Fonts without a name can be compared, and differ from each other and
  // from fonts with a name.
  const FontInfo &regular = *NamelessFont(false, false);
  const FontInfo &bold = *NamelessFont(true, false);
  EXPECT_TRUE(regular == regular);
  EXPECT_FALSE(regular == bold);
  char name[] = "Arial";
  FontInfo named;
  named.name = name;
  EXPECT_FALSE(regular == named);
  EXPECT_FALSE(named == regular);
  EXPECT_TRUE(named == named);
}

// A word with the font attributes which the API reports for it.
struct StyledWord {
  std::string text;
  // Index of the text line of the word in the unchanged image, or -1.
  int line = -1;
  bool bold = false;
  bool italic = false;
  bool has_font_name = false;
  int font_id = 0;
  int pointsize = 0;
};

// Tests of the attributes for words recognized by LSTM. phototest.tif has
// eight lines of upright text of normal weight in a single font.
class FontStyleApiTest : public testing::Test {
protected:
  void SetUp() override {
    REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
    if (api_.Init(TESSDATA_DIR, "eng", OEM_LSTM_ONLY) == -1) {
      // eng.traineddata not found.
      GTEST_SKIP();
    }
    api_.SetPageSegMode(PSM_AUTO);
    pix_ = pixRead(TESTING_DIR "/phototest.tif");
    ASSERT_TRUE(pix_ != nullptr);
  }

  void TearDown() override {
    pix_.destroy();
  }

  // Recognizes pix_ and returns its words. The first call stores the boxes of
  // the text lines in line_boxes_ as left, top, right, bottom. Changes of the
  // image can add or split lines, so later calls find the line of a word by
  // its position.
  std::vector<StyledWord> Recognize() {
    std::vector<StyledWord> words;
    const bool first = line_boxes_.empty();
    api_.SetImage(pix_);
    EXPECT_EQ(api_.Recognize(nullptr), 0);
    std::unique_ptr<ResultIterator> it(api_.GetIterator());
    if (it == nullptr) {
      ADD_FAILURE() << "No results";
      return words;
    }
    do {
      std::array<int, 4> box;
      if (first && it->IsAtBeginningOf(RIL_TEXTLINE)) {
        it->BoundingBox(RIL_TEXTLINE, &box[0], &box[1], &box[2], &box[3]);
        line_boxes_.push_back(box);
      }
      StyledWord word;
      std::unique_ptr<char[]> text(it->GetUTF8Text(RIL_WORD));
      word.text = text ? text.get() : "";
      it->BoundingBox(RIL_WORD, &box[0], &box[1], &box[2], &box[3]);
      const int y = (box[1] + box[3]) / 2;
      for (size_t line = 0; line < line_boxes_.size(); ++line) {
        if (line_boxes_[line][1] <= y && y <= line_boxes_[line][3]) {
          word.line = static_cast<int>(line);
        }
      }
      bool underlined, monospace, serif, smallcaps;
      word.has_font_name =
          it->WordFontAttributes(&word.bold, &word.italic, &underlined, &monospace, &serif,
                                 &smallcaps, &word.pointsize, &word.font_id) != nullptr;
      EXPECT_FALSE(underlined);
      EXPECT_FALSE(monospace);
      EXPECT_FALSE(serif);
      words.push_back(word);
    } while (it->Next(RIL_WORD));
    return words;
  }

  // Replaces the given text line of pix_ by the result of the function for
  // the image of the line.
  template <typename Function>
  void ChangeLine(int line, Function change) {
    ASSERT_LT(static_cast<size_t>(line), line_boxes_.size());
    // Use the whole width of the image to have room for a slant.
    const int kPadding = 2;
    const int top = line_boxes_[line][1] - kPadding;
    const int height = line_boxes_[line][3] - line_boxes_[line][1] + 2 * kPadding;
    Box *box = boxCreate(0, top, pixGetWidth(pix_), height);
    Image line_pix = pixClipRectangle(pix_, box, nullptr);
    boxDestroy(&box);
    ASSERT_TRUE(line_pix != nullptr);
    Image changed = change(line_pix);
    ASSERT_TRUE(changed != nullptr);
    pixRasterop(pix_, 0, top, pixGetWidth(pix_), height, PIX_SRC, changed, 0, 0);
    changed.destroy();
    line_pix.destroy();
  }

  TessBaseAPI api_;
  Image pix_;
  std::vector<std::array<int, 4>> line_boxes_;
};

// Without the parameter, words recognized by LSTM have no font attributes.
TEST_F(FontStyleApiTest, DisabledByDefault) {
  bool enabled = true;
  ASSERT_TRUE(api_.GetBoolVariable("lstm_font_attributes", &enabled));
  EXPECT_FALSE(enabled);
  // Slanted text is the best candidate for a wrong result.
  std::vector<StyledWord> words = Recognize();
  ChangeLine(3, [](Image line) { return Image(pixHShear(nullptr, line, pixGetHeight(line) / 2,
                                                         std::atan(0.25f), L_BRING_IN_WHITE)); });
  words = Recognize();
  ASSERT_GT(words.size(), 40u);
  for (const auto &word : words) {
    EXPECT_FALSE(word.bold) << word.text;
    EXPECT_FALSE(word.italic) << word.text;
    EXPECT_FALSE(word.has_font_name) << word.text;
    EXPECT_EQ(word.font_id, -1) << word.text;
  }
}

TEST_F(FontStyleApiTest, RegularTextHasNoStyle) {
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  const std::vector<StyledWord> words = Recognize();
  ASSERT_GT(words.size(), 40u);
  EXPECT_EQ(line_boxes_.size(), 8u);
  for (const auto &word : words) {
    EXPECT_FALSE(word.bold) << word.text;
    EXPECT_FALSE(word.italic) << word.text;
    // The attributes are an estimation, so there is no font name.
    EXPECT_FALSE(word.has_font_name) << word.text;
    EXPECT_EQ(word.font_id, -1) << word.text;
    EXPECT_GT(word.pointsize, 0) << word.text;
  }
}

// A slanted line must be italic, a line with thicker strokes must be bold,
// and the other lines must stay as they are.
TEST_F(FontStyleApiTest, FindsItalicAndBoldLines) {
  const int kItalicLine = 3;
  const int kBoldLine = 5;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  std::vector<StyledWord> words = Recognize();
  ASSERT_EQ(line_boxes_.size(), 8u);
  ChangeLine(kItalicLine, [](Image line) {
    return Image(pixHShear(nullptr, line, pixGetHeight(line) / 2, std::atan(0.25f),
                           L_BRING_IN_WHITE));
  });
  // This makes the text so heavy that it fills more than half of its box.
  ChangeLine(kBoldLine, [](Image line) { return Image(pixDilateBrick(nullptr, line, 3, 3)); });
  words = Recognize();
  int num_italic_line_words = 0;
  int num_bold_line_words = 0;
  for (const auto &word : words) {
    ASSERT_GE(word.line, 0) << word.text;
    EXPECT_EQ(word.italic, word.line == kItalicLine) << word.text << " in line " << word.line;
    EXPECT_EQ(word.bold, word.line == kBoldLine) << word.text << " in line " << word.line;
    EXPECT_FALSE(word.has_font_name) << word.text;
    num_italic_line_words += word.line == kItalicLine;
    num_bold_line_words += word.line == kBoldLine;
  }
  EXPECT_GE(num_italic_line_words, 6);
  EXPECT_GE(num_bold_line_words, 6);

  // The hOCR output marks the words of the two lines.
  std::unique_ptr<char[]> hocr(api_.GetHOCRText(0));
  ASSERT_TRUE(hocr != nullptr);
  const std::string hocr_text(hocr.get());
  auto count = [&hocr_text](const std::string &tag) {
    int num = 0;
    for (size_t pos = hocr_text.find(tag); pos != std::string::npos;
         pos = hocr_text.find(tag, pos + 1)) {
      ++num;
    }
    return num;
  };
  EXPECT_EQ(count("<em>"), num_italic_line_words);
  EXPECT_EQ(count("<strong>"), num_bold_line_words);
}

// White text on black must give the same attributes as black text on white.
TEST_F(FontStyleApiTest, FindsItalicInInverseText) {
  const int kItalicLine = 3;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  std::vector<StyledWord> words = Recognize();
  ASSERT_EQ(line_boxes_.size(), 8u);
  ChangeLine(kItalicLine, [](Image line) {
    return Image(pixHShear(nullptr, line, pixGetHeight(line) / 2, std::atan(0.25f),
                           L_BRING_IN_WHITE));
  });
  pixInvert(pix_, pix_);
  words = Recognize();
  int num_italic_line_words = 0;
  for (const auto &word : words) {
    if (word.line < 0) {
      // The black background can give words outside the text.
      continue;
    }
    EXPECT_EQ(word.italic, word.line == kItalicLine) << word.text << " in line " << word.line;
    EXPECT_FALSE(word.bold) << word.text << " in line " << word.line;
    num_italic_line_words += word.line == kItalicLine;
  }
  EXPECT_GE(num_italic_line_words, 6);
}

} // namespace tesseract
