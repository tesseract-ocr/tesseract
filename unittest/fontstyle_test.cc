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
#include <utility>
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

// Specks of noise are no strokes, even if there are more of them than rows
// of the strokes.
TEST(FontStyleTest, IgnoresSpecks) {
  for (float slant : {0.0f, 0.2f}) {
    Image pix = MakeBars(5, slant);
    const int width = pixGetWidth(pix);
    const int height = pixGetHeight(pix);
    int num_specks = 0;
    for (int y = 0; y < height; y += 2) {
      // Single pixels left and right of the bars, which touch no other text.
      for (int x = y % 4; x < width; x += 4) {
        if (x < 35 || x >= 160) {
          pixSetPixel(pix, x, y, 1);
          ++num_specks;
        }
      }
    }
    // The bars have four runs of pixels in each row.
    ASSERT_GT(num_specks, 4 * height);
    TextShape shape;
    ASSERT_TRUE(MeasureTextShape(pix, &shape));
    EXPECT_FLOAT_EQ(shape.stroke_width, 5.0f) << "slant " << slant;
    EXPECT_TRUE(shape.has_slant) << "slant " << slant;
    EXPECT_NEAR(shape.slant, slant, 0.015f) << "slant " << slant;
    pix.destroy();
  }
}

// Text which is not higher than a pixel still has a stroke width.
TEST(FontStyleTest, MeasuresSingleRow) {
  Image pix = pixCreate(50, 1, 1);
  for (int x = 10; x < 16; ++x) {
    pixSetPixel(pix, x, 0, 1);
  }
  TextShape shape;
  ASSERT_TRUE(MeasureTextShape(pix, &shape));
  EXPECT_FLOAT_EQ(shape.stroke_width, 6.0f);
  pix.destroy();
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

// Adds words of one text line with the given x-height of the line, stroke
// width and height above the baseline. The words have lowercase letters.
static void AddWords(int num_words, int row, float x_height, float stroke, float height,
                     std::vector<WordStrokes> *words) {
  for (int i = 0; i < num_words; ++i) {
    WordStrokes word;
    word.row = row;
    word.x_height = x_height;
    word.stroke = stroke;
    word.height = height;
    word.has_lower = true;
    word.reliable = true;
    words->push_back(word);
  }
}

// Text lines with the same strokes have the same font and size, so small
// differences of their x-heights must not change the weight of their words.
TEST(FontStyleTest, SameStrokesShareTextSize) {
  std::vector<WordStrokes> words;
  const float x_heights[] = {12.0f, 13.0f, 12.0f, 13.0f, 12.0f};
  for (int row = 0; row < 5; ++row) {
    // The words have ascenders, so they do not contradict the x-height.
    AddWords(8, row, x_heights[row], 3.0f, 17.0f, &words);
  }
  // Bold words in a line do not change the stroke width of the line.
  words[9].stroke = words[10].stroke = words[11].stroke = 4.5f;
  SetWordWeights(&words);
  for (size_t i = 0; i < words.size(); ++i) {
    EXPECT_FLOAT_EQ(words[i].weight, (i >= 9 && i <= 11 ? 4.5f : 3.0f) / 12.0f) << "word " << i;
  }
}

// A line which is bold throughout has other strokes than the rest of the
// page, and a line with larger text has higher words, even if its strokes
// are those of the bold line. Both keep their own x-height.
TEST(FontStyleTest, OtherStrokesKeepTextSize) {
  std::vector<WordStrokes> words;
  for (int row = 0; row < 4; ++row) {
    AddWords(8, row, 20.0f, 4.0f, 28.0f, &words);
  }
  AddWords(5, 4, 20.0f, 6.0f, 28.0f, &words);
  AddWords(5, 5, 28.0f, 5.6f, 39.0f, &words);
  SetWordWeights(&words);
  for (size_t i = 0; i < words.size(); ++i) {
    EXPECT_FLOAT_EQ(words[i].weight, words[i].row == 4 ? 0.3f : 0.2f) << "word " << i;
  }
}

// The x-height of a line is wrong if words with lowercase letters are much
// lower, or if words of capital letters are not higher.
TEST(FontStyleTest, CorrectsXHeight) {
  std::vector<WordStrokes> words;
  for (int row = 0; row < 4; ++row) {
    AddWords(8, row, 20.0f, 4.0f, 28.0f, &words);
  }
  // A bold line with the height of the capital letters as x-height: one word
  // without ascenders shows the real x-height.
  AddWords(5, 4, 28.0f, 6.0f, 28.0f, &words);
  words.back().height = 20.0f;
  // The same for a line of capital letters.
  AddWords(5, 5, 28.0f, 8.0f, 28.0f, &words);
  for (size_t i = words.size() - 5; i < words.size(); ++i) {
    words[i].has_lower = false;
    words[i].capitals_only = true;
  }
  // A line of a script without lowercase letters keeps its x-height.
  AddWords(5, 6, 28.0f, 10.0f, 28.0f, &words);
  for (size_t i = words.size() - 5; i < words.size(); ++i) {
    words[i].has_lower = false;
  }
  SetWordWeights(&words);
  for (size_t i = 0; i < words.size(); ++i) {
    const float expected[] = {0.2f, 0.2f, 0.2f, 0.2f, 6.0f / 20.0f, 8.0f / (0.7f * 28.0f),
                              10.0f / 28.0f};
    EXPECT_FLOAT_EQ(words[i].weight, expected[words[i].row]) << "word " << i;
  }
}

// A wrong x-height which the words of the line do not show is outvoted by
// the lines with the same strokes and the same height of the words.
TEST(FontStyleTest, SameHeightsShareTextSize) {
  std::vector<WordStrokes> words;
  const float x_heights[] = {20.0f, 28.0f, 20.0f, 20.0f, 28.0f};
  for (int row = 0; row < 5; ++row) {
    AddWords(8, row, x_heights[row], 4.0f, 28.0f, &words);
  }
  for (auto &word : words) {
    // Capital letters with accents say nothing about the x-height.
    word.has_lower = false;
  }
  SetWordWeights(&words);
  for (size_t i = 0; i < words.size(); ++i) {
    EXPECT_FLOAT_EQ(words[i].weight, 0.2f) << "word " << i;
  }
}

// The italic of some fonts is lighter than the upright font. Italic words
// are compared with the other italic words then.
TEST(FontStyleTest, ItalicWordsHaveOwnReference) {
  for (int num_italic : {3, 6}) {
    std::vector<WordStrokes> words;
    for (int row = 0; row < 3; ++row) {
      AddWords(10, row, 20.0f, 4.0f, 28.0f, &words);
    }
    for (int i = 0; i < num_italic; ++i) {
      words[3 * i].italic = true;
      words[3 * i].stroke = 3.0f;
    }
    // A bold italic word.
    words[25].italic = true;
    words[25].stroke = 4.6f;
    SetWordWeights(&words);
    // Seven italic words are enough for a reference, four are not.
    const float scale = num_italic == 6 ? 0.75f : 1.0f;
    for (size_t i = 0; i < words.size(); ++i) {
      const float stroke = words[i].italic ? words[i].stroke / scale : words[i].stroke;
      EXPECT_FLOAT_EQ(words[i].weight, stroke / 20.0f) << "word " << i;
    }
  }
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

// A text line for the search of underlines: an image with words of vertical
// bars, which stand for the stems of letters with the x-height kXHeight. The
// baseline rises by the given number of rows from the left to the right.
class UnderlineTest : public testing::Test {
protected:
  static const int kWidth = 600;
  static const int kHeight = 90;
  static const int kXHeight = 20;
  // Row below the stems at the left edge of the image.
  static const int kBaseline = 50;

  void SetUp() override {
    pix_ = pixCreate(kWidth, kHeight, 1);
    ASSERT_TRUE(pix_ != nullptr);
  }

  void TearDown() override {
    pix_.destroy();
  }

  // Sets the baseline. Call this before the words are added.
  void SetRise(int rise) {
    baseline_.resize(kWidth);
    for (int x = 0; x < kWidth; ++x) {
      baseline_[x] = kBaseline - static_cast<float>(rise) * x / (kWidth - 1);
    }
  }

  // Returns the first row below the baseline in the given column.
  int Row(int x) const {
    return static_cast<int>(std::floor(baseline_[x] + 0.5f));
  }

  // Adds a word from the column left to the column right.
  void AddWord(int left, int right) {
    if (baseline_.empty()) {
      SetRise(0);
    }
    for (int stem = left; stem + 3 <= right + 1; stem += 9) {
      for (int x = stem; x < stem + 3; ++x) {
        for (int y = Row(x) - kXHeight; y < Row(x); ++y) {
          pixSetPixel(pix_, x, y, 1);
        }
      }
    }
    words_.emplace_back(left, right);
  }

  // Draws a line from the column left to the column right which is parallel
  // to the baseline. offset is the distance of its upper edge from the first
  // row below the baseline.
  void AddLine(int left, int right, int offset, int thickness) {
    for (int x = left; x <= right; ++x) {
      for (int y = Row(x) + offset; y < Row(x) + offset + thickness; ++y) {
        pixSetPixel(pix_, x, y, 1);
      }
    }
  }

  std::vector<bool> Find() const {
    return FindUnderlinedWords(pix_, baseline_, kXHeight, words_);
  }

  Image pix_;
  std::vector<float> baseline_;
  std::vector<std::pair<int, int>> words_;
};

TEST_F(UnderlineTest, FindsUnderlinedWord) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddWord(280, 380);
  EXPECT_EQ(Find(), std::vector<bool>({false, false, false}));
  AddLine(160, 260, 4, 2);
  EXPECT_EQ(Find(), std::vector<bool>({false, true, false}));
}

// A line below several words underlines all of them, also the short ones.
TEST_F(UnderlineTest, FindsUnderlinedRun) {
  AddWord(60, 140);
  AddWord(160, 172);
  AddWord(190, 290);
  AddWord(310, 400);
  AddLine(160, 290, 5, 3);
  EXPECT_EQ(Find(), std::vector<bool>({false, true, true, false}));
}

// The line of a short word alone could be a part of a letter.
TEST_F(UnderlineTest, IgnoresShortLines) {
  AddWord(60, 140);
  AddWord(160, 172);
  AddWord(190, 290);
  AddLine(160, 172, 4, 2);
  EXPECT_EQ(Find(), std::vector<bool>({false, false, false}));
}

// The baseline of the layout analysis is too low where descenders join the
// text and its underline, so the line can be on it.
TEST_F(UnderlineTest, FindsLineOnBaseline) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddLine(160, 260, -3, 3);
  EXPECT_EQ(Find(), std::vector<bool>({false, true}));
}

