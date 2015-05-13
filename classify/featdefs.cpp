/******************************************************************************
 **	Filename:    featdefs.c
 **	Purpose:     Definitions of currently defined feature types.
 **	Author:      Dan Johnson
 **	History:     Mon May 21 10:26:21 1990, DSJ, Created.
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
/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#ifdef _MSC_VER
#include <mathfix.h>
#endif

#include "featdefs.h"
#include "emalloc.h"
#include "danerror.h"
#include "scanutils.h"

#include <string.h>
#include <stdio.h>

/** define errors triggered by this module */
#define ILLEGAL_NUM_SETS  3001

#define PICO_FEATURE_LENGTH 0.05

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
const char* kMicroFeatureType = "mf";
const char* kCNFeatureType = "cn";
const char* kIntFeatureType = "if";
const char* kGeoFeatureType = "tb";

// Define all of the parameters for the MicroFeature type.
StartParamDesc(MicroFeatureParams)
DefineParam(0, 0, -0.5, 0.5)
DefineParam(0, 0, -0.25, 0.75)
DefineParam(0, 1, 0.0, 1.0)
DefineParam(1, 0, 0.0, 1.0)
DefineParam (0, 1, -0.5, 0.5)
DefineParam (0, 1, -0.5, 0.5)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(MicroFeatureDesc, 5, 1, kMicroFeatureType, MicroFeatureParams)

// Define all of the parameters for the NormFeat type.
StartParamDesc (CharNormParams)
DefineParam(0, 0, -0.25, 0.75)
DefineParam(0, 1, 0.0, 1.0)
DefineParam(0, 0, 0.0, 1.0)
DefineParam(0, 0, 0.0, 1.0)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(CharNormDesc, 4, 0, kCNFeatureType, CharNormParams)

// Define all of the parameters for the IntFeature type
StartParamDesc(IntFeatParams)
DefineParam(0, 0, 0.0, 255.0)
DefineParam(0, 0, 0.0, 255.0)
DefineParam(1, 0, 0.0, 255.0)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(IntFeatDesc, 2, 1, kIntFeatureType, IntFeatParams)

// Define all of the parameters for the GeoFeature type
StartParamDesc(GeoFeatParams)
DefineParam(0, 0, 0.0, 255.0)
DefineParam(0, 0, 0.0, 255.0)
DefineParam(0, 0, 0.0, 255.0)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(GeoFeatDesc, 3, 0, kGeoFeatureType, GeoFeatParams)

// Other features used for training the adaptive classifier, but not used
// during normal training, therefore not in the DescDefs array.

// Define all of the parameters for the PicoFeature type
// define knob that can be used to adjust pico-feature length.
FLOAT32 PicoFeatureLength = PICO_FEATURE_LENGTH;
StartParamDesc(PicoFeatParams)
DefineParam(0, 0, -0.25, 0.75)
DefineParam(1, 0, 0.0, 1.0)
DefineParam(0, 0, -0.5, 0.5)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(PicoFeatDesc, 2, 1, "pf", PicoFeatParams)

// Define all of the parameters for the OutlineFeature type.
StartParamDesc(OutlineFeatParams)
DefineParam(0, 0, -0.5, 0.5)
DefineParam(0, 0, -0.25, 0.75)
DefineParam(0, 0, 0.0, 1.0)
DefineParam(1, 0, 0.0, 1.0)
EndParamDesc
// Now define the feature type itself (see features.h for parameters).
DefineFeature(OutlineFeatDesc, 3, 1, "of", OutlineFeatParams)

