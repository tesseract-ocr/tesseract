/**********************************************************************
 * File:        varblmen.cpp  (Formerly varmenu.c)
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

#include "mfcpch.h"
#include          "varblmen.h"
#include          "varblwin.h"

/**********************************************************************
 ***********************************************************************
 *
 *       BOOL_VAR_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * BOOL_VAR_MENU_LEAF::event( )
 *
 * Event selection - Toggle value
 **********************************************************************/

void BOOL_VAR_MENU_LEAF::event(                          //User clicked...
                               COMMAND_WINDOW *cmd_win,  //For UI, update etc
                               FCOORD pt,                //here
                               INT32 *,                  //Command selected
                               char *                    //Edited value
                              ) {
  char msg[MAX_CHARS + 1];

  if (box.contains (pt)) {
    var->set_value (!(BOOL8) * var);
    sprintf (msg, "TOGGLED: %s", var->info_str ());
    cmd_win->msg (msg);
    changed = TRUE;
  }
}


/**********************************************************************
 * BOOL_VAR_MENU_LEAF::plotx( )
 *
 * The real plot.  Positions assumed already calculated.
 **********************************************************************/

void BOOL_VAR_MENU_LEAF::plotx(               //draw it
                               WINDOW window  //in this window
                              ) {
  if ((BOOL8) * var) {
    text_color_index(window, BLACK); 
    fill_color_index(window, WHITE); 
  }

  LEAF_MENU_NODE::plotx(window); 

  text_color_index(window, WHITE); 
  fill_color_index(window, GREY); 
}


/**********************************************************************
 * BOOL_VAR_MENU_LEAF::write_vars( )
 *
 **********************************************************************/

