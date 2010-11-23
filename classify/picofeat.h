/******************************************************************************
 **	Filename:    picofeat.h
 **	Purpose:     Definition of pico features.
 **	Author:      Dan Johnson
 **	History:     9/4/90, DSJ, Created.
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
#ifndef   PICOFEAT_H
#define   PICOFEAT_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "ocrfeatures.h"
#include "params.h"

typedef enum
{ PicoFeatY, PicoFeatDir, PicoFeatX }
PICO_FEAT_PARAM_NAME;

#define MAX_PICO_FEATURES (1000)

/*---------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------*/

extern double_VAR_H(classify_pico_feature_length, 0.05, "Pico Feature Length");


/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
#define GetPicoFeatureLength()  (PicoFeatureLength)

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern FLOAT32 PicoFeatureLength;
#endif
