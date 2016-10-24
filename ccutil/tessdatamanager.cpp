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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "tessdatamanager.h"

#include <stdio.h>

#include "helpers.h"
#include "serialis.h"
#include "strngs.h"
#include "tprintf.h"
#include "params.h"

namespace tesseract {

bool TessdataManager::Init(const char *data_file_name, int debug_level) {
  int i;
  debug_level_ = debug_level;
  data_file_name_ = data_file_name;
  data_file_ = fopen(data_file_name, "rb");
  if (data_file_ == NULL) {
    tprintf("Error opening data file %s\n", data_file_name);
    tprintf("Please make sure the TESSDATA_PREFIX environment variable is set "
            "to the parent directory of your \"tessdata\" directory.\n");
    return false;
  }
  fread(&actual_tessdata_num_entries_, sizeof(inT32), 1, data_file_);
  swap_ = (actual_tessdata_num_entries_ > kMaxNumTessdataEntries);
  if (swap_) {
    ReverseN(&actual_tessdata_num_entries_,
             sizeof(actual_tessdata_num_entries_));
  }
  if (actual_tessdata_num_entries_ > TESSDATA_NUM_ENTRIES) {
    // For forward compatibility, truncate to the number we can handle.
    actual_tessdata_num_entries_ = TESSDATA_NUM_ENTRIES;
  }
  fread(offset_table_, sizeof(inT64),
        actual_tessdata_num_entries_, data_file_);
  if (swap_) {
    for (i = 0 ; i < actual_tessdata_num_entries_; ++i) {
      ReverseN(&offset_table_[i], sizeof(offset_table_[i]));
    }
  }
  if (debug_level_) {
    tprintf("TessdataManager loaded %d types of tesseract data files.\n",
            actual_tessdata_num_entries_);
    for (i = 0; i < actual_tessdata_num_entries_; ++i) {
      tprintf("Offset for type %d is %lld\n", i, offset_table_[i]);
    }
  }
  return true;
}

void TessdataManager::CopyFile(FILE *input_file, FILE *output_file,
                               bool newline_end, inT64 num_bytes_to_copy) {
  if (num_bytes_to_copy == 0) return;
  int buffer_size = 1024;
  if (num_bytes_to_copy > 0 && buffer_size > num_bytes_to_copy) {
    buffer_size = num_bytes_to_copy;
  }
  inT64 num_bytes_copied = 0;
  char *chunk = new char[buffer_size];
  int bytes_read;
  char last_char = 0x0;
  while ((bytes_read = fread(chunk, sizeof(char),
                             buffer_size, input_file))) {
    fwrite(chunk, sizeof(char), bytes_read, output_file);
    last_char = chunk[bytes_read-1];
    if (num_bytes_to_copy > 0) {
      num_bytes_copied += bytes_read;
      if (num_bytes_copied == num_bytes_to_copy) break;
      if (num_bytes_copied + buffer_size > num_bytes_to_copy) {
        buffer_size = num_bytes_to_copy - num_bytes_copied;
      }
    }
  }
  if (newline_end) ASSERT_HOST(last_char == '\n');
  delete[] chunk;
}

bool TessdataManager::WriteMetadata(inT64 *offset_table,
                                    const char * language_data_path_prefix,
                                    FILE *output_file) {
  inT32 num_entries = TESSDATA_NUM_ENTRIES;
  bool result = true;
  if (fseek(output_file, 0, SEEK_SET) != 0 ||
      fwrite(&num_entries, sizeof(inT32), 1, output_file) != 1 ||
      fwrite(offset_table, sizeof(inT64), TESSDATA_NUM_ENTRIES,
             output_file) != TESSDATA_NUM_ENTRIES) {
    fclose(output_file);
    result = false;
    tprintf("WriteMetadata failed in TessdataManager!\n");
  } else if (fclose(output_file)) {
    result = false;
    tprintf("WriteMetadata failed to close file!\n");
  } else {
    tprintf("TessdataManager combined tesseract data files.\n");
    for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
      tprintf("Offset for type %2d (%s%-22s) is %lld\n", i,
              language_data_path_prefix, kTessdataFileSuffixes[i],
              offset_table[i]);
    }
  }
  return result;
}

