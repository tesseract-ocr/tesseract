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

static const int kMaxNumEdges =  10000000;

int main(int argc, char** argv) {
  int min_word_length;
  int max_word_length;
  if (!(argc == 4 || (argc == 5 && strcmp(argv[1], "-t") == 0) ||
        (argc == 7 && strcmp(argv[1], "-l") == 0 &&
         sscanf(argv[2], "%d", &min_word_length) == 1 &&
         sscanf(argv[3], "%d", &max_word_length) == 1))) {
    printf("Usage: %s [-t | -l min_len max_len] word_list_file"
           " dawg_file unicharset_file", argv[0]);
    return 1;
  }
  tesseract::Classify *classify = new tesseract::Classify();
  int argv_index = 0;
  if (argc == 5) ++argv_index;
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
  if (argc == 4) {
    tesseract::Trie trie(
        // the first 3 arguments are not used in this case
        tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM,
        kMaxNumEdges, unicharset.size(),
        classify->getDict().dawg_debug_level);
    tprintf("Reading word list from '%s'\n", wordlist_filename);
    if (!trie.read_word_list(wordlist_filename, unicharset)) {
      tprintf("Failed to read word list from '%s'\n", wordlist_filename);
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
  } else if (argc == 7) {
    // Place words of different lengths in separate Dawgs.
    char str[CHARS_PER_LINE];
    FILE *word_file = fopen(wordlist_filename, "r");
    if (word_file == NULL) {
      tprintf("Failed to open wordlist file %s\n", wordlist_filename);
      exit(1);
    }
    FILE *dawg_file = fopen(dawg_filename, "wb");
    if (dawg_file == NULL) {
      tprintf("Failed to open dawg output file %s\n", dawg_filename);
      exit(1);
    }
    tprintf("Reading word list from '%s'\n", wordlist_filename);
    GenericVector<tesseract::Trie *> trie_vec;
    int i;
    for (i = min_word_length; i <= max_word_length; ++i) {
      trie_vec.push_back(new tesseract::Trie(
          // the first 3 arguments are not used in this case
          tesseract::DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM,
          kMaxNumEdges, unicharset.size(),
          classify->getDict().dawg_debug_level));
    }
    while (fgets(str, CHARS_PER_LINE, word_file) != NULL) {
      chomp_string(str);  // remove newline
      WERD_CHOICE word(str, unicharset);
      if (word.length() >= min_word_length &&
          word.length() <= max_word_length &&
          !word.contains_unichar_id(INVALID_UNICHAR_ID)) {
        tesseract::Trie *curr_trie = trie_vec[word.length()-min_word_length];
        if (!curr_trie->word_in_dawg(word)) {
          curr_trie->add_word_to_dawg(word);
          if (classify->getDict().dawg_debug_level > 1) {
            tprintf("Added word %s of length %d\n", str, word.length());
          }
          if (!curr_trie->word_in_dawg(word)) {
            tprintf("Error: word '%s' not in DAWG after adding it\n", str);
            exit(1);
          }
        }
      }
    }
    fclose(word_file);
    tprintf("Writing fixed length dawgs to '%s'\n", dawg_filename);
    GenericVector<tesseract::SquishedDawg *> dawg_vec;
    for (i = 0; i <= max_word_length; ++i) {
      dawg_vec.push_back(i < min_word_length ? NULL :
                         trie_vec[i-min_word_length]->trie_to_dawg());
    }
    tesseract::Dict::WriteFixedLengthDawgs(
        dawg_vec, max_word_length - min_word_length + 1,
        classify->getDict().dawg_debug_level, dawg_file);
    fclose(dawg_file);
    dawg_vec.delete_data_pointers();
    trie_vec.delete_data_pointers();
  } else {  // should never get here
    tprintf("Invalid command-line options\n");
    exit(1);
  }
  delete classify;
  return 0;
}
