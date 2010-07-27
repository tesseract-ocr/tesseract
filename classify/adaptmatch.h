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

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
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
extern double_VAR_H(matcher_good_threshold, 0.125, "Good Match (0-1)");
extern double_VAR_H(matcher_great_threshold, 0.0, "Great Match (0-1)");
extern INT_VAR_H(matcher_failed_adaptations_before_reset, 150,
                 "Number of failed adaptions before adapted templates reset");
extern INT_VAR_H(matcher_min_examples_for_prototyping, 2,
               "Reliable Config Threshold");
extern BOOL_VAR_H(tess_cn_matching, 0, "Character Normalized Matching");
extern BOOL_VAR_H(tess_bn_matching, 0, "Baseline Normalized Matching");
extern INT_VAR_H(classify_learning_debug_level, 0, "Learning Debug Level: ");

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
int GetAdaptiveFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_FEATURE_ARRAY IntFeatures,
                        FEATURE_SET *FloatFeatures);

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
#endif
