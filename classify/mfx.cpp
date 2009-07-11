/******************************************************************************
 **      Filename:       mfx.c
 **      Purpose:        Micro feature extraction routines
 **      Author:         Dan Johnson
 **      History:        7/21/89, DSJ, Created.
 **
 **      (c) Copyright Hewlett-Packard Company, 1988.
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
#include "mfdefs.h"
#include "mfoutline.h"
#include "clusttool.h"           //NEEDED
#include "const.h"
#include "intfx.h"
#include "varable.h"

#include <math.h>

/**----------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------**/

/* old numbers corresponded to 10.0 degrees and 80.0 degrees */
double_VAR(classify_min_slope, 0.414213562,
           "Slope below which lines are called horizontal");
double_VAR(classify_max_slope, 2.414213562,
           "Slope above which lines are called vertical");
double_VAR(classify_noise_segment_length, 0.00,
           "Length below which outline segments are treated as noise");

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
/* miscellaneous macros */
#define NormalizeAngle(A)       ( (((A)<0)?((A)+2*PI):(A)) / (2*PI) )

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void ComputeBulges(MFOUTLINE Start, MFOUTLINE End, MICROFEATURE MicroFeature);

FLOAT32 ComputeOrientation(MFEDGEPT *Start, MFEDGEPT *End);

MICROFEATURES ConvertToMicroFeatures(MFOUTLINE Outline,
                                     MICROFEATURES MicroFeatures);

MICROFEATURE ExtractMicroFeature(MFOUTLINE Start, MFOUTLINE End);

void SmearBulges(MICROFEATURES MicroFeatures, FLOAT32 XScale, FLOAT32 YScale);

/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/

/*---------------------------------------------------------------------------*/
CHAR_FEATURES BlobMicroFeatures(TBLOB *Blob, LINE_STATS *LineStats) {
/*
 **      Parameters:
 **              Blob            blob to extract micro-features from
 **              LineStats       statistics for text line normalization
 **      Operation:
 **              This routine extracts micro-features from the specified
 **              blob and returns a list of the micro-features.  All
 **              micro-features are normalized according to the specified
 **              line statistics.
 **      Return: List of micro-features extracted from the blob.
 **      Exceptions: none
 **      History: 7/21/89, DSJ, Created.
 */
  MICROFEATURES MicroFeatures = NIL;
  FLOAT32 XScale, YScale;
  LIST Outlines;
  LIST RemainingOutlines;
  MFOUTLINE Outline;
  INT_FEATURE_ARRAY blfeatures;
  INT_FEATURE_ARRAY cnfeatures;
  INT_FX_RESULT_STRUCT results;

  if (Blob != NULL) {
    Outlines = ConvertBlob (Blob);
//    NormalizeOutlines(Outlines, LineStats, &XScale, &YScale);
    if (!ExtractIntFeat(Blob, blfeatures, cnfeatures, &results))
      return NULL;
    XScale = 0.2f / results.Ry;
    YScale = 0.2f / results.Rx;

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = (MFOUTLINE) first_node (RemainingOutlines);
      CharNormalizeOutline (Outline,
        results.Xmean, results.Ymean,
        XScale, YScale);
    }

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = (MFOUTLINE) first_node (RemainingOutlines);
      FindDirectionChanges(Outline, classify_min_slope, classify_max_slope);
      FilterEdgeNoise(Outline, classify_noise_segment_length);
      MarkDirectionChanges(Outline);
      SmearExtremities(Outline, XScale, YScale);
      MicroFeatures = ConvertToMicroFeatures (Outline, MicroFeatures);
    }
    SmearBulges(MicroFeatures, XScale, YScale);
    FreeOutlines(Outlines);
  }
  return ((CHAR_FEATURES) MicroFeatures);
}                                /* BlobMicroFeatures */


/**----------------------------------------------------------------------------
              Private Macros
----------------------------------------------------------------------------**/
/**********************************************************************
 * angle_of
 *
 * Return the angle of the line between two points.
 **********************************************************************/
#define angle_of(x1,y1,x2,y2)                   \
((x2-x1) ?                                    \
  (atan2 (y2-y1, x2-x1)) :                     \
  ((y2<y1) ? (- PI / 2.0) : (PI / 2.0)))   \


/**********************************************************************
 * scale_angle
 *
 * Make sure that the angle is non-negative.  Scale it to the right
 * amount.
 **********************************************************************/

#define scale_angle(x)                             \
(((x<0) ? (2.0 * PI + x) : (x)) * 0.5 / PI)  \

