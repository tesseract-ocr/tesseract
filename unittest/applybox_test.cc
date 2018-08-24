
#include <string>
#include "leptonica/include/allheaders.h"
#include "tesseract/api/baseapi.h"
#include "tesseract/ccmain/resultiterator.h"
#include "tesseract/ccstruct/boxread.h"
#include "tesseract/ccstruct/rect.h"

namespace {

using tesseract::ResultIterator;

const char* kTruthTextWords = "To simple burn running of goods lately.\n\n";
const char* kTruthTextLine = "Tosimpleburnrunningofgoodslately.\n\n";

// The fixture for testing Tesseract.
class ApplyBoxTest : public testing::Test {
 protected:
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir,
                          "testdata/" + name);
  }
  string TessdataPath() {
    return file::JoinPath(FLAGS_test_srcdir,
                          "tessdata");
  }
  string OutputNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }

  ApplyBoxTest() {
    src_pix_ = NULL;
  }
  ~ApplyBoxTest() {
    pixDestroy(&src_pix_);
  }

  void SetImage(const char* filename) {
    pixDestroy(&src_pix_);
    src_pix_ = pixRead(TestDataNameToPath(filename).c_str());
    api_.Init(TessdataPath().c_str(), "eng", tesseract::OEM_TESSERACT_ONLY);
    api_.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
    api_.SetImage(src_pix_);
    api_.SetVariable("tessedit_make_boxes_from_boxes", "1");
    api_.SetInputName(TestDataNameToPath(filename).c_str());
  }

  // Runs ApplyBoxes (via setting the appropriate variables and Recognize)
  // and checks that the output ocr text matches the truth_str, and that
  // the boxes match the given box file well enough.
  // If line_mode is true, ApplyBoxes is run in line segmentation mode,
  // otherwise the input box file is assumed to have character-level boxes.
  void VerifyBoxesAndText(const char* imagefile, const char* truth_str,
                          const char* target_box_file, bool line_mode) {
    SetImage(imagefile);
    if (line_mode)
      api_.SetVariable("tessedit_resegment_from_line_boxes", "1");
    else
      api_.SetVariable("tessedit_resegment_from_boxes", "1");
    api_.Recognize(NULL);
    char* ocr_text = api_.GetUTF8Text();
    EXPECT_STREQ(truth_str, ocr_text);
    delete [] ocr_text;
    // Test the boxes by reading the target box file in parallel with the
    // bounding boxes in the ocr output.
    string box_filename = TestDataNameToPath(target_box_file);
    FILE* box_file = OpenBoxFile(STRING(box_filename.c_str()));
    ASSERT_TRUE(box_file != NULL);
    int height = pixGetHeight(src_pix_);
    ResultIterator* it = api_.GetIterator();
    do {
      int left, top, right, bottom;
      EXPECT_TRUE(it->BoundingBox(tesseract::RIL_SYMBOL,
                                  &left, &top, &right, &bottom));
      TBOX ocr_box(ICOORD(left, height - bottom),
                   ICOORD(right, height - top));
      int line_number;
      TBOX truth_box;
      STRING box_text;
      EXPECT_TRUE(ReadNextBox(0, &line_number, box_file, &box_text,
                                &truth_box));
      // Testing for major overlap is a bit weak, but if they all
      // major overlap successfully, then it has to be fairly close.
      EXPECT_TRUE(ocr_box.major_overlap(truth_box));
      // Also check that the symbol text matches the box text.
      char* symbol_text = it->GetUTF8Text(tesseract::RIL_SYMBOL);
      EXPECT_STREQ(box_text.string(), symbol_text);
      delete [] symbol_text;
    } while (it->Next(tesseract::RIL_SYMBOL));
    delete it;
  }

  Pix* src_pix_;
  string ocr_text_;
  tesseract::TessBaseAPI api_;
};

// Tests character-level applyboxes on normal Times New Roman.
TEST_F(ApplyBoxTest, TimesCharLevel) {
  VerifyBoxesAndText("trainingtimes.tif", kTruthTextWords,
                     "trainingtimes.box", false);
}

// Tests character-level applyboxes on italic Times New Roman.
TEST_F(ApplyBoxTest, ItalicCharLevel) {
  VerifyBoxesAndText("trainingital.tif", kTruthTextWords,
                     "trainingital.box", false);
}

// Tests line-level applyboxes on normal Times New Roman.
TEST_F(ApplyBoxTest, TimesLineLevel) {
  VerifyBoxesAndText("trainingtimesline.tif", kTruthTextLine,
                     "trainingtimes.box", true);
}

// Tests line-level applyboxes on italic Times New Roman.
TEST_F(ApplyBoxTest, ItalLineLevel) {
  VerifyBoxesAndText("trainingitalline.tif", kTruthTextLine,
                     "trainingital.box", true);
}

}  // namespace
