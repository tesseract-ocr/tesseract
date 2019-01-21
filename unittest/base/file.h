// Copyright 2010-2012 Google
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_BASE_FILE_H_
#define OR_TOOLS_BASE_FILE_H_

#include <cstdlib>
#include <cstdio>
#include <string>

#include "base/integral_types.h"

// This file defines some IO interfaces to compatible with Google
// IO specifications.
namespace operations_research {
class File {
 public:
  // Opens file "name" with flags specified by "flag".
  // Flags are defined by fopen(), that is "r", "r+", "w", "w+". "a", and "a+".
  static File* Open(const char* const name, const char* const flag);

#ifndef SWIG  // no overloading
  inline static File* Open(const std::string& name, const char* const mode) {
    return Open(name.c_str(), mode);
  }
#endif

  // Opens file "name" with flags specified by "flag"
  // If open failed, program will exit.
  static File* OpenOrDie(const char* const name, const char* const  flag);

  // Reads "size" bytes to buff from file, buff should be pre-allocated.
  size_t Read(void* const buff, size_t size);

  // Reads "size" bytes to buff from file, buff should be pre-allocated.
  // If read failed, program will exit.
  void ReadOrDie(void* const buff, size_t size);

  // Reads a line from file to a string.
  // Each line must be no more than max_length bytes
  char* ReadLine(char* const output, uint64 max_length);

  // Reads the whole file to a string, with a maximum length of 'max_length'.
  // Returns the number of bytes read.
  int64 ReadToString(std::string* const line, uint64 max_length);

  // Writes "size" bytes of buff to file, buff should be pre-allocated.
  size_t Write(const void* const buff, size_t size);

  // Writes "size" bytes of buff to file, buff should be pre-allocated.
  // If write failed, program will exit.
  void WriteOrDie(const void* const buff, size_t size);

  // Writse a string to file.
  size_t WriteString(const std::string& line);

  // Writes a string to file and append a "\n".
  bool WriteLine(const std::string& line);

  // Closes the file.
  bool Close();

  // Flushes buffer.
  bool Flush();

  // Returns file size.
  size_t Size();

  // Inits internal data structures.
  static void Init();

  // Returns the file name.
  std::string CreateFileName() const;

  // Deletes a file.
  static bool Delete(char* const name);

  // Tests if a file exists.
  static bool Exists(char* const name);

  bool Open() const;

 private:
  File(FILE* const descriptor, const std::string& name);

  FILE* f_;
  const std::string name_;
};
}  // namespace operations_research

#endif  // OR_TOOLS_BASE_FILE_H_