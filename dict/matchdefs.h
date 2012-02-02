/******************************************************************************
 **	Filename:    matchdefs.h
 **	Purpose:     Generic interface definitions for feature matchers.
 **	Author:      Dan Johnson
 **	History:     Fri Jan 19 09:21:25 1990, DSJ, Created.
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
#ifndef   MATCHDEFS_H
#define   MATCHDEFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "host.h"
#include <stdio.h>
#include "unichar.h"

/* define the maximum number of classes defined for any matcher
  and the maximum class id for any matcher. This must be changed
  if more different classes need to be classified */
#define MAX_NUM_CLASSES   12288
#define MAX_CLASS_ID    (MAX_NUM_CLASSES - 1)

/** a CLASS_ID is the ascii character to be associated with a class */
typedef UNICHAR_ID CLASS_ID;
#define NO_CLASS      (0)

/** a PROTO_ID is the index of a prototype within it's class.  Valid proto
  id's are 0 to N-1 where N is the number of prototypes that make up the
  class. */
typedef inT16 PROTO_ID;
#define NO_PROTO  (-1)

/** FEATURE_ID is the index of a feature within a character description
  The feature id ranges from 0 to N-1 where N is the number
  of features in a character description. */
typedef uinT8 FEATURE_ID;
#define NO_FEATURE      255
#define NOISE_FEATURE   254
#define MISSING_PROTO   254
#define MAX_NUM_FEAT    40
#define MAX_FEATURE_ID    250

/** a RATING is the match rating returned by a classifier.
  Higher is better. */
typedef FLOAT32 RATING;

/** a CERTAINTY is an indication of the degree of confidence of the
  classifier.  Higher is better.  0 means the match is as good as the
  mean of the matches seen in training.  -1 means the match was one
  standard deviation worse than the training matches, etc. */
typedef FLOAT32 CERTAINTY;

/** define a data structure to hold a single match result */
typedef struct
{
  CLASS_ID Class;
  RATING Rating;
  CERTAINTY Certainty;
}


MATCH_RESULT;

/** define a data structure for holding an array of match results */
typedef MATCH_RESULT SORTED_CLASSES[MAX_CLASS_ID + 1];

/*----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------*/
/**
  all feature matchers that are to be used with the high level
  classifier must support the following interface.  The names will, of
  course, be unique for each different matcher.  Note also that
  FEATURE_STRUCT is a data structure that is defined specifically for
  each feature extractor/matcher pair.
*/

/* misc test functions for proto id's and feature id's */
#define IsValidFeature(Fid) ((Fid) < MAX_FEATURE_ID)
#define IsValidProto(Pid) ((Pid) >= 0)

#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif

/* matchdefs.c */
int CompareMatchResults
_ARGS ((MATCH_RESULT * Result1, MATCH_RESULT * Result2));

void PrintMatchResult _ARGS ((FILE * File, MATCH_RESULT * MatchResult));

void PrintMatchResults
_ARGS ((FILE * File, int N, MATCH_RESULT MatchResults[]));

#undef _ARGS

/*----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
#endif
