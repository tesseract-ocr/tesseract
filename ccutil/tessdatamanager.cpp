///////////////////////////////////////////////////////////////////////
// File:        tessdatamanager.cpp
// Description: Functions to handle loading/combining tesseract data files.
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

#include <stdio.h>

#include "serialis.h"
#include "strngs.h"
#include "tprintf.h"
#include "varable.h"

BOOL_VAR(global_load_system_dawg, true, "Load system word dawg.");
BOOL_VAR(global_load_freq_dawg, true, "Load frequent word dawg.");
BOOL_VAR(global_load_punc_dawg, true, "Load dawg with punctuation patterns.");
BOOL_VAR(global_load_number_dawg, true, "Load dawg with number patterns.");

INT_VAR(global_tessdata_manager_debug_level, 0,
        "Debug level for TessdataManager functions.");

namespace tesseract {

void TessdataManager::Init(const char *data_file_name) {
  int i;
  data_file_ = fopen(data_file_name, "rb");
  if (data_file_ == NULL) {
    tprintf("Error openning data file %s\n", data_file_name);
    exit(1);
  }
  fread(&actual_tessdata_num_entries_, sizeof(inT32), 1, data_file_);
  bool swap = (actual_tessdata_num_entries_ > kMaxNumTessdataEntries);
  if (swap) {
    actual_tessdata_num_entries_ = reverse32(actual_tessdata_num_entries_);
  }
  ASSERT_HOST(actual_tessdata_num_entries_ <= TESSDATA_NUM_ENTRIES);
  fread(offset_table_, sizeof(inT64),
        actual_tessdata_num_entries_, data_file_);
  if (swap) {
    for (i = 0 ; i < actual_tessdata_num_entries_; ++i) {
      offset_table_[i] = reverse64(offset_table_[i]);
    }
  }
  if (global_tessdata_manager_debug_level) {
    tprintf("TessdataManager loaded %d types of tesseract data files.\n",
            actual_tessdata_num_entries_);
    for (i = 0; i < actual_tessdata_num_entries_; ++i) {
      tprintf("Offset for type %d is %lld\n", i, offset_table_[i]);
    }
  }
}

FILE *TessdataManager::GetFilePtr(const char *language_data_path_prefix,
                                  const char *file_suffix, bool required_file,
                                  bool text_file) {
  STRING file_name = language_data_path_prefix;
  file_name += file_suffix;
  FILE *file_ptr = fopen(file_name.string(), text_file ? "r" : "rb");
  if (required_file && (file_ptr == NULL)) {
    tprintf("Error openning required file %s\n", file_name.string());
    exit(1);
  }
  return file_ptr;
}

void TessdataManager::CopyFile(FILE *input_file, FILE *output_file,
                               bool newline_end) {
  int buffer_size = 1024;
  char *chunk = new char[buffer_size];
  int bytes_read;
  char last_char = 0x0;
  while ((bytes_read = fread(chunk, sizeof(char),
                             buffer_size, input_file))) {
    fwrite(chunk, sizeof(char), bytes_read, output_file);
    last_char = chunk[bytes_read-1];
  }
  if (newline_end) ASSERT_HOST(last_char == '\n');
  delete[] chunk;
}

void TessdataManager::CombineDataFiles(
    const char *language_data_path_prefix,
    const char *output_filename) {
  FILE *file_ptr;
  STRING file_name;
  int i;
  inT64 offset_table[TESSDATA_NUM_ENTRIES];
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) offset_table[i] = -1;
  FILE *output_file = fopen(output_filename, "wb");
  // Leave some space for recording the offset_table.
  fseek(output_file,
        sizeof(inT32) + sizeof(inT64) * TESSDATA_NUM_ENTRIES, SEEK_SET);

  // Record language-specific tesseract config file.
  file_ptr = GetFilePtr(language_data_path_prefix,
                        kLangConfigFileSuffix, false, true);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_LANG_CONFIG] = ftell(output_file);
    CopyFile(file_ptr, output_file, true);
    fclose(file_ptr);
  }

  // Record unicharset.
  file_ptr = GetFilePtr(language_data_path_prefix,
                        kUnicharsetFileSuffix, true, true);
  offset_table[TESSDATA_UNICHARSET] = ftell(output_file);
  CopyFile(file_ptr, output_file, true);
  fclose(file_ptr);

  // Record ambiguities.
  file_ptr = GetFilePtr(language_data_path_prefix,
                        kAmbigsFileSuffix, false, true);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_AMBIGS] = ftell(output_file);
    CopyFile(file_ptr, output_file, true);
    fclose(file_ptr);
  }

  // Record inttemp.
  file_ptr =
    GetFilePtr(language_data_path_prefix,
               kBuiltInTemplatesFileSuffix, false, false);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_INTTEMP] = ftell(output_file);
    CopyFile(file_ptr, output_file, false);
    fclose(file_ptr);

    // Record pffmtable.
    file_ptr = GetFilePtr(language_data_path_prefix,
                          kBuiltInCutoffsFileSuffix, true, true);
    offset_table[TESSDATA_PFFMTABLE] = ftell(output_file);
    CopyFile(file_ptr, output_file, true);
    fclose(file_ptr);

    // Record normproto.
    file_ptr = GetFilePtr(language_data_path_prefix,
                          kNormProtoFileSuffix, true, true);
    offset_table[TESSDATA_NORMPROTO] = ftell(output_file);
    CopyFile(file_ptr, output_file, true);
    fclose(file_ptr);
  }

  // Record dawgs.
  file_ptr = GetFilePtr(language_data_path_prefix,
                        kPuncDawgFileSuffix, false, false);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_PUNC_DAWG] = ftell(output_file);
    CopyFile(file_ptr, output_file, false);
    fclose(file_ptr);
  }

  file_ptr = GetFilePtr(language_data_path_prefix,
                        kSystemDawgFileSuffix, false, false);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_SYSTEM_DAWG] = ftell(output_file);
    CopyFile(file_ptr, output_file, false);
    fclose(file_ptr);
  }

  file_ptr = GetFilePtr(language_data_path_prefix,
                        kNumberDawgFileSuffix, false, false);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_NUMBER_DAWG] = ftell(output_file);
    CopyFile(file_ptr, output_file, false);
    fclose(file_ptr);
  }

  file_ptr = GetFilePtr(language_data_path_prefix,
                        kFreqDawgFileSuffix, false, false);
  if (file_ptr != NULL) {
    offset_table[TESSDATA_FREQ_DAWG] = ftell(output_file);
    CopyFile(file_ptr, output_file, false);
    fclose(file_ptr);
  }

  fseek(output_file, 0, SEEK_SET);
  inT32 num_entries = TESSDATA_NUM_ENTRIES;
  fwrite(&num_entries, sizeof(inT32), 1, output_file);
  fwrite(offset_table, sizeof(inT64), TESSDATA_NUM_ENTRIES, output_file);
  fclose(output_file);

  tprintf("TessdataManager combined tesseract data files.\n");
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    tprintf("Offset for type %d is %lld\n", i, offset_table[i]);
  }
}

}  // namespace tesseract
