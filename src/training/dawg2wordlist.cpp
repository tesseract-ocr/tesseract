///////////////////////////////////////////////////////////////////////
// File:        dawg2wordlist.cpp
// Description: Program to create a word list from a DAWG and unicharset.
// Author:      David Eger
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

#include "commontraining.h" // CheckSharedLibraryVersion
#include "dawg.h"
#include "trie.h"
#include "unicharset.h"

#include "serialis.h"

using namespace tesseract;

static std::unique_ptr<tesseract::Dawg> LoadSquishedDawg(const UNICHARSET &unicharset, const char *filename) {
  const int kDictDebugLevel = 1;
  tesseract::TFile dawg_file;
  if (!dawg_file.Open(filename, nullptr)) {
    tprintf("Could not open %s for reading.\n", filename);
    return nullptr;
  }
  tprintf("Loading word list from %s\n", filename);
  auto retval = std::make_unique<tesseract::SquishedDawg>(tesseract::DAWG_TYPE_WORD, "eng",
                                                          SYSTEM_DAWG_PERM, kDictDebugLevel);
  if (!retval->Load(&dawg_file)) {
    tprintf("Could not read %s\n", filename);
    return nullptr;
  }
  tprintf("Word list loaded.\n");
  return retval;
}

class WordOutputter {
public:
  WordOutputter(FILE *file) : file_(file) {}
  void output_word(const char *word) {
    fprintf(file_, "%s\n", word);
  }

private:
  FILE *file_;
};

// returns 0 if successful.
static int WriteDawgAsWordlist(const UNICHARSET &unicharset, const tesseract::Dawg *dawg,
                               const char *outfile_name) {
  FILE *out = fopen(outfile_name, "wb");
  if (out == nullptr) {
    tprintf("Could not open %s for writing.\n", outfile_name);
    return EXIT_FAILURE;
  }
  WordOutputter outputter(out);
  using namespace std::placeholders; // for _1
  dawg->iterate_words(unicharset, std::bind(&WordOutputter::output_word, &outputter, _1));
  return fclose(out);
}

int main(int argc, char *argv[]) {
  tesseract::CheckSharedLibraryVersion();

  if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
    printf("%s\n", tesseract::TessBaseAPI::Version());
    return 0;
  } else if (argc != 4) {
    tprintf("Print all the words in a given dawg.\n");
    tprintf(
        "Usage: %s -v | --version | %s <unicharset> <dawgfile> "
        "<wordlistfile>\n",
        argv[0], argv[0]);
    return EXIT_FAILURE;
  }
  const char *unicharset_file = argv[1];
  const char *dawg_file = argv[2];
  const char *wordlist_file = argv[3];
  UNICHARSET unicharset;
  if (!unicharset.load_from_file(unicharset_file)) {
    tprintf("Error loading unicharset from %s.\n", unicharset_file);
    return EXIT_FAILURE;
  }
  auto dict = LoadSquishedDawg(unicharset, dawg_file);
  if (dict == nullptr) {
    tprintf("Error loading dictionary from %s.\n", dawg_file);
    return EXIT_FAILURE;
  }
  int retval = WriteDawgAsWordlist(unicharset, dict.get(), wordlist_file);
  return retval;
}
