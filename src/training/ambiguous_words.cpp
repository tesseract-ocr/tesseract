///////////////////////////////////////////////////////////////////////
// File:        ambiguous_words.cpp
// Description: A program that takes a text file with a list of words as
//              input (one per line) and outputs a file with the words
//              that were found in the dictionary followed by the words
//              that are ambiguous to them.
// Author:      Rika Antonova
// Created:     Fri Oct 21 11:26:43 PDT 2011
//
// (C) Copyright 2011, Google Inc.
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
//

#include "baseapi.h"
#include "commontraining.h"     // CheckSharedLibraryVersion
#include "helpers.h"
#include "strngs.h"
#include "dict.h"
#include "tesseractclass.h"

int main(int argc, char** argv) {
  tesseract::CheckSharedLibraryVersion();

  // Parse input arguments.
  if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
    printf("%s\n", tesseract::TessBaseAPI::Version());
    return 0;
  } else if (argc != 4 && (argc != 6 || strcmp(argv[1], "-l") != 0)) {
    printf("Usage: %s -v | --version | %s [-l lang] tessdata_dir wordlist_file"
           " output_ambiguous_wordlist_file\n", argv[0], argv[0]);
    return 1;
  }
  int argv_offset = 0;
  STRING lang;
  if (argc == 6) {
    lang = argv[2];
    argv_offset = 2;
  } else {
    lang = "eng";
  }
  const char *tessdata_dir = argv[++argv_offset];
  const char *input_file_str = argv[++argv_offset];
  const char *output_file_str = argv[++argv_offset];

  // Initialize Tesseract.
  tesseract::TessBaseAPI api;
  GenericVector<STRING> vars_vec;
  GenericVector<STRING> vars_values;
  vars_vec.push_back("output_ambig_words_file");
  vars_values.push_back(output_file_str);
  api.Init(tessdata_dir, lang.string(), tesseract::OEM_TESSERACT_ONLY, nullptr,
           0, &vars_vec, &vars_values, false);
  tesseract::Dict &dict = api.tesseract()->getDict();
  FILE *input_file = fopen(input_file_str, "rb");
  if (input_file == nullptr) {
    tprintf("Failed to open input wordlist file %s\n", input_file_str);
    exit(1);
  }
  char str[CHARS_PER_LINE];

  // Read word list and call Dict::NoDangerousAmbig() for each word
  // to record ambiguities in the output file.
  while (fgets(str, CHARS_PER_LINE, input_file) != nullptr) {
    chomp_string(str);  // remove newline
    WERD_CHOICE word(str, dict.getUnicharset());
    dict.NoDangerousAmbig(&word, nullptr, false, nullptr);
  }
  // Clean up.
  fclose(input_file);
}
