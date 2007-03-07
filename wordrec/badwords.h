/******************************************************************************
 **	Filename:    badwords.h
 **	Purpose:     Routines to keep the bad words in sorted order.
 **	Author:      Dan Johnson
 **	History:     Thu Apr 25 09:06:48 1991, DSJ, Created.
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
#ifndef __BADWERDS__
#define __BADWERDS__

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include <stdio.h>

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void PrintBadWords(FILE *File); 

void SaveBadWord(const char *Word, FLOAT32 Certainty); 
extern BOOL_VAR_H (tessedit_save_stats, FALSE, "Save final recognition statistics");

/*
#if defined(__STDC__) || defined(__cplusplus)
# define        _ARGS(s) s
#else
# define        _ARGS(s) ()
#endif*/

/* badwords.c
void PrintBadWords
    _ARGS((FILE *File));

void SaveBadWord
    _ARGS((char *Word,
  FLOAT32 Certainty));

#undef _ARGS
*/
#endif
