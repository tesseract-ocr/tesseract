#include <memory>

#include "tesseract/training/stringrenderer.h"

#include <string.h>

#include "leptonica/include/allheaders.h"
#include "tesseract/ccstruct/boxread.h"
#include "tesseract/ccutil/genericvector.h"
#include "tesseract/ccutil/strngs.h"
#include "tesseract/training/boxchar.h"
#include "tesseract/training/commandlineflags.h"

DEFINE_bool(display, false, "Display image for inspection");

// Flags defined in pango_font_info.cpp
DECLARE_BOOL_PARAM_FLAG(use_only_legacy_fonts);
DECLARE_STRING_PARAM_FLAG(fonts_dir);
DECLARE_STRING_PARAM_FLAG(fontconfig_tmpdir);

namespace {

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

using tesseract::BoxChar;
using tesseract::StringRenderer;

class StringRendererTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    l_chooseDisplayProg(L_DISPLAY_WITH_XZGV);
    FLAGS_fonts_dir = file::JoinPath(FLAGS_test_srcdir, "testdata");
    FLAGS_fontconfig_tmpdir = FLAGS_test_tmpdir;
    FLAGS_use_only_legacy_fonts = false;
    // Needed for reliable heapchecking of pango layout structures.
    FLAGS_heap_check_max_pointer_offset = -1;
  }

  void DisplayClusterBoxes(Pix* pix) {
    if (!FLAGS_display) return;
    const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
    Boxa* boxes = boxaCreate(0);
    for (int i = 0; i < boxchars.size(); ++i) {
      if (boxchars[i]->box())
        boxaAddBox(boxes, const_cast<Box*>(boxchars[i]->box()), L_CLONE);
    }
    Pix* box_pix = pixDrawBoxaRandom(pix, boxes, 1);
    boxaDestroy(&boxes);
    pixDisplay(box_pix, 0, 0);
    pixDestroy(&box_pix);
  }
  std::unique_ptr<StringRenderer> renderer_;
};

