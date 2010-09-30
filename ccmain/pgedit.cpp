/**********************************************************************
 * File:        pgedit.cpp (Formerly pgeditor.c)
 * Description: Page structure file editor
 * Author:      Phil Cheatle
 * Created:     Thu Oct 10 16:25:24 BST 1991
 *
 *(C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0(the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http:// www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include          "pgedit.h"

#include          <ctype.h>
#include          <math.h>

#include          "genblob.h"
#include          "tessio.h"
#include          "tessout.h"
#include          "tordmain.h"
#include          "statistc.h"
#include          "debugwin.h"
#include          "svshowim.h"
#include          "mainblk.h"
#include          "varabled.h"
#include          "string.h"

#include          "scrollview.h"
#include          "svmnode.h"

#include          "control.h"
#include "tesseractclass.h"

#include          "blread.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED
#define ASC_HEIGHT     (2 * bln_baseline_offset + bln_x_height)
#define X_HEIGHT       (bln_baseline_offset + bln_x_height)
#define BL_HEIGHT     bln_baseline_offset
#define DESC_HEIGHT     0
#define MAXSPACING      128      /*max expected spacing in pix */

const ERRCODE EMPTYBLOCKLIST = "No blocks to edit";
extern IMAGE page_image;

enum CMD_EVENTS
{
  NULL_CMD_EVENT,
  DELETE_CMD_EVENT,
  COPY_CMD_EVENT,
  CHANGE_DISP_CMD_EVENT,
  CHANGE_TEXT_CMD_EVENT,
  TOGGLE_SEG_CMD_EVENT,
  DUMP_WERD_CMD_EVENT,
  SHOW_POINT_CMD_EVENT,
  ROW_SPACE_STAT_CMD_EVENT,
  BLOCK_SPACE_STAT_CMD_EVENT,
  SHOW_BLN_WERD_CMD_EVENT,
  SEGMENT_WERD_CMD_EVENT,
  BOUNDING_BOX_CMD_EVENT,
  CORRECT_TEXT_CMD_EVENT,
  POLYGONAL_CMD_EVENT,
  BL_NORM_CMD_EVENT,
  BITMAP_CMD_EVENT,
  TIDY_CMD_EVENT,
  VIEW_CMD_EVENT,
  IMAGE_CMD_EVENT,
  BLOCKS_CMD_EVENT,
  BASELINES_CMD_EVENT,
  WRITE_CMD_EVENT,
  NEW_SOURCE_CMD_EVENT,
  UNIFORM_DISP_CMD_EVENT,
  REFRESH_CMD_EVENT,
  QUIT_CMD_EVENT,
  RECOG_WERDS,
  RECOG_PSEUDO
};

/*
 *
 *  Some global data
 *
 */

ScrollView* image_win;
VariablesEditor* ve;
bool stillRunning = false;

#ifdef __UNIX__
FILE *debug_window = NULL;       // opened on demand
#endif
                                 // baseline norm words
ScrollView* bln_word_window = NULL;

CMD_EVENTS mode = CHANGE_DISP_CMD_EVENT;
                                 // Selected words op

BITS16 word_display_mode;
BOOL8 display_image = FALSE;
BOOL8 display_blocks = FALSE;
BOOL8 display_baselines = FALSE;
BOOL8 viewing_source = TRUE;

BLOCK_LIST *source_block_list = NULL;    // image blocks
BLOCK_LIST target_block_list;    // target blocks
BLOCK_LIST *other_block_list = &target_block_list;

BOOL8 source_changed = FALSE;    // Changes not saved
BOOL8 target_changed = FALSE;    // Changes not saved
BOOL8 *other_image_changed = &target_changed;


/* Public globals */

#define EXTERN

EXTERN BLOCK_LIST *current_block_list = NULL;
EXTERN BOOL8 *current_image_changed = &source_changed;

/* Variables */

EXTERN STRING_VAR(editor_image_win_name, "EditorImage",
"Editor image window name");
EXTERN INT_VAR(editor_image_xpos, 590, "Editor image X Pos");
EXTERN INT_VAR(editor_image_ypos, 10, "Editor image Y Pos");
EXTERN INT_VAR(editor_image_menuheight, 50, "Add to image height for menu bar");
EXTERN INT_VAR(editor_image_word_bb_color, ScrollView::BLUE,
"Word bounding box colour");
EXTERN INT_VAR(editor_image_blob_bb_color, ScrollView::YELLOW,
"Blob bounding box colour");
EXTERN INT_VAR(editor_image_text_color, ScrollView::WHITE,
"Correct text colour");

EXTERN STRING_VAR(editor_dbwin_name, "EditorDBWin",
"Editor debug window name");
EXTERN INT_VAR(editor_dbwin_xpos, 50, "Editor debug window X Pos");
EXTERN INT_VAR(editor_dbwin_ypos, 500, "Editor debug window Y Pos");
EXTERN INT_VAR(editor_dbwin_height, 24, "Editor debug window height");
EXTERN INT_VAR(editor_dbwin_width, 80, "Editor debug window width");

EXTERN STRING_VAR(editor_word_name, "BlnWords", "BL normalised word window");
EXTERN INT_VAR(editor_word_xpos, 60, "Word window X Pos");
EXTERN INT_VAR(editor_word_ypos, 510, "Word window Y Pos");
EXTERN INT_VAR(editor_word_height, 240, "Word window height");
EXTERN INT_VAR(editor_word_width, 655, "Word window width");

EXTERN double_VAR(editor_smd_scale_factor, 1.0, "Scaling for smd image");

/**
 *  add_word()
 *
 *  Inserts the a word into a specified block list. The list is searched for a
 *  block and row of the same file as those of the word to be added, which
 *  contain the bounding box of the word.  If such a row is found, the new
 *  word is inserted into the row in the correct X order.  If the
 *  block is found, but not the row, a copy of the word's old row is added to
 *  the block in the correct Y order, and the word is put in that row.
 *  If neither the row nor the block are found, then the word's old block is
 *  copied with only the word's row. It is added to the block list in the
 *  correct Y order.
 */

