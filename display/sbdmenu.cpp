/**********************************************************************
 * File:        sbdmenu.cpp  (Formerly menu.c)
 * Description: Command Window MENU class
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
#include "sbdmenu.h"

#define MENU_COLUMN_SPACING   (menu_char_height / 2)
#define MENU_ITEM_HEIGHT      (int)(menu_char_height * 1.5)
#define MENU_SEPARATOR_HEIGHT MENU_COLUMN_SPACING
#define MENU_TEXT_X_OFFSET      (menu_char_height / 2)
#define MENU_TEXT_Y_OFFSET  menu_char_height/2

#include          "notdll.h"
ELISTIZE(MENU_L); 

const ERRCODE NOT_SUBMENU_NODE = "Button is not leaf of radio submenu";
const ERRCODE SHOULDNT_BE_CALLED = "Should never get to here!!";

INT_VAR (menu_char_width, 8, "Width of characters in menu text");
INT_VAR (menu_char_height, 14, "Height characters in menu text");

/**********************************************************************
 ***********************************************************************
 *
 *       LEAF_MENU_NODE     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * LEAF_MENU_NODE::plotx( )
 *
 * The real plot.  Positions assumed already calculated.
 **********************************************************************/

void LEAF_MENU_NODE::plotx(               //draw it
                           WINDOW window  //in this window
                          ) {
  box.plot (window);

  text2d (window,
    box.left () + MENU_TEXT_X_OFFSET,
    box.top () - MENU_TEXT_Y_OFFSET, name.string (), 0, FALSE);
}


/**********************************************************************
 ***********************************************************************
 *
 *       MENU_NODE     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * MENU_NODE::plot( )
 *
 * Initialise, then call plotx to do the real work
 * (This means that we can initialise plotting once only REGARDLESS of what menu
 * subclass we start plotting.  (EG - we don't always start with a MENU_ROOT.)
 **********************************************************************/

void MENU_NODE::plot(               //draw it
                     WINDOW window  //window to draw in
                    ) {
  character_height(window, menu_char_height); 
  interior_style(window, INT_SOLID, TRUE); 
  perimeter_color_index(window, LIGHT_GREY); 
  text_color_index(window, WHITE); 
  fill_color_index(window, GREY); 
  plotx(window); 
}


/**********************************************************************
 * MENU_NODE::recalc_bounding_box( )
 *
 * SHOULD NEVER BE CALLED.  Compiler needs it as this can't be a pure virtual
 * function as not all classes provide it and the compiler will not let you do
 * "new" on a class which inherits a pure virtual function which it cannot
 * resolve into a real function.
 **********************************************************************/

BOX &MENU_NODE::recalc_bounding_box(        // calculate size
                                    INT16,  // start topleft x
                                    INT16   // start topleft y
                                   ) {
  SHOULDNT_BE_CALLED.error ("MENU_ROOT::recalc_bounding_box", ABORT, NULL);
  return box;
}


/**********************************************************************
 * MENU_NODE::recalc_fixed_width_bb( )
 *
 * Calculate the selectable area of the menu item and hence its display
 * position.  Note that the width is predetermined.
 **********************************************************************/

BOX &MENU_NODE::recalc_fixed_width_bb(             // calc BB
                                      INT16 tl_x,  // start topleft x
                                      INT16 tl_y,  // start topleft y
                                      INT16 width  // width
                                     ) {
  box = BOX (ICOORD (tl_x, tl_y - MENU_ITEM_HEIGHT),
    ICOORD (tl_x + width, tl_y));
  return box;
}


/**********************************************************************
 ***********************************************************************
 *
 *       MENU_ROOT     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * MENU_ROOT::plotx( )
 *
 * The real plot - only called internally after initialisation.
 * Display the menu recursively.  Positions assumed already calculated.
 **********************************************************************/

void MENU_ROOT::plotx(               //draw it
                      WINDOW window  //in this window
                     ) {
  MENU_L_IT it(&menu_list); 

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->ptr->plotx (window);
}


/**********************************************************************
 * MENU_ROOT::recalc_bounding_box( )
 *
 * Calculate the selectable areas of the menu items and hence their display
 * positions.
 **********************************************************************/

BOX &MENU_ROOT::recalc_bounding_box(             // calculate size
                                    INT16 tl_x,  // start topleft x
                                    INT16 tl_y   // start topleft y
                                   ) {
  MENU_L_IT it(&menu_list); 

  box = BOX (ICOORD (tl_x, tl_y), ICOORD (tl_x, tl_y));

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    box += it.data ()->ptr->recalc_bounding_box (box.right (), box.top ());
    box.move_right_edge (MENU_COLUMN_SPACING);
  }
  box.move_right_edge (-MENU_COLUMN_SPACING);
  return box;
}


