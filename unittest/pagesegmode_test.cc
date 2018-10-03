
#include <string>
#include "leptonica/include/allheaders.h"
#include "tesseract/api/baseapi.h"
#include "tesseract/ccutil/helpers.h"

namespace {

// The fixture for testing Tesseract.
class PageSegModeTest : public testing::Test {
 protected:
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
  string TessdataPath() {
    return file::JoinPath(FLAGS_test_srcdir, "tessdata");
  }

  PageSegModeTest() { src_pix_ = nullptr; }
  ~PageSegModeTest() { pixDestroy(&src_pix_); }

  void SetImage(const char* filename) {
    pixDestroy(&src_pix_);
    src_pix_ = pixRead(TestDataNameToPath(filename).c_str());
    api_.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
    api_.SetImage(src_pix_);
  }

  // Tests that the given rectangle produces exactly the given text in the
  // given segmentation mode (after chopping off the last 2 newlines.)
  void VerifyRectText(tesseract::PageSegMode mode, const char* str, int left,
                      int top, int width, int height) {
    api_.SetPageSegMode(mode);
    api_.SetRectangle(left, top, width, height);
    char* result = api_.GetUTF8Text();
    chomp_string(result);
    chomp_string(result);
    EXPECT_STREQ(str, result);
    delete[] result;
  }

  // Tests that the given rectangle does NOT produce the given text in the
  // given segmentation mode.
  void NotRectText(tesseract::PageSegMode mode, const char* str, int left,
                   int top, int width, int height) {
    api_.SetPageSegMode(mode);
    api_.SetRectangle(left, top, width, height);
    char* result = api_.GetUTF8Text();
    EXPECT_STRNE(str, result);
    delete[] result;
  }

  Pix* src_pix_;
  string ocr_text_;
  tesseract::TessBaseAPI api_;
};

// Tests the single-word segmentation mode, and that it performs correctly
// and differently to line and block mode.
TEST_F(PageSegModeTest, WordTest) {
  SetImage("segmodeimg.tif");
  // Test various rectangles around the inverse page number.
  VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1482, 146, 72, 44);
  VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1474, 134, 82, 72);
  VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1459, 116, 118, 112);
  // Test a random pair of words as a line
  VerifyRectText(tesseract::PSM_SINGLE_LINE, "What should", 1119, 621, 245, 54);
  // Test a random pair of words as a word
  VerifyRectText(tesseract::PSM_SINGLE_WORD, "Whatshould", 1119, 621, 245, 54);
  // Test single block mode.
  VerifyRectText(tesseract::PSM_SINGLE_BLOCK, "both the\nfrom the", 181, 676,
                 179, 104);
  // But doesn't work in line or word mode.
  NotRectText(tesseract::PSM_SINGLE_LINE, "both the\nfrom the", 181, 676, 179,
              104);
  NotRectText(tesseract::PSM_SINGLE_WORD, "both the\nfrom the", 181, 676, 179,
              104);
}

}  // namespace
