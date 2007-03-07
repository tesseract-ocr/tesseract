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
#include "mergenf.h"
#include "general.h"
#include "efio.h"
#include "clusttool.h"
#include "cluster.h"
#include "oldlist.h"
#include "protos.h"
#include "minmax.h"
#include "ocrfeatures.h"
#include "debug.h"
#include "const.h"
#include "featdefs.h"
#include "intproto.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


/**----------------------------------------------------------------------------
					Variables
-----------------------------------------------------------------------------**/
/*-------------------once in subfeat---------------------------------*/
make_float_var (AngleMatchScale, 1.0, MakeAngleMatchScale,
                7, 2, SetAngleMatchScale,  "Angle Match Scale ...")

make_float_var (SimilarityMidpoint, 0.0075, MakeSimilarityMidpoint,
                7, 3, SetSimilarityMidpoint,  "Similarity Midpoint ...")

make_float_var (SimilarityCurl, 2.0, MakeSimilarityCurl,
                7, 4, SetSimilarityCurl,  "Similarity Curl ...")

/*-----------------------------once in fasttrain----------------------------------*/
make_float_var (TangentBBoxPad, 0.5, MakeTangentBBoxPad,
                15, 3, SetTangentBBoxPad,  "Tangent bounding box pad ...")

make_float_var (OrthogonalBBoxPad, 2.5, MakeOrthogonalBBoxPad,
                15, 4, SetOrthogonalBBoxPad,  "Orthogonal bounding box pad ...")

make_float_var (AnglePad, 45.0, MakeAnglePad,
                15, 5, SetAnglePad,  "Angle pad ...")

/**----------------------------------------------------------------------------
		  		Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
//int row_number;			/* kludge due to linking problems */

/**----------------------------------------------------------------------------
							Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FLOAT32 CompareProtos (
     PROTO	p1,
	 PROTO	p2)

/*
**	Parameters:
**		p1, p2		protos to be compared
**	Globals: none
**	Operation: Compare protos p1 and p2 and return an estimate of the
**		worst evidence rating that will result for any part of p1
**		that is compared to p2.  In other words, if p1 were broken
**		into pico-features and each pico-feature was matched to p2,
**		what is the worst evidence rating that will be achieved for
**		any pico-feature.
**	Return: Worst possible result when matching p1 to p2.
**	Exceptions: none
**	History: Mon Nov 26 08:27:53 1990, DSJ, Created.
*/

{
	FEATURE	Feature;
	FLOAT32	WorstEvidence = WORST_EVIDENCE;
	FLOAT32	Evidence;
	FLOAT32	Angle, Length;

	/* if p1 and p2 are not close in length, don't let them match */
	Length = fabs (ProtoLength (p1) - ProtoLength (p2));
	if (Length > MAX_LENGTH_MISMATCH)
		return (0.0);

	/* create a dummy pico-feature to be used for comparisons */
	Feature = NewFeature (&PicoFeatDesc);
	ParamOf (Feature, PicoFeatDir) = ProtoAngle (p1);

	/* convert angle to radians */
	Angle = ProtoAngle (p1) * 2.0 * PI;

	/* find distance from center of p1 to 1/2 picofeat from end */
	Length = ProtoLength (p1) / 2.0 - GetPicoFeatureLength () / 2.0;
	if (Length < 0) Length = 0;

	/* set the dummy pico-feature at one end of p1 and match it to p2 */
	ParamOf (Feature, PicoFeatX) = ProtoX (p1) + cos (Angle) * Length;
	ParamOf (Feature, PicoFeatY) = ProtoY (p1) + sin (Angle) * Length;
	if (DummyFastMatch (Feature, p2))
    {
		Evidence = SubfeatureEvidence (Feature, p2);
		if (Evidence < WorstEvidence)
			WorstEvidence = Evidence;
    }
	else
    {
		FreeFeature (Feature);
		return (0.0);
    }

	/* set the dummy pico-feature at the other end of p1 and match it to p2 */
	ParamOf (Feature, PicoFeatX) = ProtoX (p1) - cos (Angle) * Length;
	ParamOf (Feature, PicoFeatY) = ProtoY (p1) - sin (Angle) * Length;
	if (DummyFastMatch (Feature, p2))
    {
		Evidence = SubfeatureEvidence (Feature, p2);
		if (Evidence < WorstEvidence)
			WorstEvidence = Evidence;
    }
	else
    {
		FreeFeature (Feature);
		return (0.0);
    }

	FreeFeature (Feature);
	return (WorstEvidence);

}	/* CompareProtos */

/*---------------------------------------------------------------------------*/
void ComputeMergedProto (
     PROTO	p1,
	 PROTO	p2,
     FLOAT32	w1,
	 FLOAT32	w2,
     PROTO	MergedProto)

