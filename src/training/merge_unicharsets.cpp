///////////////////////////////////////////////////////////////////////
// File:        merge_unicharsets.cpp
// Description: Simple tool to merge two or more unicharsets.
// Author:      Ray Smith
//
// (C) Copyright 2015, Google Inc.
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

#include "commontraining.h" // CheckSharedLibraryVersion
#include "unicharset.h"

int main(int argc, char **argv) {
  tesseract::CheckSharedLibraryVersion();

  if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
    printf("%s\n", tesseract::TessBaseAPI::Version());
    return EXIT_SUCCESS;
  } else if (argc < 4) {
    // Print usage
    printf(
        "Usage: %s -v | --version |\n"
        "       %s unicharset-in-1 ... unicharset-in-n unicharset-out\n",
        argv[0], argv[0]);
    return EXIT_FAILURE;
  }

  tesseract::UNICHARSET input_unicharset, result_unicharset;
  for (int arg = 1; arg < argc - 1; ++arg) {
    // Load the input unicharset
    if (input_unicharset.load_from_file(argv[arg])) {
      printf("Loaded unicharset of size %zu from file %s\n", input_unicharset.size(), argv[arg]);
      result_unicharset.AppendOtherUnicharset(input_unicharset);
    } else {
      printf("Failed to load unicharset from file %s!!\n", argv[arg]);
      return EXIT_FAILURE;
    }
  }

  // Save the combined unicharset.
  if (result_unicharset.save_to_file(argv[argc - 1])) {
    printf("Wrote unicharset file %s.\n", argv[argc - 1]);
  } else {
    printf("Cannot save unicharset file %s.\n", argv[argc - 1]);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