TEST_F(StringRendererTest, DoesRenderToImage) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);

  renderer_.reset(new StringRenderer("UnBatang 10", 600, 600));
  EXPECT_EQ(strlen(kKorText),
            renderer_->RenderToImage(kKorText, strlen(kKorText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);

  renderer_.reset(new StringRenderer("Lohit Hindi 10", 600, 600));
  EXPECT_EQ(strlen(kHinText),
            renderer_->RenderToImage(kHinText, strlen(kHinText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);

  // RTL text
  renderer_.reset(new StringRenderer("Arab 10", 600, 600));
  EXPECT_EQ(strlen(kArabicText),
            renderer_->RenderToImage(kArabicText, strlen(kArabicText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);

  // Mixed direction Arabic + english text
  renderer_.reset(new StringRenderer("Arab 10", 600, 600));
  EXPECT_EQ(strlen(kMixedText),
            renderer_->RenderToImage(kMixedText, strlen(kMixedText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
}

TEST_F(StringRendererTest, DoesRenderToImageWithUnderline) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  // Underline all words but NOT intervening spaces.
  renderer_->set_underline_start_prob(1.0);
  renderer_->set_underline_continuation_prob(0);
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
  renderer_->ClearBoxes();

  // Underline all words AND intervening spaces.
  renderer_->set_underline_start_prob(1.0);
  renderer_->set_underline_continuation_prob(1.0);
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
  renderer_->ClearBoxes();

  // Underline words and intervening spaces with 0.5 prob.
  renderer_->set_underline_start_prob(0.5);
  renderer_->set_underline_continuation_prob(0.5);
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
}

TEST_F(StringRendererTest, DoesHandleNewlineCharacters) {
  const char kRawText[] = "\n\n\n A \nB \nC \n\n\n";
  const char kStrippedText[] = " A B C ";  // text with newline chars removed
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kRawText),
            renderer_->RenderToImage(kRawText, strlen(kRawText), &pix));
  EXPECT_TRUE(pix != nullptr);
  // 3 characters + 4 spaces => 7 boxes
  EXPECT_EQ(7, renderer_->GetBoxes().size());
  // Verify the text content of the boxchars
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  for (int i = 0; i < strlen(kStrippedText); ++i) {
    EXPECT_EQ(string(1, kStrippedText[i]), boxchars[i]->ch());
  }
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
}

TEST_F(StringRendererTest, DoesRenderLigatures) {
  renderer_.reset(new StringRenderer("Arab 12", 600, 250));
  const char kArabicLigature[] = "لا";

  Pix* pix = nullptr;
  EXPECT_EQ(
      strlen(kArabicLigature),
      renderer_->RenderToImage(kArabicLigature, strlen(kArabicLigature), &pix));
  EXPECT_TRUE(pix != nullptr);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const std::vector<BoxChar*>& boxes = renderer_->GetBoxes();
  EXPECT_EQ(1, boxes.size());
  EXPECT_TRUE(boxes[0]->box() != nullptr);
  EXPECT_STREQ(kArabicLigature, boxes[0]->ch().c_str());
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);

  renderer_.reset(new StringRenderer("Arab 12", 600, 250));
  const char kArabicMixedText[] = "والفكر والصراع 1234,\nوالفكر لا والصراع";
  renderer_->RenderToImage(kArabicMixedText, strlen(kArabicMixedText), &pix);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
}

static int FindBoxCharXCoord(const std::vector<BoxChar*>& boxchars,
                             const string& ch) {
  for (int i = 0; i < boxchars.size(); ++i) {
    if (boxchars[i]->ch() == ch) return boxchars[i]->box()->x;
  }
  return kint32max;
}

TEST_F(StringRendererTest, ArabicBoxcharsInLTROrder) {
  renderer_.reset(new StringRenderer("Arab 10", 600, 600));
  Pix* pix = nullptr;
  // Arabic letters should be in decreasing x-coordinates
  const char kArabicWord[] = "\u0644\u0627\u0641\u0643\u0631";
  const string kRevWord = "\u0631\u0643\u0641\u0627\u0644";
  renderer_->RenderToImage(kArabicWord, strlen(kArabicWord), &pix);
  string boxes_str = renderer_->GetBoxesStr();
  // Decode to get the box text strings.
  EXPECT_FALSE(boxes_str.empty());
  GenericVector<STRING> texts;
  EXPECT_TRUE(ReadMemBoxes(0, false, boxes_str.c_str(), nullptr, &texts,
                           nullptr, nullptr));
  string ltr_str;
  for (int i = 0; i < texts.size(); ++i)
    absl::StrAppend(&ltr_str, texts[i].string());
  // The string should come out perfectly reversed, despite there being a
  // ligature.
  EXPECT_EQ(ltr_str, kRevWord);
  // Just to prove there was a ligature, the number of texts is less than the
  // number of unicodes.
  EXPECT_LT(texts.size(), 5);
  pixDestroy(&pix);
}

TEST_F(StringRendererTest, DoesOutputBoxcharsInReadingOrder) {
  renderer_.reset(new StringRenderer("Arab 10", 600, 600));
  Pix* pix = nullptr;
  // Arabic letters should be in decreasing x-coordinates
  const char kArabicWord[] = "والفكر";
  renderer_->RenderToImage(kArabicWord, strlen(kArabicWord), &pix);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  for (int i = 0; i < boxchars.size() - 1; ++i) {
    EXPECT_GT(boxchars[i]->box()->x, boxchars[i + 1]->box()->x)
        << boxchars[i]->ch();
  }
  pixDestroy(&pix);

  // English letters should be in increasing x-coordinates
  const char kEnglishWord[] = "Google";
  renderer_->ClearBoxes();
  renderer_->RenderToImage(kEnglishWord, strlen(kEnglishWord), &pix);
  EXPECT_EQ(boxchars.size(), strlen(kEnglishWord));
  for (int i = 0; i < boxchars.size() - 1; ++i) {
    EXPECT_LT(boxchars[i]->box()->x, boxchars[i + 1]->box()->x)
        << boxchars[i]->ch();
  }
  pixDestroy(&pix);

  // Mixed text should satisfy both.
  renderer_->ClearBoxes();
  renderer_->RenderToImage(kMixedText, strlen(kMixedText), &pix);
  CHECK_LT(FindBoxCharXCoord(boxchars, "a"), FindBoxCharXCoord(boxchars, "b"));
  CHECK_LT(FindBoxCharXCoord(boxchars, "1"), FindBoxCharXCoord(boxchars, "2"));
  CHECK_GT(FindBoxCharXCoord(boxchars, "و"), FindBoxCharXCoord(boxchars, "ر"));
  pixDestroy(&pix);
}

TEST_F(StringRendererTest, DoesRenderVerticalText) {
  Pix* pix = nullptr;
  renderer_.reset(new StringRenderer("UnBatang 10", 600, 600));
  renderer_->set_vertical_text(true);
  EXPECT_EQ(strlen(kKorText),
            renderer_->RenderToImage(kKorText, strlen(kKorText), &pix));
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  DisplayClusterBoxes(pix);
  pixDestroy(&pix);
}

// Checks that we preserve charboxes across RenderToImage calls, with
// appropriate page numbers.
TEST_F(StringRendererTest, DoesKeepAllImageBoxes) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  Pix* pix = nullptr;
  int num_boxes_per_page = 0;
  const int kNumTrials = 2;
  for (int i = 0; i < kNumTrials; ++i) {
    EXPECT_EQ(strlen(kEngText),
              renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
    EXPECT_TRUE(pix != nullptr);
    pixDestroy(&pix);
    EXPECT_GT(renderer_->GetBoxes().size(), 0);
    if (!num_boxes_per_page) {
      num_boxes_per_page = renderer_->GetBoxes().size();
    } else {
      EXPECT_EQ((i + 1) * num_boxes_per_page, renderer_->GetBoxes().size());
    }
    for (int j = i * num_boxes_per_page; j < (i + 1) * num_boxes_per_page;
         ++j) {
      EXPECT_EQ(i, renderer_->GetBoxes()[j]->page());
    }
  }
}

TEST_F(StringRendererTest, DoesClearBoxes) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pixDestroy(&pix);
  EXPECT_GT(renderer_->GetBoxes().size(), 0);
  const int num_boxes_per_page = renderer_->GetBoxes().size();

  renderer_->ClearBoxes();
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pixDestroy(&pix);
  EXPECT_EQ(num_boxes_per_page, renderer_->GetBoxes().size());
}

TEST_F(StringRendererTest, DoesLigatureTextForRendering) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  renderer_->set_add_ligatures(true);
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngNonLigatureText),
            renderer_->RenderToImage(kEngNonLigatureText,
                                     strlen(kEngNonLigatureText), &pix));
  pixDestroy(&pix);
  // There should be one less box than letters due to the 'fi' ligature.
  EXPECT_EQ(strlen(kEngNonLigatureText) - 1, renderer_->GetBoxes().size());
  // The output box text should be ligatured.
  EXPECT_STREQ("ﬁ", renderer_->GetBoxes()[0]->ch().c_str());
}

