/**********************************************************************
 * File:        submen.h  (Formerly submenu.h)
 * Description: Variables submenu subclasses
 * Author:      Phil Cheatle
 * Created:     Wed Nov 13 11:17:26 GMT 1991
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

#ifndef SUBMEN_H
#define SUBMEN_H

/**********************************************************************

Class VAR_SUB_MENU - A leaf in the top level variable editor window

**********************************************************************/

class VAR_SUB_MENU:public LEAF_MENU_NODE
{
  private:
    MENU_ROOT * root;            //Sub window menu
    COMMAND_WINDOW *sub_window;  //Sub window window

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    void write_vars(                       //save config vars
                    FILE *fp,              //in this file
                    BOOL8 changes_only) {  //Changed vars only?
      fprintf (fp, "\n");
                                 //propogate to submenu
      root->write_vars (fp, changes_only);
    }

  public:
    VAR_SUB_MENU (               //constructor
                                 //variable to edit
      const char *menu_text, MENU_ROOT * sub_menu
    ):LEAF_MENU_NODE (menu_text) {
      root = sub_menu;
      sub_window = NULL;
    }

    void child_closed() { 
      sub_window = NULL;
    }
};
#endif
