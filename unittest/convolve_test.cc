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

#include "convolve.h"
#include "networkio.h"

namespace tesseract {

TEST(ConvolveTest, PaddingFollowsTrainingState) {
  const std::vector<std::pair<int, int>> sizes = {{3, 4}, {2, 3}, {1, 1}};
  StrideMap map;
  map.SetStride(sizes);
  for (bool int_mode : {false, true}) {
    NetworkIO input;
    input.ResizeToMap(int_mode, map, 2);
    StrideMap::Index index(map);
    do {
      for (int c = 0; c < 2; ++c) {
        int value = index.index(FD_BATCH) * 30 + index.index(FD_HEIGHT) * 6 +
                    index.index(FD_WIDTH) * 2 + c - 20;
        if (int_mode) {
          input.SetPixel(index.t(), c, 128 + value, 0.0f, 128.0f);
        } else {
          input.f(index.t())[c] = value / 64.0f;
        }
      }
    } while (index.Increment());

    for (int half : {0, 1}) {
      for (int seed : {12345, 54321}) {
        Convolve convolve("convolve", 2, half, half);
        TRand randomizer, expected_randomizer;
        randomizer.set_seed(seed);
        expected_randomizer.set_seed(seed);
        convolve.SetRandomizer(&randomizer);
        for (TrainingState state :
             {TS_DISABLED, TS_ENABLED, TS_TEMP_DISABLE, TS_RE_ENABLE}) {
          convolve.SetEnableTraining(state);
          const bool training = state == TS_ENABLED || state == TS_RE_ENABLE;
          for (int repeat = 0; repeat < 2; ++repeat) {
            SCOPED_TRACE(::testing::Message()
                         << int_mode << ", " << half << ", " << seed << ", "
                         << state << ", " << repeat);
            NetworkIO output;
            convolve.Forward(false, input, nullptr, nullptr, &output);
            ASSERT_EQ(input.Width(), output.Width());
            ASSERT_EQ(2 * (2 * half + 1) * (2 * half + 1),
                      output.NumFeatures());
            index.InitToFirst();
            do {
              int b = index.index(FD_BATCH);
              int feature = 0;
              for (int dx = -half; dx <= half; ++dx) {
                for (int dy = -half; dy <= half; ++dy) {
                  int x = index.index(FD_WIDTH) + dx;
                  int y = index.index(FD_HEIGHT) + dy;
                  bool valid = x >= 0 && x < sizes[b].second && y >= 0 &&
                               y < sizes[b].first;
                  for (int c = 0; c < 2; ++c, ++feature) {
                    int value = b * 30 + y * 6 + x * 2 + c - 20;
                    if (int_mode) {
                      int expected = valid ? value : INT8_MAX;
                      if (!valid && training) {
                        expected = IntCastRounded(
                            expected_randomizer.SignedRand(INT8_MAX));
                      }
                      EXPECT_EQ(expected, output.i(index.t())[feature]);
                    } else {
                      float expected = valid ? value / 64.0f : 1.0f;
                      if (!valid && training) {
                        expected = expected_randomizer.SignedRand(1.0);
                      }
                      EXPECT_EQ(expected, output.f(index.t())[feature]);
                    }
                  }
                }
              }
            } while (index.Increment());
            // Recognition must not consume random values; training must consume
            // exactly the existing padding sequence, including on re-enable.
            TRand randomizer_copy = randomizer;
            TRand expected_copy = expected_randomizer;
            EXPECT_EQ(expected_copy.IntRand(), randomizer_copy.IntRand());
          }
        }
      }
    }
  }
}

} // namespace tesseract
