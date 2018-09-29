
#include "tesseract/ccmain/resultiterator.h"
#include <string>
#include "leptonica/include/allheaders.h"
#include "tesseract/api/baseapi.h"
#include "tesseract/ccutil/genericvector.h"
#include "tesseract/viewer/scrollview.h"

namespace {

DEFINE_string(tess_config, "", "config file for tesseract");
DEFINE_bool(visual_test, false, "Runs a visual test using scrollview");

using tesseract::PageIterator;
using tesseract::PageIteratorLevel;
using tesseract::ResultIterator;

// Helper functions for converting to STL vectors
template <typename T>
void ToVector(const GenericVector<T>& from, std::vector<T>* to) {
  to->clear();
  for (int i = 0; i < from.size(); i++) to->push_back(from[i]);
}

template <typename T>
void ToVector(const GenericVectorEqEq<T>& from, std::vector<T>* to) {
  to->clear();
  for (int i = 0; i < from.size(); i++) to->push_back(from[i]);
}

// The fixture for testing Tesseract.
class ResultIteratorTest : public testing::Test {
 protected:
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
  string TessdataPath() {
    return file::JoinPath(FLAGS_test_srcdir, "tessdata");
  }
  string OutputNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }

  ResultIteratorTest() { src_pix_ = nullptr; }
  ~ResultIteratorTest() {}

  void SetImage(const char* filename) {
    src_pix_ = pixRead(TestDataNameToPath(filename).c_str());
    api_.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
    if (!FLAGS_tess_config.empty())
      api_.ReadConfigFile(FLAGS_tess_config.c_str());
    api_.SetPageSegMode(tesseract::PSM_AUTO);
    api_.SetImage(src_pix_);
    pixDestroy(&src_pix_);
    src_pix_ = api_.GetInputImage();
  }

  // Rebuilds the image using the binary images at the given level, and
  // EXPECTs that the number of pixels in the xor of the rebuilt image with
  // the original is at most max_diff.
  void VerifyRebuild(int max_diff, PageIteratorLevel level, PageIterator* it) {
    it->Begin();
    int width = pixGetWidth(src_pix_);
    int height = pixGetHeight(src_pix_);
    int depth = pixGetDepth(src_pix_);
    Pix* pix = pixCreate(width, height, depth);
    EXPECT_TRUE(depth == 1 || depth == 8);
    if (depth == 8) pixSetAll(pix);
    do {
      int left, top, right, bottom;
      PageIteratorLevel im_level = level;
      // If the return is false, it is a non-text block so get the block image.
      if (!it->BoundingBox(level, &left, &top, &right, &bottom)) {
        im_level = tesseract::RIL_BLOCK;
        EXPECT_TRUE(it->BoundingBox(im_level, &left, &top, &right, &bottom));
      }
      VLOG(1) << "BBox: [L:" << left << ", T:" << top << ", R:" << right
              << ", B:" << bottom << "]";
      Pix* block_pix;
      if (depth == 1) {
        block_pix = it->GetBinaryImage(im_level);
        pixRasterop(pix, left, top, right - left, bottom - top,
                    PIX_SRC ^ PIX_DST, block_pix, 0, 0);
      } else {
        block_pix = it->GetImage(im_level, 2, src_pix_, &left, &top);
        pixRasterop(pix, left, top, pixGetWidth(block_pix),
                    pixGetHeight(block_pix), PIX_SRC & PIX_DST, block_pix, 0,
                    0);
      }
      CHECK(block_pix != nullptr);
      pixDestroy(&block_pix);
    } while (it->Next(level));
    if (base::GetFlag(FLAGS_v) >= 1)
      pixWrite(OutputNameToPath("rebuilt.png").c_str(), pix, IFF_PNG);
    pixRasterop(pix, 0, 0, width, height, PIX_SRC ^ PIX_DST, src_pix_, 0, 0);
    if (depth == 8) {
      Pix* binary_pix = pixThresholdToBinary(pix, 128);
      pixDestroy(&pix);
      pixInvert(binary_pix, binary_pix);
      pix = binary_pix;
    }
    if (base::GetFlag(FLAGS_v) >= 1)
      pixWrite(OutputNameToPath("rebuiltxor.png").c_str(), pix, IFF_PNG);
    l_int32 pixcount;
    pixCountPixels(pix, &pixcount, nullptr);
    if (pixcount > max_diff) {
      string outfile = OutputNameToPath("failedxor.png");
      VLOG(1) << "outfile = " << outfile;
      pixWrite(outfile.c_str(), pix, IFF_PNG);
    }
    pixDestroy(&pix);
    VLOG(1) << StringPrintf("At level %d: pix diff = %d\n", level, pixcount);
    EXPECT_LE(pixcount, max_diff);
    if (base::GetFlag(FLAGS_v) > 1) CHECK_LE(pixcount, max_diff);
  }

