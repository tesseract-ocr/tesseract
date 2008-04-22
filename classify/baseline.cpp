/* -*-C-*-
 ********************************************************************************
 *
 * File:        baseline.c  (Formerly baseline.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 16:16:13 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 **************************************************************************/

/*----------------------------------------------------------------------
              I n c l u d e s
---------------------------------------------------------------------*/
#include "baseline.h"
//#include "blobs.h"
#include "debug.h"
#include "hideedge.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef TPOINT SCALE;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
inT8 baseline_normalized = TRUE;

make_int_var (baseline_enable, 1, make_baseline_enable,
4, 3, set_baseline_enable, "Baseline Enable");
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * init_baseline
 *
 * Initialize the needed baseline variables.
 **********************************************************************/
void init_baseline() {
  make_baseline_enable();

  baseline_normalized = baseline_enable;
}
