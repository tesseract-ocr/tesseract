/******************************************************************************
 **	Filename:    xform2d.c
 **	Purpose:     Library routines for performing 2D point transformations
 **	Author:      Dan Johnson
 **	History:     Fri Sep 22 09:54:17 1989, DSJ, Created.
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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "xform2d.h"
#include <math.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void RotateMatrix(MATRIX_2D_PTR Matrix, FLOAT32 Angle) { 
/*
 **	Parameters:
 **		Matrix		transformation matrix to rotate
 **		Angle		angle to rotate matrix
 **	Globals: none
 **	Operation:
 **		Rotate the coordinate system (as specified by Matrix) about
 **		its origin by Angle radians.  In matrix notation the
 **		effect is as follows:
 **
 **			Matrix = R X Matrix
 **
 **		where R is the following matrix
 **
 **			cos Angle	sin Angle	0
 **			-sin Angle	cos Angle	0
 **			0		0		1
 **	Return: none
 **	Exceptions: none
 **	History: 7/27/89, DSJ, Create.
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
