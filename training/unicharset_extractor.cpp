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

#include "unichar.h"
#include "unicharset.h"
#include "strngs.h"
#include "tessopt.h"

static const char* const kUnicharsetFileName = "unicharset";

int main(int argc, char** argv) {
  int option;
  const char* output_directory = ".";
  STRING unicharset_file_name;
  UNICHARSET unicharset;

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
        output_directory = optarg;
        ++optind;
        break;
    }
  }

  // Save file name
  unicharset_file_name = output_directory;
  unicharset_file_name += "/";
  unicharset_file_name += kUnicharsetFileName;

  // Load box files
  for (; optind < argc; ++optind) {
    printf("Extracting unicharset from %s\n", argv[optind]);

    FILE* box_file = fopen(argv[optind], "r");
    if (box_file == NULL) {
      printf("Cannot open box file %s\n", argv[optind]);
      return -1;
    }

    while (!feof(box_file)) {
      int x_min, y_min, x_max, y_max;
      char buffer[256];
      char c_string[256];

      fgets(buffer, sizeof (buffer), box_file);
      sscanf(buffer, "%s %d %d %d %d",
             c_string, &x_min, &y_min, &x_max, &y_max);

      unicharset.unichar_insert(c_string);
    }
    fclose(box_file);
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
