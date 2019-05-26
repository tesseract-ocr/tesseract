/******************************************************************************
 ** Filename:       mfx.c
 ** Purpose:        Micro feature extraction routines
 ** Author:         Dan Johnson
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
 *****************************************************************************/
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/

#include "mfx.h"
#include "mfdefs.h"
#include "mfoutline.h"
#include "clusttool.h"          //NEEDED
#include "intfx.h"
#include "normalis.h"
#include "params.h"

/*----------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------*/

/* old numbers corresponded to 10.0 degrees and 80.0 degrees */
double_VAR(classify_min_slope, 0.414213562,
           "Slope below which lines are called horizontal");
double_VAR(classify_max_slope, 2.414213562,
           "Slope above which lines are called vertical");

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/

MICROFEATURES ConvertToMicroFeatures(MFOUTLINE Outline,
                                     MICROFEATURES MicroFeatures);

MICROFEATURE ExtractMicroFeature(MFOUTLINE Start, MFOUTLINE End);

/*----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------*/

/**
 * This routine extracts micro-features from the specified
 * blob and returns a list of the micro-features.  All
 * micro-features are normalized according to the specified
 * line statistics.
 * @param Blob blob to extract micro-features from
 * @param cn_denorm control parameter to feature extractor
 * @return List of micro-features extracted from the blob.
 */
MICROFEATURES BlobMicroFeatures(TBLOB* Blob, const DENORM& cn_denorm) {
  MICROFEATURES MicroFeatures = NIL_LIST;
  LIST Outlines;
  LIST RemainingOutlines;
  MFOUTLINE Outline;

  if (Blob != nullptr) {
    Outlines = ConvertBlob(Blob);

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = static_cast<MFOUTLINE>first_node (RemainingOutlines);
      CharNormalizeOutline(Outline, cn_denorm);
    }

    RemainingOutlines = Outlines;
    iterate(RemainingOutlines) {
      Outline = static_cast<MFOUTLINE>first_node(RemainingOutlines);
      FindDirectionChanges(Outline, classify_min_slope, classify_max_slope);
      MarkDirectionChanges(Outline);
      MicroFeatures = ConvertToMicroFeatures(Outline, MicroFeatures);
    }
    FreeOutlines(Outlines);
  }
  return MicroFeatures;
}                                /* BlobMicroFeatures */

/*---------------------------------------------------------------------------
            Private Code
---------------------------------------------------------------------------*/

/**
 * Convert Outline to MicroFeatures
 * @param Outline         outline to extract micro-features from
 * @param MicroFeatures   list of micro-features to add to
 * @return List of micro-features with new features added to front.
 * @note Globals: none
 */
MICROFEATURES ConvertToMicroFeatures(MFOUTLINE Outline,
                                     MICROFEATURES MicroFeatures) {
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
      if (NewFeature != nullptr)
        MicroFeatures = push (MicroFeatures, NewFeature);
    }
    Last = Current;
  }
  while (Last != First);

  return (MicroFeatures);
}                                /* ConvertToMicroFeatures */

/**
 * This routine computes the feature parameters which describe
 * the micro-feature that starts and Start and ends at End.
 * A new micro-feature is allocated, filled with the feature
 * parameters, and returned.  The routine assumes that
 * Start and End are not the same point.  If they are the
 * same point, nullptr is returned, a warning message is
 * printed, and the current outline is dumped to stdout.
 * @param Start starting point of micro-feature
 * @param End ending point of micro-feature
 * @return New micro-feature or nullptr if the feature was rejected.
 * @note Globals: none
 */
MICROFEATURE ExtractMicroFeature(MFOUTLINE Start, MFOUTLINE End) {
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