TEST_F(UnderlineTest, FindsLineOfSkewedText) {
  SetRise(24);
  AddWord(60, 140);
  AddWord(160, 300);
  AddWord(320, 420);
  AddLine(160, 300, 4, 2);
  EXPECT_EQ(Find(), std::vector<bool>({false, true, false}));
}

// A line which goes on beyond the text is a ruling, for example of a table.
TEST_F(UnderlineTest, RulingIsNoUnderline) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddWord(280, 380);
  AddLine(60, 380, 6, 2);
  EXPECT_EQ(Find(), std::vector<bool>({true, true, true}));
  AddLine(381, 420, 6, 2);
  EXPECT_EQ(Find(), std::vector<bool>({false, false, false}));
}

// The same if the layout analysis added the end of the ruling to a word.
TEST_F(UnderlineTest, RulingInWordIsNoUnderline) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddLine(20, 300, 6, 2);
  words_[0].first = 20;
  words_[1].second = 300;
  EXPECT_EQ(Find(), std::vector<bool>({false, false}));
}

// A full stop has no pixels at the height of the letters, but it is text, so
// the line below it does not go on beyond the text.
TEST_F(UnderlineTest, LowPunctuationIsText) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddLine(272, 275, -4, 4);
  words_[1].second = 275;
  AddLine(160, 275, 4, 2);
  EXPECT_EQ(Find(), std::vector<bool>({false, true}));
}

