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

// Generating the training data:
// If the format of the lstmf (ImageData) file changes, the training data will
// have to be regenerated as follows:
//
// Use --xsize 800 for text2image to be similar to original training data.
//
// src/training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng \
// --linedata_only   --noextract_font_properties --langdata_dir ../langdata_lstm \
// --tessdata_dir ../tessdata --output_dir ~/tesseract/test/testdata \
// --fontlist "Arial" --maxpages 10
//

#include "lstm_test.h"

namespace tesseract {

// Tests that some simple networks can learn Arial and meet accuracy targets.
TEST_F(LSTMTrainerTest, BasicTest) {
  // A Convolver sliding window classifier without LSTM.
  SetupTrainer(
      "[1,32,0,1 Ct5,5,16 Mp4,4 Ct1,1,16 Ct3,3,128 Mp4,1 Ct1,1,64 S2,1 "
      "Ct1,1,64O1c1]",
      "no-lstm", "eng/eng.unicharset", "eng.Arial.exp0.lstmf", false, false,
      2e-4, false, "eng");
  double non_lstm_err = TrainIterations(kTrainerIterations * 4);
  EXPECT_LT(non_lstm_err, 98);
  LOG(INFO) << "********** Expected  < 98 ************\n" ;

  // A basic single-layer, single direction LSTM.
  SetupTrainerEng("[1,1,0,32 Lfx100 O1c1]", "1D-lstm", false, false);
  double lstm_uni_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(lstm_uni_err, 86);
   LOG(INFO) << "********** Expected  < 86 ************\n" ;
  // Beats the convolver. (Although it does have a lot more weights, it still
  // iterates faster.)
  EXPECT_LT(lstm_uni_err, non_lstm_err);
}

// Color learns almost as fast as normalized grey/2D.
TEST_F(LSTMTrainerTest, ColorTest) {
  // A basic single-layer, single direction LSTM.
  SetupTrainerEng("[1,32,0,3 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2D-color-lstm", true, true);
  double lstm_uni_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(lstm_uni_err, 85);
//  EXPECT_GT(lstm_uni_err, 66);
  LOG(INFO) << "********** Expected  < 85 ************\n" ;
}

TEST_F(LSTMTrainerTest, BidiTest) {
  // A basic single-layer, bi-di 1d LSTM.
  SetupTrainerEng("[1,1,0,32 Lbx100 O1c1]", "bidi-lstm", false, false);
  double lstm_bi_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(lstm_bi_err, 75);
  LOG(INFO) << "********** Expected   < 75 ************\n" ;
  // Int mode training is dead, so convert the trained network to int and check
  // that its error rate is close to the float version.
  TestIntMode(kTrainerIterations);
}

// Tests that a 2d-2-layer network learns correctly.
// It takes a lot of iterations to get there.
TEST_F(LSTMTrainerTest, Test2D) {
  // A 2-layer LSTM with a 2-D feature-extracting LSTM on the bottom.
  SetupTrainerEng("[1,32,0,1 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2-D-2-layer-lstm", false, false);
  double lstm_2d_err = TrainIterations(kTrainerIterations * 3 / 2 );
  EXPECT_LT(lstm_2d_err, 98);
//  EXPECT_GT(lstm_2d_err, 90);
  LOG(INFO) << "********** Expected  < 98 ************\n" ;
  // Int mode training is dead, so convert the trained network to int and check
  // that its error rate is close to the float version.
  TestIntMode(kTrainerIterations);
}

// Tests that a 2d-2-layer network with Adam does *a lot* better than
// without it.
TEST_F(LSTMTrainerTest, TestAdam) {
  // A 2-layer LSTM with a 2-D feature-extracting LSTM on the bottom.
  SetupTrainerEng("[1,32,0,1 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2-D-2-layer-lstm", false, true);
  double lstm_2d_err = TrainIterations(kTrainerIterations);
  EXPECT_LT(lstm_2d_err, 70);
  LOG(INFO) << "********** Expected   < 70 ************\n" ;
  TestIntMode(kTrainerIterations);
}

// Trivial test of training speed on a fairly complex network.
TEST_F(LSTMTrainerTest, SpeedTest) {
  SetupTrainerEng(
      "[1,30,0,1 Ct5,5,16 Mp2,2 L2xy24 Ct1,1,48 Mp5,1 Ct1,1,32 S3,1 Lbx64 "
      "O1c1]",
      "2-D-2-layer-lstm", false, true);
  TrainIterations(kTrainerIterations);
   LOG(INFO) << "********** *** ************\n" ;
}

// Tests that two identical networks trained the same get the same results.
// Also tests that the same happens with a serialize/deserialize in the middle.
TEST_F(LSTMTrainerTest, DeterminismTest) {
  SetupTrainerEng("[1,32,0,1 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2-D-2-layer-lstm", false, false);
  double lstm_2d_err_a = TrainIterations(kTrainerIterations);
  double act_error_a = trainer_->ActivationError();
  double char_error_a = trainer_->CharError();
  GenericVector<char> trainer_a_data;
  EXPECT_TRUE(trainer_->SaveTrainingDump(NO_BEST_TRAINER, trainer_.get(),
                                         &trainer_a_data));
  SetupTrainerEng("[1,32,0,1 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2-D-2-layer-lstm", false, false);
  double lstm_2d_err_b = TrainIterations(kTrainerIterations);
  double act_error_b = trainer_->ActivationError();
  double char_error_b = trainer_->CharError();
  EXPECT_FLOAT_EQ(lstm_2d_err_a, lstm_2d_err_b);
  EXPECT_FLOAT_EQ(act_error_a, act_error_b);
  EXPECT_FLOAT_EQ(char_error_a, char_error_b);
  // Now train some more iterations.
  lstm_2d_err_b = TrainIterations(kTrainerIterations / 3);
  act_error_b = trainer_->ActivationError();
  char_error_b = trainer_->CharError();
  // Unpack into a new trainer and train that some more too.
  SetupTrainerEng("[1,32,0,1 S4,2 L2xy16 Ct1,1,16 S8,1 Lbx100 O1c1]",
                  "2-D-2-layer-lstm", false, false);
  EXPECT_TRUE(trainer_->ReadTrainingDump(trainer_a_data, trainer_.get()));
  lstm_2d_err_a = TrainIterations(kTrainerIterations / 3);
  act_error_a = trainer_->ActivationError();
  char_error_a = trainer_->CharError();
  EXPECT_FLOAT_EQ(lstm_2d_err_a, lstm_2d_err_b);
  EXPECT_FLOAT_EQ(act_error_a, act_error_b);
  EXPECT_FLOAT_EQ(char_error_a, char_error_b);
  LOG(INFO) << "********** *** ************\n" ;
}

// The baseline network against which to test the built-in softmax.
TEST_F(LSTMTrainerTest, SoftmaxBaselineTest) {
  // A basic single-layer, single direction LSTM.
  SetupTrainerEng("[1,1,0,32 Lfx96 O1c1]", "1D-lstm", false, true);
  double lstm_uni_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(lstm_uni_err, 60);
//  EXPECT_GT(lstm_uni_err, 48);
  LOG(INFO) << "********** Expected  < 60 ************\n" ;
  // Check that it works in int mode too.
  TestIntMode(kTrainerIterations);
  // If we run TestIntMode again, it tests that int_mode networks can
  // serialize and deserialize correctly.
  double delta = TestIntMode(kTrainerIterations);
  // The two tests (both of int mode this time) should be almost identical.
  LOG(INFO) << "Delta in Int mode error rates = " << delta << "\n";
  EXPECT_LT(delta, 0.01);
}

// Tests that the built-in softmax does better than the external one,
// which has an error rate slightly less than 55%, as tested by
// SoftmaxBaselineTest.
TEST_F(LSTMTrainerTest, SoftmaxTest) {
  // LSTM with a built-in softmax can beat the external softmax.
  SetupTrainerEng("[1,1,0,32 LS96]", "Lstm-+-softmax", false, true);
  double lstm_sm_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(lstm_sm_err, 49.0);
  LOG(INFO) << "********** Expected  < 49 ************\n" ;
  // Check that it works in int mode too.
  TestIntMode(kTrainerIterations);
}

// Tests that the built-in encoded softmax does better than the external one.
// It takes a lot of iterations to get there.
TEST_F(LSTMTrainerTest, EncodedSoftmaxTest) {
  // LSTM with a built-in encoded softmax can beat the external softmax.
  SetupTrainerEng("[1,1,0,32 LE96]", "Lstm-+-softmax", false, true);
  double lstm_sm_err = TrainIterations(kTrainerIterations * 2);
  EXPECT_LT(lstm_sm_err, 62.0);
  LOG(INFO) << "********** Expected   < 62 ************\n" ;
  // Check that it works in int mode too.
  TestIntMode(kTrainerIterations);
}

// Tests that layer access methods work correctly.
TEST_F(LSTMTrainerTest, TestLayerAccess) {
  // A 2-layer LSTM with a Squashed feature-extracting LSTM on the bottom.
  SetupTrainerEng("[1,32,0,1 Ct5,5,16 Mp2,2 Lfys32 Lbx128 O1c1]", "SQU-lstm",
                  false, false);
  // Number of layers.
  const int kNumLayers = 8;
  // Expected layer names.
  const char* kLayerIds[kNumLayers] = {":0",   ":1:0", ":1:1",   ":2",
                                       ":3:0", ":4:0", ":4:1:0", ":5"};
  const char* kLayerNames[kNumLayers] = {"Input",   "Convolve", "ConvNL",
                                         "Maxpool", "Lfys32",   "Lbx128LTR",
                                         "Lbx128",  "Output"};
  // Expected number of weights.
  const int kNumWeights[kNumLayers] = {0,
                                       0,
                                       16 * (25 + 1),
                                       0,
                                       32 * (4 * (32 + 16 + 1)),
                                       128 * (4 * (128 + 32 + 1)),
                                       128 * (4 * (128 + 32 + 1)),
                                       112 * (2 * 128 + 1)};

  GenericVector<STRING> layers = trainer_->EnumerateLayers();
  EXPECT_EQ(kNumLayers, layers.size());
  for (int i = 0; i < kNumLayers && i < layers.size(); ++i) {
    EXPECT_STREQ(kLayerIds[i], layers[i].string());
    EXPECT_STREQ(kLayerNames[i],
                 trainer_->GetLayer(layers[i])->name().string());
    EXPECT_EQ(kNumWeights[i], trainer_->GetLayer(layers[i])->num_weights());
  }
}

}  // namespace tesseract.
