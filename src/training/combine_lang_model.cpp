// Copyright 2017 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Purpose: Program to generate a traineddata file that can be used to train an
//          LSTM-based neural network model from a unicharset and an optional
//          set of wordlists. Eliminates the need to run
//          set_unicharset_properties, wordlist2dawg, some non-existent binary
//          to generate the recoder, and finally combine_tessdata.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "commandlineflags.h"
#include "commontraining.h"     // CheckSharedLibraryVersion
#include "lang_model_helpers.h"
#include "tprintf.h"
#include "unicharset_training_utils.h"

static STRING_PARAM_FLAG(input_unicharset, "",
                         "Filename with unicharset to complete and use in encoding");
static STRING_PARAM_FLAG(script_dir, "",
                         "Directory name for input script unicharsets");
static STRING_PARAM_FLAG(words, "",
                         "File listing words to use for the system dictionary");
static STRING_PARAM_FLAG(puncs, "", "File listing punctuation patterns");
static STRING_PARAM_FLAG(numbers, "", "File listing number patterns");
static STRING_PARAM_FLAG(output_dir, "", "Root directory for output files");
static STRING_PARAM_FLAG(version_str, "", "Version string to add to traineddata file");
static STRING_PARAM_FLAG(lang, "", "Name of language being processed");
static BOOL_PARAM_FLAG(lang_is_rtl, false,
                       "True if lang being processed is written right-to-left");
static BOOL_PARAM_FLAG(pass_through_recoder, false,
                       "If true, the recoder is a simple pass-through of the "
                       "unicharset. Otherwise, potentially a compression of it");

int main(int argc, char** argv) {
  // Sets properties on the input unicharset file, and writes:
  //   rootdir/lang/lang.charset_size=ddd.txt
  //   rootdir/lang/lang.traineddata
  //   rootdir/lang/lang.unicharset
  // If the 3 word lists are provided, the dawgs are also added
  // to the traineddata file.
  // The output unicharset and charset_size files are just for
  // human readability.
  tesseract::CheckSharedLibraryVersion();
  tesseract::ParseCommandLineFlags(argv[0], &argc, &argv, true);

  GenericVector<STRING> words, puncs, numbers;
  // If these reads fail, we get a warning message and an empty list of words.
  tesseract::ReadFile(FLAGS_words.c_str(), nullptr).split('\n', &words);
  tesseract::ReadFile(FLAGS_puncs.c_str(), nullptr).split('\n', &puncs);
  tesseract::ReadFile(FLAGS_numbers.c_str(), nullptr).split('\n', &numbers);
  // Load the input unicharset
  UNICHARSET unicharset;
  if (!unicharset.load_from_file(FLAGS_input_unicharset.c_str(), false)) {
    tprintf("Failed to load unicharset from %s\n",
            FLAGS_input_unicharset.c_str());
    return 1;
  }
  tprintf("Loaded unicharset of size %d from file %s\n", unicharset.size(),
          FLAGS_input_unicharset.c_str());

  // Set unichar properties
  tprintf("Setting unichar properties\n");
  tesseract::SetupBasicProperties(/*report_errors*/ true,
                                  /*decompose (NFD)*/ false, &unicharset);
  tprintf("Setting script properties\n");
  tesseract::SetScriptProperties(FLAGS_script_dir.c_str(), &unicharset);
  // Combine everything into a traineddata file.
  return tesseract::CombineLangModel(
      unicharset, FLAGS_script_dir.c_str(), FLAGS_version_str.c_str(),
      FLAGS_output_dir.c_str(), FLAGS_lang.c_str(), FLAGS_pass_through_recoder,
      words, puncs, numbers, FLAGS_lang_is_rtl, /*reader*/ nullptr,
      /*writer*/ nullptr);
}
