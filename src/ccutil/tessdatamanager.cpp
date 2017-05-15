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
#ifndef NDEBUG
  tprintf("TessdataManager::%s(%s)\n", __func__, filename);
#endif
  archive *a = archive_read_new();
  if (a != nullptr) {
    archive_read_support_filter_all(a); // new
    archive_read_support_format_all(a);
      std::string zipfile(filename);
      bool found = false;
      if (archive_read_open_filename(a, zipfile.c_str(), 4096) == ARCHIVE_OK) {
        found = true;
      } else {
        archive_read_free(a);
        zipfile += ".zip";
        a = archive_read_new();
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);
        if (archive_read_open_filename(a, zipfile.c_str(), 4096) == ARCHIVE_OK) {
          found = true;
        }
      }
      if (found) {
        archive_entry *ae;
        while (archive_read_next_header(a, &ae) == ARCHIVE_OK) {
          const char *fileName = archive_entry_pathname(ae);
          TessdataType type;
          if (fileName != nullptr &&
              TessdataTypeFromFileName(fileName, &type)) {
#if !defined(NDEBUG)
            tprintf("TessdataTypeFromFileName(%s, ...) passed, type %d\n",
                    fileName, type);
#endif
            int64_t size = archive_entry_size(ae);
            if (size > 0) {
              entries_[type].resize_no_init(size);
              if (archive_read_data(a, &entries_[type][0], size) == size) {
              }
            }
          }
        }
        is_loaded_ = true;
        result = true;
      }
      archive_read_close(a);
    //~ }
    archive_read_free(a);
  }
  return result;
}
#endif

#if defined(HAVE_LIBZIP)
bool TessdataManager::LoadZipFile(const char *filename) {
  bool result = false;
#ifndef NDEBUG
  tprintf("TessdataManager::%s(%s)\n", __func__, filename);
#endif
  std::string zipfile(filename);
  int err;
  zip_t *uf = zip_open(zipfile.c_str(), ZIP_RDONLY, &err);
  if (uf == nullptr) {
    zipfile += ".zip";
    uf = zip_open(zipfile.c_str(), ZIP_RDONLY, &err);
  }
  if (uf != nullptr) {
#ifndef NDEBUG
    tprintf("zip_open(%s) passed\n", zipfile.c_str());
#endif
    int64_t nEntries = zip_get_num_entries(uf, ZIP_FL_UNCHANGED);
    for (int i = 0; i < nEntries; i++) {
      zip_stat_t zipStat;
      if (zip_stat_index(uf, i, ZIP_FL_UNCHANGED, &zipStat) == 0 &&
          (zipStat.valid & ZIP_STAT_NAME) && (zipStat.valid & ZIP_STAT_SIZE)) {
#ifndef NDEBUG
        tprintf("zip_get_name(...) passed, file %s\n", zipStat.name);
#endif
        TessdataType type;
        if (TessdataTypeFromFileName(zipStat.name, &type)) {
#ifndef NDEBUG
          tprintf("TessdataTypeFromFileName(%s, ...) passed, type %d\n",
                  zipStat.name, type);
#endif
          zip_file_t *zipFile = zip_fopen_index(uf, i, ZIP_FL_UNCHANGED);
          if (zipFile == nullptr) {
#ifndef NDEBUG
            tprintf("zip_fopen_index(...) failed\n");
#endif
          } else {
            entries_[type].resize_no_init(zipStat.size);
            if (zip_fread(zipFile, &entries_[type][0], zipStat.size) !=
                static_cast<int64_t>(zipStat.size)) {
#ifndef NDEBUG
              tprintf("zip_fread(...) failed\n");
#endif
            }
            zip_fclose(zipFile);
          }
        }
      }
    }
    is_loaded_ = true;
    err = zip_close(uf);
    if (err != 0) {
#ifndef NDEBUG
      tprintf("zip_close(...) failed\n");
#endif
    }
    result = true;
  } else {
#ifndef NDEBUG
    tprintf("zip_open(%s) failed\n", zipfile.c_str());
#endif
  }
  return result;
}
#endif

