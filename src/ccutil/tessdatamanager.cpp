///////////////////////////////////////////////////////////////////////
// File:        tessdatamanager.cpp
// Description: Functions to handle loading/combining tesseract data files.
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "tessdatamanager.h"

#include <cstdio>
#include <string>

#if defined(HAVE_LIBARCHIVE)
#  include <archive.h>
#  include <archive_entry.h>
#endif

#include <tesseract/version.h>
#include "errcode.h"
#include "helpers.h"
#include "params.h"
#include "serialis.h"
#include "tprintf.h"

namespace tesseract {

TessdataManager::TessdataManager() : reader_(nullptr), is_loaded_(false), swap_(false) {
  SetVersionString(TESSERACT_VERSION_STR);
}

TessdataManager::TessdataManager(FileReader reader)
    : reader_(reader), is_loaded_(false), swap_(false) {
  SetVersionString(TESSERACT_VERSION_STR);
}

// Lazily loads from the given filename. Won't actually read the file
// until it needs it.
void TessdataManager::LoadFileLater(const char *data_file_name) {
  Clear();
  data_file_name_ = data_file_name;
}

#if defined(HAVE_LIBARCHIVE)
bool TessdataManager::LoadArchiveFile(const char *filename) {
  bool result = false;
  archive *a = archive_read_new();
  if (a != nullptr) {
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    if (archive_read_open_filename(a, filename, 8192) == ARCHIVE_OK) {
      archive_entry *ae;
      while (archive_read_next_header(a, &ae) == ARCHIVE_OK) {
        const char *component = archive_entry_pathname(ae);
        if (component != nullptr) {
          TessdataType type;
          if (TessdataTypeFromFileName(component, &type)) {
            int64_t size = archive_entry_size(ae);
            if (size > 0) {
              entries_[type].resize(size);
              if (archive_read_data(a, &entries_[type][0], size) == size) {
                is_loaded_ = true;
              }
            }
          }
        }
      }
      result = is_loaded_;
    }
    archive_read_free(a);
  }
  return result;
}

bool TessdataManager::SaveArchiveFile(const char *filename) const{
  bool result = false;
  archive *a = archive_write_new();
  archive_entry *ae = archive_entry_new();
  if (a != nullptr) {
    archive_write_set_format_zip(a);
    archive_write_open_filename(a, filename);
    std::string filename_str = filename;
    filename_str += ".";
    archive_entry_set_filetype(ae, AE_IFREG);
    archive_entry_set_perm(ae, 333);
    for (unsigned i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
      if (!entries_[i].empty()) {
        archive_entry_set_pathname(ae, (filename_str + kTessdataFileSuffixes[i]).c_str());
        archive_entry_set_size(ae, entries_[i].size());
        archive_write_header(a, ae);
        archive_write_data(a, &entries_[i][0], entries_[i].size());
      }
    }
    result = archive_write_close(a) == ARCHIVE_OK;
    archive_write_free(a);
    return result;
  }
  return result;
}
#endif

bool TessdataManager::Init(const char *data_file_name) {
  std::vector<char> data;
  if (reader_ == nullptr) {
#if defined(HAVE_LIBARCHIVE)
    if (LoadArchiveFile(data_file_name)) {
      return true;
    }
#endif
    if (!LoadDataFromFile(data_file_name, &data)) {
      return false;
    }
  } else {
    if (!(*reader_)(data_file_name, &data)) {
      return false;
    }
  }
  return LoadMemBuffer(data_file_name, &data[0], data.size());
}

// Loads from the given memory buffer as if a file.
bool TessdataManager::LoadMemBuffer(const char *name, const char *data, int size) {
  // TODO: This method supports only the proprietary file format.
  Clear();
  data_file_name_ = name;
  TFile fp;
  fp.Open(data, size);
  uint32_t num_entries;
  if (!fp.DeSerialize(&num_entries)) {
    return false;
  }
  swap_ = num_entries > kMaxNumTessdataEntries;
  fp.set_swap(swap_);
  if (swap_) {
    ReverseN(&num_entries, sizeof(num_entries));
  }
  if (num_entries > kMaxNumTessdataEntries) {
    return false;
  }
  // TODO: optimize (no init required).
  std::vector<int64_t> offset_table(num_entries);
  if (!fp.DeSerialize(&offset_table[0], num_entries)) {
    return false;
  }
  for (unsigned i = 0; i < num_entries && i < TESSDATA_NUM_ENTRIES; ++i) {
    if (offset_table[i] >= 0) {
      int64_t entry_size = size - offset_table[i];
      unsigned j = i + 1;
      while (j < num_entries && offset_table[j] == -1) {
        ++j;
      }
      if (j < num_entries) {
        entry_size = offset_table[j] - offset_table[i];
      }
      entries_[i].resize(entry_size);
      if (!fp.DeSerialize(&entries_[i][0], entry_size)) {
        return false;
      }
    }
  }
  if (entries_[TESSDATA_VERSION].empty()) {
    SetVersionString("Pre-4.0.0");
  }
  is_loaded_ = true;
  return true;
}

// Overwrites a single entry of the given type.
void TessdataManager::OverwriteEntry(TessdataType type, const char *data, int size) {
  is_loaded_ = true;
  entries_[type].resize(size);
  memcpy(&entries_[type][0], data, size);
}

// Saves to the given filename.
bool TessdataManager::SaveFile(const char *filename, FileWriter writer) const {
// TODO: This method supports only the proprietary file format.
  ASSERT_HOST(is_loaded_);
  std::vector<char> data;
  Serialize(&data);
  if (writer == nullptr) {
#if defined(HAVE_LIBARCHIVE)
    return SaveArchiveFile(filename);
#else
    return SaveDataToFile(data, filename);
#endif
  } else {
    return (*writer)(data, filename);
  }
}

// Serializes to the given vector.
void TessdataManager::Serialize(std::vector<char> *data) const {
  // TODO: This method supports only the proprietary file format.
  ASSERT_HOST(is_loaded_);
  // Compute the offset_table and total size.
  int64_t offset_table[TESSDATA_NUM_ENTRIES];
  int64_t offset = sizeof(int32_t) + sizeof(offset_table);
  for (unsigned i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (entries_[i].empty()) {
      offset_table[i] = -1;
    } else {
      offset_table[i] = offset;
      offset += entries_[i].size();
    }
  }
  data->resize(offset, 0);
  int32_t num_entries = TESSDATA_NUM_ENTRIES;
  TFile fp;
  fp.OpenWrite(data);
  fp.Serialize(&num_entries);
  fp.Serialize(&offset_table[0], countof(offset_table));
  for (const auto &entry : entries_) {
    if (!entry.empty()) {
      fp.Serialize(&entry[0], entry.size());
    }
  }
}

// Resets to the initial state, keeping the reader.
void TessdataManager::Clear() {
  for (auto &entry : entries_) {
    entry.clear();
  }
  is_loaded_ = false;
}

// Prints a directory of contents.
void TessdataManager::Directory() const {
  printf("Version:%s\n", VersionString().c_str());
  auto offset = TESSDATA_NUM_ENTRIES * sizeof(int64_t);
  for (unsigned i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (!entries_[i].empty()) {
      printf("%u:%s:size=%zu, offset=%zu\n", i, kTessdataFileSuffixes[i], entries_[i].size(),
              offset);
      offset += entries_[i].size();
    }
  }
}

// Opens the given TFile pointer to the given component type.
// Returns false in case of failure.
bool TessdataManager::GetComponent(TessdataType type, TFile *fp) {
  if (!is_loaded_ && !Init(data_file_name_.c_str())) {
    return false;
  }
  const TessdataManager *const_this = this;
  return const_this->GetComponent(type, fp);
}

// As non-const version except it can't load the component if not already
// loaded.
bool TessdataManager::GetComponent(TessdataType type, TFile *fp) const {
  ASSERT_HOST(is_loaded_);
  if (entries_[type].empty()) {
    return false;
  }
  fp->Open(&entries_[type][0], entries_[type].size());
  fp->set_swap(swap_);
  return true;
}

// Returns the current version string.
std::string TessdataManager::VersionString() const {
  return std::string(&entries_[TESSDATA_VERSION][0], entries_[TESSDATA_VERSION].size());
}

// Sets the version string to the given v_str.
void TessdataManager::SetVersionString(const std::string &v_str) {
  entries_[TESSDATA_VERSION].resize(v_str.size());
  memcpy(&entries_[TESSDATA_VERSION][0], v_str.data(), v_str.size());
}

bool TessdataManager::CombineDataFiles(const char *language_data_path_prefix,
                                       const char *output_filename) {
  // Load individual tessdata components from files.
  for (auto filesuffix : kTessdataFileSuffixes) {
    TessdataType type;
    ASSERT_HOST(TessdataTypeFromFileSuffix(filesuffix, &type));
    std::string filename = language_data_path_prefix;
    filename += filesuffix;
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp != nullptr) {
      fclose(fp);
      if (!LoadDataFromFile(filename.c_str(), &entries_[type])) {
        tprintf("Load of file %s failed!\n", filename.c_str());
        return false;
      }
    }
  }
  is_loaded_ = true;

