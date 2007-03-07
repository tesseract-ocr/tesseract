/**********************************************************************
 * File:        pgedit.cpp  (Formerly pgeditor.c)
 * Description: Page structure file editor
 * Author:      Phil Cheatle
 * Created:     Thu Oct 10 16:25:24 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include "mfcpch.h"
#include          <ctype.h>
#include          <math.h>
#include          "cmndwin.h"
#include          "genblob.h"
#include          "pgedit.h"
#include          "pgeditx.h"
#include          "tessio.h"
#include          "tessout.h"
#include          "submen.h"
#include          "varblwin.h"
#include          "tordmain.h"
#include          "statistc.h"
#include          "debugwin.h"
#include          "showim.h"
#include          "mainblk.h"
#include          "evnts.h"

#define ASC_HEIGHT      (2 * bln_baseline_offset + bln_x_height)
#define X_HEIGHT        (bln_baseline_offset + bln_x_height)
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
  SMD_CMD_EVENT,
  NEW_SOURCE_CMD_EVENT,
  UNIFORM_DISP_CMD_EVENT,
  REFRESH_CMD_EVENT,
  QUIT_CMD_EVENT
};

#define EXTENDED_MODES_BASE 1000 //for extended cmd ids
#define EXTENDED_OTHER_BASE 2000

/**********************************************************************
 *
 *  Some global data
 *
 **********************************************************************/

RADIO_MENU *modes_menu_item;
RADIO_MENU_LEAF *change_display_menu_item;
SIMPLE_MENU_LEAF *view_menu_item;
RADIO_MENU_LEAF *copy_menu_item;
VARIABLE_MENU_LEAF *write_menu_item;

#ifdef __UNIX__
FILE *debug_window = NULL;       //opened on demand
#endif
                                 //baseline norm words
WINDOW bln_word_window = NO_WINDOW;

CMD_EVENTS mode = CHANGE_DISP_CMD_EVENT;
                                 //Selected words op

BITS16 word_display_mode;
BOOL8 display_image = FALSE;
BOOL8 display_blocks = FALSE;
BOOL8 display_baselines = FALSE;
BOOL8 viewing_source = TRUE;

BLOCK_LIST *source_block_list = NULL;    //image blocks
BLOCK_LIST target_block_list;    //target blocks
BLOCK_LIST *other_block_list = &target_block_list;

BOOL8 source_changed = FALSE;    //Changes not saved
BOOL8 target_changed = FALSE;    //Changes not saved
BOOL8 *other_image_changed = &target_changed;

/* Public globals */

#define EXTERN

                                 //image window
EXTERN WINDOW image_win = NO_WINDOW;
EXTERN COMMAND_WINDOW *command_window;

EXTERN BLOCK_LIST *current_block_list = NULL;
EXTERN BOOL8 *current_image_changed = &source_changed;
EXTERN void (*show_pt_handler) (GRAPHICS_EVENT *) = NULL;

/* Variables */

EXTERN STRING_VAR (editor_image_win_name, "EditorImage",
"Editor image window name");
EXTERN INT_VAR (editor_image_xpos, 590, "Editor image X Pos");
EXTERN INT_VAR (editor_image_ypos, 10, "Editor image Y Pos");
EXTERN INT_VAR (editor_image_height, 680, "Editor image height");
EXTERN INT_VAR (editor_image_width, 655, "Editor image width");
EXTERN INT_VAR (editor_image_word_bb_color, BLUE, "Word bounding box colour");
EXTERN INT_VAR (editor_image_blob_bb_color, YELLOW,
"Blob bounding box colour");
EXTERN INT_VAR (editor_image_text_color, WHITE, "Correct text colour");

EXTERN STRING_VAR (editor_dbwin_name, "EditorDBWin",
"Editor debug window name");
EXTERN INT_VAR (editor_dbwin_xpos, 50, "Editor debug window X Pos");
EXTERN INT_VAR (editor_dbwin_ypos, 500, "Editor debug window Y Pos");
EXTERN INT_VAR (editor_dbwin_height, 24, "Editor debug window height");
EXTERN INT_VAR (editor_dbwin_width, 80, "Editor debug window width");

EXTERN STRING_VAR (editor_word_name, "BlnWords", "BL normalised word window");
EXTERN INT_VAR (editor_word_xpos, 60, "Word window X Pos");
EXTERN INT_VAR (editor_word_ypos, 510, "Word window Y Pos");
EXTERN INT_VAR (editor_word_height, 240, "Word window height");
EXTERN INT_VAR (editor_word_width, 655, "Word window width");

EXTERN double_VAR (editor_smd_scale_factor, 1.0, "Scaling for smd image");

/**********************************************************************
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
 **********************************************************************/

void add_word(                             //to block list
              WERD *word,                  //word to be added
              ROW *src_row,                //source row
              BLOCK *src_block,            //source block
              BLOCK_LIST *dest_block_list  //add to this
             ) {
  BLOCK_IT block_it(dest_block_list);
  BLOCK *block;                  //current block
  BLOCK *dest_block = NULL;      //destination block
  ROW_IT row_it;
  ROW *row;                      //destination row
  ROW *dest_row = NULL;          //destination row
  WERD_IT word_it;
  BOX word_box = word->bounding_box ();
  BOX insert_point_word_box;
  BOOL8 seen_blocks_for_current_file = FALSE;

  block_it.mark_cycle_pt ();
  while (!block_it.cycled_list () && (dest_block == NULL)) {
    block = block_it.data ();
    if ((block->bounding_box ().contains (word_box)) &&
    (strcmp (block->name (), src_block->name ()) == 0)) {
      dest_block = block;        //found dest block
      row_it.set_to_list (block->row_list ());
      row_it.mark_cycle_pt ();
      while ((!row_it.cycled_list ()) && (dest_row == NULL)) {
        row = row_it.data ();
        if (row->bounding_box ().contains (word_box))
          dest_row = row;        //found dest row
        else
          row_it.forward ();
      }
    }
    else
      block_it.forward ();
  }

  if (dest_block == NULL) {      //make a new one
    dest_block = new BLOCK;
    *dest_block = *src_block;

    block_it.set_to_list (dest_block_list);
    for (block_it.mark_cycle_pt ();
    !block_it.cycled_list (); block_it.forward ()) {
      block = block_it.data ();

      if (!seen_blocks_for_current_file &&
        (strcmp (block->name (), dest_block->name ()) == 0))
        seen_blocks_for_current_file = TRUE;

      if (seen_blocks_for_current_file &&
        ((strcmp (block->name (), dest_block->name ()) != 0) ||
        (block->bounding_box ().top () <
        dest_block->bounding_box ().top ())))
        break;
    }

    if (block_it.cycled_list ())
                                 //didn't find insrt pt
      block_it.add_to_end (dest_block);
    else
                                 //did find insert pt
      block_it.add_before_stay_put (dest_block);
  }

  if (dest_row == NULL) {        //make a new one
    dest_row = new ROW;
    *dest_row = *src_row;

    row_it.set_to_list (dest_block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      if (row_it.data ()->bounding_box ().top () <
        dest_row->bounding_box ().top ())
        break;
    }

    if (row_it.cycled_list ())
                                 //didn't find insrt pt
        row_it.add_to_end (dest_row);
    else
                                 //did find insert pt
      row_it.add_before_stay_put (dest_row);
  }

  /* dest_block and dest_row are now found or built and inserted as necessesary
    so add the word to dest row */

  word_it.set_to_list (dest_row->word_list ());
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    if (word_it.data ()->bounding_box ().right () >= word_box.left ())
      break;
  }

  if (word_it.cycled_list ())
    word_it.add_to_end (word);   //didn't find insrt pt
  else {                         //did find insert pt
    insert_point_word_box = word_it.data ()->bounding_box ();
    if (insert_point_word_box.contains (word_box) ||
      word_box.contains (insert_point_word_box))
      command_window->
        msg
        ("Refusing to add words which obliterate, or are obliterated by, others");
    else {
      if (word_it.data ()->bounding_box ().left () >
        word->bounding_box ().left ())
                                 //infront of insert pt
        word_it.add_before_stay_put (word);
      else
                                 //behind insert pt
        word_it.add_after_stay_put (word);
    }
  }
}


