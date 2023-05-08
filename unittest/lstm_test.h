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

#ifndef TESSERACT_UNITTEST_LSTM_TEST_H_
#define TESSERACT_UNITTEST_LSTM_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "include_gunit.h"

#include "helpers.h"
#include "tprintf.h"

#include "functions.h"
#include "lang_model_helpers.h"
#include "log.h" // for LOG
#include "lstmtrainer.h"
#include "unicharset.h"

namespace tesseract {

#if DEBUG_DETAIL == 0
// Number of iterations to run all the trainers.
const int kTrainerIterations = 600;
// Number of iterations between accuracy checks.
const int kBatchIterations = 100;
#else
// Number of iterations to run all the trainers.
const int kTrainerIterations = 2;
// Number of iterations between accuracy checks.
const int kBatchIterations = 1;
#endif

// The fixture for testing LSTMTrainer.
class LSTMTrainerTest : public testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
    file::MakeTmpdir();
  }

  LSTMTrainerTest() = default;
  std::string TestDataNameToPath(const std::string &name) {
    return file::JoinPath(TESTDATA_DIR, "" + name);
  }
  std::string TessDataNameToPath(const std::string &name) {
    return file::JoinPath(TESSDATA_DIR, "" + name);
  }
  std::string TestingNameToPath(const std::string &name) {
    return file::JoinPath(TESTING_DIR, "" + name);
  }

  void SetupTrainerEng(const std::string &network_spec, const std::string &model_name, bool recode,
                       bool adam) {
    SetupTrainer(network_spec, model_name, "eng/eng.unicharset", "eng.Arial.exp0.lstmf", recode,
                 adam, 5e-4, false, "eng");
  }
  void SetupTrainer(const std::string &network_spec, const std::string &model_name,
                    const std::string &unicharset_file, const std::string &lstmf_file, bool recode,
                    bool adam, float learning_rate, bool layer_specific, const std::string &kLang) {
    //    constexpr char kLang[] = "eng";  // Exact value doesn't matter.
    std::string unicharset_name = TestDataNameToPath(unicharset_file);
    UNICHARSET unicharset;
    ASSERT_TRUE(unicharset.load_from_file(unicharset_name.c_str(), false));
    std::string script_dir = file::JoinPath(LANGDATA_DIR, "");
    std::vector<std::string> words;
    EXPECT_EQ(0, CombineLangModel(unicharset, script_dir, "", FLAGS_test_tmpdir, kLang, !recode,
                                  words, words, words, false, nullptr, nullptr));
    std::string model_path = file::JoinPath(FLAGS_test_tmpdir, model_name);
    std::string checkpoint_path = model_path + "_checkpoint";
    trainer_ = std::make_unique<LSTMTrainer>(model_path.c_str(), checkpoint_path.c_str(), 0, 0);
    trainer_->InitCharSet(
        file::JoinPath(FLAGS_test_tmpdir, kLang, kLang) + ".traineddata");
    int net_mode = adam ? NF_ADAM : 0;
    // Adam needs a higher learning rate, due to not multiplying the effective
    // rate by 1/(1-momentum).
    if (adam) {
      learning_rate *= 20.0f;
    }
    if (layer_specific) {
      net_mode |= NF_LAYER_SPECIFIC_LR;
    }
    EXPECT_TRUE(
        trainer_->InitNetwork(network_spec.c_str(), -1, net_mode, 0.1, learning_rate, 0.9, 0.999));
    std::vector<std::string> filenames;
    filenames.emplace_back(TestDataNameToPath(lstmf_file).c_str());
    EXPECT_TRUE(trainer_->LoadAllTrainingData(filenames, CS_SEQUENTIAL, false));
    LOG(INFO) << "Setup network:" << model_name << "\n";
  }
  // Trains for a given number of iterations and returns the char error rate.
  double TrainIterations(int max_iterations) {
    int iteration = trainer_->training_iteration();
    int iteration_limit = iteration + max_iterations;
    double best_error = 100.0;
    do {
      std::stringstream log_str;
      int target_iteration = iteration + kBatchIterations;
      // Train a few.
      double mean_error = 0.0;
      while (iteration < target_iteration && iteration < iteration_limit) {
        trainer_->TrainOnLine(trainer_.get(), false);
        iteration = trainer_->training_iteration();
        mean_error += trainer_->LastSingleError(ET_CHAR_ERROR);
      }
      trainer_->MaintainCheckpoints(nullptr, log_str);
      iteration = trainer_->training_iteration();
      mean_error *= 100.0 / kBatchIterations;
      if (mean_error < best_error) {
        best_error = mean_error;
      }
    } while (iteration < iteration_limit);
    LOG(INFO) << "Trainer error rate = " << best_error << "\n";
    return best_error;
  }
  // Tests for a given number of iterations and returns the char error rate.
  double TestIterations(int max_iterations) {
    CHECK_GT(max_iterations, 0);
    int iteration = trainer_->sample_iteration();
    double mean_error = 0.0;
    int error_count = 0;
    while (error_count < max_iterations) {
      const ImageData &trainingdata =
          *trainer_->mutable_training_data()->GetPageBySerial(iteration);
      NetworkIO fwd_outputs, targets;
      if (trainer_->PrepareForBackward(&trainingdata, &fwd_outputs, &targets) != UNENCODABLE) {
        mean_error += trainer_->NewSingleError(ET_CHAR_ERROR);
        ++error_count;
      }
      trainer_->SetIteration(++iteration);
    }
    mean_error *= 100.0 / max_iterations;
    LOG(INFO) << "Tester error rate = " << mean_error << "\n";
    return mean_error;
  }
  // Tests that the current trainer_ can be converted to int mode and still gets
  // within 1% of the error rate. Returns the increase in error from float to
  // int.
  double TestIntMode(int test_iterations) {
    std::vector<char> trainer_data;
    EXPECT_TRUE(trainer_->SaveTrainingDump(NO_BEST_TRAINER, *trainer_, &trainer_data));
    // Get the error on the next few iterations in float mode.
    double float_err = TestIterations(test_iterations);
    // Restore the dump, convert to int and test error on that.
    EXPECT_TRUE(trainer_->ReadTrainingDump(trainer_data, *trainer_));
    trainer_->ConvertToInt();
    double int_err = TestIterations(test_iterations);
    EXPECT_LT(int_err, float_err + 1.0);
    return int_err - float_err;
  }
  // Sets up a trainer with the given language and given recode+ctc condition.
  // It then verifies that the given str encodes and decodes back to the same
  // string.
  void TestEncodeDecode(const std::string &lang, const std::string &str, bool recode) {
    std::string unicharset_name = lang + "/" + lang + ".unicharset";
    std::string lstmf_name = lang + ".Arial_Unicode_MS.exp0.lstmf";
    SetupTrainer("[1,1,0,32 Lbx100 O1c1]", "bidi-lstm", unicharset_name, lstmf_name, recode, true,
                 5e-4f, true, lang);
    std::vector<int> labels;
    EXPECT_TRUE(trainer_->EncodeString(str.c_str(), &labels));
    std::string decoded = trainer_->DecodeLabels(labels);
    std::string decoded_str(&decoded[0], decoded.length());
    EXPECT_EQ(str, decoded_str);
  }
  // Calls TestEncodeDeode with both recode on and off.
  void TestEncodeDecodeBoth(const std::string &lang, const std::string &str) {
    TestEncodeDecode(lang, str, false);
    TestEncodeDecode(lang, str, true);
  }

  std::unique_ptr<LSTMTrainer> trainer_;
};

} // namespace tesseract.

#endif // THIRD_PARTY_TESSERACT_UNITTEST_LSTM_TEST_H_
