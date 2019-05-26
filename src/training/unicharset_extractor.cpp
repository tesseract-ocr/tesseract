///////////////////////////////////////////////////////////////////////
// File:        unicharset_extractor.cpp
// Description: Unicode character/ligature set extractor.
// Author:      Thomas Kielbus
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

// Given a list of box files or text files on the command line, this program
// normalizes the text according to command-line options and generates
// a unicharset.

#include <cstdlib>
#include "boxread.h"
#include "commandlineflags.h"
#include "commontraining.h"     // CheckSharedLibraryVersion
#include "genericvector.h"
#include "lang_model_helpers.h"
#include "normstrngs.h"
#include "strngs.h"
#include "unicharset.h"
#include "unicharset_training_utils.h"

static STRING_PARAM_FLAG(output_unicharset, "unicharset", "Output file path");
static INT_PARAM_FLAG(norm_mode, 1,
                      "Normalization mode: 1=Combine graphemes, "
                      "2=Split graphemes, 3=Pure unicode");

namespace tesseract {

// Helper normalizes and segments the given strings according to norm_mode, and
// adds the segmented parts to unicharset.
static void AddStringsToUnicharset(const GenericVector<STRING>& strings,
                                   int norm_mode, UNICHARSET* unicharset) {
  for (int i = 0; i < strings.size(); ++i) {
    std::vector<std::string> normalized;
    if (NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                     static_cast<GraphemeNormMode>(norm_mode),
                                     /*report_errors*/ true,
                                     strings[i].string(), &normalized)) {
      for (const std::string& normed : normalized) {

       // normed is a UTF-8 encoded string
        if (normed.empty() || IsUTF8Whitespace(normed.c_str())) continue;
        unicharset->unichar_insert(normed.c_str());
      }
    } else {
      tprintf("Normalization failed for string '%s'\n", strings[i].c_str());
    }
  }
}

static int Main(int argc, char** argv) {
  UNICHARSET unicharset;
  // Load input files
  for (int arg = 1; arg < argc; ++arg) {
    STRING file_data = tesseract::ReadFile(argv[arg], /*reader*/ nullptr);
    if (file_data.length() == 0) continue;
    GenericVector<STRING> texts;
    if (ReadMemBoxes(-1, /*skip_blanks*/ true, &file_data[0],
                     /*continue_on_failure*/ false, /*boxes*/ nullptr,
                     &texts, /*box_texts*/ nullptr, /*pages*/ nullptr)) {
      tprintf("Extracting unicharset from box file %s\n", argv[arg]);
    } else {
      tprintf("Extracting unicharset from plain text file %s\n", argv[arg]);
      texts.truncate(0);
      file_data.split('\n', &texts);
    }
    AddStringsToUnicharset(texts, FLAGS_norm_mode, &unicharset);
  }
  SetupBasicProperties(/*report_errors*/ true, /*decompose*/ false,
                       &unicharset);
  // Write unicharset file.
  if (unicharset.save_to_file(FLAGS_output_unicharset.c_str())) {
    tprintf("Wrote unicharset file %s\n", FLAGS_output_unicharset.c_str());
  } else {
    tprintf("Cannot save unicharset file %s\n",
            FLAGS_output_unicharset.c_str());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

}  // namespace tesseract

int main(int argc, char** argv) {
  tesseract::CheckSharedLibraryVersion();
  if (argc > 1) {
    tesseract::ParseCommandLineFlags(argv[0], &argc, &argv, true);
  }
  if (argc < 2) {
    tprintf(
        "Usage: %s [--output_unicharset filename] [--norm_mode mode]"
        " box_or_text_file [...]\n",
        argv[0]);
    tprintf("Where mode means:\n");
    tprintf(" 1=combine graphemes (use for Latin and other simple scripts)\n");
    tprintf(" 2=split graphemes (use for Indic/Khmer/Myanmar)\n");
    tprintf(" 3=pure unicode (use for Arabic/Hebrew/Thai/Tibetan)\n");
    tprintf("Reads box or plain text files to extract the unicharset.\n");
    return EXIT_FAILURE;
  }
  return tesseract::Main(argc, argv);
}