/**********************************************************************
 *  bln_word_window_handle()
 *
 *  Return a WINDOW for the word window, creating it if necessary
 **********************************************************************/

WINDOW bln_word_window_handle() {  //return handle
                                 //not opened yet
  if (bln_word_window == NO_WINDOW) {
    pgeditor_msg ("Creating BLN word window...");
                                 // xmin, xmax
    bln_word_window = create_window (editor_word_name.string (), SCROLLINGWIN, editor_word_xpos, editor_word_ypos, editor_word_width, editor_word_height, -2000.0, 2000.0,
      DESC_HEIGHT - 30.0f,
      ASC_HEIGHT + 30.0f,
                                 // ymin, ymax
      TRUE, FALSE, FALSE, FALSE);
    // down event only

    pgeditor_msg ("Creating BLN word window...Done");
  }
  return bln_word_window;
}


/**********************************************************************
 *  build_image_window()
 *
 *  Destroy the existing image window if there is one.  Work out how big the
 *  new window needs to be. Create it and re-display.
 **********************************************************************/

void build_image_window(BOX page_bounding_box) {
  if (image_win != NO_WINDOW)
    destroy_window(image_win);

                                 // xmin
  image_win = create_window (editor_image_win_name.string (), SCROLLINGWIN, editor_image_xpos, editor_image_ypos, editor_image_width, editor_image_height, 0.0,
    (float) page_bounding_box.right () + 1,
  // xmax
    0.0,                         // ymin
    (float) page_bounding_box.top () + 1,
  // ymax
    TRUE, FALSE, TRUE, FALSE);   //down and up only
}


/**********************************************************************
 *  build_menu()
 *
 *  Construct the menu tree used by the command window
 **********************************************************************/

MENU_ROOT *build_menu() {
  NON_RADIO_MENU *parent_menu;
  MENU_ROOT *root_menu_item;

  root_menu_item = new MENU_ROOT ();

  modes_menu_item = new RADIO_MENU ("MODES");
  root_menu_item->add_child (modes_menu_item);

  change_display_menu_item = new RADIO_MENU_LEAF ("Change Display",
    CHANGE_DISP_CMD_EVENT);
  modes_menu_item->add_child (change_display_menu_item);
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Delete",
    DELETE_CMD_EVENT));
  copy_menu_item = new RADIO_MENU_LEAF ("Copy to TARGET", COPY_CMD_EVENT);
  modes_menu_item->add_child (copy_menu_item);
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Change Text",
    CHANGE_TEXT_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Toggle Correct Seg Flg",
    TOGGLE_SEG_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Dump Word",
    DUMP_WERD_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Show Point",
    SHOW_POINT_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Row gaps hist",
    ROW_SPACE_STAT_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Block gaps hist",
    BLOCK_SPACE_STAT_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Show BL Norm Word",
    SHOW_BLN_WERD_CMD_EVENT));
  modes_menu_item->add_child (new RADIO_MENU_LEAF ("Re-Segment Word",
    SEGMENT_WERD_CMD_EVENT));

  parent_menu = new NON_RADIO_MENU ("DISPLAY");
  root_menu_item->add_child (parent_menu);

  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Bounding Boxes",
    BOUNDING_BOX_CMD_EVENT,
    TRUE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Correct Text",
    CORRECT_TEXT_CMD_EVENT,
    FALSE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Polygonal Approx",
    POLYGONAL_CMD_EVENT, FALSE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Baseline Normalised",
    BL_NORM_CMD_EVENT, FALSE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Edge Steps",
    BITMAP_CMD_EVENT, FALSE));

  parent_menu = new NON_RADIO_MENU ("OTHER");
  root_menu_item->add_child (parent_menu);

  parent_menu->add_child (new SIMPLE_MENU_LEAF ("Quit", QUIT_CMD_EVENT));
  parent_menu->add_child (new SIMPLE_MENU_LEAF ("Tidy Target",
    TIDY_CMD_EVENT));
  view_menu_item = new SIMPLE_MENU_LEAF ("View TARGET", VIEW_CMD_EVENT);
  parent_menu->add_child (view_menu_item);
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Show Image",
    IMAGE_CMD_EVENT, FALSE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("ShowBlock Outlines",
    BLOCKS_CMD_EVENT, FALSE));
  parent_menu->add_child (new TOGGLE_MENU_LEAF ("Show Baselines",
    BASELINES_CMD_EVENT, FALSE));
  write_menu_item = new VARIABLE_MENU_LEAF ("Write File",
    WRITE_CMD_EVENT,
    imagebasename.string ());
  parent_menu->add_child (write_menu_item);
  parent_menu->add_child (new SIMPLE_MENU_LEAF ("Make SMD Image",
    SMD_CMD_EVENT));
  parent_menu->add_child (new VARIABLE_MENU_LEAF ("New Source File",
    NEW_SOURCE_CMD_EVENT,
    imagebasename.string ()));
  parent_menu->add_child (new SIMPLE_MENU_LEAF ("Uniform Display",
    UNIFORM_DISP_CMD_EVENT));
  parent_menu->add_child (new SIMPLE_MENU_LEAF ("Refresh Display",
    REFRESH_CMD_EVENT));

                                 //Call driver program
  extend_menu(modes_menu_item,
              EXTENDED_MODES_BASE,
              parent_menu,
              EXTENDED_OTHER_BASE);
  return root_menu_item;
}


/**********************************************************************
 *  debug_window_handle()
 *
 *  Return a FILE* for the debug window, creating it if necessary
 **********************************************************************/

