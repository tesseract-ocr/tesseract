///////////////////////////////////////////////////////////////////////
// File:        lstmtester.cpp
// Description: Top-level line evaluation class for LSTM-based networks.
// Author:      Ray Smith
//
// (C) Copyright 2016, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include "lstmtester.h"
#include <iomanip>  // for std::setprecision
#include <thread>   // for std::thread
#include "fileio.h" // for LoadFileLinesToStrings

namespace tesseract {

LSTMTester::LSTMTester(int64_t max_memory) : test_data_(max_memory) {}

// Loads a set of lstmf files that were created using the lstm.train config to
// tesseract into memory ready for testing. Returns false if nothing was
// loaded. The arg is a filename of a file that lists the filenames.
bool LSTMTester::LoadAllEvalData(const char *filenames_file) {
  std::vector<std::string> filenames;
  if (!LoadFileLinesToStrings(filenames_file, &filenames)) {
    tprintf("Failed to load list of eval filenames from %s\n", filenames_file);
    return false;
  }
  return LoadAllEvalData(filenames);
}

// Loads a set of lstmf files that were created using the lstm.train config to
// tesseract into memory ready for testing. Returns false if nothing was
// loaded.
bool LSTMTester::LoadAllEvalData(const std::vector<std::string> &filenames) {
  test_data_.Clear();
  bool result = test_data_.LoadDocuments(filenames, CS_SEQUENTIAL, nullptr);
  total_pages_ = test_data_.TotalPages();
  return result;
}

// Runs an evaluation asynchronously on the stored data and returns a string
// describing the results of the previous test.
std::string LSTMTester::RunEvalAsync(int iteration, const double *training_errors,
                                     const TessdataManager &model_mgr, int training_stage) {
  std::string result;
  if (total_pages_ == 0) {
    result += "No test data at iteration " + std::to_string(iteration);
    return result;
  }
  if (!LockIfNotRunning()) {
    result += "Previous test incomplete, skipping test at iteration " + std::to_string(iteration);
    return result;
  }
  // Save the args.
  std::string prev_result = test_result_;
  test_result_ = "";
  if (training_errors != nullptr) {
    test_iteration_ = iteration;
    test_training_errors_ = training_errors;
    test_model_mgr_ = model_mgr;
    test_training_stage_ = training_stage;
    std::thread t(&LSTMTester::ThreadFunc, this);
    t.detach();
  } else {
    UnlockRunning();
  }
  return prev_result;
}

// Runs an evaluation synchronously on the stored data and returns a string
// describing the results.
std::string LSTMTester::RunEvalSync(int iteration, const double *training_errors,
                                    const TessdataManager &model_mgr, int training_stage,
                                    int verbosity) {
  LSTMTrainer trainer;
  trainer.InitCharSet(model_mgr);
  TFile fp;
  if (!model_mgr.GetComponent(TESSDATA_LSTM, &fp) || !trainer.DeSerialize(&model_mgr, &fp)) {
    return "Deserialize failed";
  }
  int eval_iteration = 0;
  double char_error = 0.0;
  double word_error = 0.0;
  int error_count = 0;
  while (error_count < total_pages_) {
    const ImageData *trainingdata = test_data_.GetPageBySerial(eval_iteration);
    trainer.SetIteration(++eval_iteration);
    NetworkIO fwd_outputs, targets;
    Trainability result = trainer.PrepareForBackward(trainingdata, &fwd_outputs, &targets);
    if (result != UNENCODABLE) {
      char_error += trainer.NewSingleError(tesseract::ET_CHAR_ERROR);
      word_error += trainer.NewSingleError(tesseract::ET_WORD_RECERR);
      ++error_count;
      if (verbosity > 1 || (verbosity > 0 && result != PERFECT)) {
        tprintf("Truth:%s\n", trainingdata->transcription().c_str());
        std::vector<int> ocr_labels;
        std::vector<int> xcoords;
        trainer.LabelsFromOutputs(fwd_outputs, &ocr_labels, &xcoords);
        std::string ocr_text = trainer.DecodeLabels(ocr_labels);
        tprintf("OCR  :%s\n", ocr_text.c_str());
        if (verbosity > 2 || (verbosity > 1 && result != PERFECT)) {
          tprintf("Line BCER=%f, BWER=%f\n\n",
                  trainer.NewSingleError(tesseract::ET_CHAR_ERROR),
                  trainer.NewSingleError(tesseract::ET_WORD_RECERR));
        }
      }
    }
  }
  char_error *= 100.0 / total_pages_;
  word_error *= 100.0 / total_pages_;
  std::stringstream result;
  result.imbue(std::locale::classic());
  result << std::fixed << std::setprecision(3);
  if (iteration != 0 || training_stage != 0) {
    result << "At iteration " << iteration
           << ", stage " << training_stage << ", ";
  }
  result << "BCER eval=" << char_error << ", BWER eval=" << word_error;
  return result.str();
}

// Helper thread function for RunEvalAsync.
// LockIfNotRunning must have returned true before calling ThreadFunc, and
// it will call UnlockRunning to release the lock after RunEvalSync completes.
void LSTMTester::ThreadFunc() {
  test_result_ =
      RunEvalSync(test_iteration_, test_training_errors_, test_model_mgr_, test_training_stage_,
                  /*verbosity*/ 0);
  UnlockRunning();
}

// Returns true if there is currently nothing running, and takes the lock
// if there is nothing running.
bool LSTMTester::LockIfNotRunning() {
  std::lock_guard<std::mutex> lock(running_mutex_);
  if (async_running_) {
    return false;
  }
  async_running_ = true;
  return true;
}

// Releases the running lock.
void LSTMTester::UnlockRunning() {
  std::lock_guard<std::mutex> lock(running_mutex_);
  async_running_ = false;
}

} // namespace tesseract
