/* -*-C-*-
 ********************************************************************************
 *
 * File:        permnum.h  (Formerly permnum.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 16:30:03 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
#ifndef PERMNUM_H
#define PERMNUM_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "choicearr.h"
#include "varable.h"
#include "permute.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define GOOD_NUMBER     1.1
#define OK_NUMBER     1.4

extern double_VAR_H(segment_penalty_number_good, GOOD_NUMBER,
                    "Score multiplier for good-looking numbers "
                    "(lower is better).");

extern double_VAR_H(segment_penalty_number_ok, OK_NUMBER,
                    "Score multiplier for ok-looking numbers "
                    "(lower is better).");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

void init_permnum();
#endif