void debug_window_handle() {  //return handle
  //      if      ( debug_winth == NULL )                                                                 //not opened yet
  //      {
  //    pgeditor_msg( "Creating debug window..." );
  //  create_debug_window();
  pgeditor_msg ("Creating debug window...Done");
  //      }
}


/**********************************************************************
 *  display_bln_lines()
 *
 *  Display normalised baseline, x-height, ascender limit and descender limit
 **********************************************************************/

void display_bln_lines(WINDOW window,
                       COLOUR colour,
                       float scale_factor,
                       float y_offset,
                       float minx,
                       float maxx) {
  line_color_index(window, colour);
  line_type(window, SOLID);
  move2d (window, minx, y_offset + scale_factor * DESC_HEIGHT);
  draw2d (window, maxx, y_offset + scale_factor * DESC_HEIGHT);
  move2d (window, minx, y_offset + scale_factor * BL_HEIGHT);
  draw2d (window, maxx, y_offset + scale_factor * BL_HEIGHT);
  move2d (window, minx, y_offset + scale_factor * X_HEIGHT);
  draw2d (window, maxx, y_offset + scale_factor * X_HEIGHT);
  move2d (window, minx, y_offset + scale_factor * ASC_HEIGHT);
  draw2d (window, maxx, y_offset + scale_factor * ASC_HEIGHT);

}


/**********************************************************************
 *  do_new_source()
 *
 *  Change to another source file.  Automatically tidy page first
 *
 **********************************************************************/

void do_new_source(            //serialise
                   char *name  //file name
                  ) {
  FILE *infp;                    //input file
  char msg_str[MAX_CHARS + 1];
  STRING name_str(name);
  char response_str[MAX_CHARS + 1];
  char *token;                   //first response token

  if (source_changed) {
    response_str[0] = '\0';
    command_window->prompt ("Source changes will be LOST.  Continue? (Y/N)",
      response_str);
    token = strtok (response_str, " ");
    if (tolower (token[0]) != 'y')
      return;
  }

                                 //if not file exists
  if (!(infp = fopen (name, "r"))) {
    sprintf (msg_str, "Cant open file " "%s" "", name);
    command_window->msg (msg_str);
    return;
  }

  fclose(infp);
  sprintf (msg_str, "Reading file " "%s" "...", name);
  command_window->msg (msg_str);
  source_block_list->clear ();
                                 //appends to SOURCE
  pgeditor_read_file(name_str, source_block_list);
  source_changed = FALSE;
  command_window->msg ("Doing automatic Tidy Target...");
  viewing_source = FALSE;        //Force viewing source
  do_tidy_cmd();
  command_window->msg ("Doing automatic Tidy Target...Done");
}


/**********************************************************************
 *  do_re_display()
 *
 *  Redisplay page
 **********************************************************************/

void
                                 //function to call
do_re_display (BOOL8 word_painter (
BLOCK *, ROW *, WERD *)) {
  BLOCK_IT block_it(current_block_list);
  BLOCK *block;
  int block_count = 1;

  ROW_IT row_it;
  ROW *row;

  WERD_IT word_it;
  WERD *word;

  clear_view_surface(image_win);
  if (display_image) {
    show_sub_image (&page_image, 0, 0,
      page_image.get_xsize (), page_image.get_ysize (),
      image_win, 0, 0);
  }

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        word_painter(block, row, word);
      }
      if (display_baselines)
        row->plot_baseline (image_win, GREEN);
    }
    if (display_blocks)
      block->plot (image_win, block_count++, RED);
  }
}


/**********************************************************************
 *  do_tidy_cmd()
 *
 *  Tidy TARGET page
 **********************************************************************/

const BOX do_tidy_cmd() {  //tidy
  ICOORD shift_vector;
  BOX tidy_box;                  //Just the tidy area
  BOX source_box;                //source file area

  source_box = block_list_bounding_box (source_block_list);
  //find src area

  if (!target_block_list.empty ()) {
    tidy_box = block_list_compress (&target_block_list);

    /* Shift tidied target above the source image area. */

    shift_vector = ICOORD (0, source_box.top () + BLOCK_SPACING)
      - tidy_box.botleft ();
    block_list_move(&target_block_list, shift_vector);
    tidy_box.move (shift_vector);
  }
  source_box += tidy_box;
                                 //big enough for both
  build_image_window(source_box);
  do_view_cmd();
  return tidy_box;
}


/**********************************************************************
 *  do_view_cmd()
 *
 *  View TARGET/View SOURCE command
 **********************************************************************/

void do_view_cmd() {
  viewing_source = !viewing_source;
  clear_view_surface(image_win);
  if (viewing_source) {
    current_block_list = source_block_list;
    current_image_changed = &source_changed;
    other_block_list = &target_block_list;
    other_image_changed = &target_changed;
    do_re_display(&word_display);

    command_window->replace_menu_text (view_menu_item, "View TARGET");
    command_window->replace_menu_text (copy_menu_item, "Copy to TARGET");
    write_menu_item->replace_value (imagebasename.string ());
  }
  else {
    current_block_list = &target_block_list;
    current_image_changed = &target_changed;
    other_block_list = source_block_list;
    other_image_changed = &source_changed;
    do_re_display(&word_display);

    command_window->replace_menu_text (view_menu_item, "View SOURCE");
    command_window->replace_menu_text (copy_menu_item, "");
    write_menu_item->replace_value ((imagebasename + ".bits.pg").string ());
  }
}


/**********************************************************************
 *  do_write_file()
 *
 *  Serialise a block list to file
 *
 *  If writing image, tidy page and move to (0,0) first
 **********************************************************************/

void do_write_file(            //serialise
                   char *name  //file name
                  ) {
  FILE *infp;                    //input file
  char msg_str[MAX_CHARS + 1];
  char response_str[MAX_CHARS + 1];
  char *token;                   //first response token
  BOX enclosing_box;

                                 //if file exists
  if ((infp = fopen (name, "r")) != NULL) {
    fclose(infp);
    sprintf (msg_str, "Overwrite file " "%s" "? (Y/N)", name);
    response_str[0] = '\0';
    if (!command_window->prompt (msg_str, response_str))
      return;
    token = strtok (response_str, " ");
    if (tolower (token[0]) != 'y')
      return;                    // dont write
  }

  infp = fopen (name, "w");      //can we write to it?
  if (infp == NULL) {
    sprintf (msg_str, "Cant write to file " "%s" "", name);
    command_window->msg (msg_str);
    return;
  }
  fclose(infp);

  if (!viewing_source && !target_block_list.empty ()) {
                                 //Tidy & move to (0,0)
    command_window->msg ("Automatic tidy...");
    viewing_source = TRUE;       //Stay viewing target!
    enclosing_box = do_tidy_cmd ();
    block_list_move (&target_block_list, -enclosing_box.botleft ());
    command_window->msg ("Writing file...");
    pgeditor_write_file(name, &target_block_list);
                                 //move back
    block_list_move (&target_block_list,
      enclosing_box.botleft ());
  }
  else {
    command_window->msg ("Writing file...");
    pgeditor_write_file(name, current_block_list);
  }
  command_window->msg ("Writing file...Done");
  *current_image_changed = FALSE;
}


