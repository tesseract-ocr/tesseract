/* -*-C-*-
 ********************************************************************************
 *
 * File:        permdawg.h  (Formerly permdawg.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 16:45:29 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef PERMDAWG_H
#define PERMDAWG_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "dawg.h"
#include "choices.h"
#include "choicearr.h"
#include "varable.h"
#include "permute.h"

/*---------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern BOOL_VAR_H(segment_dawg_debug, 0, "Debug mode for word segmentation");

extern double_VAR_H(segment_penalty_dict_case_bad, 1.3125,
           "Default score multiplier for word matches, which may have case or "
           "punctuation issues (lower is better).");

extern double_VAR_H(segment_penalty_dict_case_ok, 1.1,
           "Score multiplier for word matches that have good case "
           "(lower is better).");

extern double_VAR_H(segment_penalty_dict_frequent_word, 1.0,
           "Score multiplier for word matches which have good case and are "
           "frequent in the given language (lower is better).");


/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
            Public Function Prototypes
----------------------------------------------------------------------*/

#endif
