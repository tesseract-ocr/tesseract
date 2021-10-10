/**********************************************************************
 * File:        serialis.cpp  (Formerly serialmac.h)
 * Description: Inline routines and macros for serialisation functions
 * Author:      Phil Cheatle
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "serialis.h"

#include "errcode.h"

#include "helpers.h" // for ReverseN

#include <climits> // for INT_MAX
#include <cstdio>

namespace tesseract {

// The default FileReader loads the whole file into the vector of char,
// returning false on error.
bool LoadDataFromFile(const char *filename, std::vector<char> *data) {
  bool result = false;
  FILE *fp = fopen(filename, "rb");
  if (fp != nullptr) {
    fseek(fp, 0, SEEK_END);
    auto size = std::ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // Trying to open a directory on Linux sets size to LONG_MAX. Catch it here.
    if (size > 0 && size < LONG_MAX) {
      // reserve an extra byte in case caller wants to append a '\0' character
      data->reserve(size + 1);
      data->resize(size); // TODO: optimize no init
      result = static_cast<long>(fread(&(*data)[0], 1, size, fp)) == size;
    }
    fclose(fp);
  }
  return result;
}

// The default FileWriter writes the vector of char to the filename file,
// returning false on error.
bool SaveDataToFile(const std::vector<char> &data, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (fp == nullptr) {
    return false;
  }
  bool result = fwrite(&data[0], 1, data.size(), fp) == data.size();
  fclose(fp);
  return result;
}

TFile::TFile() {
}

TFile::~TFile() {
  if (data_is_owned_) {
    delete data_;
  }
}

bool TFile::DeSerializeSize(int32_t *pSize) {
  uint32_t size;
  if (FReadEndian(&size, sizeof(size), 1) != 1) {
    return false;
  }
  if (size > data_->size() / 4) {
    // Reverse endianness.
    swap_ = !swap_;
    ReverseN(&size, 4);
  }
  *pSize = size;
  return true;
}

bool TFile::DeSerializeSkip(size_t size) {
  uint32_t len;
  if (!DeSerialize(&len)) {
    return false;
  }
  return Skip(len * size);
}

bool TFile::DeSerialize(std::string &data) {
  uint32_t size;
  if (!DeSerialize(&size)) {
    return false;
  } else if (size > 0) {
    // TODO: optimize.
    data.resize(size);
    return DeSerialize(&data[0], size);
  }
  data.clear();
  return true;
}

bool TFile::Serialize(const std::string &data) {
  uint32_t size = data.size();
  return Serialize(&size) && Serialize(data.c_str(), size);
}

bool TFile::DeSerialize(std::vector<char> &data) {
  uint32_t size;
  if (!DeSerialize(&size)) {
    return false;
  } else if (size > 0) {
    // TODO: optimize.
    data.resize(size);
    return DeSerialize(&data[0], data.size());
  }
  data.clear();
  return true;
}

bool TFile::Serialize(const std::vector<char> &data) {
  uint32_t size = data.size();
  if (!Serialize(&size)) {
    return false;
  } else if (size > 0) {
    return Serialize(&data[0], size);
  }
  return true;
}

bool TFile::Skip(size_t count) {
  offset_ += count;
  return true;
}

bool TFile::Open(const char *filename, FileReader reader) {
  if (!data_is_owned_) {
    data_ = new std::vector<char>;
    data_is_owned_ = true;
  }
  offset_ = 0;
  is_writing_ = false;
  swap_ = false;
  if (reader == nullptr) {
    return LoadDataFromFile(filename, data_);
  } else {
    return (*reader)(filename, data_);
  }
}

bool TFile::Open(const char *data, size_t size) {
  offset_ = 0;
  if (!data_is_owned_) {
    data_ = new std::vector<char>;
    data_is_owned_ = true;
  }
  is_writing_ = false;
  swap_ = false;
  data_->resize(size); // TODO: optimize no init
  memcpy(&(*data_)[0], data, size);
  return true;
}

bool TFile::Open(FILE *fp, int64_t end_offset) {
  offset_ = 0;
  auto current_pos = std::ftell(fp);
  if (current_pos < 0) {
    // ftell failed.
    return false;
  }
  if (end_offset < 0) {
    if (fseek(fp, 0, SEEK_END)) {
      return false;
    }
    end_offset = ftell(fp);
    if (fseek(fp, current_pos, SEEK_SET)) {
      return false;
    }
  }
  size_t size = end_offset - current_pos;
  is_writing_ = false;
  swap_ = false;
  if (!data_is_owned_) {
    data_ = new std::vector<char>;
    data_is_owned_ = true;
  }
  data_->resize(size); // TODO: optimize no init
  return fread(&(*data_)[0], 1, size, fp) == size;
}

char *TFile::FGets(char *buffer, int buffer_size) {
  ASSERT_HOST(!is_writing_);
  int size = 0;
  while (size + 1 < buffer_size && offset_ < data_->size()) {
    buffer[size++] = (*data_)[offset_++];
    if ((*data_)[offset_ - 1] == '\n') {
      break;
    }
  }
  if (size < buffer_size) {
    buffer[size] = '\0';
  }
  return size > 0 ? buffer : nullptr;
}

size_t TFile::FReadEndian(void *buffer, size_t size, size_t count) {
  auto num_read = FRead(buffer, size, count);
  if (swap_ && size != 1) {
    char *char_buffer = static_cast<char *>(buffer);
    for (size_t i = 0; i < num_read; ++i, char_buffer += size) {
      ReverseN(char_buffer, size);
    }
  }
  return num_read;
}

size_t TFile::FRead(void *buffer, size_t size, size_t count) {
  ASSERT_HOST(!is_writing_);
  ASSERT_HOST(size > 0);
  size_t required_size;
  if (SIZE_MAX / size <= count) {
    // Avoid integer overflow.
    required_size = data_->size() - offset_;
  } else {
    required_size = size * count;
    if (data_->size() - offset_ < required_size) {
      required_size = data_->size() - offset_;
    }
  }
  if (required_size > 0 && buffer != nullptr) {
    memcpy(buffer, &(*data_)[offset_], required_size);
  }
  offset_ += required_size;
  return required_size / size;
}

void TFile::Rewind() {
  ASSERT_HOST(!is_writing_);
  offset_ = 0;
}

void TFile::OpenWrite(std::vector<char> *data) {
  offset_ = 0;
  if (data != nullptr) {
    if (data_is_owned_) {
      delete data_;
    }
    data_ = data;
    data_is_owned_ = false;
  } else if (!data_is_owned_) {
    data_ = new std::vector<char>;
    data_is_owned_ = true;
  }
  is_writing_ = true;
  swap_ = false;
  data_->clear();
}

bool TFile::CloseWrite(const char *filename, FileWriter writer) {
  ASSERT_HOST(is_writing_);
  if (writer == nullptr) {
    return SaveDataToFile(*data_, filename);
  } else {
    return (*writer)(*data_, filename);
  }
}

size_t TFile::FWrite(const void *buffer, size_t size, size_t count) {
  ASSERT_HOST(is_writing_);
  ASSERT_HOST(size > 0);
  ASSERT_HOST(SIZE_MAX / size > count);
  size_t total = size * count;
  const char *buf = static_cast<const char *>(buffer);
  // This isn't very efficient, but memory is so fast compared to disk
  // that it is relatively unimportant, and very simple.
  for (size_t i = 0; i < total; ++i) {
    data_->push_back(buf[i]);
  }
  return count;
}

} // namespace tesseract.
