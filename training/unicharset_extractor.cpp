///////////////////////////////////////////////////////////////////////
// File:        unicharset_extractor.cpp
// Description: Unicode character/ligature set extractor.
// Author:      Thomas Kielbus
// Created:     Wed Jun 28 17:05:01 PDT 2006
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

// Given a list of box files on the command line, this program generates a file
// containing a unicharset, a list of all the characters used by Tesseract
//
// The file contains the size of the set on the first line, and then one
// unichar per line.

#include <stdio.h>
/*
** Include automatically generated configuration file if running autoconf
*/
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#if defined(HAVE_WCHAR_T) || defined(__MSW32__) || defined(GOOGLE3)
#include <wchar.h>
#include <wctype.h>
#define USING_WCTYPE
#endif
#include <locale.h>

#include "unichar.h"
#include "unicharset.h"
#include "strngs.h"
#include "boxread.h"
#include "tessopt.h"

static const char* const kUnicharsetFileName = "unicharset";

// Set character properties using wctype if we have it.
// Contributed by piggy@gmail.com.
// Modified by Ray to use UNICHAR for unicode conversion
// and to check for wctype using autoconf/presence of windows.
void set_properties(UNICHARSET *unicharset, const char* const c_string) {
#ifdef USING_WCTYPE
  UNICHAR_ID id;
  int wc;

  // Convert the string to a unichar id.
  id = unicharset->unichar_to_id(c_string);

  int step = UNICHAR::utf8_step(c_string);
  if (step == 0)
    return; // Invalid utf-8.

  // Get the next Unicode cond point in the string.
  UNICHAR ch(c_string, step);
  wc = ch.first_uni();

  /* Copy the properties. */
  if (iswalpha(wc)) {
    unicharset->set_isalpha(id, 1);
    if (iswlower(wc))
      unicharset->set_islower(id, 1);
    if (iswupper(wc))
      unicharset->set_isupper(id, 1);
  }
  if (iswdigit(wc))
    unicharset->set_isdigit(id, 1);
#endif
}

int main(int argc, char** argv) {
  int option;
  const char* output_directory = ".";
  STRING unicharset_file_name;
  UNICHARSET unicharset;

  setlocale(LC_ALL, "");
  // Space character needed to represent NIL classification
  unicharset.unichar_insert(" ");

  // Print usage
  if (argc <= 1) {
    printf("Usage: %s [-D DIRECTORY] FILE...\n", argv[0]);
    exit(1);

  }

  // Parse arguments
  while ((option = tessopt(argc, argv, "D" )) != EOF) {
    switch (option) {
      case 'D':
        output_directory = tessoptarg;
        ++tessoptind;
        break;
    }
  }

  // Save file name
  unicharset_file_name = output_directory;
  unicharset_file_name += "/";
  unicharset_file_name += kUnicharsetFileName;

  // Load box files
  for (; tessoptind < argc; ++tessoptind) {
    printf("Extracting unicharset from %s\n", argv[tessoptind]);

    FILE* box_file = fopen(argv[tessoptind], "r");
    if (box_file == NULL) {
      printf("Cannot open box file %s\n", argv[tessoptind]);
      return -1;
    }

    int x_min, y_min, x_max, y_max;
    char c_string[kBoxReadBufSize];
    while (read_next_box(box_file, c_string, &x_min, &y_min, &x_max, &y_max)) {
      unicharset.unichar_insert(c_string);
      set_properties(&unicharset, c_string);
    }
  }

  // Write unicharset file
  if (unicharset.save_to_file(unicharset_file_name.string())) {
    printf("Wrote unicharset file %s.\n", unicharset_file_name.string());
  }
  else {
    printf("Cannot save unicharset file %s.\n", unicharset_file_name.string());
    return -1;
  }
  return 0;
}
