/******************************************************************************
 **	Filename:	mfx.h
 **	Purpose:	Definition of micro-feature extraction routines
 **	Author:		Dan Johnson
 **	History:	5/29/89, DSJ, Created.
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
#ifndef   MFX_H
#define   MFX_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "mfdefs.h"
#include "params.h"
/**----------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------**/

/* old numbers corresponded to 10.0 degrees and 80.0 degrees */
extern double_VAR_H(classify_min_slope, 0.414213562,
                    "Slope below which lines are called horizontal");
extern double_VAR_H(classify_max_slope, 2.414213562,
                    "Slope above which lines are called vertical");

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
MICROFEATURES BlobMicroFeatures(TBLOB* Blob, const DENORM& cn_denorm);

#endif
