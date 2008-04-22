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

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define BASELINE_OFFSET 64
#define BASELINE_SCALE  128

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern inT8 baseline_normalized;
extern int baseline_enable;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * is_baseline_normalized
 *
 * Check the baseline_normalized flag to see if it is set.
 **********************************************************************/
#define is_baseline_normalized()  \
(baseline_normalized)

/**********************************************************************
 * reset_baseline_normalized
 *
 * Reset the baseline_normalized flag to show that it is not being done.
 **********************************************************************/
#define reset_baseline_normalized()  \
(baseline_normalized = FALSE)

/**********************************************************************
 * set_baseline_normalized
 *
 * Set the baseline_normalized flag to show that it is being done.
 **********************************************************************/
#define set_baseline_normalized()  \
(baseline_normalized = TRUE)

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void init_baseline();

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* baseline.c
void init_baseline
  _ARGS((void));

#undef _ARGS
*/
#endif
