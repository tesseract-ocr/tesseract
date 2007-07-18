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
#include "efio.h"
#include "scanutils.h"
#include "serialis.h"
#include "unichar.h"
#include "globals.h"
#include <stdio.h>

#define REALLY_QUOTE_IT(x) QUOTE_IT(x)

#define MAX_CUTOFF      1000

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ReadNewCutoffs(const char *Filename,
                    CLASS_TO_INDEX ClassMapper,
                    CLASS_CUTOFF_ARRAY Cutoffs) {
/*
 **	Parameters:
 **		Filename	name of file containing cutoff definitions
 **             ClassMapper     array which maps class id's to class indexes
 **		Cutoffs		array to put cutoffs into
 **	Globals: none
 **	Operation: Open Filename, read in all of the class-id/cutoff pairs
 **		and insert them into the Cutoffs array.  Cutoffs are
 **		inserted in the array so that the array is indexed by
 **		class index rather than class id.  Unused entries in the
 **		array are set to an arbitrarily high cutoff value.
 **	Return: none
 **	Exceptions: none
 **	History: Wed Feb 20 09:38:26 1991, DSJ, Created.
 */
  FILE *CutoffFile;
  char Class[UNICHAR_LEN + 1];
  CLASS_ID ClassId;
  int Cutoff;
  int i;

  CutoffFile = Efopen (Filename, "r");

  for (i = 0; i < MAX_NUM_CLASSES; i++)
    Cutoffs[i] = MAX_CUTOFF;

  while (fscanf (CutoffFile, "%" REALLY_QUOTE_IT(UNICHAR_LEN) "s %d",
                 Class, &Cutoff) == 2) {
    ClassId = unicharset.unichar_to_id(Class);
    Cutoffs[ClassMapper[ClassId]] = Cutoff;
  }
  fclose(CutoffFile);

}                                /* ReadNewCutoffs */
