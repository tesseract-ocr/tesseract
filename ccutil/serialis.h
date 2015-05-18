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

#ifndef SERIALIS_H
#define SERIALIS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "host.h"

template <typename T> class GenericVector;
class STRING;

/***********************************************************************
  QUOTE_IT   MACRO DEFINITION
  ===========================
Replace <parm> with "<parm>".  <parm> may be an arbitrary number of tokens
***********************************************************************/

#define QUOTE_IT( parm ) #parm

namespace tesseract {

// Function to read a GenericVector<char> from a whole file.
// Returns false on failure.
typedef bool (*FileReader)(const STRING& filename, GenericVector<char>* data);
// Function to write a GenericVector<char> to a whole file.
// Returns false on failure.
typedef bool (*FileWriter)(const GenericVector<char>& data,
                           const STRING& filename);

// Simple file class.
// Allows for portable file input from memory and from foreign file systems.
class TFile {
 public:
  TFile();
  ~TFile();

  // All the Open methods load the whole file into memory for reading.
  // Opens a file with a supplied reader, or NULL to use the default.
  // Note that mixed read/write is not supported.
  bool Open(const STRING& filename, FileReader reader);
  // From an existing memory buffer.
  bool Open(const char* data, int size);
  // From an open file and an end offset.
  bool Open(FILE* fp, inT64 end_offset);

  // Reads a line like fgets. Returns NULL on EOF, otherwise buffer.
  // Reads at most buffer_size bytes, including '\0' terminator, even if
  // the line is longer. Does nothing if buffer_size <= 0.
  // To use fscanf use FGets and sscanf.
  char* FGets(char* buffer, int buffer_size);
  // Replicates fread, returning the number of items read.
  int FRead(void* buffer, int size, int count);
  // Resets the TFile as if it has been Opened, but nothing read.
  // Only allowed while reading!
  void Rewind();

  // Open for writing. Either supply a non-NULL data with OpenWrite before
  // calling FWrite, (no close required), or supply a NULL data to OpenWrite
  // and call CloseWrite to write to a file after the FWrites.
  void OpenWrite(GenericVector<char>* data);
  bool CloseWrite(const STRING& filename, FileWriter writer);

  // Replicates fwrite, returning the number of items written.
  // To use fprintf, use snprintf and FWrite.
  int FWrite(const void* buffer, int size, int count);

 private:
  // The number of bytes used so far.
  int offset_;
  // The buffered data from the file.
  GenericVector<char>* data_;
  // True if the data_ pointer is owned by *this.
  bool data_is_owned_;
  // True if the TFile is open for writing.
  bool is_writing_;
};

}  // namespace tesseract.

#endif
