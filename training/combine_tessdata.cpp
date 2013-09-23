///////////////////////////////////////////////////////////////////////
// File:        combine_tessdata
// Description: Creates a unified traineddata file from several
//              data files produced by the training process.
// Author:      Daria Antonova
// Created:     Wed Jun 03 11:26:43 PST 2009
//
// (C) Copyright 2009, Google Inc.
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

#include "tessdatamanager.h"

// Main program to combine/extract/overwrite tessdata components
// in [lang].traineddata files.
//
// To combine all the individual tessdata components (unicharset, DAWGs,
// classifier templates, ambiguities, language configs) located at, say,
// /home/$USER/temp/eng.* run:
//
//   combine_tessdata /home/$USER/temp/eng.
//
// The result will be a combined tessdata file /home/$USER/temp/eng.traineddata
//
// Specify option -e if you would like to extract individual components
// from a combined traineddata file. For example, to extract language config
// file and the unicharset from tessdata/eng.traineddata run:
//
//   combine_tessdata -e tessdata/eng.traineddata
//   /home/$USER/temp/eng.config /home/$USER/temp/eng.unicharset
//
// The desired config file and unicharset will be written to
// /home/$USER/temp/eng.config /home/$USER/temp/eng.unicharset
//
// Specify option -o to overwrite individual components of the given
// [lang].traineddata file. For example, to overwrite language config
// and unichar ambiguities files in tessdata/eng.traineddata use:
//
//   combine_tessdata -o tessdata/eng.traineddata
//   /home/$USER/temp/eng.config /home/$USER/temp/eng.unicharambigs
//
// As a result, tessdata/eng.traineddata will contain the new language config
// and unichar ambigs, plus all the original DAWGs, classifier teamples, etc.
//
// Note: the file names of the files to extract to and to overwrite from should
// have the appropriate file suffixes (extensions) indicating their tessdata
// component type (.unicharset for the unicharset, .unicharambigs for unichar
// ambigs, etc). See k*FileSuffix variable in ccutil/tessdatamanager.h.
//
// Specify option -u to unpack all the components to the specified path:
//
// combine_tessdata -u tessdata/eng.traineddata /home/$USER/temp/eng.
//
// This will create  /home/$USER/temp/eng.* files with individual tessdata
// components from tessdata/eng.traineddata.
//
int main(int argc, char **argv) {
  int i;
  if (argc == 2) {
    printf("Combining tessdata files\n");
    STRING lang = argv[1];
    char* last = &argv[1][strlen(argv[1])-1];
    if (*last != '.')
      lang += '.';
    STRING output_file = lang;
    output_file += kTrainedDataSuffix;
    if (!tesseract::TessdataManager::CombineDataFiles(
        lang.string(), output_file.string())) {
      printf("Error combining tessdata files into %s\n",
             output_file.string());
    } else {
      printf("Output %s created sucessfully.\n", output_file.string());
    }
  } else if (argc >= 4 && (strcmp(argv[1], "-e") == 0 ||
                           strcmp(argv[1], "-u") == 0)) {
    // Initialize TessdataManager with the data in the given traineddata file.
    tesseract::TessdataManager tm;
    tm.Init(argv[2], 0);
    printf("Extracting tessdata components from %s\n", argv[2]);
    if (strcmp(argv[1], "-e") == 0) {
      for (i = 3; i < argc; ++i) {
        if (tm.ExtractToFile(argv[i])) {
          printf("Wrote %s\n", argv[i]);
        } else {
          printf("Not extracting %s, since this component"
                 " is not present\n", argv[i]);
        }
      }
    } else {  // extract all the components
      for (i = 0; i < tesseract::TESSDATA_NUM_ENTRIES; ++i) {
        STRING filename = argv[3];
        char* last = &argv[3][strlen(argv[3])-1];
        if (*last != '.')
          filename += '.';
        filename += tesseract::kTessdataFileSuffixes[i];
        if (tm.ExtractToFile(filename.string())) {
          printf("Wrote %s\n", filename.string());
        }
      }
    }
    tm.End();
  } else if (argc >= 4 && strcmp(argv[1], "-o") == 0) {
    // Rename the current traineddata file to a temporary name.
    const char *new_traineddata_filename = argv[2];
    STRING traineddata_filename = new_traineddata_filename;
    traineddata_filename += ".__tmp__";
    if (rename(new_traineddata_filename, traineddata_filename.string()) != 0) {
      tprintf("Failed to create a temporary file %s\n",
              traineddata_filename.string());
      exit(1);
    }

    // Initialize TessdataManager with the data in the given traineddata file.
    tesseract::TessdataManager tm;
    tm.Init(traineddata_filename.string(), 0);

    // Write the updated traineddata file.
    tm.OverwriteComponents(new_traineddata_filename, argv+3, argc-3);
    tm.End();
  } else {
    printf("Usage for combining tessdata components:\n"
           "  %s language_data_path_prefix\n"
           "  (e.g. %s tessdata/eng.)\n\n", argv[0], argv[0]);
    printf("Usage for extracting tessdata components:\n"
           "  %s -e traineddata_file [output_component_file...]\n"
           "  (e.g. %s -e eng.traineddata eng.unicharset)\n\n",
           argv[0], argv[0]);
    printf("Usage for overwriting tessdata components:\n"
           "  %s -o traineddata_file [input_component_file...]\n"
           "  (e.g. %s -o eng.traineddata eng.unicharset)\n\n",
           argv[0], argv[0]);
    printf("Usage for unpacking all tessdata components:\n"
           "  %s -u traineddata_file output_path_prefix\n"
           "  (e.g. %s -u eng.traineddata tmp/eng.)\n", argv[0], argv[0]);
    return 1;
  }
}
