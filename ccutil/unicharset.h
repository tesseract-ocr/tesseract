///////////////////////////////////////////////////////////////////////
// File:        unicharset.h
// Description: Unicode character/ligature set class.
// Author:      Thomas Kielbus
// Created:     Wed Jun 28 17:05:01 PDT 2006
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__
#define THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__

#include "unichar.h"
#include "unicharmap.h"

// The UNICHARSET class is an utility class for Tesseract that holds the
// set of characters that are used by the engine. Each character is identified
// by a unique number, from 0 to (size - 1).
class UNICHARSET {
 public:

  // Create an empty UNICHARSET
  UNICHARSET();

  ~UNICHARSET();

  // Return the UNICHAR_ID of a given unichar representation within the
  // UNICHARSET.
  const UNICHAR_ID unichar_to_id(const char* const unichar_repr) const;

  // Return the UNICHAR_ID of a given unichar representation within the
  // UNICHARSET. Only the first length characters from unichar_repr are used.
  const UNICHAR_ID unichar_to_id(const char* const unichar_repr,
                                 int length) const;

  // Return the unichar representation corresponding to the given UNICHAR_ID
  // within the UNICHARSET.
  const char* const id_to_unichar(UNICHAR_ID id) const;

  // Add a unichar representation to the set.
  void unichar_insert(const char* const unichar_repr);

  // Return true if the given unichar representation exists within the set.
  bool contains_unichar(const char* const unichar_repr);

  // Return true if the given unichar representation corresponds to the given
  // UNICHAR_ID within the set.
  bool eq(UNICHAR_ID unichar_id, const char* const unichar_repr);

  // Clear the UNICHARSET (all the previous data is lost).
  void clear() {
    if (size_reserved > 0) {
      delete[] unichars;
      unichars = 0;
      size_reserved = 0;
      size_used = 0;
    }
    ids.clear();
  }

  // Return the size of the set (the number of different UNICHAR it holds).
  int size() const {
    return size_used;
  }

  // Reserve enough memory space for the given number of UNICHARS
  void reserve(int unichars_number);

  // Save the content of the UNICHARSET to the given file. Return true if the
  // operation is successful.
  bool save_to_file(const char* const filename) const;

  // Load the UNICHARSET from the given file. The previous data is lost. Return
  // true if the operation is successful.
  bool load_from_file(const char* const filename);

 private:

  typedef char UNICHAR_SLOT[UNICHAR_LEN + 1];

  UNICHAR_SLOT* unichars;
  UNICHARMAP ids;
  int size_used;
  int size_reserved;
};

#endif  // THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__
