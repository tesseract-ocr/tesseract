///////////////////////////////////////////////////////////////////////
// File:        paramsd.cpp
// Description: Tesseract parameter Editor
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
//
// The parameters editor is used to edit all the parameters used within
// tesseract from the ui.
#ifdef _WIN32
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include <map>

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED
#include "paramsd.h"


#include "params.h"
#include "scrollview.h"
#include "svmnode.h"


#define VARDIR        "configs/" /*parameters files */
#define MAX_ITEMS_IN_SUBMENU 30

// The following variables should remain static globals, since they
// are used by debug editor, which uses a single Tesseract instance.
//
// Contains the mappings from unique VC ids to their actual pointers.
static std::map<int, ParamContent*> vcMap;
static int nrParams = 0;
static int writeCommands[2];

ELISTIZE(ParamContent)

// Constructors for the various ParamTypes.
ParamContent::ParamContent(tesseract::StringParam* it) {
  my_id_ = nrParams;
  nrParams++;
  param_type_ = VT_STRING;
  sIt = it;
  vcMap[my_id_] = this;
}
// Constructors for the various ParamTypes.
ParamContent::ParamContent(tesseract::IntParam* it) {
  my_id_ = nrParams;
  nrParams++;
  param_type_ = VT_INTEGER;
  iIt = it;
  vcMap[my_id_] = this;
}
// Constructors for the various ParamTypes.
ParamContent::ParamContent(tesseract::BoolParam* it) {
  my_id_ = nrParams;
  nrParams++;
  param_type_ = VT_BOOLEAN;
  bIt = it;
  vcMap[my_id_] = this;
}
// Constructors for the various ParamTypes.
ParamContent::ParamContent(tesseract::DoubleParam* it) {
  my_id_ = nrParams;
  nrParams++;
  param_type_ = VT_DOUBLE;
  dIt = it;
  vcMap[my_id_] = this;
}

// Gets a VC object identified by its ID.
ParamContent* ParamContent::GetParamContentById(int id) {
  return vcMap[id];
}

// Copy the first N words from the source string to the target string.
// Words are delimited by "_".
void ParamsEditor::GetFirstWords(
                     const char *s,  // source string
                     int n,          // number of words
                     char *t         // target string
                    ) {
  int full_length = strlen(s);
  int reqd_len = 0;              // No. of chars requird
  const char *next_word = s;

  while ((n > 0) && reqd_len < full_length) {
    reqd_len += strcspn(next_word, "_") + 1;
    next_word += reqd_len;
    n--;
  }
  strncpy(t, s, reqd_len);
  t[reqd_len] = '\0';            // ensure null terminal
}

// Getter for the name.
const char* ParamContent::GetName() const {
  if (param_type_ == VT_INTEGER) { return iIt->name_str(); }
  else if (param_type_ == VT_BOOLEAN) { return bIt->name_str(); }
  else if (param_type_ == VT_DOUBLE) { return dIt->name_str(); }
  else if (param_type_ == VT_STRING) { return sIt->name_str(); }
  else
    return "ERROR: ParamContent::GetName()";
}

// Getter for the description.
const char* ParamContent::GetDescription() const {
  if (param_type_ == VT_INTEGER) { return iIt->info_str(); }
  else if (param_type_ == VT_BOOLEAN) { return bIt->info_str(); }
  else if (param_type_ == VT_DOUBLE) { return dIt->info_str(); }
  else if (param_type_ == VT_STRING) { return sIt->info_str(); }
  else return NULL;
}

// Getter for the value.
STRING ParamContent::GetValue() const {
  STRING result;
  if (param_type_ == VT_INTEGER) {
    result.add_str_int("", *iIt);
  } else if (param_type_ == VT_BOOLEAN) {
    result.add_str_int("", *bIt);
  } else if (param_type_ == VT_DOUBLE) {
    result.add_str_double("", *dIt);
  } else if (param_type_ == VT_STRING) {
    if (((STRING) * (sIt)).string() != NULL) {
      result = sIt->string();
    } else {
      result = "Null";
    }
  }
  return result;
}

