/******************************************************************************
 **	Filename:    adaptmatch.h
 **	Purpose:     Interface to high-level adaptive matcher
 **	Author:      Dan Johnson
 **	History:     Mon Mar 11 11:48:48 1991, DSJ, Created.
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
#ifndef ADAPTMATCH_H
#define ADAPTMATCH_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "oldlist.h"
#include "tessclas.h"
#include "fxdefs.h"
#include "matchdefs.h"
#include "adaptive.h"
#include "ocrfeatures.h"
#include "ratngs.h"

/*---------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------*/
extern float GoodAdaptiveMatch;
extern float GreatAdaptiveMatch;
extern int ReliableConfigThreshold;
extern int tess_cn_matching;
extern int tess_bn_matching;
extern int LearningDebugLevel;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
LIST AdaptiveClassifier(TBLOB *Blob, TBLOB *DotBlob, TEXTROW *Row);
/**/
void AdaptToWord(TWERD *Word,
                 TEXTROW *Row,
                 const WERD_CHOICE& BestChoice,
                 const WERD_CHOICE& BestRawChoice,
                 const char *rejmap);

void EndAdaptiveClassifier();

void InitAdaptiveClassifier();

void ResetAdaptiveClassifier();

void InitAdaptiveClassifierVars();

void PrintAdaptiveStatistics(FILE *File);

void SettupPass1();

void SettupPass2();

void MakeNewAdaptedClass(TBLOB *Blob,
                         LINE_STATS *LineStats,
                         CLASS_ID ClassId,
                         ADAPT_TEMPLATES Templates);

int GetAdaptiveFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_FEATURE_ARRAY IntFeatures,
                        FEATURE_SET *FloatFeatures);

int AdaptableWord(TWERD *Word,
                  const char *BestChoice,
                  const char *BestChoice_lengths,
                  const char *BestRawChoice,
                  const char *BestRawChoice_lengths);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#endif
