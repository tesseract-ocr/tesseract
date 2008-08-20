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

#include "dawg.h"
#include "makedawg.h"
#include "lookdawg.h"
#include "reduce.h"
#include "freelist.h"
#include "emalloc.h"

int main(int argc, char** argv) {

  if (argc == 3) {
    const char* wordlist_filename = argv[1];
    const char* dawg_filename = argv[2];

    EDGE_ARRAY dawg;
    inT32 max_num_edges =  10000000;
    inT32 reserved_edges =  1000000;

    dawg = (EDGE_ARRAY) Emalloc(sizeof (EDGE_RECORD) * max_num_edges);
    if (dawg == NULL) {
      printf("error: Could not allocate enough memory for DAWG  ");
      printf("(%d ,%d bytes needed)\n",
              static_cast<int>(sizeof (EDGE_RECORD) * max_num_edges / 1000),
              static_cast<int>(sizeof (EDGE_RECORD) * max_num_edges % 1000));
      exit(1);
    }

    printf("Building DAWG from word list in file, '%s'\n", wordlist_filename);
    read_word_list(wordlist_filename, dawg, max_num_edges, reserved_edges);

    trie_to_dawg(dawg, max_num_edges, reserved_edges);

    printf("Writing squished DAWG file, '%s'\n", dawg_filename);
    write_squished_dawg(dawg_filename, dawg, max_num_edges, reserved_edges);

    return 0;
  } else if (argc == 4 && strcmp(argv[1], "-t") == 0) {
    EDGE_ARRAY words = read_squished_dawg(argv[3]);
    check_for_words(words, argv[2]);
    memfree(words);
    return 0;
  }

  printf("Usage: %s [-t] word_list_file dawg_file\n", argv[0]);
  return 1;
}
