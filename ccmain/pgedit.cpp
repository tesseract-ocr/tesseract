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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include          "pgedit.h"

#include          <ctype.h>
#include          <math.h>

#include          "tordmain.h"
#include          "statistc.h"
#include          "debugwin.h"
#include          "svshowim.h"
#include          "paramsd.h"
#include          "string.h"

#include          "scrollview.h"
#include          "svmnode.h"

#include          "control.h"
#include "tesseractclass.h"

#include          "blread.h"

#ifndef GRAPHICS_DISABLED
#define ASC_HEIGHT     (2 * kBlnBaselineOffset + kBlnXHeight)
#define X_HEIGHT       (kBlnBaselineOffset + kBlnXHeight)
#define BL_HEIGHT     kBlnBaselineOffset
#define DESC_HEIGHT     0
#define MAXSPACING      128      /*max expected spacing in pix */

const ERRCODE EMPTYBLOCKLIST = "No blocks to edit";

enum CMD_EVENTS
{
  NULL_CMD_EVENT,
  CHANGE_DISP_CMD_EVENT,
  DUMP_WERD_CMD_EVENT,
  SHOW_POINT_CMD_EVENT,
  SHOW_BLN_WERD_CMD_EVENT,
  DEBUG_WERD_CMD_EVENT,
  BOUNDING_BOX_CMD_EVENT,
  CORRECT_TEXT_CMD_EVENT,
  POLYGONAL_CMD_EVENT,
  BL_NORM_CMD_EVENT,
  BITMAP_CMD_EVENT,
  IMAGE_CMD_EVENT,
  BLOCKS_CMD_EVENT,
  BASELINES_CMD_EVENT,
  UNIFORM_DISP_CMD_EVENT,
  REFRESH_CMD_EVENT,
  QUIT_CMD_EVENT,
  RECOG_WERDS,
  RECOG_PSEUDO,
  SHOW_SUBSCRIPT_CMD_EVENT,
  SHOW_SUPERSCRIPT_CMD_EVENT,
  SHOW_ITALIC_CMD_EVENT,
  SHOW_BOLD_CMD_EVENT,
  SHOW_UNDERLINE_CMD_EVENT,
  SHOW_FIXEDPITCH_CMD_EVENT,
  SHOW_SERIF_CMD_EVENT,
  SHOW_SMALLCAPS_CMD_EVENT,
  SHOW_DROPCAPS_CMD_EVENT,
};

enum ColorationMode {
  CM_RAINBOW,
  CM_SUBSCRIPT,
  CM_SUPERSCRIPT,
  CM_ITALIC,
  CM_BOLD,
  CM_UNDERLINE,
  CM_FIXEDPITCH,
  CM_SERIF,
  CM_SMALLCAPS,
  CM_DROPCAPS
};

/*
 *
 *  Some global data
 *
 */

ScrollView* image_win;
ParamsEditor* pe;
bool stillRunning = false;

#ifdef __UNIX__
FILE *debug_window = NULL;                // opened on demand
#endif
ScrollView* bln_word_window = NULL;       // baseline norm words

CMD_EVENTS mode = CHANGE_DISP_CMD_EVENT;  // selected words op

// These variables should remain global, since they are only used for the
// debug mode (in which only a single Tesseract thread/instance will be exist).
BITS16 word_display_mode;
static ColorationMode color_mode = CM_RAINBOW;
BOOL8 display_image = FALSE;
BOOL8 display_blocks = FALSE;
BOOL8 display_baselines = FALSE;

PAGE_RES *current_page_res = NULL;

STRING_VAR(editor_image_win_name, "EditorImage",
           "Editor image window name");
INT_VAR(editor_image_xpos, 590, "Editor image X Pos");
INT_VAR(editor_image_ypos, 10, "Editor image Y Pos");
INT_VAR(editor_image_menuheight, 50, "Add to image height for menu bar");
INT_VAR(editor_image_word_bb_color, ScrollView::BLUE,
        "Word bounding box colour");
INT_VAR(editor_image_blob_bb_color, ScrollView::YELLOW,
        "Blob bounding box colour");
INT_VAR(editor_image_text_color, ScrollView::WHITE,
        "Correct text colour");

STRING_VAR(editor_dbwin_name, "EditorDBWin",
           "Editor debug window name");
INT_VAR(editor_dbwin_xpos, 50, "Editor debug window X Pos");
INT_VAR(editor_dbwin_ypos, 500, "Editor debug window Y Pos");
INT_VAR(editor_dbwin_height, 24, "Editor debug window height");
INT_VAR(editor_dbwin_width, 80, "Editor debug window width");

