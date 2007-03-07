/**********************************************************************
 * File:        cmndwin.h  (Formerly cmdwin.h)
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

#ifndef CMNDWIN_H
#define CMNDWIN_H

#include <stdio.h>
#include "hosthplb.h"
#include "grphics.h"
#include "evnts.h"
#include "sbdmenu.h"
#include          "notdll.h"

#define MAX_CHARS     80
                                 //still decoding event
#define UNIDENTIFIED_COMMAND    -1
#define NULL_COMMAND    0        //no processing reqd

class COMMAND_WINDOW
{
  friend class VARIABLE_MENU_LEAF;
  friend class DBL_VAR_MENU_LEAF;
  friend class INT_VAR_MENU_LEAF;
  friend class STR_VAR_MENU_LEAF;

  INT16 window_width;            //Displable width
                                 //Message display area
  char message_str[MAX_CHARS + 1];
  char prompt_str[MAX_CHARS + 1];//Prompt display area
  INT16 x_pos;                   //Current win pos
  INT16 y_pos;                   //Current win pos
  char win_name[MAX_CHARS + 1];  //Window name
  static INT16 next_win_x_pos;   //Posn of next cmd win
  static INT16 next_win_y_pos;   //Posn of next cmd win

  protected:
    WINDOW fd;                   //Cmd Window handle
    MENU_ROOT *menu_root;        //Root of menu tree

                                 //Prompt user(No plot)
    virtual BOOL8 internal_prompt(const char *prompt_str,  //Prompt message
                                  char *response);         //Response & Default

  public:
    COMMAND_WINDOW(                   //Constructor
                   const char *name,  //window name
                   MENU_ROOT *menu_ptr);

    void event(                   //Process event //Command event type
               GRAPHICS_EVENT &g_event,
               INT32 *c_event,
               char *new_value);  //of menu item

    void msg(                              //Display message
             const char *message_string);  //Text to display

    void plot();  //(re)paint the window

    void plot_msg_area(); 

    void plot_prompt_area(); 

    void press_radio_button(                                  //Change selected item //of this radio set
                            RADIO_MENU *radio_sub_menu_item,  //This button
                            RADIO_MENU_LEAF *button_menu_item);

    void update_menu_tree();  //Re calc BBoxes etc

    BOOL8 prompt(                         //Prompt user(& plot)
                 const char *prompt_str,  //Prompt message
                 char *response);         //Response & Default

    void replace_menu_text(                            //Change & show label
                           LEAF_MENU_NODE *menu_item,  //for this item
                           const char *new_label);     //New label

    void set_toggle(                              //Change value & show
                    TOGGLE_MENU_LEAF *menu_item,  //for this item
                    BOOL8 new_value);

    WINDOW window() {  //Return window handle
      return fd;
    }
};
extern INT_VAR_H (editor_cmdwin_width, 950,
"CmdWin max non scrollable width");
extern INT_VAR_H (editor_cmdwin_height, 550,
"CmdWin max non scrollable height");
extern INT_VAR_H (editor_cmdwin_xpos1, 20, "X pos of first command window");
extern INT_VAR_H (editor_cmdwin_ypos1, 20, "Y pos of first command window");
extern INT_VAR_H (editor_cmdwin_xoffset, 30,
"X offset between command windws");
extern INT_VAR_H (editor_cmdwin_yoffset, 30,
"Y offset between command windws");
#endif