// MUST be kept in-sync with ExtractorDefs in fxdefs.cpp.
static const FEATURE_DESC_STRUCT *DescDefs[NUM_FEATURE_TYPES] = {
  &MicroFeatureDesc,
  &CharNormDesc,
  &IntFeatDesc,
  &GeoFeatDesc
};

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
void InitFeatureDefs(FEATURE_DEFS_STRUCT *featuredefs) {
  featuredefs->NumFeatureTypes = NUM_FEATURE_TYPES;
  for (int i = 0; i < NUM_FEATURE_TYPES; ++i) {
    featuredefs->FeatureDesc[i] = DescDefs[i];
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Release the memory consumed by the specified character
 * description and all of the features in that description.
 *
 * @param CharDesc character description to be deallocated
 *
 * Globals: 
 * - none
 *
 * @note Exceptions: none
 * @note History: Wed May 23 13:52:19 1990, DSJ, Created.
 */
void FreeCharDescription(CHAR_DESC CharDesc) {
  int i;

  if (CharDesc) {
    for (i = 0; i < CharDesc->NumFeatureSets; i++)
      FreeFeatureSet (CharDesc->FeatureSets[i]);
    Efree(CharDesc);
  }
}                                /* FreeCharDescription */


/*---------------------------------------------------------------------------*/
/**
 * Allocate a new character description, initialize its
 * feature sets to be empty, and return it.
 *
 * Globals: 
 * - none
 *
 * @return New character description structure.
 * @note Exceptions: none
 * @note History: Wed May 23 15:27:10 1990, DSJ, Created.
 */
CHAR_DESC NewCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs) {
  CHAR_DESC CharDesc;
  int i;

  CharDesc = (CHAR_DESC) Emalloc (sizeof (CHAR_DESC_STRUCT));
  CharDesc->NumFeatureSets = FeatureDefs.NumFeatureTypes;

  for (i = 0; i < CharDesc->NumFeatureSets; i++)
    CharDesc->FeatureSets[i] = NULL;

  return (CharDesc);

}                                /* NewCharDescription */


/*---------------------------------------------------------------------------*/
/**
 * Appends a textual representation of CharDesc to str.
 * The format used is to write out the number of feature
 * sets which will be written followed by a representation of
 * each feature set.
 *
 * Each set starts with the short name for that feature followed
 * by a description of the feature set.  Feature sets which are
 * not present are not written.
 *
 * @param FeatureDefs    definitions of feature types/extractors
 * @param str            string to append CharDesc to
 * @param CharDesc       character description to write to File
 *
 * @note Exceptions: none
 * @note History: Wed May 23 17:21:18 1990, DSJ, Created.
 */
void WriteCharDescription(const FEATURE_DEFS_STRUCT& FeatureDefs,
                          CHAR_DESC CharDesc, STRING* str) {
  int Type;
  int NumSetsToWrite = 0;

  for (Type = 0; Type < CharDesc->NumFeatureSets; Type++)
    if (CharDesc->FeatureSets[Type])
      NumSetsToWrite++;

  str->add_str_int(" ", NumSetsToWrite);
  *str += "\n";
  for (Type = 0; Type < CharDesc->NumFeatureSets; Type++) {
    if (CharDesc->FeatureSets[Type]) {
      *str += FeatureDefs.FeatureDesc[Type]->ShortName;
      *str += " ";
      WriteFeatureSet(CharDesc->FeatureSets[Type], str);
    }
  }
}                                /* WriteCharDescription */

// Return whether all of the fields of the given feature set
// are well defined (not inf or nan).
bool ValidCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs,
                          CHAR_DESC CharDesc) {
  bool anything_written = false;
  bool well_formed = true;
  for (int Type = 0; Type < CharDesc->NumFeatureSets; Type++) {
    if (CharDesc->FeatureSets[Type]) {
      for (int i = 0; i < CharDesc->FeatureSets[Type]->NumFeatures; i++) {
        FEATURE feat = CharDesc->FeatureSets[Type]->Features[i];
        for (int p = 0; p < feat->Type->NumParams; p++) {
          if (isnan(feat->Params[p]) || isinf(feat->Params[p]))
            well_formed = false;
          else
            anything_written = true;
        }
      }
    } else {
      return false;
    }
  }
  return anything_written && well_formed;
}                                /* ValidCharDescription */

/*---------------------------------------------------------------------------*/
/**
 * Read a character description from File, and return
 * a data structure containing this information.  The data
 * is formatted as follows:
 * @verbatim
     NumberOfSets
             ShortNameForSet1 Set1
             ShortNameForSet2 Set2
             ...
   @endverbatim
 *
 * Globals: 
 * - none
 * 
 * @param FeatureDefs    definitions of feature types/extractors
 * @param File open text file to read character description from
 * @return Character description read from File.
 * @note Exceptions: 
 * - ILLEGAL_NUM_SETS
 * @note History: Wed May 23 17:32:48 1990, DSJ, Created.
 */
CHAR_DESC ReadCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              FILE *File) {
  int NumSetsToRead;
  char ShortName[FEAT_NAME_SIZE];
  CHAR_DESC CharDesc;
  int Type;

  if (tfscanf(File, "%d", &NumSetsToRead) != 1 ||
    NumSetsToRead < 0 || NumSetsToRead > FeatureDefs.NumFeatureTypes)
    DoError (ILLEGAL_NUM_SETS, "Illegal number of feature sets");

  CharDesc = NewCharDescription(FeatureDefs);
  for (; NumSetsToRead > 0; NumSetsToRead--) {
    tfscanf(File, "%s", ShortName);
    Type = ShortNameToFeatureType(FeatureDefs, ShortName);
    CharDesc->FeatureSets[Type] =
      ReadFeatureSet (File, FeatureDefs.FeatureDesc[Type]);
  }
  return (CharDesc);

}                                // ReadCharDescription


/*---------------------------------------------------------------------------*/
/**
 * Search thru all features currently defined and return
 * the feature type for the feature with the specified short
 * name.  Trap an error if the specified name is not found.
 *
 * Globals: 
 * - none
 *
 * @param FeatureDefs    definitions of feature types/extractors
 * @param ShortName short name of a feature type
 * @return Feature type which corresponds to ShortName.
 * @note Exceptions: 
 * - ILLEGAL_SHORT_NAME
 * @note History: Wed May 23 15:36:05 1990, DSJ, Created.
 */
int ShortNameToFeatureType(const FEATURE_DEFS_STRUCT &FeatureDefs,
                           const char *ShortName) {
  int i;

  for (i = 0; i < FeatureDefs.NumFeatureTypes; i++)
    if (!strcmp ((FeatureDefs.FeatureDesc[i]->ShortName), ShortName))
      return (i);
  DoError (ILLEGAL_SHORT_NAME, "Illegal short name for a feature");
  return 0;

}                                // ShortNameToFeatureType
