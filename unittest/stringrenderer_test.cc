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

#include "boxchar.h"
#include "boxread.h"
#include "commandlineflags.h"
#include "stringrenderer.h"

#include <allheaders.h>

#include <memory>
#include <string>

BOOL_PARAM_FLAG(display, false, "Display image for inspection");

namespace tesseract {

const char kEngText[] = "the quick brown fox jumps over the lazy dog";
const char kHinText[] = "पिताने विवाह की | हो गई उद्विग्न वह सोचा";

const char kKorText[] = "이는 것으로 다시 넣을 1234 수는 있지만 선택의 의미는";
const char kArabicText[] =
    "والفكر والصراع ، بالتأمل والفهم والتحليل ، "
    "بالعلم والفن ، وأخيرا بالضحك أوبالبكاء ، ";
const char kMixedText[] = "والفكر 123 والصراع abc";

const char kEngNonLigatureText[] = "fidelity";
// Same as kEngNonLigatureText, but with "fi" replaced with its ligature.
const char kEngLigatureText[] = "ﬁdelity";

static PangoFontMap *font_map;

class StringRendererTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (!font_map) {
      font_map = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
    }
    pango_cairo_font_map_set_default(PANGO_CAIRO_FONT_MAP(font_map));
  }

  static void SetUpTestCase() {
    static std::locale system_locale("");
    std::locale::global(system_locale);

    l_chooseDisplayProg(L_DISPLAY_WITH_XZGV);
    FLAGS_fonts_dir = TESTING_DIR;
    FLAGS_fontconfig_tmpdir = FLAGS_test_tmpdir;
    file::MakeTmpdir();
    PangoFontInfo::SoftInitFontConfig(); // init early
  }

  void DisplayClusterBoxes(Image pix) {
    if (!FLAGS_display) {
      return;
    }
    const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
    Boxa *boxes = boxaCreate(0);
    for (const auto &boxchar : boxchars) {
      if (boxchar->box()) {
        boxaAddBox(boxes, const_cast<Box *>(boxchar->box()), L_CLONE);
      }
    }
    Image box_pix = pixDrawBoxaRandom(pix, boxes, 1);
    boxaDestroy(&boxes);
    pixDisplay(box_pix, 0, 0);
    box_pix.destroy();
  }
  std::unique_ptr<StringRenderer> renderer_;
};

