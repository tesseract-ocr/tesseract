///////////////////////////////////////////////////////////////////////
// File:        unicharmap.cpp
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

#include <cassert>
#include "unichar.h"
#include "host.h"
#include "unicharmap.h"

UNICHARMAP::UNICHARMAP() :
nodes(nullptr) {
}

UNICHARMAP::~UNICHARMAP() {
  delete[] nodes;
}

// Search the given unichar representation in the tree, using length characters
// from it maximum. Each character in the string is interpreted as an index in
// an array of nodes.
UNICHAR_ID UNICHARMAP::unichar_to_id(const char* const unichar_repr,
                                     int length) const {
  UNICHARMAP_NODE* current_nodes = nodes;

  assert(*unichar_repr != '\0');
  assert(length > 0 && length <= UNICHAR_LEN);

  int index = 0;
  if (index >= length || unichar_repr[index] == '\0') return INVALID_UNICHAR_ID;
  do {
    if (index + 1 >= length || unichar_repr[index + 1] == '\0')
      return current_nodes[static_cast<unsigned char>(unichar_repr[index])].id;
    current_nodes =
        current_nodes[static_cast<unsigned char>(unichar_repr[index])].children;
    ++index;
  } while (true);
}

// Search the given unichar representation in the tree, creating the possibly
// missing nodes. Once the right place has been found, insert the given id and
// update the inserted flag to keep track of the insert. Each character in the
// string is interpreted as an index in an array of nodes.
void UNICHARMAP::insert(const char* const unichar_repr, UNICHAR_ID id) {
  const char* current_char = unichar_repr;
  if (*current_char == '\0') return;
  UNICHARMAP_NODE** current_nodes_pointer = &nodes;
  do {
    if (*current_nodes_pointer == nullptr)
      *current_nodes_pointer = new UNICHARMAP_NODE[256];
    if (current_char[1] == '\0') {
      (*current_nodes_pointer)
          [static_cast<unsigned char>(*current_char)].id = id;
      return;
    }
    current_nodes_pointer =
        &((*current_nodes_pointer)
          [static_cast<unsigned char>(*current_char)].children);
    ++current_char;
  } while (true);
}

// Search the given unichar representation in the tree, using length characters
// from it maximum. Each character in the string is interpreted as an index in
// an array of nodes. Stop once the tree does not have anymore nodes or once we
// found the right unichar_repr.
bool UNICHARMAP::contains(const char* const unichar_repr,
                          int length) const {
  if (unichar_repr == nullptr || *unichar_repr == '\0') return false;
  if (length <= 0 || length > UNICHAR_LEN) return false;
  int index = 0;
  if (unichar_repr[index] == '\0') return false;
  UNICHARMAP_NODE* current_nodes = nodes;

  while (current_nodes != nullptr && index + 1 < length &&
         unichar_repr[index + 1] != '\0') {
    current_nodes =
        current_nodes[static_cast<unsigned char>(unichar_repr[index])].children;
    ++index;
  }
  return current_nodes != nullptr &&
         (index + 1 >= length || unichar_repr[index + 1] == '\0') &&
         current_nodes[static_cast<unsigned char>(unichar_repr[index])].id >= 0;
}

// Return the minimum number of characters that must be used from this string
// to obtain a match in the UNICHARMAP.
int UNICHARMAP::minmatch(const char* const unichar_repr) const {
  const char* current_char = unichar_repr;
  if (*current_char == '\0') return 0;
  UNICHARMAP_NODE* current_nodes = nodes;

  while (current_nodes != nullptr && *current_char != '\0') {
    if (current_nodes[static_cast<unsigned char>(*current_char)].id >= 0)
      return current_char + 1 - unichar_repr;
    current_nodes =
        current_nodes[static_cast<unsigned char>(*current_char)].children;
    ++current_char;
  }
  return 0;
}

void UNICHARMAP::clear() {
  delete[] nodes;
  nodes = nullptr;
}

UNICHARMAP::UNICHARMAP_NODE::UNICHARMAP_NODE() :
children(nullptr),
id(-1) {
}

// Recursively delete the children
UNICHARMAP::UNICHARMAP_NODE::~UNICHARMAP_NODE() {
  delete[] children;
}
