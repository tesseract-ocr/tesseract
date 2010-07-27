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

#include <math.h>
#include <signal.h>
#include "tessinit.h"
#include "wordrec.h"
#include "varable.h"

/*----------------------------------------------------------------------
            Variables
----------------------------------------------------------------------*/

INT_VAR(wordrec_plots_fx, 0, "plots_fx");
INT_VAR(wordrec_plots_ocr, 0, "plots_ocr");
INT_VAR(wordrec_debugs_fx, 0, "debugs_fx");
INT_VAR(wordrec_debugs_ocr, 0, "debugs_ocr");
INT_VAR(wordrec_acts_fx, 2048, "acts_fx");
INT_VAR(wordrec_acts_ocr, 32, "acts_ocr");
INT_VAR(wordrec_corner_0, 0, "wordrec_corner_0");
INT_VAR(wordrec_corner_1, 0, "wordrec_corner_1");
INT_VAR(wordrec_corner_2, 2550, "wordrec_corner_2");
INT_VAR(wordrec_corner_3, 3508, "wordrec_corner_3");
INT_VAR(wordrec_resolution, 300, "Resolution");
STRING_VAR(wordrec_debugfile, "debugfile", "Debug file name");

int row_number;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

namespace tesseract {
/**
 * @name program_init
 *
 * Initialize all the things in the program that need to be initialized.
 */
void Wordrec::program_init() {
  /* Plots flags */
  plots[OCR] = wordrec_plots_ocr;
  debugs[OCR] = wordrec_debugs_ocr;
  acts[OCR] = wordrec_acts_ocr;
  plots[FX] = wordrec_plots_fx;
  debugs[FX] = wordrec_debugs_fx;
  acts[FX] = wordrec_acts_fx;

  corners[0] = wordrec_corner_0;
  corners[1] = wordrec_corner_1;
  corners[2] = wordrec_corner_2;
  corners[3] = wordrec_corner_3;
}
}  // namespace tesseract


/**
 * @name matherr
 *
 * Trap procedure for the Standard Math library
 */
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