void smd_cmd() {
  char response_str[MAX_CHARS + 1];
  WINDOW display_window;         //temp
  ICOORD tr, bl;
  BOX page_box = block_list_bounding_box (current_block_list);

  bl = ICOORD (0, 0);
  tr = ICOORD (page_image.get_xsize () + 1, page_image.get_ysize () + 1);
  page_box += BOX (bl, tr);

  strcpy (response_str, imagebasename.string ());
  strcpy (response_str + imagebasename.length (), ".smd.tif");
  command_window->prompt ("SMD File Name?", response_str);

  display_window = image_win;
                                 // xmin
  image_win = create_window (response_str, SMDWINDOW, 0, 0, (INT16) (page_box.width () * editor_smd_scale_factor), (INT16) (page_box.height () * editor_smd_scale_factor), 0.0,
    page_box.width (),           // xmax
    0.0,                         // ymin
    page_box.height (),          // ymax
    FALSE, FALSE, FALSE, FALSE); //down and up only
  do_re_display(&word_display);
  destroy_window(image_win);  //Dumps sbd file
  image_win = display_window;
}


/**********************************************************************
 *  pgeditor_main()
 *
 *  Top level editor operation:
 *  Read events and send them to the appropriate command processor.
 *
 **********************************************************************/

void pgeditor_main(BLOCK_LIST *blocks) {
  GRAPHICS_EVENT event;
  INT32 cmd_event = 0;
  char new_value[MAX_CHARS + 1];
  BOOL8 exit = FALSE;

  source_block_list = blocks;
  current_block_list = blocks;
  if (current_block_list->empty ())
    return;

  command_window = new COMMAND_WINDOW ("WordEditorCmd", build_menu ());
  build_image_window (block_list_bounding_box (source_block_list));
  do_re_display(&word_display);
  word_display_mode.turn_on_bit (DF_BOX);

  while (!exit) {
    overlap_picture_ops(TRUE);
    await_event (0,              //all windows
      TRUE,                      //wait for event
      ANY_EVENT, &event);
                                 //Command win event
    if (event.fd == command_window->window ()) {
      command_window->msg ("");  //Clear old message
      command_window->event (event, &cmd_event, new_value);
      exit = process_cmd_win_event (cmd_event, new_value);
    }
    else {
      if (event.fd == image_win)
        process_image_event(event);
      else
        pgeditor_show_point(&event);
    }
    current_word_quit.set_value (FALSE);
    selection_quit.set_value (FALSE);
                                 //replot all var wins
    VARIABLES_WINDOW::plot_all();
  }
}


/**********************************************************************
 * pgeditor_msg()
 *
 * Display a message - in the command window if there is one, or to stdout
 **********************************************************************/

void pgeditor_msg(  //message display
                  const char *msg) {
  if (command_window == NO_WINDOW) {
    tprintf(msg);
    tprintf ("\n");
  }
  else
    command_window->msg (msg);
}


/**********************************************************************
 * pgeditor_read_file()
 *
 * Deserialise source file
 **********************************************************************/

void pgeditor_read_file(                    //of serialised file
                        STRING &name,
                        BLOCK_LIST *blocks  //block list to add to
                       ) {
  int c;                         //input character
  FILE *infp;                    //input file
  BLOCK_IT block_it(blocks);  //iterator
  BLOCK *block;                  //current block

  ICOORD page_tr;                //topright of page

  char *filename_extension;

  block_it.move_to_last ();

                                 // ptr to last dot
  filename_extension = strrchr (name.string (), '.');
  #ifdef __UNIX__
  /*    TEXTROW*                tessrows;
      TBLOB*                  tessblobs;
      TPOINT                  tess_tr;

    if (strcmp( filename_extension, ".r" ) == 0)
    {
      tprintf( "Converting from .r file format.\n" );
      tessrows = get_tess_row_file( name.string(),	//get the row file
                          &tess_tr );
      page_tr = ICOORD( tess_tr.x, tess_tr.y );
      make_blocks_from_rows( tessrows, name.string(),	//reconstruct blocks
                    page_tr, TRUE, &block_it );
    }
    else if (strcmp( filename_extension, ".b" ) == 0)
    {
      tprintf( "Converting from .b file format.\n" );
      tessblobs = get_tess_blob_file( name.string(),	//get the blob file
                          &tess_tr );
      page_tr = ICOORD( tess_tr.x, tess_tr.y );
      make_blocks_from_blobs( tessblobs, name.string(),
                              //reconstruct blocks
                    page_tr, FALSE,blocks);
    }
     else*/
  if (strcmp (filename_extension, ".pb") == 0) {
    tprintf ("Converting from .pb file format.\n");
                                 //construct blocks
    read_and_textord (name.string (), blocks);
  }
  else
  #endif
  if ((strcmp (filename_extension, ".pg") == 0) ||
    // read a .pg file
                               // or a .sp file
  (strcmp (filename_extension, ".sp") == 0)) {
    tprintf ("Reading %s file format.\n", filename_extension);
    infp = fopen (name.string (), "r");
    if (infp == NULL)
      CANTOPENFILE.error ("pgeditor_read_file", EXIT, name.string ());
    //can't open file

    while (((c = fgetc (infp)) != EOF) && (ungetc (c, infp) != EOF)) {
                               //get one
      block = BLOCK::de_serialise (infp);
                               //add to list
      block_it.add_after_then_move (block);
    }
    fclose(infp);
  } else {
    edges_and_textord (name.string (), blocks);
  }
}


/**********************************************************************
 * pgeditor_show_point()
 *
 * Display the coordinates of a point in the command window
 **********************************************************************/

void pgeditor_show_point(  //display coords
                         GRAPHICS_EVENT *event) {
  char msg[160];

  sprintf (msg, "Pointing at (%f, %f)", event->x, event->y);
  command_window->msg (msg);
}


/**********************************************************************
 *  pgeditor_write_file()
 *
 *  Serialise a block list to file
 *
 **********************************************************************/

void pgeditor_write_file(                    //serialise
                         char *name,         //file name
                         BLOCK_LIST *blocks  //block list to write
                        ) {
  FILE *infp;                    //input file
  BLOCK_IT block_it(blocks);  //block iterator
  BLOCK *block;                  //current block
  ROW_IT row_it;                 //row iterator

  infp = fopen (name, "w");      //create output file
  if (infp == NULL)
    CANTCREATEFILE.error ("pgeditor_write_file", EXIT, name);

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.extract ();

    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ())
                                 //ensure correct
      row_it.data ()->recalc_bounding_box ();

    block->serialise (infp);     //serialize     non-empty
    block_it.add_after_then_move (block);
  }
  fclose(infp);
}


/**********************************************************************
 *  process_cmd_win_event()
 *
 *  Process a command returned from the command window
 *  (Just call the appropriate command handler)
 **********************************************************************/

