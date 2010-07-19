/******************************************************************************
 **	Filename:    normfeat.c
 **	Purpose:     Definition of char normalization features.
 **	Author:      Dan Johnson
 **	History:     12/14/90, DSJ, Created.
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
#include "normfeat.h"
#include "mfoutline.h"
#include "intfx.h"

#include "ocrfeatures.h"         //Debug
#include <stdio.h>               //Debug
#include "efio.h"                //Debug
//#include "christydbg.h"

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FLOAT32 ActualOutlineLength(FEATURE Feature) {
/*
 **	Parameters:
 **		Feature		normalization feature
 **	Globals: none
 **	Operation: This routine returns the length that the outline
 **		would have been if it were baseline normalized instead
 **		of character normalized.
 **	Return: Baseline normalized length of outline.
 **	Exceptions: none
 **	History: Thu Dec 20 14:50:57 1990, DSJ, Created.
 */
  return (Feature->Params[CharNormLength] * LENGTH_COMPRESSION);

}                                /* ActualOutlineLength */


/*---------------------------------------------------------------------------*/
FEATURE_SET ExtractCharNormFeatures(TBLOB *Blob, LINE_STATS *LineStats) {
/*
 **	Parameters:
 **		Blob		blob to extract char norm feature from
 **		LineStats	statistics on text row blob is in
 **	Globals: none
 **	Operation: Compute a feature whose parameters describe how a
 **		character will be affected by the character normalization
 **		algorithm.  The feature parameters are:
 **			y position of center of mass in baseline coordinates
 **			total length of outlines in baseline coordinates
 **				divided by a scale factor
 **			radii of gyration about the center of mass in
 **				baseline coordinates
 **	Return: Character normalization feature for Blob.
 **	Exceptions: none
 **	History: Wed May 23 18:06:38 1990, DSJ, Created.
 */
  FEATURE_SET FeatureSet;
  FEATURE Feature;
  FLOAT32 Scale;
  FLOAT32 Baseline;
  LIST Outlines;
  INT_FEATURE_ARRAY blfeatures;
  INT_FEATURE_ARRAY cnfeatures;
  INT_FX_RESULT_STRUCT FXInfo;

  /* allocate the feature and feature set - note that there is always one
     and only one char normalization feature for any blob */
  FeatureSet = NewFeatureSet (1);
  Feature = NewFeature (&CharNormDesc);
  AddFeature(FeatureSet, Feature);

  /* compute the normalization statistics for this blob */
  Outlines = ConvertBlob (Blob);
#ifdef DEBUG_NORMFEAT
  FILE* OFile;
  OFile = fopen ("nfOutline.logCPP", "r");
  if (OFile == NULL)
  {
    OFile = Efopen ("nfOutline.logCPP", "w");
    WriteOutlines(OFile, Outlines);
  }
  else
  {
    fclose (OFile);
    OFile = Efopen ("nfOutline.logCPP", "a");
  }
  WriteOutlines(OFile, Outlines);
  fclose (OFile);
#endif

  ExtractIntFeat(Blob, blfeatures, cnfeatures, &FXInfo);
  Baseline = BaselineAt (LineStats, FXInfo.Xmean);
  Scale = ComputeScaleFactor (LineStats);
  Feature->Params[CharNormY] = (FXInfo.Ymean - Baseline) * Scale;
  Feature->Params[CharNormLength] =
    FXInfo.Length * Scale / LENGTH_COMPRESSION;
  Feature->Params[CharNormRx] = FXInfo.Rx * Scale;
  Feature->Params[CharNormRy] = FXInfo.Ry * Scale;

#ifdef DEBUG_NORMFEAT
  FILE* File;
  File = fopen ("nfFeatSet.logCPP", "r");
  if (File == NULL)
  {
    File = Efopen ("nfFeatSet.logCPP", "w");
    WriteFeatureSet(File, FeatureSet);
  }
  else
  {
    fclose (File);
    File = Efopen ("nfFeatSet.logCPP", "a");
  }
  WriteFeatureSet(File, FeatureSet);
  fclose (File);
#endif
  FreeOutlines(Outlines);
  return (FeatureSet);
}                                /* ExtractCharNormFeatures */
