/******************************************************************************
 **	Filename:    badwords.c
 **	Purpose:     Routines to keep the bad words in sorted order.
 **	Author:      Dan Johnson
 **	History:     Thu Apr 25 08:40:19 1991, DSJ, Created.
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

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "general.h"
#include "oldheap.h"
#include "callcpp.h"

#include <string.h>
#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#define MAX_NUM_BAD_WERDS 1000

/*----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
/** heap that bad words are stored in */
static HEAP *BadWords = NULL;
BOOL_VAR (tessedit_save_stats, FALSE, "Save final recognition statistics");

/*----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine prints the bad words stored in BadWords
 * to file ordered by certainty (worst certainty first).
 *
 * @param File open text file to print bad words to
 *
 * Globals: 
 * - BadWords heap that bad words are stored in
 *
 * @note Exceptions: none
 * @note History: Thu Apr 25 08:57:08 1991, DSJ, Created.
 */
void PrintBadWords(FILE *File) {
  HEAPENTRY NextWord;

  if (BadWords == NULL)
    return;

  fprintf (File, "\n");
  fprintf (File, "Bad Word         Certainty\n");
  fprintf (File, "---------------- ---------\n");
  while (GetTopOfHeap (BadWords, &NextWord) != EMPTY) {
    fprintf (File, "%16s %6.2f\n", (const char *) NextWord.Data,
      NextWord.Key);
    free_string ((char *) NextWord.Data);
  }
  fprintf (File, "\n");

}                                /* PrintBadWords */


/*---------------------------------------------------------------------------*/
/**
 * This routine saves all words flagged as bad in a heap
 * with the worst word on the top of the heap.  The contents
 * of this heap can be printed to a file by calling
 * PrintBadWords (File).
 *
 * @param Word bad word to be saved
 * @param Certainty certainty of word
 *
 * Globals: 
 * - BadWords heap to keep bad words in
 *
 * History: Thu Apr 25 08:41:00 1991, DSJ, Created.
 */
void SaveBadWord(const char *Word, FLOAT32 Certainty) {
  HEAPENTRY NewWord;

  assert (Word != NULL);

  if (BadWords == NULL) {
    BadWords = MakeHeap (MAX_NUM_BAD_WERDS);
    InitHeap(BadWords);
  } else if (HeapFull(BadWords)) {
    return;
  }

  NewWord.Key = Certainty;
  NewWord.Data = alloc_string (strlen (Word) + 1);
  strcpy ((char *) NewWord.Data, Word);
  HeapStore(BadWords, &NewWord);

}                                /* SaveBadWord */
