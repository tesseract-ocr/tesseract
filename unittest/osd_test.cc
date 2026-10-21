///////////////////////////////////////////////////////////////////////
// File:        osd_test.cc
// Description: OSD Tests for Tesseract.
// Author:      ShreeDevi Kumar
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

// based on https://gist.github.com/amitdo/7c7a522004dd79b398340c9595b377e1

// expects clones of tessdata, tessdata_fast and tessdata_best repos

//#include "log.h"
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>
#include <algorithm>
#include <iostream>
#include <memory> // std::unique_ptr
#include <string>
#include <vector>
#include "include_gunit.h"
#include "image.h"

namespace tesseract {

class TestClass : public testing::Test {
protected:
};

#ifndef DISABLED_LEGACY_ENGINE
static void OSDTester(int expected_deg, const char *imgname, const char *tessdatadir) {
  // log.info() << tessdatadir << " for image: " << imgname << std::endl;
  auto api = std::make_unique<tesseract::TessBaseAPI>();
  ASSERT_FALSE(api->Init(tessdatadir, "osd")) << "Could not initialize tesseract.";
  Image image = pixRead(imgname);
  ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
  api->SetImage(image);
  int orient_deg;
  float orient_conf;
  const char *script_name;
  float script_conf;
  bool detected =
      api->DetectOrientationScript(&orient_deg, &orient_conf, &script_name, &script_conf);
  ASSERT_FALSE(!detected) << "Failed to detect OSD.";
  printf(
      "************ Orientation in degrees: %d, Orientation confidence: %.2f\n"
      "             Script: %s, Script confidence: %.2f\n",
      orient_deg, orient_conf, script_name, script_conf);
  EXPECT_EQ(expected_deg, orient_deg);
  api->End();
  image.destroy();
}
#endif

class OSDTest : public TestClass,
                public ::testing::WithParamInterface<std::tuple<int, const char *, const char *>> {
};

TEST_P(OSDTest, MatchOrientationDegrees) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because TessBaseAPI::DetectOrientationScript is missing.
  GTEST_SKIP();
#else
  OSDTester(std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()));
#endif
}

INSTANTIATE_TEST_SUITE_P(TessdataEngEuroHebrew, OSDTest,
                         ::testing::Combine(::testing::Values(0),
                                            ::testing::Values(TESTING_DIR "/phototest.tif",
                                                              TESTING_DIR "/eurotext.tif",
                                                              TESTING_DIR "/hebrew.png"),
                                            ::testing::Values(TESSDATA_DIR)));

INSTANTIATE_TEST_SUITE_P(TessdataBestEngEuroHebrew, OSDTest,
                         ::testing::Combine(::testing::Values(0),
                                            ::testing::Values(TESTING_DIR "/phototest.tif",
                                                              TESTING_DIR "/eurotext.tif",
                                                              TESTING_DIR "/hebrew.png"),
                                            ::testing::Values(TESSDATA_DIR "_best")));