/*
**	Parameters:
**		p1, p2		protos to be merged
**		w1, w2		weight of each proto
**		MergedProto	place to put resulting merged proto
**	Globals: none
**	Operation: This routine computes a proto which is the weighted
**		average of protos p1 and p2.  The new proto is returned
**		in MergedProto.
**	Return: none (results are returned in MergedProto)
**	Exceptions: none
**	History: Mon Nov 26 08:15:08 1990, DSJ, Created.
*/

{
	FLOAT32	TotalWeight;

	TotalWeight = w1 + w2;
	w1 /= TotalWeight;
	w2 /= TotalWeight;

	ProtoX      (MergedProto) = ProtoX      (p1) * w1 + ProtoX      (p2) * w2;
	ProtoY      (MergedProto) = ProtoY      (p1) * w1 + ProtoY      (p2) * w2;
	ProtoLength (MergedProto) = ProtoLength (p1) * w1 + ProtoLength (p2) * w2;
	ProtoAngle  (MergedProto) = ProtoAngle  (p1) * w1 + ProtoAngle  (p2) * w2;
	FillABC     (MergedProto);

}	/* ComputeMergedProto */

/*---------------------------------------------------------------------------*/
int FindClosestExistingProto (
     CLASS_TYPE	Class,
     int       	NumMerged[],
     PROTOTYPE	*Prototype)

/*
**	Parameters:
**		Class		class to search for matching old proto in
**		NumMerged[]	# of protos merged into each proto of Class
**		Prototype	new proto to find match for
**	Globals: none
**	Operation: This routine searches thru all of the prototypes in
**		Class and returns the id of the proto which would provide
**		the best approximation of Prototype.  If no close
**		approximation can be found, NO_PROTO is returned.
**	Return: Id of closest proto in Class or NO_PROTO.
**	Exceptions: none
**	History: Sat Nov 24 11:42:58 1990, DSJ, Created.
*/

{
	PROTO_STRUCT	NewProto;
	PROTO_STRUCT	MergedProto;
	int		Pid;
	PROTO		Proto;
	int		BestProto;
	FLOAT32	BestMatch;
	FLOAT32	Match, OldMatch, NewMatch;

	MakeNewFromOld (&NewProto, Prototype);

	BestProto = NO_PROTO;
	BestMatch = WORST_MATCH_ALLOWED;
	for (Pid = 0; Pid < NumProtosIn (Class); Pid++)
    {
		Proto  = ProtoIn (Class, Pid);
		ComputeMergedProto (Proto, &NewProto,
			(FLOAT32) NumMerged[Pid], 1.0, &MergedProto);
		OldMatch = CompareProtos (Proto, &MergedProto);
		NewMatch = CompareProtos (&NewProto, &MergedProto);
		Match = MIN (OldMatch, NewMatch);
		if (Match > BestMatch)
		{
			BestProto = Pid;
			BestMatch = Match;
		}
    }
	return (BestProto);

}	/* FindClosestExistingProto */

/*---------------------------------------------------------------------------*/
void MakeNewFromOld (
     PROTO	New,
     PROTOTYPE	*Old)

/*
**	Parameters:
**		New	new proto to be filled in
**		Old	old proto to be converted
**	Globals: none
**	Operation: This fills in the fields of the New proto based on the
**		fields of the Old proto.
**	Return: none
**	Exceptions: none
**	History: Mon Nov 26 09:45:39 1990, DSJ, Created.
*/

{
	ProtoX      (New) = CenterX       (Old->Mean);
	ProtoY      (New) = CenterY       (Old->Mean);
	ProtoLength (New) = LengthOf      (Old->Mean);
	ProtoAngle  (New) = OrientationOf (Old->Mean);
	FillABC     (New);

}	/* MakeNewFromOld */

/*-------------------once in subfeat---------------------------------*/
/**********************************************************************
* InitSubfeatureVars
*
* Create and set up all menus and variables needed for this file.
**********************************************************************/
void InitSubfeatureVars ()
{
	MakeAngleMatchScale     ();
	MakeSimilarityCurl      ();
	MakeSimilarityMidpoint  ();
}


/**********************************************************************
* SubfeatureEvidence
*
* Compare a feature to a prototype. Print the result.
**********************************************************************/
FLOAT32 SubfeatureEvidence (
   FEATURE     Feature,
   PROTO       Proto)
{
	float       Distance;
	float       Dangle;

	Dangle   = ProtoAngle (Proto) - ParamOf(Feature, PicoFeatDir);
	if (Dangle < -0.5) Dangle += 1.0;
	if (Dangle >  0.5) Dangle -= 1.0;
	Dangle   *= AngleMatchScale;

	Distance = CoefficientA (Proto) * ParamOf(Feature, PicoFeatX) +
		CoefficientB (Proto) * ParamOf(Feature, PicoFeatY) +
		CoefficientC (Proto);

	return (EvidenceOf (Distance * Distance + Dangle * Dangle));
}

