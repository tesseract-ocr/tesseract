/**********************************************************************
 * File:        serialis.h  (Formerly serialmac.h)
 * Description: Inline routines and macros for serialisation functions
 * Author:      Phil Cheatle
 * Created:     Tue Oct 08 08:33:12 BST 1991
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
#include <stdio.h>
#include "genericvector.h"

namespace tesseract {

TFile::TFile()
    : offset_(0), data_(NULL), data_is_owned_(false), is_writing_(false) {
}

TFile::~TFile() {
  if (data_is_owned_)
    delete data_;
}

bool TFile::Open(const STRING& filename, FileReader reader) {
  if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  offset_ = 0;
  is_writing_ = false;
  if (reader == NULL)
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
  data_->resize_no_init(size);
  memcpy(&(*data_)[0], data, size);
  return true;
}

bool TFile::Open(FILE* fp, inT64 end_offset) {
  offset_ = 0;
  inT64 current_pos = ftell(fp);
  if (end_offset < 0) {
    if (fseek(fp, 0, SEEK_END))
      return false;
    end_offset = ftell(fp);
    if (fseek(fp, current_pos, SEEK_SET))
      return false;
  }
  int size = end_offset - current_pos;
  is_writing_ = false;
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
  return size > 0 ? buffer : NULL;
}

int TFile::FRead(void* buffer, int size, int count) {
  ASSERT_HOST(!is_writing_);
  int required_size = size * count;
  if (required_size <= 0) return 0;
  char* char_buffer = reinterpret_cast<char*>(buffer);
  if (data_->size() - offset_ < required_size)
    required_size = data_->size() - offset_;
  if (required_size > 0 && char_buffer != NULL)
    memcpy(char_buffer, &(*data_)[offset_], required_size);
  offset_ += required_size;
  return required_size / size;
}

void TFile::Rewind() {
  ASSERT_HOST(!is_writing_);
  offset_ = 0;
}

void TFile::OpenWrite(GenericVector<char>* data) {
  offset_ = 0;
  if (data != NULL) {
    if (data_is_owned_) delete data_;
    data_ = data;
    data_is_owned_ = false;
  } else if (!data_is_owned_) {
    data_ = new GenericVector<char>;
    data_is_owned_ = true;
  }
  is_writing_ = true;
  data_->truncate(0);
}

bool TFile::CloseWrite(const STRING& filename, FileWriter writer) {
  ASSERT_HOST(is_writing_);
  if (writer == NULL)
    return SaveDataToFile(*data_, filename);
  else
    return (*writer)(*data_, filename);
}

int TFile::FWrite(const void* buffer, int size, int count) {
  ASSERT_HOST(is_writing_);
  int total = size * count;
  if (total <= 0) return 0;
  const char* buf = reinterpret_cast<const char*>(buffer);
  // This isn't very efficient, but memory is so fast compared to disk
  // that it is relatively unimportant, and very simple.
  for (int i = 0; i < total; ++i)
    data_->push_back(buf[i]);
  return count;
}


}  // namespace tesseract.

