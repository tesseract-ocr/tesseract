/**********************************************************************
 * File:        varblmen.h  (Formerly varmenu.h)
 * Description: Variables menu subclasses
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

#ifndef VARBLMEN_H
#define VARBLMEN_H

#include          "sbdmenu.h"
#include          "submen.h"
#include          "notdll.h"

/**********************************************************************

Class BOOL_VAR_MENU_LEAF - Boolean Variable leaf

**********************************************************************/

class BOOL_VAR_MENU_LEAF:public LEAF_MENU_NODE
{
  private:
    BOOL_VARIABLE * var;         //The variable
    BOOL8 changed;               //User modified?

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    void plotx(                 //draw it
               WINDOW window);  //in this window

    const char *cmp_str() {  //get ptr to name
      return var->name_str ();
    }

    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?

  public:
    BOOL_VAR_MENU_LEAF (         //constructor
      BOOL_VARIABLE * this_var   //variable to edit
    ): LEAF_MENU_NODE ("") {
      var = this_var;
      changed = FALSE;
    }
};

/**********************************************************************

Class DBL_VAR_MENU_LEAF - Double Variable leaf

**********************************************************************/

class DBL_VAR_MENU_LEAF:public LEAF_MENU_NODE
{
  private:
    double_VARIABLE * var;       //The variable
    BOOL8 changed;               //User modified?

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    const char *cmp_str() {  //get ptr to name
      return var->name_str ();
    }

    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?

  public:
    DBL_VAR_MENU_LEAF (          //constructor
      double_VARIABLE * this_var //variable to edit
    ): LEAF_MENU_NODE ("") {
      var = this_var;
      changed = FALSE;
    }
};

/**********************************************************************

Class INT_VAR_MENU_LEAF - Integer Variable leaf

**********************************************************************/

class INT_VAR_MENU_LEAF:public LEAF_MENU_NODE
{
  private:
    INT_VARIABLE * var;          //The variable
    BOOL8 changed;               //User modified?

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    const char *cmp_str() {  //get ptr to name
      return var->name_str ();
    }

    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?

  public:
    INT_VAR_MENU_LEAF (          //constructor
      INT_VARIABLE * this_var    //variable to edit
    ): LEAF_MENU_NODE ("") {
      var = this_var;
      changed = FALSE;
    }
};

/**********************************************************************

Class STR_VAR_MENU_LEAF - String Variable leaf

**********************************************************************/

class STR_VAR_MENU_LEAF:public LEAF_MENU_NODE
{
  private:
    STRING_VARIABLE * var;       //The variable
    BOOL8 changed;               //User modified?

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    const char *cmp_str() {  //get ptr to name
      return var->name_str ();
    }

    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?

  public:
    STR_VAR_MENU_LEAF (          //constructor
      STRING_VARIABLE * this_var //variable to edit
    ): LEAF_MENU_NODE ("") {
      var = this_var;
      changed = FALSE;
    }
};

/**********************************************************************

Class VAR_NON_RADIO_MENU - A sub menu containing variables

**********************************************************************/

class VAR_NON_RADIO_MENU:public NON_LEAF_MENU_NODE
{
  private:
    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?

  public:
    VAR_NON_RADIO_MENU (         //constructor
    const char *menu_text):NON_LEAF_MENU_NODE (menu_text) {
    }

    void add_child(  //add to sub-menu end //item to add
                   SIMPLE_MENU_LEAF *new_child) {
      link_child(new_child); 
    }

    void add_child(                            //add to sub-menu end
                   VAR_SUB_MENU *new_child) {  //item to add
      link_child(new_child); 
    }

    void add_child(                      //add to sub-menu end
                   MENU_L *new_child) {  //item to add
      link_child_link(new_child); 
    }
};
#endif
