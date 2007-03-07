/******************************************************************************
 **	Filename:    intfx.h
 **	Purpose:     Interface to high level integer feature extractor.
 **	Author:      Robert Moss
 **	History:     Tue May 21 15:51:57 MDT 1991, RWM, Created.
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
#ifndef   INTFX_H
#define   INTFX_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "tessclas.h"
#include "hideedge.h"
#include "intproto.h"
#include <math.h>

typedef struct
{
  INT32 Length;                  /* total length of all outlines   */
  INT16 Xmean, Ymean;            /* center of mass of all outlines */
  INT16 Rx, Ry;                  /* radius of gyration             */
  INT16 NumBL, NumCN;            /* number of features extracted   */
}


INT_FX_RESULT_STRUCT, *INT_FX_RESULT;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void InitIntegerFX(); 

int ExtractIntFeat(TBLOB *Blob,
                   INT_FEATURE_ARRAY BLFeat,
                   INT_FEATURE_ARRAY CNFeat,
                   INT_FX_RESULT Results);

UINT8 TableLookup(INT32 Y, INT32 X); 

int SaveFeature(INT_FEATURE_ARRAY FeatureArray,
                UINT16 FeatureNum,
                INT16 X,
                INT16 Y,
                UINT8 Theta);

UINT16 MySqrt(INT32 X, INT32 Y); 

UINT8 MySqrt2(UINT16 N, UINT32 I, UINT8 *Exp); 

void ClipRadius(UINT8 *RxInv, UINT8 *RxExp, UINT8 *RyInv, UINT8 *RyExp); 
#endif
