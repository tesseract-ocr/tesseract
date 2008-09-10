/******************************************************************************
 **	Filename:    stopper.h
 **	Purpose:     Stopping criteria for word classifier.
 **	Author:      Dan Johnson
 **	History:     Wed May  1 09:42:57 1991, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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
#ifndef   STOPPER_H
#define   STOPPER_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/

#include "ratngs.h"
#include "states.h"
#include "varable.h"

typedef uinT8 BLOB_WIDTH;

typedef struct
{
  inT16 index;
  unsigned bad_length:8;
  unsigned good_length:8;
} DANGERR;

enum ACCEPTABLE_CHOICE_CALLER { CHOPPER_CALLER, ASSOCIATOR_CALLER };
typedef struct
{
  UNICHAR_ID Class;
  uinT16 NumChunks;
  float Certainty;
}


CHAR_CHOICE;

typedef struct
{
  float Rating;
  float Certainty;
  FLOAT32 AdjustFactor;
  int Length;
  bool ComposedFromCharFragments;
  CHAR_CHOICE Blob[1];
} VIABLE_CHOICE_STRUCT;
typedef VIABLE_CHOICE_STRUCT *VIABLE_CHOICE;

/*---------------------------------------------------------------------------
          Variables
---------------------------------------------------------------------------*/
extern double_VAR_H(stopper_certainty_per_char, -0.50,
     "Certainty to add for each dict char above small word size.");

extern double_VAR_H(stopper_nondict_certainty_base, -2.50,
    "Certainty threshold for non-dict words");

extern double_VAR_H(stopper_phase2_certainty_rejection_offset, 1.0,
           "Reject certainty offset");

extern INT_VAR_H(stopper_debug_level, 0, "Stopper debug level");


/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/
#define DisableChoiceAccum()  (KeepWordChoices = FALSE)
#define EnableChoiceAccum() (KeepWordChoices = TRUE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
int AlternativeChoicesWorseThan(FLOAT32 Threshold);

void FilterWordChoices();

void FindClassifierErrors(FLOAT32 MinRating,
                          FLOAT32 MaxRating,
                          FLOAT32 RatingMargin,
                          FLOAT32 Thresholds[]);

void InitStopperVars();

void InitChoiceAccum();

void LogNewSegmentation(PIECES_STATE BlobWidth);

void LogNewSplit(int Blob);

void SettupStopperPass1();

void SettupStopperPass2();

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern BOOL8 KeepWordChoices;
#endif