void add_word(                            // to block list
              WERD *word,                  //< word to be added
              ROW *src_row,                //< source row
              BLOCK *src_block,            //< source block
              BLOCK_LIST *dest_block_list  //< add to this
             ) {
  BLOCK_IT block_it(dest_block_list);
  BLOCK *block;                  // current block
  BLOCK *dest_block = NULL;      // destination block
  ROW_IT row_it;
  ROW *row;                      // destination row
  ROW *dest_row = NULL;          // destination row
  WERD_IT word_it;
  TBOX word_box = word->bounding_box();
  TBOX insert_point_word_box;
  BOOL8 seen_blocks_for_current_file = FALSE;

  block_it.mark_cycle_pt();
  while(!block_it.cycled_list() &&(dest_block == NULL)) {
    block = block_it.data();
    if ((block->bounding_box().contains(word_box)) &&
   (strcmp(block->name(), src_block->name()) == 0)) {
      dest_block = block;        // found dest block
      row_it.set_to_list(block->row_list());
      row_it.mark_cycle_pt();
      while((!row_it.cycled_list()) &&(dest_row == NULL)) {
        row = row_it.data();
        if (row->bounding_box().contains(word_box))
          dest_row = row;        // found dest row
        else
          row_it.forward();
      }
    }
    else
      block_it.forward();
  }

  if (dest_block == NULL) {      // make a new one
    dest_block = new BLOCK;
    *dest_block = *src_block;

    block_it.set_to_list(dest_block_list);
    for (block_it.mark_cycle_pt();
    !block_it.cycled_list(); block_it.forward()) {
      block = block_it.data();

      if (!seen_blocks_for_current_file &&
       (strcmp(block->name(), dest_block->name()) == 0))
        seen_blocks_for_current_file = TRUE;

      if (seen_blocks_for_current_file &&
       ((strcmp(block->name(), dest_block->name()) != 0) ||
       (block->bounding_box().top() <
        dest_block->bounding_box().top())))
        break;
    }

    if (block_it.cycled_list())
                                 // didn't find insrt pt
      block_it.add_to_end(dest_block);
    else
                                 // did find insert pt
      block_it.add_before_stay_put(dest_block);
  }

  if (dest_row == NULL) {        // make a new one
    dest_row = new ROW;
    *dest_row = *src_row;

    row_it.set_to_list(dest_block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      if (row_it.data()->bounding_box().top() <
        dest_row->bounding_box().top())
        break;
    }

    if (row_it.cycled_list())
                                 // didn't find insrt pt
        row_it.add_to_end(dest_row);
    else
                                 // did find insert pt
      row_it.add_before_stay_put(dest_row);
  }

  /* dest_block and dest_row are now found or built and inserted as necessesary
    so add the word to dest row */

  word_it.set_to_list(dest_row->word_list());
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    if (word_it.data()->bounding_box().right() >= word_box.left())
      break;
  }

  if (word_it.cycled_list())
    word_it.add_to_end(word);   // didn't find insrt pt
  else {                         // did find insert pt
    insert_point_word_box = word_it.data()->bounding_box();
    if (insert_point_word_box.contains(word_box) ||
      word_box.contains(insert_point_word_box))
      image_win->AddMessage("Refusing to add words which obliterate,"
                               " or are obliterated by, others");
    else {
      if (word_it.data()->bounding_box().left() >
        word->bounding_box().left())
                                 // infront of insert pt
        word_it.add_before_stay_put(word);
      else
                                 // behind insert pt
        word_it.add_after_stay_put(word);
    }
  }
}


class BlnEventHandler : public SVEventHandler {
 public:
  void Notify(const SVEvent* sv_event) {
    if (sv_event->type == SVET_DESTROY)
      bln_word_window = NULL;
    else if (sv_event->type == SVET_CLICK)
      show_point(current_block_list, sv_event->x, sv_event->y);
  }
};

/**
 *  bln_word_window_handle()
 *
 *  @return a WINDOW for the word window, creating it if necessary
 */
ScrollView* bln_word_window_handle() {  // return handle
                                 // not opened yet
  if (bln_word_window == NULL) {
    pgeditor_msg("Creating BLN word window...");
    bln_word_window = new ScrollView(editor_word_name.string(),
      editor_word_xpos, editor_word_ypos, editor_word_width,
      editor_word_height, 4000, 4000, true);
    BlnEventHandler* a = new BlnEventHandler();
    bln_word_window->AddEventHandler(a);
    pgeditor_msg("Creating BLN word window...Done");
  }
  return bln_word_window;
}

/**
 *  build_image_window()
 *
 *  Destroy the existing image window if there is one.  Work out how big the
 *  new window needs to be. Create it and re-display.
 */

void build_image_window(TBOX page_bounding_box) {
  if (image_win != NULL) { delete image_win; }
  image_win = new ScrollView(editor_image_win_name.string(),
                             editor_image_xpos, editor_image_ypos,
                             page_bounding_box.right() + 1,
                             page_bounding_box.top() +
                               editor_image_menuheight + 1,
                             page_bounding_box.right() + 1,
                             page_bounding_box.top() + 1,
                             true);
}


/**
 *  build_menu()
 *
 *  Construct the menu tree used by the command window
 */
namespace tesseract {
SVMenuNode *Tesseract::build_menu_new() {

  SVMenuNode* parent_menu;
  SVMenuNode* root_menu_item = new SVMenuNode();

  SVMenuNode* modes_menu_item = root_menu_item->AddChild("MODES");

  modes_menu_item->AddChild("Change Display",
      CHANGE_DISP_CMD_EVENT);
  modes_menu_item->AddChild("Delete",
      DELETE_CMD_EVENT);
  modes_menu_item->AddChild("Copy to TARGET",
      COPY_CMD_EVENT);
  modes_menu_item->AddChild("Change Text",
      CHANGE_TEXT_CMD_EVENT);
  modes_menu_item->AddChild("Toggle Correct Seg Flg",
      TOGGLE_SEG_CMD_EVENT);
  modes_menu_item->AddChild("Dump Word",
      DUMP_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Show Point",
      SHOW_POINT_CMD_EVENT);
  modes_menu_item->AddChild("Row gaps hist",
      ROW_SPACE_STAT_CMD_EVENT);
  modes_menu_item->AddChild("Block gaps hist",
      BLOCK_SPACE_STAT_CMD_EVENT);
  modes_menu_item->AddChild("Show BL Norm Word",
      SHOW_BLN_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Re-Segment Word",
      SEGMENT_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Recog Words",
      RECOG_WERDS);
  modes_menu_item->AddChild("Recog Blobs",
      RECOG_PSEUDO);

  parent_menu = root_menu_item->AddChild("DISPLAY");

  parent_menu->AddChild("Bounding Boxes",
      BOUNDING_BOX_CMD_EVENT, FALSE);
  parent_menu->AddChild("Correct Text",
      CORRECT_TEXT_CMD_EVENT, FALSE);
  parent_menu->AddChild("Polygonal Approx",
      POLYGONAL_CMD_EVENT, FALSE);
  parent_menu->AddChild("Baseline Normalised",
      BL_NORM_CMD_EVENT, FALSE);
  parent_menu->AddChild("Edge Steps",
      BITMAP_CMD_EVENT, TRUE);

  parent_menu = root_menu_item->AddChild("OTHER");

  parent_menu->AddChild("Quit",
      QUIT_CMD_EVENT);
  parent_menu->AddChild("Tidy Target",
      TIDY_CMD_EVENT);

  parent_menu->AddChild("View TARGET",
      VIEW_CMD_EVENT, FALSE);
  parent_menu->AddChild("Show Image",
      IMAGE_CMD_EVENT, FALSE);
  parent_menu->AddChild("ShowBlock Outlines",
      BLOCKS_CMD_EVENT, FALSE);
  parent_menu->AddChild("Show Baselines",
      BASELINES_CMD_EVENT, FALSE);
  parent_menu->AddChild("Write File",
      WRITE_CMD_EVENT,    imagebasename.string());
  parent_menu->AddChild("New Source File",
      NEW_SOURCE_CMD_EVENT,    imagebasename.string());
  parent_menu->AddChild("Uniform Display",
      UNIFORM_DISP_CMD_EVENT);
  parent_menu->AddChild("Refresh Display",
      REFRESH_CMD_EVENT);

  return root_menu_item;
}

}  // namespace tesseract


