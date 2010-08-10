/******************************************************************************
**  Filename:    MergeNF.c
**  Purpose:     Program for merging similar nano-feature protos
**  Author:      Dan Johnson
**  History:     Wed Nov 21 09:55:23 1990, DSJ, Created.
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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "mergenf.h"
#include "general.h"
#include "efio.h"
#include "clusttool.h"
#include "cluster.h"
#include "oldlist.h"
#include "protos.h"
#include "ndminx.h"
#include "ocrfeatures.h"
#include "const.h"
#include "featdefs.h"
#include "intproto.h"
#include "varable.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


/*----------------------------------------------------------------------------
          Variables
-----------------------------------------------------------------------------*/
/*-------------------once in subfeat---------------------------------*/
double_VAR(training_angle_match_scale, 1.0, "Angle Match Scale ...");

double_VAR(training_similarity_midpoint, 0.0075, "Similarity Midpoint ...");

double_VAR(training_similarity_curl, 2.0, "Similarity Curl ...");

/*-----------------------------once in fasttrain----------------------------------*/
double_VAR(training_tangent_bbox_pad, 0.5, "Tangent bounding box pad ...");

double_VAR(training_orthogonal_bbox_pad, 2.5, "Orthogonal bounding box pad ...");

double_VAR(training_angle_pad, 45.0, "Angle pad ...");

/*----------------------------------------------------------------------------
          Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
//int row_number;     /* kludge due to linking problems */

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * Compare protos p1 and p2 and return an estimate of the
 * worst evidence rating that will result for any part of p1
 * that is compared to p2.  In other words, if p1 were broken
 * into pico-features and each pico-feature was matched to p2,
 * what is the worst evidence rating that will be achieved for
 * any pico-feature.
 *
 * @param p1, p2    protos to be compared
 *
 * Globals: none
 *
 * @return Worst possible result when matching p1 to p2.
 * @note Exceptions: none
 * @note History: Mon Nov 26 08:27:53 1990, DSJ, Created.
 */
FLOAT32 CompareProtos(PROTO p1, PROTO p2) {
  FEATURE Feature;
  FLOAT32 WorstEvidence = WORST_EVIDENCE;
  FLOAT32 Evidence;
  FLOAT32 Angle, Length;

  /* if p1 and p2 are not close in length, don't let them match */
  Length = fabs (p1->Length - p2->Length);
  if (Length > MAX_LENGTH_MISMATCH)
    return (0.0);

  /* create a dummy pico-feature to be used for comparisons */
  Feature = NewFeature (&PicoFeatDesc);
  Feature->Params[PicoFeatDir] = p1->Angle;

  /* convert angle to radians */
  Angle = p1->Angle * 2.0 * PI;

  /* find distance from center of p1 to 1/2 picofeat from end */
  Length = p1->Length / 2.0 - GetPicoFeatureLength () / 2.0;
  if (Length < 0) Length = 0;

  /* set the dummy pico-feature at one end of p1 and match it to p2 */
  Feature->Params[PicoFeatX] = p1->X + cos (Angle) * Length;
  Feature->Params[PicoFeatY] = p1->Y + sin (Angle) * Length;
  if (DummyFastMatch (Feature, p2)) {
    Evidence = SubfeatureEvidence (Feature, p2);
    if (Evidence < WorstEvidence)
      WorstEvidence = Evidence;
  } else {
    FreeFeature(Feature);
    return 0.0;
  }

  /* set the dummy pico-feature at the other end of p1 and match it to p2 */
  Feature->Params[PicoFeatX] = p1->X - cos (Angle) * Length;
  Feature->Params[PicoFeatY] = p1->Y - sin (Angle) * Length;
  if (DummyFastMatch (Feature, p2)) {
    Evidence = SubfeatureEvidence (Feature, p2);
    if (Evidence < WorstEvidence)
      WorstEvidence = Evidence;
  } else {
    FreeFeature(Feature);
    return 0.0;
  }

  FreeFeature (Feature);
  return (WorstEvidence);

} /* CompareProtos */

/*---------------------------------------------------------------------------*/
/**
 * This routine computes a proto which is the weighted
 * average of protos p1 and p2.  The new proto is returned
 * in MergedProto.
 *
 * @param p1, p2    protos to be merged
 * @param w1, w2    weight of each proto
 * @param MergedProto place to put resulting merged proto
 *
 * Globals: none
 *
 * @return none (results are returned in MergedProto)
 * @note Exceptions: none
 * @note History: Mon Nov 26 08:15:08 1990, DSJ, Created.
 */
