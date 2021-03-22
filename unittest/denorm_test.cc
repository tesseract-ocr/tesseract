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

#include "blobs.h"
#include "normalis.h"

#include "include_gunit.h"

namespace tesseract {

class DENORMTest : public testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

public:
  void TearDown() override {}

  void ExpectCorrectTransform(const DENORM &denorm, const TPOINT &src, const TPOINT &result,
                              bool local) {
    TPOINT normed;
    if (local) {
      denorm.LocalNormTransform(src, &normed);
    } else {
      denorm.NormTransform(nullptr, src, &normed);
    }
    EXPECT_EQ(result.x, normed.x);
    EXPECT_EQ(result.y, normed.y);
    // Now undo
    TPOINT denormed;
    if (local) {
      denorm.LocalDenormTransform(normed, &denormed);
    } else {
      denorm.DenormTransform(nullptr, normed, &denormed);
    }
    EXPECT_EQ(src.x, denormed.x);
    EXPECT_EQ(src.y, denormed.y);
  }
};

// Tests a simple baseline-style normalization.
TEST_F(DENORMTest, NoRotations) {
  DENORM denorm;
  denorm.SetupNormalization(nullptr, nullptr, nullptr, 1000.0f, 2000.0f, 2.0f, 3.0f, 0.0f,
                            static_cast<float>(kBlnBaselineOffset));
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
  denorm.SetupNormalization(nullptr, &rotation90, nullptr, 1000.0f, 2000.0f, 2.0f, 3.0f, 0.0f,
                            static_cast<float>(kBlnBaselineOffset));

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
  denorm.SetupNormalization(nullptr, nullptr, nullptr, 1000.0f, 2000.0f, 2.0f, 3.0f, 0.0f,
                            static_cast<float>(kBlnBaselineOffset));

  DENORM denorm2;
  FCOORD rotation90(0.0f, 1.0f);
  denorm2.SetupNormalization(nullptr, &rotation90, &denorm, 128.0f, 128.0f, 0.5f, 0.25f, 0.0f,
                             0.0f);
  TPOINT pt1(1050, 2000);
  TPOINT result1(100, kBlnBaselineOffset);
  ExpectCorrectTransform(denorm, pt1, result1, true);
  ExpectCorrectTransform(denorm, pt1, result1, false);
  TPOINT result2(kBlnBaselineOffset / 4, -14);
  ExpectCorrectTransform(denorm2, result1, result2, true);
  ExpectCorrectTransform(denorm2, pt1, result2, false);
}

} // namespace tesseract
