/**********************************************************************
 * File:        varblwin.h  (Formerly varwin.h)
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

#ifndef VARBLWIN_H
#define VARBLWIN_H

#include          "sbdmenu.h"
#include          "notdll.h"
#include          "notdll.h"

#define KILL_WINDOW_CMD   1
#define WRITE_ALL_CMD   2
#define WRITE_CHANGED_CMD 3

class VARIABLES_WINDOW;          //Fwd Decl

/**********************************************************************
 *
 * ASSOC class
 *
 * Maintain an association between a window handle and the VARIABLES_WINDOW
 * which manages events in that window.
 **********************************************************************/

class ASSOC:public ELIST_LINK
{
  friend class ASSOCIATION_LIST;

  WINDOW fd;                     //Window handle
  VARIABLES_WINDOW *var_win;     //Associated var windw

  public:
    ASSOC() { 
    }                            //contstructor

    ASSOC(                //contstructor
          WINDOW new_fd,  //Window handle //Associated var windw
          VARIABLES_WINDOW *new_var_win) {
      fd = new_fd;
      var_win = new_var_win;
    }
};

ELISTIZEH (ASSOC)
/**********************************************************************
 *
 * ASSOCIATION_LIST class
 *
 * Maintain a list of associations between window handles and the
 * VARIABLES_WINDOWs which manages events in those windows.
 **********************************************************************/
class ASSOCIATION_LIST
{
  ASSOC_LIST associations;       //List of pairs

  public:
    ASSOCIATION_LIST() { 
    };

    void add(WINDOW new_fd,  //Window handle //Associated var windw
             VARIABLES_WINDOW *new_var_win);

                                 //Window handle
    VARIABLES_WINDOW *lookup(WINDOW fd); 

    void plot_all();  //Redisplay all wins

    void remove(WINDOW fd);  //Window handle

    void turn_off_interrupts();  //for all windws listd

    void turn_on_interrupts(  //for all windws listd //handler
                            EVENT_HANDLER interrupt_proc);
};

/**********************************************************************
 *
 * VARIABLES_WINDOW class
 *
 * A subclass of the basic COMMAND_WINDOW which is used for variables editor
 * windows.  The chief difference is that this window type awaits interrupts.
 **********************************************************************/

class VARIABLES_WINDOW:public COMMAND_WINDOW
{
  private:
                                 //fd -> var win assocs
    static ASSOCIATION_LIST win_assocs;
    VAR_SUB_MENU *my_creator;    //tell it when I die

    void write_vars(                      //Build config file
                    char *filename,       // in this file
                    BOOL8 changes_only);  // Changed vars only?

                                 //for all windows
    static void interrupt_handler(GRAPHICS_EVENT *g_event); 

    void v_event(  //for specified window
                 GRAPHICS_EVENT &g_event);

  public:
    VARIABLES_WINDOW(                   //constructor
                     const char *name,  //window name
                     MENU_ROOT *menu_root,
                     VAR_SUB_MENU *creator);

    BOOL8 internal_prompt(                         //Prompt user(No plot)
                          const char *prompt_str,  //Prompt message
                          char *response);         //Response & Default

    static void plot_all() {  //Redisplay all vars
      win_assocs.plot_all ();
    }
};
#endif