void ComputeMergedProto (PROTO  p1,
                         PROTO  p2,
                         FLOAT32  w1,
                         FLOAT32  w2,
                         PROTO  MergedProto)
{
  FLOAT32 TotalWeight;

  TotalWeight = w1 + w2;
  w1 /= TotalWeight;
  w2 /= TotalWeight;

  MergedProto->X = p1->X * w1 + p2->X * w2;
  MergedProto->Y = p1->Y * w1 + p2->Y * w2;
  MergedProto->Length = p1->Length * w1 + p2->Length * w2;
  MergedProto->Angle = p1->Angle * w1 + p2->Angle * w2;
  FillABC(MergedProto);
} /* ComputeMergedProto */

/*---------------------------------------------------------------------------*/
/**
 * This routine searches thru all of the prototypes in
 * Class and returns the id of the proto which would provide
 * the best approximation of Prototype.  If no close
 * approximation can be found, NO_PROTO is returned.
 *
 * @param Class   class to search for matching old proto in
 * @param NumMerged # of protos merged into each proto of Class
 * @param  Prototype new proto to find match for
 *
 * Globals: none
 *
 * @return Id of closest proto in Class or NO_PROTO.
 * @note Exceptions: none
 * @note History: Sat Nov 24 11:42:58 1990, DSJ, Created.
 */
int FindClosestExistingProto(CLASS_TYPE Class, int NumMerged[],
                             PROTOTYPE  *Prototype) {
  PROTO_STRUCT  NewProto;
  PROTO_STRUCT  MergedProto;
  int   Pid;
  PROTO   Proto;
  int   BestProto;
  FLOAT32 BestMatch;
  FLOAT32 Match, OldMatch, NewMatch;

  MakeNewFromOld (&NewProto, Prototype);

  BestProto = NO_PROTO;
  BestMatch = WORST_MATCH_ALLOWED;
  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    Proto  = ProtoIn(Class, Pid);
    ComputeMergedProto(Proto, &NewProto,
      (FLOAT32) NumMerged[Pid], 1.0, &MergedProto);
    OldMatch = CompareProtos(Proto, &MergedProto);
    NewMatch = CompareProtos(&NewProto, &MergedProto);
    Match = MIN(OldMatch, NewMatch);
    if (Match > BestMatch) {
      BestProto = Pid;
      BestMatch = Match;
    }
  }
  return BestProto;
} /* FindClosestExistingProto */

/*---------------------------------------------------------------------------*/
/**
 * This fills in the fields of the New proto based on the
 * fields of the Old proto.
 *
 * @param New new proto to be filled in
 * @param Old old proto to be converted
 * 
 *  Globals: none
 * 
 * Exceptions: none
 * History: Mon Nov 26 09:45:39 1990, DSJ, Created.
 */
void MakeNewFromOld(PROTO New, PROTOTYPE *Old) {
  New->X = CenterX(Old->Mean);
  New->Y = CenterY(Old->Mean);
  New->Length = LengthOf(Old->Mean);
  New->Angle = OrientationOf(Old->Mean);
  FillABC(New);
} /* MakeNewFromOld */

/*-------------------once in subfeat---------------------------------*/

/**
 * @name SubfeatureEvidence
 *
 * Compare a feature to a prototype. Print the result.
 */
FLOAT32 SubfeatureEvidence(FEATURE Feature, PROTO Proto) {
  float       Distance;
  float       Dangle;

  Dangle   = Proto->Angle - Feature->Params[PicoFeatDir];
  if (Dangle < -0.5) Dangle += 1.0;
  if (Dangle >  0.5) Dangle -= 1.0;
  Dangle *= training_angle_match_scale;

  Distance = Proto->A * Feature->Params[PicoFeatX] +
    Proto->B * Feature->Params[PicoFeatY] +
    Proto->C;

  return (EvidenceOf (Distance * Distance + Dangle * Dangle));
}

/**
 * @name EvidenceOf
 *
 * Return the new type of evidence number corresponding to this
 * distance value.  This number is no longer based on the chi squared
 * approximation.  The equation that represents the transform is:
 *       1 / (1 + (sim / midpoint) ^ curl)
 */
