#ifndef TESSERACT_UNITTEST_LSTM_TEST_H_
#define TESSERACT_UNITTEST_LSTM_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "base/logging.h"
#include "base/stringprintf.h"
#include "file/base/file.h"
#include "file/base/helpers.h"
#include "file/base/path.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/str_cat.h"
#include "tesseract/ccutil/unicharset.h"
#include "tesseract/lstm/functions.h"
#include "tesseract/lstm/lstmtrainer.h"
#include "tesseract/training/lang_model_helpers.h"

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
  LSTMTrainerTest() {}
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir,
                          "tesseract/testdata/" + name);
  }

  void SetupTrainerEng(const string& network_spec, const string& model_name,
                       bool recode, bool adam) {
    SetupTrainer(network_spec, model_name, "eng.unicharset",
                 "lstm_training.arial.lstmf", recode, adam, 5e-4, false);
  }
  void SetupTrainer(const string& network_spec, const string& model_name,
                    const string& unicharset_file, const string& lstmf_file,
                    bool recode, bool adam, double learning_rate,
                    bool layer_specific) {
    constexpr char kLang[] = "eng";  // Exact value doesn't matter.
    string unicharset_name = TestDataNameToPath(unicharset_file);
    UNICHARSET unicharset;
    ASSERT_TRUE(unicharset.load_from_file(unicharset_name.c_str(), false));
    string script_dir = file::JoinPath(
        FLAGS_test_srcdir, "tesseract/training/langdata");
    GenericVector<STRING> words;
    EXPECT_EQ(0, CombineLangModel(unicharset, script_dir, "", FLAGS_test_tmpdir,
                                  kLang, !recode, words, words, words, false,
                                  nullptr, nullptr));
    string model_path = file::JoinPath(FLAGS_test_tmpdir, model_name);
    string checkpoint_path = model_path + "_checkpoint";
    trainer_.reset(new LSTMTrainer(nullptr, nullptr, nullptr, nullptr,
                                   model_path.c_str(), checkpoint_path.c_str(),
                                   0, 0));
    trainer_->InitCharSet(file::JoinPath(FLAGS_test_tmpdir, kLang,
                                         absl::StrCat(kLang, ".traineddata")));
    int net_mode = adam ? NF_ADAM : 0;
    // Adam needs a higher learning rate, due to not multiplying the effective
    // rate by 1/(1-momentum).
    if (adam) learning_rate *= 20.0;
    if (layer_specific) net_mode |= NF_LAYER_SPECIFIC_LR;
    EXPECT_TRUE(trainer_->InitNetwork(network_spec.c_str(), -1, net_mode, 0.1,
                                      learning_rate, 0.9, 0.999));
    GenericVector<STRING> filenames;
    filenames.push_back(STRING(TestDataNameToPath(lstmf_file).c_str()));
    EXPECT_TRUE(trainer_->LoadAllTrainingData(filenames, CS_SEQUENTIAL, false));
    LOG(INFO) << "Setup network:" << model_name;
  }
  // Trains for a given number of iterations and returns the char error rate.
  double TrainIterations(int max_iterations) {
    int iteration = trainer_->training_iteration();
    int iteration_limit = iteration + max_iterations;
    double best_error = 100.0;
    do {
      STRING log_str;
      int target_iteration = iteration + kBatchIterations;
      // Train a few.
      double mean_error = 0.0;
      while (iteration < target_iteration && iteration < iteration_limit) {
        trainer_->TrainOnLine(trainer_.get(), false);
        iteration = trainer_->training_iteration();
        mean_error += trainer_->LastSingleError(ET_CHAR_ERROR);
      }
      trainer_->MaintainCheckpoints(NULL, &log_str);
      iteration = trainer_->training_iteration();
      mean_error *= 100.0 / kBatchIterations;
      LOG(INFO) << log_str.string();
      LOG(INFO) << "Batch error = " << mean_error;
      if (mean_error < best_error) best_error = mean_error;
    } while (iteration < iteration_limit);
    LOG(INFO) << "Trainer error rate = " << best_error;
    return best_error;
  }
  // Tests for a given number of iterations and returns the char error rate.
  double TestIterations(int max_iterations) {
    CHECK_GT(max_iterations, 0);
    int iteration = trainer_->sample_iteration();
    double mean_error = 0.0;
    int error_count = 0;
    while (error_count < max_iterations) {
      const ImageData& trainingdata =
          *trainer_->mutable_training_data()->GetPageBySerial(iteration);
      NetworkIO fwd_outputs, targets;
      if (trainer_->PrepareForBackward(&trainingdata, &fwd_outputs, &targets) !=
          UNENCODABLE) {
        mean_error += trainer_->NewSingleError(ET_CHAR_ERROR);
        ++error_count;
      }
      trainer_->SetIteration(++iteration);
    }
    mean_error *= 100.0 / max_iterations;
    LOG(INFO) << "Tester error rate = " << mean_error;
    return mean_error;
  }
  // Tests that the current trainer_ can be converted to int mode and still gets
  // within 1% of the error rate. Returns the increase in error from float to
  // int.
  double TestIntMode(int test_iterations) {
    GenericVector<char> trainer_data;
    EXPECT_TRUE(trainer_->SaveTrainingDump(NO_BEST_TRAINER, trainer_.get(),
                                           &trainer_data));
    // Get the error on the next few iterations in float mode.
    double float_err = TestIterations(test_iterations);
    // Restore the dump, convert to int and test error on that.
    EXPECT_TRUE(trainer_->ReadTrainingDump(trainer_data, trainer_.get()));
    trainer_->ConvertToInt();
    double int_err = TestIterations(test_iterations);
    EXPECT_LT(int_err, float_err + 1.0);
    return int_err - float_err;
  }
  // Sets up a trainer with the given language and given recode+ctc condition.
  // It then verifies that the given str encodes and decodes back to the same
  // string.
  void TestEncodeDecode(const string& lang, const string& str, bool recode) {
    string unicharset_name = lang + ".unicharset";
    SetupTrainer("[1,1,0,32 Lbx100 O1c1]", "bidi-lstm", unicharset_name,
                 "arialuni.kor.lstmf", recode, true, 5e-4, true);
    GenericVector<int> labels;
    EXPECT_TRUE(trainer_->EncodeString(str.c_str(), &labels));
    STRING decoded = trainer_->DecodeLabels(labels);
    string decoded_str(&decoded[0], decoded.length());
    EXPECT_EQ(str, decoded_str);
  }
  // Calls TestEncodeDeode with both recode on and off.
  void TestEncodeDecodeBoth(const string& lang, const string& str) {
    TestEncodeDecode(lang, str, false);
    TestEncodeDecode(lang, str, true);
  }

  std::unique_ptr<LSTMTrainer> trainer_;
};

}  // namespace tesseract.

#endif  // THIRD_PARTY_TESSERACT_UNITTEST_LSTM_TEST_H_
