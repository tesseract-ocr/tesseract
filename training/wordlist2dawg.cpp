///////////////////////////////////////////////////////////////////////
// File:        wordlist2dawg.cpp
// Description: Program to generate a DAWG from a word list file
// Author:      Thomas Kielbus
// Created:     Thu May 10 18:11:42 PDT 2007
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

// Given a file that contains a list of words (one word per line) this program
// generates the corresponding squished DAWG file.

#include <stdio.h>

#include "classify.h"
#include "dawg.h"
#include "emalloc.h"
#include "freelist.h"
#include "trie.h"
#include "unicharset.h"

static const int kMaxNumEdges =  10000000;

int main(int argc, char** argv) {
  if (!(argc == 4 || (argc == 5 && strcmp(argv[1], "-t") == 0))) {
    printf("Usage: %s [-t] word_list_file dawg_file unicharset_file\n", argv[0]);
    return 1;
  }
  tesseract::Classify classify;
  int argv_index = 0;
  if (argc == 5) ++argv_index;
  const char* wordlist_filename = argv[++argv_index];
  const char* dawg_filename = argv[++argv_index];
  const char* unicharset_file = argv[++argv_index];
  if (!classify.getDict().getUnicharset().load_from_file(unicharset_file)) {
    tprintf("Failed to load unicharset from '%s'\n", unicharset_file);
    return 1;
  }
  const UNICHARSET &unicharset = classify.getDict().getUnicharset();
  if (argc == 4) {
    tesseract::Trie trie(
        // the first 3 arguments are not used in this case
        tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM,
        kMaxNumEdges, unicharset.size());
    printf("Reading word list from '%s'\n", wordlist_filename);
    if (!trie.read_word_list(wordlist_filename, unicharset)) {
      printf("Failed to read word list from '%s'\n", wordlist_filename);
      exit(1);
    }
    printf("Reducing Trie to SquishedDawg\n");
    tesseract::SquishedDawg *dawg = trie.trie_to_dawg();
    printf("Writing squished DAWG to '%s'\n", dawg_filename);
    dawg->write_squished_dawg(dawg_filename);
    delete dawg;
  } else {
    printf("Loading dawg DAWG from '%s'\n", dawg_filename);
    tesseract::SquishedDawg words(
        dawg_filename,
        // these 3 arguments are not used in this case
        tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM);
    printf("Checking word list from '%s'\n", wordlist_filename);
    words.check_for_words(wordlist_filename, unicharset, true);
  }
  return 0;
}
