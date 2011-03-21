// Copyright 2008 Google Inc. All Rights Reserved.
// Author: scharron@google.com (Samuel Charron)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "commontraining.h"

#include "oldlist.h"
#include "globals.h"
#include "mf.h"
#include "clusttool.h"
#include "cluster.h"
#include "mergenf.h"
#include "tessopt.h"
#include "featdefs.h"
#include "efio.h"
#include "emalloc.h"
#include "tprintf.h"
#include "freelist.h"
#include "unicity_table.h"

#include <math.h>

#define round(x,frag)(floor(x/frag+.5)*frag)


// Global Variables.
char	*Directory = NULL;

const char *InputUnicharsetFile = NULL;
const char *OutputUnicharsetFile = NULL;

const char *InputFontInfoFile = NULL;
const char *InputXHeightsFile = NULL;

FLOAT32 RoundingAccuracy = 0.0f;

char CTFontName[MAXNAMESIZE];

const char* test_ch = "";

/*---------------------------------------------------------------------------*/
void ParseArguments(int argc, char **argv) {
/*
 **	Parameters:
 **		argc	number of command line arguments to parse
 **		argv	command line arguments
 **	Globals:
 **		ShowSignificantProtos	flag controlling proto display
 **		ShowInsignificantProtos	flag controlling proto display
 **		Config			current clustering parameters
 **		tessoptarg, tessoptind		defined by tessopt sys call
 **		Argc, Argv		global copies of argc and argv
 **	Operation:
 **		This routine parses the command line arguments that were
 **		passed to the program.  The legal arguments are:
 **			-d		"turn off display of samples"
 **			-S [ spherical | elliptical | mixed | automatic ]
 **			-M MinSamples	"min samples per prototype (%)"
 **			-B MaxIllegal	"max illegal chars per cluster (%)"
 **			-I Independence	"0 to 1"
 **			-C Confidence	"1e-200 to 1.0"
 **			-D Directory
 **			-R RoundingAccuracy
 **			-U InputUnicharsetFile
 **			-O OutputUnicharsetFile
 **			-X InputXHeightsFile

 **	Return: none
 **	Exceptions: Illegal options terminate the program.
 **	History: 7/24/89, DSJ, Created.
 */
  int    Option;
  int    ParametersRead;
  BOOL8  Error;

  Error = FALSE;
  while ((Option = tessopt(argc, argv, "F:O:U:R:D:C:I:M:B:S:X:")) != EOF) {
    switch (Option) {
      case 'C':
        ParametersRead = sscanf( tessoptarg, "%lf", &(Config.Confidence) );
        if ( ParametersRead != 1 ) Error = TRUE;
        else if ( Config.Confidence > 1 ) Config.Confidence = 1;
        else if ( Config.Confidence < 0 ) Config.Confidence = 0;
        break;
      case 'I':
        ParametersRead = sscanf( tessoptarg, "%f", &(Config.Independence) );
        if ( ParametersRead != 1 ) Error = TRUE;
        else if ( Config.Independence > 1 ) Config.Independence = 1;
        else if ( Config.Independence < 0 ) Config.Independence = 0;
        break;
      case 'M':
        ParametersRead = sscanf( tessoptarg, "%f", &(Config.MinSamples) );
        if ( ParametersRead != 1 ) Error = TRUE;
        else if ( Config.MinSamples > 1 ) Config.MinSamples = 1;
        else if ( Config.MinSamples < 0 ) Config.MinSamples = 0;
        break;
      case 'B':
        ParametersRead = sscanf( tessoptarg, "%f", &(Config.MaxIllegal) );
        if ( ParametersRead != 1 ) Error = TRUE;
        else if ( Config.MaxIllegal > 1 ) Config.MaxIllegal = 1;
        else if ( Config.MaxIllegal < 0 ) Config.MaxIllegal = 0;
        break;
      case 'R':
        ParametersRead = sscanf( tessoptarg, "%f", &RoundingAccuracy );
        if ( ParametersRead != 1 ) Error = TRUE;
        else if ( RoundingAccuracy > 0.01f ) RoundingAccuracy = 0.01f;
        else if ( RoundingAccuracy < 0.0f ) RoundingAccuracy = 0.0f;
        break;
      case 'S':
        switch ( tessoptarg[0] )
        {
          case 's': Config.ProtoStyle = spherical; break;
          case 'e': Config.ProtoStyle = elliptical; break;
          case 'm': Config.ProtoStyle = mixed; break;
          case 'a': Config.ProtoStyle = automatic; break;
          default: Error = TRUE;
        }
        break;
      case 'D':
        Directory = tessoptarg;
        break;
      case 'U':
        InputUnicharsetFile = tessoptarg;
        break;
      case 'O':
        OutputUnicharsetFile = tessoptarg;
        break;
      case 'F':
        InputFontInfoFile = tessoptarg;
        break;
      case 'X':
        InputXHeightsFile = tessoptarg;
        printf("InputXHeightsFile %s\n", InputXHeightsFile);
        break;
      case '?':
        Error = TRUE;
        break;
    }
    if ( Error )
    {
      fprintf (stderr, "usage: %s [-d] [-p] [-n]\n", argv[0] );
      fprintf (stderr, "\t[-S ProtoStyle]\n");
      fprintf (stderr, "\t[-M MinSamples] [-B MaxBad] [-I Independence]\n");
      fprintf (stderr, "\t[-C Confidence] [-D Directory]\n");
      fprintf (stderr, "\t[-U InputUnicharsetFile] [-O OutputUnicharsetFile]\n");
      fprintf (stderr, "\t[-F FontInfoFile]\n");
      fprintf (stderr, "\t[-X InputXHeightsFile]\n");
      fprintf (stderr, "\t[ TrainingPage ... ]\n");
      exit (2);
    }
  }
}	// ParseArguments