/**
 *  display_bln_lines()
 *
 *  Display normalised baseline, x-height, ascender limit and descender limit
 */

void display_bln_lines(ScrollView* window,
                       ScrollView::Color colour,
                       float scale_factor,
                       float y_offset,
                       float minx,
                       float maxx) {
  window->Pen(colour);
  window->Line(minx, y_offset + scale_factor * DESC_HEIGHT,
               maxx, y_offset + scale_factor * DESC_HEIGHT);
  window->Line(minx, y_offset + scale_factor * BL_HEIGHT,
               maxx, y_offset + scale_factor * BL_HEIGHT);
  window->Line(minx, y_offset + scale_factor * X_HEIGHT,
               maxx, y_offset + scale_factor * X_HEIGHT);
  window->Line(minx, y_offset + scale_factor * ASC_HEIGHT,
               maxx, y_offset + scale_factor * ASC_HEIGHT);
}


/**
 *  do_new_source()
 *
 *  Change to another source file.  Automatically tidy page first
 *
 */
namespace tesseract {
void Tesseract::do_new_source(           // serialise
                  ) {
  FILE *infp;                    // input file

  char* name = image_win->ShowInputDialog("New Source File name");

  STRING name_str(name);
  delete[] name;

  if (source_changed) {

    int a = image_win->ShowYesNoDialog(
        "Source changes will be LOST.  Continue?(Y/N)");
	if (a != 'y') { image_win->AddMessage("Write cancelled"); return; }

  }

                                 // if not file exists
  if (!(infp = fopen(name_str.string(), "r"))) {

     image_win->AddMessage("Cant open file " "%s" "", name_str.string());
    return;
  }

  fclose(infp);

  image_win->AddMessage("Reading file " "%s" "...", name_str.string());
  source_block_list->clear();
                                 // appends to SOURCE
  pgeditor_read_file(name_str, source_block_list);
  source_changed = FALSE;

  image_win->AddMessage("Doing automatic Tidy Target...");
  viewing_source = FALSE;        // Force viewing source
  do_tidy_cmd();

  image_win->AddMessage("Doing automatic Tidy Target...Done");

}
}  // namespace tesseract


/**
 *  do_re_display()
 *
 *  Redisplay page
 */

void
                                 // function to call
do_re_display(BOOL8 word_painter(
BLOCK *, ROW *, WERD *)) {
  BLOCK_IT block_it(current_block_list);
  BLOCK *block;
  int block_count = 1;

  ROW_IT row_it;
  ROW *row;

  WERD_IT word_it;
  WERD *word;

  image_win->Clear();
  if (display_image != 0) {
    sv_show_sub_image(&page_image, 0, 0,
      page_image.get_xsize(), page_image.get_ysize(),
      image_win, 0, 0);
  }

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block = block_it.data();
    row_it.set_to_list(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      row = row_it.data();
      word_it.set_to_list(row->word_list());
      for (word_it.mark_cycle_pt();
      !word_it.cycled_list(); word_it.forward()) {
        word = word_it.data();
        word_painter(block, row, word);
      }
      if (display_baselines)
        row->plot_baseline(image_win, ScrollView::GREEN);
    }
    if (display_blocks)
      block->plot(image_win, block_count++, ScrollView::RED);
  }
  image_win->Update();
}


/**
 *  do_tidy_cmd()
 *
 *  Tidy TARGET page
 */

const TBOX do_tidy_cmd() {  // tidy
  ICOORD shift_vector;
  TBOX tidy_box;                  // Just the tidy area
  TBOX source_box;                // source file area

  source_box = block_list_bounding_box(source_block_list);
  // find src area

  if (!target_block_list.empty()) {
    tidy_box = block_list_compress(&target_block_list);

    /* Shift tidied target above the source image area. */

    shift_vector = ICOORD(0, source_box.top() + BLOCK_SPACING)
      - tidy_box.botleft();
    block_list_move(&target_block_list, shift_vector);
    tidy_box.move(shift_vector);
  }
  source_box += tidy_box;
                                 // big enough for both
  build_image_window(source_box);
  do_view_cmd();
  return tidy_box;
}


/**
 *  do_view_cmd()
 *
 *  View TARGET/View SOURCE command
 */

void do_view_cmd() {
  viewing_source = !viewing_source;
  image_win->Clear();
  if (viewing_source) {
    current_block_list = source_block_list;
    current_image_changed = &source_changed;
    other_block_list = &target_block_list;
    other_image_changed = &target_changed;
    do_re_display(&word_display);
  }
  else {
    current_block_list = &target_block_list;
    current_image_changed = &target_changed;
    other_block_list = source_block_list;
    other_image_changed = &source_changed;
    do_re_display(&word_display);
  }
}


/**
 *  do_write_file()
 *
 *  Serialise a block list to file
 *
 *  If writing image, tidy page and move to(0,0) first
 */

void do_write_file(           // serialise
                  ) {

  char* name = image_win->ShowInputDialog("File Name");

  FILE *infp;                    // input file
  char msg_str[80];

  TBOX enclosing_box;

                                 // if file exists
  if ((infp = fopen(name, "r")) != NULL) {
    fclose(infp);
    sprintf(msg_str, "Overwrite file " "%s" "?(Y/N)", name);

    int a = image_win->ShowYesNoDialog(msg_str);
	if (a != 'y') { image_win->AddMessage("Write cancelled"); delete[] name; return; }
  }

  infp = fopen(name, "w");      // can we write to it?
  if (infp == NULL) {

  image_win->AddMessage("Cant write to file " "%s" "", name);
    delete[] name;
    return;
  }
  fclose(infp);

  delete [] name;

  if (!viewing_source && !target_block_list.empty()) {
                                 // Tidy & move to(0,0)
    image_win->AddMessage("Automatic tidy...");
    viewing_source = TRUE;       // Stay viewing target!
    enclosing_box = do_tidy_cmd();
    block_list_move(&target_block_list, -enclosing_box.botleft());
    image_win->AddMessage("Writing file...");
    pgeditor_write_file(name, &target_block_list);
                                 // move back
    block_list_move(&target_block_list,
      enclosing_box.botleft());
  }
  else {
    image_win->AddMessage("Writing file...");
    pgeditor_write_file(name, current_block_list);
  }
  image_win->AddMessage("Writing file...Done");
  *current_image_changed = FALSE;

}