void BOOL_VAR_MENU_LEAF::write_vars(                    //save config vars
                                    FILE *fp,           //in this file
                                    BOOL8 changes_only  //Changed vars only?
                                   ) {
  if (!changes_only || changed) {
    fprintf (fp, "%-25s   %-12s   # %s\n",
      var->name_str (),
      (BOOL8) * var ? "TRUE" : "FALSE", var->info_str ());
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       DBL_VAR_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * DBL_VAR_MENU_LEAF::event( )
 *
 * Event selection - Toggle value
 **********************************************************************/

void DBL_VAR_MENU_LEAF::event(                          //User clicked...
                              COMMAND_WINDOW *cmd_win,  //For UI, update etc
                              FCOORD pt,                //here
                              INT32 *,                  //Command selected
                              char *                    //Edited value
                             ) {
  char ascii_new_value[MAX_CHARS + 1];
  const char *prompt_msg;
  double new_value;

  if (box.contains (pt)) {
    sprintf (ascii_new_value, "%g", (double) *var);
    prompt_msg = var->info_str ();
    while (cmd_win->internal_prompt (prompt_msg, ascii_new_value)) {
      if (sscanf (ascii_new_value, "%lf", &new_value) == 1) {
        if (new_value != (double) *var) {
          var->set_value (new_value);
          cmd_win->msg ("Value changed");
          changed = TRUE;
          return;
        }
        else {
          cmd_win->msg ("Value unchanged");
          return;
        }
      }
      else
        prompt_msg = "Invalid value for double - try again or click";
    }
  }
}


/**********************************************************************
 * DBL_VAR_MENU_LEAF::write_vars( )
 *
 **********************************************************************/

void DBL_VAR_MENU_LEAF::write_vars(                    //save config vars
                                   FILE *fp,           //in this file
                                   BOOL8 changes_only  //Changed vars only?
                                  ) {
  if (!changes_only || changed) {
    fprintf (fp, "%-25s   %-12e   # %s\n",
      var->name_str (), (double) *var, var->info_str ());
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       INT_VAR_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * INT_VAR_MENU_LEAF::event( )
 *
 * Event selection - Toggle value
 **********************************************************************/

void INT_VAR_MENU_LEAF::event(                          //User clicked...
                              COMMAND_WINDOW *cmd_win,  //For UI, update etc
                              FCOORD pt,                //here
                              INT32 *,                  //Command selected
                              char *                    //Edited value
                             ) {
  char ascii_new_value[MAX_CHARS + 1];
  const char *prompt_msg;
  INT32 new_value;

  if (box.contains (pt)) {
    sprintf (ascii_new_value, INT32FORMAT, (INT32) * var);
    prompt_msg = var->info_str ();
    while (cmd_win->internal_prompt (prompt_msg, ascii_new_value)) {
      if (sscanf (ascii_new_value, INT32FORMAT, &new_value) == 1) {
        if (new_value != (INT32) * var) {
          var->set_value (new_value);
          cmd_win->msg ("Value changed");
          changed = TRUE;
          return;
        }
        else {
          cmd_win->msg ("Value unchanged");
          return;
        }
      }
      else
        prompt_msg = "Invalid value for INT32 - try again or click";
    }
  }
}


/**********************************************************************
 * INT_VAR_MENU_LEAF::write_vars( )
 *
 **********************************************************************/

void INT_VAR_MENU_LEAF::write_vars(                    //save config vars
                                   FILE *fp,           //in this file
                                   BOOL8 changes_only  //Changed vars only?
                                  ) {
  if (!changes_only || changed) {
    fprintf (fp, "%-25s   %-12d   # %s\n",
      var->name_str (), (int) *var, var->info_str ());
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       STR_VAR_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * STR_VAR_MENU_LEAF::event( )
 *
 * Event selection - Toggle value
 **********************************************************************/

void STR_VAR_MENU_LEAF::event(                          //User clicked...
                              COMMAND_WINDOW *cmd_win,  //For UI, update etc
                              FCOORD pt,                //here
                              INT32 *,                  //Command selected
                              char *                    //Edited value
                             ) {
  char ascii_new_value[MAX_CHARS + 1];

  if (box.contains (pt)) {
    sprintf (ascii_new_value, "%s", ((STRING) * var).string ());
    if (cmd_win->internal_prompt (var->info_str (), ascii_new_value)) {
      if (strcmp (ascii_new_value, ((STRING) * var).string ()) != 0) {
        var->set_value (ascii_new_value);
        cmd_win->msg ("Value changed");
        changed = TRUE;
        return;
      }
      else {
        cmd_win->msg ("Value unchanged");
        return;
      }
    }
  }
}


/**********************************************************************
 * STR_VAR_MENU_LEAF::write_vars( )
 *
 **********************************************************************/

void STR_VAR_MENU_LEAF::write_vars(                    //save config vars
                                   FILE *fp,           //in this file
                                   BOOL8 changes_only  //Changed vars only?
                                  ) {
  if (!changes_only || changed) {
    fprintf (fp, "%-25s   %-12s   # %s\n",
      var->name_str (),
      ((STRING) * var).string (), var->info_str ());
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       VAR_NON_RADIO_MENU     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * VAR_NON_RADIO_MENU::write_vars( )
 *
 * Just propogate down the menu tree
 **********************************************************************/

void VAR_NON_RADIO_MENU::write_vars(                    //save config vars
                                    FILE *fp,           //in this file
                                    BOOL8 changes_only  //Changed vars only?
                                   ) {
  MENU_L_IT it(&menu_list); 

  fprintf (fp, "\n");
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->ptr->write_vars (fp, changes_only);
}


/**********************************************************************
 ***********************************************************************
 *
 *       VAR_SUB_MENU     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * VAR_SUB_MENU::event( )
 *
 * Event selection - Toggle value
 **********************************************************************/

void VAR_SUB_MENU::event(                      //User clicked...
                         COMMAND_WINDOW *win,  //For UI, update etc
                         FCOORD pt,            //here
                         INT32 *,              //Command selected
                         char *                //Edited value
                        ) {
  if (box.contains (pt)) {
    if (sub_window != NULL)
      win->msg ("Sub menu is already open!");
    else {
      sub_window = new VARIABLES_WINDOW (name.string (), root, this);
    }
  }
}