BOOL8 process_cmd_win_event(                  //UI command semantics
                            INT32 cmd_event,  //which menu item?
                            char *new_value   //any prompt data
                           ) {
  char msg[160];
  BOOL8 exit = FALSE;
  char response_str[MAX_CHARS + 1];
  char *token;                   //first response token

  switch (cmd_event) {
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
      mode = (CMD_EVENTS) cmd_event;
      break;
    case COPY_CMD_EVENT:
      mode = (CMD_EVENTS) cmd_event;
      if (!viewing_source)
        command_window->msg ("Can't COPY while viewing target!");
      break;
    case BOUNDING_BOX_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit (DF_BOX);
      else
        word_display_mode.turn_off_bit (DF_BOX);
      command_window->press_radio_button (modes_menu_item,
        change_display_menu_item);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case CORRECT_TEXT_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit (DF_TEXT);
      else
        word_display_mode.turn_off_bit (DF_TEXT);
      command_window->press_radio_button (modes_menu_item,
        change_display_menu_item);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case POLYGONAL_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit (DF_POLYGONAL);
      else
        word_display_mode.turn_off_bit (DF_POLYGONAL);
      command_window->press_radio_button (modes_menu_item,
        change_display_menu_item);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case BL_NORM_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit (DF_BN_POLYGONAL);
      else
        word_display_mode.turn_off_bit (DF_BN_POLYGONAL);
      command_window->press_radio_button (modes_menu_item,
        change_display_menu_item);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case BITMAP_CMD_EVENT:
      if (new_value[0] == 'T')
        word_display_mode.turn_on_bit (DF_EDGE_STEP);
      else
        word_display_mode.turn_off_bit (DF_EDGE_STEP);
      command_window->press_radio_button (modes_menu_item,
        change_display_menu_item);
      mode = CHANGE_DISP_CMD_EVENT;
      break;
    case UNIFORM_DISP_CMD_EVENT:
      do_re_display(&word_set_display);
      *current_image_changed = TRUE;
      break;
    case WRITE_CMD_EVENT:
      do_write_file(new_value);
      break;
    case SMD_CMD_EVENT:
      smd_cmd();
      break;
    case TIDY_CMD_EVENT:
      if (!target_block_list.empty ()) {
        viewing_source = TRUE;   //Force viewing target
        do_tidy_cmd();
      }
      break;
    case NEW_SOURCE_CMD_EVENT:
      do_new_source(new_value);
      break;
    case IMAGE_CMD_EVENT:
      display_image = (new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case BLOCKS_CMD_EVENT:
      display_blocks = (new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case BASELINES_CMD_EVENT:
      display_baselines = (new_value[0] == 'T');
      do_re_display(&word_display);
      break;
    case REFRESH_CMD_EVENT:
      do_re_display(&word_display);
      break;
    case QUIT_CMD_EVENT:
      if (source_changed || target_changed) {
        response_str[0] = '\0';
        command_window->prompt ("Changes not saved. Exit anyway? (Y/N)",
          response_str);
        token = strtok (response_str, " ");
        if (tolower (token[0]) == 'y')
          exit = TRUE;
      }
      else
        exit = TRUE;
      break;
    default:
      if ((cmd_event >= EXTENDED_MODES_BASE) &&
        (cmd_event < EXTENDED_OTHER_BASE))
        mode = (CMD_EVENTS) cmd_event;
      else {
        if (cmd_event >= EXTENDED_OTHER_BASE)
          extend_unmoded_commands (cmd_event - EXTENDED_OTHER_BASE,
            new_value);
        else {
          sprintf (msg, "Unrecognised event " INT32FORMAT " (%s)",
            cmd_event, new_value);
          command_window->msg (msg);
        }
      }
      break;
  }
  return exit;
}


/**********************************************************************
 * process_image_event()
 *
 * User has done something in the image window - mouse down or up.  Work out
 * what it is and do something with it.
 * If DOWN - just remember where it was.
 * If UP - for each word in the selected area do the operation defined by
 * the current mode.
 **********************************************************************/

void process_image_event(  //action in image win
                         GRAPHICS_EVENT event) {
  static ICOORD down;
  ICOORD up;
  BOX selection_box;
  char msg[80];

  switch (event.type) {
    case DOWN_EVENT:
      down.set_x ((INT16) floor (event.x + 0.5));
      down.set_y ((INT16) floor (event.y + 0.5));
      if (mode == SHOW_POINT_CMD_EVENT)
        show_point (current_block_list, event.x, event.y);
      break;

    case UP_EVENT:
    case SELECT_EVENT:
      if (event.type == SELECT_EVENT) {
        down.set_x ((INT16) floor (event.xmax + 0.5));
        down.set_y ((INT16) floor (event.ymax + 0.5));
        if (mode == SHOW_POINT_CMD_EVENT)
          show_point (current_block_list, event.x, event.y);
      }
      if (mode != SHOW_POINT_CMD_EVENT)
        command_window->msg ("");//Clear old message
      up.set_x ((INT16) floor (event.x + 0.5));
      up.set_y ((INT16) floor (event.y + 0.5));
      selection_box = BOX (up, down);

      switch (mode) {
        case CHANGE_DISP_CMD_EVENT:
          process_selected_words(current_block_list,
                                 selection_box,
                                 &word_blank_and_set_display);
          break;
        case COPY_CMD_EVENT:
          if (!viewing_source)
            command_window->msg ("Can't COPY while viewing target!");
          else
            process_selected_words(current_block_list,
                                   selection_box,
                                   &word_copy);
          break;
        case DELETE_CMD_EVENT:
          process_selected_words_it(current_block_list,
                                    selection_box,
                                    &word_delete);
          break;
        case CHANGE_TEXT_CMD_EVENT:
          process_selected_words(current_block_list,
                                 selection_box,
                                 &word_change_text);
          break;
        case TOGGLE_SEG_CMD_EVENT:
          process_selected_words(current_block_list,
                                 selection_box,
                                 &word_toggle_seg);
          break;
        case DUMP_WERD_CMD_EVENT:
          process_selected_words(current_block_list,
                                 selection_box,
                                 &word_dumper);
          break;
        case SHOW_BLN_WERD_CMD_EVENT:
          process_selected_words(current_block_list,
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
          break;                 //ignore up event
        default:
          if ((mode >= EXTENDED_MODES_BASE) && (mode < EXTENDED_OTHER_BASE))
            extend_moded_commands (mode - EXTENDED_MODES_BASE, selection_box);
          else {
            sprintf (msg, "Mode %d not yet implemented", mode);
            command_window->msg (msg);
          }
          break;
      }
    default:
      break;
  }
}


/**********************************************************************
 * re_scale_and_move_bln_word()
 *
 * Scale and move a bln word so that it fits in a specified bounding box.
 * Scale by width or height to generate the largest image
 **********************************************************************/

float re_scale_and_move_bln_word(                  //put bln word in       box
                                 WERD *norm_word,  //BL normalised word
                                 const BOX &box    //destination box
                                ) {
  BOX norm_box = norm_word->bounding_box ();
  float width_scale_factor;
  float height_scale_factor;
  float selected_scale_factor;

  width_scale_factor = box.width () / (float) norm_box.width ();
  height_scale_factor = box.height () / (float) ASC_HEIGHT;

  if ((ASC_HEIGHT * width_scale_factor) <= box.height ())
    selected_scale_factor = width_scale_factor;
  else
    selected_scale_factor = height_scale_factor;

  norm_word->scale (selected_scale_factor);
  norm_word->move (ICOORD ((box.left () + box.width () / 2), box.bottom ()));
  return selected_scale_factor;
}


/**********************************************************************
 * re_segment_word()
 *
 * If all selected blobs are in the same row, remove them from their current
 * word(s) and put them in a new word.  Insert the new word in the row at the
 * appropriate point. Delete any empty words.
 *
 **********************************************************************/

void re_segment_word(                         //break/join words
                     BLOCK_LIST *block_list,  //blocks to check
                     BOX &selection_box) {
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

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          if (row_to_process == NULL) {
            block_to_process = block;
            row_to_process = row;
          }
          else {
            command_window->
              msg ("Cant resegment words in more than one row");
            return;
          }
        }
      }
    }
  }
  /* Continue with row_to_process */

  word_it.set_to_list (row_to_process->word_list ());
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();
    polyg = word->flag (W_POLYGON);
    if (word->bounding_box ().overlap (selection_box)) {
      blob_it.set_to_list (word->gblob_list ());
      for (blob_it.mark_cycle_pt ();
      !blob_it.cycled_list (); blob_it.forward ()) {
        blob = blob_it.data ();
        if (gblob_bounding_box (blob, polyg).overlap (selection_box)) {
          if (new_word == NULL) {
            new_word = word->shallow_copy ();
            new_blob_it.set_to_list (new_word->gblob_list ());
          }
          new_blob_it.add_to_end (blob_it.extract ());
          //move blob
        }
      }
      if (blob_it.empty ()) {    //no blobs in word
                                 //so delete word
        delete word_it.extract ();
      }
    }
  }
  if (new_word != NULL) {
    gblob_sort_list (new_word->gblob_list (), polyg);
    word_it.add_to_end (new_word);
    word_it.sort (word_comparator);
    row_to_process->bounding_box ().plot (image_win,
      INT_SOLID, FALSE, BLACK, BLACK);
    word_it.set_to_list (row_to_process->word_list ());
    for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ())
    word_display (block_to_process, row_to_process, word_it.data ());
    *current_image_changed = TRUE;
  }
}


