///////////////////////////////////////////////////////////////////////
// File:        svmnode.cpp
// description_: ScrollView Menu Node
// Author:      Joern Wanke
// Created:     Thu Nov 29 2007
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
// A SVMenuNode is an entity which contains the mapping from a menu entry on
// the server side to the corresponding associated commands on the client.
// It is designed to be a tree structure with a root node, which can then be
// used to generate the appropriate messages to the server to display the
// menu structure there.
// A SVMenuNode can both be used in the context_ of popup menus as well as
// menu bars.

#include <cstring>
#include <iostream>

#include "svmnode.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED

#include "scrollview.h"

// Create the empty root menu node. with just a caption. All other nodes should
// be added to this or one of the submenus.
SVMenuNode::SVMenuNode() {
  cmd_event_ = -1;
  child_ = nullptr;
  next_ = nullptr;
  parent_ = nullptr;
  toggle_value_ = false;
  is_check_box_entry_ = false;
}

SVMenuNode::~SVMenuNode() {
}

// Create a new sub menu node with just a caption.  This is used to create
// nodes which act as parent nodes to other nodes (e.g. submenus).
SVMenuNode* SVMenuNode::AddChild(const char* txt) {
  SVMenuNode* s = new SVMenuNode(-1, txt, false, false, nullptr, nullptr);
  this->AddChild(s);
  return s;
}

// Create a "normal" menu node which is associated with a command event.
void SVMenuNode::AddChild(const char* txt, int command_event) {
  this->AddChild(new SVMenuNode(command_event, txt, false, false, nullptr, nullptr));
}

// Create a menu node with an associated value (which might be changed
// through the gui).
void SVMenuNode::AddChild(const char* txt, int command_event,
                          const char* val) {
  this->AddChild(new SVMenuNode(command_event, txt, false, false, val, nullptr));
}

// Create a menu node with an associated value and description_.
void SVMenuNode::AddChild(const char* txt, int command_event, const char* val,
                          const char* desc) {
  this->AddChild(new SVMenuNode(command_event, txt, false, false, val, desc));
}

// Create a flag menu node.
void SVMenuNode::AddChild(const char* txt, int command_event, int tv) {
  this->AddChild(new SVMenuNode(command_event, txt, tv, true, nullptr, nullptr));
}

// Convenience function called from the different constructors to initialize
// the different values of the menu node.
SVMenuNode::SVMenuNode(int command_event, const char* txt,
                       int tv, bool check_box_entry, const char* val,
                       const char* desc)
  : text_(txt), value_(val), description_(desc) {
  cmd_event_ = command_event;

  child_ = nullptr;
  next_ = nullptr;
  parent_ = nullptr;
  toggle_value_ = tv != 0;
  is_check_box_entry_ = check_box_entry;
}

// Add a child node to this menu node.
void SVMenuNode::AddChild(SVMenuNode* svmn) {
  svmn->parent_ = this;
  // No children yet.
  if (child_ == nullptr) {
    child_ = svmn;
  } else {
    SVMenuNode* cur = child_;
    while (cur->next_ != nullptr) { cur = cur->next_; }
    cur->next_ = svmn;
  }
}

// Build a menu structure for the server and send the necessary messages.
// Should be called on the root node. If menu_bar is true, a menu_bar menu
// is built (e.g. on top of the window), if it is false a popup menu is
// built which gets shown by right clicking on the window.
// Deletes itself afterwards.
void SVMenuNode::BuildMenu(ScrollView* sv, bool menu_bar) {
  if ((parent_ != nullptr) && (menu_bar)) {
    if (is_check_box_entry_) {
      sv->MenuItem(parent_->text_.string(), text_.string(), cmd_event_,
                   toggle_value_);
    } else {
      sv->MenuItem(parent_->text_.string(), text_.string(), cmd_event_); }
  } else if ((parent_ != nullptr) && (!menu_bar)) {
    if (description_.length() > 0) {
      sv->PopupItem(parent_->text_.string(), text_.string(), cmd_event_,
                    value_.string(), description_.string());
      } else {
      sv->PopupItem(parent_->text_.string(), text_.string());
    }
  }
  if (child_ != nullptr) {
    child_->BuildMenu(sv, menu_bar); delete child_;
  }
  if (next_ != nullptr) {
    next_->BuildMenu(sv, menu_bar); delete next_;
  }
}

#endif  // GRAPHICS_DISABLED