TEST_F(StringRendererTest, DoesRenderToImage) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();

  renderer_ = std::make_unique<StringRenderer>("UnBatang 10", 600, 600);
  EXPECT_EQ(strlen(kKorText), renderer_->RenderToImage(kKorText, strlen(kKorText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();

  renderer_ = std::make_unique<StringRenderer>("Lohit Hindi 10", 600, 600);
  EXPECT_EQ(strlen(kHinText), renderer_->RenderToImage(kHinText, strlen(kHinText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();

  // RTL text
  renderer_ = std::make_unique<StringRenderer>("Arab 10", 600, 600);
  EXPECT_EQ(strlen(kArabicText), renderer_->RenderToImage(kArabicText, strlen(kArabicText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();

  // Mixed direction Arabic + english text
  renderer_ = std::make_unique<StringRenderer>("Arab 10", 600, 600);
  EXPECT_EQ(strlen(kMixedText), renderer_->RenderToImage(kMixedText, strlen(kMixedText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();
}

TEST_F(StringRendererTest, DoesRenderToImageWithUnderline) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  // Underline all words but NOT intervening spaces.
  renderer_->set_underline_start_prob(1.0);
  renderer_->set_underline_continuation_prob(0);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();
  renderer_->ClearBoxes();

  // Underline all words AND intervening spaces.
  renderer_->set_underline_start_prob(1.0);
  renderer_->set_underline_continuation_prob(1.0);
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();
  renderer_->ClearBoxes();

  // Underline words and intervening spaces with 0.5 prob.
  renderer_->set_underline_start_prob(0.5);
  renderer_->set_underline_continuation_prob(0.5);
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();
}

TEST_F(StringRendererTest, DoesHandleNewlineCharacters) {
  const char kRawText[] = "\n\n\n A \nB \nC \n\n\n";
  const char kStrippedText[] = " A B C "; // text with newline chars removed
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kRawText), renderer_->RenderToImage(kRawText, strlen(kRawText), &pix));
  EXPECT_TRUE(pix != nullptr);
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  // 3 characters + 4 spaces => 7 boxes
  EXPECT_EQ(7, boxchars.size());
  if (boxchars.size() == 7) {
    // Verify the text content of the boxchars
    for (size_t i = 0; i < boxchars.size(); ++i) {
      EXPECT_EQ(std::string(1, kStrippedText[i]), boxchars[i]->ch());
    }
  }
  DisplayClusterBoxes(pix);
  pix.destroy();
}

TEST_F(StringRendererTest, DoesRenderLigatures) {
  renderer_ = std::make_unique<StringRenderer>("Arab 12", 600, 250);
  const char kArabicLigature[] = "لا";

  Image pix = nullptr;
  EXPECT_EQ(strlen(kArabicLigature),
            renderer_->RenderToImage(kArabicLigature, strlen(kArabicLigature), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const std::vector<BoxChar *> &boxes = renderer_->GetBoxes();
  EXPECT_EQ(1, boxes.size());
  EXPECT_TRUE(boxes[0]->box() != nullptr);
  EXPECT_STREQ(kArabicLigature, boxes[0]->ch().c_str());
  DisplayClusterBoxes(pix);
  pix.destroy();

  renderer_ = std::make_unique<StringRenderer>("Arab 12", 600, 250);
  const char kArabicMixedText[] = "والفكر والصراع 1234,\nوالفكر لا والصراع";
  renderer_->RenderToImage(kArabicMixedText, strlen(kArabicMixedText), &pix);
  DisplayClusterBoxes(pix);
  pix.destroy();
}

static int FindBoxCharXCoord(const std::vector<BoxChar *> &boxchars, const std::string &ch) {
  for (const auto &boxchar : boxchars) {
    if (boxchar->ch() == ch) {
      return boxchar->box()->x;
    }
  }
  return INT_MAX;
}

TEST_F(StringRendererTest, ArabicBoxcharsInLTROrder) {
  renderer_ = std::make_unique<StringRenderer>("Arab 10", 600, 600);
  Image pix = nullptr;
  // Arabic letters should be in decreasing x-coordinates
  const char kArabicWord[] = "\u0644\u0627\u0641\u0643\u0631";
  const std::string kRevWord = "\u0631\u0643\u0641\u0627\u0644";
  renderer_->RenderToImage(kArabicWord, strlen(kArabicWord), &pix);
  std::string boxes_str = renderer_->GetBoxesStr();
  // Decode to get the box text strings.
  EXPECT_FALSE(boxes_str.empty());
  std::vector<std::string> texts;
  EXPECT_TRUE(ReadMemBoxes(0, false, boxes_str.c_str(), false, nullptr, &texts, nullptr, nullptr));
  std::string ltr_str;
  for (auto &text : texts) {
    ltr_str += text.c_str();
  }
  // The string should come out perfectly reversed, despite there being a
  // ligature.
  EXPECT_EQ(ltr_str, kRevWord);
  // Just to prove there was a ligature, the number of texts is less than the
  // number of unicodes.
  EXPECT_LT(texts.size(), 5);
  pix.destroy();
}

TEST_F(StringRendererTest, DoesOutputBoxcharsInReadingOrder) {
  renderer_ = std::make_unique<StringRenderer>("Arab 10", 600, 600);
  Image pix = nullptr;
  // Arabic letters should be in decreasing x-coordinates
  const char kArabicWord[] = "والفكر";
  renderer_->RenderToImage(kArabicWord, strlen(kArabicWord), &pix);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  for (size_t i = 1; i < boxchars.size(); ++i) {
    EXPECT_GT(boxchars[i - 1]->box()->x, boxchars[i]->box()->x) << boxchars[i - 1]->ch();
  }
  pix.destroy();

  // English letters should be in increasing x-coordinates
  const char kEnglishWord[] = "Google";
  renderer_->ClearBoxes();
  renderer_->RenderToImage(kEnglishWord, strlen(kEnglishWord), &pix);
  EXPECT_EQ(boxchars.size(), strlen(kEnglishWord));
  for (size_t i = 1; i < boxchars.size(); ++i) {
    EXPECT_LT(boxchars[i - 1]->box()->x, boxchars[i]->box()->x) << boxchars[i - 1]->ch();
  }
  pix.destroy();

  // Mixed text should satisfy both.
  renderer_->ClearBoxes();
  renderer_->RenderToImage(kMixedText, strlen(kMixedText), &pix);
  EXPECT_LT(FindBoxCharXCoord(boxchars, "a"), FindBoxCharXCoord(boxchars, "b"));
  EXPECT_LT(FindBoxCharXCoord(boxchars, "1"), FindBoxCharXCoord(boxchars, "2"));
  EXPECT_GT(FindBoxCharXCoord(boxchars, "و"), FindBoxCharXCoord(boxchars, "ر"));
  pix.destroy();
}

TEST_F(StringRendererTest, DoesRenderVerticalText) {
  Image pix = nullptr;
  renderer_ = std::make_unique<StringRenderer>("UnBatang 10", 600, 600);
  renderer_->set_vertical_text(true);
  EXPECT_EQ(strlen(kKorText), renderer_->RenderToImage(kKorText, strlen(kKorText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pix.destroy();
}

// Checks that we preserve charboxes across RenderToImage calls, with
// appropriate page numbers.
TEST_F(StringRendererTest, DoesKeepAllImageBoxes) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  Image pix = nullptr;
  int num_boxes_per_page = 0;
  const int kNumTrials = 2;
  for (int i = 0; i < kNumTrials; ++i) {
    EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
    EXPECT_TRUE(pix != nullptr);
    pix.destroy();
    EXPECT_GT(renderer_->GetBoxes().size(), 0);
    if (!num_boxes_per_page) {
      num_boxes_per_page = renderer_->GetBoxes().size();
    } else {
      EXPECT_EQ((i + 1) * num_boxes_per_page, renderer_->GetBoxes().size());
    }
    for (int j = i * num_boxes_per_page; j < (i + 1) * num_boxes_per_page; ++j) {
      EXPECT_EQ(i, renderer_->GetBoxes()[j]->page());
    }
  }
}

TEST_F(StringRendererTest, DoesClearBoxes) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pix.destroy();
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const int num_boxes_per_page = renderer_->GetBoxes().size();

  renderer_->ClearBoxes();
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pix.destroy();
  EXPECT_EQ(num_boxes_per_page, renderer_->GetBoxes().size());
}

TEST_F(StringRendererTest, DoesLigatureTextForRendering) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  renderer_->set_add_ligatures(true);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngNonLigatureText),
            renderer_->RenderToImage(kEngNonLigatureText, strlen(kEngNonLigatureText), &pix));
  pix.destroy();
#if 0 // not with NFC normalization
  // There should be one less box than letters due to the 'fi' ligature.
  EXPECT_EQ(strlen(kEngNonLigatureText) - 1, renderer_->GetBoxes().size());
  // The output box text should be ligatured.
  EXPECT_STREQ("ﬁ", renderer_->GetBoxes()[0]->ch().c_str());
#endif
}

TEST_F(StringRendererTest, DoesRetainInputLigatureForRendering) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngLigatureText),
            renderer_->RenderToImage(kEngLigatureText, strlen(kEngLigatureText), &pix));
  pix.destroy();
  // There should be one less box than letters due to the 'fi' ligature.
  EXPECT_EQ(strlen(kEngNonLigatureText) - 1, renderer_->GetBoxes().size());
  // The output box text should be ligatured.
  EXPECT_STREQ("\uFB01", renderer_->GetBoxes()[0]->ch().c_str());
}

TEST_F(StringRendererTest, DoesStripUnrenderableWords) {
  // Verdana should only be able to render the english letters and numbers in
  // the mixed text.
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  std::string text(kMixedText);
  EXPECT_GT(renderer_->StripUnrenderableWords(&text), 0);
  EXPECT_EQ(" 123  abc", text);
}

TEST_F(StringRendererTest, DoesRenderWordBoxes) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  renderer_->set_output_word_boxes(true);
  Image pix = nullptr;
  EXPECT_EQ(strlen(kEngText), renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pix.destroy();
  // Verify #boxchars = #words + #spaces
  std::vector<std::string> words = split(kEngText, ' ');
  const int kNumSpaces = words.size() - 1;
  const int kExpectedNumBoxes = words.size() + kNumSpaces;
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kExpectedNumBoxes, boxchars.size());
  // Verify content of words and spaces
  for (size_t i = 0; i < boxchars.size(); i += 2) {
    EXPECT_EQ(words[i / 2], boxchars[i]->ch());
    if (i < boxchars.size() - 1) {
      EXPECT_EQ(" ", boxchars[i + 1]->ch());
      EXPECT_TRUE(boxchars[i + 1]->box() == nullptr);
    }
  }
}

TEST_F(StringRendererTest, DoesRenderWordBoxesFromMultiLineText) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 600, 600);
  renderer_->set_output_word_boxes(true);
  Image pix = nullptr;
  const char kMultlineText[] = "the quick brown fox\njumps over the lazy dog";
  EXPECT_EQ(strlen(kMultlineText), renderer_->RenderToImage(kMultlineText, strlen(kEngText), &pix));
  pix.destroy();
  // Verify #boxchars = #words + #spaces + #newlines
  std::vector<std::string> words;
  for (auto &line : split(kMultlineText, '\n')) {
    for (auto &word : split(line, ' ')) {
      words.push_back(word);
    }
  }
  const int kNumSeparators = words.size() - 1;
  const int kExpectedNumBoxes = words.size() + kNumSeparators;
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kExpectedNumBoxes, boxchars.size());
  // Verify content of words and spaces
  for (size_t i = 0; i < boxchars.size(); i += 2) {
    EXPECT_EQ(words[i / 2], boxchars[i]->ch());
    if (i + 1 < boxchars.size()) {
      EXPECT_EQ(" ", boxchars[i + 1]->ch());
      EXPECT_TRUE(boxchars[i + 1]->box() == nullptr);
    }
  }
}

TEST_F(StringRendererTest, DoesRenderAllFontsToImage) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 1200, 1200);
  size_t offset = 0;
  std::string font_used;
  do {
    Image pix = nullptr;
    font_used.clear();
    offset += renderer_->RenderAllFontsToImage(1.0, kEngText + offset, strlen(kEngText + offset),
                                               &font_used, &pix);
    if (offset < strlen(kEngText)) {
      EXPECT_TRUE(pix != nullptr);
      EXPECT_STRNE("", font_used.c_str());
    }
    if (FLAGS_display) {
      pixDisplay(pix, 0, 0);
    }
    pix.destroy();
  } while (offset < strlen(kEngText));
}

