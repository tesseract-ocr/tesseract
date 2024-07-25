// (C) Copyright 2022, Povilas Kanapickas <povilas@radix.lt>.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "blob_bounds_calculator.h"

#include "include_gunit.h"

namespace tesseract {

namespace {

BoxBoundariesCalculatorConfig get_default_config() {
    BoxBoundariesCalculatorConfig config;
    config.merge_cost = 1;
    config.split_cost = 1;
    config.pos_diff_cost = 1;
    config.max_pos_diff = 2;
    config.box_with_no_symbol_cost = 2;
    config.symbol_with_no_box_cost = 2;
    return config;
}

} // namespace

TEST(BoxBoundariesCalculatorTest, MatchesExactly) {
  BoxBoundariesCalculator calc{{{10, 20}, {21, 30}, {31, 40}, {41, 50}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 2},
    {31, 2, 40, 3},
    {41, 3, 50, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {10, 20}, {20, 30}, {30, 40}, {40, 50}
  }));
}

TEST(BoxBoundariesCalculatorTest, OneMergedInMiddle) {
  BoxBoundariesCalculator calc{{{10, 20}, {21, 40}, {41, 50}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 2},
    {30, 1, 40, 2},
    {41, 2, 50, 3}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {10, 20}, {20, 30}, {30, 40}, {40, 50}
  }));
}

TEST(BoxBoundariesCalculatorTest, OneSplit) {
  BoxBoundariesCalculator calc{{{10, 20}, {21, 25}, {26, 30}, {31, 40}, {41, 50}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 3},
    {31, 3, 40, 4},
    {41, 4, 50, 5}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {10, 20}, {20, 30}, {30, 40}, {40, 50}
  }));
}

TEST(BoxBoundariesCalculatorTest, ManySplitAtEnd) {
  BoxBoundariesCalculator calc{
    {
      {10, 20}, {21, 30}, {31, 40}, {41, 50}, {51, 60}, {61, 70}
    },
    get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 2},
    {31, 2, 40, 3},
    {41, 3, 70, 6}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {10, 20}, {20, 30}, {30, 40}, {40, 50}
  }));
}

TEST(BoxBoundariesCalculatorTest, ShiftedSymbolPositionsForward) {
  BoxBoundariesCalculator calc{{{10, 20}, {21, 30}, {31, 40}, {41, 50}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 2},
    {31, 2, 40, 3},
    {41, 3, 50, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {15, 25}, {25, 35}, {35, 45}, {45, 55}
  }));
}

TEST(BoxBoundariesCalculatorTest, VeryShiftedSymbolPositionsForward) {
  BoxBoundariesCalculator calc{{{10, 20}, {21, 30}, {31, 40}, {41, 50}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {10, 0, 20, 1},
    {21, 1, 30, 2},
    {31, 2, 40, 3},
    {41, 3, 50, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {25, 35}, {35, 45}, {45, 55}, {55, 65}
  }));
}

TEST(BoxBoundariesCalculatorTest, ShiftedSymbolPositionsBackward) {
  BoxBoundariesCalculator calc{{{110, 120}, {121, 130}, {131, 140}, {141, 150}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {110, 0, 120, 1},
    {121, 1, 130, 2},
    {131, 2, 140, 3},
    {141, 3, 150, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {105, 115}, {115, 125}, {125, 135}, {135, 145}
  }));
}

TEST(BoxBoundariesCalculatorTest, VeryShiftedSymbolPositionsBackward) {
  BoxBoundariesCalculator calc{{{110, 120}, {121, 130}, {131, 140}, {141, 150}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {110, 0, 120, 1},
    {121, 1, 130, 2},
    {131, 2, 140, 3},
    {141, 3, 150, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {95, 105}, {105, 115}, {115, 125}, {125, 135}
  }));
}

TEST(BoxBoundariesCalculatorTest, HoleInMiddle) {
  BoxBoundariesCalculator calc{{{110, 120}, {121, 130}, {131, 140}, {141, 150}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {110, 0, 120, 1},
    {121, 1, 130, 2},
    {131, 2, 140, 3},
    {141, 3, 150, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {105, 115}, {115, 125}, {135, 145}, {145, 155}
  }));
}

TEST(BoxBoundariesCalculatorTest, LargeHoleInMiddle) {
  BoxBoundariesCalculator calc{{{110, 120}, {121, 130}, {131, 140}, {141, 150}},
                               get_default_config()};

  std::vector<CharacterBoundaries> expected = {
    {110, 0, 120, 1},
    {121, 1, 130, 2},
    {131, 2, 140, 3},
    {141, 3, 150, 4}
  };

  ASSERT_EQ(expected, calc.calculate_bounds({
    {95, 105}, {105, 115}, {145, 155}, {155, 165}
  }));
}

} // namespace tesseract