/**********************************************************************
 * MENU_ROOT::write_vars( )
 *
 * Just propogate down the menu tree
 **********************************************************************/

void MENU_ROOT::write_vars(                    //save config vars
                           FILE *fp,           //in this file
                           BOOL8 changes_only  //Changed vars only?
                          ) {
  MENU_L_IT it(&menu_list); 

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->ptr->write_vars (fp, changes_only);
}


/**********************************************************************
 ***********************************************************************
 *
 *       NON_LEAF_MENU_NODE     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * NON_LEAF_MENU_NODE::link_child( MENU_NODE* )
 *
 * Add a child to the end of the menu list
 **********************************************************************/

void NON_LEAF_MENU_NODE::link_child(  //add to menu end
                                    MENU_NODE *new_child) {
  MENU_L_IT it(&menu_list); 

  it.add_to_end (new MENU_L (new_child));
}


/**********************************************************************
 * NON_LEAF_MENU_NODE::link_child_link( )
 *
 * Add a child to the end of the menu list
 **********************************************************************/

void NON_LEAF_MENU_NODE::link_child_link(  //add to menu
                                         MENU_L *new_child) {
  MENU_L_IT it(&menu_list); 

  it.add_to_end (new_child);
}


/**********************************************************************
 * NON_LEAF_MENU_NODE::event( )
 *
 * Event selection
 **********************************************************************/

void NON_LEAF_MENU_NODE::event(                          //User clicked...
                               COMMAND_WINDOW *cmd_win,  //For UI, update etc
                               FCOORD pt,                //here
                               INT32 *cmd_event_id,      //Command selected
                               char *new_value           //Edited value
                              ) {
  MENU_L_IT it(&menu_list); 

  if (box.contains (pt))
    for (it.mark_cycle_pt ();
    (*cmd_event_id == UNIDENTIFIED_COMMAND) && !it.cycled_list ();
    it.forward ())
  it.data ()->ptr->event (cmd_win, pt, cmd_event_id, new_value);
}


/**********************************************************************
 * NON_LEAF_MENU_NODE::max_num_chars( )
 *
 * Calculate the max character width of any item of this and its children
 **********************************************************************/

INT8 NON_LEAF_MENU_NODE::max_num_chars() {  //calc char width
  MENU_L_IT it(&menu_list); 
  INT8 max_so_far = 0;
  INT8 current_char_count = 0;

  max_so_far = name.length ();
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    current_char_count = it.data ()->ptr->max_num_chars ();
    if (current_char_count > max_so_far)
      max_so_far = current_char_count;
  }
  return max_so_far;
}


/**********************************************************************
 * NON_LEAF_MENU_NODE::plotx( )  (Other than ROOT)
 *
 * The real plot - only called internally after initialisation.
 * Display the menu recursively.  Positions assumed already calculated.
 **********************************************************************/

void NON_LEAF_MENU_NODE::plotx(               //draw it
                               WINDOW window  //in this window
                              ) {
  MENU_L_IT it(&menu_list); 

  box.plot (window);
  text2d (window,
    box.left () + MENU_TEXT_X_OFFSET,
    box.top () - MENU_TEXT_Y_OFFSET, name.string (), 0, FALSE);

  move2d (window,
    box.left (),
    box.top () - MENU_ITEM_HEIGHT - MENU_SEPARATOR_HEIGHT / 2);
  draw2d (window,
    box.right (),
    box.top () - MENU_ITEM_HEIGHT - MENU_SEPARATOR_HEIGHT / 2);

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->ptr->plotx (window);
}


/**********************************************************************
 * NON_LEAF_MENU_NODE::recalc_bounding_box( )
 *
 * Calculate the selectable areas of the menu items and hence their display
 * positions.
 **********************************************************************/

BOX &NON_LEAF_MENU_NODE::recalc_bounding_box(             // calc BB
                                             INT16 tl_x,  // start topleft x
                                             INT16 tl_y   // start topleft y
                                            ) {
  MENU_L_IT it(&menu_list); 
  INT16
    menu_width = max_num_chars () * menu_char_width + 2 * MENU_TEXT_X_OFFSET;

  box = BOX (ICOORD (tl_x, tl_y - MENU_ITEM_HEIGHT - MENU_SEPARATOR_HEIGHT),
    ICOORD (tl_x + menu_width, tl_y));

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    box += it.data ()->ptr->recalc_fixed_width_bb (box.left (),
      box.bottom (),
      box.width ());
  }
  return box;
}


/**********************************************************************
 ***********************************************************************
 *
 *       RADIO_MENU     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * RADIO_MENU::add_child( )
 *
 * Add a radio button
 **********************************************************************/

