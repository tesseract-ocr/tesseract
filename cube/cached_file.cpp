/**********************************************************************
 * File:        cached_file.pp
 * Description: Implementation of an Cached File Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

#include <string>
#include <stdlib.h>
#include <cstring>
#include "cached_file.h"

namespace tesseract {

CachedFile::CachedFile(string file_name) {
  file_name_ = file_name;
  buff_ = NULL;
  buff_pos_ = 0;
  buff_size_ = 0;
  file_pos_ = 0;
  file_size_ = 0;
  fp_ = NULL;
}

CachedFile::~CachedFile() {
  if (fp_ != NULL) {
    fclose(fp_);
    fp_ = NULL;
  }

  if (buff_ != NULL) {
    delete []buff_;
    buff_ = NULL;
  }
}

// free buffers and init vars
bool CachedFile::Open() {
  if (fp_ != NULL) {
    return true;
  }

  fp_ = fopen(file_name_.c_str(), "rb");
  if (fp_ == NULL) {
    return false;
  }

  // seek to the end
  fseek(fp_, 0, SEEK_END);
  // get file size
  file_size_ = ftell(fp_);
  if (file_size_ < 1) {
    return false;
  }
  // rewind again
  rewind(fp_);
  // alloc memory for buffer
  buff_ = new unsigned char[kCacheSize];
  // init counters
  buff_size_ = 0;
  buff_pos_ = 0;
  file_pos_ = 0;
  return true;
}

// add a new sample
int CachedFile::Read(void *read_buff, int bytes) {
  int read_bytes = 0;
  unsigned char *buff = (unsigned char *)read_buff;

  // do we need to read beyond the buffer
  if ((buff_pos_ + bytes) > buff_size_) {
    // copy as much bytes from the current buffer if any
    int copy_bytes = buff_size_ - buff_pos_;

    if (copy_bytes > 0) {
      memcpy(buff, buff_ + buff_pos_, copy_bytes);
      buff += copy_bytes;
      bytes -= copy_bytes;
      read_bytes += copy_bytes;
    }

    // determine how much to read
    buff_size_ = kCacheSize;

    if ((file_pos_ + buff_size_) > file_size_) {
      buff_size_ = static_cast<int>(file_size_ - file_pos_);
    }

    // EOF ?
    if (buff_size_ <= 0 || bytes > buff_size_) {
      return read_bytes;
    }

    // read the first chunck
    if (fread(buff_, 1, buff_size_, fp_) != buff_size_) {
      return read_bytes;
    }

    buff_pos_ = 0;
    file_pos_ += buff_size_;
  }

  memcpy(buff, buff_ + buff_pos_, bytes);
  read_bytes += bytes;
  buff_pos_ += bytes;

  return read_bytes;
}

long CachedFile::Size() {
  if (fp_ == NULL && Open() == false) {
    return 0;
  }

  return file_size_;
}

long CachedFile::Tell() {
  if (fp_ == NULL && Open() == false) {
    return 0;
  }

  return file_pos_ - buff_size_ + buff_pos_;
}

bool CachedFile::eof() {
  if (fp_ == NULL && Open() == false) {
    return true;
  }

  return (file_pos_ - buff_size_ + buff_pos_) >= file_size_;
}

}  // namespace tesseract