INSTANTIATE_TEST_SUITE_P(TessdataFastEngEuroHebrew, OSDTest,
                         ::testing::Combine(::testing::Values(0),
                                            ::testing::Values(TESTING_DIR "/phototest.tif",
                                                              TESTING_DIR "/eurotext.tif",
                                                              TESTING_DIR "/hebrew.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_SUITE_P(TessdataFastRotated90, OSDTest,
                         ::testing::Combine(::testing::Values(90),
                                            ::testing::Values(TESTING_DIR
                                                              "/phototest-rotated-R.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_SUITE_P(TessdataFastRotated180, OSDTest,
                         ::testing::Combine(::testing::Values(180),
                                            ::testing::Values(TESTING_DIR
                                                              "/phototest-rotated-180.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_SUITE_P(TessdataFastRotated270, OSDTest,
                         ::testing::Combine(::testing::Values(270),
                                            ::testing::Values(TESTING_DIR
                                                              "/phototest-rotated-L.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_SUITE_P(TessdataFastDevaRotated270, OSDTest,
                         ::testing::Combine(::testing::Values(270),
                                            ::testing::Values(TESTING_DIR
                                                              "/devatest-rotated-270.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

INSTANTIATE_TEST_SUITE_P(TessdataFastDeva, OSDTest,
                         ::testing::Combine(::testing::Values(0),
                                            ::testing::Values(TESTING_DIR "/devatest.png"),
                                            ::testing::Values(TESSDATA_DIR "_fast")));

#ifndef DISABLED_LEGACY_ENGINE
// phototest.tif anchor words. Both survive a correct quarter-turn.
static const char *kMixedAnchorA = "jumped";
static const char *kMixedAnchorB = "lazy";

// Leptonica pixRotate90 direction. 0 leaves the image upright.
const int kTurnUpright = 0;
const int kTurnClockwise = 1;
const int kTurnCounterclockwise = -1;

// Clear vertical gutter so the two orientations stay separate blocks, and a
// much smaller gap when a side is stacked only to make it the dominant direction.
const int kMixedGutter = 128;
const int kStackGap = 16;

struct MixedBlock {
  Orientation orientation;
  WritingDirection writing_direction;
  std::string text;
};

static Image RotateQuarter(Image src, int direction) {
  if (direction == 0) {
    return src.copy();
  }
  return pixRotate90(src, direction);
}

// Dominant region on the left (stacked when requested), minority on the right.
// Stacking keeps blob counts from tying at the 0.5 vertical-text ratio.
static Image ComposeMixedPage(Image src, int dominant_turn, int minority_turn, bool stack_dominant) {
  Image dominant = RotateQuarter(src, dominant_turn);
  Image minority = RotateQuarter(src, minority_turn);
  int copies = stack_dominant ? 2 : 1;
  int dom_w = pixGetWidth(dominant);
  int dom_h = pixGetHeight(dominant);
  int min_w = pixGetWidth(minority);
  int min_h = pixGetHeight(minority);
  int stack_h = copies * dom_h + (copies - 1) * kStackGap;
  int width = dom_w + kMixedGutter + min_w;
  int height = std::max(stack_h, min_h);
  Image page(pixCreate(width, height, pixGetDepth(src)));
  l_int32 xres = pixGetXRes(src);
  l_int32 yres = pixGetYRes(src);
  if (xres > 0 && yres > 0) {
    pixSetResolution(page, xres, yres);
  }
  for (int i = 0; i < copies; ++i) {
    pixRasterop(page, 0, i * (dom_h + kStackGap), dom_w, dom_h, PIX_SRC, dominant, 0, 0);
  }
  int minor_y = std::max(0, (height - min_h) / 2);
  pixRasterop(page, dom_w + kMixedGutter, minor_y, min_w, min_h, PIX_SRC, minority, 0, 0);
  dominant.destroy();
  minority.destroy();
  return page;
}

// hOCR textangle is counterclockwise from upright. OSD's orient_deg is the
// clockwise rotation of the glyph away from upright (orientation id * 90).
// Those numbers are not interchangeable: a clockwise quarter-turn is OSD 90
// and hOCR textangle 270 (ORIENTATION_PAGE_RIGHT).
static int HocrTextAngle(Orientation orientation) {
  return 360 - static_cast<int>(orientation) * 90;
}

static bool RecognizeAutoOsd(Image image, const char *margin, std::string *hocr,
                             std::vector<MixedBlock> *blocks) {
  TessBaseAPI api;
  if (api.Init(TESSDATA_DIR, "eng") != 0) {
    return false;
  }
  api.SetPageSegMode(PSM_AUTO_OSD);
  if (margin != nullptr) {
    api.SetVariable("min_orientation_margin", margin);
  }
  api.SetImage(image);
  if (api.Recognize(nullptr) < 0) {
    api.End();
    return false;
  }
  char *hocr_text = api.GetHOCRText(0);
  *hocr = hocr_text == nullptr ? "" : hocr_text;
  delete[] hocr_text;
  ResultIterator *it = api.GetIterator();
  if (it != nullptr) {
    do {
      MixedBlock block;
      TextlineOrder order;
      float deskew = 0.0f;
      it->Orientation(&block.orientation, &block.writing_direction, &order, &deskew);
      char *text = it->GetUTF8Text(RIL_BLOCK);
      block.text = text == nullptr ? "" : text;
      delete[] text;
      blocks->push_back(block);
    } while (it->Next(RIL_BLOCK));
    delete it;
  }
  ResultIterator *words = api.GetIterator();
  if (words != nullptr) {
    int width = pixGetWidth(image);
    int height = pixGetHeight(image);
    do {
      int left, top, right, bottom;
      if (words->BoundingBox(RIL_WORD, &left, &top, &right, &bottom)) {
        EXPECT_GE(left, 0);
        EXPECT_GE(top, 0);
        EXPECT_LE(right, width);
        EXPECT_LE(bottom, height);
      }
    } while (words->Next(RIL_WORD));
    delete words;
  }
  api.End();
  return true;
}

static std::string TextForOrientation(const std::vector<MixedBlock> &blocks, Orientation orientation) {
  std::string text;
  for (const auto &block : blocks) {
    if (block.orientation == orientation) {
      text += block.text;
      // Corrected Latin is read left to right. A 90 degree classify_rotation
      // would report vertical writing and disagree with the block textangle.
      EXPECT_EQ(WRITING_DIRECTION_LEFT_TO_RIGHT, block.writing_direction);
    }
  }
  return text;
}

static void ExpectAnchors(const std::string &text) {
  EXPECT_NE(text.find(kMixedAnchorA), std::string::npos) << text;
  EXPECT_NE(text.find(kMixedAnchorB), std::string::npos) << text;
}

static void ExpectTextAngle(const std::string &hocr, Orientation orientation) {
  if (orientation == ORIENTATION_PAGE_UP) {
    return;
  }
  int angle = HocrTextAngle(orientation);
  if (orientation == ORIENTATION_PAGE_RIGHT) {
    // Clockwise 90 (OSD 90) is not hOCR 90.
    EXPECT_EQ(270, angle);
    EXPECT_NE(90, angle);
  } else if (orientation == ORIENTATION_PAGE_LEFT) {
    EXPECT_EQ(90, angle);
    EXPECT_NE(270, angle);
  }
  EXPECT_NE(hocr.find("textangle " + std::to_string(angle)), std::string::npos) << hocr;
}

// direction is the leptonica quarter-turn applied to an upright page.
// The image orientation of that text is PAGE_RIGHT for clockwise and
// PAGE_LEFT for counterclockwise.
static Orientation OrientationForTurn(int direction) {
  if (direction == kTurnClockwise) {
    return ORIENTATION_PAGE_RIGHT;
  }
  if (direction == kTurnCounterclockwise) {
    return ORIENTATION_PAGE_LEFT;
  }
  return ORIENTATION_PAGE_UP;
}

static void ExpectStandaloneTurn(Image src, int direction) {
  Image turned = RotateQuarter(src, direction);
  std::string hocr;
  std::vector<MixedBlock> blocks;
  if (!RecognizeAutoOsd(turned, nullptr, &hocr, &blocks)) {
    turned.destroy();
    GTEST_SKIP() << "eng traineddata not available";
  }
  Orientation orientation = OrientationForTurn(direction);
  ExpectAnchors(TextForOrientation(blocks, orientation));
  ExpectTextAngle(hocr, orientation);
  turned.destroy();
}

// Upright dominant page plus one quarter-turned copy. The clockwise minority
// is upside down under the fixed quarter-turn convention; block-local OSD
// must match standalone recognition of that same rotation.
static void ExpectUprightDominantMinority(Image src, int minority_turn) {
  ExpectStandaloneTurn(src, minority_turn);
  if (::testing::Test::IsSkipped()) {
    return;
  }
  Image page = ComposeMixedPage(src, kTurnUpright, minority_turn, true);
  std::string hocr;
  std::vector<MixedBlock> blocks;
  bool ok = RecognizeAutoOsd(page, nullptr, &hocr, &blocks);
  page.destroy();
  ASSERT_TRUE(ok);
  // Anchors in both orientations: the minority was not given the page orientation.
  ExpectAnchors(TextForOrientation(blocks, ORIENTATION_PAGE_UP));
  Orientation minority = OrientationForTurn(minority_turn);
  ExpectAnchors(TextForOrientation(blocks, minority));
  ExpectTextAngle(hocr, minority);
}
#endif // !DISABLED_LEGACY_ENGINE

TEST(OSDMixedOrientationTest, ClockwiseMinorityMatchesStandalone) {
#ifdef DISABLED_LEGACY_ENGINE
  GTEST_SKIP() << "Skip test because block OSD requires the legacy engine.";
#else
  Image src = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(src != nullptr);
  ExpectUprightDominantMinority(src, kTurnClockwise);
  src.destroy();
#endif
}

TEST(OSDMixedOrientationTest, CounterclockwiseMinorityMatchesStandalone) {
#ifdef DISABLED_LEGACY_ENGINE
  GTEST_SKIP() << "Skip test because block OSD requires the legacy engine.";
#else
  Image src = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(src != nullptr);
  ExpectUprightDominantMinority(src, kTurnCounterclockwise);
  src.destroy();
#endif
}

// Same side-by-side layout with the rotated copy dominant. The upright
// minority must stay page-up, and every word box must stay inside the image.
TEST(OSDMixedOrientationTest, DominantRotatedKeepsUprightMinority) {
#ifdef DISABLED_LEGACY_ENGINE
  GTEST_SKIP() << "Skip test because block OSD requires the legacy engine.";
#else
  Image src = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(src != nullptr);
  for (int dominant_turn : {kTurnClockwise, kTurnCounterclockwise}) {
    Image page = ComposeMixedPage(src, dominant_turn, kTurnUpright, true);
    std::string hocr;
    std::vector<MixedBlock> blocks;
    if (!RecognizeAutoOsd(page, nullptr, &hocr, &blocks)) {
      page.destroy();
      src.destroy();
      GTEST_SKIP() << "eng traineddata not available";
    }
    Orientation dominant = OrientationForTurn(dominant_turn);
    ExpectAnchors(TextForOrientation(blocks, dominant));
    ExpectAnchors(TextForOrientation(blocks, ORIENTATION_PAGE_UP));
    ExpectTextAngle(hocr, dominant);
    page.destroy();
  }
  src.destroy();
#endif
}

// A huge margin makes the local CW-versus-CCW evidence "weak". The minority
// block keeps the historical clockwise quarter-turn (PAGE_LEFT / textangle 90)
// and the upright block is still read, so page-level scores were not replaced.
TEST(OSDMixedOrientationTest, WeakEvidenceKeepsConvention) {
#ifdef DISABLED_LEGACY_ENGINE
  GTEST_SKIP() << "Skip test because block OSD requires the legacy engine.";
#else
  Image src = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(src != nullptr);
  Image page = ComposeMixedPage(src, kTurnUpright, kTurnClockwise, true);
  std::string hocr;
  std::vector<MixedBlock> blocks;
  if (!RecognizeAutoOsd(page, "1000", &hocr, &blocks)) {
    page.destroy();
    src.destroy();
    GTEST_SKIP() << "eng traineddata not available";
  }
  ExpectAnchors(TextForOrientation(blocks, ORIENTATION_PAGE_UP));
  // Fallback is PAGE_LEFT / textangle 90. textangle 270 would mean the weak
  // CW-versus-CCW margin was treated as decisive.
  ExpectTextAngle(hocr, ORIENTATION_PAGE_LEFT);
  EXPECT_EQ(hocr.find("textangle 270"), std::string::npos) << hocr;
  page.destroy();
  src.destroy();
#endif
}

// A short clockwise line has too few usable blobs, so os_detect_blobs declines
// the block and the historical quarter-turn stays (PAGE_LEFT / textangle 90).
// The upright block is still read, so the page orientation was not replaced.
TEST(OSDMixedOrientationTest, FewMinorityCharactersKeepConvention) {
#ifdef DISABLED_LEGACY_ENGINE
  GTEST_SKIP() << "Skip test because block OSD requires the legacy engine.";
#else
  Image src = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(src != nullptr);
  // First phototest line starts near y=92. This clip is about ten glyphs.
  Box *clip = boxCreate(30, 88, 170, 36);
  Image line = pixClipRectangle(src, clip, nullptr);
  boxDestroy(&clip);
  ASSERT_TRUE(line != nullptr);
  Image turned = pixRotate90(line, kTurnClockwise);
  line.destroy();
  int gutter = kMixedGutter;
  int width = pixGetWidth(src) + gutter + pixGetWidth(turned);
  int height = std::max(pixGetHeight(src), pixGetHeight(turned));
  Image page(pixCreate(width, height, pixGetDepth(src)));
  l_int32 xres = pixGetXRes(src);
  l_int32 yres = pixGetYRes(src);
  if (xres > 0 && yres > 0) {
    pixSetResolution(page, xres, yres);
  }
  pixRasterop(page, 0, 0, pixGetWidth(src), pixGetHeight(src), PIX_SRC, src, 0, 0);
  pixRasterop(page, pixGetWidth(src) + gutter, 0, pixGetWidth(turned), pixGetHeight(turned),
              PIX_SRC, turned, 0, 0);
  turned.destroy();
  std::string hocr;
  std::vector<MixedBlock> blocks;
  if (!RecognizeAutoOsd(page, nullptr, &hocr, &blocks)) {
    page.destroy();
    src.destroy();
    GTEST_SKIP() << "eng traineddata not available";
  }
  ExpectAnchors(TextForOrientation(blocks, ORIENTATION_PAGE_UP));
  ExpectTextAngle(hocr, ORIENTATION_PAGE_LEFT);
  EXPECT_EQ(hocr.find("textangle 270"), std::string::npos) << hocr;
  page.destroy();
  src.destroy();
#endif
}

} // namespace tesseract