bool TessdataManager::CombineDataFiles(
    const char *language_data_path_prefix,
    const char *output_filename) {
  int i;
  inT64 offset_table[TESSDATA_NUM_ENTRIES];
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) offset_table[i] = -1;
  FILE *output_file = fopen(output_filename, "wb");
  if (output_file == NULL) {
    tprintf("Error opening %s for writing\n", output_filename);
    return false;
  }
  // Leave some space for recording the offset_table.
  if (fseek(output_file,
            sizeof(inT32) + sizeof(inT64) * TESSDATA_NUM_ENTRIES, SEEK_SET)) {
    tprintf("Error seeking %s\n", output_filename);
    fclose(output_file);
    return false;
  }

  TessdataType type = TESSDATA_NUM_ENTRIES;
  bool text_file = false;
  FILE *file_ptr[TESSDATA_NUM_ENTRIES];

  // Load individual tessdata components from files.
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    ASSERT_HOST(TessdataTypeFromFileSuffix(
        kTessdataFileSuffixes[i], &type, &text_file));
    STRING filename = language_data_path_prefix;
    filename += kTessdataFileSuffixes[i];
    file_ptr[i] =  fopen(filename.string(), "rb");
    if (file_ptr[i] != NULL) {
      offset_table[type] = ftell(output_file);
      CopyFile(file_ptr[i], output_file, text_file, -1);
      fclose(file_ptr[i]);
    }
  }

  // Make sure that the required components are present.
  if (file_ptr[TESSDATA_UNICHARSET] == NULL) {
    tprintf("Error opening %sunicharset file\n", language_data_path_prefix);
    fclose(output_file);
    return false;
  }
  if (file_ptr[TESSDATA_INTTEMP] != NULL &&
      (file_ptr[TESSDATA_PFFMTABLE] == NULL ||
       file_ptr[TESSDATA_NORMPROTO] == NULL)) {
    tprintf("Error opening %spffmtable and/or %snormproto files"
            " while %sinttemp file was present\n", language_data_path_prefix,
            language_data_path_prefix, language_data_path_prefix);
    fclose(output_file);
    return false;
  }

  return WriteMetadata(offset_table, language_data_path_prefix, output_file);
}

bool TessdataManager::OverwriteComponents(
    const char *new_traineddata_filename,
    char **component_filenames,
    int num_new_components) {
  int i;
  inT64 offset_table[TESSDATA_NUM_ENTRIES];
  TessdataType type = TESSDATA_NUM_ENTRIES;
  bool text_file = false;
  FILE *file_ptr[TESSDATA_NUM_ENTRIES];
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    offset_table[i] = -1;
    file_ptr[i] = NULL;
  }
  FILE *output_file = fopen(new_traineddata_filename, "wb");
  if (output_file == NULL) {
    tprintf("Error opening %s for writing\n", new_traineddata_filename);
    return false;
  }

  // Leave some space for recording the offset_table.
  if (fseek(output_file,
            sizeof(inT32) + sizeof(inT64) * TESSDATA_NUM_ENTRIES, SEEK_SET)) {
    fclose(output_file);
    tprintf("Error seeking %s\n", new_traineddata_filename);
    return false;
  }

  // Open the files with the new components.
  for (i = 0; i < num_new_components; ++i) {
    if (TessdataTypeFromFileName(component_filenames[i], &type, &text_file))
      file_ptr[type] = fopen(component_filenames[i], "rb");
  }

  // Write updated data to the output traineddata file.
  for (i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (file_ptr[i] != NULL) {
      // Get the data from the opened component file.
      offset_table[i] = ftell(output_file);
      CopyFile(file_ptr[i], output_file, kTessdataFileIsText[i], -1);
      fclose(file_ptr[i]);
    } else {
      // Get this data component from the loaded data file.
      if (SeekToStart(static_cast<TessdataType>(i))) {
        offset_table[i] = ftell(output_file);
        CopyFile(data_file_, output_file, kTessdataFileIsText[i],
                 GetEndOffset(static_cast<TessdataType>(i)) -
                 ftell(data_file_) + 1);
      }
    }
  }
  const char *language_data_path_prefix = strchr(new_traineddata_filename, '.');
  return WriteMetadata(offset_table, language_data_path_prefix, output_file);
}

bool TessdataManager::TessdataTypeFromFileSuffix(
    const char *suffix, TessdataType *type, bool *text_file) {
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (strcmp(kTessdataFileSuffixes[i], suffix) == 0) {
      *type = static_cast<TessdataType>(i);
      *text_file = kTessdataFileIsText[i];
      return true;
    }
  }
  tprintf("TessdataManager can't determine which tessdata"
         " component is represented by %s\n", suffix);
  return false;
}

bool TessdataManager::TessdataTypeFromFileName(
    const char *filename, TessdataType *type, bool *text_file) {
  // Get the file suffix (extension)
  const char *suffix = strrchr(filename, '.');
  if (suffix == NULL || *(++suffix) == '\0') return false;
  return TessdataTypeFromFileSuffix(suffix, type, text_file);
}

bool TessdataManager::ExtractToFile(const char *filename) {
  TessdataType type = TESSDATA_NUM_ENTRIES;
  bool text_file = false;
  ASSERT_HOST(tesseract::TessdataManager::TessdataTypeFromFileName(
      filename, &type, &text_file));
  if (!SeekToStart(type)) return false;

  FILE *output_file = fopen(filename, "wb");
  if (output_file == NULL) {
    tprintf("Error opening %s\n", filename);
    exit(1);
  }
  inT64 begin_offset = ftell(GetDataFilePtr());
  inT64 end_offset = GetEndOffset(type);
  tesseract::TessdataManager::CopyFile(
      GetDataFilePtr(), output_file, text_file,
      end_offset - begin_offset + 1);
  fclose(output_file);
  return true;
}

}  // namespace tesseract