TEST_F(StringRendererTest, DoesRetainInputLigatureForRendering) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngLigatureText),
            renderer_->RenderToImage(kEngLigatureText, strlen(kEngLigatureText),
                                     &pix));
  pixDestroy(&pix);
  // There should be one less box than letters due to the 'fi' ligature.
  EXPECT_EQ(strlen(kEngNonLigatureText) - 1, renderer_->GetBoxes().size());
  // The output box text should be ligatured.
  EXPECT_STREQ("\uFB01", renderer_->GetBoxes()[0]->ch().c_str());
}

TEST_F(StringRendererTest, DoesStripUnrenderableWords) {
  // Verdana should only be able to render the english letters and numbers in
  // the mixed text.
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  string text(kMixedText);
  EXPECT_GT(renderer_->StripUnrenderableWords(&text), 0);
  EXPECT_EQ(" 123  abc", text);
}

TEST_F(StringRendererTest, DoesRenderWordBoxes) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  renderer_->set_output_word_boxes(true);
  Pix* pix = nullptr;
  EXPECT_EQ(strlen(kEngText),
            renderer_->RenderToImage(kEngText, strlen(kEngText), &pix));
  pixDestroy(&pix);
  // Verify #boxchars = #words + #spaces
  std::vector<string> words = absl::StrSplit(kEngText, ' ', absl::SkipEmpty());
  const int kNumSpaces = words.size() - 1;
  const int kExpectedNumBoxes = words.size() + kNumSpaces;
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kExpectedNumBoxes, boxchars.size());
  // Verify content of words and spaces
  for (int i = 0; i < boxchars.size(); i += 2) {
    EXPECT_EQ(words[i / 2], boxchars[i]->ch());
    if (i < boxchars.size() - 1) {
      EXPECT_EQ(" ", boxchars[i + 1]->ch());
      EXPECT_TRUE(boxchars[i + 1]->box() == nullptr);
    }
  }
}

