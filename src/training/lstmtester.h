///////////////////////////////////////////////////////////////////////
// File:        lstmtester.h
// Description: Top-level line evaluation class for LSTM-based networks.
// Author:      Ray Smith
// Created:     Wed Nov 23 11:05:06 PST 2016
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

#include "genericvector.h"
#include "lstmtrainer.h"
#include "strngs.h"
#include "svutil.h"

namespace tesseract {

class LSTMTester {
 public:
  LSTMTester(int64_t max_memory);

  // Loads a set of lstmf files that were created using the lstm.train config to
  // tesseract into memory ready for testing. Returns false if nothing was
  // loaded. The arg is a filename of a file that lists the filenames, with one
  // name per line. Conveniently, tesstrain.sh generates such a file, along
  // with the files themselves.
  bool LoadAllEvalData(const STRING& filenames_file);
  // Loads a set of lstmf files that were created using the lstm.train config to
  // tesseract into memory ready for testing. Returns false if nothing was
  // loaded.
  bool LoadAllEvalData(const GenericVector<STRING>& filenames);

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
  STRING RunEvalAsync(int iteration, const double* training_errors,
                      const TessdataManager& model_mgr, int training_stage);
  // Runs an evaluation synchronously on the stored eval data and returns a
  // string describing the results. Args as RunEvalAsync, except verbosity,
  // which outputs errors, if 1, or all results if 2.
  STRING RunEvalSync(int iteration, const double* training_errors,
                     const TessdataManager& model_mgr, int training_stage,
                     int verbosity);

 private:
  // Static helper thread function for RunEvalAsync, with a specific signature
  // required by SVSync::StartThread. Actually a member function pretending to
  // be static, its arg is a this pointer that it will cast back to LSTMTester*
  // to call RunEvalSync using the stored args that RunEvalAsync saves in *this.
  // LockIfNotRunning must have returned true before calling ThreadFunc, and
  // it will call UnlockRunning to release the lock after RunEvalSync completes.
  static void* ThreadFunc(void* lstmtester_void);
  // Returns true if there is currently nothing running, and takes the lock
  // if there is nothing running.
  bool LockIfNotRunning();
  // Releases the running lock.
  void UnlockRunning();

  // The data to test with.
  DocumentCache test_data_;
  int total_pages_;
  // Flag that indicates an asynchronous test is currently running.
  // Protected by running_mutex_.
  bool async_running_;
  SVMutex running_mutex_;
  // Stored copies of the args for use while running asynchronously.
  int test_iteration_;
  const double* test_training_errors_;
  TessdataManager test_model_mgr_;
  int test_training_stage_;
  STRING test_result_;
};

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_LSTMTESTER_H_
