///////////////////////////////////////////////////////////////////////
// File:        dawg2wordlist.cpp
// Description: Program to create a word list from a DAWG and unicharset.
// Author:      David Eger
// Created:     Thu 22 Dec 2011
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

#include "dawg.h"
#include "host.h"
#include "tesscallback.h"
#include "trie.h"
#include "unicharset.h"

const int kDictDebugLevel = 1;

tesseract::Dawg *LoadSquishedDawg(const UNICHARSET &unicharset,
                                  const char *filename) {
  const int kDictDebugLevel = 1;
  FILE *dawg_file = fopen(filename, "rb");
  if (dawg_file == NULL) {
    tprintf("Could not open %s for reading.\n", filename);
    return NULL;
  }
  tprintf("Loading word list from %s\n", filename);
  tesseract::Dawg *retval = new tesseract::SquishedDawg(
      dawg_file, tesseract::DAWG_TYPE_WORD, "eng", SYSTEM_DAWG_PERM,
      kDictDebugLevel);
  tprintf("Word list loaded.\n");
  fclose(dawg_file);
  return retval;
}

class WordOutputter {
 public:
  WordOutputter(FILE *file) : file_(file) {}
  void output_word(const char *word) { fprintf(file_, "%s\n", word); }
 private:
  FILE *file_;
};

// returns 0 if successful.
int WriteDawgAsWordlist(const UNICHARSET &unicharset,
                        const tesseract::Dawg *dawg,
                        const char *outfile_name) {
  FILE *out = fopen(outfile_name, "wb");
  if (out == NULL) {
    tprintf("Could not open %s for writing.\n", outfile_name);
    return 1;
  }
  WordOutputter outputter(out);
  TessCallback1<const char *> *print_word_cb =
      NewPermanentTessCallback(&outputter, &WordOutputter::output_word);
  dawg->iterate_words(unicharset, print_word_cb);
  delete print_word_cb;
  return fclose(out);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    tprintf("Print all the words in a given dawg.\n");
    tprintf("Usage: %s <unicharset> <dawgfile> <wordlistfile>\n",
            argv[0]);
    return 1;
  }
  const char *unicharset_file = argv[1];
  const char *dawg_file = argv[2];
  const char *wordlist_file = argv[3];
  UNICHARSET unicharset;
  if (!unicharset.load_from_file(unicharset_file)) {
    tprintf("Error loading unicharset from %s.\n", unicharset_file);
    return 1;
  }
  tesseract::Dawg *dict = LoadSquishedDawg(unicharset, dawg_file);
  if (dict == NULL) {
    tprintf("Error loading dictionary from %s.\n", dawg_file);
    return 1;
  }
  int retval = WriteDawgAsWordlist(unicharset, dict, wordlist_file);
  delete dict;
  return retval;
}
