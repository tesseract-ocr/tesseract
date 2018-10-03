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

#include "linlsq.h"

#include "include_gunit.h"

namespace {

class LLSQTest : public testing::Test {
 public:
  void SetUp() {}

  void TearDown() {}

  void ExpectCorrectLine(const LLSQ& llsq, double m, double c, double rms,
                         double pearson, double tolerance) {
    EXPECT_NEAR(m, llsq.m(), tolerance);
    EXPECT_NEAR(c, llsq.c(llsq.m()), tolerance);
    EXPECT_NEAR(rms, llsq.rms(llsq.m(), llsq.c(llsq.m())), tolerance);
    EXPECT_NEAR(pearson, llsq.pearson(), tolerance);
  }
  FCOORD PtsMean(const std::vector<FCOORD>& pts) {
    FCOORD total(0, 0);
    for (int i = 0; i < pts.size(); i++) {
      total += pts[i];
    }
    return (pts.size() > 0) ? total / pts.size() : total;
  }
  void VerifyRmsOrth(const std::vector<FCOORD>& pts, const FCOORD& orth) {
    LLSQ llsq;
    FCOORD xavg = PtsMean(pts);
    FCOORD nvec = !orth;
    nvec.normalise();
    double expected_answer = 0;
    for (int i = 0; i < pts.size(); i++) {
      llsq.add(pts[i].x(), pts[i].y());
      double dot = nvec % (pts[i] - xavg);
      expected_answer += dot * dot;
    }
    expected_answer /= pts.size();
    expected_answer = sqrt(expected_answer);
    EXPECT_NEAR(expected_answer, llsq.rms_orth(orth), 0.0001);
  }
  void ExpectCorrectVector(const LLSQ& llsq, FCOORD correct_mean_pt,
                           FCOORD correct_vector, float tolerance) {
    FCOORD mean_pt = llsq.mean_point();
    FCOORD vector = llsq.vector_fit();
    EXPECT_NEAR(correct_mean_pt.x(), mean_pt.x(), tolerance);
    EXPECT_NEAR(correct_mean_pt.y(), mean_pt.y(), tolerance);
    EXPECT_NEAR(correct_vector.x(), vector.x(), tolerance);
    EXPECT_NEAR(correct_vector.y(), vector.y(), tolerance);
  }
};

// Tests a simple baseline-style normalization.
TEST_F(LLSQTest, BasicLines) {
  LLSQ llsq;
  llsq.add(1.0, 1.0);
  llsq.add(2.0, 2.0);
  ExpectCorrectLine(llsq, 1.0, 0.0, 0.0, 1.0, 1e-6);
  float half_root_2 = sqrt(2.0) / 2.0f;
  ExpectCorrectVector(llsq, FCOORD(1.5f, 1.5f),
                      FCOORD(half_root_2, half_root_2), 1e-6);
  llsq.remove(2.0, 2.0);
  llsq.add(1.0, 2.0);
  llsq.add(10.0, 1.0);
  llsq.add(-8.0, 1.0);
  // The point at 1,2 pulls the result away from what would otherwise be a
  // perfect fit to a horizontal line by 0.25 unit, with rms error of 0.433.
  ExpectCorrectLine(llsq, 0.0, 1.25, 0.433, 0.0, 1e-2);
  ExpectCorrectVector(llsq, FCOORD(1.0f, 1.25f), FCOORD(1.0f, 0.0f), 1e-3);
  llsq.add(1.0, 2.0, 10.0);
  // With a heavy weight, the point at 1,2 pulls the line nearer.
  ExpectCorrectLine(llsq, 0.0, 1.786, 0.41, 0.0, 1e-2);
  ExpectCorrectVector(llsq, FCOORD(1.0f, 1.786f), FCOORD(1.0f, 0.0f), 1e-3);
}

// Tests a simple baseline-style normalization with a rotation.
TEST_F(LLSQTest, Vectors) {
  LLSQ llsq;
  llsq.add(1.0, 1.0);
  llsq.add(1.0, -1.0);
  ExpectCorrectVector(llsq, FCOORD(1.0f, 0.0f), FCOORD(0.0f, 1.0f), 1e-6);
  llsq.add(0.9, -2.0);
  llsq.add(1.1, -3.0);
  llsq.add(0.9, 2.0);
  llsq.add(1.10001, 3.0);
  ExpectCorrectVector(llsq, FCOORD(1.0f, 0.0f), FCOORD(0.0f, 1.0f), 1e-3);
}

// Verify that rms_orth() actually calculates:
//   sqrt( sum (!nvec * (x_i - x_avg))^2 / n)
TEST_F(LLSQTest, RmsOrthWorksAsIntended) {
  std::vector<FCOORD> pts;
  pts.push_back(FCOORD(0.56, 0.95));
  pts.push_back(FCOORD(0.09, 0.09));
  pts.push_back(FCOORD(0.13, 0.77));
  pts.push_back(FCOORD(0.16, 0.83));
  pts.push_back(FCOORD(0.45, 0.79));
  VerifyRmsOrth(pts, FCOORD(1, 0));
  VerifyRmsOrth(pts, FCOORD(1, 1));
  VerifyRmsOrth(pts, FCOORD(1, 2));
  VerifyRmsOrth(pts, FCOORD(2, 1));
}

}  // namespace.
