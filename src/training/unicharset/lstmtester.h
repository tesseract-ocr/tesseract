///////////////////////////////////////////////////////////////////////
// File:        lstmtester.h
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

#ifndef TESSERACT_TRAINING_LSTMTESTER_H_
#define TESSERACT_TRAINING_LSTMTESTER_H_

#include "export.h"

#include "lstmtrainer.h"

#include <mutex>
#include <string>
#include <vector>

namespace tesseract {

class TESS_UNICHARSET_TRAINING_API LSTMTester {
public:
  LSTMTester(int64_t max_memory);

  // Loads a set of lstmf files that were created using the lstm.train config to
  // tesseract into memory ready for testing. Returns false if nothing was
  // loaded. The arg is a filename of a file that lists the filenames, with one
  // name per line. Conveniently, tesstrain.py generates such a file, along
  // with the files themselves.
  bool LoadAllEvalData(const char *filenames_file);
  // Loads a set of lstmf files that were created using the lstm.train config to
  // tesseract into memory ready for testing. Returns false if nothing was
  // loaded.
  bool LoadAllEvalData(const std::vector<std::string> &filenames);

  // Runs an evaluation asynchronously on the stored eval data and returns a
  // string describing the results of the previous test. Args match TestCallback
  // declared in lstmtrainer.h:
  // iteration: Current learning iteration number.
  // training_errors: If not null, is an array of size ET_COUNT, indexed by
  //   the ErrorTypes enum and indicates the current errors measured by the
  //   trainer, and this is a serious request to run an evaluation. If null,
  //   then the caller is just polling for the results of the previous eval.
  // model_data: is the model to evaluate, which should be a serialized
  //   LSTMTrainer.
  // training_stage: an arbitrary number on the progress of training.
  std::string RunEvalAsync(int iteration, const double *training_errors,
                           const TessdataManager &model_mgr, int training_stage);
  // Runs an evaluation synchronously on the stored eval data and returns a
  // string describing the results. Args as RunEvalAsync, except verbosity,
  // which outputs errors, if 1, or all results if 2.
  std::string RunEvalSync(int iteration, const double *training_errors, const TessdataManager &model_mgr,
                          int training_stage, int verbosity);

private:
  // Helper thread function for RunEvalAsync.
  // LockIfNotRunning must have returned true before calling ThreadFunc, and
  // it will call UnlockRunning to release the lock after RunEvalSync completes.
  void ThreadFunc();
  // Returns true if there is currently nothing running, and takes the lock
  // if there is nothing running.
  bool LockIfNotRunning();
  // Releases the running lock.
  void UnlockRunning();

  // The data to test with.
  DocumentCache test_data_;
  int total_pages_ = 0;
  // Flag that indicates an asynchronous test is currently running.
  // Protected by running_mutex_.
  bool async_running_ = false;
  std::mutex running_mutex_;
  // Stored copies of the args for use while running asynchronously.
  int test_iteration_ = 0;
  const double *test_training_errors_ = nullptr;
  TessdataManager test_model_mgr_;
  int test_training_stage_ = 0;
  std::string test_result_;
};

} // namespace tesseract

#endif // TESSERACT_TRAINING_LSTMTESTER_H_
