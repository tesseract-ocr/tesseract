///////////////////////////////////////////////////////////////////////
// File:        lstmeval.cpp
// Description: Evaluation program for LSTM-based networks.
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

#ifdef GOOGLE_TESSERACT
#include "base/commandlineflags.h"
#endif
#include "commontraining.h"
#include "genericvector.h"
#include "lstmtester.h"
#include "strngs.h"
#include "tprintf.h"

static STRING_PARAM_FLAG(model, "", "Name of model file (training or recognition)");
static STRING_PARAM_FLAG(traineddata, "",
                         "If model is a training checkpoint, then traineddata must "
                         "be the traineddata file that was given to the trainer");
static STRING_PARAM_FLAG(eval_listfile, "",
                         "File listing sample files in lstmf training format.");
static INT_PARAM_FLAG(max_image_MB, 2000, "Max memory to use for images.");
static INT_PARAM_FLAG(verbosity, 1,
                      "Amount of diagnosting information to output (0-2).");

int main(int argc, char **argv) {
  tesseract::CheckSharedLibraryVersion();
  ParseArguments(&argc, &argv);
  if (FLAGS_model.empty()) {
    tprintf("Must provide a --model!\n");
    return 1;
  }
  if (FLAGS_eval_listfile.empty()) {
    tprintf("Must provide a --eval_listfile!\n");
    return 1;
  }
  tesseract::TessdataManager mgr;
  if (!mgr.Init(FLAGS_model.c_str())) {
    if (FLAGS_traineddata.empty()) {
      tprintf("Must supply --traineddata to eval a training checkpoint!\n");
      return 1;
    }
    tprintf("%s is not a recognition model, trying training checkpoint...\n",
            FLAGS_model.c_str());
    if (!mgr.Init(FLAGS_traineddata.c_str())) {
      tprintf("Failed to load language model from %s!\n",
              FLAGS_traineddata.c_str());
      return 1;
    }
    GenericVector<char> model_data;
    if (!tesseract::LoadDataFromFile(FLAGS_model.c_str(), &model_data)) {
      tprintf("Failed to load model from: %s\n", FLAGS_model.c_str());
      return 1;
    }
    mgr.OverwriteEntry(tesseract::TESSDATA_LSTM, &model_data[0],
                       model_data.size());
  }
  tesseract::LSTMTester tester(static_cast<int64_t>(FLAGS_max_image_MB) *
                               1048576);
  if (!tester.LoadAllEvalData(FLAGS_eval_listfile.c_str())) {
    tprintf("Failed to load eval data from: %s\n", FLAGS_eval_listfile.c_str());
    return 1;
  }
  double errs = 0.0;
  STRING result =
      tester.RunEvalSync(0, &errs, mgr,
                         /*training_stage (irrelevant)*/ 0, FLAGS_verbosity);
  tprintf("%s\n", result.string());
  return 0;
} /* main */
