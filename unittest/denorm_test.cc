
#include "tesseract/ccstruct/blobs.h"
#include "tesseract/ccstruct/normalis.h"

namespace {

class DENORMTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }

  void ExpectCorrectTransform(const DENORM& denorm, const TPOINT& src,
                              const TPOINT& result, bool local) {
    TPOINT normed;
    if (local)
      denorm.LocalNormTransform(src, &normed);
    else
      denorm.NormTransform(NULL, src, &normed);
    EXPECT_EQ(result.x, normed.x);
    EXPECT_EQ(result.y, normed.y);
    // Now undo
    TPOINT denormed;
    if (local)
      denorm.LocalDenormTransform(normed, &denormed);
    else
      denorm.DenormTransform(NULL, normed, &denormed);
    EXPECT_EQ(src.x, denormed.x);
    EXPECT_EQ(src.y, denormed.y);
  }
};

// Tests a simple baseline-style normalization.
TEST_F(DENORMTest, NoRotations) {
  DENORM denorm;
  denorm.SetupNormalization(NULL, NULL, NULL,
                            1000.0f, 2000.0f, 2.0f, 3.0f,
                            0.0f, static_cast<float>(kBlnBaselineOffset));
  TPOINT pt1(1100, 2000);
  TPOINT result1(200, kBlnBaselineOffset);
  ExpectCorrectTransform(denorm, pt1, result1, true);
  ExpectCorrectTransform(denorm, pt1, result1, false);
  TPOINT pt2(900, 2100);
  TPOINT result2(-200, 300 + kBlnBaselineOffset);
  ExpectCorrectTransform(denorm, pt2, result2, true);
  ExpectCorrectTransform(denorm, pt2, result2, false);
}

// Tests a simple baseline-style normalization with a rotation.
TEST_F(DENORMTest, WithRotations) {
  DENORM denorm;
  FCOORD rotation90(0.0f, 1.0f);
  denorm.SetupNormalization(NULL, &rotation90, NULL,
                            1000.0f, 2000.0f, 2.0f, 3.0f,
                            0.0f, static_cast<float>(kBlnBaselineOffset));

  TPOINT pt1(1100, 2000);
  TPOINT result1(0, 200 + kBlnBaselineOffset);
  ExpectCorrectTransform(denorm, pt1, result1, true);
  ExpectCorrectTransform(denorm, pt1, result1, false);
  TPOINT pt2(900, 2100);
  TPOINT result2(-300, kBlnBaselineOffset - 200);
  ExpectCorrectTransform(denorm, pt2, result2, true);
  ExpectCorrectTransform(denorm, pt2, result2, false);
}

// Tests a simple baseline-style normalization with a second rotation & scale.
TEST_F(DENORMTest, Multiple) {
  DENORM denorm;
  denorm.SetupNormalization(NULL, NULL, NULL,
                            1000.0f, 2000.0f, 2.0f, 3.0f,
                            0.0f, static_cast<float>(kBlnBaselineOffset));

  DENORM denorm2;
  FCOORD rotation90(0.0f, 1.0f);
  denorm2.SetupNormalization(NULL, &rotation90, &denorm,
                            128.0f, 128.0f, 0.5f, 0.25f, 0.0f, 0.0f);
  TPOINT pt1(1050, 2000);
  TPOINT result1(100, kBlnBaselineOffset);
  ExpectCorrectTransform(denorm, pt1, result1, true);
  ExpectCorrectTransform(denorm, pt1, result1, false);
  TPOINT result2(kBlnBaselineOffset / 4, -14);
  ExpectCorrectTransform(denorm2, result1, result2, true);
  ExpectCorrectTransform(denorm2, pt1, result2, false);
}

}  // namespace.
