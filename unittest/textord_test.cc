// (C) Copyright 2026, Goutam Adwant
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include "bbgrid.h"
#include "blobbox.h"
#include "ccstruct.h"
#include "gap_map.h"
#include "ocrrow.h"
#include "stepblob.h"
#include "werd.h"

#include <memory>

#define private public
#include "textord.h"
#undef private

namespace tesseract {
namespace {

std::unique_ptr<ROW> MakeRow(int dot_count, int norm_count) {
  int32_t xstarts[] = {0, 1000};
  double coeffs[] = {0.0, 0.0, 0.0};
  auto row = std::make_unique<ROW>(1, xstarts, coeffs, 100.0f, 0.0f, 0.0f, 0, 0);

  C_BLOB_LIST blobs;
  C_BLOB_IT blob_it(&blobs);
  for (int x = 0; x < dot_count; ++x) {
    blob_it.add_after_then_move(C_BLOB::FakeBlob(TBOX(x * 30, 0, x * 30 + 20, 20)));
  }
  for (int x = 0; x < norm_count; ++x) {
    blob_it.add_after_then_move(C_BLOB::FakeBlob(TBOX(500 + x * 80, 0, 560 + x * 80, 60)));
  }

  WERD_IT word_it(row->word_list());
  word_it.add_after_then_move(new WERD(&blobs, 0, nullptr));
  return row;
}

} // namespace

TEST(TextordTest, RowNoiseUsesRowThreshold) {
  CCStruct ccstruct;
  Textord textord(&ccstruct);

  auto retained_row = MakeRow(5, 2);
  EXPECT_FALSE(textord.clean_noise_from_row(retained_row.get()));

  auto rejected_row = MakeRow(13, 2);
  EXPECT_TRUE(textord.clean_noise_from_row(rejected_row.get()));
}

} // namespace tesseract