/**
 *  notify()
 *
 *  Event handler that processes incoming events, either forwarding
 *  them to process_cmd_win_event or process_image_event.
 *
 */

void PGEventHandler::Notify(const SVEvent* event) {
  char myval = '0';
  if (event->type == SVET_POPUP) {
ve->Notify(event);
  } // These are handled by Var. Editor
  else if (event->type == SVET_EXIT) { stillRunning = false; }
  else if (event->type == SVET_MENU) {
     if (strcmp(event->parameter, "true") == 0) { myval = 'T'; }
     else if (strcmp(event->parameter, "false") == 0) { myval = 'F'; }
     tess_->process_cmd_win_event(event->command_id, &myval);
  }
  else {
     tess_->process_image_event(*event);
     // else    pgeditor_show_point(*event);
  }
  current_word_quit.set_value(FALSE);
  selection_quit.set_value(FALSE);
                                 // replot all var wins
}


/**
 *  pgeditor_main()
 *
 *  Top level editor operation:
 *  Setup a new window and an according event handler
 *
 */

namespace tesseract {
void Tesseract::pgeditor_main(BLOCK_LIST *blocks) {

  source_block_list = blocks;
  current_block_list = blocks;
  if (current_block_list->empty())
    return;

  stillRunning = true;

  build_image_window(block_list_bounding_box(source_block_list));
  word_display_mode.turn_on_bit(DF_EDGE_STEP);
  do_re_display(&word_set_display);
#ifndef GRAPHICS_DISABLED
  ve = new VariablesEditor(this, image_win);
#endif
  PGEventHandler pgEventHandler(this);

  image_win->AddEventHandler(&pgEventHandler);
  image_win->AddMessageBox();

  SVMenuNode* svMenuRoot = build_menu_new();

  svMenuRoot->BuildMenu(image_win);
  image_win->SetVisible(true);

  image_win->AwaitEvent(SVET_DESTROY);
  image_win->AddEventHandler(NULL);
}
}  // namespace tesseract


/**
 * pgeditor_msg()
 *
 * Display a message - in the command window if there is one, or to stdout
 */

void pgeditor_msg( // message display
                  const char *msg) {
    image_win->AddMessage(msg);
}


/**
 * pgeditor_read_file()
 *
 * Deserialise source file
 */

namespace tesseract {
void Tesseract::pgeditor_read_file(                   // of serialised file
                                   STRING &filename,
                                   BLOCK_LIST *blocks  // block list to add to
                                  ) {
  STRING name = filename;        //truncated name
  const char *lastdot;           //of name
  TO_BLOCK_LIST land_blocks, port_blocks;
  TBOX page_box;

  lastdot = strrchr (name.string (), '.');
  if (lastdot != NULL)
    name[lastdot-name.string()] = '\0';
  if (!read_unlv_file(name, page_image.get_xsize(), page_image.get_ysize(),
                     blocks))
    FullPageBlock(page_image.get_xsize(), page_image.get_ysize(), blocks);
  find_components(blocks, &land_blocks, &port_blocks, &page_box);
  textord_page(page_box.topright(), blocks, &land_blocks, &port_blocks, this);
}
}  // namespace tesseract

/**
 * pgeditor_show_point()
 *
 * Display the coordinates of a point in the command window
 */

void pgeditor_show_point( // display coords
                         SVEvent *event) {
  image_win->AddMessage("Pointing at(%d, %d)", event->x, event->y);
}


/**
 *  pgeditor_write_file()
 *
 *  Serialise a block list to file
 *
 */

void pgeditor_write_file(                   // serialise
                         char *name,         // file name
                         BLOCK_LIST *blocks  // block list to write
                        ) {
  FILE *infp;                    // input file
  BLOCK_IT block_it(blocks);  // block iterator
  BLOCK *block;                  // current block
  ROW_IT row_it;                 // row iterator

  infp = fopen(name, "w");      // create output file
  if (infp == NULL)
    CANTCREATEFILE.error("pgeditor_write_file", EXIT, name);

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block = block_it.extract();

    row_it.set_to_list(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward())
                                 // ensure correct
      row_it.data()->recalc_bounding_box();

    block->serialise(infp);     // serialize     non-empty
    block_it.add_after_then_move(block);
  }
  fclose(infp);
}


/**
 *  process_cmd_win_event()
 *
 *  Process a command returned from the command window
 * (Just call the appropriate command handler)
 */