/**********************************************************************
* EvidenceOf
*
* Return the new type of evidence number corresponding to this
* distance value.  This number is no longer based on the chi squared
* approximation.  The equation that represents the transform is:
*       1 / (1 + (sim / midpoint) ^ curl)
**********************************************************************/
FLOAT32 EvidenceOf (
  register FLOAT32   Similarity)
{

	Similarity /= SimilarityMidpoint;

	if (SimilarityCurl == 3)
		Similarity = Similarity * Similarity * Similarity;
	else if (SimilarityCurl == 2)
		Similarity = Similarity * Similarity;
	else
		Similarity = pow (Similarity, SimilarityCurl);

	return (1.0 / (1.0 + Similarity));
}

/*-----------------------------once in fasttrain----------------------------------*/
void InitFastTrainerVars ()
/*
**	Parameters: none
**	Globals: none
**	Operation: This routine initializes all of the control variables
**		for the fast trainer.
**	Return: none
**	Exceptions: none
**	History: Mon Nov 12 13:27:35 1990, DSJ, Created.
*/

{
	MakeTangentBBoxPad ();
	MakeOrthogonalBBoxPad ();
	MakeAnglePad ();

}	/* InitFastTrainerVars */

/*---------------------------------------------------------------------------*/
BOOL8 DummyFastMatch (
     FEATURE	Feature,
     PROTO	Proto)

/*
**	Parameters:
**		Feature		feature to be "fast matched" to proto
**		Proto		proto being "fast matched" against
**	Globals:
**		TangentBBoxPad		bounding box pad tangent to proto
**		OrthogonalBBoxPad	bounding box pad orthogonal to proto
**	Operation: This routine returns TRUE if Feature would be matched
**		by a fast match table built from Proto.
**	Return: TRUE if feature could match Proto.
**	Exceptions: none
**	History: Wed Nov 14 17:19:58 1990, DSJ, Created.
*/

{
	FRECT		BoundingBox;
	FLOAT32	MaxAngleError;
	FLOAT32	AngleError;

	MaxAngleError = AnglePad / 360.0;
	AngleError = fabs (ProtoAngle (Proto) - ParamOf (Feature, PicoFeatDir));
	if (AngleError > 0.5)
		AngleError = 1.0 - AngleError;

	if (AngleError > MaxAngleError)
		return (FALSE);

	ComputePaddedBoundingBox (Proto,
		TangentBBoxPad * GetPicoFeatureLength (),
		OrthogonalBBoxPad * GetPicoFeatureLength (),
		&BoundingBox);

	return (PointInside (&BoundingBox,
		ParamOf (Feature, PicoFeatX),
		ParamOf (Feature, PicoFeatY)));

}	/* DummyFastMatch */

/*----------------------------------------------------------------------------*/
void ComputePaddedBoundingBox (
     PROTO	Proto,
     FLOAT32	TangentPad,
	 FLOAT32	OrthogonalPad,
     FRECT	*BoundingBox)

/*
**	Parameters:
**		Proto		proto to compute bounding box for
**		TangentPad	amount of pad to add in direction of segment
**		OrthogonalPad	amount of pad to add orthogonal to segment
**		BoundingBox	place to put results
**	Globals: none
**	Operation: This routine computes a bounding box that encloses the
**		specified proto along with some padding.  The
**		amount of padding is specified as separate distances
**		in the tangential and orthogonal directions.
**	Return: none (results are returned in BoundingBox)
**	Exceptions: none
**	History: Wed Nov 14 14:55:30 1990, DSJ, Created.
*/

{
	FLOAT32	Pad, Length, Angle;
	FLOAT32	CosOfAngle, SinOfAngle;

	Length     = ProtoLength (Proto) / 2.0 + TangentPad;
	Angle      = ProtoAngle (Proto) * 2.0 * PI;
	CosOfAngle = fabs (cos (Angle));
	SinOfAngle = fabs (sin (Angle));

	Pad = MAX (CosOfAngle * Length, SinOfAngle * OrthogonalPad);
	BoundingBox->MinX = ProtoX (Proto) - Pad;
	BoundingBox->MaxX = ProtoX (Proto) + Pad;

	Pad = MAX (SinOfAngle * Length, CosOfAngle * OrthogonalPad);
	BoundingBox->MinY = ProtoY (Proto) - Pad;
	BoundingBox->MaxY = ProtoY (Proto) + Pad;

}	/* ComputePaddedBoundingBox */

/*--------------------------------------------------------------------------*/
BOOL8 PointInside (
     FRECT	*Rectangle,
     FLOAT32	X,
	 FLOAT32	Y)

/*
**	Parameters:
**	Globals: none
**	Operation: Return TRUE if point (X,Y) is inside of Rectangle.
**	Return:  Return TRUE if point (X,Y) is inside of Rectangle.
**	Exceptions: none
**	History: Wed Nov 14 17:26:35 1990, DSJ, Created.
*/

{
	if (X < Rectangle->MinX) return (FALSE);
	if (X > Rectangle->MaxX) return (FALSE);
	if (Y < Rectangle->MinY) return (FALSE);
	if (Y > Rectangle->MaxY) return (FALSE);
	return (TRUE);

}	/* PointInside */
