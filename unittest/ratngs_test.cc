// (C) Copyright 2026, Stefan Weil
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

#include "ratngs.h"
#include "unicharset.h"

#include <cfloat>
#include <cmath>
#include <limits>

namespace tesseract {

// Regression test for tesseract-ocr/tesseract#4627.
//
// A word that the recognizer could not classify carries the "bad" certainty
// WERD_CHOICE::kBadCertainty. Textord::CleanupSingleRowResult() sums the
// certainties of the words in a row, and the per-word confidence is computed
// as 100 + 5 * certainty (AllWordConfidences, ChoiceIterator::Confidence).
// With the old sentinel of -FLT_MAX, a row holding two such words overflowed
// the float sum to -inf (and 5 * -FLT_MAX overflowed as well), which the
// tesseract CLI turns into a SIGFPE because main1() enables the FE_OVERFLOW
// trap, and which silently yields a -inf confidence for library users.
// kBadCertainty must therefore be finite *and* small enough in magnitude that
// these float sums and scalings cannot overflow.

class RatngsTest : public ::testing::Test {
protected:
  void SetUp() override {
    unicharset_.clear();
    unicharset_.unichar_insert(" ");
  }

  // The certainty a word is given when it could not be classified.
  float BadCertainty() const {
    return WERD_CHOICE::kBadCertainty;
  }

  UNICHARSET unicharset_;
};

// The sentinel itself must be a finite float; it is the value a missing
// classification is given (WERD_CHOICE::make_bad,
// WERD_RES::FakeWordFromRatings).
TEST_F(RatngsTest, BadCertaintyIsFinite) {
  const float bad = BadCertainty();
  EXPECT_TRUE(std::isfinite(bad));
  // It is a "bad" (very low) certainty, i.e. negative.
  EXPECT_LT(bad, 0.0f);
  EXPECT_GT(bad, std::numeric_limits<float>::lowest());
}

// make_bad() (and, by construction, FakeWordFromRatings which reuses the same
// constant) must leave a finite certainty that can be summed/scaled without
// overflowing the float arithmetic used downstream.
TEST_F(RatngsTest, BadWordCertaintyIsSummable) {
  WERD_CHOICE bad(&unicharset_);
  bad.make_bad();
  const float cert = bad.certainty();
  EXPECT_TRUE(std::isfinite(cert));
  EXPECT_FLOAT_EQ(WERD_CHOICE::kBadRating, bad.rating());

  // Two unclassified words in one row (Textord::CleanupSingleRowResult sums a
  // float per row). With the former -FLT_MAX sentinel this was -inf.
  EXPECT_TRUE(std::isfinite(cert + cert));
  EXPECT_FALSE(std::isinf(cert + cert));

  // Per-word confidence (AllWordConfidences / ChoiceIterator::Confidence).
  // With the former -FLT_MAX sentinel 5 * cert overflowed to -inf.
  EXPECT_TRUE(std::isfinite(100.0f + 5.0f * cert));
  EXPECT_FALSE(std::isinf(100.0f + 5.0f * cert));
}

} // namespace tesseract.
