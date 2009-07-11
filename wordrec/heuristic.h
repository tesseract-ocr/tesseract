/* -*-C-*-
 ********************************************************************************
 *
 * File:        heuristic.h  (Formerly heuristic.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  9 17:14:44 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef HEURISTIC_H
#define HEURISTIC_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "associate.h"
#include "bestfirst.h"
#include "seam.h"

extern INT_VAR_H(segment_adjust_debug, 0,
       "Segmentation adjustment debug");
extern BOOL_VAR_H(assume_fixed_pitch_char_segment, 0,
       "include fixed-pitch heuristics in char segmentation");
extern BOOL_VAR_H(use_new_state_cost, 0,
       "use new state cost heuristics for segmentation evaluation");
extern double_VAR_H(heuristic_segcost_rating_base, 1.25,
       "base factor for adding segmentation cost into word rating."
       "It's a multiplying factor, the larger the value above 1, "
       "the bigger the effect of segmentation cost.");
extern double_VAR_H(heuristic_weight_rating, 1,
       "weight associated with char rating in combined cost of state");
extern double_VAR_H(heuristic_weight_width, 0,
       "weight associated with width evidence in combined cost of state");
extern double_VAR_H(heuristic_weight_seamcut, 0,
       "weight associated with seam cut in combined cost of state");
extern double_VAR_H(heuristic_max_char_wh_ratio, MAX_SQUAT,
       "max char width-to-height ratio allowed in segmentation");

#endif
