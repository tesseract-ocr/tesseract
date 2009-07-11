/******************************************************************************
 **	Filename:    picofeat.c
 **	Purpose:     Definition of pico-features.
 **	Author:      Dan Johnson
 **	History:     9/4/90, DSJ, Created.
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
#include "picofeat.h"
#include "mfoutline.h"
#include "hideedge.h"
#include "fpoint.h"
#include "varable.h"

#include <math.h>

#include "ocrfeatures.h"         //Debug
#include <stdio.h>               //Debug
#include "efio.h"                //Debug

/*---------------------------------------------------------------------------
          Variables
----------------------------------------------------------------------------*/

double_VAR(classify_pico_feature_length, 0.05, "Pico Feature Length");

/*---------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------*/
void ConvertSegmentToPicoFeat(FPOINT *Start,
                              FPOINT *End,
                              FEATURE_SET FeatureSet);

void ConvertToPicoFeatures2(MFOUTLINE Outline, FEATURE_SET FeatureSet);

void NormalizePicoX(FEATURE_SET FeatureSet);

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FEATURE_SET ExtractPicoFeatures(TBLOB *Blob, LINE_STATS *LineStats) {
/*
 **	Parameters:
 **		Blob		blob to extract pico-features from
 **		LineStats	statistics on text row blob is in
 **	Globals:
 **		classify_norm_method	normalization method currently specified
 **	Operation: Dummy for now.
 **	Return: Pico-features for Blob.
 **	Exceptions: none
 **	History: 9/4/90, DSJ, Created.
 */
  LIST Outlines;
  LIST RemainingOutlines;
  MFOUTLINE Outline;
  FEATURE_SET FeatureSet;
  FLOAT32 XScale, YScale;

  FeatureSet = NewFeatureSet (MAX_PICO_FEATURES);

  Outlines = ConvertBlob (Blob);

  NormalizeOutlines(Outlines, LineStats, &XScale, &YScale);
  RemainingOutlines = Outlines;
  iterate(RemainingOutlines) {
    Outline = (MFOUTLINE) first_node (RemainingOutlines);
    /*---------Debug--------------------------------------------------*
    OFile = fopen ("f:/ims/debug/pfOutline.logCPP", "r");
    if (OFile == NULL)
    {
      OFile = Efopen ("f:/ims/debug/pfOutline.logCPP", "w");
      WriteOutline(OFile, Outline);
    }
    else
    {
      fclose (OFile);
      OFile = Efopen ("f:/ims/debug/pfOutline.logCPP", "a");
    }
    WriteOutline(OFile, Outline);
    fclose (OFile);
    *--------------------------------------------------------------------*/
    ConvertToPicoFeatures2(Outline, FeatureSet);
  }
  if (classify_norm_method == baseline)
    NormalizePicoX(FeatureSet);
  /*---------Debug--------------------------------------------------*
  File = fopen ("f:/ims/debug/pfFeatSet.logCPP", "r");
  if (File == NULL)
  {
    File = Efopen ("f:/ims/debug/pfFeatSet.logCPP", "w");
    WriteFeatureSet(File, FeatureSet);
  }
  else
  {
    fclose (File);
    File = Efopen ("f:/ims/debug/pfFeatSet.logCPP", "a");
  }
  WriteFeatureSet(File, FeatureSet);
  fclose (File);
  *--------------------------------------------------------------------*/
  FreeOutlines(Outlines);
  return (FeatureSet);

}                                /* ExtractPicoFeatures */

