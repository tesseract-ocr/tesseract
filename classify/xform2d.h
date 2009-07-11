/******************************************************************************
 ** Filename:    xform2d.h
 ** Purpose:     Definitions for using 2D point transformation library
 ** Author:      Dan Johnson
 ** History:     Fri Sep 22 09:57:08 1989, DSJ, Created.
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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
#ifndef   XFORM2D_H
#define   XFORM2D_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "fpoint.h"

typedef struct
{
  FLOAT32 a, b, c, d, tx, ty;
}


MATRIX_2D, *MATRIX_2D_PTR;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/

void InitMatrix(MATRIX_2D *M);
void CopyMatrix(MATRIX_2D *A, MATRIX_2D *B);

/* matrix scaling, translation, rotation, mirroring, etc.*/
void TranslateMatrix(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y);
void ScaleMatrix(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y);

void MirrorMatrixInX(MATRIX_2D *M);
void MirrorMatrixInY(MATRIX_2D *M);
void MirrorMatrixInXY(MATRIX_2D *M);

/* using a matrix to map points*/
FLOAT32 MapX(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y);

FLOAT32 MapY(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y);

void MapPoint(MATRIX_2D *M, const FPOINT &A, FPOINT* B);

FLOAT32 MapDx(MATRIX_2D *M, FLOAT32 DX, FLOAT32 DY);
FLOAT32 MapDy(MATRIX_2D M, FLOAT32 DX, FLOAT32 DY);

  void RotateMatrix(MATRIX_2D_PTR Matrix, FLOAT32 Angle);
#endif
