///////////////////////////////////////////////////////////////////////
// File:        unicharmap.h
// Description: Unicode character/ligature to integer id class.
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

#ifndef TESSERACT_CCUTIL_UNICHARMAP_H_
#define TESSERACT_CCUTIL_UNICHARMAP_H_

#include <tesseract/unichar.h>

namespace tesseract {

// A UNICHARMAP stores unique unichars. Each of them is associated with one
// UNICHAR_ID.
class TESS_API UNICHARMAP {
public:
  // Create an empty UNICHARMAP
  UNICHARMAP();

  ~UNICHARMAP();

  // Insert the given unichar representation in the UNICHARMAP and associate it
  // with the given id. The length of the representation MUST be non-zero.
  void insert(const char *const unichar_repr, UNICHAR_ID id);

  // Return the id associated with the given unichar representation,
  // this representation MUST exist within the UNICHARMAP. The first
  // length characters (maximum) from unichar_repr are used. The length
  // MUST be non-zero.
  UNICHAR_ID unichar_to_id(const char *const unichar_repr, int length) const;

  // Return true if the given unichar representation is already present in the
  // UNICHARMAP. The first length characters (maximum) from unichar_repr are
  // used. The length MUST be non-zero.
  bool contains(const char *const unichar_repr, int length) const;

  // Return the minimum number of characters that must be used from this string
  // to obtain a match in the UNICHARMAP.
  int minmatch(const char *const unichar_repr) const;

  // Clear the UNICHARMAP. All previous data is lost.
  void clear();

private:
  // The UNICHARMAP is represented as a tree whose nodes are of type
  // UNICHARMAP_NODE.
  struct UNICHARMAP_NODE {
    UNICHARMAP_NODE();
    ~UNICHARMAP_NODE();

    UNICHARMAP_NODE *children;
    UNICHAR_ID id;
  };

  UNICHARMAP_NODE *nodes;
};

} // namespace tesseract

#endif // TESSERACT_CCUTIL_UNICHARMAP_H_
