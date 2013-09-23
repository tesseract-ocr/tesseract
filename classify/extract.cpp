/******************************************************************************
 **	Filename:    extract.c
 **	Purpose:     Generic high level feature extractor routines.
 **	Author:      Dan Johnson
 **	History:     Sun Jan 21 09:44:08 1990, DSJ, Created.
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
#include "extract.h"
#include "flexfx.h"
#include "danerror.h"

typedef CHAR_FEATURES (*CF_FUNC) ();

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void ExtractorStub(); 

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * Extract features from Blob by calling the feature
 * extractor which is currently being used.  This routine
 * simply provides a high level interface to feature
 * extraction.  The caller can extract any type of features
 * from a blob without understanding any lower level details.
 *
 * @param FeatureDefs	definitions of feature types/extractors
 * @param denorm	Normalize/denormalize to access original image
 * @param Blob		blob to extract features from
 *
 * @return The character features extracted from Blob.
 * @note Exceptions: none
 * @note History: Sun Jan 21 10:07:28 1990, DSJ, Created.
 */
CHAR_DESC ExtractBlobFeatures(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              const DENORM& bl_denorm, const DENORM& cn_denorm,
                              const INT_FX_RESULT_STRUCT& fx_info,
                              TBLOB *Blob) {
  return ExtractFlexFeatures(FeatureDefs, Blob, bl_denorm, cn_denorm, fx_info);
}                                /* ExtractBlobFeatures */

/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
ExtractorStub ()
/**
 * This routine is used to stub out feature extractors
 * that are no longer used.  It simply calls DoError.
 *
 * @note Exceptions: none
 * @note History: Wed Jan  2 14:16:49 1991, DSJ, Created.
 */
#define DUMMY_ERROR     1
{
  DoError (DUMMY_ERROR, "Selected feature extractor has been stubbed out!");
}                                /* ExtractorStub */
