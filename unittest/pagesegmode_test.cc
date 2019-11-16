
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
  std::string filename = file::JoinPath(TESTING_DIR, "segmodeimg.tif");
  if (!file_exists(filename.c_str())) {
    LOG(INFO) << "Skip test because of missing " << filename << '\n';
    GTEST_SKIP();
  } else {
    SetImage(filename.c_str());
    // Test various rectangles around the inverse page number.
    VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1419, 264, 69, 34);
    VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1411, 252, 78, 62);
    VerifyRectText(tesseract::PSM_SINGLE_WORD, "183", 1396, 218, 114, 102);
    // Test a random pair of words as a line
    VerifyRectText(tesseract::PSM_SINGLE_LINE,
                   "What should", 237, 393, 256, 36);
    // Test a random pair of words as a word
    VerifyRectText(tesseract::PSM_SINGLE_WORD,
                   "Whatshould", 237, 393, 256, 36);
    // Test single block mode.
    VerifyRectText(tesseract::PSM_SINGLE_BLOCK,
                   "both the\nfrom the", 237, 450, 172, 94);
    // But doesn't work in line or word mode.
    NotRectText(tesseract::PSM_SINGLE_LINE,
                "both the\nfrom the", 237, 450, 172, 94);
    NotRectText(tesseract::PSM_SINGLE_WORD,
                "both the\nfrom the", 237, 450, 172, 94);
  }
}

}  // namespace
