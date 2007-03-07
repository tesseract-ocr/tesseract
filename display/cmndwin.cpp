/**********************************************************************
 * File:        cmndwin.cpp  (Formerly cmdwin.c)
 * Description: Command Window class
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
#include <ctype.h>
#include "evnts.h"
#include "cmndwin.h"

#define BACKSPACE_KEY       '\010'
#define RETURN_KEY          '\015'

#define BORDER_HEIGHT             (menu_char_height / 2)

#define MSG_AREA_TOP_LEFT_X       0
#define MSG_AREA_TOP_LEFT_Y       0
#define MSG_AREA_HEIGHT           (int)(menu_char_height * 1.5 )
#define MSG_TEXT_START_X          (menu_char_height / 2)
#define MSG_TEXT_START_Y          (MSG_AREA_TOP_LEFT_Y - menu_char_height)

#define PROMPT_AREA_TOP_LEFT_X    0
#define PROMPT_AREA_TOP_LEFT_Y    \
	(MSG_AREA_TOP_LEFT_Y - MSG_AREA_HEIGHT - BORDER_HEIGHT)
#define PROMPT_AREA_HEIGHT        MSG_AREA_HEIGHT
#define PROMPT_TEXT_START_X       (menu_char_height / 2)
#define PROMPT_TEXT_START_Y       (PROMPT_AREA_TOP_LEFT_Y - menu_char_height)

#define MENU_AREA_TOP_LEFT_X      0
#define MENU_AREA_TOP_LEFT_Y      \
	(PROMPT_AREA_TOP_LEFT_Y - PROMPT_AREA_HEIGHT -  BORDER_HEIGHT)

#define MIN_WINDOW_WIDTH          (PROMPT_TEXT_START_X + 80 * menu_char_width)

#define EXTERN

EXTERN INT_VAR (editor_cmdwin_width, 950, "CmdWin max non scrollable width");
EXTERN INT_VAR (editor_cmdwin_height, 550,
"CmdWin max non scrollable height");
EXTERN INT_VAR (editor_cmdwin_xpos1, 20, "X pos of first command window");
EXTERN INT_VAR (editor_cmdwin_ypos1, 20, "Y pos of first command window");
EXTERN INT_VAR (editor_cmdwin_xoffset, 30, "X offset between command windws");
EXTERN INT_VAR (editor_cmdwin_yoffset, 30, "Y offset between command windws");

INT16
COMMAND_WINDOW::next_win_x_pos = 0;
INT16
COMMAND_WINDOW::next_win_y_pos = 0;

/**********************************************************************
 * COMMAND_WINDOW::COMMAND_WINDOW()
 *
 * COMMAND_WINDOW constructor
 **********************************************************************/

                                 // window name
COMMAND_WINDOW::COMMAND_WINDOW(const char *name,
                               MENU_ROOT *menu_ptr  // root of menu
                              ) {
  BOX menu_box;
  INT16 window_height;
  INT8 window_type = FULLSIZEWIN;//Default
  INT16 xsize;
  INT16 ysize;

  message_str[0] = '\0';
  prompt_str[0] = '\0';
  strcpy(win_name, name); 

  menu_root = menu_ptr;
  menu_box = menu_root->recalc_bounding_box (MENU_AREA_TOP_LEFT_X,
    MENU_AREA_TOP_LEFT_Y);

  if (menu_box.width () > MIN_WINDOW_WIDTH) {
    window_width = menu_box.width ();
  }
  else {
    window_width = MIN_WINDOW_WIDTH;
  }

  window_height = MSG_AREA_TOP_LEFT_Y - menu_box.bottom ();

  xsize = window_width;
  ysize = window_height;

  if (window_width > editor_cmdwin_width) {
    window_type = SCROLLINGWIN;
    xsize = editor_cmdwin_width;
  }

  if (window_height > editor_cmdwin_height) {
    window_type = SCROLLINGWIN;
    ysize = editor_cmdwin_height;
  }

  if ((next_win_x_pos == 0) && (next_win_x_pos == 0)) {
                                 //Init for 1st win
    next_win_x_pos = editor_cmdwin_xpos1;
    next_win_y_pos = editor_cmdwin_ypos1;
  }
  x_pos = next_win_x_pos;
  y_pos = next_win_y_pos;

                                 //min x
  fd = create_window (name, window_type, x_pos, y_pos, xsize, ysize, MSG_AREA_TOP_LEFT_X,
    window_width,                //max x
    menu_box.bottom (),          //min y
    MSG_AREA_TOP_LEFT_Y,         //max_y
    TRUE,                        //mouse DOWN
    FALSE, FALSE, TRUE);         //key press

  vdc_extent (fd, 0, 0, xsize, ysize);
  next_win_x_pos += editor_cmdwin_xoffset;
  next_win_y_pos += editor_cmdwin_yoffset;

  plot(); 
}


/**********************************************************************
 * COMMAND_WINDOW::event()
 *
 * COMMAND_WINDOW event handler
 **********************************************************************/