  // Rebuilds the text from the iterator strings at the given level, and
  // EXPECTs that the rebuild string exactly matches the truth string.
  void VerifyIteratorText(const string& truth, PageIteratorLevel level,
                          ResultIterator* it) {
    VLOG(1) << "Text Test Level " << level;
    it->Begin();
    string result;
    do {
      char* text = it->GetUTF8Text(level);
      result += text;
      delete[] text;
      if ((level == tesseract::RIL_WORD || level == tesseract::RIL_SYMBOL) &&
          it->IsAtFinalElement(tesseract::RIL_WORD, level)) {
        if (it->IsAtFinalElement(tesseract::RIL_TEXTLINE, level)) {
          result += '\n';
        } else {
          result += ' ';
        }
        if (it->IsAtFinalElement(tesseract::RIL_PARA, level)) result += '\n';
      }
    } while (it->Next(level));
    EXPECT_STREQ(truth.c_str(), result.c_str())
        << "Rebuild failed at Text Level " << level;
  }

  void VerifyRebuilds(int block_limit, int para_limit, int line_limit,
                      int word_limit, int symbol_limit, PageIterator* it) {
    VerifyRebuild(block_limit, tesseract::RIL_BLOCK, it);
    VerifyRebuild(para_limit, tesseract::RIL_PARA, it);
    VerifyRebuild(line_limit, tesseract::RIL_TEXTLINE, it);
    VerifyRebuild(word_limit, tesseract::RIL_WORD, it);
    VerifyRebuild(symbol_limit, tesseract::RIL_SYMBOL, it);
  }

  void VerifyAllText(const string& truth, ResultIterator* it) {
    VerifyIteratorText(truth, tesseract::RIL_BLOCK, it);
    VerifyIteratorText(truth, tesseract::RIL_PARA, it);
    VerifyIteratorText(truth, tesseract::RIL_TEXTLINE, it);
    VerifyIteratorText(truth, tesseract::RIL_WORD, it);
    VerifyIteratorText(truth, tesseract::RIL_SYMBOL, it);
  }

  // Verifies that ResultIterator::CalculateTextlineOrder() produces the right
  // results given an array of word directions (word_dirs[num_words]), an
  // expected output reading order
  // (expected_reading_order[num_reading_order_entries]) and a given reading
  // context (ltr or rtl).
  void ExpectTextlineReadingOrder(bool in_ltr_context,
                                  StrongScriptDirection* word_dirs,
                                  int num_words, int* expected_reading_order,
                                  int num_reading_order_entries) const {
    GenericVector<StrongScriptDirection> gv_word_dirs;
    for (int i = 0; i < num_words; i++) {
      gv_word_dirs.push_back(word_dirs[i]);
    }

    GenericVectorEqEq<int> output;
    ResultIterator::CalculateTextlineOrder(in_ltr_context, gv_word_dirs,
                                           &output);
    // STL vector can be used with EXPECT_EQ, so convert...
    std::vector<int> correct_order(
        expected_reading_order,
        expected_reading_order + num_reading_order_entries);
    std::vector<int> calculated_order;
    ToVector(output, &calculated_order);
    EXPECT_EQ(correct_order, calculated_order);
  }

