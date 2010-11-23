/**********************************************************************
 * File:        cached_file.h
 * Description: Declaration of a Cached File class
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

#ifndef CACHED_FILE_H
#define CACHED_FILE_H

// The CachedFile class provides a large-cache read access to a file
// It is mainly designed for loading large word dump files

#include <stdio.h>
#include <string>
#ifdef USE_STD_NAMESPACE
using std::string;
#endif

namespace tesseract {
class CachedFile {
 public:
  explicit CachedFile(string file_name);
  ~CachedFile();

  // reads a specified number of bytes to the specified buffer and
  // returns the actual number of bytes read
  int Read(void *read_buff, int bytes);
  // Returns the file size
  long Size();
  // returns the current position in the file
  long Tell();
  // End of file flag
  bool eof();

 private:
  static const unsigned int kCacheSize = 0x8000000;
  // file name
  string file_name_;
  // internal file buffer
  unsigned char *buff_;
  // file position
  long file_pos_;
  // file size
  long file_size_;
  // position of file within buffer
  int buff_pos_;
  // buffer size
  int buff_size_;
  // file handle
  FILE *fp_;
  // Opens the file
  bool Open();
};
}

#endif  // CACHED_FILE_H
