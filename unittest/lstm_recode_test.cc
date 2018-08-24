
#include "tesseract/unittest/lstm_test.h"

namespace tesseract {

// Tests that training with unicharset recoding learns faster than without,
// for Korean. This test is split in two, so it can be run sharded.
TEST_F(LSTMTrainerTest, RecodeTestKorBase) {
  // A basic single-layer, bi-di 1d LSTM on Korean.
  SetupTrainer("[1,1,0,32 Lbx96 O1c1]", "kor-full", "kor.unicharset",
               "arialuni.kor.lstmf", false, true, 5e-4, false);
  double kor_full_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(kor_full_err, 88);
  EXPECT_GT(kor_full_err, 85);
}

TEST_F(LSTMTrainerTest, RecodeTestKor) {
  // A basic single-layer, bi-di 1d LSTM on Korean.
  SetupTrainer("[1,1,0,32 Lbx96 O1c1]", "kor-recode", "kor.unicharset",
               "arialuni.kor.lstmf", true, true, 5e-4, false);
  double kor_recode_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(kor_recode_err, 60);
}

}  // namespace tesseract.