namespace tesseract {
BOOL8 Tesseract::process_cmd_win_event(                 // UI command semantics
                                       inT32 cmd_event,  // which menu item?
                                       char *new_value   // any prompt data
                                      ) {
  char msg[160];
  BOOL8 exit = FALSE;

  switch(cmd_event) {
    case NULL_CMD_EVENT:
      break;

    case VIEW_CMD_EVENT:
      do_view_cmd();
      break;
    case CHANGE_DISP_CMD_EVENT:
    case DELETE_CMD_EVENT:
    case CHANGE_TEXT_CMD_EVENT:
    case TOGGLE_SEG_CMD_EVENT:
    case DUMP_WERD_CMD_EVENT:
    case SHOW_POINT_CMD_EVENT:
    case ROW_SPACE_STAT_CMD_EVENT:
    case BLOCK_SPACE_STAT_CMD_EVENT:
    case SHOW_BLN_WERD_CMD_EVENT:
    case SEGMENT_WERD_CMD_EVENT:
      mode =(CMD_EVENTS) cmd_event;
      break;
    case COPY_CMD_EVENT:
      mode =(CMD_EVENTS) cmd_event;
      if (!viewing_source)
        image_win->AddMessage("Can't COPY while viewing target!");
      break;
    case BOUNDING_BOX_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit(DF_BOX);
      else
        word_display_mode.turn_off_bit(DF_BOX);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case CORRECT_TEXT_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit(DF_TEXT);
      else
        word_display_mode.turn_off_bit(DF_TEXT);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case POLYGONAL_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit(DF_POLYGONAL);
      else
        word_display_mode.turn_off_bit(DF_POLYGONAL);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case BL_NORM_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit(DF_BN_POLYGONAL);
      else
        word_display_mode.turn_off_bit(DF_BN_POLYGONAL);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case BITMAP_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit(DF_EDGE_STEP);
      else
        word_display_mode.turn_off_bit(DF_EDGE_STEP);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case UNIFORM_DISP_CMD_EVENT:
      do_re_display(&word_set_display);
      *current_image_changed = TRUE;
      break;
    case WRITE_CMD_EVENT:
      do_write_file();
      break;
    case TIDY_CMD_EVENT:
      if (!target_block_list.empty()) {
        viewing_source = TRUE;   // Force viewing target
        do_tidy_cmd();
      }
      break;
    case NEW_SOURCE_CMD_EVENT:
      do_new_source();
      break;
    case IMAGE_CMD_EVENT:
      display_image =(new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case BLOCKS_CMD_EVENT:
      display_blocks =(new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case BASELINES_CMD_EVENT:
      display_baselines =(new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case REFRESH_CMD_EVENT:
      do_re_display(&word_display);
      break;
    case QUIT_CMD_EVENT:
      if (source_changed || target_changed) {
        int a = image_win->ShowYesNoDialog(
            "Changes not saved. Exit anyway?(Y/N)");
	if (a == 'y') { exit = TRUE; ScrollView::Exit(); }
      }
      else {
        exit = TRUE;
	ScrollView::Exit();
	}
      break;
    case RECOG_WERDS:
      mode = RECOG_WERDS;
    break;
    case RECOG_PSEUDO:
      mode = RECOG_PSEUDO;
    break;

    default:
      sprintf(msg, "Unrecognised event " INT32FORMAT "(%s)",
               cmd_event, new_value);
      image_win->AddMessage(msg);
    break;
  }
  return exit;
}


/**
 * process_image_event()
 *
 * User has done something in the image window - mouse down or up.  Work out
 * what it is and do something with it.
 * If DOWN - just remember where it was.
 * If UP - for each word in the selected area do the operation defined by
 * the current mode.
 */
void Tesseract::process_image_event( // action in image win
                                    const SVEvent &event) {
  static ICOORD down;
  ICOORD up;
  TBOX selection_box;
  char msg[80];

  switch(event.type) {

    case SVET_SELECTION:
      if (event.type == SVET_SELECTION) {
	down.set_x(event.x - event.x_size);
        down.set_y(event.y + event.y_size);
        if (mode == SHOW_POINT_CMD_EVENT)
          show_point(current_block_list, event.x, event.y);
      }

      up.set_x(event.x);
      up.set_y(event.y);

      selection_box = TBOX(down, up);

      switch(mode) {
        case CHANGE_DISP_CMD_EVENT:
          ::process_selected_words(current_block_list,
                                 selection_box,
                                 &word_blank_and_set_display);
          break;
        case COPY_CMD_EVENT:
          if (!viewing_source)
            image_win->AddMessage("Can't COPY while viewing target!");
          else
            ::process_selected_words(current_block_list,
                                   selection_box,
                                   &word_copy);
          break;
        case DELETE_CMD_EVENT:
          ::process_selected_words_it(current_block_list,
                                    selection_box,
                                    &word_delete);
          break;
        case CHANGE_TEXT_CMD_EVENT:
          ::process_selected_words(current_block_list,
                                 selection_box,
                                 &word_change_text);
          break;
        case TOGGLE_SEG_CMD_EVENT:
          ::process_selected_words(current_block_list,
                                   selection_box,
                                   &word_toggle_seg);
          break;
        case DUMP_WERD_CMD_EVENT:
          ::process_selected_words(current_block_list,
                                   selection_box,
                                   &word_dumper);
          break;
        case SHOW_BLN_WERD_CMD_EVENT:
          ::process_selected_words(current_block_list,
                                   selection_box,
                                   &word_bln_display);
          break;
        case SEGMENT_WERD_CMD_EVENT:
          re_segment_word(current_block_list, selection_box);
          break;
        case ROW_SPACE_STAT_CMD_EVENT:
          row_space_stat(current_block_list, selection_box);
          break;
        case BLOCK_SPACE_STAT_CMD_EVENT:
          block_space_stat(current_block_list, selection_box);
          break;
        case SHOW_POINT_CMD_EVENT:
          break;                 // ignore up event

        case RECOG_WERDS:
          image_win->AddMessage("Recogging selected words");
          this->process_selected_words(current_block_list,
                                       selection_box,
                                       &Tesseract::recog_interactive);
          break;
        case RECOG_PSEUDO:
          image_win->AddMessage("Recogging selected blobs");
          recog_pseudo_word(current_block_list, selection_box);
          break;

        default:
          sprintf(msg, "Mode %d not yet implemented", mode);
          image_win->AddMessage(msg);
          break;
      }
    default:
      break;
  }
}
}  // namespace tesseract


/**
 * re_scale_and_move_bln_word()
 *
 * Scale and move a bln word so that it fits in a specified bounding box.
 * Scale by width or height to generate the largest image
 */

float re_scale_and_move_bln_word(                 // put bln word in       box
                                 WERD *norm_word,  //< BL normalised word
                                 const TBOX &box   //< destination box
                                ) {
  TBOX norm_box = norm_word->bounding_box();
  float width_scale_factor;
  float height_scale_factor;
  float selected_scale_factor;

  width_scale_factor = box.width() /(float) norm_box.width();
  height_scale_factor = box.height() /(float) ASC_HEIGHT;

  if ((ASC_HEIGHT * width_scale_factor) <= box.height())
    selected_scale_factor = width_scale_factor;
  else
    selected_scale_factor = height_scale_factor;

  norm_word->scale(selected_scale_factor);
  norm_word->move(ICOORD((box.left() + box.width() / 2), box.bottom()));
  return selected_scale_factor;
}


/**
 * re_segment_word()
 *
 * If all selected blobs are in the same row, remove them from their current
 * word(s) and put them in a new word.  Insert the new word in the row at the
 * appropriate point. Delete any empty words.
 *
 */

void re_segment_word(                        // break/join words
                     BLOCK_LIST *block_list,  // blocks to check
                     TBOX &selection_box) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  BLOCK *block_to_process = NULL;
  ROW_IT row_it;
  ROW *row;
  ROW *row_to_process = NULL;
  WERD_IT word_it;
  WERD *word;
  WERD *new_word = NULL;
  BOOL8 polyg = false;
  PBLOB_IT blob_it;
  PBLOB_LIST dummy;  // Just to initialize new_blob_it.
  PBLOB_IT new_blob_it = &dummy;
  PBLOB *blob;

  /* Find row to process - error if selections from more than one row */

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block = block_it.data();
    if (block->bounding_box().overlap(selection_box)) {
      row_it.set_to_list(block->row_list());
      for (row_it.mark_cycle_pt();
      !row_it.cycled_list(); row_it.forward()) {
        row = row_it.data();
        if (row->bounding_box().overlap(selection_box)) {
          if (row_to_process == NULL) {
            block_to_process = block;
            row_to_process = row;
          }
          else {
	    	image_win->AddMessage("Cant resegment words "
                                     "in more than one row");
            return;
          }
        }
      }
    }
  }
  /* Continue with row_to_process */

  word_it.set_to_list(row_to_process->word_list());
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data();
    polyg = word->flag(W_POLYGON);
    if (word->bounding_box().overlap(selection_box)) {
      blob_it.set_to_list(word->gblob_list());
      for (blob_it.mark_cycle_pt();
      !blob_it.cycled_list(); blob_it.forward()) {
        blob = blob_it.data();
        if (gblob_bounding_box(blob, polyg).overlap(selection_box)) {
          if (new_word == NULL) {
            new_word = word->shallow_copy();
            new_blob_it.set_to_list(new_word->gblob_list());
          }
          new_blob_it.add_to_end(blob_it.extract());
          // move blob
        }
      }
      if (blob_it.empty()) {    // no blobs in word
                                 // so delete word
        delete word_it.extract();
      }
    }
  }
  if (new_word != NULL) {
    gblob_sort_list(new_word->gblob_list(), polyg);
    word_it.add_to_end(new_word);
    word_it.sort(word_comparator);
    row_to_process->bounding_box().plot(image_win,
      ScrollView::BLACK, ScrollView::BLACK);
    word_it.set_to_list(row_to_process->word_list());
    for (word_it.mark_cycle_pt();
      !word_it.cycled_list(); word_it.forward())
    word_display(block_to_process, row_to_process, word_it.data());
    *current_image_changed = TRUE;
  }
}

