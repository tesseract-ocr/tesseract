///////////////////////////////////////////////////////////////////////
// File:        svmnode.h
// description_: ScrollView Menu Node
// Author:      Joern Wanke
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

#ifndef TESSERACT_VIEWER_SVMNODE_H_
#define TESSERACT_VIEWER_SVMNODE_H_

#ifndef GRAPHICS_DISABLED

#include <tesseract/export.h>

#include <string>

namespace tesseract {

class ScrollView;

class TESS_API SVMenuNode {
public:
  // Creating the (empty) root menu node.
  SVMenuNode();

  // Destructor for every node.
  ~SVMenuNode();

  // Create a new sub menu node with just a caption.  This is used to create
  // nodes which act as parent nodes to other nodes (e.g. submenus).
  SVMenuNode *AddChild(const char *txt);

  // Create a "normal" menu node which is associated with a command event.
  void AddChild(const char *txt, int command_event);

  // Create a flag menu node.
  void AddChild(const char *txt, int command_event, int tv);

  // Create a menu node with an associated value (which might be changed
  // through the gui).
  void AddChild(const char *txt, int command_event, const char *val);

  // Create a menu node with an associated value and description_.
  void AddChild(const char *txt, int command_event, const char *val, const char *desc);

  // Build a menu structure for the server and send the necessary messages.
  // Should be called on the root node. If menu_bar is true, a menu_bar menu
  // is built (e.g. on top of the window), if it is false a popup menu is
  // built which gets shown by right clicking on the window.
  void BuildMenu(ScrollView *sv, bool menu_bar = true);

private:
  // Constructor holding the actual node data.
  SVMenuNode(int command_event, const char *txt, int tv, bool check_box_entry, const char *val = "",
             const char *desc = "");

  // Adds a new menu node to the current node.
  void AddChild(SVMenuNode *svmn);

  // The parent node of this node.
  SVMenuNode *parent_;
  // The first child of this node.
  SVMenuNode *child_;
  // The next "sibling" of this node (e.g. same parent).
  SVMenuNode *next_;
  // Whether this menu node actually is a flag.
  bool is_check_box_entry_;
  // The value of the flag (if this menu node is a flag).
  bool toggle_value_;

  // The command event associated with a specific menu node. Should be unique.
  int cmd_event_;
  // The caption associated with a specific menu node.
  std::string text_;
  // The value of the menu node. (optional)
   std::string value_;
  // A description_ of the value. (optional)
   std::string description_;
};

} // namespace tesseract

#endif // !GRAPHICS_DISABLED

#endif // TESSERACT_VIEWER_SVMNODE_H_