/*---------------------------------------------------------------------------*/
char *GetNextFilename (int Argc, char** Argv)
  /*
   **	Parameters: none
   **	Globals:
   **		tessoptind			defined by tessopt sys call
   **	Operation:
   **		This routine returns the next command line argument.  If
   **		there are no remaining command line arguments, it returns
   **		NULL.  This routine should only be called after all option
   **		arguments have been parsed and removed with ParseArguments.
   **	Return: Next command line argument or NULL.
   **	Exceptions: none
   **	History: Fri Aug 18 09:34:12 1989, DSJ, Created.
   */

{
  if (tessoptind < Argc)
    return (Argv [tessoptind++]);
  else
    return (NULL);

}	/* GetNextFilename */



/*---------------------------------------------------------------------------*/
LABELEDLIST FindList (
    LIST	List,
    char	*Label)

/*
 **	Parameters:
 **		List		list to search
 **		Label		label to search for
 **	Globals: none
 **	Operation:
 **		This routine searches thru a list of labeled lists to find
 **		a list with the specified label.  If a matching labeled list
 **		cannot be found, NULL is returned.
 **	Return: Labeled list with the specified Label or NULL.
 **	Exceptions: none
 **	History: Fri Aug 18 15:57:41 1989, DSJ, Created.
 */

{
  LABELEDLIST	LabeledList;

  iterate (List)
  {
    LabeledList = (LABELEDLIST) first_node (List);
    if (strcmp (LabeledList->Label, Label) == 0)
      return (LabeledList);
  }
  return (NULL);

}	/* FindList */

/*---------------------------------------------------------------------------*/
LABELEDLIST NewLabeledList (
    const char	*Label)

/*
 **	Parameters:
 **		Label	label for new list
 **	Globals: none
 **	Operation:
 **		This routine allocates a new, empty labeled list and gives
 **		it the specified label.
 **	Return: New, empty labeled list.
 **	Exceptions: none
 **	History: Fri Aug 18 16:08:46 1989, DSJ, Created.
 */

{
  LABELEDLIST	LabeledList;

  LabeledList = (LABELEDLIST) Emalloc (sizeof (LABELEDLISTNODE));
  LabeledList->Label = (char*)Emalloc (strlen (Label)+1);
  strcpy (LabeledList->Label, Label);
  LabeledList->List = NIL_LIST;
  LabeledList->SampleCount = 0;
  LabeledList->font_sample_count = 0;
  return (LabeledList);

}	/* NewLabeledList */