STRING_VAR(editor_word_name, "BlnWords", "BL normalized word window");
INT_VAR(editor_word_xpos, 60, "Word window X Pos");
INT_VAR(editor_word_ypos, 510, "Word window Y Pos");
INT_VAR(editor_word_height, 240, "Word window height");
INT_VAR(editor_word_width, 655, "Word window width");

STRING_VAR(editor_debug_config_file, "", "Config file to apply to single words");

class BlnEventHandler : public SVEventHandler {
 public:
  void Notify(const SVEvent* sv_event) {
    if (sv_event->type == SVET_DESTROY)
      bln_word_window = NULL;
    else if (sv_event->type == SVET_CLICK)
      show_point(current_page_res, sv_event->x, sv_event->y);
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

void build_image_window(int width, int height) {
  if (image_win != NULL) { delete image_win; }
  image_win = new ScrollView(editor_image_win_name.string(),
                             editor_image_xpos, editor_image_ypos,
                             width + 1,
                             height + editor_image_menuheight + 1,
                             width + 1,
                             height + 1,
                             true);
}

/**
 *  display_bln_lines()
 *
 *  Display normalized baseline, x-height, ascender limit and descender limit
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
 *  notify()
 *
 *  Event handler that processes incoming events, either forwarding
 *  them to process_cmd_win_event or process_image_event.
 *
 */

void PGEventHandler::Notify(const SVEvent* event) {
  char myval = '0';
  if (event->type == SVET_POPUP) {
    pe->Notify(event);
  } // These are handled by ParamsEditor
  else if (event->type == SVET_EXIT) { stillRunning = false; }
  else if (event->type == SVET_MENU) {
     if (strcmp(event->parameter, "true") == 0) { myval = 'T'; }
     else if (strcmp(event->parameter, "false") == 0) { myval = 'F'; }
     tess_->process_cmd_win_event(event->command_id, &myval);
  }
  else {
    tess_->process_image_event(*event);
  }
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

  modes_menu_item->AddChild("Change Display", CHANGE_DISP_CMD_EVENT);
  modes_menu_item->AddChild("Dump Word", DUMP_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Show Point", SHOW_POINT_CMD_EVENT);
  modes_menu_item->AddChild("Show BL Norm Word", SHOW_BLN_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Config Words", DEBUG_WERD_CMD_EVENT);
  modes_menu_item->AddChild("Recog Words", RECOG_WERDS);
  modes_menu_item->AddChild("Recog Blobs", RECOG_PSEUDO);

  parent_menu = root_menu_item->AddChild("DISPLAY");

  parent_menu->AddChild("Bounding Boxes", BOUNDING_BOX_CMD_EVENT, FALSE);
  parent_menu->AddChild("Correct Text", CORRECT_TEXT_CMD_EVENT, FALSE);
  parent_menu->AddChild("Polygonal Approx", POLYGONAL_CMD_EVENT, FALSE);
  parent_menu->AddChild("Baseline Normalized", BL_NORM_CMD_EVENT, FALSE);
  parent_menu->AddChild("Edge Steps", BITMAP_CMD_EVENT, TRUE);
  parent_menu->AddChild("Subscripts", SHOW_SUBSCRIPT_CMD_EVENT);
  parent_menu->AddChild("Superscripts", SHOW_SUPERSCRIPT_CMD_EVENT);
  parent_menu->AddChild("Italics", SHOW_ITALIC_CMD_EVENT);
  parent_menu->AddChild("Bold", SHOW_BOLD_CMD_EVENT);
  parent_menu->AddChild("Underline", SHOW_UNDERLINE_CMD_EVENT);
  parent_menu->AddChild("FixedPitch", SHOW_FIXEDPITCH_CMD_EVENT);
  parent_menu->AddChild("Serifs", SHOW_SERIF_CMD_EVENT);
  parent_menu->AddChild("SmallCaps", SHOW_SMALLCAPS_CMD_EVENT);
  parent_menu->AddChild("DropCaps", SHOW_DROPCAPS_CMD_EVENT);


  parent_menu = root_menu_item->AddChild("OTHER");

  parent_menu->AddChild("Quit", QUIT_CMD_EVENT);
  parent_menu->AddChild("Show Image", IMAGE_CMD_EVENT, FALSE);
  parent_menu->AddChild("ShowBlock Outlines", BLOCKS_CMD_EVENT, FALSE);
  parent_menu->AddChild("Show Baselines", BASELINES_CMD_EVENT, FALSE);
  parent_menu->AddChild("Uniform Display", UNIFORM_DISP_CMD_EVENT);
  parent_menu->AddChild("Refresh Display", REFRESH_CMD_EVENT);

  return root_menu_item;
}

/**
 *  do_re_display()
 *
 *  Redisplay page
 */
void Tesseract::do_re_display(
    BOOL8 (tesseract::Tesseract::*word_painter)(BLOCK* block,
                                                ROW* row,
                                                WERD_RES* word_res)) {
  PAGE_RES_IT pr_it(current_page_res);
  int block_count = 1;

  image_win->Clear();
  if (display_image != 0) {
    image_win->Image(pix_binary_, 0, 0);
  }

  for (WERD_RES* word = pr_it.word(); word != NULL; word = pr_it.forward()) {
    (this->*word_painter)(pr_it.block()->block, pr_it.row()->row, word);
    if (display_baselines && pr_it.row() != pr_it.prev_row())
      pr_it.row()->row->plot_baseline(image_win, ScrollView::GREEN);
    if (display_blocks && pr_it.block() != pr_it.prev_block())
      pr_it.block()->block->plot(image_win, block_count++, ScrollView::RED);
  }
  image_win->Update();
}

/**
 *  pgeditor_main()
 *
 *  Top level editor operation:
 *  Setup a new window and an according event handler
 *
 */

void Tesseract::pgeditor_main(int width, int height, PAGE_RES *page_res) {

  current_page_res = page_res;
  if (current_page_res->block_res_list.empty())
    return;

  stillRunning = true;

  build_image_window(width, height);
  word_display_mode.turn_on_bit(DF_EDGE_STEP);
  do_re_display(&tesseract::Tesseract::word_set_display);
#ifndef GRAPHICS_DISABLED
  pe = new ParamsEditor(this, image_win);
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
 * pgeditor_show_point()
 *
 * Display the coordinates of a point in the command window
 */

void pgeditor_show_point( // display coords
                         SVEvent *event) {
  image_win->AddMessage("Pointing at(%d, %d)", event->x, event->y);
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

  color_mode = CM_RAINBOW;
  switch (cmd_event) {
    case NULL_CMD_EVENT:
      break;

    case CHANGE_DISP_CMD_EVENT:
    case DUMP_WERD_CMD_EVENT:
    case SHOW_POINT_CMD_EVENT:
    case SHOW_BLN_WERD_CMD_EVENT:
    case RECOG_WERDS:
    case RECOG_PSEUDO:
      mode =(CMD_EVENTS) cmd_event;
      break;
    case DEBUG_WERD_CMD_EVENT:
      mode = DEBUG_WERD_CMD_EVENT;
      word_config_ = image_win->ShowInputDialog("Config File Name");
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
      do_re_display(&tesseract::Tesseract::word_set_display);
      break;
    case IMAGE_CMD_EVENT:
      display_image =(new_value[0] == 'T');
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case BLOCKS_CMD_EVENT:
      display_blocks =(new_value[0] == 'T');
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case BASELINES_CMD_EVENT:
      display_baselines =(new_value[0] == 'T');
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_SUBSCRIPT_CMD_EVENT:
      color_mode = CM_SUBSCRIPT;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_SUPERSCRIPT_CMD_EVENT:
      color_mode = CM_SUPERSCRIPT;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_ITALIC_CMD_EVENT:
      color_mode = CM_ITALIC;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_BOLD_CMD_EVENT:
      color_mode = CM_BOLD;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_UNDERLINE_CMD_EVENT:
      color_mode = CM_UNDERLINE;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_FIXEDPITCH_CMD_EVENT:
      color_mode = CM_FIXEDPITCH;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_SERIF_CMD_EVENT:
      color_mode = CM_SERIF;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_SMALLCAPS_CMD_EVENT:
      color_mode = CM_SMALLCAPS;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case SHOW_DROPCAPS_CMD_EVENT:
      color_mode = CM_DROPCAPS;
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case REFRESH_CMD_EVENT:
      do_re_display(&tesseract::Tesseract::word_display);
      break;
    case QUIT_CMD_EVENT:
      exit = TRUE;
      ScrollView::Exit();
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
   // The following variable should remain static, since it is used by
   // debug editor, which uses a single Tesseract instance.
  static ICOORD down;
  ICOORD up;
  TBOX selection_box;
  char msg[80];

  switch(event.type) {

    case SVET_SELECTION:
      if (event.type == SVET_SELECTION) {
        down.set_x(event.x + event.x_size);
        down.set_y(event.y + event.y_size);
        if (mode == SHOW_POINT_CMD_EVENT)
          show_point(current_page_res, event.x, event.y);
      }

      up.set_x(event.x);
      up.set_y(event.y);

      selection_box = TBOX(down, up);

      switch(mode) {
        case CHANGE_DISP_CMD_EVENT:
          process_selected_words(
              current_page_res,
              selection_box,
              &tesseract::Tesseract::word_blank_and_set_display);
          break;
       case DUMP_WERD_CMD_EVENT:
          process_selected_words(current_page_res,
                                 selection_box,
                                 &tesseract::Tesseract::word_dumper);
          break;
        case SHOW_BLN_WERD_CMD_EVENT:
          process_selected_words(current_page_res,
                                 selection_box,
                                 &tesseract::Tesseract::word_bln_display);
          break;
        case DEBUG_WERD_CMD_EVENT:
          debug_word(current_page_res, selection_box);
          break;
        case SHOW_POINT_CMD_EVENT:
          break;                 // ignore up event

        case RECOG_WERDS:
          image_win->AddMessage("Recogging selected words");
          this->process_selected_words(current_page_res,
                                       selection_box,
                                       &Tesseract::recog_interactive);
          break;
        case RECOG_PSEUDO:
          image_win->AddMessage("Recogging selected blobs");
          recog_pseudo_word(current_page_res, selection_box);
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

/**
 * debug_word
 *
 * Process the whole image, but load word_config_ for the selected word(s).
 */
void Tesseract::debug_word(PAGE_RES* page_res, const TBOX &selection_box) {
  ResetAdaptiveClassifier();
  recog_all_words(page_res, NULL, &selection_box, word_config_.string(), 0);
}
}  // namespace tesseract


/**
 * show_point()
 *
 * Show coords of point, blob bounding box, word bounding box and offset from
 * row baseline
 */

void show_point(PAGE_RES* page_res, float x, float y) {
  FCOORD pt(x, y);
  PAGE_RES_IT pr_it(page_res);

  char msg[160];
  char *msg_ptr = msg;

  msg_ptr += sprintf(msg_ptr, "Pt:(%0.3f, %0.3f) ", x, y);

  for (WERD_RES* word = pr_it.word(); word != NULL; word = pr_it.forward()) {
    if (pr_it.row() != pr_it.prev_row() &&
        pr_it.row()->row->bounding_box().contains(pt)) {
      msg_ptr += sprintf(msg_ptr, "BL(x)=%0.3f ",
                         pr_it.row()->row->base_line(x));
    }
    if (word->word->bounding_box().contains(pt)) {
      TBOX box = word->word->bounding_box();
      msg_ptr += sprintf(msg_ptr, "Wd(%d, %d)/(%d, %d) ",
                         box.left(), box.bottom(),
                         box.right(), box.top());
      C_BLOB_IT cblob_it(word->word->cblob_list());
      for (cblob_it.mark_cycle_pt();
           !cblob_it.cycled_list();
           cblob_it.forward()) {
        C_BLOB* cblob = cblob_it.data();
        box = cblob->bounding_box();
        if (box.contains(pt)) {
          msg_ptr += sprintf(msg_ptr,
                             "CBlb(%d, %d)/(%d, %d) ",
                             box.left(), box.bottom(),
                             box.right(), box.top());
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
namespace tesseract {
BOOL8 Tesseract:: word_blank_and_set_display(BLOCK* block, ROW* row,
                                             WERD_RES* word_res) {
  word_res->word->bounding_box().plot(image_win, ScrollView::BLACK,
                                      ScrollView::BLACK);
  return word_set_display(block, row, word_res);
}


/**
 * word_bln_display()
 *
 * Normalize word and display in word window
 */
BOOL8 Tesseract::word_bln_display(BLOCK* block, ROW* row, WERD_RES* word_res) {
  TWERD *bln_word = word_res->chopped_word;
  if (bln_word == NULL) {
    word_res->SetupForRecognition(unicharset, false, row, block);
    bln_word = word_res->chopped_word;
  }
  bln_word_window_handle()->Clear();
  display_bln_lines(bln_word_window_handle(), ScrollView::CYAN,
                     1.0, 0.0f, -1000.0f, 1000.0f);
  bln_word->plot(bln_word_window_handle());
  bln_word_window_handle()->Update();
  return TRUE;
}



/**
 *  word_display()  Word Processor
 *
 *  Display a word according to its display modes
 */
BOOL8 Tesseract::word_display(BLOCK* block, ROW* row, WERD_RES* word_res) {
  WERD* word = word_res->word;
  TBOX word_bb;                   // word bounding box
  int word_height;               // ht of word BB
  BOOL8 displayed_something = FALSE;
  float shift;                   // from bot left
  C_BLOB_IT c_it;                // cblob iterator

  if (color_mode != CM_RAINBOW && word_res->box_word != NULL) {
    BoxWord* box_word = word_res->box_word;
    int length = box_word->length();
    int font_id = word_res->fontinfo_id;
    if (font_id < 0) font_id = 0;
    const UnicityTable<FontInfo> &font_table = get_fontinfo_table();
    FontInfo font_info = font_table.get(font_id);
    for (int i = 0; i < length; ++i) {
      ScrollView::Color color = ScrollView::GREEN;
      switch (color_mode) {
        case CM_SUBSCRIPT:
          if (box_word->BlobPosition(i) == SP_SUBSCRIPT)
            color = ScrollView::RED;
          break;
        case CM_SUPERSCRIPT:
          if (box_word->BlobPosition(i) == SP_SUPERSCRIPT)
            color = ScrollView::RED;
          break;
        case CM_ITALIC:
          if (font_info.is_italic())
            color = ScrollView::RED;
          break;
        case CM_BOLD:
          if (font_info.is_bold())
            color = ScrollView::RED;
          break;
        case CM_FIXEDPITCH:
          if (font_info.is_fixed_pitch())
            color = ScrollView::RED;
          break;
        case CM_SERIF:
          if (font_info.is_serif())
            color = ScrollView::RED;
          break;
        case CM_SMALLCAPS:
          if (word_res->small_caps)
            color = ScrollView::RED;
          break;
        case CM_DROPCAPS:
          if (box_word->BlobPosition(i) == SP_DROPCAP)
            color = ScrollView::RED;
          break;
          // TODO(rays) underline is currently completely unsupported.
        case CM_UNDERLINE:
        default:
          break;
      }
      image_win->Pen(color);
      TBOX box = box_word->BlobBox(i);
      image_win->Rectangle(box.left(), box.bottom(), box.right(), box.top());
    }
    return true;
  }
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
    c_it.set_to_list(word->cblob_list());
    for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward())
      c_it.data()->bounding_box().plot(image_win);
    displayed_something = TRUE;
  }

                                 // display edge steps
  if (word->display_flag(DF_EDGE_STEP)) {     // edgesteps available
    word->plot(image_win);      // rainbow colors
    displayed_something = TRUE;
  }

                                 // display poly approx
  if (word->display_flag(DF_POLYGONAL)) {
                                 // need to convert
    TWERD* tword = TWERD::PolygonalCopy(word);
    tword->plot(image_win);
    delete tword;
    displayed_something = TRUE;
  }

  // display correct text
  if (word->display_flag(DF_TEXT) && word->text() != NULL) {
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
BOOL8 Tesseract::word_dumper(BLOCK* block, ROW* row, WERD_RES* word_res) {
  if (block != NULL) {
    tprintf("\nBlock data...\n");
    block->print(NULL, FALSE);
  }
  tprintf("\nRow data...\n");
  row->print(NULL);
  tprintf("\nWord data...\n");
  word_res->word->print();
  return TRUE;
}


/**
 * word_set_display()  Word processor
 *
 * Display word according to current display mode settings
 */
BOOL8 Tesseract::word_set_display(BLOCK* block, ROW* row, WERD_RES* word_res) {
  WERD* word = word_res->word;
  word->set_display_flag(DF_BOX, word_display_mode.bit(DF_BOX));
  word->set_display_flag(DF_TEXT, word_display_mode.bit(DF_TEXT));
  word->set_display_flag(DF_POLYGONAL, word_display_mode.bit(DF_POLYGONAL));
  word->set_display_flag(DF_EDGE_STEP, word_display_mode.bit(DF_EDGE_STEP));
  word->set_display_flag(DF_BN_POLYGONAL,
    word_display_mode.bit(DF_BN_POLYGONAL));
  return word_display(block, row, word_res);
}
}  // namespace tesseract


#endif  // GRAPHICS_DISABLED