/*---------------------------------------------------------------------------
            Private Code
---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ComputeBulges(MFOUTLINE Start, MFOUTLINE End, MICROFEATURE MicroFeature) {
/*
 **      Parameters:
 **              Start           starting point of micro-feature
 **              End             ending point of micro-feature
 **              MicroFeature    micro-feature whose bulges are to be computed
 **      Globals: none
 **      Operation:
 **              This routine computes the size of the "bulges" of the
 **              specified micro-feature.  The bulges are the deviations
 **              of the micro-features from a straight line at the 1/3
 **              and 2/3 points along the straight line approximation of
 **              the micro-feature.  The size of each bulge is normalized
 **              to the range -0.5 to 0.5.  A positive bulge indicates a
 **              deviation in the counterclockwise direction and vice versa.
 **              A size of 0.5 (+ or -) corresponds to the largest bulge that
 **              could ever occur for the given feature independent of
 **              orientation.  This routine assumes that Start
 **              and End are not the same point.  It also assumes that the
 **              orientation and length parameters of the micro-feature
 **              have already been computed.
 **      Return: none
 **      Exceptions: none
 **      History: 7/27/89, DSJ, Created.
 */
  MATRIX_2D Matrix;
  MFEDGEPT *Origin;
  MFOUTLINE SegmentStart, SegmentEnd;
  FPOINT CurrentPoint, LastPoint;
  FLOAT32 BulgePosition;

  /* check for simple case */
  if (End == NextPointAfter (Start))
    MicroFeature[FIRSTBULGE] = MicroFeature[SECONDBULGE] = 0;
  else {
    Origin = PointAt (Start);

    InitMatrix(&Matrix);
    RotateMatrix (&Matrix, MicroFeature[ORIENTATION] * -2.0 * PI);
    TranslateMatrix (&Matrix, -Origin->Point.x, -Origin->Point.y);

    SegmentEnd = Start;
    CurrentPoint.x = 0.0f;
    CurrentPoint.y =  0.0f;
    BulgePosition = MicroFeature[MFLENGTH] / 3;
    LastPoint = CurrentPoint;
    while (CurrentPoint.x < BulgePosition) {
      SegmentStart = SegmentEnd;
      SegmentEnd = NextPointAfter (SegmentStart);
      LastPoint = CurrentPoint;

      MapPoint(&Matrix, PointAt(SegmentEnd)->Point, &CurrentPoint);
    }
    MicroFeature[FIRSTBULGE] =
      XIntersectionOf(LastPoint, CurrentPoint, BulgePosition);

    BulgePosition *= 2;

    // Prevents from copying the points before computing the bulge if
    // CurrentPoint will not change. (Which would cause to output nan
    // for the SecondBulge.)
    if (CurrentPoint.x < BulgePosition)
      LastPoint = CurrentPoint;
    while (CurrentPoint.x < BulgePosition) {
      SegmentStart = SegmentEnd;
      SegmentEnd = NextPointAfter (SegmentStart);
      LastPoint = CurrentPoint;
      MapPoint(&Matrix, PointAt(SegmentEnd)->Point, &CurrentPoint);
    }
    MicroFeature[SECONDBULGE] =
      XIntersectionOf(LastPoint, CurrentPoint, BulgePosition);

    MicroFeature[FIRSTBULGE] /= BULGENORMALIZER * MicroFeature[MFLENGTH];
    MicroFeature[SECONDBULGE] /= BULGENORMALIZER * MicroFeature[MFLENGTH];
  }
}                                /* ComputeBulges */


/*---------------------------------------------------------------------------*/
FLOAT32 ComputeOrientation(MFEDGEPT *Start, MFEDGEPT *End) {
/*
 **      Parameters:
 **              Start           starting edge point of micro-feature
 **              End             ending edge point of micro-feature
 **      Globals: none
 **      Operation:
 **              This routine computes the orientation parameter of the
 **              specified micro-feature.  The orientation is the angle of
 **              the vector from Start to End.  It is normalized to a number
 **              between 0 and 1 where 0 corresponds to 0 degrees and 1
 **              corresponds to 360 degrees.  The actual range is [0,1), i.e.
 **              1 is excluded from the range (since it is actual the
 **              same orientation as 0).  This routine assumes that Start
 **              and End are not the same point.
 **      Return: Orientation parameter for the specified micro-feature.
 **      Exceptions: none
 **      History: 7/27/89, DSJ, Created.
 */
  FLOAT32 Orientation;

  Orientation = NormalizeAngle (AngleFrom (Start->Point, End->Point));

  /* ensure that round-off errors do not put circular param out of range */
  if ((Orientation < 0) || (Orientation >= 1))
    Orientation = 0;
  return (Orientation);
}                                /* ComputeOrientation */


