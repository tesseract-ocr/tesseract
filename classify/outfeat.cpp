/******************************************************************************
 **	Filename:    outfeat.c
 **	Purpose:     Definition of outline-features.
 **	Author:      Dan Johnson
 **	History:     11/13/90, DSJ, Created.
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
#include "outfeat.h"
#include "mfoutline.h"

#include "ocrfeatures.h"         //Debug
#include <stdio.h>               //Debug
#include "efio.h"                //Debug

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FEATURE_SET ExtractOutlineFeatures(TBLOB *Blob, LINE_STATS *LineStats) {
/*
 **	Parameters:
 **		Blob		blob to extract pico-features from
 **		LineStats	statistics on text row blob is in
 **	Globals: none
 **	Operation: Convert each segment in the outline to a feature
 **		and return the features.
 **	Return: Outline-features for Blob.
 **	Exceptions: none
 **	History: 11/13/90, DSJ, Created.
 **		05/24/91, DSJ, Updated for either char or baseline normalize.
 */
  LIST Outlines;
  LIST RemainingOutlines;
  MFOUTLINE Outline;
  FEATURE_SET FeatureSet;
  FLOAT32 XScale, YScale;

  FeatureSet = NewFeatureSet (MAX_OUTLINE_FEATURES);
  if (Blob == NULL)
    return (FeatureSet);

  Outlines = ConvertBlob (Blob);

  NormalizeOutlines(Outlines, LineStats, &XScale, &YScale);
  RemainingOutlines = Outlines;
  iterate(RemainingOutlines) {
    Outline = (MFOUTLINE) first_node (RemainingOutlines);
    ConvertToOutlineFeatures(Outline, FeatureSet);
  }
  if (classify_norm_method == baseline)
    NormalizeOutlineX(FeatureSet);
  FreeOutlines(Outlines);
  return (FeatureSet);
}                                /* ExtractOutlineFeatures */

/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void AddOutlineFeatureToSet(FPOINT *Start,
                            FPOINT *End,
                            FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		Start		starting point of outline-feature
 **		End		ending point of outline-feature
 **		FeatureSet	set to add outline-feature to
 **	Globals: none
 **	Operation: This routine computes the midpoint between Start and
 **		End to obtain the x,y position of the outline-feature.  It
 **		also computes the direction from Start to End as the
 **		direction of the outline-feature and the distance from
 **		Start to End as the length of the outline-feature.
 **		This feature is then
 **		inserted into the next feature slot in FeatureSet.
 **	Return: none (results are placed in FeatureSet)
 **	Exceptions: none
 **	History: 11/13/90, DSJ, Created.
 */
  FEATURE Feature;

  Feature = NewFeature(&OutlineFeatDesc);
  Feature->Params[OutlineFeatDir] = NormalizedAngleFrom(Start, End, 1.0);
  Feature->Params[OutlineFeatX] = AverageOf(Start->x, End->x);
  Feature->Params[OutlineFeatY] = AverageOf(Start->y, End->y);
  Feature->Params[OutlineFeatLength] = DistanceBetween(*Start, *End);
  AddFeature(FeatureSet, Feature);

}                                /* AddOutlineFeatureToSet */


/*---------------------------------------------------------------------------*/
void ConvertToOutlineFeatures(MFOUTLINE Outline, FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		Outline		outline to extract outline-features from
 **		FeatureSet	set of features to add outline-features to
 **	Globals: none
 **	Operation:
 **		This routine steps converts each section in the specified
 **		outline to a feature described by its x,y position, length
 **		and angle.
 **	Return: none (results are returned in FeatureSet)
 **	Exceptions: none
 **	History: 11/13/90, DSJ, Created.
 **			5/24/91, DSJ, Added hidden edge capability.
 */
  MFOUTLINE Next;
  MFOUTLINE First;
  FPOINT FeatureStart;
  FPOINT FeatureEnd;

  if (DegenerateOutline (Outline))
    return;

  First = Outline;
  Next = First;
  do {
    FeatureStart = PointAt(Next)->Point;
    Next = NextPointAfter(Next);

    /* note that an edge is hidden if the ending point of the edge is
       marked as hidden.  This situation happens because the order of
       the outlines is reversed when they are converted from the old
       format.  In the old format, a hidden edge is marked by the
       starting point for that edge. */
    if (!PointAt(Next)->Hidden) {
      FeatureEnd = PointAt(Next)->Point;
      AddOutlineFeatureToSet(&FeatureStart, &FeatureEnd, FeatureSet);
    }
  }
  while (Next != First);
}                                /* ConvertToOutlineFeatures */


/*---------------------------------------------------------------------------*/
void NormalizeOutlineX(FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		FeatureSet	outline-features to be normalized
 **	Globals: none
 **	Operation: This routine computes the weighted average x position
 **		over all of the outline-features in FeatureSet and then
 **		renormalizes the outline-features to force this average
 **		to be the x origin (i.e. x=0).
 **	Return: none (FeatureSet is changed)
 **	Exceptions: none
 **	History: 11/13/90, DSJ, Created.
 */
  int i;
  FEATURE Feature;
  FLOAT32 Length;
  FLOAT32 TotalX = 0.0;
  FLOAT32 TotalWeight = 0.0;
  FLOAT32 Origin;

  if (FeatureSet->NumFeatures <= 0)
    return;

  for (i = 0; i < FeatureSet->NumFeatures; i++) {
    Feature = FeatureSet->Features[i];
    Length = Feature->Params[OutlineFeatLength];
    TotalX += Feature->Params[OutlineFeatX] * Length;
    TotalWeight += Length;
  }
  Origin = TotalX / TotalWeight;

  for (i = 0; i < FeatureSet->NumFeatures; i++) {
    Feature = FeatureSet->Features[i];
    Feature->Params[OutlineFeatX] -= Origin;
  }
}                                /* NormalizeOutlineX */