FLOAT32 EvidenceOf (
  register FLOAT32   Similarity)
{

  Similarity /= training_similarity_midpoint;

  if (training_similarity_curl == 3)
    Similarity = Similarity * Similarity * Similarity;
  else if (training_similarity_curl == 2)
    Similarity = Similarity * Similarity;
  else
    Similarity = static_cast<float>(pow(static_cast<double>(Similarity),
	                                    training_similarity_curl));

  return (1.0 / (1.0 + Similarity));
}

/*---------------------------------------------------------------------------*/
/**
 * This routine returns TRUE if Feature would be matched
 * by a fast match table built from Proto.
 *
 * @param Feature   feature to be "fast matched" to proto
 * @param Proto   proto being "fast matched" against
 *
 * Globals:
 * - training_tangent_bbox_pad    bounding box pad tangent to proto
 * - training_orthogonal_bbox_pad bounding box pad orthogonal to proto
 *
 * @return TRUE if feature could match Proto.
 * @note Exceptions: none
 * @note History: Wed Nov 14 17:19:58 1990, DSJ, Created.
 */
BOOL8 DummyFastMatch (
     FEATURE  Feature,
     PROTO  Proto)
{
  FRECT   BoundingBox;
  FLOAT32 MaxAngleError;
  FLOAT32 AngleError;

  MaxAngleError = training_angle_pad / 360.0;
  AngleError = fabs (Proto->Angle - Feature->Params[PicoFeatDir]);
  if (AngleError > 0.5)
    AngleError = 1.0 - AngleError;

  if (AngleError > MaxAngleError)
    return (FALSE);

  ComputePaddedBoundingBox (Proto,
    training_tangent_bbox_pad * GetPicoFeatureLength (),
    training_orthogonal_bbox_pad * GetPicoFeatureLength (),
    &BoundingBox);

  return PointInside(&BoundingBox, Feature->Params[PicoFeatX],
                     Feature->Params[PicoFeatY]);
} /* DummyFastMatch */

/*----------------------------------------------------------------------------*/
/**
 * This routine computes a bounding box that encloses the
 * specified proto along with some padding.  The
 * amount of padding is specified as separate distances
 * in the tangential and orthogonal directions.
 *
 * @param Proto   proto to compute bounding box for
 * @param TangentPad  amount of pad to add in direction of segment
 * @param OrthogonalPad amount of pad to add orthogonal to segment
 * @param[out] BoundingBox place to put results
 *
 * Globals: none
 *
 * @return none (results are returned in BoundingBox)
 * @note Exceptions: none
 * @note History: Wed Nov 14 14:55:30 1990, DSJ, Created.
 */
void ComputePaddedBoundingBox (PROTO  Proto, FLOAT32  TangentPad,
                               FLOAT32  OrthogonalPad, FRECT  *BoundingBox) {
  FLOAT32 Pad, Length, Angle;
  FLOAT32 CosOfAngle, SinOfAngle;

  Length     = Proto->Length / 2.0 + TangentPad;
  Angle      = Proto->Angle * 2.0 * PI;
  CosOfAngle = fabs(cos(Angle));
  SinOfAngle = fabs(sin(Angle));

  Pad = MAX (CosOfAngle * Length, SinOfAngle * OrthogonalPad);
  BoundingBox->MinX = Proto->X - Pad;
  BoundingBox->MaxX = Proto->X + Pad;

  Pad = MAX(SinOfAngle * Length, CosOfAngle * OrthogonalPad);
  BoundingBox->MinY = Proto->Y - Pad;
  BoundingBox->MaxY = Proto->Y + Pad;

} /* ComputePaddedBoundingBox */

/*--------------------------------------------------------------------------*/
/**
 * Return TRUE if point (X,Y) is inside of Rectangle.
 *
 * Globals: none
 *
 * @return TRUE if point (X,Y) is inside of Rectangle.
 * @note Exceptions: none
 * @note History: Wed Nov 14 17:26:35 1990, DSJ, Created.
 */
BOOL8 PointInside(FRECT *Rectangle, FLOAT32 X, FLOAT32  Y) {
  if (X < Rectangle->MinX) return (FALSE);
  if (X > Rectangle->MaxX) return (FALSE);
  if (Y < Rectangle->MinY) return (FALSE);
  if (Y > Rectangle->MaxY) return (FALSE);
  return (TRUE);

} /* PointInside */
