/******************************************************************************
 **	Filename:    features.c
 **	Purpose:     Generic definition of a feature.
 **	Author:      Dan Johnson
 **	History:     Mon May 21 10:49:04 1990, DSJ, Created.
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
#include "ocrfeatures.h"
#include "emalloc.h"
#include "callcpp.h"
#include "danerror.h"
#include "freelist.h"
#include "scanutils.h"

#include <assert.h>
#include <math.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
BOOL8 AddFeature(FEATURE_SET FeatureSet, FEATURE Feature) {
/*
 **	Parameters:
 **		FeatureSet	set of features to add Feature to
 **		Feature		feature to be added to FeatureSet
 **	Globals: none
 **	Operation: Add a feature to a feature set.  If the feature set is
 **		already full, FALSE is returned to indicate that the
 **		feature could not be added to the set; otherwise, TRUE is
 **		returned.
 **	Return: TRUE if feature added to set, FALSE if set is already full.
 **	Exceptions: none
 **	History: Tue May 22 17:22:23 1990, DSJ, Created.
 */
  if (FeatureSet->NumFeatures >= FeatureSet->MaxNumFeatures) {
    FreeFeature(Feature);
    return FALSE;
  }

  FeatureSet->Features[FeatureSet->NumFeatures++] = Feature;
  return TRUE;
}                                /* AddFeature */

/*---------------------------------------------------------------------------*/
void FreeFeature(FEATURE Feature) {
/*
 **	Parameters:
 **		Feature		feature to be deallocated.
 **	Globals: none
 **	Operation: Release the memory consumed by the specified feature.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 21 13:33:27 1990, DSJ, Created.
 */
  if (Feature) {
    free_struct (Feature, sizeof (FEATURE_STRUCT)
      + sizeof (FLOAT32) * (Feature->Type->NumParams - 1),
      "sizeof(FEATURE_STRUCT)+sizeof(FLOAT32)*(NumParamsIn(Feature)-1)");
  }

}                                /* FreeFeature */


/*---------------------------------------------------------------------------*/
void FreeFeatureSet(FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		FeatureSet	set of features to be freed
 **	Globals: none
 **	Operation: Release the memory consumed by the specified feature
 **		set.  This routine also frees the memory consumed by the
 **		features contained in the set.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 21 13:59:46 1990, DSJ, Created.
 */
  int i;

  if (FeatureSet) {
    for (i = 0; i < FeatureSet->NumFeatures; i++)
      FreeFeature(FeatureSet->Features[i]);
    memfree(FeatureSet);
  }
}                                /* FreeFeatureSet */


/*---------------------------------------------------------------------------*/
FEATURE NewFeature(const FEATURE_DESC_STRUCT* FeatureDesc) {
/*
 **	Parameters:
 **		FeatureDesc	description of feature to be created.
 **	Globals: none
 **	Operation: Allocate and return a new feature of the specified
 **		type.
 **	Return: New feature.
 **	Exceptions: none
 **	History: Mon May 21 14:06:42 1990, DSJ, Created.
 */
  FEATURE Feature;

  Feature = (FEATURE) alloc_struct (sizeof (FEATURE_STRUCT) +
    (FeatureDesc->NumParams - 1) *
    sizeof (FLOAT32),
    "sizeof(FEATURE_STRUCT)+sizeof(FLOAT32)*(NumParamsIn(Feature)-1)");
  Feature->Type = FeatureDesc;
  return (Feature);

}                                /* NewFeature */


/*---------------------------------------------------------------------------*/
FEATURE_SET NewFeatureSet(int NumFeatures) {
/*
 **	Parameters:
 **		NumFeatures	maximum # of features to be put in feature set
 **	Globals: none
 **	Operation: Allocate and return a new feature set large enough to
 **		hold the specified number of features.
 **	Return: New feature set.
 **	Exceptions: none
 **	History: Mon May 21 14:22:40 1990, DSJ, Created.
 */
  FEATURE_SET FeatureSet;

  FeatureSet = (FEATURE_SET) Emalloc (sizeof (FEATURE_SET_STRUCT) +
    (NumFeatures - 1) * sizeof (FEATURE));
  FeatureSet->MaxNumFeatures = NumFeatures;
  FeatureSet->NumFeatures = 0;
  return (FeatureSet);

}                                /* NewFeatureSet */


