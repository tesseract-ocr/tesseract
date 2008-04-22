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
#include "choicearr.h"
#include "states.h"

typedef uinT8 BLOB_WIDTH;

typedef struct
{
  inT16 index;
  unsigned bad_length:8;
  unsigned good_length:8;
} DANGERR;

/*---------------------------------------------------------------------------
          Variables
---------------------------------------------------------------------------*/
extern float CertaintyPerChar;
extern float NonDictCertainty;
extern float RejectCertaintyOffset;
extern int StopperDebugLevel;

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/
#define DisableChoiceAccum()  (KeepWordChoices = FALSE)
#define EnableChoiceAccum() (KeepWordChoices = TRUE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
int AcceptableChoice(CHOICES_LIST Choices,
                     A_CHOICE *BestChoice,
                     A_CHOICE *RawChoice,
                     DANGERR *fixpt);

int AcceptableResult(A_CHOICE *BestChoice, A_CHOICE *RawChoice);

int AlternativeChoicesWorseThan(FLOAT32 Threshold);

int CurrentBestChoiceIs(const char *Word, const char *Word_lengths);

FLOAT32 CurrentBestChoiceAdjustFactor();

int CurrentWordAmbig();

void DebugWordChoices();

void FilterWordChoices();

void FindClassifierErrors (FLOAT32 MinRating,
FLOAT32 MaxRating,
FLOAT32 RatingMargin, FLOAT32 Thresholds[]);

void InitStopperVars();

void InitChoiceAccum();

void LogNewRawChoice (A_CHOICE * Choice,
FLOAT32 AdjustFactor, float Certainties[]);

void LogNewSegmentation(PIECES_STATE BlobWidth);

void LogNewSplit(int Blob);

void LogNewWordChoice (A_CHOICE * Choice,
FLOAT32 AdjustFactor, float Certainties[]);

int NoDangerousAmbig(const char *Word,
                     const char *Word_lengths,
                     DANGERR *fixpt);
void EndDangerousAmbigs();

void SettupStopperPass1();

void SettupStopperPass2();

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern BOOL8 KeepWordChoices;
#endif