  // Verify that ResultIterator::CalculateTextlineOrder() produces sane output
  // for a given array of word_dirs[num_words] in ltr or rtl context.
  // Sane means that the output contains some permutation of the indices
  // 0..[num_words - 1] interspersed optionally with negative (marker) values.
  void VerifySaneTextlineOrder(bool in_ltr_context,
                               StrongScriptDirection* word_dirs,
                               int num_words) const {
    GenericVector<StrongScriptDirection> gv_word_dirs;
    for (int i = 0; i < num_words; i++) {
      gv_word_dirs.push_back(word_dirs[i]);
    }

    GenericVectorEqEq<int> output;
    ResultIterator::CalculateTextlineOrder(in_ltr_context, gv_word_dirs,
                                           &output);
    ASSERT_GE(output.size(), num_words);
    GenericVector<int> output_copy(output);
    output_copy.sort();
    bool sane = true;
    int j = 0;
    while (j < output_copy.size() && output_copy[j] < 0) j++;
    for (int i = 0; i < num_words; i++, j++) {
      if (output_copy[j] != i) {
        sane = false;
        break;
      }
    }
    if (j != output_copy.size()) {
      sane = false;
    }
    if (!sane) {
      std::vector<int> output_copy2, empty;
      ToVector(output, &output_copy2);
      EXPECT_EQ(output_copy2, empty)
          << " permutation of 0.." << num_words - 1 << " not found in "
          << (in_ltr_context ? "ltr" : "rtl") << " context.";
    }
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  Pix* src_pix_;  // Borrowed from api_. Do not destroy.
  string ocr_text_;
  tesseract::TessBaseAPI api_;
};

// Tests layout analysis output (and scrollview) on the UNLV page numbered
// 8087_054.3G.tif. (Dubrovnik), but only if --visual_test is true.
TEST_F(ResultIteratorTest, VisualTest) {
  if (!FLAGS_visual_test) return;
  const char* kIms[] = {"8087_054.3G.tif", "8071_093.3B.tif", nullptr};
  for (int i = 0; kIms[i] != nullptr; ++i) {
    SetImage(kIms[i]);
    // Just run layout analysis.
    PageIterator* it = api_.AnalyseLayout();
    EXPECT_FALSE(it == nullptr);
    // Make a scrollview window for the display.
    int width = pixGetWidth(src_pix_);
    int height = pixGetHeight(src_pix_);
    ScrollView* win =
        new ScrollView(kIms[i], 100, 100, width / 2, height / 2, width, height);
    win->Image(src_pix_, 0, 0);
    it->Begin();
    ScrollView::Color color = ScrollView::RED;
    win->Brush(ScrollView::NONE);
    do {
      Pta* pts = it->BlockPolygon();
      if (pts != nullptr) {
        win->Pen(color);
        int num_pts = ptaGetCount(pts);
        l_float32 x, y;
        ptaGetPt(pts, num_pts - 1, &x, &y);
        win->SetCursor(static_cast<int>(x), static_cast<int>(y));
        for (int p = 0; p < num_pts; ++p) {
          ptaGetPt(pts, p, &x, &y);
          win->DrawTo(static_cast<int>(x), static_cast<int>(y));
        }
      }
      ptaDestroy(&pts);
    } while (it->Next(tesseract::RIL_BLOCK));
    win->Update();
    delete win->AwaitEvent(SVET_DESTROY);
    delete win;
    delete it;
  }
}

// Tests that Tesseract gets exactly the right answer on phototest.
TEST_F(ResultIteratorTest, EasyTest) {
  SetImage("phototest.tif");
  // Just run layout analysis.
  PageIterator* p_it = api_.AnalyseLayout();
  EXPECT_FALSE(p_it == nullptr);
  // Check iterator position.
  EXPECT_TRUE(p_it->IsAtBeginningOf(tesseract::RIL_BLOCK));
  // This should be a single block.
  EXPECT_FALSE(p_it->Next(tesseract::RIL_BLOCK));
  EXPECT_FALSE(p_it->IsAtBeginningOf(tesseract::RIL_BLOCK));

  // The images should rebuild almost perfectly.
  LOG(INFO) << "Verifying image rebuilds 1 (pageiterator)";
  VerifyRebuilds(10, 10, 0, 0, 0, p_it);
  delete p_it;

  char* result = api_.GetUTF8Text();
  ocr_text_ = result;
  delete[] result;
  ResultIterator* r_it = api_.GetIterator();
  // The images should rebuild almost perfectly.
  LOG(INFO) << "Verifying image rebuilds 2a (resultiterator)";
  VerifyRebuilds(8, 8, 0, 0, 40, r_it);
  // Test the text.
  LOG(INFO) << "Verifying text rebuilds 1 (resultiterator)";
  VerifyAllText(ocr_text_, r_it);

  // The images should rebuild almost perfectly.
  LOG(INFO) << "Verifying image rebuilds 2b (resultiterator)";
  VerifyRebuilds(8, 8, 0, 0, 40, r_it);

  r_it->Begin();
  // Test baseline of the first line.
  int x1, y1, x2, y2;
  r_it->Baseline(tesseract::RIL_TEXTLINE, &x1, &y1, &x2, &y2);
  VLOG(1) << StringPrintf("Baseline (%d,%d)->(%d,%d)", x1, y1, x2, y2);
  // Make sure we have a decent vector.
  EXPECT_GE(x2, x1 + 400);
  // The point 200,116 should be very close to the baseline.
  // (x3,y3) is the vector from (x1,y1) to (200,116)
  int x3 = 200 - x1;
  int y3 = 116 - y1;
  x2 -= x1;
  y2 -= y1;
  // The cross product (x2,y1)x(x3,y3) should be small.
  int product = x2 * y3 - x3 * y2;
  EXPECT_LE(abs(product), x2);

  // Test font attributes for each word.
  do {
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    const char* font =
        r_it->WordFontAttributes(&bold, &italic, &underlined, &monospace,
                                 &serif, &smallcaps, &pointsize, &font_id);
    float confidence = r_it->Confidence(tesseract::RIL_WORD);
    EXPECT_GE(confidence, 80.0f);
    char* word_str = r_it->GetUTF8Text(tesseract::RIL_WORD);
    VLOG(1) << StringPrintf("Word %s in font %s, id %d, size %d, conf %g",
                            word_str, font, font_id, pointsize, confidence);
    delete[] word_str;
    EXPECT_FALSE(bold);
    EXPECT_FALSE(italic);
    EXPECT_FALSE(underlined);
    EXPECT_FALSE(monospace);
    EXPECT_FALSE(serif);
    // The text is about 31 pixels high.  Above we say the source is 200 ppi,
    // which translates to:
    // 31 pixels / textline * (72 pts / inch) / (200 pixels / inch) = 11.16 pts
    EXPECT_GE(pointsize, 11.16 - 1.50);
    EXPECT_LE(pointsize, 11.16 + 1.50);
  } while (r_it->Next(tesseract::RIL_WORD));
  delete r_it;
}

// Tests image rebuild on the UNLV page numbered 8087_054.3B.tif. (Dubrovnik)
TEST_F(ResultIteratorTest, ComplexTest) {
  SetImage("8087_054.3B.tif");
  // Just run layout analysis.
  PageIterator* it = api_.AnalyseLayout();
  EXPECT_FALSE(it == nullptr);
  // The images should rebuild almost perfectly.
  VerifyRebuilds(400, 400, 400, 400, 650, it);
  delete it;
}

// Tests image rebuild on the UNLV page numbered 8087_054.3G.tif. (Dubrovnik)
TEST_F(ResultIteratorTest, GreyTest) {
  SetImage("8087_054.3G.tif");
  // Just run layout analysis.
  PageIterator* it = api_.AnalyseLayout();
  EXPECT_FALSE(it == nullptr);
  // The images should rebuild almost perfectly.
  VerifyRebuilds(600, 600, 600, 600, 600, it);
  delete it;
}

// Tests that Tesseract gets smallcaps and dropcaps.
TEST_F(ResultIteratorTest, SmallCapDropCapTest) {
  SetImage("8071_093.3B.tif");
  char* result = api_.GetUTF8Text();
  delete[] result;
  ResultIterator* r_it = api_.GetIterator();
  // Iterate over the words.
  int found_dropcaps = 0;
  int found_smallcaps = 0;
  int false_positives = 0;
  do {
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    r_it->WordFontAttributes(&bold, &italic, &underlined, &monospace, &serif,
                             &smallcaps, &pointsize, &font_id);
    char* word_str = r_it->GetUTF8Text(tesseract::RIL_WORD);
    if (word_str != nullptr) {
      VLOG(1) << StringPrintf("Word %s is %s", word_str,
                              smallcaps ? "Smallcaps" : "Normal");
      if (r_it->SymbolIsDropcap()) {
        ++found_dropcaps;
      }
      if (strcmp(word_str, "SHE") == 0 || strcmp(word_str, "MOPED") == 0 ||
          strcmp(word_str, "RALPH") == 0 ||
          strcmp(word_str, "KINNEY") == 0 ||  // Not working yet.
          strcmp(word_str, "BENNETT") == 0) {
        EXPECT_TRUE(smallcaps) << word_str;
        ++found_smallcaps;
      } else {
        if (smallcaps) ++false_positives;
      }
      // No symbol other than the first of any word should be dropcap.
      ResultIterator s_it(*r_it);
      while (s_it.Next(tesseract::RIL_SYMBOL) &&
             !s_it.IsAtBeginningOf(tesseract::RIL_WORD)) {
        if (s_it.SymbolIsDropcap()) {
          char* sym_str = s_it.GetUTF8Text(tesseract::RIL_SYMBOL);
          LOG(ERROR) << StringPrintf("Symbol %s of word %s is dropcap", sym_str,
                                     word_str);
          delete[] sym_str;
        }
        EXPECT_FALSE(s_it.SymbolIsDropcap());
      }
      delete[] word_str;
    }
  } while (r_it->Next(tesseract::RIL_WORD));
  delete r_it;
  EXPECT_EQ(1, found_dropcaps);
  EXPECT_GE(4, found_smallcaps);
  EXPECT_LE(false_positives, 3);
}
#if 0
// TODO(rays) uncomment on the next change to layout analysis.
// CL 22736106 breaks it, but it is fixed in the change when
// the textline finders start to collapse.

// Tests that Tesseract gets subscript and superscript.
// TODO(rays) This test is a bit feeble, due to bad textline finding on this
// image, so beef up the test a bit when we get less false positive subs.
TEST_F(ResultIteratorTest, SubSuperTest) {
  SetImage("0146_281.3B.tif");
  char* result = api_.GetUTF8Text();
  delete [] result;
  ResultIterator* r_it = api_.GetIterator();
  // Iterate over the symbols.
  // Accuracy isn't great, so just count up and expect a decent count of
  // positives and negatives.
  const char kAllowedSupers[] = "O0123456789-";
  int found_subs = 0;
  int found_supers = 0;
  int found_normal = 0;
  do {
    if (r_it->SymbolIsSubscript()) {
      ++found_subs;
    } else if (r_it->SymbolIsSuperscript()) {
      result = r_it->GetUTF8Text(tesseract::RIL_SYMBOL);
      if (strchr(kAllowedSupers, result[0]) == nullptr) {
        char* word = r_it->GetUTF8Text(tesseract::RIL_WORD);
        LOG(ERROR) << StringPrintf("Char %s in word %s is unexpected super!",
                                    result, word);
        delete [] word;
        EXPECT_TRUE(strchr(kAllowedSupers, result[0]) != nullptr);
      }
      delete [] result;
      ++found_supers;
    } else {
      ++found_normal;
    }
  } while (r_it->Next(tesseract::RIL_SYMBOL));
  delete r_it;
  VLOG(1) << StringPrintf("Subs = %d, supers= %d, normal = %d",
                          found_subs, found_supers, found_normal);
  EXPECT_GE(found_subs, 25);
  EXPECT_GE(found_supers, 25);
  EXPECT_GE(found_normal, 1350);
}
#endif

static const StrongScriptDirection dL = DIR_LEFT_TO_RIGHT;
static const StrongScriptDirection dR = DIR_RIGHT_TO_LEFT;
static const StrongScriptDirection dN = DIR_NEUTRAL;
static const StrongScriptDirection dZ = DIR_MIX;

// Test that a sequence of words that could be interpreted to start from
// the left side left-to-right or from the right side right-to-left is
// interpreted appropriately in different contexts.
TEST_F(ResultIteratorTest, DualStartTextlineOrderTest) {
  StrongScriptDirection word_dirs[] = {dL, dL, dN, dL, dN, dR, dR, dR};
  int reading_order_rtl_context[] = {7, 6, 5, 4, ResultIterator::kMinorRunStart,
                                     0, 1, 2, 3, ResultIterator::kMinorRunEnd};
  int reading_order_ltr_context[] = {0, 1,
                                     2, 3,
                                     4, ResultIterator::kMinorRunStart,
                                     7, 6,
                                     5, ResultIterator::kMinorRunEnd};

  ExpectTextlineReadingOrder(true, word_dirs, ABSL_ARRAYSIZE(word_dirs),
                             reading_order_ltr_context,
                             ABSL_ARRAYSIZE(reading_order_ltr_context));
  ExpectTextlineReadingOrder(false, word_dirs, ABSL_ARRAYSIZE(word_dirs),
                             reading_order_rtl_context,
                             ABSL_ARRAYSIZE(reading_order_rtl_context));
}

// Tests that clearly left-direction text (with no right-to-left indications)
// comes out strictly left to right no matter the context.
TEST_F(ResultIteratorTest, LeftwardTextlineOrderTest) {
  StrongScriptDirection word_dirs[] = {dL, dL, dN, dL, dN, dN, dL, dL};
  // The order here is just left to right, nothing fancy.
  int reading_order_ltr_context[] = {0, 1, 2, 3, 4, 5, 6, 7};
  // In the strange event that this shows up in an RTL paragraph, nonetheless
  // just presume the whole thing is an LTR line.
  int reading_order_rtl_context[] = {
      ResultIterator::kMinorRunStart, 0, 1, 2, 3, 4, 5, 6, 7,
      ResultIterator::kMinorRunEnd};

  ExpectTextlineReadingOrder(true, word_dirs, ABSL_ARRAYSIZE(word_dirs),
                             reading_order_ltr_context,
                             ABSL_ARRAYSIZE(reading_order_ltr_context));
  ExpectTextlineReadingOrder(false, word_dirs, ABSL_ARRAYSIZE(word_dirs),
                             reading_order_rtl_context,
                             ABSL_ARRAYSIZE(reading_order_rtl_context));
}

// Test that right-direction text comes out strictly right-to-left in
// a right-to-left context.
TEST_F(ResultIteratorTest, RightwardTextlineOrderTest) {
  StrongScriptDirection word_dirs[] = {dR, dR, dN, dR, dN, dN, dR, dR};
  // The order here is just right-to-left, nothing fancy.
  int reading_order_rtl_context[] = {7, 6, 5, 4, 3, 2, 1, 0};
  ExpectTextlineReadingOrder(false, word_dirs, ABSL_ARRAYSIZE(word_dirs),
                             reading_order_rtl_context,
                             ABSL_ARRAYSIZE(reading_order_rtl_context));
}

TEST_F(ResultIteratorTest, TextlineOrderSanityCheck) {
  // Iterate through all 7-word sequences and make sure that the output
  // contains each of the indices 0..6 exactly once.
  const int kNumWords(7);
  const int kNumCombos = 1 << (2 * kNumWords);  // 4 ^ 7 combinations
  StrongScriptDirection word_dirs[kNumWords];
  for (int i = 0; i < kNumCombos; i++) {
    // generate the next combination.
    int tmp = i;
    for (int j = 0; j < kNumWords; j++) {
      word_dirs[j] = static_cast<StrongScriptDirection>(tmp % 4);
      tmp = tmp / 4;
    }
    VerifySaneTextlineOrder(true, word_dirs, kNumWords);
    VerifySaneTextlineOrder(false, word_dirs, kNumWords);
  }
}

TEST_F(ResultIteratorTest, NonNullChoicesTest) {
  SetImage("5318c4b679264.jpg");
  char* result = api_.GetUTF8Text();
  delete[] result;
  ResultIterator* r_it = api_.GetIterator();
  // Iterate over the words.
  do {
    char* word_str = r_it->GetUTF8Text(tesseract::RIL_WORD);
    if (word_str != nullptr) {
      VLOG(1) << StringPrintf("Word %s:", word_str);
      ResultIterator s_it = *r_it;
      do {
        tesseract::ChoiceIterator c_it(s_it);
        do {
          const char* char_str = c_it.GetUTF8Text();
          if (char_str == nullptr)
            VLOG(1) << "Null char choice";
          else
            VLOG(1) << "Char choice " << char_str;
          CHECK(char_str != nullptr);
        } while (c_it.Next());
      } while (
          !s_it.IsAtFinalElement(tesseract::RIL_WORD, tesseract::RIL_SYMBOL) &&
          s_it.Next(tesseract::RIL_SYMBOL));
      delete[] word_str;
    }
  } while (r_it->Next(tesseract::RIL_WORD));
  delete r_it;
}

TEST_F(ResultIteratorTest, NonNullConfidencesTest) {
  SetImage("line6.tiff");
  api_.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
  // Force recognition so we can used the result iterator.
  // We don't care about the return from GetUTF8Text.
  char* result = api_.GetUTF8Text();
  delete[] result;
  ResultIterator* r_it = api_.GetIterator();
  // Iterate over the words.
  do {
    char* word_str = r_it->GetUTF8Text(tesseract::RIL_WORD);
    if (word_str != nullptr) {
      EXPECT_FALSE(r_it->Empty(tesseract::RIL_WORD));
      EXPECT_FALSE(r_it->Empty(tesseract::RIL_SYMBOL));
      ResultIterator s_it = *r_it;
      do {
        const char* char_str = s_it.GetUTF8Text(tesseract::RIL_SYMBOL);
        CHECK(char_str != nullptr);
        float confidence = s_it.Confidence(tesseract::RIL_SYMBOL);
        VLOG(1) << StringPrintf("Char %s has confidence %g\n", char_str,
                                confidence);
        delete[] char_str;
      } while (
          !s_it.IsAtFinalElement(tesseract::RIL_WORD, tesseract::RIL_SYMBOL) &&
          s_it.Next(tesseract::RIL_SYMBOL));
      delete[] word_str;
    } else {
      VLOG(1) << "Empty word found";
    }
  } while (r_it->Next(tesseract::RIL_WORD));
  delete r_it;
}

}  // namespace