/*---------------------------------------------------------------------------*/
FEATURE ReadFeature(FILE *File, const FEATURE_DESC_STRUCT* FeatureDesc) {
/*
 **	Parameters:
 **		File		open text file to read feature from
 **		FeatureDesc	specifies type of feature to read from File
 **	Globals: none
 **	Operation: Create a new feature of the specified type and read in
 **		the value of its parameters from File.  The extra penalty
 **		for the feature is also computed by calling the appropriate
 **		function for the specified feature type.  The correct text
 **		representation for a feature is a list of N floats where
 **		N is the number of parameters in the feature.
 **	Return: New feature read from File.
 **	Exceptions: ILLEGAL_FEATURE_PARAM if text file doesn't match expected format
 **	History: Wed May 23 08:53:16 1990, DSJ, Created.
 */
  FEATURE Feature;
  int i;

  Feature = NewFeature (FeatureDesc);
  for (i = 0; i < Feature->Type->NumParams; i++) {
    if (fscanf (File, "%f", &(Feature->Params[i])) != 1)
      DoError (ILLEGAL_FEATURE_PARAM, "Illegal feature parameter spec");
#ifndef WIN32
    assert (!isnan(Feature->Params[i]));
#endif
  }
  return (Feature);

}                                /* ReadFeature */


/*---------------------------------------------------------------------------*/
FEATURE_SET ReadFeatureSet(FILE *File, const FEATURE_DESC_STRUCT* FeatureDesc) {
/*
 **	Parameters:
 **		File		open text file to read new feature set from
 **		FeatureDesc	specifies type of feature to read from File
 **	Globals: none
 **	Operation: Create a new feature set of the specified type and read in
 **		the features from File.  The correct text representation
 **		for a feature set is an integer which specifies the number (N)
 **		of features in a set followed by a list of N feature
 **		descriptions.
 **	Return: New feature set read from File.
 **	Exceptions: none
 **	History: Wed May 23 09:17:31 1990, DSJ, Created.
 */
  FEATURE_SET FeatureSet;
  int NumFeatures;
  int i;

  if (fscanf (File, "%d", &NumFeatures) != 1 || NumFeatures < 0)
    DoError (ILLEGAL_NUM_FEATURES, "Illegal number of features in set");

  FeatureSet = NewFeatureSet (NumFeatures);
  for (i = 0; i < NumFeatures; i++)
    AddFeature (FeatureSet, ReadFeature (File, FeatureDesc));

  return (FeatureSet);

}                                /* ReadFeatureSet */


/*---------------------------------------------------------------------------*/
void WriteFeature(FILE *File, FEATURE Feature) {
/*
 **	Parameters:
 **		File		open text file to write Feature to
 **		Feature		feature to write out to File
 **	Globals: none
 **	Operation: Write a textual representation of Feature to File.
 **		This representation is simply a list of the N parameters
 **		of the feature, terminated with a newline.  It is assumed
 **		that the ExtraPenalty field can be reconstructed from the
 **		parameters of the feature.  It is also assumed that the
 **		feature type information is specified or assumed elsewhere.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 23 09:28:18 1990, DSJ, Created.
 */
  int i;

  for (i = 0; i < Feature->Type->NumParams; i++) {
#ifndef WIN32
    assert (!isnan(Feature->Params[i]));
#endif
    fprintf (File, " %12g", Feature->Params[i]);
  }
  fprintf (File, "\n");

}                                /* WriteFeature */


/*---------------------------------------------------------------------------*/
void WriteFeatureSet(FILE *File, FEATURE_SET FeatureSet) {
/*
 **	Parameters:
 **		File		open text file to write FeatureSet to
 **		FeatureSet	feature set to write to File
 **	Globals: none
 **	Operation: Write a textual representation of FeatureSet to File.
 **		This representation is an integer specifying the number of
 **		features in the set, followed by a newline, followed by
 **		text representations for each feature in the set.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 23 10:06:03 1990, DSJ, Created.
 */
  int i;

  if (FeatureSet) {
    fprintf (File, "%d\n", FeatureSet->NumFeatures);
    for (i = 0; i < FeatureSet->NumFeatures; i++)
      WriteFeature (File, FeatureSet->Features[i]);
  }
}                                /* WriteFeatureSet */


/*---------------------------------------------------------------------------*/
void WriteOldParamDesc(FILE *File, const FEATURE_DESC_STRUCT* FeatureDesc) {
/*
 **	Parameters:
 **		File		open text file to write FeatureDesc to
 **		FeatureDesc	feature descriptor to write to File
 **	Globals: none
 **	Operation: Write a textual representation of FeatureDesc to File
 **		in the old format (i.e. the format used by the clusterer).
 **		This format is:
 **			Number of Params
 **			Description of Param 1
 **			...
 **	Return: none
 **	Exceptions: none
 **	History: Fri May 25 15:27:18 1990, DSJ, Created.
 */
  int i;

  fprintf (File, "%d\n", FeatureDesc->NumParams);
  for (i = 0; i < FeatureDesc->NumParams; i++) {
    if (FeatureDesc->ParamDesc[i].Circular)
      fprintf (File, "circular ");
    else
      fprintf (File, "linear   ");

    if (FeatureDesc->ParamDesc[i].NonEssential)
      fprintf (File, "non-essential  ");
    else
      fprintf (File, "essential      ");

    fprintf (File, "%f  %f\n",
      FeatureDesc->ParamDesc[i].Min, FeatureDesc->ParamDesc[i].Max);
  }
}                                /* WriteOldParamDesc */
