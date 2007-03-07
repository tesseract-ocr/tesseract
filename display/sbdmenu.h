/**********************************************************************
 * File:        sbdmenu.h  (Formerly menu.h)
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

#ifndef SBDMENU_H
#define SBDMENU_H

#include "hosthplb.h"
#include "elst.h"
#include "strngs.h"
#include "grphics.h"
#include "varable.h"
#include "rect.h"
#include "points.h"
#include          "notdll.h"

extern INT_VAR_H (menu_char_width, 8, "Width of characters in menu text");
extern INT_VAR_H (menu_char_height, 14, "Height characters in menu text");

class MENU_L;                    //Forward Declaration
ELISTIZEH(MENU_L); 

class COMMAND_WINDOW;            //Fwd Decl #inc at end

/* Forward declarations showing the inheritance tree of the MENU_NODE
  subclasses */

class MENU_NODE;
class LEAF_MENU_NODE;
class SIMPLE_MENU_LEAF;
class TOGGLE_MENU_LEAF;
class RADIO_MENU_LEAF;
class VARIABLE_MENU_LEAF;
class NON_LEAF_MENU_NODE;
class MENU_ROOT;
class NON_RADIO_MENU;
class RADIO_MENU;

/**********************************************************************

Class MENU_NODE - Generic menu item - an abstract class.

**********************************************************************/

class MENU_NODE
{
  friend class MENU_ROOT;
  friend class NON_LEAF_MENU_NODE;
  friend class LEAF_MENU_NODE;
  friend class RADIO_MENU;
  protected:
    STRING name;                 //Text of option
    BOX box;                     //Display area

    MENU_NODE(  //constructor
              const char *txt) {
      name = STRING (txt);
    }

    virtual void event (         //User clicked...
      COMMAND_WINDOW * cmd_win,  //For UI, update etc
      FCOORD pt,                 //here
      INT32 * cmd_event_id,      //Command selected
      char *new_value) = 0;      //Edited value

                                 //char width reqd
    virtual INT8 max_num_chars () = 0;

    virtual void plotx (         //draw it
      WINDOW window) = 0;        //in this window

                                 //build box
    virtual BOX &recalc_bounding_box(INT16 tl_x,   //top left x
                                     INT16 tl_y);  //top left y

  private:
    BOX &recalc_fixed_width_bb(               //build box
                               INT16 tl_x,    //top left x
                               INT16 tl_y,    //top left y
                               INT16 width);  //required width

  public:
                                 //return ptr to name
    virtual const char *cmp_str() { 
      return name.string ();
    }

    void plot(                 //draw it
              WINDOW window);  //in this window

    void new_label(  //replace name
                   const char *label) {
      name = label;
    }

    virtual void write_vars(          //save config vars
                            FILE *,   //in this file
                            BOOL8) {  //Changed vars only?
                                 //default do nothing
    }
};

/**********************************************************************

Class LEAF_MENU_NODE - Generic menu leaf

**********************************************************************/

class LEAF_MENU_NODE:public MENU_NODE
{
  protected:
    LEAF_MENU_NODE (             //constructor
    const char *menu_text):MENU_NODE (menu_text) {
    }

    INT8 max_num_chars() {  //char width reqd
      return (INT8) name.length ();
    }

    virtual void plotx(                 //draw it
                       WINDOW window);  //in this window
};

/**********************************************************************

Class SIMPLE_MENU_LEAF - Just returns its event code when selected

**********************************************************************/

class SIMPLE_MENU_LEAF:public LEAF_MENU_NODE
{
  INT32 event_id;                //Return event code

  private:
    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

  public:
    SIMPLE_MENU_LEAF (           //constructor
      const char *menu_text,
    INT32 code):LEAF_MENU_NODE (menu_text) {
      event_id = code;
    }
};

/**********************************************************************

Class TOGGLE_MENU_LEAF - Boolean toggle leaf

**********************************************************************/

class TOGGLE_MENU_LEAF:public LEAF_MENU_NODE
{
  friend class RADIO_MENU;

  INT32 event_id;                //Return event code
  BOOL8 state;                   //ON/OFF

  private:
    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

    void plotx(                 //draw it
               WINDOW window);  //in this window

  public:
    TOGGLE_MENU_LEAF (           //constructor
      const char *menu_text,
      INT32 code,
    BOOL8 initial_state):LEAF_MENU_NODE (menu_text) {
      event_id = code;
      state = initial_state;
    }

    void set_toggle(  //Explicit override
                    BOOL8 value) {
      state = value;
    }
};

/**********************************************************************

Class RADIO_MENU_LEAF - Press the radio button and return the event code
on selection

**********************************************************************/

class RADIO_MENU_LEAF:public TOGGLE_MENU_LEAF
{
  public:
    RADIO_MENU_LEAF (            //constructor
      const char *menu_text,
    INT32 code):TOGGLE_MENU_LEAF (menu_text, code, FALSE) {
    }
};

/**********************************************************************

Class VARIABLE_MENU_LEAF - Prompt for new value and return it

**********************************************************************/

