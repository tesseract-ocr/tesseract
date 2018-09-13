
#include "tesseract/unittest/lstm_test.h"

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
  double lstm_2d_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(lstm_2d_err, 80);
  TestIntMode(kTrainerIterations);
}

}  // namespace tesseract.