// The feet of the letters are short lines on the baseline.
TEST_F(UnderlineTest, SerifsAreNoUnderline) {
  AddWord(60, 240);
  for (int stem = 60; stem + 3 <= 241; stem += 9) {
    AddLine(stem - 2, stem + 4, -2, 2);
  }
  EXPECT_EQ(Find(), std::vector<bool>({false}));
}

// Neither is the bar of a wide letter, which is shorter than a line.
TEST_F(UnderlineTest, LetterBarIsNoUnderline) {
  AddWord(60, 84);
  AddLine(60, 84, -3, 3);
  EXPECT_EQ(Find(), std::vector<bool>({false}));
}

// Text on a dark background has no line below it.
TEST_F(UnderlineTest, DarkBackgroundIsNoUnderline) {
  AddWord(60, 140);
  AddWord(160, 260);
  AddLine(60, 260, 2, 30);
  EXPECT_EQ(Find(), std::vector<bool>({false, false}));
}

TEST_F(UnderlineTest, RejectsUnusableInput) {
  AddWord(60, 140);
  AddLine(60, 140, 4, 2);
  EXPECT_EQ(Find(), std::vector<bool>({true}));
  EXPECT_EQ(FindUnderlinedWords(nullptr, baseline_, kXHeight, words_), std::vector<bool>({false}));
  EXPECT_EQ(FindUnderlinedWords(pix_, baseline_, 0.0f, words_), std::vector<bool>({false}));
  baseline_.pop_back();
  EXPECT_EQ(Find(), std::vector<bool>({false}));
  Image grey = pixCreate(kWidth, kHeight, 8);
  baseline_.push_back(kBaseline);
  EXPECT_EQ(FindUnderlinedWords(grey, baseline_, kXHeight, words_), std::vector<bool>({false}));
  grey.destroy();
  words_.clear();
  EXPECT_TRUE(Find().empty());
}