void block_space_stat(                         //show space stats
                      BLOCK_LIST *block_list,  //blocks to check
                      BOX &selection_box) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  int block_idx = 0;
  STATS all_gap_stats (0, MAXSPACING);
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT cblob_it;
  C_BLOB *cblob;
  BOX box;
  INT16 prev_box_right;
  INT16 gap_width;
  INT16 min_inter_word_gap;
  INT16 max_inter_char_gap;

  /* Find blocks to process */

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block_idx++;
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      /* Process a block */
      tprintf ("\nBlock %d\n", block_idx);
      min_inter_word_gap = 3000;
      max_inter_char_gap = 0;
      all_gap_stats.clear ();
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        prev_box_right = -1;
        word_it.set_to_list (row->word_list ());
        for (word_it.mark_cycle_pt ();
        !word_it.cycled_list (); word_it.forward ()) {
          word = word_it.data ();
          if (word->flag (W_POLYGON)) {
            blob_it.set_to_list (word->blob_list ());
            for (blob_it.mark_cycle_pt ();
            !blob_it.cycled_list (); blob_it.forward ()) {
              blob = blob_it.data ();
              box = blob->bounding_box ();
              if (prev_box_right > -1) {
                gap_width = box.left () - prev_box_right;
                all_gap_stats.add (gap_width, 1);
                if (blob_it.at_first ()) {
                  if (gap_width < min_inter_word_gap)
                    min_inter_word_gap = gap_width;
                }
                else {
                  if (gap_width > max_inter_char_gap)
                    max_inter_char_gap = gap_width;
                }
              }
              prev_box_right = box.right ();
            }
          }
          else {
            cblob_it.set_to_list (word->cblob_list ());
            for (cblob_it.mark_cycle_pt ();
            !cblob_it.cycled_list (); cblob_it.forward ()) {
              cblob = cblob_it.data ();
              box = cblob->bounding_box ();
              if (prev_box_right > -1) {
                gap_width = box.left () - prev_box_right;
                all_gap_stats.add (gap_width, 1);
                if (cblob_it.at_first ()) {
                  if (gap_width < min_inter_word_gap)
                    min_inter_word_gap = gap_width;
                }
                else {
                  if (gap_width > max_inter_char_gap)
                    max_inter_char_gap = gap_width;
                }
              }
              prev_box_right = box.right ();
            }
          }
        }
      }
      tprintf ("Max inter char gap = %d.\nMin inter word gap = %d.\n",
        max_inter_char_gap, min_inter_word_gap);
      all_gap_stats.short_print (NULL, TRUE);
      all_gap_stats.smooth (2);
      tprintf ("SMOOTHED DATA...\n");
      all_gap_stats.short_print (NULL, TRUE);
    }
  }
}


void row_space_stat(                         //show space stats
                    BLOCK_LIST *block_list,  //blocks to check
                    BOX &selection_box) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  int block_idx = 0;
  int row_idx;
  STATS all_gap_stats (0, MAXSPACING);
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT cblob_it;
  C_BLOB *cblob;
  BOX box;
  INT16 prev_box_right;
  INT16 gap_width;
  INT16 min_inter_word_gap;
  INT16 max_inter_char_gap;

  /* Find rows to process */

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block_idx++;
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      row_idx = 0;
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row_idx++;
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          /* Process a row */

          tprintf ("\nBlock %d Row %d\n", block_idx, row_idx);
          min_inter_word_gap = 3000;
          max_inter_char_gap = 0;
          prev_box_right = -1;
          all_gap_stats.clear ();
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt ();
          !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->flag (W_POLYGON)) {
              blob_it.set_to_list (word->blob_list ());
              for (blob_it.mark_cycle_pt ();
              !blob_it.cycled_list (); blob_it.forward ()) {
                blob = blob_it.data ();
                box = blob->bounding_box ();
                if (prev_box_right > -1) {
                  gap_width = box.left () - prev_box_right;
                  all_gap_stats.add (gap_width, 1);
                  if (blob_it.at_first ()) {
                    if (gap_width < min_inter_word_gap)
                      min_inter_word_gap = gap_width;
                  }
                  else {
                    if (gap_width > max_inter_char_gap)
                      max_inter_char_gap = gap_width;
                  }
                }
                prev_box_right = box.right ();
              }
            }
            else {
              cblob_it.set_to_list (word->cblob_list ());
              for (cblob_it.mark_cycle_pt ();
              !cblob_it.cycled_list (); cblob_it.forward ()) {
                cblob = cblob_it.data ();
                box = cblob->bounding_box ();
                if (prev_box_right > -1) {
                  gap_width = box.left () - prev_box_right;
                  all_gap_stats.add (gap_width, 1);
                  if (cblob_it.at_first ()) {
                    if (gap_width < min_inter_word_gap)
                      min_inter_word_gap = gap_width;
                  }
                  else {
                    if (gap_width > max_inter_char_gap)
                      max_inter_char_gap = gap_width;
                  }
                }
                prev_box_right = box.right ();
              }
            }
          }
          tprintf
            ("Max inter char gap = %d.\nMin inter word gap = %d.\n",
            max_inter_char_gap, min_inter_word_gap);
          all_gap_stats.short_print (NULL, TRUE);
          all_gap_stats.smooth (2);
          tprintf ("SMOOTHED DATA...\n");
          all_gap_stats.short_print (NULL, TRUE);
        }
      }
    }
  }
}


