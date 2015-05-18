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
#include "dict.h"
#include "emalloc.h"
#include "freelist.h"
#include "helpers.h"
#include "serialis.h"
#include "trie.h"
#include "unicharset.h"

int main(int argc, char** argv) {
  if (!(argc == 4 || (argc == 5 && strcmp(argv[1], "-t") == 0) ||
      (argc == 6 && strcmp(argv[1], "-r") == 0))) {
    printf("Usage: %s [-t | -r [reverse policy] ] word_list_file"
           " dawg_file unicharset_file\n", argv[0]);
    return 1;
  }
  tesseract::Classify *classify = new tesseract::Classify();
  int argv_index = 0;
  if (argc == 5) ++argv_index;
  tesseract::Trie::RTLReversePolicy reverse_policy =
      tesseract::Trie::RRP_DO_NO_REVERSE;
  if (argc == 6) {
    ++argv_index;
    int tmp_int;
    sscanf(argv[++argv_index], "%d", &tmp_int);
    reverse_policy = static_cast<tesseract::Trie::RTLReversePolicy>(tmp_int);
    tprintf("Set reverse_policy to %s\n",
            tesseract::Trie::get_reverse_policy_name(reverse_policy));
  }
  if (argc == 7) argv_index += 3;
  const char* wordlist_filename = argv[++argv_index];
  const char* dawg_filename = argv[++argv_index];
  const char* unicharset_file = argv[++argv_index];
  tprintf("Loading unicharset from '%s'\n", unicharset_file);
  if (!classify->getDict().getUnicharset().load_from_file(unicharset_file)) {
    tprintf("Failed to load unicharset from '%s'\n", unicharset_file);
    delete classify;
    return 1;
  }
  const UNICHARSET &unicharset = classify->getDict().getUnicharset();
  if (argc == 4 || argc == 6) {
    tesseract::Trie trie(
        // the first 3 arguments are not used in this case
        tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM,
        unicharset.size(), classify->getDict().dawg_debug_level);
    tprintf("Reading word list from '%s'\n", wordlist_filename);
    if (!trie.read_and_add_word_list(wordlist_filename, unicharset,
                                     reverse_policy)) {
      tprintf("Failed to add word list from '%s'\n", wordlist_filename);
      exit(1);
    }
    tprintf("Reducing Trie to SquishedDawg\n");
    tesseract::SquishedDawg *dawg = trie.trie_to_dawg();
    if (dawg != NULL && dawg->NumEdges() > 0) {
      tprintf("Writing squished DAWG to '%s'\n", dawg_filename);
      dawg->write_squished_dawg(dawg_filename);
    } else {
      tprintf("Dawg is empty, skip producing the output file\n");
    }
    delete dawg;
  } else if (argc == 5) {
    tprintf("Loading dawg DAWG from '%s'\n", dawg_filename);
    tesseract::SquishedDawg words(
        dawg_filename,
        // these 3 arguments are not used in this case
        tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM,
        classify->getDict().dawg_debug_level);
    tprintf("Checking word list from '%s'\n", wordlist_filename);
    words.check_for_words(wordlist_filename, unicharset, true);
  } else {  // should never get here
    tprintf("Invalid command-line options\n");
    exit(1);
  }
  delete classify;
  return 0;
}
