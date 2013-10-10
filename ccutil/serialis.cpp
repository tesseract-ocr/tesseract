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

namespace tesseract {

TFile::TFile() : offset_(0) {
}

bool TFile::Open(const STRING& filename, FileReader reader) {
  offset_ = 0;
  if (reader == NULL)
    return LoadDataFromFile(filename, &data_);
  else
    return (*reader)(filename, &data_);
}

bool TFile::Open(const char* data, int size) {
  offset_ = 0;
  data_.init_to_size(size, 0);
  memcpy(&data_[0], data, size);
  return true;
}

bool TFile::Open(FILE* fp, inT64 end_offset) {
  offset_ = 0;
  inT64 current_pos = ftell(fp);
  if (end_offset < 0) {
    fseek(fp, 0, SEEK_END);
    end_offset = ftell(fp);
    fseek(fp, current_pos, SEEK_SET);
  }
  int size = end_offset - current_pos;
  data_.init_to_size(size, 0);
  return fread(&data_[0], 1, size, fp) == size;
}

char* TFile::FGets(char* buffer, int buffer_size) {
  int size = 0;
  while (size + 1 < buffer_size && offset_ < data_.size()) {
    buffer[size++] = data_[offset_++];
    if (data_[offset_ - 1] == '\n') break;
  }
  if (size < buffer_size) buffer[size] = '\0';
  return size > 0 ? buffer : NULL;
}

int TFile::FRead(void* buffer, int size, int count) {
  char* char_buffer = reinterpret_cast<char*>(buffer);
  int required_size = size * count;
  if (data_.size() - offset_ < required_size)
    required_size = data_.size() - offset_;
  memcpy(char_buffer, &data_[offset_], required_size);
  offset_ += required_size;
  return required_size / size;
}


}  // namespace tesseract.

