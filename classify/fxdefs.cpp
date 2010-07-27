/******************************************************************************
 **	Filename:    fxdefs.c
 **	Purpose:     Utility functions to be used by feature extractors.
 **	Author:      Dan Johnson
 **	History:     Sun Jan 21 15:29:02 1990, DSJ, Created.
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
#include "fxdefs.h"
#include "featdefs.h"
#include "mf.h"
#include "outfeat.h"
#include "picofeat.h"
#include "normfeat.h"

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
/** flag to control learn mode vs. classify mode */
int ExtractMode;

// Definitions of extractors separated from feature definitions.
DefineFeatureExt (MicroFeatureExt, ExtractMicros)
DefineFeatureExt (PicoFeatExt, NULL)
DefineFeatureExt (CharNormExt, ExtractCharNormFeatures)
DefineFeatureExt (OutlineFeatExt, NULL)

FEATURE_EXT_STRUCT* ExtractorDefs[NUM_FEATURE_TYPES] = {
  &MicroFeatureExt,
  &PicoFeatExt,
  &OutlineFeatExt,
  &CharNormExt
};


/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void SetupExtractors() {
  for (int i = 0; i < NUM_FEATURE_TYPES; ++i)
    FeatureDefs.FeatureExtractors[i] = ExtractorDefs[i];
}

/**
 * This routine copies the relavent fields from the
 * Row struct to the LineStats struct.
 *
 * Globals: 
 * - none
 *
 * @param Row text row to get line statistics from
 * @param[out] LineStats place to put line statistics
 *
 * @note History: Mon Mar 11 10:38:43 1991, DSJ, Created.
 */
void GetLineStatsFromRow(TEXTROW *Row, LINE_STATS *LineStats) {
  LineStats->Baseline = &(Row->baseline);
  LineStats->XHeightLine = &(Row->xheight);
  LineStats->xheight = Row->lineheight;
  LineStats->AscRise = Row->ascrise;
  LineStats->DescDrop = Row->descdrop;
  LineStats->TextRow = Row;      /* kludge - only needed by fx for */
  /* fast matcher - remove later */

}                                /* GetLineStatsFromRow */
