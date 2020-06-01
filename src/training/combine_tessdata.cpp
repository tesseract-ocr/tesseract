///////////////////////////////////////////////////////////////////////
// File:        combine_tessdata.cpp
// Description: Creates a unified traineddata file from several
//              data files produced by the training process.
// Author:      Daria Antonova
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

#include <cerrno>
#include "commontraining.h"     // CheckSharedLibraryVersion
#include "lstmrecognizer.h"
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
  tesseract::CheckSharedLibraryVersion();

  int i;
  tesseract::TessdataManager tm;
  if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
    printf("%s\n", tesseract::TessBaseAPI::Version());
    return EXIT_SUCCESS;
  } else if (argc == 2) {
    printf("Combining tessdata files\n");
    STRING lang = argv[1];
    char* last = &argv[1][strlen(argv[1])-1];
    if (*last != '.')
      lang += '.';
    STRING output_file = lang;
    output_file += kTrainedDataSuffix;
    if (!tm.CombineDataFiles(lang.c_str(), output_file.c_str())) {
      printf("Error combining tessdata files into %s\n",
             output_file.c_str());
    } else {
      printf("Output %s created successfully.\n", output_file.c_str());
    }
  } else if (argc >= 4 && (strcmp(argv[1], "-e") == 0 ||
                           strcmp(argv[1], "-u") == 0)) {
    // Initialize TessdataManager with the data in the given traineddata file.
    if (!tm.Init(argv[2])) {
      tprintf("Failed to read %s\n", argv[2]);
      return EXIT_FAILURE;
    }
    printf("Extracting tessdata components from %s\n", argv[2]);
    if (strcmp(argv[1], "-e") == 0) {
      for (i = 3; i < argc; ++i) {
        errno = 0;
        if (tm.ExtractToFile(argv[i])) {
          printf("Wrote %s\n", argv[i]);
        } else if (errno == 0) {
          printf("Not extracting %s, since this component"
                 " is not present\n", argv[i]);
          return EXIT_FAILURE;
        } else {
          printf("Error, could not extract %s: %s\n",
                 argv[i], strerror(errno));
          return EXIT_FAILURE;
        }
      }
    } else {  // extract all the components
      for (i = 0; i < tesseract::TESSDATA_NUM_ENTRIES; ++i) {
        STRING filename = argv[3];
        char* last = &argv[3][strlen(argv[3])-1];
        if (*last != '.')
          filename += '.';
        filename += tesseract::kTessdataFileSuffixes[i];
        errno = 0;
        if (tm.ExtractToFile(filename.c_str())) {
          printf("Wrote %s\n", filename.c_str());
        } else if (errno != 0) {
          printf("Error, could not extract %s: %s\n",
                 filename.c_str(), strerror(errno));
          return EXIT_FAILURE;
        }
      }
    }
  } else if (argc >= 4 && strcmp(argv[1], "-o") == 0) {
    // Rename the current traineddata file to a temporary name.
    const char *new_traineddata_filename = argv[2];
    STRING traineddata_filename = new_traineddata_filename;
    traineddata_filename += ".__tmp__";
    if (rename(new_traineddata_filename, traineddata_filename.c_str()) != 0) {
      tprintf("Failed to create a temporary file %s\n",
              traineddata_filename.c_str());
      return EXIT_FAILURE;
    }

    // Initialize TessdataManager with the data in the given traineddata file.
    tm.Init(traineddata_filename.c_str());

    // Write the updated traineddata file.
    tm.OverwriteComponents(new_traineddata_filename, argv+3, argc-3);
  } else if (argc == 3 && strcmp(argv[1], "-c") == 0) {
    if (!tm.Init(argv[2])) {
      tprintf("Failed to read %s\n", argv[2]);
      return EXIT_FAILURE;
    }
    tesseract::TFile fp;
    if (!tm.GetComponent(tesseract::TESSDATA_LSTM, &fp)) {
      tprintf("No LSTM Component found in %s!\n", argv[2]);
      return EXIT_FAILURE;
    }
    tesseract::LSTMRecognizer recognizer;
    if (!recognizer.DeSerialize(&tm, &fp)) {
      tprintf("Failed to deserialize LSTM in %s!\n", argv[2]);
      return EXIT_FAILURE;
    }
    recognizer.ConvertToInt();
    GenericVector<char> lstm_data;
    fp.OpenWrite(&lstm_data);
    ASSERT_HOST(recognizer.Serialize(&tm, &fp));
    tm.OverwriteEntry(tesseract::TESSDATA_LSTM, &lstm_data[0],
                      lstm_data.size());
    if (!tm.SaveFile(argv[2], nullptr)) {
      tprintf("Failed to write modified traineddata:%s!\n", argv[2]);
      return EXIT_FAILURE;
    }
  } else if (argc == 3 && strcmp(argv[1], "-d") == 0) {
    // Initialize TessdataManager with the data in the given traineddata file.
    tm.Init(argv[2]);
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
    printf(
        "Usage for listing directory of components:\n"
        "  %s -d traineddata_file\n",
        argv[0]);
    printf(
        "Usage for compacting LSTM component to int:\n"
        "  %s -c traineddata_file\n",
        argv[0]);
    return 1;
  }
  tm.Directory();
  return EXIT_SUCCESS;
}