void COMMAND_WINDOW::event(                 //Process event //Command event type
                           GRAPHICS_EVENT &g_event,
                           INT32 *cmd_event,
                           char *new_value  //of menu item
                          ) {
  message_str[0] = '\0';
  prompt_str[0] = '\0';
  *cmd_event = UNIDENTIFIED_COMMAND;
  new_value[0] = '\0';

  switch (g_event.type) {
    case DOWN_EVENT:
    {
      menu_root->event (this, FCOORD (g_event.x, g_event.y),
        cmd_event, new_value);
      if (*cmd_event == UNIDENTIFIED_COMMAND)
        *cmd_event = NULL_COMMAND;
      break;
    }
    case KEYPRESS_EVENT:
    {
      if (g_event.key == 3) {    //Control C
        exit (0);
      }
    }
    default:
    {
      //                      sprintf( message_str, "ERROR: Unrecognised graphics event %d",
      //                                      g_event.type );
      *cmd_event = NULL_COMMAND;
      break;
    }
  }
  plot(); 
}


/**********************************************************************
 * COMMAND_WINDOW::msg()
 *
 * COMMAND_WINDOW message display
 **********************************************************************/

void COMMAND_WINDOW::msg(                     //Display message
                         const char *msg_str  //Text to display
                        ) {
  strcpy(message_str, msg_str); 
  plot_msg_area(); 
  overlap_picture_ops(TRUE); 
}


/**********************************************************************
 * COMMAND_WINDOW::plot()
 *
 * COMMAND_WINDOW (re) paint the window
 **********************************************************************/

void COMMAND_WINDOW::plot() { 
  clear_view_surface(fd); 
  text_font_index (fd, 1);
  character_height(fd, menu_char_height); 

  plot_msg_area(); 
  plot_prompt_area(); 

  menu_root->plot (fd);
}


/**********************************************************************
 * COMMAND_WINDOW::plot_msg_area()
 *
 **********************************************************************/

void COMMAND_WINDOW::plot_msg_area() { 
  fill_color_index(fd, DARK_SLATE_BLUE); 
  interior_style(fd, INT_SOLID, FALSE); 

  rectangle (fd,
    MSG_AREA_TOP_LEFT_X, MSG_AREA_TOP_LEFT_Y,
    MSG_AREA_TOP_LEFT_X + window_width - 1,
    MSG_AREA_TOP_LEFT_Y - MSG_AREA_HEIGHT);

  text_color_index(fd, WHITE); 
  text2d (fd, MSG_TEXT_START_X, MSG_TEXT_START_Y, message_str, 0, FALSE);
}


/**********************************************************************
 * COMMAND_WINDOW::plot_prompt_area()
 *
 **********************************************************************/

void COMMAND_WINDOW::plot_prompt_area() { 
  INT8 i;
  INT8 prompt_len;
  char char_str[2];

  fill_color_index(fd, DARK_SLATE_BLUE); 
  interior_style(fd, INT_SOLID, FALSE); 

  rectangle (fd,
    PROMPT_AREA_TOP_LEFT_X, PROMPT_AREA_TOP_LEFT_Y,
    PROMPT_AREA_TOP_LEFT_X + window_width - 1,
    PROMPT_AREA_TOP_LEFT_Y - MSG_AREA_HEIGHT);

  text_color_index(fd, WHITE); 
  prompt_len = strlen (prompt_str);
  char_str[1] = '\0';

  for (i = 0; i < prompt_len; i++) {
    char_str[0] = prompt_str[i];
    text2d (fd,
      PROMPT_TEXT_START_X + (i * menu_char_width),
      PROMPT_TEXT_START_Y, char_str, 0, FALSE);
  }
}


/**********************************************************************
 * COMMAND_WINDOW::internal_prompt()
 *
 * COMMAND_WINDOW Prompt for and read a response
 * WITHOUT DOING a PLOT!
 **********************************************************************/

