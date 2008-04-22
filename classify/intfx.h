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
  inT32 Length;                  /* total length of all outlines   */
  inT16 Xmean, Ymean;            /* center of mass of all outlines */
  inT16 Rx, Ry;                  /* radius of gyration             */
  inT16 NumBL, NumCN;            /* number of features extracted   */
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

uinT8 TableLookup(inT32 Y, inT32 X);

int SaveFeature(INT_FEATURE_ARRAY FeatureArray,
                uinT16 FeatureNum,
                inT16 X,
                inT16 Y,
                uinT8 Theta);

uinT16 MySqrt(inT32 X, inT32 Y);

uinT8 MySqrt2(uinT16 N, uinT32 I, uinT8 *Exp);

void ClipRadius(uinT8 *RxInv, uinT8 *RxExp, uinT8 *RyInv, uinT8 *RyExp);
#endif
