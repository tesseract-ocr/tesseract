/* -*-C-*-
 ********************************************************************************
 *
 * File:        debug.c  (Formerly debug.c)
 * Description:  Combinatorial Splitter
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 08:59:01 1989
 * Modified:     Tue Feb 19 10:34:36 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>
#include "debug.h"
#include "callcpp.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
MENU_ITEM menu_table[NUM_MENUS][NUM_MENU_ITEMS];

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * set_float_value
 *
 * Set the value of a floating point variable from the break handler.
 **********************************************************************/
int set_float_value(const char *message, float *variable) { 
  char this_string[CHARS_PER_LINE];

  cprintf ("%s (%7.5f) ? ", message, *variable);
  fflush(stdout); 

  if (fgets (this_string, CHARS_PER_LINE, stdin) == NULL)
    return (1);

  if (fgets (this_string, CHARS_PER_LINE, stdin) != NULL) {
    sscanf (this_string, "%f", variable);
    cprintf ("%s = %7.5f\n", message, *variable);
  }
  return (1);
}


/**********************************************************************
 * set_int_value
 *
 * Set the value of a floating point variable from the break handler.
 **********************************************************************/
int set_int_value(const char *message, int *variable) { 
  char this_string[CHARS_PER_LINE];

  cprintf ("%s (%d) ? ", message, *variable);
  fflush(stdout); 

  if (fgets (this_string, CHARS_PER_LINE, stdin) == NULL)
    return (1);

  if (fgets (this_string, CHARS_PER_LINE, stdin) != NULL) {
    sscanf (this_string, "%d", variable);
    cprintf ("%s = %d\n", message, *variable);
  }
  return (1);
}


/**********************************************************************
 * make_menu_item
 *
 * Create an entry in the menu handler table that will create a menu
 * entry.  When this entry is selected it will invoke the requested
 * function.
 **********************************************************************/
void make_menu_item(int menu,
                    int menu_item,
                    const char *menu_string,
                    int_void menu_funct) {
  menu_table[menu][menu_item].menu_string = menu_string;
  menu_table[menu][menu_item].menu_function = (void_proc) menu_funct;
}