BOOL8 COMMAND_WINDOW::internal_prompt(                      //Prompt user
                                      const char *msg_str,  //Prompt message
                                      char *response_str    //Response & Default
                                     ) {
  GRAPHICS_EVENT event;
  INT8 pos;
  BOOL8 ok = TRUE;

  strcpy(message_str, msg_str); 
  strcpy(prompt_str, response_str); 
  plot_msg_area(); 
  plot_prompt_area(); 
  overlap_picture_ops(TRUE); 

  /* MODE THE UI SO THAT IT ONLY RESPONDS TO KEYPRESSes IN THE COMMAND WINDOW */

  event.key = '\0';

  while (event.key != RETURN_KEY) {
    await_event(fd,         //just cmd window
                TRUE,       //wait for event
                ANY_EVENT,  //ONLY keypresses
                &event);

    if (event.type != KEYPRESS_EVENT) {
      ok = FALSE;
      response_str[0] = '\0';
      break;
    }

    pos = strlen (response_str);
    if (isprint (event.key) && pos < MAX_CHARS) {
      response_str[pos] = event.key;
      response_str[pos + 1] = '\0';
      text2d (fd,
        PROMPT_TEXT_START_X + (pos * menu_char_width),
        PROMPT_TEXT_START_Y, response_str + pos, 0, FALSE);
    }
    else {
      switch (event.key) {
        case BACKSPACE_KEY:
          if (pos > 0) {
            response_str[pos - 1] = '\0';
            fill_color_index(fd, DARK_SLATE_BLUE); 
            interior_style(fd, INT_SOLID, FALSE); 

            rectangle (fd,
              PROMPT_TEXT_START_X + (pos -
              1) * menu_char_width,
              PROMPT_AREA_TOP_LEFT_Y,
              PROMPT_AREA_TOP_LEFT_X + window_width - 1,
              PROMPT_AREA_TOP_LEFT_Y - PROMPT_AREA_HEIGHT);
          }
          break;
        case RETURN_KEY:
          break;
        default:
          sprintf (message_str, "NON PRINTING CHAR: %o", event.key);
          plot_msg_area(); 
      }
    }
    overlap_picture_ops(TRUE); 
  }
  message_str[0] = '\0';
  prompt_str[0] = '\0';
  #ifdef __UNIX__
  //   clear_event_queue(0);                                                             //clear ALL win events
  #endif
  return ok;
}


/**********************************************************************
 * COMMAND_WINDOW::press_radio_button()
 *
 * COMMAND_WINDOW Change the selected button in a radio set
 **********************************************************************/

void COMMAND_WINDOW::press_radio_button(                                  //of this radio set
                                        RADIO_MENU *radio_sub_menu_item,  //This button
                                        RADIO_MENU_LEAF *button_menu_item) {
  radio_sub_menu_item->press_radio_button (button_menu_item);
  plot(); 
}


/**********************************************************************
 * COMMAND_WINDOW::update_menu_tree()
 *
 * Following a CHANGE to the menu tree, the
 * boundng boxes must be recalculated, this may require the window to be
 * re-created, the window must then be re-displayed.
 **********************************************************************/

void COMMAND_WINDOW::update_menu_tree() { 
  BOX menu_box;
  INT16 window_height;
  INT8 window_type = FULLSIZEWIN;//Default
  INT16 xsize;
  INT16 ysize;

  menu_box = menu_root->recalc_bounding_box (MENU_AREA_TOP_LEFT_X,
    MENU_AREA_TOP_LEFT_Y);
  if (menu_box.width () > MIN_WINDOW_WIDTH) {
    window_width = menu_box.width ();
  }
  else {
    window_width = MIN_WINDOW_WIDTH;
  }

  window_height = MSG_AREA_TOP_LEFT_Y - menu_box.bottom ();

  xsize = window_width;
  ysize = window_height;

  if (window_width > editor_cmdwin_width) {
    window_type = SCROLLINGWIN;
    xsize = editor_cmdwin_width;
  }

  if (window_height > editor_cmdwin_height) {
    window_type = SCROLLINGWIN;
    ysize = editor_cmdwin_height;
  }
  destroy_window(fd); 
                                 //min x
  fd = create_window (win_name, window_type, x_pos, y_pos, xsize, ysize, MSG_AREA_TOP_LEFT_X,
    window_width,                //max x
    menu_box.bottom (),          //min y
    MSG_AREA_TOP_LEFT_Y,         //max_y
    TRUE,                        //mouse DOWN
    FALSE, FALSE, TRUE);         //key press

  vdc_extent (fd, 0, 0, xsize, ysize);
  plot(); 
}


/**********************************************************************
 * COMMAND_WINDOW::prompt()
 *
 * COMMAND_WINDOW Prompt for and read a response
 **********************************************************************/

BOOL8 COMMAND_WINDOW::prompt(                      //Prompt user
                             const char *msg_str,  //Prompt message
                             char *response_str    //Response & Default
                            ) {
  BOOL8 ok;

  ok = internal_prompt (msg_str, response_str);
  plot(); 
  return ok;
}


/**********************************************************************
 * COMMAND_WINDOW::replace_menu_text()
 *
 * COMMAND_WINDOW Change the label of a specified menu item
 **********************************************************************/

void COMMAND_WINDOW::replace_menu_text(                       //for this item
                                       LEAF_MENU_NODE *menu_item,
                                       const char *new_label  //New label
                                      ) {
  menu_item->new_label (new_label);
  menu_item->plot (fd);
}


/**********************************************************************
 * COMMAND_WINDOW::set_toggle()
 *
 * COMMAND_WINDOW Change the value of a specified menu item
 **********************************************************************/

void COMMAND_WINDOW::set_toggle(  //for this item
                                TOGGLE_MENU_LEAF *menu_item,
                                BOOL8 new_value) {
  menu_item->set_toggle (new_value);
  menu_item->plot (fd);
}
