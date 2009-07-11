/* -*-C-*-
 ********************************************************************************
 *
 * File:        baseline.h  (Formerly baseline.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Feb 27 13:39:35 1991 (Mark Seaman) marks@hpgrlt
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
 *************************************************************************/
#ifndef BASELINE_H
#define BASELINE_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "general.h"
#include "tessclas.h"
#include "varable.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define BASELINE_OFFSET 64
#define BASELINE_SCALE  128

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

extern INT_VAR_H(classify_baseline_normalized, 1, "Baseline Enable");

#endif
