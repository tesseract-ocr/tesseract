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

#include "genericvector.h"
#include "params.h"
#include "states.h"
#include "unichar.h"

typedef uinT8 BLOB_WIDTH;

struct DANGERR_INFO {
  DANGERR_INFO() :
    begin(-1), end(-1), dangerous(false), correct_is_ngram(false) {}
  DANGERR_INFO(int b, int e, bool d, bool n) :
    begin(b), end(e), dangerous(d), correct_is_ngram(n) {}
  int begin;
  int end;
  bool dangerous;
  bool correct_is_ngram;
};

typedef GenericVector<DANGERR_INFO> DANGERR;

enum ACCEPTABLE_CHOICE_CALLER { CHOPPER_CALLER, ASSOCIATOR_CALLER };

struct CHAR_CHOICE {
  UNICHAR_ID Class;
  uinT16 NumChunks;
  float Certainty;
};

struct VIABLE_CHOICE_STRUCT {
  float Rating;
  float Certainty;
  FLOAT32 AdjustFactor;
  int Length;
  bool ComposedFromCharFragments;
  CHAR_CHOICE Blob[1];
};

typedef VIABLE_CHOICE_STRUCT *VIABLE_CHOICE;

#endif