void RADIO_MENU::add_child(                               //Add button
                           RADIO_MENU_LEAF *new_child) {  //item to add
  if (menu_list.empty ()) {      //Init 1st choice ON
    on_button = new_child;
    new_child->state = TRUE;
  }
  link_child(new_child); 
}


/**********************************************************************
 * RADIO_MENU::event( )
 *
 * Event selection
 **********************************************************************/

void RADIO_MENU::event(                          //User clicked...
                       COMMAND_WINDOW *cmd_win,  //For UI, update etc
                       FCOORD pt,                //here
                       INT32 *cmd_event_id,      //Command selected
                       char *new_value           //Edited value
                      ) {
  MENU_L_IT it(&menu_list); 

  if (box.contains (pt)) {
    if (!on_button->box.contains (pt)) {
      for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
        it.data ()->ptr->event (cmd_win, pt, cmd_event_id, new_value);
        if (*cmd_event_id != UNIDENTIFIED_COMMAND) {
          on_button->state = FALSE;
          on_button = (RADIO_MENU_LEAF *) it.data ()->ptr;
          break;
        }
      }
    }
  }
}


/**********************************************************************
 * RADIO_MENU::press_radio_button()
 *
 * Change the selected button in a radio menu
 **********************************************************************/

void RADIO_MENU::press_radio_button(                         //Chnge selctd butn
                                    RADIO_MENU_LEAF *button  //to this one
                                   ) {
  MENU_L_IT it(&menu_list); 

  if (button != on_button) {
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      if (it.data ()->ptr == button)
        break;
    }

    if (it.cycled_list ())       //couldnt find it
      NOT_SUBMENU_NODE.error ("MENU::press_radio_button", ABORT, NULL);

    on_button->state = FALSE;
    button->state = TRUE;
    on_button = button;
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       SIMPLE_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * SIMPLE_MENU_LEAF::event( )
 *
 * Event selection
 **********************************************************************/

void SIMPLE_MENU_LEAF::event(                      //User clicked...
                             COMMAND_WINDOW *,     //For UI, update etc
                             FCOORD pt,            //here
                             INT32 *cmd_event_id,  //Command selected
                             char *new_value       //Edited value
                            ) {
  if (box.contains (pt)) {
    *cmd_event_id = event_id;
    new_value[0] = '\0';
  }
}


/**********************************************************************
 ***********************************************************************
 *
 *       TOGGLE_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * TOGGLE_MENU_LEAF::event( )
 *
 * Event selection
 **********************************************************************/

void TOGGLE_MENU_LEAF::event(                      //User clicked...
                             COMMAND_WINDOW *,     //For UI, update etc
                             FCOORD pt,            //here
                             INT32 *cmd_event_id,  //Command selected
                             char *new_value       //Edited value
                            ) {
  if (box.contains (pt)) {
    state = !state;
    *cmd_event_id = event_id;
    strcpy (new_value, state ? "T" : "F");
  }
}


/**********************************************************************
 * TOGGLE_MENU_LEAF::plotx( )
 *
 * The real plot.  Positions assumed already calculated.
 **********************************************************************/

void TOGGLE_MENU_LEAF::plotx(               //draw it
                             WINDOW window  //in this window
                            ) {
  if (state) {
    text_color_index(window, BLACK); 
    fill_color_index(window, WHITE); 
  }
  else {
    text_color_index(window, WHITE); 
    fill_color_index(window, GREY); 
  }

  LEAF_MENU_NODE::plotx(window); 

  text_color_index(window, WHITE); 
  fill_color_index(window, GREY); 
}


/**********************************************************************
 ***********************************************************************
 *
 *       VARIABLE_MENU_LEAF     MEMBER FUNCTIONS
 *
 ***********************************************************************
 **********************************************************************/

/**********************************************************************
 * VARIABLE_MENU_LEAF::event( )
 *
 * Event selection
 **********************************************************************/

void VARIABLE_MENU_LEAF::event(                          //User clicked...
                               COMMAND_WINDOW *cmd_win,  //For UI, update etc
                               FCOORD pt,                //here
                               INT32 *cmd_event_id,      //Command selected
                               char *new_value           //Edited value
                              ) {
  char prompt_msg[MAX_CHARS + 1];

  if (box.contains (pt)) {
    strcpy (new_value, current_value.string ());
    sprintf (prompt_msg, "New value for " "%s" "?", name.string ());
    if (cmd_win->internal_prompt (prompt_msg, new_value)) {
      current_value = new_value;
      *cmd_event_id = event_id;
    }
    else
      *cmd_event_id = NULL_COMMAND;
  }
}