/*---------------------------------------------------------------------------*/
void ReadTrainingSamples(const FEATURE_DEFS_STRUCT& feature_defs,
                         const char *feature_name, int max_samples,
                         float linear_spread, float circular_spread,
                         UNICHARSET* unicharset,
                         FILE* file, LIST* training_samples) {
/*
**  Parameters:
**    file    open text file to read samples from
**  Globals: none
**  Operation:
**    This routine reads training samples from a file and
**    places them into a data structure which organizes the
**    samples by FontName and CharName.  It then returns this
**    data structure.
**  Return: none
**  Exceptions: none
**  History: Fri Aug 18 13:11:39 1989, DSJ, Created.
**       Tue May 17 1998 simplifications to structure, illiminated
**        font, and feature specification levels of structure.
*/
  char    unichar[UNICHAR_LEN + 1];
  LABELEDLIST char_sample;
  FEATURE_SET feature_samples;
  CHAR_DESC char_desc;
  int   i;
  int feature_type = ShortNameToFeatureType(feature_defs, feature_name);
  // Description of feature of type feature_type.
  const FEATURE_DESC_STRUCT* f_desc = feature_defs.FeatureDesc[feature_type];

  // Zero out the font_sample_count for all the classes.
  LIST it = *training_samples;
  iterate(it) {
    char_sample = reinterpret_cast<LABELEDLIST>(first_node(it));
    char_sample->font_sample_count = 0;
  }

  while (fscanf(file, "%s %s", CTFontName, unichar) == 2) {
    if (unicharset != NULL && !unicharset->contains_unichar(unichar)) {
      unicharset->unichar_insert(unichar);
      if (unicharset->size() > MAX_NUM_CLASSES) {
        tprintf("Error: Size of unicharset in training is "
                "greater than MAX_NUM_CLASSES\n");
        exit(1);
      }
    }
    char_sample = FindList(*training_samples, unichar);
    if (char_sample == NULL) {
      char_sample = NewLabeledList(unichar);
      *training_samples = push(*training_samples, char_sample);
    }
    char_desc = ReadCharDescription(feature_defs, file);
    feature_samples = char_desc->FeatureSets[feature_type];
    if (char_sample->font_sample_count < max_samples || max_samples <= 0) {
      for (int feature = 0; feature < feature_samples->NumFeatures; ++feature) {
        FEATURE f = feature_samples->Features[feature];
        for (int dim =0; dim < f->Type->NumParams; ++dim)
          f->Params[dim] += f_desc->ParamDesc[dim].Circular
              ? UniformRandomNumber(-circular_spread, circular_spread)
              : UniformRandomNumber(-linear_spread, linear_spread);
      }
      char_sample->List = push(char_sample->List, feature_samples);
      char_sample->SampleCount++;
      char_sample->font_sample_count++;
    } else {
      FreeFeatureSet(feature_samples);
    }
    for (i = 0; i < char_desc->NumFeatureSets; i++) {
      if (feature_type != i)
        FreeFeatureSet(char_desc->FeatureSets[i]);
    }
    free(char_desc);
  }
}  // ReadTrainingSamples

/*---------------------------------------------------------------------------*/
void WriteTrainingSamples (
    const FEATURE_DEFS_STRUCT &FeatureDefs,
    char *Directory,
    LIST CharList,
    const char* program_feature_type)

/*
 **	Parameters:
 **		Directory	directory to place sample files into
 **		FontList	list of fonts used in the training samples
 **	Operation:
 **		This routine writes the specified samples into files which
 **		are organized according to the font name and character name
 **		of the samples.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Aug 18 16:17:06 1989, DSJ, Created.
 */