// Setter for the value.
void ParamContent::SetValue(const char* val) {
// TODO (wanke) Test if the values actually are properly converted.
// (Quickly visible impacts?)
  changed_ = TRUE;
  if (param_type_ == VT_INTEGER) {
    iIt->set_value(atoi(val));
  } else if (param_type_ == VT_BOOLEAN) {
    bIt->set_value(atoi(val));
  } else if (param_type_ == VT_DOUBLE) {
    dIt->set_value(strtod(val, NULL));
  } else if (param_type_ == VT_STRING) {
    sIt->set_value(val);
  }
}

// Gets the up to the first 3 prefixes from s (split by _).
// For example, tesseract_foo_bar will be split into tesseract,foo and bar.
void ParamsEditor::GetPrefixes(const char* s, STRING* level_one,
                               STRING* level_two,
                               STRING* level_three) {
  char* p = new char[1024];
  GetFirstWords(s, 1, p);
  *level_one = p;
  GetFirstWords(s, 2, p);
  *level_two = p;
  GetFirstWords(s, 3, p);
  *level_three = p;
  delete[] p;
}

// Compare two VC objects by their name.
int ParamContent::Compare(const void* v1, const void* v2) {
  const ParamContent* one =
    *reinterpret_cast<const ParamContent* const *>(v1);
  const ParamContent* two =
    *reinterpret_cast<const ParamContent* const *>(v2);
  return strcmp(one->GetName(), two->GetName());
}

// Find all editable parameters used within tesseract and create a
// SVMenuNode tree from it.
// TODO (wanke): This is actually sort of hackish.
SVMenuNode* ParamsEditor::BuildListOfAllLeaves(tesseract::Tesseract *tess) {
  SVMenuNode* mr = new SVMenuNode();
  ParamContent_LIST vclist;
  ParamContent_IT vc_it(&vclist);
  // Amount counts the number of entries for a specific char*.
  // TODO(rays) get rid of the use of std::map.
  std::map<const char*, int> amount;

  // Add all parameters to a list.
  int v, i;
  int num_iterations = (tess->params() == NULL) ? 1 : 2;
  for (v = 0; v < num_iterations; ++v) {
    tesseract::ParamsVectors *vec = (v == 0) ? GlobalParams() : tess->params();
    for (i = 0; i < vec->int_params.size(); ++i) {
      vc_it.add_after_then_move(new ParamContent(vec->int_params[i]));
    }
    for (i = 0; i < vec->bool_params.size(); ++i) {
      vc_it.add_after_then_move(new ParamContent(vec->bool_params[i]));
    }
    for (i = 0; i < vec->string_params.size(); ++i) {
      vc_it.add_after_then_move(new ParamContent(vec->string_params[i]));
    }
    for (i = 0; i < vec->double_params.size(); ++i) {
      vc_it.add_after_then_move(new ParamContent(vec->double_params[i]));
    }
  }

  // Count the # of entries starting with a specific prefix.
  for (vc_it.mark_cycle_pt(); !vc_it.cycled_list(); vc_it.forward()) {
    ParamContent* vc = vc_it.data();
    STRING tag;
    STRING tag2;
    STRING tag3;

    GetPrefixes(vc->GetName(), &tag, &tag2, &tag3);
    amount[tag.string()]++;
    amount[tag2.string()]++;
    amount[tag3.string()]++;
  }

  vclist.sort(ParamContent::Compare);  // Sort the list alphabetically.

  SVMenuNode* other = mr->AddChild("OTHER");

  // go through the list again and this time create the menu structure.
  vc_it.move_to_first();
  for (vc_it.mark_cycle_pt(); !vc_it.cycled_list(); vc_it.forward()) {
    ParamContent* vc = vc_it.data();
    STRING tag;
    STRING tag2;
    STRING tag3;
    GetPrefixes(vc->GetName(), &tag, &tag2, &tag3);

    if (amount[tag.string()] == 1) {
      other->AddChild(vc->GetName(), vc->GetId(), vc->GetValue().string(),
                      vc->GetDescription());
    } else {  // More than one would use this submenu -> create submenu.
      SVMenuNode* sv = mr->AddChild(tag.string());
      if ((amount[tag.string()] <= MAX_ITEMS_IN_SUBMENU) ||
          (amount[tag2.string()] <= 1)) {
        sv->AddChild(vc->GetName(), vc->GetId(),
                     vc->GetValue().string(), vc->GetDescription());
      } else {  // Make subsubmenus.
        SVMenuNode* sv2 = sv->AddChild(tag2.string());
        sv2->AddChild(vc->GetName(), vc->GetId(),
                      vc->GetValue().string(), vc->GetDescription());
      }
    }
  }
  return mr;
}

