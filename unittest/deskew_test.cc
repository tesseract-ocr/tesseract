// SPDX-License-Identifier: Apache-2.0

#include "image.h"
#include <gtest/gtest.h>
#include <cmath>

namespace tesseract {

TEST(DeskewTest, PageDeskewRadians) {
  EXPECT_EQ(0, PageDeskewRadians(nullptr));
  Pix *page = pixCreate(800, 600, 1);
  ASSERT_NE(nullptr, page);
  EXPECT_EQ(0, PageDeskewRadians(page));

  const double tilt = 2 * std::acos(-1.0) / 180;
  for (double angle : {0.0, tilt}) {
    pixClearAll(page);
    for (int y = 40; y < 560; y += 30) {
      for (int x = 40; x < 760; ++x) {
        if (x % 40 < 30) {
          int top = y - static_cast<int>(x * std::tan(angle));
          for (int i = 0; i < 5; ++i) {
            pixSetPixel(page, x, top + i, 1);
          }
        }
      }
    }
    EXPECT_NEAR(-angle, PageDeskewRadians(page), angle == 0 ? 0.005 : 0.006);
    if (angle == 0) {
      Pix *gray = pixConvertTo8(page, 0);
      EXPECT_NE(nullptr, gray);
      EXPECT_NEAR(0, PageDeskewRadians(gray), 0.005);
      pixDestroy(&gray);
    }
  }
  pixDestroy(&page);
}

} // namespace tesseract