{
  LABELEDLIST	char_sample;
  FEATURE_SET	FeatureSet;
  LIST		FeatureList;
  FILE		*File;
  char		Filename[MAXNAMESIZE];
  int		NumSamples;

  iterate (CharList)		// iterate thru all of the fonts
  {
    char_sample = (LABELEDLIST) first_node (CharList);

    // construct the full pathname for the current samples file
    strcpy (Filename, "");
    if (Directory != NULL)
    {
      strcat (Filename, Directory);
      strcat (Filename, "/");
    }
    strcat (Filename, CTFontName);
    strcat (Filename, "/");
    strcat (Filename, char_sample->Label);
    strcat (Filename, ".");
    strcat (Filename, program_feature_type);
    printf ("\nWriting %s ...", Filename);

    /* if file does not exist, create a new one with an appropriate
       header; otherwise append samples to the existing file */
    File = fopen (Filename, "r");
    if (File == NULL)
    {
      File = Efopen (Filename, "w");
      WriteOldParamDesc(
          File,
          FeatureDefs.FeatureDesc[ShortNameToFeatureType(
              FeatureDefs, program_feature_type)]);
    }
    else
    {
      fclose (File);
      File = Efopen (Filename, "a");
    }

    // append samples onto the file
    FeatureList = char_sample->List;
    NumSamples = 0;
    iterate (FeatureList)
    {
      FeatureSet = (FEATURE_SET) first_node (FeatureList);
      WriteFeatureSet (File, FeatureSet);
      NumSamples++;
    }
    fclose (File);
  }
}	/* WriteTrainingSamples */

/*---------------------------------------------------------------------------*/
void FreeTrainingSamples (
    LIST	CharList)

/*
 **	Parameters:
 **		FontList	list of all fonts in document
 **	Globals: none
 **	Operation:
 **		This routine deallocates all of the space allocated to
 **		the specified list of training samples.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Aug 18 17:44:27 1989, DSJ, Created.
 */

{
  LABELEDLIST	char_sample;
  FEATURE_SET	FeatureSet;
  LIST		FeatureList;


  // 	printf ("FreeTrainingSamples...\n");
  iterate (CharList) 		/* iterate thru all of the fonts */
  {
    char_sample = (LABELEDLIST) first_node (CharList);
    FeatureList = char_sample->List;
    iterate (FeatureList)	/* iterate thru all of the classes */
    {
      FeatureSet = (FEATURE_SET) first_node (FeatureList);
      FreeFeatureSet (FeatureSet);
    }
    FreeLabeledList (char_sample);
  }
  destroy (CharList);

}	/* FreeTrainingSamples */

/*---------------------------------------------------------------------------*/
void FreeLabeledList (
    LABELEDLIST	LabeledList)

/*
 **	Parameters:
 **		LabeledList	labeled list to be freed
 **	Globals: none
 **	Operation:
 **		This routine deallocates all of the memory consumed by
 **		a labeled list.  It does not free any memory which may be
 **		consumed by the items in the list.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Aug 18 17:52:45 1989, DSJ, Created.
 */

{
  destroy (LabeledList->List);
  free (LabeledList->Label);
  free (LabeledList);

}	/* FreeLabeledList */

/*---------------------------------------------------------------------------*/
CLUSTERER *SetUpForClustering(
    const FEATURE_DEFS_STRUCT &FeatureDefs,
    LABELEDLIST	char_sample,
    const char* program_feature_type)

/*
 **	Parameters:
 **		char_sample: LABELEDLIST that holds all the feature information for a
 **		given character.
 **	Globals:
 **		None
 **	Operation:
 **		This routine reads samples from a LABELEDLIST and enters
 **		those samples into a clusterer data structure.  This
 **		data structure is then returned to the caller.
 **	Return:
 **		Pointer to new clusterer data structure.
 **	Exceptions:
 **		None
 **	History:
 **		8/16/89, DSJ, Created.
 */

{
  uinT16	N;
  int		i, j;
  FLOAT32	*Sample = NULL;
  CLUSTERER	*Clusterer;
  inT32		CharID;
  LIST FeatureList = NULL;
  FEATURE_SET FeatureSet = NULL;

  int desc_index = ShortNameToFeatureType(FeatureDefs, program_feature_type);
  N = FeatureDefs.FeatureDesc[desc_index]->NumParams;
  Clusterer = MakeClusterer(N, FeatureDefs.FeatureDesc[desc_index]->ParamDesc);

  FeatureList = char_sample->List;
  CharID = 0;
  iterate(FeatureList)
  {
    FeatureSet = (FEATURE_SET) first_node (FeatureList);
    for (i=0; i < FeatureSet->MaxNumFeatures; i++)
    {
      if (Sample == NULL)
        Sample = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
      for (j=0; j < N; j++)
        if (RoundingAccuracy != 0.0f)
          Sample[j] = round(FeatureSet->Features[i]->Params[j], RoundingAccuracy);
        else
          Sample[j] = FeatureSet->Features[i]->Params[j];
      MakeSample (Clusterer, Sample, CharID);
    }
    CharID++;
  }
  if ( Sample != NULL ) free( Sample );
  return( Clusterer );

}	/* SetUpForClustering */

