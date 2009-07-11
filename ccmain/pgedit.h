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

#ifndef           PGEDIT_H
#define           PGEDIT_H

#include          "ocrblock.h"
#include          "ocrrow.h"
#include          "werd.h"
#include          "rect.h"
#include          "pagewalk.h"
#include          "varable.h"
#include          "notdll.h"
#include          "tesseractclass.h"

class ScrollView;
class SVMenuNode;
struct SVEvent;

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
extern BOOL8 *current_image_changed;
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

void add_word(                             //to block list
              WERD *word,                  //word to be added
              ROW *src_row,                //source row
              BLOCK *src_block,            //source block
              BLOCK_LIST *dest_block_list  //add to this
             );
ScrollView* bln_word_window_handle();  //return handle
void build_image_window(TBOX page_bounding_box);
void display_bln_lines(ScrollView window,
                       ScrollView::Color colour,
                       float scale_factor,
                       float y_offset,
                       float minx,
                       float maxx);
                                 //function to call
void do_re_display (BOOL8 word_painter (
BLOCK *, ROW *, WERD *));
const TBOX do_tidy_cmd();  //tidy
void do_view_cmd();
void do_write_file(            //serialise
                   char *name  //file name
                  );
void pgeditor_msg(  //message display
                  const char *msg);
void pgeditor_show_point(  //display coords
                         SVEvent *event);
void pgeditor_write_file(                    //serialise
                         char *name,         //file name
                         BLOCK_LIST *blocks  //block list to write
                        );
                                 //put bln word in       box
float re_scale_and_move_bln_word(WERD *norm_word,  //BL normalised word
                                 const TBOX &box    //destination box
                                );
void re_segment_word(                         //break/join words
                     BLOCK_LIST *block_list,  //blocks to check
                     TBOX &selection_box);
void block_space_stat(                         //show space stats
                      BLOCK_LIST *block_list,  //blocks to check
                      TBOX &selection_box);
void row_space_stat(                         //show space stats
                    BLOCK_LIST *block_list,  //blocks to check
                    TBOX &selection_box);
void show_point(                         //display posn of bloba word
                BLOCK_LIST *block_list,  //blocks to check
                float x,
                float y);
                                 //display a word
BOOL8 word_blank_and_set_display(BLOCK *block,  //block holding word
                                 ROW *row,      //row holding word
                                 WERD *word     //word to be processed
                                );
BOOL8 word_bln_display(            //bln & display
                       BLOCK *,    //block holding word
                       ROW *row,   //row holding word
                       WERD *word  //word to be processed
                      );
BOOL8 word_change_text(               //change correct text
                       BLOCK *block,  //block holding word
                       ROW *row,      //row holding word
                       WERD *word     //word to be processed
                      );
BOOL8 word_copy(               //copy a word
                BLOCK *block,  //block holding word
                ROW *row,      //row holding word
                WERD *word     //word to be processed
               );
BOOL8 word_delete(                     //delete a word
                  BLOCK *block,        //block holding word
                  ROW *row,            //row holding word
                  WERD *word,          //word to be processed
                  BLOCK_IT &block_it,  //block list iterator
                  ROW_IT &row_it,      //row list iterator
                  WERD_IT &word_it     //word list iterator
                 );
BOOL8 word_display(            // display a word
                   BLOCK *,    //block holding word
                   ROW *row,   //row holding word
                   WERD *word  //word to be processed
                  );
BOOL8 word_dumper(               //dump word
                  BLOCK *block,  //block holding word
                  ROW *row,      //row holding word
                  WERD *word     //word to be processed
                 );
BOOL8 word_set_display(               //display a word
                       BLOCK *block,  //block holding word
                       ROW *row,      //row holding word
                       WERD *word     //word to be processed
                      );
BOOL8 word_toggle_seg(            //toggle seg flag
                      BLOCK *,    //block holding word
                      ROW *,      //row holding word
                      WERD *word  //word to be processed
                     );
void do_check_mem(  //do it
                  inT32 level);
#endif