// Event listener. Waits for SVET_POPUP events and processes them.
void ParamsEditor::Notify(const SVEvent* sve) {
  if (sve->type == SVET_POPUP) {  // only catch SVET_POPUP!
    char* param = sve->parameter;
    if (sve->command_id == writeCommands[0]) {
      WriteParams(param, false);
    } else if (sve->command_id == writeCommands[1]) {
      WriteParams(param, true);
    } else {
      ParamContent* vc = ParamContent::GetParamContentById(
          sve->command_id);
      vc->SetValue(param);
      sv_window_->AddMessage("Setting %s to %s",
                             vc->GetName(), vc->GetValue().string());
    }
  }
}

// Integrate the parameters editor as popupmenu into the existing scrollview
// window (usually the pg editor). If sv == null, create a new empty
// empty window and attach the parameters editor to that window (ugly).
ParamsEditor::ParamsEditor(tesseract::Tesseract* tess,
                                 ScrollView* sv) {
  if (sv == NULL) {
    const char* name = "ParamEditorMAIN";
    sv = new ScrollView(name, 1, 1, 200, 200, 300, 200);
  }

  sv_window_ = sv;

  //Only one event handler per window.
  //sv->AddEventHandler((SVEventHandler*) this);

  SVMenuNode* svMenuRoot = BuildListOfAllLeaves(tess);

  STRING paramfile;
  paramfile = tess->datadir;
  paramfile += VARDIR;             // parameters dir
  paramfile += "edited";           // actual name

  SVMenuNode* std_menu = svMenuRoot->AddChild ("Build Config File");

  writeCommands[0] = nrParams+1;
  std_menu->AddChild("All Parameters", writeCommands[0],
                     paramfile.string(), "Config file name?");

  writeCommands[1] = nrParams+2;
  std_menu->AddChild ("changed_ Parameters Only", writeCommands[1],
                      paramfile.string(), "Config file name?");

  svMenuRoot->BuildMenu(sv, false);
}


// Write all (changed_) parameters to a config file.
void ParamsEditor::WriteParams(char *filename,
                               bool changes_only) {
  FILE *fp;                      // input file
  char msg_str[255];
                                 // if file exists
  if ((fp = fopen (filename, "rb")) != NULL) {
    fclose(fp);
    sprintf (msg_str, "Overwrite file " "%s" "? (Y/N)", filename);
    int a = sv_window_->ShowYesNoDialog(msg_str);
    if (a == 'n') {
      return;
    }  // don't write
  }


  fp = fopen (filename, "wb");  // can we write to it?
  if (fp == NULL) {
    sv_window_->AddMessage(
        "Can't write to file "
        "%s"
        "",
        filename);
    return;
  }

  for (std::map<int, ParamContent*>::iterator iter = vcMap.begin();
                                          iter != vcMap.end();
                                          ++iter) {
    ParamContent* cur = iter->second;
    if (!changes_only || cur->HasChanged()) {
      fprintf(fp, "%-25s   %-12s   # %s\n",
              cur->GetName(), cur->GetValue().string(), cur->GetDescription());
    }
  }
  fclose(fp);
}
#endif
