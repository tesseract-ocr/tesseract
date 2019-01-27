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

// Tests that a Squashed network learns correctly.
// Almost as fast as the 2d-lstm.
TEST_F(LSTMTrainerTest, TestSquashed) {
  // A 2-layer LSTM with a Squashed feature-extracting LSTM on the bottom, and
  // a small convolution/maxpool below that.
  // Match training conditions to those typically used with this spec:
  // recoding on, adam on.
  SetupTrainerEng("[1,32,0,1 Ct3,3,16 Mp3,3 Lfys48 Lbx96 O1c1]",
                  "SQU-2-layer-lstm", /*recode*/ true, /*adam*/ true);
  double lstm_2d_err = TrainIterations(kTrainerIterations * 3 / 2);
  EXPECT_LT(lstm_2d_err, 80);
  LOG(INFO) << "********** < 80 ************\n" ;
  TestIntMode(kTrainerIterations);
}

}  // namespace tesseract.
