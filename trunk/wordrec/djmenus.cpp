/* -*-C-*-
 ********************************************************************************
 *
 * File:        djmenus.c  (Formerly djmenus.c)
 * Description:  Menu creation and initialization
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 08:59:01 1989
 * Modified:     Fri Jul 12 13:33:29 1991 (Dan Johnson) danj@hpgrlj
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
#include <signal.h>
#include "djmenus.h"
#include "intmatcher.h"
#include "adaptmatch.h"
#include "badwords.h"
#include "sigmenu.h"
#include "mfoutline.h"
#include "normmatch.h"
#include "speckle.h"
#include "stopper.h"

#ifndef GRAPHICS_DISABLED
/*---------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
int handle_menu_16(); 
int handle_menu_17(); 
int handle_menu_18(); 
int handle_menu_19(); 
#endif

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * dj_cleanup
 *
 * Call at end of program to clean up
 **********************************************************************/
void dj_cleanup() { 
  EndAdaptiveClassifier(); 
}


/**********************************************************************
 * dj_statistics
 *
 * Call at end of program to print out statistics gathered
 **********************************************************************/
void dj_statistics(FILE *File) { 
  PrintAdaptiveStatistics(File); 
  PrintBadWords(File); 
}


/**********************************************************************
 * init_dj_debug
 *
 * Initialize the traps for handling the splitter debug stuff.
 **********************************************************************/
void init_dj_debug() { 
  static int first_time = 1;

  if (first_time) {
    first_time = 0;
    /* Set up the interrupts */
    #ifndef GRAPHICS_DISABLED
    #ifndef SECURE_NAMES
    AddSignalMenuItem (SIGINT, 16, "Int Matcher Menu      ...",
      handle_menu_16);
    AddSignalMenuItem (SIGINT, 17, "Stopping Criterion    ...",
      handle_menu_17);
    AddSignalMenuItem (SIGINT, 18, "Adaptive Matcher      ...",
      handle_menu_18);
    AddSignalMenuItem (SIGINT, 19, "Word Spacing          ...",
      handle_menu_19);
    #endif
    #endif
    InitAdaptiveClassifierVars(); 
    InitMFOutlineVars(); 
    InitNormProtoVars(); 
    InitIntProtoVars(); 
    InitIntegerMatcherVars(); 
    InitSpeckleVars(); 
    InitStopperVars(); 
  }
}


/**********************************************************************
 * handle_menu_X
 *
 * Initialize the traps for handling the debug stuff.
 **********************************************************************/
#ifndef GRAPHICS_DISABLED
handle_menu (16, handle_menu_16)
handle_menu (17, handle_menu_17)
handle_menu (18, handle_menu_18) handle_menu (19, handle_menu_19)
#endif
