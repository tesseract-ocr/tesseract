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
#include <string>

#if defined(HAVE_LIBARCHIVE)
#include <archive.h>
#include <archive_entry.h>
#endif
#if defined(HAVE_LIBZIP)
#if defined(HAVE_MINIZIP)
// libminizip provides minizip/zip.h. Hack to get the right one.
#include "/usr/include/zip.h"
#else
#include <zip.h>
#endif
#endif
#if defined(HAVE_MINIZIP)
#include <unzip.h>
#endif
#if defined(HAVE_ZZIPLIB)
#include <zzip/lib.h>
#endif

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
              entries_[type].resize_no_init(size);
              if (archive_read_data(a, &entries_[type][0], size) == size) {
                is_loaded_ = true;
              }
            }
          }
        }
      }
      result = is_loaded_;
    } else {
      tprintf("archive_read_open_filename(...,%s,...) failed, %s\n",
              filename, strerror(archive_errno(a)));
    }
    archive_read_free(a);
  }
  return result;
}
#endif

#if defined(HAVE_LIBZIP)
bool TessdataManager::LoadZipFile(const char *filename) {
  bool result = false;
  int err;
  zip_t *uf = zip_open(filename, ZIP_RDONLY, &err);
  if (uf != nullptr) {
    int64_t nEntries = zip_get_num_entries(uf, ZIP_FL_UNCHANGED);
    for (int i = 0; i < nEntries; i++) {
      zip_stat_t zipStat;
      if (zip_stat_index(uf, i, ZIP_FL_UNCHANGED, &zipStat) == 0 &&
          (zipStat.valid & ZIP_STAT_NAME) && (zipStat.valid & ZIP_STAT_SIZE)) {
        TessdataType type;
        if (TessdataTypeFromFileName(zipStat.name, &type)) {
          zip_file_t *zipFile = zip_fopen_index(uf, i, ZIP_FL_UNCHANGED);
          if (zipFile == nullptr) {
            tprintf("zip_fopen_index(...) failed\n");
          } else {
            entries_[type].resize_no_init(zipStat.size);
            if (zip_fread(zipFile, &entries_[type][0], zipStat.size) !=
                static_cast<int64_t>(zipStat.size)) {
              tprintf("zip_fread(...) failed\n");
            }
            zip_fclose(zipFile);
          }
        }
      }
    }
    is_loaded_ = true;
    err = zip_close(uf);
    if (err != 0) {
      tprintf("zip_close(...) failed\n");
    }
    result = true;
  }
  return result;
}
#endif

#if defined(HAVE_MINIZIP)
bool TessdataManager::LoadMinizipFile(const char *filename) {
  bool result = false;
  unzFile uf = unzOpen(filename);
  if (uf != nullptr) {
    unz_global_info global_info;
    int err;
    err = unzGetGlobalInfo(uf, &global_info);
    if (err == UNZ_OK) {
    }
    unz_file_info file_info;
    char component[32];
    char extraField[32];
    char comment[32];
    //~ $1 = {version = 798, version_needed = 20, flag = 0, compression_method = 8, dosDate = 1252768343, crc = 2481269679, compressed_size = 7131663, uncompressed_size = 16109842,
    //~ size_filename = 15, size_file_extra = 24, size_file_comment = 0, disk_num_start = 0, internal_fa = 0, external_fa = 2175008768, tmu_date = {tm_sec = 46, tm_min = 18,
    //~ tm_hour = 23, tm_mday = 11, tm_mon = 4, tm_year = 2017}}
    for (unsigned i = 0; i < global_info.number_entry; i++) {
      err = unzGetCurrentFileInfo(uf, &file_info,
                                  component, sizeof(component),
                                  extraField, sizeof(extraField),
                                  comment, sizeof(comment));
      if (err == UNZ_OK) {
        TessdataType type;
        if (TessdataTypeFromFileName(component, &type)) {
          err = unzOpenCurrentFilePassword(uf, nullptr);
          if (err != UNZ_OK) {
            tprintf("unzOpenCurrentFilePassword(...) failed, err %d\n", err);
          } else {
            entries_[type].resize_no_init(file_info.uncompressed_size);
            err = unzReadCurrentFile(uf, &entries_[type][0], file_info.uncompressed_size);
            if (err < UNZ_OK) {
              tprintf("unzReadCurrentFile(...) failed, err %d\n", err);
            }
            err = unzCloseCurrentFile(uf);
            if (err != UNZ_OK) {
              tprintf("unzCloseCurrentFile(...) failed\n");
            }
          }
        }
      }

      err = unzGoToNextFile(uf);
      if (err != UNZ_OK) {
        tprintf("unzGoToNextFile(...) failed\n");
      }
    }
    is_loaded_ = true;
    err = unzClose(uf);
    if (err != UNZ_OK) {
      tprintf("unzClose(...) failed\n");
    }
    result = true;
  }
  return result;
}
#endif

#if defined(HAVE_ZZIPLIB)
bool TessdataManager::LoadZzipFile(const char *filename) {
  bool result = false;
  zzip_error_t err;
  ZZIP_DIR *dir = zzip_dir_open(filename, &err);
  if (dir != nullptr) {
    ZZIP_DIRENT d;
    while (zzip_dir_read(dir, &d)) {
      TessdataType type;
      if (TessdataTypeFromFileName(d.d_name, &type)) {
        ZZIP_FILE *f = zzip_file_open(dir, d.d_name, 0);
        if (f != nullptr) {
          entries_[type].resize_no_init(d.st_size);
          ssize_t len = zzip_file_read(f, &entries_[type][0], d.st_size);
          if (len != d.st_size) {
            tprintf("zzip_file_read(...) failed\n");
          }
          zzip_file_close(f);
        }
      }
    }
    is_loaded_ = true;
    zzip_dir_close(dir);
    result = true;
  }
  return result;
}
#endif

bool TessdataManager::Init(const char *data_file_name) {
  GenericVector<char> data;
  if (reader_ == nullptr) {
    const char *tessarchive = getenv("TESSARCHIVE");
#if defined(HAVE_LIBARCHIVE)
    if (tessarchive == nullptr || strcmp(tessarchive, "libarchive") == 0)
    if (LoadArchiveFile(data_file_name)) return true;
#endif
#if defined(HAVE_MINIZIP)
    if (tessarchive == nullptr || strcmp(tessarchive, "libminizip") == 0)
    if (LoadZipFile(data_file_name)) return true;
#endif // HAVE_MINIZIP
#if defined(HAVE_LIBZIP)
    if (tessarchive == nullptr || strcmp(tessarchive, "libzip") == 0)
    if (LoadZipFile(data_file_name)) return true;
#endif // HAVE_MINIZIP
#if defined(HAVE_ZZIPLIB)
    if (tessarchive == nullptr || strcmp(tessarchive, "libzzip") == 0)
    if (LoadZzipFile(data_file_name)) return true;
#endif
    if (!LoadDataFromFile(data_file_name, &data)) return false;
  } else {
    if (!(*reader_)(data_file_name, &data)) return false;
  }
  return LoadMemBuffer(data_file_name, &data[0], data.size());
}

// Loads from the given memory buffer as if a file.
bool TessdataManager::LoadMemBuffer(const char *name, const char *data,
                                    int size) {
  // TODO: This method supports only the proprietary file format.
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
  // TODO: This method supports only the proprietary file format.
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
  // TODO: This method supports only the proprietary file format.
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
