///////////////////////////////////////////////////////////////////////
// File:        pgedit.h
// Description: Page structure file editor
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

#ifndef PGEDIT_H
#define PGEDIT_H

#include "params.h"      // for INT_VAR_H, IntParam, STRING_VAR_H, StringParam
#include "scrollview.h"  // for SVEvent (ptr only), SVEventHandler, ScrollView

class BLOCK_LIST;
class PAGE_RES;

namespace tesseract {
  class Tesseract;
}

// A small event handler class to process incoming events to
// this window.
class PGEventHandler : public SVEventHandler {
  public:
   PGEventHandler(tesseract::Tesseract* tess) : tess_(tess) {
   }
   void Notify(const SVEvent* sve);
  private:
    tesseract::Tesseract* tess_;
};

extern BLOCK_LIST *current_block_list;
extern STRING_VAR_H (editor_image_win_name, "EditorImage",
"Editor image window name");
extern INT_VAR_H (editor_image_xpos, 590, "Editor image X Pos");
extern INT_VAR_H (editor_image_ypos, 10, "Editor image Y Pos");
extern INT_VAR_H (editor_image_height, 680, "Editor image height");
extern INT_VAR_H (editor_image_width, 655, "Editor image width");
extern INT_VAR_H (editor_image_word_bb_color, BLUE,
"Word bounding box colour");
extern INT_VAR_H (editor_image_blob_bb_color, YELLOW,
"Blob bounding box colour");
extern INT_VAR_H (editor_image_text_color, WHITE, "Correct text colour");
extern STRING_VAR_H (editor_dbwin_name, "EditorDBWin",
"Editor debug window name");
extern INT_VAR_H (editor_dbwin_xpos, 50, "Editor debug window X Pos");
extern INT_VAR_H (editor_dbwin_ypos, 500, "Editor debug window Y Pos");
extern INT_VAR_H (editor_dbwin_height, 24, "Editor debug window height");
extern INT_VAR_H (editor_dbwin_width, 80, "Editor debug window width");
extern STRING_VAR_H (editor_word_name, "BlnWords",
"BL normalised word window");
extern INT_VAR_H (editor_word_xpos, 60, "Word window X Pos");
extern INT_VAR_H (editor_word_ypos, 510, "Word window Y Pos");
extern INT_VAR_H (editor_word_height, 240, "Word window height");
extern INT_VAR_H (editor_word_width, 655, "Word window width");
extern double_VAR_H (editor_smd_scale_factor, 1.0, "Scaling for smd image");

ScrollView* bln_word_window_handle();  //return handle
void build_image_window(int width, int height);
void pgeditor_msg(  //message display
                  const char *msg);
void pgeditor_show_point(  //display coords
                         SVEvent *event);
                                 //put bln word in       box
void show_point(PAGE_RES* page_res, float x, float y);

#endif
