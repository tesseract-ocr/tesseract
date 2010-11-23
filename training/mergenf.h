/******************************************************************************
**	Filename:    MergeNF.c
**	Purpose:     Program for merging similar nano-feature protos
**	Author:      Dan Johnson
**	History:     Wed Nov 21 09:55:23 1990, DSJ, Created.
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
#include "protos.h"
#include "cluster.h"
#include "ocrfeatures.h"
#include "callcpp.h"
#include "picofeat.h"


#define WORST_MATCH_ALLOWED	(0.9)
#define WORST_EVIDENCE (1.0)
#define MAX_LENGTH_MISMATCH	(2.0 * GetPicoFeatureLength ())


#define PROTO_SUFFIX		".mf.p"
#define CONFIG_SUFFIX		".cl"
#define NO_PROTO	(-1)
#define XPOSITION			0
#define YPOSITION			1
#define MFLENGTH			2
#define ORIENTATION			3

typedef struct
{
  FLOAT32	MinX, MaxX, MinY, MaxY;
} FRECT;

/**----------------------------------------------------------------------------
					Public Macros
----------------------------------------------------------------------------**/
#define CenterX(M)		( (M)[XPOSITION] )
#define CenterY(M)		( (M)[YPOSITION] )
#define LengthOf(M)		( (M)[MFLENGTH] )
#define OrientationOf(M)	( (M)[ORIENTATION] )

/**----------------------------------------------------------------------------
					Public Function Prototypes
----------------------------------------------------------------------------**/
FLOAT32 CompareProtos (
     PROTO	p1,
	 PROTO	p2);

void ComputeMergedProto (
     PROTO	p1,
	 PROTO	p2,
     FLOAT32	w1,
	 FLOAT32	w2,
     PROTO	MergedProto);

int FindClosestExistingProto (
     CLASS_TYPE	Class,
     int       	NumMerged[],
     PROTOTYPE	*Prototype);

void MakeNewFromOld (
     PROTO	New,
     PROTOTYPE	*Old);

FLOAT32 SubfeatureEvidence (
   FEATURE     Feature,
   PROTO       Proto);

double EvidenceOf (
  register double   Similarity);

BOOL8 DummyFastMatch (
     FEATURE	Feature,
     PROTO	Proto);

void ComputePaddedBoundingBox (
     PROTO	Proto,
     FLOAT32	TangentPad,
	 FLOAT32	OrthogonalPad,
     FRECT	*BoundingBox);

BOOL8 PointInside (
     FRECT	*Rectangle,
     FLOAT32	X,
	 FLOAT32	Y);