TEST_F(StringRendererTest, DoesRenderWordBoxesFromMultiLineText) {
  renderer_.reset(new StringRenderer("Verdana 10", 600, 600));
  renderer_->set_output_word_boxes(true);
  Pix* pix = nullptr;
  const char kMultlineText[] = "the quick brown fox\njumps over the lazy dog";
  EXPECT_EQ(strlen(kMultlineText),
            renderer_->RenderToImage(kMultlineText, strlen(kEngText), &pix));
  pixDestroy(&pix);
  // Verify #boxchars = #words + #spaces + #newlines
  std::vector<string> words =
      absl::StrSplit(kMultlineText, absl::ByAnyChar(" \n"), absl::SkipEmpty());
  const int kNumSeparators = words.size() - 1;
  const int kExpectedNumBoxes = words.size() + kNumSeparators;
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kExpectedNumBoxes, boxchars.size());
  // Verify content of words and spaces
  for (int i = 0; i < boxchars.size(); i += 2) {
    EXPECT_EQ(words[i / 2], boxchars[i]->ch());
    if (i < boxchars.size() - 1) {
      EXPECT_EQ(" ", boxchars[i + 1]->ch());
      EXPECT_TRUE(boxchars[i + 1]->box() == nullptr);
    }
  }
}

TEST_F(StringRendererTest, DoesRenderAllFontsToImage) {
  renderer_.reset(new StringRenderer("Verdana 10", 1200, 1200));
  int offset = 0;
  string font_used;
  do {
    Pix* pix = nullptr;
    font_used.clear();
    offset += renderer_->RenderAllFontsToImage(
        1.0, kEngText + offset, strlen(kEngText + offset), &font_used, &pix);
    if (offset < strlen(kEngText)) {
      EXPECT_TRUE(pix != nullptr);
      EXPECT_STRNE("", font_used.c_str());
    }
    if (FLAGS_display) pixDisplay(pix, 0, 0);
    pixDestroy(&pix);
  } while (offset < strlen(kEngText));
}

TEST_F(StringRendererTest, DoesNotRenderWordJoiner) {
  renderer_.reset(new StringRenderer("Verdana 10", 500, 200));
  const string word = "A- -B C-D A BC";
  const string joined_word = StringRenderer::InsertWordJoiners(word);
  Pix* pix = nullptr;
  renderer_->RenderToImage(joined_word.c_str(), joined_word.length(), &pix);
  pixDestroy(&pix);
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  const string kWordJoinerUTF8 = "\u2060";
  ASSERT_EQ(word.length(), boxchars.size());
  for (int i = 0; i < boxchars.size(); ++i) {
    EXPECT_NE(kWordJoinerUTF8, boxchars[i]->ch());
    EXPECT_EQ(word.substr(i, 1), boxchars[i]->ch());
  }
}

