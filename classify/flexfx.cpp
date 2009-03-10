/******************************************************************************
 **	Filename:    flexfx.c
 **	Purpose:     Interface to flexible feature extractor.
 **	Author:      Dan Johnson
 **	History:     Wed May 23 13:45:10 1990, DSJ, Created.
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
#include "flexfx.h"
#include "featdefs.h"
#include "variables.h"
#include "sigmenu.h"
#include "emalloc.h"
#include <string.h>
#include <stdio.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
CHAR_DESC ExtractFlexFeatures(TBLOB *Blob, LINE_STATS *LineStats) {
/*
 **	Parameters:
 **		Blob		blob to extract features from
 **		LineStats	statistics about text line Blob is on
 **	Globals: none
 **	Operation: Allocate a new character descriptor and fill it in by
 **		calling all feature extractors which are enabled.
 **	Return: Structure containing features extracted from Blob.
 **	Exceptions: none
 **	History: Wed May 23 13:46:22 1990, DSJ, Created.
 */
  int Type;
  CHAR_DESC CharDesc;

  CharDesc = NewCharDescription ();

  for (Type = 0; Type < CharDesc->NumFeatureSets; Type++)
    if (FeatureDefs.FeatureExtractors[Type] != NULL &&
        FeatureDefs.FeatureExtractors[Type]->Extractor != NULL)
      CharDesc->FeatureSets[Type] =
        (FeatureDefs.FeatureExtractors[Type])->Extractor (Blob, LineStats);

  return (CharDesc);

}                                /* ExtractFlexFeatures */


/*---------------------------------------------------------------------------*/
void
InitFlexFXVars ()
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: Add any control variables used by the feature extractors
 **		to the variable system.  This includes the enable flag for
 **		each individual extractor.  This routine needs to create
 **		a separate name for the enable for each feature extractor
 **		and allocate a string to contain that name.  This is
 **		necessary since the "variables" routines do not create
 **		copies of the string names passed to them.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 23 15:59:23 1990, DSJ, Created.
 */
#define NamePrefix      "Enable"
#define NameSuffix      "Features"
{
  int Type;

  SetupExtractors();
  for (Type = 0; Type < FeatureDefs.NumFeatureTypes; Type++) {
    (FeatureDefs.FeatureExtractors[Type])->InitExtractorVars ();
  }
}                                /* InitFlexFXVars */
