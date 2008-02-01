/* -*-C-*-
 ********************************************************************************
 *
 * File:        msmenus.c  (Formerly msmenus.c)
 * Description:  Menu creation and initialization
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 08:59:01 1989
 * Modified:     Fri Jul 12 16:04:50 1991 (Mark Seaman) marks@hpgrlt
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
---------------------------------------------------------------------*/
#include <stdio.h>
#include <signal.h>

#include "msmenus.h"
#include "debug.h"
#include "sigmenu.h"
/* includes for init functions */
#include "plotseg.h"
#include "baseline.h"
#include "bestfirst.h"
#include "split.h"
#include "associate.h"
#include "chop.h"
#include "tordvars.h"
#include "permute.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
int handle_menu_3();
int handle_menu_4();
int handle_menu_5();
int handle_menu_6();
int handle_menu_7();
int handle_menu_8();
int handle_menu_9();
#endif

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * init_ms_debug
 *
 * Initialize the traps for handling the splitter debug stuff.
 **********************************************************************/
void init_ms_debug() {
  static int first_time = 1;

  if (first_time) {
    first_time = 0;
    /* Set up the interrupts */
    #ifndef GRAPHICS_DISABLED
    #ifndef SECURE_NAMES
    AddSignalMenuItem (SIGINT, 3, "Chopper", handle_menu_3);
    AddSignalMenuItem (SIGINT, 4, "Text Order", handle_menu_4);
    AddSignalMenuItem (SIGINT, 5, "Graphics", handle_menu_5);
    AddSignalMenuItem (SIGINT, 6, "Text Display", handle_menu_6);
    AddSignalMenuItem (SIGINT, 7, "Similarity Matcher", handle_menu_7);
    AddSignalMenuItem (SIGINT, 8, "Context", handle_menu_8);
    AddSignalMenuItem (SIGINT, 9, "Joiner", handle_menu_9);
    #endif

    init_plotseg();
    init_render_vars();
    #endif

    init_baseline();
    init_bestfirst_vars();
    init_splitter_vars();
    init_associate_vars();
    init_chop();

    init_textord_vars();
    init_permute_vars();
  }
}


/**********************************************************************
 * handle_menu_X
 *
 * Initialize the traps for handling the splitter debug stuff.
 **********************************************************************/
#ifndef GRAPHICS_DISABLED
handle_menu (3, handle_menu_3)
handle_menu (4, handle_menu_4)
handle_menu (5, handle_menu_5)
handle_menu (6, handle_menu_6)
handle_menu (7, handle_menu_7)
handle_menu (8, handle_menu_8) handle_menu (9, handle_menu_9)
#endif
