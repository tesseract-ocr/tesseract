/**********************************************************************
 * File:        tessinit.c  (Formerly tessinit.c)
 * Description: Stuff from recog.c needed by tessedit.
 * Author:		Ray Smith
 * Created:		Thu Jan 23 09:33:59 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
#include "globals.h"
#include "variables.h"           /* Feature stuff */

#include <math.h>
#include <signal.h>
#include "tessinit.h"

/*----------------------------------------------------------------------
            Variables
----------------------------------------------------------------------*/
int corner_0;
int corner_1;
int corner_2;
int corner_3;

int row_number;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * program_variables
 *
 * Initialize all the things in the program that need to be initialized.
 **********************************************************************/
void program_variables() { 
  VALUE dummy;

  int_variable (plots_fx, "plots_fx", 0);
  int_variable (plots_ocr, "plots_ocr", 0);
  int_variable (debugs_fx, "debugs_fx", 0);
  int_variable (debugs_ocr, "debugs_ocr", 0);
                                 /* PREV DEFAULT 0 */
  int_variable (acts_fx, "acts_fx", 2048);
                                 /* PREV DEFAULT 0 */
  int_variable (acts_ocr, "acts_ocr", 32);
  int_variable (corner_0, "corner_0", 0);
  int_variable (corner_1, "corner_1", 0);
  int_variable (corner_2, "corner_2", 2550);
  int_variable (corner_3, "corner_3", 3508);
  int_variable (resolution, "resolution", 300);
  string_variable (debugfile, "debugfile", "");
}


/**********************************************************************
 * program_init
 *
 * Initialize all the things in the program that need to be initialized.
 **********************************************************************/
void program_init() { 
  /* Plots flags */
  plots[OCR] = plots_ocr;
  debugs[OCR] = debugs_ocr;
  acts[OCR] = acts_ocr;
  plots[FX] = plots_fx;
  debugs[FX] = debugs_fx;
  acts[FX] = acts_fx;

  corners[0] = corner_0;
  corners[1] = corner_1;
  corners[2] = corner_2;
  corners[3] = corner_3;
}


/**********************************************************************
 * matherr
 *
 * Trap procedure for the Standard Math library
 **********************************************************************/
#ifdef __UNIX
int
matherr (error)
struct exception *error;
{
  if (error->type == DOMAIN) {
    if (!strcmp (error->name, "sqrt")) {
      cprintf ("Sqrt:Domain error!!\n");
      abort(); 
    }
    if (!strcmp (error->name, "atan2")) {
      cprintf ("Arc Tangent error: atan2 (%ld, %ld)\n",
        error->arg1, error->arg2);
      abort(); 
    }
  }
  return 0;                      /*all others default */
}
#endif
