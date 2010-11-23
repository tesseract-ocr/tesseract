/******************************************************************************
 **	Filename:    normfeat.h
 **	Purpose:     Definition of character normalization features.
 **	Author:      Dan Johnson
 **	History:     12/14/90, DSJ, Created.
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
#ifndef   NORMFEAT_H
#define   NORMFEAT_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "ocrfeatures.h"

#define LENGTH_COMPRESSION  (10.0)

typedef enum {
  CharNormY, CharNormLength, CharNormRx, CharNormRy
} NORM_PARAM_NAME;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
FLOAT32 ActualOutlineLength(FEATURE Feature);

FEATURE_SET ExtractCharNormFeatures(TBLOB *Blob, const DENORM& denorm);

#endif
