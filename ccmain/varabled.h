///////////////////////////////////////////////////////////////////////
// File:        varabled.cpp
// Description: Variables Editor
// Author:      Joern Wanke
// Created:     Wed Jul 18 10:05:01 PDT 2007
//
// (C) Copyright 2007, Google Inc.
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

/**
 * @file varabled.h
 * The variables editor is used to edit all the variables used within
 * tesseract from the ui.
 */
#ifndef GRAPHICS_DISABLED
#ifndef VARABLED_H
#define VARABLED_H

#include "elst.h"
#include "scrollview.h"
#include "varable.h"
#include "tesseractclass.h"

class SVMenuNode;

/** A list of all possible variable types used. */
enum VarType {
  VT_INTEGER,
  VT_BOOLEAN,
  VT_STRING,
  VT_DOUBLE
};

/**
 * A rather hackish helper structure which can take any kind of variable input
 * (defined by VarType) and do a couple of common operations on them, like
 * comparisond or getting its value. It is used in the context of the
 * VariablesEditor as a bridge from the internal tesseract variables to the
 * ones displayed by the ScrollView server.
 */
class VariableContent : public ELIST_LINK {
 public:
  /** Compare two VC objects by their name. */
  static int Compare(const void* v1, const void* v2);

  /** Gets a VC object identified by its ID. */
  static VariableContent* GetVariableContentById(int id);

  /** Constructors for the various VarTypes. */
  VariableContent() {
  }
  VariableContent(STRING_VARIABLE* it);
  VariableContent(INT_VARIABLE* it);
  VariableContent(BOOL_VARIABLE* it);
  VariableContent(double_VARIABLE* it);


  /** Getters and Setters. */
  void SetValue(const char* val);
  const char* GetValue() const;
  const char* GetName() const;
  const char* GetDescription() const;

  int GetId() { return my_id_; }
  bool HasChanged() { return changed_; }

 private:
  /** The unique ID of this VC object. */
  int my_id_;
  /** Whether the variable was changed_ and thus needs to be rewritten. */
  bool changed_;
  /** The actual vartype of this VC object. */
  VarType var_type_;

  STRING_VARIABLE* sIt;
  INT_VARIABLE* iIt;
  BOOL_VARIABLE* bIt;
  double_VARIABLE* dIt;
};

ELISTIZEH(VariableContent)

/**
 * The variables editor enables the user to edit all the variables used within
 * tesseract. It can be invoked on its own, but is supposed to be invoked by
 * the program editor.
 */
class VariablesEditor : public SVEventHandler {
 public:
  /**
   * Integrate the variables editor as popupmenu into the existing scrollview
   * window (usually the pg editor). If sv == null, create a new empty
   * empty window and attach the variables editor to that window (ugly).
   */
  VariablesEditor(const tesseract::Tesseract*, ScrollView* sv = NULL);

  /** Event listener. Waits for SVET_POPUP events and processes them. */
  void Notify(const SVEvent* sve);

 private:
  /**
   * Gets the up to the first 3 prefixes from s (split by _).
   * For example, tesseract_foo_bar will be split into tesseract, foo, and bar.
   */
  void GetPrefixes(const char* s, STRING* level_one,
                   STRING* level_two, STRING* level_three);

  /**
   * Gets the first n words (split by _) and puts them in t.
   * For example, tesseract_foo_bar with N=2 will yield tesseract_foo_.
   */
  void GetFirstWords(const char *s,  // source string
                     int n,          // number of words
                     char *t);       // target string

  /**
   * Find all editable variables used within tesseract and create a
   * SVMenuNode tree from it.
   */
  SVMenuNode *BuildListOfAllLeaves();

  /** Write all (changed_) variables to a config file. */
  void WriteVars(char* filename, bool changes_only);

  ScrollView* sv_window_;
};

#endif
#endif