class VARIABLE_MENU_LEAF:public LEAF_MENU_NODE
{
  INT32 event_id;                //Return event code
  STRING current_value;          //Variable value

  void event(                          //User clicked...
             COMMAND_WINDOW *cmd_win,  //For UI, update etc
             FCOORD pt,                //here
             INT32 *cmd_event_id,      //Command selected
             char *new_value);         //Edited value

  public:
    VARIABLE_MENU_LEAF (         //constructor
      const char *menu_text,
      INT32 code,
    const char *initial_value):LEAF_MENU_NODE (menu_text) {
      event_id = code;
      current_value = STRING (initial_value);
    }

    void replace_value(  //change current_value
                       const char *new_value) {
      current_value = new_value;
    }
};

/**********************************************************************

Class NON_LEAF_MENU_NODE - Generic menu item - an abstract class.

**********************************************************************/

class NON_LEAF_MENU_NODE:public MENU_NODE
{
  public:
    void clear_children() { 
      menu_list.clear ();
    };

  protected:
    MENU_L_LIST menu_list;

    NON_LEAF_MENU_NODE (         //constructor
    const char *menu_text):MENU_NODE (menu_text) {
    }

    void link_child(                        //add to sub-menu end
                    MENU_NODE *new_child);  //item to add

    void link_child_link(                     //add to sub-menu end
                         MENU_L *new_child);  //item to add

    virtual void event(                          //User clicked...
                       COMMAND_WINDOW *cmd_win,  //For UI, update etc
                       FCOORD pt,                //here
                       INT32 *cmd_event_id,      //Command selected
                       char *new_value);         //Edited value

    INT8 max_num_chars();  //char width reqd

    virtual void plotx(                 //draw it
                       WINDOW window);  //in this window

                                 //build box
    virtual BOX &recalc_bounding_box(INT16 tl_x,   //top left x
                                     INT16 tl_y);  //top left y

};

/**********************************************************************

Class MENU_ROOT - The root of a menu tree

**********************************************************************/

class MENU_ROOT:public NON_LEAF_MENU_NODE
{
  private:
    void plotx(                 //draw it
               WINDOW window);  //in this window

  public:
    MENU_ROOT ():NON_LEAF_MENU_NODE ("") {
    }
    //cnstrctr

    void add_child(  //add to sub-menu end //item to add
                   NON_LEAF_MENU_NODE *new_child) {
      link_child(new_child); 
    }

    /* Public defn of event - at menu_root only*/
    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value) {        //Edited value
      NON_LEAF_MENU_NODE::event(cmd_win, pt, cmd_event_id, new_value); 
    }

    BOX &recalc_bounding_box(              //build box
                             INT16 tl_x,   //top left x
                             INT16 tl_y);  //top left y

    /* Public defn of write_vars - at menu_root only*/
    void write_vars(                      //save config vars
                    FILE *fp,             //in this file
                    BOOL8 changes_only);  //Changed vars only?
};

/**********************************************************************

Class NON_RADIO_MENU - The root of a menu tree

**********************************************************************/

class NON_RADIO_MENU:public NON_LEAF_MENU_NODE
{
  public:
    NON_RADIO_MENU (             //constructor
    const char *menu_text):NON_LEAF_MENU_NODE (menu_text) {
    }

    void add_child(  //add to sub-menu end //item to add
                   SIMPLE_MENU_LEAF *new_child) {
      link_child(new_child); 
    }

    void add_child(  //add to sub-menu end //item to add
                   TOGGLE_MENU_LEAF *new_child) {
      link_child(new_child); 
    }

    void add_child(  //add to sub-menu end //item to add
                   VARIABLE_MENU_LEAF *new_child) {
      link_child(new_child); 
    }

};

/**********************************************************************

Class RADIO_MENU - The root of a radio sub menu

**********************************************************************/

class RADIO_MENU:public NON_LEAF_MENU_NODE
{
  private:
    RADIO_MENU_LEAF * on_button; //which one is on?

    void event(                          //User clicked...
               COMMAND_WINDOW *cmd_win,  //For UI, update etc
               FCOORD pt,                //here
               INT32 *cmd_event_id,      //Command selected
               char *new_value);         //Edited value

  public:
    RADIO_MENU (                 //constructor
    const char *menu_text):NON_LEAF_MENU_NODE (menu_text) {
    }

    void add_child(                              //add to sub-menu end
                   RADIO_MENU_LEAF *new_child);  //item to add

    void press_radio_button(                           //Change selected butn
                            RADIO_MENU_LEAF *button);  //to this one
};

/**********************************************************************

Class MENU_L - Generic menu list element for heterogeneous list.

**********************************************************************/

class MENU_L:public ELIST_LINK
{
  public:
    MENU_NODE * ptr;             //Generic menu item

    MENU_L() { 
    }                            //copy list constrctr

    MENU_L(  //normal constructor
           MENU_NODE *p) {
      ptr = p;
    }
};

#include "cmndwin.h"
#include          "notdll.h"
#endif
