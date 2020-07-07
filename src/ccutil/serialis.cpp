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

#include <tesseract/serialis.h>
#include <cstdio>
#include "errcode.h"
#include <tesseract/genericvector.h>

namespace tesseract {

bool DeSerialize(FILE* fp, char* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, float* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, int8_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, int16_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, int32_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, uint8_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, uint16_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool DeSerialize(FILE* fp, uint32_t* data, size_t n) {
  return fread(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const char* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const float* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const int8_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const int16_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const int32_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const uint8_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const uint16_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

bool Serialize(FILE* fp, const uint32_t* data, size_t n) {
  return fwrite(data, sizeof(*data), n, fp) == n;
}

TFile::TFile()
    : data_(nullptr),
      offset_(0),
      data_is_owned_(false),
      is_writing_(false),
      swap_(false) {}

TFile::~TFile() {
  if (data_is_owned_)
    delete data_;
}

bool TFile::DeSerialize(char* buffer, size_t count) {
  return FRead(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(double* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(float* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(int8_t* buffer, size_t count) {
  return FRead(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(int16_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(int32_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(int64_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(uint8_t* buffer, size_t count) {
  return FRead(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(uint16_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(uint32_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::DeSerialize(uint64_t* buffer, size_t count) {
  return FReadEndian(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const char* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const double* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const float* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const int8_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const int16_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const int32_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const int64_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const uint8_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const uint16_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const uint32_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Serialize(const uint64_t* buffer, size_t count) {
  return FWrite(buffer, sizeof(*buffer), count) == count;
}

bool TFile::Skip(size_t count) {
  offset_ += count;
  return true;
}

bool TFile::Open(const char* filename, FileReader reader) {
  if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  offset_ = 0;
  is_writing_ = false;
  swap_ = false;
  if (reader == nullptr)
    return LoadDataFromFile(filename, data_);
  else
    return (*reader)(filename, data_);
}

bool TFile::Open(const char* data, int size) {
  offset_ = 0;
  if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  is_writing_ = false;
  swap_ = false;
  data_->resize_no_init(size);
  memcpy(&(*data_)[0], data, size);
  return true;
}

bool TFile::Open(FILE* fp, int64_t end_offset) {
  offset_ = 0;
  auto current_pos = std::ftell(fp);
  if (current_pos < 0) {
    // ftell failed.
    return false;
  }
  if (end_offset < 0) {
    if (fseek(fp, 0, SEEK_END))
      return false;
    end_offset = ftell(fp);
    if (fseek(fp, current_pos, SEEK_SET))
      return false;
  }
  int size = end_offset - current_pos;
  is_writing_ = false;
  swap_ = false;
  if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  data_->resize_no_init(size);
  return static_cast<int>(fread(&(*data_)[0], 1, size, fp)) == size;
}

char* TFile::FGets(char* buffer, int buffer_size) {
  ASSERT_HOST(!is_writing_);
  int size = 0;
  while (size + 1 < buffer_size && offset_ < data_->size()) {
    buffer[size++] = (*data_)[offset_++];
    if ((*data_)[offset_ - 1] == '\n') break;
  }
  if (size < buffer_size) buffer[size] = '\0';
  return size > 0 ? buffer : nullptr;
}

int TFile::FReadEndian(void* buffer, size_t size, int count) {
  int num_read = FRead(buffer, size, count);
  if (swap_) {
    char* char_buffer = static_cast<char*>(buffer);
    for (int i = 0; i < num_read; ++i, char_buffer += size) {
      ReverseN(char_buffer, size);
    }
  }
  return num_read;
}

int TFile::FRead(void* buffer, size_t size, int count) {
  ASSERT_HOST(!is_writing_);
  ASSERT_HOST(size > 0);
  ASSERT_HOST(count >= 0);
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
  if (required_size > 0 && buffer != nullptr)
    memcpy(buffer, &(*data_)[offset_], required_size);
  offset_ += required_size;
  return required_size / size;
}

void TFile::Rewind() {
  ASSERT_HOST(!is_writing_);
  offset_ = 0;
}

void TFile::OpenWrite(GenericVector<char>* data) {
  offset_ = 0;
  if (data != nullptr) {
    if (data_is_owned_) delete data_;
    data_ = data;
    data_is_owned_ = false;
  } else if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  is_writing_ = true;
  swap_ = false;
  data_->truncate(0);
}

bool TFile::CloseWrite(const char* filename, FileWriter writer) {
  ASSERT_HOST(is_writing_);
  if (writer == nullptr)
    return SaveDataToFile(*data_, filename);
  else
    return (*writer)(*data_, filename);
}

int TFile::FWrite(const void* buffer, size_t size, int count) {
  ASSERT_HOST(is_writing_);
  ASSERT_HOST(size > 0);
  ASSERT_HOST(count >= 0);
  ASSERT_HOST(SIZE_MAX / size > count);
  size_t total = size * count;
  const char* buf = static_cast<const char*>(buffer);
  // This isn't very efficient, but memory is so fast compared to disk
  // that it is relatively unimportant, and very simple.
  for (size_t i = 0; i < total; ++i)
    data_->push_back(buf[i]);
  return count;
}

}  // namespace tesseract.
