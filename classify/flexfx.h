/******************************************************************************
 **	Filename:    flexfx.h
 **	Purpose:     Interface to flexible feature extractor.
 **	Author:      Dan Johnson
 **	History:     Wed May 23 13:36:58 1990, DSJ, Created.
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
#ifndef   FLEXFX_H
#define   FLEXFX_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "featdefs.h"
#include <stdio.h>

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
// As with all TBLOBs this one is also baseline normalized.
CHAR_DESC ExtractFlexFeatures(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              TBLOB *Blob, const DENORM& bl_denorm,
                              const DENORM& cn_denorm,
                              const INT_FX_RESULT_STRUCT& fx_info);

#endif
