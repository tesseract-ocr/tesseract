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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "tessdatamanager.h"

#include <cstdio>

#include "errcode.h"
#include "helpers.h"
#include "serialis.h"
#include "strngs.h"
#include "tprintf.h"
#include "params.h"

namespace tesseract {

TessdataManager::TessdataManager() : reader_(nullptr), is_loaded_(false), swap_(false) {
  SetVersionString(PACKAGE_VERSION);
}

TessdataManager::TessdataManager(FileReader reader)
  : reader_(reader),
    is_loaded_(false),
    swap_(false) {
  SetVersionString(PACKAGE_VERSION);
}

// Lazily loads from the the given filename. Won't actually read the file
// until it needs it.
void TessdataManager::LoadFileLater(const char *data_file_name) {
  Clear();
  data_file_name_ = data_file_name;
}

bool TessdataManager::Init(const char *data_file_name) {
  GenericVector<char> data;
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
  Clear();
  data_file_name_ = name;
  TFile fp;
  fp.Open(data, size);
  uint32_t num_entries;
  if (!fp.DeSerialize(&num_entries)) return false;
  swap_ = num_entries > kMaxNumTessdataEntries;
  fp.set_swap(swap_);
  if (swap_) ReverseN(&num_entries, sizeof(num_entries));
  if (num_entries > kMaxNumTessdataEntries) return false;
  GenericVector<int64_t> offset_table;
  offset_table.resize_no_init(num_entries);
  if (!fp.DeSerialize(&offset_table[0], num_entries)) return false;
  for (int i = 0; i < num_entries && i < TESSDATA_NUM_ENTRIES; ++i) {
    if (offset_table[i] >= 0) {
      int64_t entry_size = size - offset_table[i];
      int j = i + 1;
      while (j < num_entries && offset_table[j] == -1) ++j;
      if (j < num_entries) entry_size = offset_table[j] - offset_table[i];
      entries_[i].resize_no_init(entry_size);
      if (!fp.DeSerialize(&entries_[i][0], entry_size)) return false;
    }
  }
  if (entries_[TESSDATA_VERSION].empty()) {
    SetVersionString("Pre-4.0.0");
  }
  is_loaded_ = true;
  return true;
}

// Overwrites a single entry of the given type.
void TessdataManager::OverwriteEntry(TessdataType type, const char *data,
                                     int size) {
  is_loaded_ = true;
  entries_[type].resize_no_init(size);
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
  int64_t offset_table[TESSDATA_NUM_ENTRIES];
  int64_t offset = sizeof(int32_t) + sizeof(offset_table);
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (entries_[i].empty()) {
      offset_table[i] = -1;
    } else {
      offset_table[i] = offset;
      offset += entries_[i].size();
    }
  }
  data->init_to_size(offset, 0);
  int32_t num_entries = TESSDATA_NUM_ENTRIES;
  TFile fp;
  fp.OpenWrite(data);
  fp.Serialize(&num_entries);
  fp.Serialize(&offset_table[0], countof(offset_table));
  for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (!entries_[i].empty()) {
      fp.Serialize(&entries_[i][0], entries_[i].size());
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
  tprintf("Version string:%s\n", VersionString().c_str());
  int offset = TESSDATA_NUM_ENTRIES * sizeof(int64_t);
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
  const TessdataManager *const_this = this;
  return const_this->GetComponent(type, fp);
}

// As non-const version except it can't load the component if not already
// loaded.
bool TessdataManager::GetComponent(TessdataType type, TFile *fp) const {
  ASSERT_HOST(is_loaded_);
  if (entries_[type].empty()) return false;
  fp->Open(&entries_[type][0], entries_[type].size());
  fp->set_swap(swap_);
  return true;
}

// Returns the current version string.
std::string TessdataManager::VersionString() const {
  return std::string(&entries_[TESSDATA_VERSION][0],
                     entries_[TESSDATA_VERSION].size());
}

// Sets the version string to the given v_str.
void TessdataManager::SetVersionString(const std::string &v_str) {
  entries_[TESSDATA_VERSION].resize_no_init(v_str.size());
  memcpy(&entries_[TESSDATA_VERSION][0], v_str.data(), v_str.size());
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
