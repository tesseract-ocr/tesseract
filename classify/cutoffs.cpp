/******************************************************************************
 **	Filename:    cutoffs.c
 **	Purpose:     Routines to manipulate an array of class cutoffs.
 **	Author:      Dan Johnson
 **	History:     Wed Feb 20 09:28:51 1991, DSJ, Created.
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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "cutoffs.h"

#include <stdio.h>

#include "classify.h"
#include "efio.h"
#include "globals.h"
#include "helpers.h"
#include "scanutils.h"
#include "serialis.h"
#include "unichar.h"

#define REALLY_QUOTE_IT(x) QUOTE_IT(x)

#define MAX_CUTOFF      1000

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
namespace tesseract {
void Classify::ReadNewCutoffs(FILE *CutoffFile, bool swap, inT64 end_offset,
                              CLASS_CUTOFF_ARRAY Cutoffs) {
/*
 **	Parameters:
 **		Filename	name of file containing cutoff definitions
 **		Cutoffs		array to put cutoffs into
 **	Globals: none
 **	Operation: Open Filename, read in all of the class-id/cutoff pairs
 **		and insert them into the Cutoffs array.  Cutoffs are
 **		indexed in the array by class id.  Unused entries in the
 **		array are set to an arbitrarily high cutoff value.
 **	Return: none
 **	Exceptions: none
 **	History: Wed Feb 20 09:38:26 1991, DSJ, Created.
 */
  char Class[UNICHAR_LEN + 1];
  CLASS_ID ClassId;
  int Cutoff;
  int i;

  if (shape_table_ != NULL) {
    if (!shapetable_cutoffs_.DeSerialize(swap, CutoffFile)) {
      tprintf("Error during read of shapetable pffmtable!\n");
    }
  }
  for (i = 0; i < MAX_NUM_CLASSES; i++)
    Cutoffs[i] = MAX_CUTOFF;

  while ((end_offset < 0 || ftell(CutoffFile) < end_offset) &&
         fscanf(CutoffFile, "%" REALLY_QUOTE_IT(UNICHAR_LEN) "s %d",
                Class, &Cutoff) == 2) {
    if (strcmp(Class, "NULL") == 0) {
      ClassId = unicharset.unichar_to_id(" ");
    } else {
      ClassId = unicharset.unichar_to_id(Class);
    }
    Cutoffs[ClassId] = Cutoff;
    SkipNewline(CutoffFile);
  }
}                                /* ReadNewCutoffs */

}  // namespace tesseract