TEST_F(StringRendererTest, DoesNotRenderWordJoiner) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 500, 200);
  const std::string word = "A- -B C-D A BC";
  const std::string joined_word = StringRenderer::InsertWordJoiners(word);
  Image pix = nullptr;
  renderer_->RenderToImage(joined_word.c_str(), joined_word.length(), &pix);
  pix.destroy();
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  const std::string kWordJoinerUTF8 = "\u2060";
  ASSERT_EQ(word.length(), boxchars.size());
  for (size_t i = 0; i < boxchars.size(); ++i) {
    EXPECT_NE(kWordJoinerUTF8, boxchars[i]->ch());
    EXPECT_EQ(word.substr(i, 1), boxchars[i]->ch());
  }
}

TEST_F(StringRendererTest, DISABLED_DoesDropUncoveredChars) {
  renderer_ = std::make_unique<StringRenderer>("Verdana 10", 500, 200);
  renderer_->set_drop_uncovered_chars(true);
  const std::string kWord = "oﬀice";
  const std::string kCleanWord = "oice";
  Image pix = nullptr;
  EXPECT_FALSE(renderer_->font().CanRenderString(kWord.c_str(), kWord.length()));
  EXPECT_FALSE(renderer_->font().CoversUTF8Text(kWord.c_str(), kWord.length()));
  int offset = renderer_->RenderToImage(kWord.c_str(), kWord.length(), &pix);
  pix.destroy();
  const std::vector<BoxChar *> &boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kWord.length(), offset);
  ASSERT_EQ(kCleanWord.length(), boxchars.size());
  for (size_t i = 0; i < boxchars.size(); ++i) {
    EXPECT_EQ(kCleanWord.substr(i, 1), boxchars[i]->ch());
  }
}