/*------------------------------------------------------------------------*/
void MergeInsignificantProtos(LIST ProtoList, const char* label,
                              CLUSTERER	*Clusterer, CLUSTERCONFIG *Config) {
  PROTOTYPE	*Prototype;
  bool debug = strcmp(test_ch, label) == 0;

  LIST pProtoList = ProtoList;
  iterate(pProtoList) {
    Prototype = (PROTOTYPE *) first_node (pProtoList);
    if (Prototype->Significant || Prototype->Merged)
      continue;
    FLOAT32 best_dist = 0.125;
    PROTOTYPE* best_match = NULL;
    // Find the nearest alive prototype.
    LIST list_it = ProtoList;
    iterate(list_it) {
      PROTOTYPE* test_p = (PROTOTYPE *) first_node (list_it);
      if (test_p != Prototype && !test_p->Merged) {
        FLOAT32 dist = ComputeDistance(Clusterer->SampleSize,
                                       Clusterer->ParamDesc,
                                       Prototype->Mean, test_p->Mean);
        if (dist < best_dist) {
          best_match = test_p;
          best_dist = dist;
        }
      }
    }
    if (best_match != NULL && !best_match->Significant) {
      if (debug)
        tprintf("Merging red clusters (%d+%d) at %g,%g and %g,%g\n",
                best_match->NumSamples, Prototype->NumSamples,
                best_match->Mean[0], best_match->Mean[1],
                Prototype->Mean[0], Prototype->Mean[1]);
      best_match->NumSamples = MergeClusters(Clusterer->SampleSize,
                                             Clusterer->ParamDesc,
                                             best_match->NumSamples,
                                             Prototype->NumSamples,
                                             best_match->Mean,
                                             best_match->Mean, Prototype->Mean);
      Prototype->NumSamples = 0;
      Prototype->Merged = 1;
    } else if (best_match != NULL) {
      if (debug)
        tprintf("Red proto at %g,%g matched a green one at %g,%g\n",
                Prototype->Mean[0], Prototype->Mean[1],
                best_match->Mean[0], best_match->Mean[1]);
      Prototype->Merged = 1;
    }
  }
  // Mark significant those that now have enough samples.
  int min_samples = (inT32) (Config->MinSamples * Clusterer->NumChar);
  pProtoList = ProtoList;
  iterate(pProtoList) {
    Prototype = (PROTOTYPE *) first_node (pProtoList);
    // Process insignificant protos that do not match a green one
    if (!Prototype->Significant && Prototype->NumSamples >= min_samples &&
        !Prototype->Merged) {
      if (debug)
        tprintf("Red proto at %g,%g becoming green\n",
                Prototype->Mean[0], Prototype->Mean[1]);
      Prototype->Significant = true;
    }
  }
}	/* MergeInsignificantProtos */

/*-----------------------------------------------------------------------------*/
void CleanUpUnusedData(
    LIST ProtoList)
{
  PROTOTYPE* Prototype;

  iterate(ProtoList)
  {
    Prototype = (PROTOTYPE *) first_node (ProtoList);
    if(Prototype->Variance.Elliptical != NULL)
    {
      memfree(Prototype->Variance.Elliptical);
      Prototype->Variance.Elliptical = NULL;
    }
    if(Prototype->Magnitude.Elliptical != NULL)
    {
      memfree(Prototype->Magnitude.Elliptical);
      Prototype->Magnitude.Elliptical = NULL;
    }
    if(Prototype->Weight.Elliptical != NULL)
    {
      memfree(Prototype->Weight.Elliptical);
      Prototype->Weight.Elliptical = NULL;
    }
  }
}

/*------------------------------------------------------------------------*/
LIST RemoveInsignificantProtos(
    LIST ProtoList,
    BOOL8 KeepSigProtos,
    BOOL8 KeepInsigProtos,
    int N)

