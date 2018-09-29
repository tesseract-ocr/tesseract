
#include <string>
#include <utility>
#include "leptonica/include/allheaders.h"
#include "tesseract/api/baseapi.h"
#include "tesseract/ccmain/mutableiterator.h"
#include "tesseract/ccmain/resultiterator.h"
#include "tesseract/ccstruct/coutln.h"
#include "tesseract/ccstruct/pageres.h"
#include "tesseract/ccstruct/polyblk.h"
#include "tesseract/ccstruct/stepblob.h"

namespace {

using tesseract::MutableIterator;
using tesseract::PageIteratorLevel;
using tesseract::ResultIterator;

const char* kStrings8087_054[] = {
    "dat", "Dalmatian", "", "DAMAGED DURING", "margarine,", nullptr};
const PolyBlockType kBlocks8087_054[] = {PT_HEADING_TEXT, PT_FLOWING_TEXT,
                                         PT_PULLOUT_IMAGE, PT_CAPTION_TEXT,
                                         PT_FLOWING_TEXT};

// The fixture for testing Tesseract.
class LayoutTest : public testing::Test {
 protected:
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
  string TessdataPath() {
    return file::JoinPath(FLAGS_test_srcdir, "tessdata");
  }

  LayoutTest() { src_pix_ = nullptr; }
  ~LayoutTest() { pixDestroy(&src_pix_); }

  void SetImage(const char* filename, const char* lang) {
    pixDestroy(&src_pix_);
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
  void VerifyBlockTextOrder(const char* strings[], const PolyBlockType* blocks,
                            ResultIterator* it) {
    it->Begin();
    int string_index = 0;
    int block_index = 0;
    do {
      char* block_text = it->GetUTF8Text(tesseract::RIL_BLOCK);
      if (block_text != nullptr && it->BlockType() == blocks[string_index] &&
          strstr(block_text, strings[string_index]) != nullptr) {
        VLOG(1) << StringPrintf("Found string %s in block %d of type %s",
                                strings[string_index], block_index,
                                kPolyBlockNames[blocks[string_index]]);
        // Found this one.
        ++string_index;
      } else if (it->BlockType() == blocks[string_index] &&
                 block_text == nullptr && strings[string_index][0] == '\0') {
        VLOG(1) << StringPrintf("Found block of type %s at block %d",
                                kPolyBlockNames[blocks[string_index]],
                                block_index);
        // Found this one.
        ++string_index;
      } else {
        VLOG(1) << StringPrintf("No match found in block with text:\n%s",
                                block_text);
      }
      delete[] block_text;
      ++block_index;
      if (strings[string_index] == nullptr) break;
    } while (it->Next(tesseract::RIL_BLOCK));
    EXPECT_TRUE(strings[string_index] == nullptr);
  }

  // Tests that approximate order of the biggest text blocks is correct.
  // Correctness is tested by the following simple rules:
  // If a block overlaps its predecessor in x, then it must be below it.
  // otherwise, if the block is not below its predecessor, then it must
  // be to the left of it if right_to_left is true, or to the right otherwise.
  void VerifyRoughBlockOrder(bool right_to_left, ResultIterator* it) {
    int prev_left = 0;
    int prev_top = 0;
    int prev_right = 0;
    int prev_bottom = 0;
    it->Begin();
    do {
      int left, top, right, bottom;
      if (it->BoundingBox(tesseract::RIL_BLOCK, &left, &top, &right, &bottom) &&
          PTIsTextType(it->BlockType()) && right - left > 800 &&
          bottom - top > 200) {
        if (prev_right > prev_left) {
          if (min(right, prev_right) > max(left, prev_left)) {
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
        prev_top = top;
        prev_right = right;
        prev_bottom = bottom;
      }
    } while (it->Next(tesseract::RIL_BLOCK));
  }

  // Tests that every blob assigned to the biggest text blocks is contained
  // fully within its block by testing that the block polygon winds around
  // the center of the bounding boxes of the outlines in the blob.
  void VerifyTotalContainment(int winding_target, MutableIterator* it) {
    it->Begin();
    do {
      int left, top, right, bottom;
      if (it->BoundingBox(tesseract::RIL_BLOCK, &left, &top, &right, &bottom) &&
          PTIsTextType(it->BlockType()) && right - left > 800 &&
          bottom - top > 200) {
        const PAGE_RES_IT* pr_it = it->PageResIt();
        POLY_BLOCK* pb = pr_it->block()->block->poly_block();
        CHECK(pb != nullptr);
        FCOORD skew = pr_it->block()->block->skew();
        EXPECT_GT(skew.x(), 0.0f);
        EXPECT_GT(skew.y(), 0.0f);
        // Iterate the words in the block.
        MutableIterator word_it = *it;
        do {
          const PAGE_RES_IT* w_it = word_it.PageResIt();
          // Iterate the blobs in the word.
          C_BLOB_IT b_it(w_it->word()->word->cblob_list());
          for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
            C_BLOB* blob = b_it.data();
            // Iterate the outlines in the blob.
            C_OUTLINE_IT ol_it(blob->out_list());
            for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
              C_OUTLINE* ol = ol_it.data();
              TBOX box = ol->bounding_box();
              ICOORD middle((box.left() + box.right()) / 2,
                            (box.top() + box.bottom()) / 2);
              EXPECT_EQ(winding_target, pb->winding_number(middle));
            }
          }
        } while (word_it.Next(tesseract::RIL_WORD) &&
                 !word_it.IsAtBeginningOf(tesseract::RIL_BLOCK));
      }
    } while (it->Next(tesseract::RIL_BLOCK));
  }

  Pix* src_pix_;
  string ocr_text_;
  tesseract::TessBaseAPI api_;
};

// Tests that Tesseract gets the important blocks and in the right order
// on a UNLV page numbered 8087_054.3B.tif. (Dubrovnik)
TEST_F(LayoutTest, UNLV8087_054) {
  SetImage("8087_054.3B.tif", "eng");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  // Check iterator position.
  tesseract::ResultIterator* it = api_.GetIterator();
  VerifyBlockTextOrder(kStrings8087_054, kBlocks8087_054, it);
  delete it;
}

// Tests that Tesseract gets the important blocks and in the right order
// on a UNLV page numbered 8087_054.3B.tif. (Dubrovnik)
TEST_F(LayoutTest, HebrewOrderingAndSkew) {
  SetImage("GOOGLE:13510798882202548:74:84.sj-79.tif", "eng");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  tesseract::MutableIterator* it = api_.GetMutableIterator();
  // In eng mode, block order should not be RTL.
  VerifyRoughBlockOrder(false, it);
  VerifyTotalContainment(1, it);
  delete it;
  // Now try again using Hebrew.
  SetImage("GOOGLE:13510798882202548:74:84.sj-79.tif", "heb");
  // Just run recognition.
  EXPECT_EQ(api_.Recognize(nullptr), 0);
  it = api_.GetMutableIterator();
  // In heb mode, block order should be RTL.
  VerifyRoughBlockOrder(true, it);
  // And blobs should still be fully contained.
  VerifyTotalContainment(-1, it);
  delete it;
}

}  // namespace
