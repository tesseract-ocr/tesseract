/**********************************************************************
 * File:        fileio.h
 * Description: File I/O utilities.
 * Author:      Samuel Charron
 * Created:     Tuesday, July 9, 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required
 * by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
 * OF ANY KIND, either express or implied.  See the License for the specific
 * language governing permissions and limitations under the License.
 *
 **********************************************************************/
#ifndef TESSERACT_TRAINING_FILEIO_H_
#define TESSERACT_TRAINING_FILEIO_H_

#include <stddef.h>
#include <cstdio>
#include <string>

#ifdef USE_STD_NAMESPACE
using std::string;
#endif

namespace tesseract {

// A class to manipulate FILE*s.
class File {
 public:
  // Try to open the file 'filename' in mode 'mode'.
  // Stop the program if it cannot open it.
  static FILE* OpenOrDie(const string& filename, const string& mode);
  static FILE* Open(const string& filename, const string& mode);

  // Try to open the file 'filename' and to write 'str' in it.
  // Stop the program if it fails.
  static void WriteStringToFileOrDie(const string& str, const string& filename);

  // Return true if the file 'filename' is readable.
  static bool Readable(const string& filename);

  static void ReadFileToStringOrDie(const string& filename, string* out);
  static bool ReadFileToString(const string& filename, string* out);

  // Helper methods

  // Concatenate file paths removing any extra intervening '/' symbols.
  static string JoinPath(const string& prefix, const string& suffix);
  // Delete a filename or all filenames matching a glob pattern.
  static bool Delete(const char* pathname);
  static bool DeleteMatchingFiles(const char* pattern);
};

// A class to manipulate Files for reading.
class InputBuffer {
 public:
  explicit InputBuffer(FILE* stream);
  // 'size' is ignored.
  InputBuffer(FILE* stream, size_t size);

  ~InputBuffer();

  // Read data until end-of-file.
  // The data is stored in '*out'.
  // Return false if an error occurs, true otherwise.
  bool Read(string* out);

  // Close the FILE* used by InputBuffer.
  // Return false if an error occurs, true otherwise.
  bool CloseFile();

 private:
  FILE* stream_;
  int   filesize_;
};

// A class to manipulate Files for writing.
class OutputBuffer {
 public:
  explicit OutputBuffer(FILE* stream);
  // 'size' is ignored.
  OutputBuffer(FILE* stream, size_t size);

  ~OutputBuffer();

  // Write string 'str' to the open FILE*.
  void WriteString(const string& str);

  // Close the FILE* used by InputBuffer.
  // Return false if an error occurs, true otherwise.
  bool CloseFile();

 private:
  FILE* stream_;
};

}  // namespace tesseract
#endif  // TESSERACT_TRAINING_FILEIO_H_