/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ConvertSegmentToPicoFeat(FPOINT *Start,
                              FPOINT *End,
                              FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		Start		starting point of pico-feature
 **		End		ending point of pico-feature
 **		FeatureSet	set to add pico-feature to
 **	Globals:
 **		classify_pico_feature_length	length of a single pico-feature
 **	Operation: This routine converts an entire segment of an outline
 **		into a set of pico features which are added to
 **		FeatureSet.  The length of the segment is rounded to the
 **		nearest whole number of pico-features.  The pico-features
 **		are spaced evenly over the entire segment.
 **	Return: none (results are placed in FeatureSet)
 **	Exceptions: none
 **	History: Tue Apr 30 15:44:34 1991, DSJ, Created.
 */
  FEATURE Feature;
  FLOAT32 Angle;
  FLOAT32 Length;
  int NumFeatures;
  FPOINT Center;
  FPOINT Delta;
  int i;

  Angle = NormalizedAngleFrom (Start, End, 1.0);
  Length = DistanceBetween (*Start, *End);
  NumFeatures = (int) floor (Length / classify_pico_feature_length + 0.5);
  if (NumFeatures < 1)
    NumFeatures = 1;

  /* compute vector for one pico feature */
  Delta.x = XDelta (*Start, *End) / NumFeatures;
  Delta.y = YDelta (*Start, *End) / NumFeatures;

  /* compute position of first pico feature */
  Center.x = Start->x + Delta.x / 2.0;
  Center.y = Start->y + Delta.y / 2.0;

  /* compute each pico feature in segment and add to feature set */
  for (i = 0; i < NumFeatures; i++) {
    Feature = NewFeature (&PicoFeatDesc);
    Feature->Params[PicoFeatDir] = Angle;
    Feature->Params[PicoFeatX] = Center.x;
    Feature->Params[PicoFeatY] = Center.y;
    AddFeature(FeatureSet, Feature);

    Center.x += Delta.x;
    Center.y += Delta.y;
  }
}                                /* ConvertSegmentToPicoFeat */


/*---------------------------------------------------------------------------*/
void ConvertToPicoFeatures2(MFOUTLINE Outline, FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		Outline		outline to extract micro-features from
 **		FeatureSet	set of features to add pico-features to
 **	Globals:
 **		classify_pico_feature_length
 **                             length of features to be extracted
 **	Operation:
 **		This routine steps thru the specified outline and cuts it
 **		up into pieces of equal length.  These pieces become the
 **		desired pico-features.  Each segment in the outline
 **		is converted into an integral number of pico-features.
 **	Return: none (results are returned in FeatureSet)
 **	Exceptions: none
 **	History: 4/30/91, DSJ, Adapted from ConvertToPicoFeatures().
 */
  MFOUTLINE Next;
  MFOUTLINE First;
  MFOUTLINE Current;

  if (DegenerateOutline(Outline))
    return;

  First = Outline;
  Current = First;
  Next = NextPointAfter(Current);
  do {
    /* note that an edge is hidden if the ending point of the edge is
       marked as hidden.  This situation happens because the order of
       the outlines is reversed when they are converted from the old
       format.  In the old format, a hidden edge is marked by the
       starting point for that edge. */
    if (!(PointAt(Next)->Hidden))
      ConvertSegmentToPicoFeat (&(PointAt(Current)->Point),
        &(PointAt(Next)->Point), FeatureSet);

    Current = Next;
    Next = NextPointAfter(Current);
  }
  while (Current != First);

}                                /* ConvertToPicoFeatures2 */


/*---------------------------------------------------------------------------*/
void NormalizePicoX(FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		FeatureSet	pico-features to be normalized
 **	Globals: none
 **	Operation: This routine computes the average x position over all
 **		of the pico-features in FeatureSet and then renormalizes
 **		the pico-features to force this average to be the x origin
 **		(i.e. x=0).
 **	Return: none (FeatureSet is changed)
 **	Exceptions: none
 **	History: Tue Sep  4 16:50:08 1990, DSJ, Created.
 */
  int i;
  FEATURE Feature;
  FLOAT32 Origin = 0.0;

  for (i = 0; i < FeatureSet->NumFeatures; i++) {
    Feature = FeatureSet->Features[i];
    Origin += Feature->Params[PicoFeatX];
  }
  Origin /= FeatureSet->NumFeatures;

  for (i = 0; i < FeatureSet->NumFeatures; i++) {
    Feature = FeatureSet->Features[i];
    Feature->Params[PicoFeatX] -= Origin;
  }
}                                /* NormalizePicoX */
