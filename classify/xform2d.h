/******************************************************************************
 **	Filename:    xform2d.h
 **	Purpose:     Definitions for using 2D point transformation library
 **	Author:      Dan Johnson
 **	History:     Fri Sep 22 09:57:08 1989, DSJ, Created.
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
          Macros
----------------------------------------------------------------------------**/
/* macros for initializing transform matrices */
#define InitMatrix(M)		((M)->a = 1, (M)->b = 0,		\
				(M)->c = 0, (M)->d = 1,		\
				(M)->tx = 0, (M)->ty = 0 )

#define CopyMatrix(A,B)		((B)->a = (A)->a, (B)->b = (A)->b,	\
				(B)->c = (A)->c, (B)->d = (A)->d,	\
				(B)->tx = (A)->tx, (B)->ty = (A)->ty)

/* matrix scaling, translation, rotation, mirroring, etc.*/
#define TranslateMatrix(M,X,Y)	((M)->tx += (M)->a * (X) + (M)->c * (Y),  \
				(M)->ty += (M)->b * (X) + (M)->d * (Y) )

#define ScaleMatrix(M,X,Y)	((M)->a *= (X), (M)->b *= (X),		\
				(M)->c *= (Y), (M)->d *= (Y))

#define MirrorMatrixInX(M)  (ScaleMatrix((M),-1,1))
#define MirrorMatrixInY(M)  (ScaleMatrix((M),1,-1))
#define MirrorMatrixInXY(M) (ScaleMatrix((M),-1,-1))

/* using a matrix to map points*/
#define MapX(M,X,Y)   ((M)->a * (X) + (M)->c * (Y) + (M)->tx)
#define MapY(M,X,Y)   ((M)->b * (X) + (M)->d * (Y) + (M)->ty)
#define MapPoint(M,A,B)		(Xof(B) = MapX (M, Xof(A), Yof(A)),	\
				Yof(B) = MapY (M, Xof(A), Yof(A)))
#define MapDx(M,DX,DY)    ((M)->a * (DX) + (M)->c * (DY))
#define MapDy(M,DX,DY)    ((M)->b * (DX) + (M)->d * (DY))

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void RotateMatrix(MATRIX_2D_PTR Matrix, FLOAT32 Angle); 
#endif