/// show space stats
void block_space_stat(BLOCK_LIST *block_list,  // blocks to check
                      TBOX &selection_box) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  int block_idx = 0;
  STATS all_gap_stats(0, MAXSPACING);
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT cblob_it;
  C_BLOB *cblob;
  TBOX box;
  inT16 prev_box_right;
  inT16 gap_width;
  inT16 min_inter_word_gap;
  inT16 max_inter_char_gap;

  /* Find blocks to process */

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block_idx++;
    block = block_it.data();
    if (block->bounding_box().overlap(selection_box)) {
      /* Process a block */
      tprintf("\nBlock %d\n", block_idx);
      min_inter_word_gap = 3000;
      max_inter_char_gap = 0;
      all_gap_stats.clear();
      row_it.set_to_list(block->row_list());
      for (row_it.mark_cycle_pt();
      !row_it.cycled_list(); row_it.forward()) {
        row = row_it.data();
        prev_box_right = -1;
        word_it.set_to_list(row->word_list());
        for (word_it.mark_cycle_pt();
        !word_it.cycled_list(); word_it.forward()) {
          word = word_it.data();
          if (word->flag(W_POLYGON)) {
            blob_it.set_to_list(word->blob_list());
            for (blob_it.mark_cycle_pt();
            !blob_it.cycled_list(); blob_it.forward()) {
              blob = blob_it.data();
              box = blob->bounding_box();
              if (prev_box_right > -1) {
                gap_width = box.left() - prev_box_right;
                all_gap_stats.add(gap_width, 1);
                if (blob_it.at_first()) {
                  if (gap_width < min_inter_word_gap)
                    min_inter_word_gap = gap_width;
                }
                else {
                  if (gap_width > max_inter_char_gap)
                    max_inter_char_gap = gap_width;
                }
              }
              prev_box_right = box.right();
            }
          }
          else {
            cblob_it.set_to_list(word->cblob_list());
            for (cblob_it.mark_cycle_pt();
            !cblob_it.cycled_list(); cblob_it.forward()) {
              cblob = cblob_it.data();
              box = cblob->bounding_box();
              if (prev_box_right > -1) {
                gap_width = box.left() - prev_box_right;
                all_gap_stats.add(gap_width, 1);
                if (cblob_it.at_first()) {
                  if (gap_width < min_inter_word_gap)
                    min_inter_word_gap = gap_width;
                }
                else {
                  if (gap_width > max_inter_char_gap)
                    max_inter_char_gap = gap_width;
                }
              }
              prev_box_right = box.right();
            }
          }
        }
      }
      tprintf("Max inter char gap = %d.\nMin inter word gap = %d.\n",
        max_inter_char_gap, min_inter_word_gap);
      all_gap_stats.short_print(NULL, TRUE);
      all_gap_stats.smooth(2);
      tprintf("SMOOTHED DATA...\n");
      all_gap_stats.short_print(NULL, TRUE);
    }
  }
}

/// show space stats
void row_space_stat(BLOCK_LIST *block_list,  // blocks to check
                    TBOX &selection_box) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  int block_idx = 0;
  int row_idx;
  STATS all_gap_stats(0, MAXSPACING);
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT cblob_it;
  C_BLOB *cblob;
  TBOX box;
  inT16 prev_box_right;
  inT16 gap_width;
  inT16 min_inter_word_gap;
  inT16 max_inter_char_gap;

  /* Find rows to process */

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block_idx++;
    block = block_it.data();
    if (block->bounding_box().overlap(selection_box)) {
      row_it.set_to_list(block->row_list());
      row_idx = 0;
      for (row_it.mark_cycle_pt();
      !row_it.cycled_list(); row_it.forward()) {
        row_idx++;
        row = row_it.data();
        if (row->bounding_box().overlap(selection_box)) {
          /* Process a row */

          tprintf("\nBlock %d Row %d\n", block_idx, row_idx);
          min_inter_word_gap = 3000;
          max_inter_char_gap = 0;
          prev_box_right = -1;
          all_gap_stats.clear();
          word_it.set_to_list(row->word_list());
          for (word_it.mark_cycle_pt();
          !word_it.cycled_list(); word_it.forward()) {
            word = word_it.data();
            if (word->flag(W_POLYGON)) {
              blob_it.set_to_list(word->blob_list());
              for (blob_it.mark_cycle_pt();
              !blob_it.cycled_list(); blob_it.forward()) {
                blob = blob_it.data();
                box = blob->bounding_box();
                if (prev_box_right > -1) {
                  gap_width = box.left() - prev_box_right;
                  all_gap_stats.add(gap_width, 1);
                  if (blob_it.at_first()) {
                    if (gap_width < min_inter_word_gap)
                      min_inter_word_gap = gap_width;
                  }
                  else {
                    if (gap_width > max_inter_char_gap)
                      max_inter_char_gap = gap_width;
                  }
                }
                prev_box_right = box.right();
              }
            }
            else {
              cblob_it.set_to_list(word->cblob_list());
              for (cblob_it.mark_cycle_pt();
              !cblob_it.cycled_list(); cblob_it.forward()) {
                cblob = cblob_it.data();
                box = cblob->bounding_box();
                if (prev_box_right > -1) {
                  gap_width = box.left() - prev_box_right;
                  all_gap_stats.add(gap_width, 1);
                  if (cblob_it.at_first()) {
                    if (gap_width < min_inter_word_gap)
                      min_inter_word_gap = gap_width;
                  }
                  else {
                    if (gap_width > max_inter_char_gap)
                      max_inter_char_gap = gap_width;
                  }
                }
                prev_box_right = box.right();
              }
            }
          }
          tprintf
           ("Max inter char gap = %d.\nMin inter word gap = %d.\n",
            max_inter_char_gap, min_inter_word_gap);
          all_gap_stats.short_print(NULL, TRUE);
          all_gap_stats.smooth(2);
          tprintf("SMOOTHED DATA...\n");
          all_gap_stats.short_print(NULL, TRUE);
        }
      }
    }
  }
}