TEST_F(StringRendererTest, DoesDropUncoveredChars) {
  renderer_.reset(new StringRenderer("Verdana 10", 500, 200));
  renderer_->set_drop_uncovered_chars(true);
  const string kWord = "oﬀice";
  const string kCleanWord = "oice";
  Pix* pix = nullptr;
  EXPECT_FALSE(
      renderer_->font().CanRenderString(kWord.c_str(), kWord.length()));
  EXPECT_FALSE(renderer_->font().CoversUTF8Text(kWord.c_str(), kWord.length()));
  int offset = renderer_->RenderToImage(kWord.c_str(), kWord.length(), &pix);
  pixDestroy(&pix);
  const std::vector<BoxChar*>& boxchars = renderer_->GetBoxes();
  EXPECT_EQ(kWord.length(), offset);
  ASSERT_EQ(kCleanWord.length(), boxchars.size());
  for (int i = 0; i < boxchars.size(); ++i) {
    EXPECT_EQ(kCleanWord.substr(i, 1), boxchars[i]->ch());
  }
}

// ------------ StringRenderer::ConvertBasicLatinToFullwidthLatin() ------------

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesConvertBasicLatin) {
  const string kHalfAlpha = "ABCD";
  const string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kFullAlpha,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfAlpha));

  const string kHalfDigit = "0123";
  const string kFullDigit = "０１２３";
  EXPECT_EQ(kFullDigit,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfDigit));

  const string kHalfSym = "()[]:;!?";
  const string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kFullSym,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfSym));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertFullwidthLatin) {
  const string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kFullAlpha,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullAlpha));

  const string kFullDigit = "０１２３";
  EXPECT_EQ(kFullDigit,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullDigit));

  const string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kFullSym,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullSym));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertNonLatin) {
  const string kHalfKana = "ｱｲｳｴｵ";
  const string kFullKana = "アイウエオ";
  EXPECT_EQ(kHalfKana,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfKana));
  EXPECT_EQ(kFullKana,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullKana));
}

TEST(ConvertBasicLatinToFullwidthLatinTest, DoesNotConvertSpace) {
  const string kHalfSpace = " ";
  const string kFullSpace = "　";
  EXPECT_EQ(kHalfSpace,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kHalfSpace));
  EXPECT_EQ(kFullSpace,
            StringRenderer::ConvertBasicLatinToFullwidthLatin(kFullSpace));
}

// ------------ StringRenderer::ConvertFullwidthLatinToBasicLatin() ------------

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesConvertFullwidthLatin) {
  const string kHalfAlpha = "ABCD";
  const string kFullAlpha = "ＡＢＣＤ";
  EXPECT_EQ(kHalfAlpha,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullAlpha));

  const string kHalfDigit = "0123";
  const string kFullDigit = "０１２３";
  EXPECT_EQ(kHalfDigit,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullDigit));

  const string kHalfSym = "()[]:;!?";
  const string kFullSym = "（）［］：；！？";
  EXPECT_EQ(kHalfSym,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullSym));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertBasicLatin) {
  const string kHalfAlpha = "ABCD";
  EXPECT_EQ(kHalfAlpha,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfAlpha));

  const string kHalfDigit = "0123";
  EXPECT_EQ(kHalfDigit,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfDigit));

  const string kHalfSym = "()[]:;!?";
  EXPECT_EQ(kHalfSym,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfSym));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertNonLatin) {
  const string kHalfKana = "ｱｲｳｴｵ";
  const string kFullKana = "アイウエオ";
  EXPECT_EQ(kHalfKana,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfKana));
  EXPECT_EQ(kFullKana,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullKana));
}

TEST(ConvertFullwidthLatinToBasicLatinTest, DoesNotConvertSpace) {
  const string kHalfSpace = " ";
  const string kFullSpace = "　";
  EXPECT_EQ(kHalfSpace,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kHalfSpace));
  EXPECT_EQ(kFullSpace,
            StringRenderer::ConvertFullwidthLatinToBasicLatin(kFullSpace));
}
}  // namespace
