/**********************************************************************
 * File:        varblwin.cpp  (Formerly varwin.c)
 * Description: Variables window subclass of COMMAND_WINDOW
 * Author:      Phil Cheatle
 * Created:     Thu Nov 14 15:40:26 GMT 1991
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
#include          "sbdmenu.h"
#include          "submen.h"
#include          "varblwin.h"
#include          "notdll.h"

const ERRCODE NOT_ASSOCIATED = "Cant find associated VARIABLES_WINDOW";

ASSOCIATION_LIST
VARIABLES_WINDOW::win_assocs;
                                 // for initialisation

ELISTIZE (ASSOC)
/**********************************************************************
 *
 * ASSOCIATION_LIST class member functions
 *
 **********************************************************************/
/**********************************************************************
 * ASSOCIATION_LIST::add()
 *
 * Add a new association
 **********************************************************************/
void ASSOCIATION_LIST::add(                               //Window handle
                           WINDOW new_fd,
                           VARIABLES_WINDOW *new_var_win  //Associated var windw
                          ) {
  ASSOC_IT it(&associations); 

  it.add_to_end (new ASSOC (new_fd, new_var_win));
}


/**********************************************************************
 * ASSOCIATION_LIST::remove()
 *
 * Delete an association
 **********************************************************************/

void ASSOCIATION_LIST::remove(           // delete assoc
                              WINDOW fd  //Window handle
                             ) {
  ASSOC_IT it(&associations); 

  if (!it.empty ()) {
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      if (fd == it.data ()->fd) {
        it.extract ();
        return;
      }
    }
  }
  NOT_ASSOCIATED.error ("ASSOCIATION_LIST::remove", ABORT, NULL);
}


/**********************************************************************
 * ASSOCIATION_LIST::lookup()
 *
 * Find an association
 **********************************************************************/

VARIABLES_WINDOW *ASSOCIATION_LIST::lookup(           //find assoc
                                           WINDOW fd  //Window handle
                                          ) {
  ASSOC_IT it(&associations); 

  if (!it.empty ()) {
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      if (fd == it.data ()->fd)
        return it.data ()->var_win;
    }
  }
  NOT_ASSOCIATED.error ("ASSOCIATION_LIST::lookup", ABORT, NULL);
  return NULL;
}


/**********************************************************************
 * ASSOCIATION_LIST::plot_all()
 *
 * Re plot all windows ( to reflect changes to bool vars )
 **********************************************************************/

void ASSOCIATION_LIST::plot_all() {  //plot all wins
  ASSOC_IT it(&associations); 

  if (!it.empty ()) {
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->var_win->plot ();
    }
  }
}


/**********************************************************************
 * ASSOCIATION_LIST::turn_off_interrupts()
 *
 * Disable interrupts for all windows of this class
 **********************************************************************/

void ASSOCIATION_LIST::turn_off_interrupts() { 
  ASSOC_IT it(&associations); 

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
                                 //disable interrupts
    set_click_handler (it.data ()->fd, NULL);
}


/**********************************************************************
 * ASSOCIATION_LIST::turn_on_interrupts()
 *
 * Re-enable interrupts for all windows of this class
 **********************************************************************/

void ASSOCIATION_LIST::turn_on_interrupts(  //handler
                                          EVENT_HANDLER interrupt_proc) {
  ASSOC_IT it(&associations); 

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    set_click_handler (it.data ()->fd, interrupt_proc);
}


/**********************************************************************
 *
 * VARIABLES_WINDOW class member functions
 *
 **********************************************************************/

/**********************************************************************
 * VARIABLES_WINDOW::event()
 *
 * Do non standard event handling for specified commands
 **********************************************************************/

void VARIABLES_WINDOW::v_event(  //Process event
                               GRAPHICS_EVENT &g_event) {
  INT32 cmd_event;               //Command event type
  char new_value[80];            //of menu item
  char buff[80];

  COMMAND_WINDOW::event(g_event, &cmd_event, new_value); 

  switch (cmd_event) {
    case NULL_COMMAND:
      break;
    case WRITE_ALL_CMD:
      write_vars(new_value, FALSE); 
      break;
    case WRITE_CHANGED_CMD:
      write_vars(new_value, TRUE); 
      break;
    case KILL_WINDOW_CMD:
      win_assocs.remove (g_event.fd);
      destroy_window (g_event.fd);
      my_creator->child_closed ();
      delete this;
      break;
    default:
      sprintf (buff, "UNPROCESSED EVENT code " INT32FORMAT, cmd_event);
      msg(buff); 
  }
}


/**********************************************************************
 * VARIABLES_WINDOW::internal_prompt()
 *
 * Disable interrupts during prompting
 *
 **********************************************************************/

BOOL8 VARIABLES_WINDOW::internal_prompt(                      //Prompt user
                                        const char *msg_str,  //Prompt message
                                        char *response_str    //Response & Default
                                       ) {
  BOOL8 result;

  win_assocs.turn_off_interrupts ();
  result = COMMAND_WINDOW::internal_prompt (msg_str, response_str);
  win_assocs.turn_on_interrupts (VARIABLES_WINDOW::interrupt_handler);
  set_click_handler(fd, VARIABLES_WINDOW::interrupt_handler); 
  return result;
}


/**********************************************************************
 * VARIABLES_WINDOW::interrupt_handler()
 *
 * Forward the event to the VARIABLES_WINDOW responsible for handling events
 * for the window where the event occured.
 **********************************************************************/

void VARIABLES_WINDOW::interrupt_handler(                         //for any windw
                                         GRAPHICS_EVENT *g_event  //event recieved
                                        ) {
  (win_assocs.lookup (g_event->fd))->v_event (*g_event);
  //forward to
  overlap_picture_ops(TRUE); 
}


/**********************************************************************
 * VARIABLES_WINDOW::VARIABLES_WINDOW()
 *
 * Constructor for a variables window
 **********************************************************************/

VARIABLES_WINDOW::VARIABLES_WINDOW (
                                 //constructor
const char *name,                //window name
MENU_ROOT * menu_root,           //menu
VAR_SUB_MENU * creator           //who created me?
):
COMMAND_WINDOW(name, menu_root) { 
  win_assocs.add (fd, this);
  my_creator = creator;
  set_click_handler(fd, VARIABLES_WINDOW::interrupt_handler); 
}


/**********************************************************************
 * VARIABLES_WINDOW::write_vars()
 *
 * Write the variables to a config file
 **********************************************************************/

void VARIABLES_WINDOW::write_vars(                    //Build config file
                                  char *filename,     // in this file
                                  BOOL8 changes_only  // Changed vars only?
                                 ) {
  FILE *fp;                      //input file
  char msg_str[MAX_CHARS + 1];
  char response_str[MAX_CHARS + 1];
  char *token;                   //first response token

                                 //if file exists
  if ((fp = fopen (filename, "r")) != NULL) {
    fclose(fp); 
    sprintf (msg_str, "Overwrite file " "%s" "? (Y/N)", filename);
    response_str[0] = '\0';
    if (!prompt (msg_str, response_str))
      return;
    token = strtok (response_str, " ");
    if (tolower (token[0]) != 'y')
      return;                    // dont write
  }

  fp = fopen (filename, "w");    //can we write to it?
  if (fp == NULL) {
    sprintf (msg_str, "Cant write to file " "%s" "", filename);
    msg(msg_str); 
    return;
  }
  menu_root->write_vars (fp, changes_only);
  fclose(fp); 
}