#if defined(HAVE_MINIZIP)
bool TessdataManager::LoadMinizipFile(const char *filename) {
  bool result = false;
#ifndef NDEBUG
  tprintf("TessdataManager::%s(%s)\n", __func__, filename);
#endif
  std::string zipfile(filename);
  unzFile uf = unzOpen(zipfile.c_str());
  if (uf == nullptr) {
    zipfile += ".zip";
    uf = unzOpen(zipfile.c_str());
  }
  if (uf != nullptr) {
#ifndef NDEBUG
    tprintf("unzOpen(%s) passed\n", zipfile.c_str());
#endif
    unz_global_info global_info;
    int err;
    err = unzGetGlobalInfo(uf, &global_info);
    if (err == UNZ_OK) {
#ifndef NDEBUG
      tprintf("unzGetGlobalInfo(...) passed, zip file with %lu entries\n",
              global_info.number_entry);
#endif
    }
    unz_file_info file_info;
    char fileName[32];
    char extraField[32];
    char comment[32];
    //~ $1 = {version = 798, version_needed = 20, flag = 0, compression_method = 8, dosDate = 1252768343, crc = 2481269679, compressed_size = 7131663, uncompressed_size = 16109842,
    //~ size_filename = 15, size_file_extra = 24, size_file_comment = 0, disk_num_start = 0, internal_fa = 0, external_fa = 2175008768, tmu_date = {tm_sec = 46, tm_min = 18,
    //~ tm_hour = 23, tm_mday = 11, tm_mon = 4, tm_year = 2017}}
    for (unsigned i = 0; i < global_info.number_entry; i++) {
      err = unzGetCurrentFileInfo(uf, &file_info,
                                  fileName, sizeof(fileName),
                                  extraField, sizeof(extraField),
                                  comment, sizeof(comment));
      if (err == UNZ_OK) {
#ifndef NDEBUG
      tprintf("unzGetCurrentFileInfo(...) passed, file %s, %lu byte\n",
              fileName, file_info.uncompressed_size);
#endif
        TessdataType type;
        if (TessdataTypeFromFileName(fileName, &type)) {
#ifndef NDEBUG
          tprintf("TessdataTypeFromFileName(%s, ...) passed, type %d\n",
                  fileName, type);
#endif
          err = unzOpenCurrentFilePassword(uf, nullptr);
          if (err != UNZ_OK) {
#ifndef NDEBUG
            tprintf("unzOpenCurrentFilePassword(...) failed, err %d\n", err);
#endif
          } else {
            entries_[type].resize_no_init(file_info.uncompressed_size);
            err = unzReadCurrentFile(uf, &entries_[type][0], file_info.uncompressed_size);
            if (err < UNZ_OK) {
#ifndef NDEBUG
              tprintf("unzReadCurrentFile(...) failed, err %d\n", err);
#endif
            }
            err = unzCloseCurrentFile(uf);
            if (err != UNZ_OK) {
#ifndef NDEBUG
              tprintf("unzCloseCurrentFile(...) failed\n");
#endif
            }
          }
        }
      }
      //~ err = unzGoToFirstFile(uf);

      err = unzGoToNextFile(uf);
      if (err != UNZ_OK) {
#ifndef NDEBUG
        tprintf("unzGoToNextFile(...) failed\n");
#endif
      }
    }
    is_loaded_ = true;
    err = unzClose(uf);
    if (err != UNZ_OK) {
#ifndef NDEBUG
      tprintf("unzClose(...) failed\n");
#endif
    }
    result = true;
  } else {
#ifndef NDEBUG
    tprintf("unzOpen(%s) failed\n", zipfile.c_str());
    perror(zipfile.c_str());
#endif
  }
  return result;
}
#endif

#if defined(HAVE_ZZIPLIB)
bool TessdataManager::LoadZzipFile(const char *filename) {
  bool result = false;
#ifndef NDEBUG
  tprintf("TessdataManager::%s(%s)\n", __func__, filename);
#endif
  zzip_error_t err;
  ZZIP_DIR *dir = zzip_dir_open(filename, &err);
  if (dir != nullptr) {
    ZZIP_DIRENT d;
    while (zzip_dir_read(dir, &d)) {
      TessdataType type;
      if (TessdataTypeFromFileName(d.d_name, &type)) {
#ifndef NDEBUG
        tprintf("TessdataTypeFromFileName(%s, ...) passed, type %d\n",
                d.d_name, type);
#endif
        ZZIP_FILE *f = zzip_file_open(dir, d.d_name, 0);
        if (f != nullptr) {
          entries_[type].resize_no_init(d.st_size);
          ssize_t len = zzip_file_read(f, &entries_[type][0], d.st_size);
          if (len != d.st_size) {
#ifndef NDEBUG
            tprintf("zzip_file_read(...) failed\n");
#endif
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
#ifndef NDEBUG
  tprintf("TessdataManager::%s(%s)\n", __func__, data_file_name);
#endif
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
  // TODO: This method supports only the proprietary file format.
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