/**********************************************************************
 * show_point()
 *
 * Show coords of point, blob bounding box, word bounding box and offset from
 * row baseline
 **********************************************************************/

void show_point(                         //display posn of bloba word
                BLOCK_LIST *block_list,  //blocks to check
                float x,
                float y) {
  FCOORD pt(x, y);
  BOX box;
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

  msg_ptr += sprintf (msg_ptr, "Pt:(%0.3f, %0.3f) ", x, y);

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().contains (pt)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().contains (pt)) {
          msg_ptr += sprintf (msg_ptr, "BL(x)=%0.3f ",
            row->base_line (x));

          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt ();
          !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            box = word->bounding_box ();
            if (box.contains (pt)) {
              msg_ptr += sprintf (msg_ptr,
                "Wd(%d, %d)/(%d, %d) ",
                box.left (), box.bottom (),
                box.right (), box.top ());

              if (word->flag (W_POLYGON)) {
                blob_it.set_to_list (word->blob_list ());
                for (blob_it.mark_cycle_pt ();
                  !blob_it.cycled_list ();
                blob_it.forward ()) {
                  blob = blob_it.data ();
                  box = blob->bounding_box ();
                  if (box.contains (pt)) {
                    msg_ptr += sprintf (msg_ptr,
                      "Blb(%d, %d)/(%d, %d) ",
                      box.left (),
                      box.bottom (),
                      box.right (),
                      box.top ());
                  }
                }
              }
              else {
                cblob_it.set_to_list (word->cblob_list ());
                for (cblob_it.mark_cycle_pt ();
                  !cblob_it.cycled_list ();
                cblob_it.forward ()) {
                  cblob = cblob_it.data ();
                  box = cblob->bounding_box ();
                  if (box.contains (pt)) {
                    msg_ptr += sprintf (msg_ptr,
                      "CBlb(%d, %d)/(%d, %d) ",
                      box.left (),
                      box.bottom (),
                      box.right (),
                      box.top ());
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  command_window->msg (msg);
}


/**********************************************************************
 * WERD PROCESSOR FUNCTIONS
 * ========================
 *
 * These routines are invoked by one or mode of:
 *    process_all_words()
 *    process_selected_words()
 * or
 *    process_all_words_it()
 *    process_selected_words_it()
 * for each word to be processed
 **********************************************************************/

/**********************************************************************
 * word_blank_and_set_display()  Word processor
 *
 * Blank display of word then redisplay word according to current display mode
 * settings
 **********************************************************************/

BOOL8 word_blank_and_set_display(               //display a word
                                 BLOCK *block,  //block holding word
                                 ROW *row,      //row holding word
                                 WERD *word     //word to be processed
                                ) {
  word->bounding_box ().plot (image_win, INT_SOLID, FALSE, BLACK, BLACK);
  return word_set_display (block, row, word);
}


/**********************************************************************
 * word_bln_display()
 *
 * Normalise word and display in word window
 **********************************************************************/

BOOL8 word_bln_display(            //bln & display
                       BLOCK *,    //block holding word
                       ROW *row,   //row holding word
                       WERD *word  //word to be processed
                      ) {
  WERD *bln_word;

  bln_word = word->poly_copy (row->x_height ());
  bln_word->baseline_normalise (row);
  clear_view_surface (bln_word_window_handle ());
  display_bln_lines (bln_word_window_handle (), CYAN, 1.0, 0.0f, -1000.0f,
    1000.0f);
  bln_word->plot (bln_word_window_handle (), RED);
  delete bln_word;
  return TRUE;
}


/**********************************************************************
 * word_change_text()
 *
 * Change the correct text of a word
 **********************************************************************/

BOOL8 word_change_text(               //change correct text
                       BLOCK *block,  //block holding word
                       ROW *row,      //row holding word
                       WERD *word     //word to be processed
                      ) {
  char response_str[MAX_CHARS + 1];

  strcpy (response_str, word->text ());
  if (!command_window->
    prompt ("Enter/edit the correct text and press <<RETURN>>",
    response_str))
    return FALSE;
  else
    word->set_text (response_str);

  if (word_display_mode.bit (DF_TEXT) || word->display_flag (DF_TEXT)) {
    word_blank_and_set_display(block, row, word);
    overlap_picture_ops(TRUE);
  }

  *current_image_changed = TRUE;
  return TRUE;
}


/**********************************************************************
 * word_copy()
 *
 * Copy a word to other display list
 **********************************************************************/

BOOL8 word_copy(               //copy a word
                BLOCK *block,  //block holding word
                ROW *row,      //row holding word
                WERD *word     //word to be processed
               ) {
  WERD *copy_word = new WERD;

  *copy_word = *word;
  add_word(copy_word, row, block, other_block_list);
  *other_image_changed = TRUE;
  return TRUE;
}


/**********************************************************************
 * word_delete()
 *
 * Delete a word
 **********************************************************************/

BOOL8 word_delete(                     //delete a word
                  BLOCK *block,        //block holding word
                  ROW *row,            //row holding word
                  WERD *word,          //word to be processed
                  BLOCK_IT &block_it,  //block list iterator
                  ROW_IT &row_it,      //row list iterator
                  WERD_IT &word_it     //word list iterator
                 ) {
  word_it.extract ();
  word->bounding_box ().plot (image_win, INT_SOLID, FALSE, BLACK, BLACK);
  delete(word);

  if (word_it.empty ()) {        //no words left in row
                                 //so delete row
    row_it.extract ();
    row->bounding_box ().plot (image_win, INT_SOLID, FALSE, BLACK, BLACK);
    delete(row);

    if (row_it.empty ()) {       //no rows left in blk
                                 //so delete block
      block_it.extract ();
      block->bounding_box ().plot (image_win, INT_SOLID, FALSE,
        BLACK, BLACK);
      delete(block);
    }
  }
  *current_image_changed = TRUE;
  return TRUE;
}


/**********************************************************************
 *  word_display()  Word Processor
 *
 *  Display a word according to its display modes
 **********************************************************************/

BOOL8 word_display(            // display a word
                   BLOCK *,    //block holding word
                   ROW *row,   //row holding word
                   WERD *word  //word to be processed
                  ) {
  BOX word_bb;                   //word bounding box
  int word_height;               //ht of word BB
  BOOL8 displayed_something = FALSE;
  BOOL8 displayed_rainbow = FALSE;
  float shift;                   //from bot left
  PBLOB_IT it;                   //blob iterator
  C_BLOB_IT c_it;                //cblob iterator
  WERD *word_ptr;                //poly copy
  WERD temp_word;
  float scale_factor;            //for BN_POLYGON

  /*
    Note the double coercions of (COLOUR)((INT32)editor_image_word_bb_color)
    etc. are to keep the compiler happy.
  */

                                 //display bounding box
  if (word->display_flag (DF_BOX)) {
    word->bounding_box ().plot (image_win, INT_HOLLOW, TRUE,
      (COLOUR) ((INT32)
      editor_image_word_bb_color),
      (COLOUR) ((INT32)
      editor_image_word_bb_color));

    perimeter_color_index (image_win,
      (COLOUR) ((INT32) editor_image_blob_bb_color));
    if (word->flag (W_POLYGON)) {
      it.set_to_list (word->blob_list ());
      for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
        it.data ()->bounding_box ().plot (image_win);
    }
    else {
      c_it.set_to_list (word->cblob_list ());
      for (c_it.mark_cycle_pt (); !c_it.cycled_list (); c_it.forward ())
        c_it.data ()->bounding_box ().plot (image_win);
    }
    displayed_something = TRUE;
  }

                                 //display edge steps
  if (word->display_flag (DF_EDGE_STEP) &&
  !word->flag (W_POLYGON)) {     //edgesteps available
    word->plot (image_win);      //rainbow colors
    displayed_something = TRUE;
    displayed_rainbow = TRUE;
  }

                                 //display poly approx
  if (word->display_flag (DF_POLYGONAL)) {
                                 //need to convert
    if (!word->flag (W_POLYGON)) {
      word_ptr = word->poly_copy (row->x_height ());

      /* CALL POLYGONAL APPROXIMATOR WHEN AVAILABLE - on a temp_word */

      if (displayed_rainbow)
                                 //ensure its visible
        word_ptr->plot (image_win, WHITE);
      else
                                 //rainbow colors
          word_ptr->plot (image_win);
      delete word_ptr;
    }
    else {
      if (displayed_rainbow)
                                 //ensure its visible
        word->plot (image_win, WHITE);
      else
        word->plot (image_win);  //rainbow colors
    }

    displayed_rainbow = TRUE;
    displayed_something = TRUE;
  }

                                 //disp BN poly approx
  if (word->display_flag (DF_BN_POLYGONAL)) {
                                 //need to convert
    if (!word->flag (W_POLYGON)) {
      word_ptr = word->poly_copy (row->x_height ());
      temp_word = *word_ptr;
      delete word_ptr;

      /* CALL POLYGONAL APPROXIMATOR WHEN AVAILABLE - on a temp_word */

    }
    else
      temp_word = *word;         //copy word
    word_bb = word->bounding_box ();
    if (!temp_word.flag (W_NORMALIZED))
      temp_word.baseline_normalise (row);

    scale_factor = re_scale_and_move_bln_word (&temp_word, word_bb);
    display_bln_lines (image_win, CYAN, scale_factor, word_bb.bottom (),
      word_bb.left (), word_bb.right ());

    if (displayed_rainbow)
                                 //ensure its visible
      temp_word.plot (image_win, WHITE);
    else
      temp_word.plot (image_win);//rainbow colors

    displayed_rainbow = TRUE;
    displayed_something = TRUE;
  }

                                 //display correct       text
  if (word->display_flag (DF_TEXT)) {
    word_bb = word->bounding_box ();
    text_color_index (image_win,
      (COLOUR) ((INT32) editor_image_text_color));
    text_font_index (image_win, 1);
    word_height = word_bb.height ();
    character_height (image_win, 0.75 * word_height);

    if (word_height < word_bb.width ())
      shift = 0.25 * word_height;
    else
      shift = 0.0f;

    text2d (image_win,
      word_bb.left () + shift,
      word_bb.bottom () + 0.25 * word_height,
      word->text (), 0, FALSE);
    if (strlen (word->text ()) > 0)
      displayed_something = TRUE;
  }

  if (!displayed_something)      //display BBox anyway
    word->bounding_box ().plot (image_win, INT_HOLLOW, TRUE,
      (COLOUR) ((INT32) editor_image_word_bb_color),
      (COLOUR) ((INT32)
      editor_image_word_bb_color));
  return TRUE;
}


/**********************************************************************
 * word_dumper()
 *
 * Dump members to the debug window
 **********************************************************************/

BOOL8 word_dumper(               //dump word
                  BLOCK *block,  //block holding word
                  ROW *row,      //row holding word
                  WERD *word     //word to be processed
                 ) {

  tprintf ("\nBlock data...\n");
  block->print (NULL, FALSE);
  tprintf ("\nRow data...\n");
  row->print (NULL);
  tprintf ("\nWord data...\n");
  word->print (NULL);
  return TRUE;
}


/**********************************************************************
 * word_set_display()  Word processor
 *
 * Display word according to current display mode settings
 **********************************************************************/

BOOL8 word_set_display(               //display a word
                       BLOCK *block,  //block holding word
                       ROW *row,      //row holding word
                       WERD *word     //word to be processed
                      ) {
  BOX word_bb;                   //word bounding box

  word->set_display_flag (DF_BOX, word_display_mode.bit (DF_BOX));
  word->set_display_flag (DF_TEXT, word_display_mode.bit (DF_TEXT));
  word->set_display_flag (DF_POLYGONAL, word_display_mode.bit (DF_POLYGONAL));
  word->set_display_flag (DF_EDGE_STEP, word_display_mode.bit (DF_EDGE_STEP));
  word->set_display_flag (DF_BN_POLYGONAL,
    word_display_mode.bit (DF_BN_POLYGONAL));
  *current_image_changed = TRUE;
  return word_display (block, row, word);
}


/**********************************************************************
 * word_toggle_seg()
 *
 * Toggle the correct segmentation flag
 **********************************************************************/

BOOL8 word_toggle_seg(            //toggle seg flag
                      BLOCK *,    //block holding word
                      ROW *,      //row holding word
                      WERD *word  //word to be processed
                     ) {
  word->set_flag (W_SEGMENTED, !word->flag (W_SEGMENTED));
  *current_image_changed = TRUE;
  return TRUE;
}


/* DEBUG ONLY */

void do_check_mem(  //do it
                  INT32 level) {
  check_mem ("Doing it", level);
}
