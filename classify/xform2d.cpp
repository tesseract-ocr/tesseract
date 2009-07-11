/******************************************************************************
 ** Filename:    xform2d.c
 ** Purpose:     Library routines for performing 2D point transformations
 ** Author:      Dan Johnson
 ** History:     Fri Sep 22 09:54:17 1989, DSJ, Created.
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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "xform2d.h"
#include <math.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/

void InitMatrix(MATRIX_2D *M) {
  M->a = 1;
  M->b = 0;
  M->c = 0;
  M->d = 1;
  M->tx = 0;
  M->ty = 0;
}

void CopyMatrix(MATRIX_2D *A, MATRIX_2D *B) {
  B->a = A->a;
  B->b = A->b;
  B->c = A->c;
  B->d = A->d;
  B->tx = A->tx;
  B->ty = A->ty;
}

void TranslateMatrix(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y) {
  M->tx += M->a * X + M->c * Y;
  M->ty += M->b * X + M->d * Y;
}

void ScaleMatrix(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y) {
  M->a *= X;
  M->b *= X;
  M->c *= Y;
  M->d *= Y;
}

void MirrorMatrixInX(MATRIX_2D *M)  {ScaleMatrix(M, -1, 1);}
void MirrorMatrixInY(MATRIX_2D *M)  {ScaleMatrix(M, 1, -1);}
void MirrorMatrixInXY(MATRIX_2D *M) {ScaleMatrix(M, -1, -1);}

FLOAT32 MapX(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y) {
  return M->a * (X) + (M)->c * (Y) + (M)->tx;
}

FLOAT32 MapY(MATRIX_2D *M, FLOAT32 X, FLOAT32 Y) {
  return M->b * X + M->d * Y + M->ty;
}

void MapPoint(MATRIX_2D *M, const FPOINT &A, FPOINT* B) {
  B->x = MapX(M, A.x, A.y);
  B->y = MapY(M, A.x, A.y);
}

FLOAT32 MapDx(MATRIX_2D *M, FLOAT32 DX, FLOAT32 DY) {
  return M->a * DX + M->c * DY;
}

FLOAT32 MapDy(MATRIX_2D *M, FLOAT32 DX, FLOAT32 DY) {
  return M->b * DX + M->d * DY;
}


/*---------------------------------------------------------------------------*/
void RotateMatrix(MATRIX_2D_PTR Matrix, FLOAT32 Angle) {
/*
 ** Parameters:
 **   Matrix    transformation matrix to rotate
 **   Angle   angle to rotate matrix
 ** Globals: none
 ** Operation:
 **   Rotate the coordinate system (as specified by Matrix) about
 **   its origin by Angle radians.  In matrix notation the
 **   effect is as follows:
 **
 **     Matrix = R X Matrix
 **
 **   where R is the following matrix
 **
 **     cos Angle sin Angle 0
 **     -sin Angle  cos Angle 0
 **     0   0   1
 ** Return: none
 ** Exceptions: none
 ** History: 7/27/89, DSJ, Create.
 */
  FLOAT32 Cos, Sin;
  FLOAT32 NewA, NewB;

  Cos = cos ((double) Angle);
  Sin = sin ((double) Angle);

  NewA = Matrix->a * Cos + Matrix->c * Sin;
  NewB = Matrix->b * Cos + Matrix->d * Sin;
  Matrix->c = Matrix->a * -Sin + Matrix->c * Cos;
  Matrix->d = Matrix->b * -Sin + Matrix->d * Cos;
  Matrix->a = NewA;
  Matrix->b = NewB;

}                                /* RotateMatrix */
