/**********************************************************************
 * File:        lang_mod_edge.h
 * Description: Declaration of the Language Model Edge Base Class
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

// The LangModEdge abstracts an Edge in the language model trie
// This is an abstract class that any Language Model Edge should inherit from
// It provides methods for:
// 1- Returns the class ID corresponding to the edge
// 2- If the edge is a valid EndOfWord (EOW)
// 3- If the edge is coming from a OutOfDictionary (OOF) state machine
// 4- If the edge is a Terminal (has no children)
// 5- A Hash of the edge that will be used to retrieve the edge
// quickly from the BeamSearch lattice
// 6- If two edges are identcial
// 7- Returns a verbal description of the edge (use by debuggers)
// 8- the language model cost of the edge (if any)
// 9- The string corresponding to this edge
// 10- Getting and setting the "Root" status of the edge

#ifndef LANG_MOD_EDGE_H
#define LANG_MOD_EDGE_H

#include "cube_tuning_params.h"
#include "char_set.h"

namespace tesseract {

class LangModEdge {
 public:
   LangModEdge() {}
  virtual ~LangModEdge() {}

  // The string corresponding to this edge
  virtual const char_32 * EdgeString() const = 0;
  // Returns the class ID corresponding to the edge
  virtual int ClassID() const = 0;
  // If the edge is the root edge
  virtual bool IsRoot() const = 0;
  // Set the Root flag
  virtual void SetRoot(bool flag) = 0;
  // If the edge is a valid EndOfWord (EOW)
  virtual bool IsEOW() const = 0;
  // is the edge is coming from a OutOfDictionary (OOF) state machine
  virtual bool IsOOD() const = 0;
  // Is the edge is a Terminal (has no children)
  virtual bool IsTerminal() const = 0;
  // Returns A hash of the edge that will be used to retrieve the edge
  virtual unsigned int Hash() const = 0;
  // Are the two edges identcial?
  virtual bool IsIdentical(LangModEdge *edge) const = 0;
  // a verbal description of the edge (use by debuggers)
  virtual char *Description() const = 0;
  // the language model cost of the edge (if any)
  virtual int PathCost() const = 0;
};
}

#endif  // LANG_MOD_EDGE_H
