/* -*-C-*-
 ********************************************************************************
 *
 * File:        associate.c  (Formerly associate.c)
 * Description:  Associate the outlines and classify them
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Jan 30 14:03:25 1990
 * Modified:     Mon Jul 22 10:48:01 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 ********************************************************************************
 */

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#include "associate.h"
#include "debug.h"
#include "callcpp.h"

extern TBLOB *newblob(); 

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

make_toggle_var (enable_assoc, 1, make_enable_assoc,
9, 4, toggle_assoc, "Associator Enable");

EVALUATION_ARRAY last_segmentation;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * init_associate_vars
 *
 * Create and initialize references to debug variables that control
 * operations in this file.
 **********************************************************************/
void init_associate_vars() { 
  make_enable_assoc(); 
}
