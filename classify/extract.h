/******************************************************************************
 **	Filename:    extract.h
 **	Purpose:     Interface to high level generic feature extraction.
 **	Author:      Dan Johnson
 **	History:     1/21/90, DSJ, Created.
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
#ifndef   EXTRACT_H
#define   EXTRACT_H

#include "featdefs.h"
#include <stdio.h>

class DENORM;

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
// Deprecated! Will be deleted soon!
// In the meantime, as all TBLOBs, Blob is in baseline normalized coords.
// See SetupBLCNDenorms in intfx.cpp for other args.
CHAR_DESC ExtractBlobFeatures(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              const DENORM& bl_denorm, const DENORM& cn_denorm,
                              const INT_FX_RESULT_STRUCT& fx_info, TBLOB *Blob);

/*---------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------*/
void ExtractorStub();
#endif