{
  LIST NewProtoList = NIL_LIST;
  LIST pProtoList;
  PROTOTYPE* Proto;
  PROTOTYPE* NewProto;
  int i;

  pProtoList = ProtoList;
  iterate(pProtoList)
  {
    Proto = (PROTOTYPE *) first_node (pProtoList);
    if ((Proto->Significant && KeepSigProtos) ||
        (!Proto->Significant && KeepInsigProtos))
    {
      NewProto = (PROTOTYPE *)Emalloc(sizeof(PROTOTYPE));

      NewProto->Mean = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
      NewProto->Significant = Proto->Significant;
      NewProto->Style = Proto->Style;
      NewProto->NumSamples = Proto->NumSamples;
      NewProto->Cluster = NULL;
      NewProto->Distrib = NULL;

      for (i=0; i < N; i++)
        NewProto->Mean[i] = Proto->Mean[i];
      if (Proto->Variance.Elliptical != NULL)
      {
        NewProto->Variance.Elliptical = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
        for (i=0; i < N; i++)
          NewProto->Variance.Elliptical[i] = Proto->Variance.Elliptical[i];
      }
      else
        NewProto->Variance.Elliptical = NULL;
      //---------------------------------------------
      if (Proto->Magnitude.Elliptical != NULL)
      {
        NewProto->Magnitude.Elliptical = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
        for (i=0; i < N; i++)
          NewProto->Magnitude.Elliptical[i] = Proto->Magnitude.Elliptical[i];
      }
      else
        NewProto->Magnitude.Elliptical = NULL;
      //------------------------------------------------
      if (Proto->Weight.Elliptical != NULL)
      {
        NewProto->Weight.Elliptical = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
        for (i=0; i < N; i++)
          NewProto->Weight.Elliptical[i] = Proto->Weight.Elliptical[i];
      }
      else
        NewProto->Weight.Elliptical = NULL;

      NewProto->TotalMagnitude = Proto->TotalMagnitude;
      NewProto->LogMagnitude = Proto->LogMagnitude;
      NewProtoList = push_last(NewProtoList, NewProto);
    }
  }
  //FreeProtoList (ProtoList);
  return (NewProtoList);
}	/* RemoveInsignificantProtos */

/*----------------------------------------------------------------------------*/
MERGE_CLASS FindClass (
    LIST	List,
    char	*Label)
{
  MERGE_CLASS	MergeClass;

  iterate (List)
  {
    MergeClass = (MERGE_CLASS) first_node (List);
    if (strcmp (MergeClass->Label, Label) == 0)
      return (MergeClass);
  }
  return (NULL);

}	/* FindClass */

/*---------------------------------------------------------------------------*/
MERGE_CLASS NewLabeledClass (
    char	*Label)
{
  MERGE_CLASS	MergeClass;

  MergeClass = new MERGE_CLASS_NODE;
  MergeClass->Label = (char*)Emalloc (strlen (Label)+1);
  strcpy (MergeClass->Label, Label);
  MergeClass->Class = NewClass (MAX_NUM_PROTOS, MAX_NUM_CONFIGS);
  return (MergeClass);

}	/* NewLabeledClass */

/*-----------------------------------------------------------------------------*/
void FreeLabeledClassList (
    LIST	ClassList)

/*
 **	Parameters:
 **		FontList	list of all fonts in document
 **	Globals: none
 **	Operation:
 **		This routine deallocates all of the space allocated to
 **		the specified list of training samples.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Aug 18 17:44:27 1989, DSJ, Created.
 */

{
  MERGE_CLASS	MergeClass;

  iterate (ClassList) 		/* iterate thru all of the fonts */
  {
    MergeClass = (MERGE_CLASS) first_node (ClassList);
    free (MergeClass->Label);
    FreeClass(MergeClass->Class);
    delete MergeClass;
  }
  destroy (ClassList);

}	/* FreeLabeledClassList */

