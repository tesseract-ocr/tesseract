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

// Lazily loads from the the given filename. Won't actually read the file
// until it needs it.
void TessdataManager::LoadFileLater(const char *data_file_name) {
  Clear();
  data_file_name_ = data_file_name;
}

bool TessdataManager::Init(const char *data_file_name) {
  GenericVector<char> data;
  bool result = true;
  if (reader_ == nullptr) {
    if (!LoadDataFromFile(data_file_name, &data)) return false;
  } else {
    if (!(*reader_)(data_file_name, &data)) return false;
  }
  return LoadMemBuffer(data_file_name, &data[0], data.size());
}

// Loads from the given memory buffer as if a file.
bool TessdataManager::LoadMemBuffer(const char *name, const char *data,
                                    int size) {
  data_file_name_ = name;
  TFile fp;
  fp.Open(data, size);
  inT32 num_entries = TESSDATA_NUM_ENTRIES;
  if (fp.FRead(&num_entries, sizeof(num_entries), 1) != 1) return false;
  swap_ = num_entries > kMaxNumTessdataEntries || num_entries < 0;
  if (swap_) ReverseN(&num_entries, sizeof(num_entries));
  GenericVector<inT64> offset_table;
  offset_table.init_to_size(num_entries, -1);
  if (fp.FReadEndian(&offset_table[0], sizeof(offset_table[0]), num_entries,
                     swap_) != num_entries)
    return false;
  for (int i = 0; i < num_entries && i < TESSDATA_NUM_ENTRIES; ++i) {
    if (offset_table[i] >= 0) {
      inT64 entry_size = size - offset_table[i];
      int j = i + 1;
      while (j < num_entries && offset_table[j] == -1) ++j;
      if (j < num_entries) entry_size = offset_table[j] - offset_table[i];
      entries_[i].init_to_size(entry_size, 0);
      if (fp.FRead(&entries_[i][0], 1, entry_size) != entry_size) return false;
    }
  }
  is_loaded_ = true;
  return true;
}

// Overwrites a single entry of the given type.
void TessdataManager::OverwriteEntry(TessdataType type, const char *data,
                                     int size) {
  is_loaded_ = true;
  entries_[type].init_to_size(size, 0);
  memcpy(&entries_[type][0], data, size);
}

// Saves to the given filename.
bool TessdataManager::SaveFile(const STRING &filename,
                               FileWriter writer) const {
  ASSERT_HOST(is_loaded_);
  GenericVector<char> data;
  Serialize(&data);
  if (writer == nullptr)
    return SaveDataToFile(data, filename);
  else
    return (*writer)(data, filename);
}

// Serializes to the given vector.
void TessdataManager::Serialize(GenericVector<char> *data) const {
  ASSERT_HOST(is_loaded_);
  // Compute the offset_table and total size.
  inT64 offset_table[TESSDATA_NUM_ENTRIES];
  inT64 offset = sizeof(inT32) + sizeof(offset_table);
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (entries_[i].empty()) {
      offset_table[i] = -1;
    } else {
      offset_table[i] = offset;
      offset += entries_[i].size();
    }
  }
  data->init_to_size(offset, 0);
  inT32 num_entries = TESSDATA_NUM_ENTRIES;
  TFile fp;
  fp.OpenWrite(data);
  fp.FWrite(&num_entries, sizeof(num_entries), 1);
  fp.FWrite(offset_table, sizeof(offset_table), 1);
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (!entries_[i].empty()) {
      fp.FWrite(&entries_[i][0], entries_[i].size(), 1);
    }
  }
}

// Resets to the initial state, keeping the reader.
void TessdataManager::Clear() {
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    entries_[i].clear();
  }
  is_loaded_ = false;
}

// Prints a directory of contents.
void TessdataManager::Directory() const {
  int offset = TESSDATA_NUM_ENTRIES * sizeof(inT64);
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (!entries_[i].empty()) {
      tprintf("%d:%s:size=%d, offset=%d\n", i, kTessdataFileSuffixes[i],
              entries_[i].size(), offset);
      offset += entries_[i].size();
    }
  }
}

// Opens the given TFile pointer to the given component type.
// Returns false in case of failure.
bool TessdataManager::GetComponent(TessdataType type, TFile *fp) {
  if (!is_loaded_ && !Init(data_file_name_.string())) return false;
  if (entries_[type].empty()) return false;
  fp->Open(&entries_[type][0], entries_[type].size());
  return true;
}

bool TessdataManager::CombineDataFiles(
    const char *language_data_path_prefix,
    const char *output_filename) {
  // Load individual tessdata components from files.
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    TessdataType type;
    ASSERT_HOST(TessdataTypeFromFileSuffix(kTessdataFileSuffixes[i], &type));
    STRING filename = language_data_path_prefix;
    filename += kTessdataFileSuffixes[i];
    FILE *fp = fopen(filename.string(), "rb");
    if (fp != nullptr) {
      fclose(fp);
      if (!LoadDataFromFile(filename, &entries_[type])) {
        tprintf("Load of file %s failed!\n", filename.string());
        return false;
      }
    }
  }
  is_loaded_ = true;

  // Make sure that the required components are present.
  if (!IsBaseAvailable() && !IsLSTMAvailable()) {
    tprintf(
        "Error: traineddata file must contain at least (a unicharset file"
        "and inttemp) OR an lstm file.\n");
    return false;
  }
  // Write updated data to the output traineddata file.
  return SaveFile(output_filename, nullptr);
}

bool TessdataManager::OverwriteComponents(
    const char *new_traineddata_filename,
    char **component_filenames,
    int num_new_components) {
  // Open the files with the new components.
  for (int i = 0; i < num_new_components; ++i) {
    TessdataType type;
    if (TessdataTypeFromFileName(component_filenames[i], &type)) {
      if (!LoadDataFromFile(component_filenames[i], &entries_[type])) {
        tprintf("Failed to read component file:%s\n", component_filenames[i]);
        return false;
      }
    }
  }

  // Write updated data to the output traineddata file.
  return SaveFile(new_traineddata_filename, nullptr);
}

bool TessdataManager::ExtractToFile(const char *filename) {
  TessdataType type = TESSDATA_NUM_ENTRIES;
  ASSERT_HOST(
      tesseract::TessdataManager::TessdataTypeFromFileName(filename, &type));
  if (entries_[type].empty()) return false;
  return SaveDataToFile(entries_[type], filename);
}

bool TessdataManager::TessdataTypeFromFileSuffix(const char *suffix,
                                                 TessdataType *type) {
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (strcmp(kTessdataFileSuffixes[i], suffix) == 0) {
      *type = static_cast<TessdataType>(i);
      return true;
    }
  }
  tprintf("TessdataManager can't determine which tessdata"
         " component is represented by %s\n", suffix);
  return false;
}

bool TessdataManager::TessdataTypeFromFileName(const char *filename,
                                               TessdataType *type) {
  // Get the file suffix (extension)
  const char *suffix = strrchr(filename, '.');
  if (suffix == nullptr || *(++suffix) == '\0') return false;
  return TessdataTypeFromFileSuffix(suffix, type);
}

}  // namespace tesseract