// A word with the font attributes which the API reports for it.
struct StyledWord {
  std::string text;
  // Index of the text line of the word in the unchanged image, or -1.
  int line = -1;
  bool bold = false;
  bool italic = false;
  bool underlined = false;
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
      bool monospace, serif, smallcaps;
      word.has_font_name =
          it->WordFontAttributes(&word.bold, &word.italic, &word.underlined, &monospace, &serif,
                                 &smallcaps, &word.pointsize, &word.font_id) != nullptr;
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

// The words of a page which lies on its side must get the same attributes as
// those of the upright page.
TEST_F(FontStyleApiTest, FindsStylesInRotatedPage) {
  const int kItalicLine = 3;
  const int kBoldLine = 5;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  std::vector<StyledWord> upright = Recognize();
  ASSERT_EQ(line_boxes_.size(), 8u);
  ChangeLine(kItalicLine, [](Image line) {
    return Image(pixHShear(nullptr, line, pixGetHeight(line) / 2, std::atan(0.25f),
                           L_BRING_IN_WHITE));
  });
  ChangeLine(kBoldLine, [](Image line) { return Image(pixDilateBrick(nullptr, line, 3, 3)); });
  upright = Recognize();
  // The layout analysis finds the vertical text lines of a page which is
  // turned clockwise by a quarter without a detection of the orientation.
  Image turned = pixRotateOrth(pix_, 1);
  ASSERT_TRUE(turned != nullptr);
  pix_.destroy();
  pix_ = turned;
  const std::vector<StyledWord> rotated = Recognize();
  ASSERT_EQ(rotated.size(), upright.size());
  int num_italic = 0;
  int num_bold = 0;
  for (size_t i = 0; i < rotated.size(); ++i) {
    EXPECT_EQ(rotated[i].text, upright[i].text);
    EXPECT_EQ(rotated[i].italic, upright[i].italic) << rotated[i].text;
    EXPECT_EQ(rotated[i].bold, upright[i].bold) << rotated[i].text;
    num_italic += rotated[i].italic;
    num_bold += rotated[i].bold;
  }
  EXPECT_GE(num_italic, 6);
  EXPECT_GE(num_bold, 6);
}

// The words of a text line with a line below it must be underlined, with and
// without a layout analysis, which removes long lines from the image.
TEST_F(FontStyleApiTest, FindsUnderlinedLine) {
  const int kLine = 3;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  std::vector<StyledWord> words = Recognize();
  ASSERT_EQ(line_boxes_.size(), 8u);
  for (const auto &word : words) {
    EXPECT_FALSE(word.underlined) << word.text;
  }
  // The box of the text line ends with its descenders.
  const std::array<int, 4> &box = line_boxes_[kLine];
  for (int y = box[3] - 4; y < box[3] - 1; ++y) {
    for (int x = box[0]; x < box[2]; ++x) {
      pixSetPixel(pix_, x, y, 1);
    }
  }
  for (const PageSegMode mode : {PSM_AUTO, PSM_SINGLE_BLOCK}) {
    api_.SetPageSegMode(mode);
    words = Recognize();
    int num_underlined = 0;
    for (const auto &word : words) {
      if (word.line == kLine) {
        num_underlined += word.underlined;
      } else {
        EXPECT_FALSE(word.underlined) << word.text;
      }
    }
    EXPECT_GE(num_underlined, 6) << "mode " << mode;
  }
}

// A ruling below a text line goes on beyond the text and is no underline.
TEST_F(FontStyleApiTest, RulingIsNoUnderline) {
  const int kLine = 3;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  std::vector<StyledWord> words = Recognize();
  ASSERT_EQ(line_boxes_.size(), 8u);
  const std::array<int, 4> &box = line_boxes_[kLine];
  ASSERT_GT(box[0], 20);
  ASSERT_LT(box[2] + 20, pixGetWidth(pix_));
  for (int y = box[3] - 4; y < box[3] - 1; ++y) {
    for (int x = 4; x < pixGetWidth(pix_) - 4; ++x) {
      pixSetPixel(pix_, x, y, 1);
    }
  }
  for (const PageSegMode mode : {PSM_AUTO, PSM_SINGLE_BLOCK}) {
    api_.SetPageSegMode(mode);
    words = Recognize();
    ASSERT_GT(words.size(), 40u);
    for (const auto &word : words) {
      EXPECT_FALSE(word.underlined) << word.text;
    }
  }
}

// Larger text has wider strokes, but it is not bold.
TEST_F(FontStyleApiTest, LargerTextIsNotBold) {
  const int kBoldLine = 5;
  ASSERT_TRUE(api_.SetVariable("lstm_font_attributes", "1"));
  // Add a larger copy of the page below the page.
  Image large = pixScale(pix_, 1.4f, 1.4f);
  ASSERT_TRUE(large != nullptr);
  const int height = pixGetHeight(pix_);
  Image page = pixCreate(pixGetWidth(large), height + pixGetHeight(large), pixGetDepth(pix_));
  ASSERT_TRUE(page != nullptr);
  pixSetBlackOrWhite(page, L_SET_WHITE);
  pixRasterop(page, 0, 0, pixGetWidth(pix_), height, PIX_SRC, pix_, 0, 0);
  pixRasterop(page, 0, height, pixGetWidth(large), pixGetHeight(large), PIX_SRC, large, 0, 0);
  large.destroy();
  pix_.destroy();
  pix_ = page;
  std::vector<StyledWord> words = Recognize();
  ASSERT_EQ(line_boxes_.size(), 16u);
  for (const auto &word : words) {
    EXPECT_FALSE(word.bold) << word.text << " in line " << word.line;
  }
  ChangeLine(kBoldLine, [](Image line) { return Image(pixDilateBrick(nullptr, line, 3, 3)); });
  words = Recognize();
  int num_bold = 0;
  for (const auto &word : words) {
    EXPECT_EQ(word.bold, word.line == kBoldLine) << word.text << " in line " << word.line;
    num_bold += word.bold;
  }
  EXPECT_GE(num_bold, 6);
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