/** SetUpForFloat2Int **************************************************/
void SetUpForFloat2Int(const UNICHARSET& unicharset, LIST LabeledClassList) {
  MERGE_CLASS	MergeClass;
  CLASS_TYPE		Class;
  int				NumProtos;
  int				NumConfigs;
  int				NumWords;
  int				i, j;
  float			Values[3];
  PROTO			NewProto;
  PROTO			OldProto;
  BIT_VECTOR		NewConfig;
  BIT_VECTOR		OldConfig;

  // 	printf("Float2Int ...\n");

  iterate(LabeledClassList)
  {
    UnicityTableEqEq<int>   font_set;
    MergeClass = (MERGE_CLASS) first_node (LabeledClassList);
    Class = &TrainingData[unicharset.unichar_to_id(MergeClass->Label)];
    NumProtos = MergeClass->Class->NumProtos;
    NumConfigs = MergeClass->Class->NumConfigs;
    font_set.move(&MergeClass->Class->font_set);
    Class->NumProtos = NumProtos;
    Class->MaxNumProtos = NumProtos;
    Class->Prototypes = (PROTO) Emalloc (sizeof(PROTO_STRUCT) * NumProtos);
    for(i=0; i < NumProtos; i++)
    {
      NewProto = ProtoIn(Class, i);
      OldProto = ProtoIn(MergeClass->Class, i);
      Values[0] = OldProto->X;
      Values[1] = OldProto->Y;
      Values[2] = OldProto->Angle;
      Normalize(Values);
      NewProto->X = OldProto->X;
      NewProto->Y = OldProto->Y;
      NewProto->Length = OldProto->Length;
      NewProto->Angle = OldProto->Angle;
      NewProto->A = Values[0];
      NewProto->B = Values[1];
      NewProto->C = Values[2];
    }

    Class->NumConfigs = NumConfigs;
    Class->MaxNumConfigs = NumConfigs;
    Class->font_set.move(&font_set);
    Class->Configurations = (BIT_VECTOR*) Emalloc (sizeof(BIT_VECTOR) * NumConfigs);
    NumWords = WordsInVectorOfSize(NumProtos);
    for(i=0; i < NumConfigs; i++)
    {
      NewConfig = NewBitVector(NumProtos);
      OldConfig = MergeClass->Class->Configurations[i];
      for(j=0; j < NumWords; j++)
        NewConfig[j] = OldConfig[j];
      Class->Configurations[i] = NewConfig;
    }
  }
} // SetUpForFloat2Int

/*--------------------------------------------------------------------------*/
void Normalize (
    float  *Values)
{
  register float Slope;
  register float Intercept;
  register float Normalizer;

  Slope      = tan (Values [2] * 2 * PI);
  Intercept  = Values [1] - Slope * Values [0];
  Normalizer = 1 / sqrt (Slope * Slope + 1.0);

  Values [0] = Slope * Normalizer;
  Values [1] = - Normalizer;
  Values [2] = Intercept * Normalizer;
} // Normalize

/*-------------------------------------------------------------------------*/
void FreeNormProtoList (
    LIST	CharList)

{
  LABELEDLIST	char_sample;

  iterate (CharList) 		/* iterate thru all of the fonts */
  {
    char_sample = (LABELEDLIST) first_node (CharList);
    FreeLabeledList (char_sample);
  }
  destroy (CharList);

}	// FreeNormProtoList

/*---------------------------------------------------------------------------*/
void AddToNormProtosList(
    LIST* NormProtoList,
    LIST ProtoList,
    char* CharName)
{
  PROTOTYPE* Proto;
  LABELEDLIST LabeledProtoList;

  LabeledProtoList = NewLabeledList(CharName);
  iterate(ProtoList)
  {
    Proto = (PROTOTYPE *) first_node (ProtoList);
    LabeledProtoList->List = push(LabeledProtoList->List, Proto);
  }
  *NormProtoList = push(*NormProtoList, LabeledProtoList);
}

/*---------------------------------------------------------------------------*/
int NumberOfProtos(
    LIST ProtoList,
    BOOL8	CountSigProtos,
    BOOL8	CountInsigProtos)
{
  int N = 0;
  PROTOTYPE	*Proto;

  iterate(ProtoList)
  {
    Proto = (PROTOTYPE *) first_node ( ProtoList );
    if (( Proto->Significant && CountSigProtos )	||
        ( ! Proto->Significant && CountInsigProtos ) )
      N++;
  }
  return(N);
}