// ------------ StringRenderer::ConvertBasicLatinToFullwidthLatin() ------------

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesConvertBasicLatin) {
  const std::string kHalfAlpha = "ABCD";
  const std::string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kFullAlpha, StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfAlpha));

  const std::string kHalfDigit = "0123";
  const std::string kFullDigit = "０１２３";
  EXPECT_EQ(kFullDigit, StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfDigit));

  const std::string kHalfSym = "()[]:;!?";
  const std::string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kFullSym, StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfSym));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertFullwidthLatin) {
  const std::string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kFullAlpha, StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullAlpha));

  const std::string kFullDigit = "０１２３";
  EXPECT_EQ(kFullDigit, StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullDigit));

  const std::string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kFullSym, StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullSym));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertNonLatin) {
  const std::string kHalfKana = "ｱｲｳｴｵ";
  const std::string kFullKana = "アイウエオ";
  EXPECT_EQ(kHalfKana, StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfKana));
  EXPECT_EQ(kFullKana, StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullKana));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertSpace) {
  const std::string kHalfSpace = " ";
  const std::string kFullSpace = "　";
  EXPECT_EQ(kHalfSpace, StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfSpace));
  EXPECT_EQ(kFullSpace, StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullSpace));
}

// ------------ StringRenderer::ConvertFullwidthLatinToBasicLatin() ------------

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesConvertFullwidthLatin) {
  const std::string kHalfAlpha = "ABCD";
  const std::string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kHalfAlpha, StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullAlpha));

  const std::string kHalfDigit = "0123";
  const std::string kFullDigit = "０１２３";
  EXPECT_EQ(kHalfDigit, StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullDigit));

  const std::string kHalfSym = "()[]:;!?";
  const std::string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kHalfSym, StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullSym));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertBasicLatin) {
  const std::string kHalfAlpha = "ABCD";
  EXPECT_EQ(kHalfAlpha, StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfAlpha));

  const std::string kHalfDigit = "0123";
  EXPECT_EQ(kHalfDigit, StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfDigit));

  const std::string kHalfSym = "()[]:;!?";
  EXPECT_EQ(kHalfSym, StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfSym));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertNonLatin) {
  const std::string kHalfKana = "ｱｲｳｴｵ";
  const std::string kFullKana = "アイウエオ";
  EXPECT_EQ(kHalfKana, StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfKana));
  EXPECT_EQ(kFullKana, StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullKana));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertSpace) {
  const std::string kHalfSpace = " ";
  const std::string kFullSpace = "　";
  EXPECT_EQ(kHalfSpace, StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfSpace));
  EXPECT_EQ(kFullSpace, StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullSpace));
}
} // namespace tesseract
