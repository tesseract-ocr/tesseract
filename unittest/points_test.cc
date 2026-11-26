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

#include "points.h"

#include "include_gunit.h"

namespace tesseract {

namespace {

// Helpers to use the compound assignment operators in constant expressions.
constexpr ICOORD IPlus(ICOORD a, const ICOORD &b) {
  a += b;
  return a;
}
constexpr ICOORD IMinus(ICOORD a, const ICOORD &b) {
  a -= b;
  return a;
}
constexpr ICOORD IMul(ICOORD a, TDimension s) {
  a *= s;
  return a;
}
constexpr ICOORD IDiv(ICOORD a, TDimension s) {
  a /= s;
  return a;
}
constexpr FCOORD FPlus(FCOORD a, const FCOORD &b) {
  a += b;
  return a;
}
constexpr FCOORD FMinus(FCOORD a, const FCOORD &b) {
  a -= b;
  return a;
}
constexpr FCOORD FMul(FCOORD a, float s) {
  a *= s;
  return a;
}

} // namespace

class PointsTest : public testing::Test {
public:
  void SetUp() override {}

  void TearDown() override {}
};

// The coordinate arithmetic operators are declared constexpr. Make sure they
// can actually produce constant expressions (this is required even in C++20,
// see P1331R2) by using them in constant expressions. This used to fail to
// compile, e.g. with GCC:
//   error: uninitialized variable 'result' in 'constexpr' function
// because the FCOORD default constructor was not constexpr, so the constexpr
// FCOORD operators could never produce a constant expression (see issue
// #4664).
TEST_F(PointsTest, ICooordArithmeticIsConstant) {
  static_assert(ICOORD(3, 4) == ICOORD(1, 2) + ICOORD(2, 2), "");
  static_assert(ICOORD(1, 2) - ICOORD(2, 2) == ICOORD(-1, 0), "");
  static_assert(!ICOORD(3, 4) == ICOORD(-4, 3), "");
  static_assert(-ICOORD(3, 4) == ICOORD(-3, -4), "");
  static_assert(ICOORD(3, 4) * 2 == ICOORD(6, 8), "");
  static_assert(2 * ICOORD(3, 4) == ICOORD(6, 8), "");
  static_assert(ICOORD(6, 8) / 2 == ICOORD(3, 4), "");
  static_assert(ICOORD(3, 4) % ICOORD(1, 2) == 11, "");
  static_assert(ICOORD(3, 4) * ICOORD(1, 2) == 2, "");
  // Default constructor.
  static_assert(ICOORD() == ICOORD(0, 0), "");
  // Compound assignment operators.
  static_assert(IPlus(ICOORD(1, 2), ICOORD(2, 2)) == ICOORD(3, 4), "");
  static_assert(IMinus(ICOORD(3, 4), ICOORD(1, 2)) == ICOORD(2, 2), "");
  static_assert(IMul(ICOORD(3, 4), 2) == ICOORD(6, 8), "");
  static_assert(IDiv(ICOORD(6, 8), 2) == ICOORD(3, 4), "");
  SUCCEED();
}

TEST_F(PointsTest, FCoordsArithmeticIsConstant) {
  static_assert(FCOORD(1.5f, 2.5f) == FCOORD(1.0f, 2.0f) + FCOORD(0.5f, 0.5f),
                "");
  static_assert(FCOORD(1.0f, 2.0f) - FCOORD(0.5f, 0.5f) == FCOORD(0.5f, 1.5f),
                "");
  static_assert(!FCOORD(3.0f, 4.0f) == FCOORD(-4.0f, 3.0f), "");
  static_assert(-FCOORD(3.0f, 4.0f) == FCOORD(-3.0f, -4.0f), "");
  static_assert(FCOORD(3.0f, 4.0f) * 2.0f == FCOORD(6.0f, 8.0f), "");
  static_assert(2.0f * FCOORD(3.0f, 4.0f) == FCOORD(6.0f, 8.0f), "");
  static_assert(FCOORD(3.0f, 4.0f) % FCOORD(1.0f, 2.0f) == 11.0f, "");
  static_assert(FCOORD(3.0f, 4.0f) * FCOORD(1.0f, 2.0f) == 2.0f, "");
  // Default constructor and the constructor from ICOORD.
  static_assert(FCOORD() == FCOORD(0.0f, 0.0f), "");
  static_assert(FCOORD(ICOORD(3, 4)) == FCOORD(3.0f, 4.0f), "");
  // Compound assignment operators.
  static_assert(
      FPlus(FCOORD(1.0f, 2.0f), FCOORD(0.5f, 0.5f)) == FCOORD(1.5f, 2.5f), "");
  static_assert(
      FMinus(FCOORD(3.0f, 4.0f), FCOORD(1.0f, 2.0f)) == FCOORD(2.0f, 2.0f), "");
  static_assert(FMul(FCOORD(3.0f, 4.0f), 2.0f) == FCOORD(6.0f, 8.0f), "");
  SUCCEED();
}

// Runtime sanity checks of the same operators.
TEST_F(PointsTest, ICooordArithmetic) {
  ICOORD a(3, 4);
  ICOORD b(1, 2);

  EXPECT_EQ(ICOORD(4, 6), a + b);
  EXPECT_EQ(ICOORD(2, 2), a - b);
  EXPECT_EQ(ICOORD(-4, 3), !a);
  EXPECT_EQ(ICOORD(-3, -4), -a);
  EXPECT_EQ(ICOORD(6, 8), a * 2);
  EXPECT_EQ(ICOORD(6, 8), 2 * a);
  EXPECT_EQ(ICOORD(3, 4), ICOORD(6, 8) / 2);
  EXPECT_EQ(11, a % b); // scalar product
  EXPECT_EQ(2, a * b);  // cross product

  ICOORD c(1, 1);
  c += a;
  EXPECT_EQ(ICOORD(4, 5), c);
  c -= b;
  EXPECT_EQ(ICOORD(3, 3), c);
  c *= 2;
  EXPECT_EQ(ICOORD(6, 6), c);
  c /= 2;
  EXPECT_EQ(ICOORD(3, 3), c);

  EXPECT_EQ(ICOORD(0, 0), ICOORD());
}

TEST_F(PointsTest, FCoordsArithmetic) {
  FCOORD a(3.0f, 4.0f);
  FCOORD b(1.0f, 2.0f);

  EXPECT_EQ(FCOORD(4.0f, 6.0f), a + b);
  EXPECT_EQ(FCOORD(2.0f, 2.0f), a - b);
  EXPECT_EQ(FCOORD(-4.0f, 3.0f), !a);
  EXPECT_EQ(FCOORD(-3.0f, -4.0f), -a);
  EXPECT_EQ(FCOORD(6.0f, 8.0f), a * 2.0f);
  EXPECT_EQ(FCOORD(6.0f, 8.0f), 2.0f * a);
  EXPECT_FLOAT_EQ(11.0f, a % b); // scalar product
  EXPECT_FLOAT_EQ(2.0f, a * b);  // cross product

  FCOORD c(1.0f, 1.0f);
  c += a;
  EXPECT_EQ(FCOORD(4.0f, 5.0f), c);
  c -= b;
  EXPECT_EQ(FCOORD(3.0f, 3.0f), c);
  c *= 2.0f;
  EXPECT_EQ(FCOORD(6.0f, 6.0f), c);

  FCOORD d(2.0f, 2.0f);
  d /= 2.0f;
  EXPECT_EQ(FCOORD(1.0f, 1.0f), d);

  EXPECT_EQ(FCOORD(0.0f, 0.0f), FCOORD());
  EXPECT_EQ(FCOORD(3.0f, 4.0f), FCOORD(ICOORD(3, 4)));
}

} // namespace tesseract