  // Make sure that the required components are present.
  if (!IsBaseAvailable() && !IsLSTMAvailable()) {
    tprintf(
        "Error: traineddata file must contain at least (a unicharset file"
        " and inttemp) OR an lstm file.\n");
    return false;
  }
  // Write updated data to the output traineddata file.
  return SaveFile(output_filename, nullptr);
}

bool TessdataManager::OverwriteComponents(const char *new_traineddata_filename,
                                          char **component_filenames, int num_new_components) {
  // Open the files with the new components.
  // TODO: This method supports only the proprietary file format.
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
  ASSERT_HOST(tesseract::TessdataManager::TessdataTypeFromFileName(filename, &type));
  if (entries_[type].empty()) {
    return false;
  }
  return SaveDataToFile(entries_[type], filename);
}

bool TessdataManager::TessdataTypeFromFileSuffix(const char *suffix, TessdataType *type) {
  for (unsigned i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
    if (strcmp(kTessdataFileSuffixes[i], suffix) == 0) {
      *type = static_cast<TessdataType>(i);
      return true;
    }
  }
#if !defined(NDEBUG)
  tprintf(
      "TessdataManager can't determine which tessdata"
      " component is represented by %s\n",
      suffix);
#endif
  return false;
}

bool TessdataManager::TessdataTypeFromFileName(const char *filename, TessdataType *type) {
  // Get the file suffix (extension)
  const char *suffix = strrchr(filename, '.');
  if (suffix == nullptr || *(++suffix) == '\0') {
    return false;
  }
  return TessdataTypeFromFileSuffix(suffix, type);
}

} // namespace tesseract
