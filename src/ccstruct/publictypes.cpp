///////////////////////////////////////////////////////////////////////
// File:        publictypes.cpp
// Description: Types used in both the API and internally
// Author:      Ray Smith
// Created:     Wed Mar 03 11:17:09 PST 2010
//
// (C) Copyright 2010, Google Inc.
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

#include "publictypes.h"

/** String name for each block type. Keep in sync with PolyBlockType. */
const char* kPolyBlockNames[] = {
  "Unknown",
  "Flowing Text",
  "Heading Text",
  "Pullout Text",
  "Equation",
  "Inline Equation",
  "Table",
  "Vertical Text",
  "Caption Text",
  "Flowing Image",
  "Heading Image",
  "Pullout Image",
  "Horizontal Line",
  "Vertical Line",
  "Noise",
  ""  // End marker for testing that sizes match.
};
