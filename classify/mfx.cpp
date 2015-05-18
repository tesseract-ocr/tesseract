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
#include "normalis.h"
#include "params.h"

#include <math.h>

/**----------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------**/

/* old numbers corresponded to 10.0 degrees and 80.0 degrees */
double_VAR(classify_min_slope, 0.414213562,
           "Slope below which lines are called horizontal");
double_VAR(classify_max_slope, 2.414213562,
           "Slope above which lines are called vertical");

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
/* miscellaneous macros */
#define NormalizeAngle(A)       ( (((A)<0)?((A)+2*PI):(A)) / (2*PI) )

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
FLOAT32 ComputeOrientation(MFEDGEPT *Start, MFEDGEPT *End);

MICROFEATURES ConvertToMicroFeatures(MFOUTLINE Outline,
                                     MICROFEATURES MicroFeatures);

MICROFEATURE ExtractMicroFeature(MFOUTLINE Start, MFOUTLINE End);

/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/

/*---------------------------------------------------------------------------*/
CHAR_FEATURES BlobMicroFeatures(TBLOB *Blob, const DENORM& bl_denorm,
                                const DENORM& cn_denorm,
                                const INT_FX_RESULT_STRUCT& fx_info) {
/*
 **      Parameters:
 **              Blob            blob to extract micro-features from
 **              denorm          control parameter to feature extractor
 **      Operation:
 **              This routine extracts micro-features from the specified
 **              blob and returns a list of the micro-features.  All
 **              micro-features are normalized according to the specified
 **              line statistics.
 **      Return: List of micro-features extracted from the blob.
 **      Exceptions: none
 **      History: 7/21/89, DSJ, Created.
 */
  MICROFEATURES MicroFeatures = NIL_LIST;
  LIST Outlines;
  LIST RemainingOutlines;
  MFOUTLINE Outline;

  if (Blob != NULL) {
    Outlines = ConvertBlob(Blob);

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = (MFOUTLINE) first_node (RemainingOutlines);
      CharNormalizeOutline(Outline, cn_denorm);
    }

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = (MFOUTLINE) first_node(RemainingOutlines);
      FindDirectionChanges(Outline, classify_min_slope, classify_max_slope);
      MarkDirectionChanges(Outline);
      MicroFeatures = ConvertToMicroFeatures(Outline, MicroFeatures);
    }
    FreeOutlines(Outlines);
  }
  return ((CHAR_FEATURES) MicroFeatures);
}                                /* BlobMicroFeatures */


/*---------------------------------------------------------------------------
            Private Code
---------------------------------------------------------------------------*/

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
    if (!PointAt(Current)->Hidden) {
      NewFeature = ExtractMicroFeature (Last, Current);
      if (NewFeature != NULL)
        MicroFeatures = push (MicroFeatures, NewFeature);
    }
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

  P1 = PointAt(Start);
  P2 = PointAt(End);

  NewFeature = NewMicroFeature ();
  NewFeature[XPOSITION] = AverageOf(P1->Point.x, P2->Point.x);
  NewFeature[YPOSITION] = AverageOf(P1->Point.y, P2->Point.y);
  NewFeature[MFLENGTH] = DistanceBetween(P1->Point, P2->Point);
  NewFeature[ORIENTATION] = NormalizedAngleFrom(&P1->Point, &P2->Point, 1.0);
  NewFeature[FIRSTBULGE] = 0.0f;  // deprecated
  NewFeature[SECONDBULGE] = 0.0f;  // deprecated

  return NewFeature;
}                                /* ExtractMicroFeature */