/*---------------------------------------------------------------------------*/
MICROFEATURES ConvertToMicroFeatures(MFOUTLINE Outline,
                                     MICROFEATURES MicroFeatures) {
/*
 **      Parameters:
 **              Outline         outline to extract micro-features from
 **              MicroFeatures   list of micro-features to add to
 **      Globals: none
 **      Operation:
 **              This routine
 **      Return: List of micro-features with new features added to front.
 **      Exceptions: none
 **      History: 7/26/89, DSJ, Created.
 */
  MFOUTLINE Current;
  MFOUTLINE Last;
  MFOUTLINE First;
  MICROFEATURE NewFeature;

  if (DegenerateOutline (Outline))
    return (MicroFeatures);

  First = NextExtremity (Outline);
  Last = First;
  do {
    Current = NextExtremity (Last);
    NewFeature = ExtractMicroFeature (Last, Current);
    if (NewFeature != NULL)
      MicroFeatures = push (MicroFeatures, NewFeature);
    Last = Current;
  }
  while (Last != First);

  return (MicroFeatures);
}                                /* ConvertToMicroFeatures */


/*---------------------------------------------------------------------------*/
MICROFEATURE ExtractMicroFeature(MFOUTLINE Start, MFOUTLINE End) {
/*
 **      Parameters:
 **              Start           starting point of micro-feature
 **              End             ending point of micro-feature
 **      Globals: none
 **      Operation:
 **              This routine computes the feature parameters which describe
 **              the micro-feature that starts and Start and ends at End.
 **              A new micro-feature is allocated, filled with the feature
 **              parameters, and returned.  The routine assumes that
 **              Start and End are not the same point.  If they are the
 **              same point, NULL is returned, a warning message is
 **              printed, and the current outline is dumped to stdout.
 **      Return: New micro-feature or NULL if the feature was rejected.
 **      Exceptions: none
 **      History: 7/26/89, DSJ, Created.
 **              11/17/89, DSJ, Added handling for Start and End same point.
 */
  MICROFEATURE NewFeature;
  MFEDGEPT *P1, *P2;

  P1 = PointAt (Start);
  P2 = PointAt (End);

  NewFeature = NewMicroFeature ();
  NewFeature[XPOSITION] = AverageOf (P1->Point.x, P2->Point.x);
  NewFeature[YPOSITION] = AverageOf (P1->Point.y, P2->Point.y);
  NewFeature[MFLENGTH] = DistanceBetween (P1->Point, P2->Point);
  NewFeature[ORIENTATION] = NormalizedAngleFrom(&P1->Point, &P2->Point, 1.0);
  ComputeBulges(Start, End, NewFeature);
  return (NewFeature);
}                                /* ExtractMicroFeature */


/*---------------------------------------------------------------------------*/
void SmearBulges(MICROFEATURES MicroFeatures, FLOAT32 XScale, FLOAT32 YScale) {
/*
 **      Parameters:
 **              MicroFeatures   features to be smeared
 **   XScale    # of normalized units per pixel in x dir
 **   YScale    # of normalized units per pixel in y dir
 **      Globals: none
 **      Operation: Add a random amount to each bulge parameter of each
 **              feature.  The amount added is between -0.5 pixels and
 **              0.5 pixels.  This is done to prevent the prototypes
 **              generated in training from being unrealistically tight.
 **      Return: none
 **      Exceptions: none
 **      History: Thu Jun 28 18:03:38 1990, DSJ, Created.
 */
  MICROFEATURE MicroFeature;
  FLOAT32 MinSmear;
  FLOAT32 MaxSmear;
  FLOAT32 Cos, Sin;
  FLOAT32 Scale;

  iterate(MicroFeatures) {
    MicroFeature = NextFeatureOf (MicroFeatures);

    Cos = fabs(cos(2.0 * PI * MicroFeature[ORIENTATION]));
    Sin = fabs(sin(2.0 * PI * MicroFeature[ORIENTATION]));
    Scale = YScale * Cos + XScale * Sin;

    MinSmear = -0.5 * Scale / (BULGENORMALIZER * MicroFeature[MFLENGTH]);
    MaxSmear = 0.5 * Scale / (BULGENORMALIZER * MicroFeature[MFLENGTH]);

    MicroFeature[FIRSTBULGE] += UniformRandomNumber (MinSmear, MaxSmear);
    MicroFeature[SECONDBULGE] += UniformRandomNumber (MinSmear, MaxSmear);
  }
}                                /* SmearBulges */
