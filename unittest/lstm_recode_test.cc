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

#include "lstm_test.h"

namespace tesseract {

// Tests that training with unicharset recoding learns faster than without,
// for Korean. This test is split in two, so it can be run sharded.

TEST_F(LSTMTrainerTest, RecodeTestKorBase) {
  // A basic single-layer, bi-di 1d LSTM on Korean.
  SetupTrainer("[1,1,0,32 Lbx96 O1c1]", "kor-full", "kor/kor.unicharset",
               "kor.Arial_Unicode_MS.exp0.lstmf", false, true, 5e-4, false, "kor");
  double kor_full_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(kor_full_err, 88);
  //  EXPECT_GT(kor_full_err, 85);
  LOG(INFO) << "********** Expected  < 88 ************\n";
}

TEST_F(LSTMTrainerTest, RecodeTestKor) {
  // A basic single-layer, bi-di 1d LSTM on Korean.
  SetupTrainer("[1,1,0,32 Lbx96 O1c1]", "kor-recode", "kor/kor.unicharset",
               "kor.Arial_Unicode_MS.exp0.lstmf", true, true, 5e-4, false, "kor");
  double kor_recode_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(kor_recode_err, 60);
  LOG(INFO) << "********** Expected  < 60 ************\n";
}

// Tests that the given string encodes and decodes back to the same
// with both recode on and off for Korean.

TEST_F(LSTMTrainerTest, EncodeDecodeBothTestKor) {
  TestEncodeDecodeBoth("kor", "한국어 위키백과에 오신 것을 환영합니다!");
}

} // namespace tesseract.