/**
 * show_point()
 *
 * Show coords of point, blob bounding box, word bounding box and offset from
 * row baseline
 */

void show_point(                        // display posn of bloba word
                BLOCK_LIST *block_list,  // blocks to check
                float x,
                float y) {
  FCOORD pt(x, y);
  TBOX box;
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT cblob_it;
  C_BLOB *cblob;

  char msg[160];
  char *msg_ptr = msg;

  msg_ptr += sprintf(msg_ptr, "Pt:(%0.3f, %0.3f) ", x, y);

  for (block_it.mark_cycle_pt();
  !block_it.cycled_list(); block_it.forward()) {
    block = block_it.data();
    if (block->bounding_box().contains(pt)) {
      row_it.set_to_list(block->row_list());
      for (row_it.mark_cycle_pt();
      !row_it.cycled_list(); row_it.forward()) {
        row = row_it.data();
        if (row->bounding_box().contains(pt)) {
          msg_ptr += sprintf(msg_ptr, "BL(x)=%0.3f ",
            row->base_line(x));

          word_it.set_to_list(row->word_list());
          for (word_it.mark_cycle_pt();
          !word_it.cycled_list(); word_it.forward()) {
            word = word_it.data();
            box = word->bounding_box();
            if (box.contains(pt)) {
              msg_ptr += sprintf(msg_ptr,
                "Wd(%d, %d)/(%d, %d) ",
                box.left(), box.bottom(),
                box.right(), box.top());

              if (word->flag(W_POLYGON)) {
                blob_it.set_to_list(word->blob_list());
                for (blob_it.mark_cycle_pt();
                  !blob_it.cycled_list();
                blob_it.forward()) {
                  blob = blob_it.data();
                  box = blob->bounding_box();
                  if (box.contains(pt)) {
                    msg_ptr += sprintf(msg_ptr,
                      "Blb(%d, %d)/(%d, %d) ",
                      box.left(),
                      box.bottom(),
                      box.right(),
                      box.top());
                  }
                }
              }
              else {
                cblob_it.set_to_list(word->cblob_list());
                for (cblob_it.mark_cycle_pt();
                  !cblob_it.cycled_list();
                cblob_it.forward()) {
                  cblob = cblob_it.data();
                  box = cblob->bounding_box();
                  if (box.contains(pt)) {
                    msg_ptr += sprintf(msg_ptr,
                      "CBlb(%d, %d)/(%d, %d) ",
                      box.left(),
                      box.bottom(),
                      box.right(),
                      box.top());
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  image_win->AddMessage(msg);
}


/**********************************************************************
 * WERD PROCESSOR FUNCTIONS
 * ========================
 *
 * These routines are invoked by one or more of:
 *    process_all_words()
 *    process_selected_words()
 * or
 *    process_all_words_it()
 *    process_selected_words_it()
 * for each word to be processed
 **********************************************************************/

/**
 * word_blank_and_set_display()  Word processor
 *
 * Blank display of word then redisplay word according to current display mode
 * settings
 */

BOOL8 word_blank_and_set_display(              // display a word
                                 BLOCK *block,  // block holding word
                                 ROW *row,      // row holding word
                                 WERD *word     // word to be processed
                                ) {
  word->bounding_box().plot(image_win, ScrollView::BLACK, ScrollView::BLACK);
  return word_set_display(block, row, word);
}


/**
 * word_bln_display()
 *
 * Normalise word and display in word window
 */

BOOL8 word_bln_display(           // bln & display
                       BLOCK *,    // block holding word
                       ROW *row,   // row holding word
                       WERD *word  // word to be processed
                      ) {
  WERD *bln_word;

  bln_word = word->poly_copy(row->x_height());
  bln_word->baseline_normalise(row);
  bln_word_window_handle()->Clear();
  display_bln_lines(bln_word_window_handle(), ScrollView::CYAN,
                     1.0, 0.0f, -1000.0f, 1000.0f);
  bln_word->plot(bln_word_window_handle(), ScrollView::RED);
  delete bln_word;
  return TRUE;
}


/**
 * word_change_text()
 *
 * Change the correct text of a word
 */

BOOL8 word_change_text(              // change correct text
                       BLOCK *block,  // block holding word
                       ROW *row,      // row holding word
                       WERD *word     // word to be processed
                      ) {
  char* cp = image_win->ShowInputDialog(
      "Enter/edit the correct text and press <<RETURN>>");
  word->set_text(cp);
  delete[] cp;

  if (word_display_mode.bit(DF_TEXT) || word->display_flag(DF_TEXT)) {
    word_blank_and_set_display(block, row, word);
    ScrollView::Update();
  }

  *current_image_changed = TRUE;
  return TRUE;
}


/**
 * word_copy()
 *
 * Copy a word to other display list
 */

BOOL8 word_copy(              // copy a word
                BLOCK *block,  // block holding word
                ROW *row,      // row holding word
                WERD *word     // word to be processed
               ) {
  WERD *copy_word = new WERD;

  *copy_word = *word;
  add_word(copy_word, row, block, other_block_list);
  *other_image_changed = TRUE;
  return TRUE;
}


/**
 * word_delete()
 *
 * Delete a word
 */

BOOL8 word_delete(                    // delete a word
                  BLOCK *block,        // block holding word
                  ROW *row,            // row holding word
                  WERD *word,          // word to be processed
                  BLOCK_IT &block_it,  // block list iterator
                  ROW_IT &row_it,      // row list iterator
                  WERD_IT &word_it     // word list iterator
                 ) {
  word_it.extract();
  word->bounding_box().plot(image_win, ScrollView::BLACK, ScrollView::BLACK);
  delete(word);

  if (word_it.empty()) {        // no words left in row
                                 // so delete row
    row_it.extract();
    row->bounding_box().plot(image_win, ScrollView::BLACK, ScrollView::BLACK);
    delete(row);

    if (row_it.empty()) {       // no rows left in blk
                                 // so delete block
      block_it.extract();
      block->bounding_box().plot(image_win, ScrollView::BLACK, ScrollView::BLACK);
      delete(block);
    }
  }
  *current_image_changed = TRUE;
  return TRUE;
}


/**
 *  word_display()  Word Processor
 *
 *  Display a word according to its display modes
 */

BOOL8 word_display(           // display a word
                   BLOCK *,    // block holding word
                   ROW *row,   // row holding word
                   WERD *word  // word to be processed
                  ) {
  TBOX word_bb;                   // word bounding box
  int word_height;               // ht of word BB
  BOOL8 displayed_something = FALSE;
  BOOL8 displayed_rainbow = FALSE;
  float shift;                   // from bot left
  PBLOB_IT it;                   // blob iterator
  C_BLOB_IT c_it;                // cblob iterator
  WERD *word_ptr;                // poly copy
  WERD temp_word;
  float scale_factor;            // for BN_POLYGON

  /*
    Note the double coercions of(COLOUR)((inT32)editor_image_word_bb_color)
    etc. are to keep the compiler happy.
  */

                                 // display bounding box
  if (word->display_flag(DF_BOX)) {
    word->bounding_box().plot(image_win,
     (ScrollView::Color)((inT32)
      editor_image_word_bb_color),
     (ScrollView::Color)((inT32)
      editor_image_word_bb_color));

    ScrollView::Color c = (ScrollView::Color)
       ((inT32) editor_image_blob_bb_color);
    image_win->Pen(c);
    if (word->flag(W_POLYGON)) {
      it.set_to_list(word->blob_list());
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())
        it.data()->bounding_box().plot(image_win);
    }
    else {
      c_it.set_to_list(word->cblob_list());
      for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward())
        c_it.data()->bounding_box().plot(image_win);
    }
    displayed_something = TRUE;
  }

                                 // display edge steps
  if (word->display_flag(DF_EDGE_STEP) &&
  !word->flag(W_POLYGON)) {     // edgesteps available
    word->plot(image_win);      // rainbow colors
    displayed_something = TRUE;
    displayed_rainbow = TRUE;
  }

                                 // display poly approx
  if (word->display_flag(DF_POLYGONAL)) {
                                 // need to convert
    if (!word->flag(W_POLYGON)) {
      word_ptr = word->poly_copy(row->x_height());

      /* CALL POLYGONAL APPROXIMATOR WHEN AVAILABLE - on a temp_word */

      if (displayed_rainbow)
                                 // ensure its visible
        word_ptr->plot(image_win, ScrollView::WHITE);
      else
                                 // rainbow colors
          word_ptr->plot(image_win);
      delete word_ptr;
    }
    else {
      if (displayed_rainbow)
                                 // ensure its visible
        word->plot(image_win, ScrollView::WHITE);
      else
        word->plot(image_win);  // rainbow colors
    }

    displayed_rainbow = TRUE;
    displayed_something = TRUE;
  }

                                 // disp BN poly approx
  if (word->display_flag(DF_BN_POLYGONAL)) {
                                 // need to convert
    if (!word->flag(W_POLYGON)) {
      word_ptr = word->poly_copy(row->x_height());
      temp_word = *word_ptr;
      delete word_ptr;

      /* CALL POLYGONAL APPROXIMATOR WHEN AVAILABLE - on a temp_word */

    }
    else
      temp_word = *word;         // copy word
    word_bb = word->bounding_box();
    if (!temp_word.flag(W_NORMALIZED))
      temp_word.baseline_normalise(row);

    scale_factor = re_scale_and_move_bln_word(&temp_word, word_bb);
    display_bln_lines(image_win, ScrollView::CYAN, scale_factor,
      word_bb.bottom(), word_bb.left(), word_bb.right());

    if (displayed_rainbow)
                                 // ensure its visible
      temp_word.plot(image_win, ScrollView::WHITE);
    else
      temp_word.plot(image_win); // rainbow colors

    displayed_rainbow = TRUE;
    displayed_something = TRUE;
  }

  // display correct text
  if (word->display_flag(DF_TEXT)) {
    word_bb = word->bounding_box();
    ScrollView::Color c =(ScrollView::Color)
       ((inT32) editor_image_blob_bb_color);
    image_win->Pen(c);
    word_height = word_bb.height();
    image_win->TextAttributes("Times", 0.75 * word_height,
                              false, false, false);
    if (word_height < word_bb.width())
      shift = 0.25 * word_height;
    else
      shift = 0.0f;

    image_win->Text(word_bb.left() + shift,
                    word_bb.bottom() + 0.25 * word_height, word->text());

    if (strlen(word->text()) > 0)
      displayed_something = TRUE;
  }

  if (!displayed_something)      // display BBox anyway
    word->bounding_box().plot(image_win,
     (ScrollView::Color)((inT32) editor_image_word_bb_color),
     (ScrollView::Color)((inT32)
      editor_image_word_bb_color));
  return TRUE;
}


/**
 * word_dumper()
 *
 * Dump members to the debug window
 */

BOOL8 word_dumper(              // dump word
                  BLOCK *block,  //< block holding word
                  ROW *row,      //< row holding word
                  WERD *word     //< word to be processed
                 ) {

  if (block != NULL) {
    tprintf("\nBlock data...\n");
    block->print(NULL, FALSE);
  }
  tprintf("\nRow data...\n");
  row->print(NULL);
  tprintf("\nWord data...\n");
  word->print(NULL);
  return TRUE;
}


/**
 * word_set_display()  Word processor
 *
 * Display word according to current display mode settings
 */

BOOL8 word_set_display(              // display a word
                       BLOCK *block,  //< block holding word
                       ROW *row,      //< row holding word
                       WERD *word     //< word to be processed
                      ) {
  TBOX word_bb;                   // word bounding box

  word->set_display_flag(DF_BOX, word_display_mode.bit(DF_BOX));
  word->set_display_flag(DF_TEXT, word_display_mode.bit(DF_TEXT));
  word->set_display_flag(DF_POLYGONAL, word_display_mode.bit(DF_POLYGONAL));
  word->set_display_flag(DF_EDGE_STEP, word_display_mode.bit(DF_EDGE_STEP));
  word->set_display_flag(DF_BN_POLYGONAL,
    word_display_mode.bit(DF_BN_POLYGONAL));
  *current_image_changed = TRUE;
  return word_display(block, row, word);
}


/**
 * word_toggle_seg()
 *
 * Toggle the correct segmentation flag
 */

BOOL8 word_toggle_seg(           // toggle seg flag
                      BLOCK *,    //< block holding word
                      ROW *,      //< row holding word
                      WERD *word  //< word to be processed
                     ) {
  word->set_flag(W_SEGMENTED, !word->flag(W_SEGMENTED));
  *current_image_changed = TRUE;
  return TRUE;
}

#endif  // GRAPHICS_DISABLED

/* DEBUG ONLY */

void do_check_mem( // do it
                  inT32 level) {
  check_mem("Doing it", level);
}
