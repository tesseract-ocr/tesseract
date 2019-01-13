/******************************************************************************
 ** Filename:    cutoffs.h
 ** Purpose:     Routines to manipulate an array of class cutoffs.
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

#ifndef CUTOFFS_H
#define CUTOFFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "matchdefs.h"

typedef uint16_t CLASS_CUTOFF_ARRAY[MAX_NUM_CLASSES];

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* cutoffs.c
void ReadNewCutoffs
  _ARGS((char *Filename,
  CLASS_CUTOFF_ARRAY Cutoffs));
#undef _ARGS
*/
#endif
