/**********************************************************************
 * File:        serialis.h  (Formerly serialmac.h)
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

#ifndef SERIALIS_H
#define SERIALIS_H

#include <tesseract/baseapi.h> // FileReader
#include <cstdint>  // uint8_t
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>   // std::vector

namespace tesseract {

/***********************************************************************
  QUOTE_IT   MACRO DEFINITION
  ===========================
Replace <parm> with "<parm>".  <parm> may be an arbitrary number of tokens
***********************************************************************/

#define QUOTE_IT(parm) #parm

// Return number of elements of an array.
template <typename T, size_t N>
constexpr size_t countof(T const (&)[N]) noexcept {
  return N;
}

// Function to write a std::vector<char> to a whole file.
// Returns false on failure.
using FileWriter = bool (*)(const std::vector<char>& data,
                            const char* filename);

TESS_API
bool LoadDataFromFile(const char* filename, std::vector<char>* data);
TESS_API
bool SaveDataToFile(const std::vector<char>& data, const char* filename);

// Deserialize data from file.
template <typename T>
bool DeSerialize(FILE *fp, T *data, size_t n = 1) {
  return fread(data, sizeof(T), n, fp) == n;
}

// Serialize data to file.
template <typename T>
bool Serialize(FILE *fp, const T *data, size_t n = 1) {
  return fwrite(data, sizeof(T), n, fp) == n;
}

// Simple file class.
// Allows for portable file input from memory and from foreign file systems.
class TESS_API TFile {
 public:
  TFile();
  ~TFile();

  // All the Open methods load the whole file into memory for reading.
  // Opens a file with a supplied reader, or nullptr to use the default.
  // Note that mixed read/write is not supported.
  bool Open(const char* filename, FileReader reader);
  // From an existing memory buffer.
  bool Open(const char* data, int size);
  // From an open file and an end offset.
  bool Open(FILE* fp, int64_t end_offset);
  // Sets the value of the swap flag, so that FReadEndian does the right thing.
  void set_swap(bool value) {
    swap_ = value;
  }

  // Deserialize data.
  bool DeSerialize(std::string& data);
  bool DeSerialize(std::vector<char>& data);
  template <typename T> bool DeSerialize(std::vector<T>& data);
  template <typename T>
  bool DeSerialize(T *data, size_t count = 1) {
      return FReadEndian(data, sizeof(T), count) == count;
  }

  // Serialize data.
  bool Serialize(const std::string& data);
  bool Serialize(const std::vector<char>& data);
  template <typename T> bool Serialize(const std::vector<T>& data);
  template <typename T>
  bool Serialize(const T *data, size_t count = 1) {
      return FWrite(data, sizeof(T), count) == count;
  }

  // Skip data.
  bool Skip(size_t count);

  // Reads a line like fgets. Returns nullptr on EOF, otherwise buffer.
  // Reads at most buffer_size bytes, including '\0' terminator, even if
  // the line is longer. Does nothing if buffer_size <= 0.
  char* FGets(char* buffer, int buffer_size);
  // Replicates fread, followed by a swap of the bytes if needed, returning the
  // number of items read. If swap_ is true then the count items will each have
  // size bytes reversed.
  int FReadEndian(void* buffer, size_t size, int count);
  // Replicates fread, returning the number of items read.
  int FRead(void* buffer, size_t size, int count);
  // Resets the TFile as if it has been Opened, but nothing read.
  // Only allowed while reading!
  void Rewind();

  // Open for writing. Either supply a non-nullptr data with OpenWrite before
  // calling FWrite, (no close required), or supply a nullptr data to OpenWrite
  // and call CloseWrite to write to a file after the FWrites.
  void OpenWrite(std::vector<char>* data);
  bool CloseWrite(const char* filename, FileWriter writer);

  // Replicates fwrite, returning the number of items written.
  // To use fprintf, use snprintf and FWrite.
  int FWrite(const void* buffer, size_t size, int count);

 private:
  // The buffered data from the file.
  std::vector<char>* data_;
  // The number of bytes used so far.
  int offset_;
  // True if the data_ pointer is owned by *this.
  bool data_is_owned_;
  // True if the TFile is open for writing.
  bool is_writing_;
  // True if bytes need to be swapped in FReadEndian.
  bool swap_;
};

}  // namespace tesseract.

#endif
