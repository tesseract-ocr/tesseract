/******************************************************************************
**	Filename:	mfTraining.c
**	Purpose:	Separates training pages into files for each character.
**				Strips from files only the features and there parameters of
				the feature type mf.
**	Author:		Dan Johnson
**	Revisment:	Christy Russon
**	Environment: HPUX 6.5
**	Library:     HPUX 6.5
**	History:     Fri Aug 18 08:53:50 1989, DSJ, Created.
**		     5/25/90, DSJ, Adapted to multiple feature types.
**				Tuesday, May 17, 1998 Changes made to make feature specific and
**				simplify structures. First step in simplifying training process.
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
#include "oldlist.h"
#include "efio.h"
#include "emalloc.h"
#include "featdefs.h"
#include "tessopt.h"
#include "ocrfeatures.h"
#include "mf.h"
#include "general.h"
#include "clusttool.h"
#include "cluster.h"
#include "protos.h"
#include "minmax.h"
#include "debug.h"
#include "tprintf.h"
#include "const.h"
#include "mergenf.h"
#include "name2char.h"
#include "intproto.h"
#include "variables.h"
#include "freelist.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"

#include <string.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#ifdef WIN32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#endif

#define MAXNAMESIZE	80
#define MAX_NUM_SAMPLES	10000
#define PROGRAM_FEATURE_TYPE "mf"
#define MINSD (1.0f / 128.0f)
#define MINSD_ANGLE (1.0f / 64.0f)

int	row_number;						/* cjn: fixes link problem */

typedef struct
{
  char		*Label;
  int       SampleCount;
  LIST		List;
}
LABELEDLISTNODE, *LABELEDLIST;

typedef struct
{
	char* Label;
	int	NumMerged[MAX_NUM_PROTOS];
	CLASS_TYPE Class;
}MERGE_CLASS_NODE;
typedef MERGE_CLASS_NODE* MERGE_CLASS;

#define round(x,frag)(floor(x/frag+.5)*frag)

/**----------------------------------------------------------------------------
					Public Function Prototypes
----------------------------------------------------------------------------**/
int main (
     int	argc,
     char	**argv);

/**----------------------------------------------------------------------------
					Private Function Prototypes
----------------------------------------------------------------------------**/
void ParseArguments(
int	argc,
char	**argv);

char *GetNextFilename ();

LIST ReadTrainingSamples (
     FILE	*File);

LABELEDLIST FindList (
     LIST	List,
     char	*Label);

MERGE_CLASS FindClass (
     LIST	List,
     char	*Label);

LABELEDLIST NewLabeledList (
     char	*Label);

MERGE_CLASS NewLabeledClass (
     char	*Label);

void WriteTrainingSamples (
     char	*Directory,
     LIST	CharList);

void WriteClusteredTrainingSamples (
     char	*Directory,
     LIST	ProtoList,
	 CLUSTERER *Clusterer,
	 LABELEDLIST CharSample);
/**/
void WriteMergedTrainingSamples(
    char	*Directory,
	LIST ClassList);

void WriteMicrofeat(
    char	*Directory,
	LIST	ClassList);

void WriteProtos(
	FILE* File,
	MERGE_CLASS MergeClass);

void WriteConfigs(
	FILE* File,
	CLASS_TYPE Class);

void FreeTrainingSamples (
     LIST	CharList);

void FreeLabeledClassList (
     LIST	ClassList);

void FreeLabeledList (
     LABELEDLIST	LabeledList);

CLUSTERER *SetUpForClustering(
     LABELEDLIST	CharSample);
/*
PARAMDESC *ConvertToPARAMDESC(
	PARAM_DESC* Param_Desc,
	int N);
*/
void MergeInsignificantProtos(LIST ProtoList, const char* label,
                              CLUSTERER	*Clusterer, CLUSTERCONFIG *Config);

LIST RemoveInsignificantProtos(
	LIST ProtoList,
	BOOL8 KeepSigProtos,
	BOOL8 KeepInsigProtos,
	int N);

void CleanUpUnusedData(
	LIST ProtoList);

void Normalize (
   float  *Values);

void SetUpForFloat2Int(
	LIST LabeledClassList);

void WritePFFMTable(INT_TEMPLATES Templates, const char* filename);

//--------------Global Data Definitions and Declarations--------------
static char FontName[MAXNAMESIZE];
// globals used for parsing command line arguments
static char	*Directory = NULL;
static int	MaxNumSamples = MAX_NUM_SAMPLES;
static int	Argc;
static char	**Argv;

// globals used to control what information is saved in the output file
static BOOL8		ShowAllSamples = FALSE;
static BOOL8		ShowSignificantProtos = TRUE;
static BOOL8		ShowInsignificantProtos = FALSE;

// global variable to hold configuration parameters to control clustering
// -M 0.40   -B 0.05   -I 1.0   -C 1e-6.
static CLUSTERCONFIG Config =
{ elliptical, 0.625, 0.05, 1.0, 1e-6, 0 };

static FLOAT32 RoundingAccuracy = 0.0f;

// The unicharset used during mftraining
static UNICHARSET unicharset_mftraining;

const char* test_ch = "";

/*----------------------------------------------------------------------------
						Public Code
-----------------------------------------------------------------------------*/
void DisplayProtoList(const char* ch, LIST protolist) {
  void* window = c_create_window("Char samples", 50, 200,
                                 520, 520, -130.0, 130.0, -130.0, 130.0);
  LIST proto = protolist;
  iterate(proto) {
    PROTOTYPE* prototype = reinterpret_cast<PROTOTYPE *>(first_node(proto));
    if (prototype->Significant)
      c_line_color_index(window, Green);
    else if (prototype->NumSamples == 0)
      c_line_color_index(window, Blue);
    else if (prototype->Merged)
      c_line_color_index(window, Magenta);
    else
      c_line_color_index(window, Red);
    float x = CenterX(prototype->Mean);
    float y = CenterY(prototype->Mean);
    double angle = OrientationOf(prototype->Mean) * 2 * M_PI;
    float dx = static_cast<float>(LengthOf(prototype->Mean) * cos(angle) / 2);
    float dy = static_cast<float>(LengthOf(prototype->Mean) * sin(angle) / 2);
    c_move(window, (x - dx) * 256, (y - dy) * 256);
    c_draw(window, (x + dx) * 256, (y + dy) * 256);
    if (prototype->Significant)
      tprintf("Green proto at (%g,%g)+(%g,%g) %d samples\n",
              x, y, dx, dy, prototype->NumSamples);
    else if (prototype->NumSamples > 0 && !prototype->Merged)
      tprintf("Red proto at (%g,%g)+(%g,%g) %d samples\n",
              x, y, dx, dy, prototype->NumSamples);
  }
  c_make_current(window);
}

/*---------------------------------------------------------------------------*/
int main (int argc, char **argv) {
/*
**	Parameters:
**		argc	number of command line arguments
**		argv	array of command line arguments
**	Globals: none
**	Operation:
**		This program reads in a text file consisting of feature
**		samples from a training page in the following format:
**
**			FontName CharName NumberOfFeatureTypes(N)
**			   FeatureTypeName1 NumberOfFeatures(M)
**			      Feature1
**			      ...
**			      FeatureM
**			   FeatureTypeName2 NumberOfFeatures(M)
**			      Feature1
**			      ...
**			      FeatureM
**			   ...
**			   FeatureTypeNameN NumberOfFeatures(M)
**			      Feature1
**			      ...
**			      FeatureM
**			FontName CharName ...
**
**		The result of this program is a binary inttemp file used by
**		the OCR engine.
**	Return: none
**	Exceptions: none
**	History:	Fri Aug 18 08:56:17 1989, DSJ, Created.
**				Mon May 18 1998, Christy Russson, Revistion started.
*/
  char	*PageName;
  FILE	*TrainingPage;
  FILE	*OutFile;
  LIST	CharList;
  CLUSTERER	*Clusterer = NULL;
  LIST		ProtoList = NIL;
  LABELEDLIST CharSample;
  PROTOTYPE	*Prototype;
  LIST   	ClassList = NIL;
  int		Cid, Pid;
  PROTO		Proto;
  PROTO_STRUCT	DummyProto;
  BIT_VECTOR	Config2;
  MERGE_CLASS	MergeClass;
  INT_TEMPLATES	IntTemplates;
  LIST pCharList, pProtoList;
  char Filename[MAXNAMESIZE];

  // Clean the unichar set
  unicharset_mftraining.clear();
  // Space character needed to represent NIL classification
  unicharset_mftraining.unichar_insert(" ");

  ParseArguments (argc, argv);
  InitFastTrainerVars ();
  InitSubfeatureVars ();
  while ((PageName = GetNextFilename()) != NULL) {
    printf ("Reading %s ...\n", PageName);
    TrainingPage = Efopen (PageName, "r");
    CharList = ReadTrainingSamples (TrainingPage);
    fclose (TrainingPage);
    //WriteTrainingSamples (Directory, CharList);
    pCharList = CharList;
    iterate(pCharList) {
      //Cluster
      CharSample = (LABELEDLIST) first_node (pCharList);
//    printf ("\nClustering %s ...", CharSample->Label);
      Clusterer = SetUpForClustering(CharSample);
      Config.MagicSamples = CharSample->SampleCount;
      ProtoList = ClusterSamples(Clusterer, &Config);
      CleanUpUnusedData(ProtoList);

      //Merge
      MergeInsignificantProtos(ProtoList, CharSample->Label,
                               Clusterer, &Config);
      if (strcmp(test_ch, CharSample->Label) == 0)
        DisplayProtoList(test_ch, ProtoList);
      ProtoList = RemoveInsignificantProtos(ProtoList, ShowSignificantProtos,
                                            ShowInsignificantProtos,
                                            Clusterer->SampleSize);
      FreeClusterer(Clusterer);
      MergeClass = FindClass (ClassList, CharSample->Label);
      if (MergeClass == NULL) {
        MergeClass = NewLabeledClass (CharSample->Label);
        ClassList = push (ClassList, MergeClass);
      }
      Cid = AddConfigToClass(MergeClass->Class);
      pProtoList = ProtoList;
      iterate (pProtoList) {
        Prototype = (PROTOTYPE *) first_node (pProtoList);

        // see if proto can be approximated by existing proto
        Pid = FindClosestExistingProto(MergeClass->Class,
                                       MergeClass->NumMerged, Prototype);
        if (Pid == NO_PROTO) {
          Pid = AddProtoToClass (MergeClass->Class);
          Proto = ProtoIn (MergeClass->Class, Pid);
          MakeNewFromOld (Proto, Prototype);
          MergeClass->NumMerged[Pid] = 1;
        }
        else {
          MakeNewFromOld (&DummyProto, Prototype);
          ComputeMergedProto (ProtoIn (MergeClass->Class, Pid), &DummyProto,
              (FLOAT32) MergeClass->NumMerged[Pid], 1.0,
              ProtoIn (MergeClass->Class, Pid));
          MergeClass->NumMerged[Pid] ++;
        }
        Config2 = MergeClass->Class->Configurations[Cid];
        AddProtoToConfig (Pid, Config2);
      }
      FreeProtoList (&ProtoList);
    }
    FreeTrainingSamples (CharList);
  }
  //WriteMergedTrainingSamples(Directory,ClassList);
  WriteMicrofeat(Directory, ClassList);
  InitIntProtoVars ();
  InitPrototypes ();
  SetUpForFloat2Int(ClassList);
  IntTemplates = CreateIntTemplates(TrainingData, unicharset_mftraining);
  strcpy (Filename, "");
  if (Directory != NULL) {
    strcat (Filename, Directory);
    strcat (Filename, "/");
  }
  strcat (Filename, "inttemp");
#ifdef __UNIX__
  OutFile = Efopen (Filename, "w");
#else
  OutFile = Efopen (Filename, "wb");
#endif
  WriteIntTemplates(OutFile, IntTemplates, unicharset_mftraining);
  fclose (OutFile);
  strcpy (Filename, "");
  if (Directory != NULL) {
    strcat (Filename, Directory);
    strcat (Filename, "/");
  }
  strcat (Filename, "pffmtable");
  // Now create pffmtable.
  WritePFFMTable(IntTemplates, Filename);
  printf ("Done!\n"); /**/
  FreeLabeledClassList (ClassList);
  return 0;
}	/* main */


/**----------------------------------------------------------------------------
							Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ParseArguments(
int	argc,
char	**argv)

/*
**	Parameters:
**		argc	number of command line arguments to parse
**		argv	command line arguments
**	Globals:
**		ShowAllSamples		flag controlling samples display
**		ShowSignificantProtos	flag controlling proto display
**		ShowInsignificantProtos	flag controlling proto display
**		Config			current clustering parameters
**		tessoptarg, tessoptind		defined by tessopt sys call
**		Argc, Argv		global copies of argc and argv
**	Operation:
**		This routine parses the command line arguments that were
**		passed to the program.  The legal arguments are:
**			-d		"turn off display of samples"
**			-p		"turn off significant protos"
**			-n		"turn off insignificant proto"
**			-S [ spherical | elliptical | mixed | automatic ]
**			-M MinSamples	"min samples per prototype (%)"
**			-B MaxIllegal	"max illegal chars per cluster (%)"
**			-I Independence	"0 to 1"
**			-C Confidence	"1e-200 to 1.0"
**			-D Directory
**			-N MaxNumSamples
**			-R RoundingAccuracy
**	Return: none
**	Exceptions: Illegal options terminate the program.
**	History: 7/24/89, DSJ, Created.
*/

{
	int		Option;
	int		ParametersRead;
	BOOL8		Error;

	Error = FALSE;
	Argc = argc;
	Argv = argv;
	while (( Option = tessopt( argc, argv, "R:N:D:C:I:M:B:S:d:n:p" )) != EOF )
	{
		switch ( Option )
		{
		case 'n':
			ShowInsignificantProtos = FALSE;
			break;
		case 'p':
			ShowSignificantProtos = FALSE;
			break;
		case 'd':
			ShowAllSamples = FALSE;
			break;
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
			case 'N':
				if (sscanf (tessoptarg, "%d", &MaxNumSamples) != 1 ||
					MaxNumSamples <= 0)
					Error = TRUE;
				break;
			case '?':
				Error = TRUE;
				break;
		}
		if ( Error )
		{
			fprintf (stderr, "usage: %s [-D] [-P] [-N]\n", argv[0] );
			fprintf (stderr, "\t[-S ProtoStyle]\n");
			fprintf (stderr, "\t[-M MinSamples] [-B MaxBad] [-I Independence] [-C Confidence]\n" );
			fprintf (stderr, "\t[-d directory] [-n MaxNumSamples] [ TrainingPage ... ]\n");
			exit (2);
		}
	}
}	// ParseArguments

/*---------------------------------------------------------------------------*/
char *GetNextFilename ()
/*
**	Parameters: none
**	Globals:
**		tessoptind			defined by tessopt sys call
**		Argc, Argv		global copies of argc and argv
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
LIST ReadTrainingSamples (
     FILE	*File)

/*
**	Parameters:
**		File		open text file to read samples from
**	Globals: none
**	Operation:
**		This routine reads training samples from a file and
**		places them into a data structure which organizes the
**		samples by FontName and CharName.  It then returns this
**		data structure.
**	Return: none
**	Exceptions: none
**	History: Fri Aug 18 13:11:39 1989, DSJ, Created.
**			 Tue May 17 1998 simplifications to structure, illiminated
**				font, and feature specification levels of structure.
*/

{
	char			unichar[UNICHAR_LEN + 1];
	LABELEDLIST             CharSample;
        FEATURE_SET             FeatureSamples;
	LIST			TrainingSamples = NIL;
	CHAR_DESC		CharDesc;
	int			Type, i;

	while (fscanf (File, "%s %s", FontName, unichar) == 2) {
          if (!unicharset_mftraining.contains_unichar(unichar)) {
            unicharset_mftraining.unichar_insert(unichar);
            if (unicharset_mftraining.size() > MAX_NUM_CLASSES) {
              cprintf("Error: Size of unicharset of mftraining is "
                      "greater than MAX_NUM_CLASSES\n");
              exit(1);
            }
          }
		CharSample = FindList (TrainingSamples, unichar);
		if (CharSample == NULL) {
			CharSample = NewLabeledList (unichar);
			TrainingSamples = push (TrainingSamples, CharSample);
		}
		CharDesc = ReadCharDescription (File);
		Type = ShortNameToFeatureType(PROGRAM_FEATURE_TYPE);
		FeatureSamples = CharDesc->FeatureSets[Type];
                for (int feature = 0; feature < FeatureSamples->NumFeatures; ++feature) {
                  FEATURE f = FeatureSamples->Features[feature];
                  for (int dim =0; dim < f->Type->NumParams; ++dim)
                    f->Params[dim] += dim == MFDirection ?
                                    UniformRandomNumber(-MINSD_ANGLE, MINSD_ANGLE) :
                                    UniformRandomNumber(-MINSD, MINSD);
                }
		CharSample->List = push (CharSample->List, FeatureSamples);
        CharSample->SampleCount++;
		for (i = 0; i < CharDesc->NumFeatureSets; i++)
                  if (Type != i)
                    FreeFeatureSet(CharDesc->FeatureSets[i]);
		free (CharDesc);
        }
	return (TrainingSamples);

}	/* ReadTrainingSamples */

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
LABELEDLIST NewLabeledList (
     char	*Label)

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
	LabeledList->List = NIL;
    LabeledList->SampleCount = 0;
	return (LabeledList);

}	/* NewLabeledList */

/*---------------------------------------------------------------------------*/
MERGE_CLASS NewLabeledClass (
     char	*Label)
{
	MERGE_CLASS	MergeClass;

	MergeClass = (MERGE_CLASS) Emalloc (sizeof (MERGE_CLASS_NODE));
	MergeClass->Label = (char*)Emalloc (strlen (Label)+1);
	strcpy (MergeClass->Label, Label);
	MergeClass->Class = NewClass (MAX_NUM_PROTOS, MAX_NUM_CONFIGS);
	return (MergeClass);

}	/* NewLabeledClass */

/*---------------------------------------------------------------------------*/
void WriteTrainingSamples (
     char	*Directory,
     LIST	CharList)

/*
**	Parameters:
**		Directory	directory to place sample files into
**		FontList	list of fonts used in the training samples
**	Globals:
**		MaxNumSamples	max number of samples per class to write
**	Operation:
**		This routine writes the specified samples into files which
**		are organized according to the font name and character name
**		of the samples.
**	Return: none
**	Exceptions: none
**	History: Fri Aug 18 16:17:06 1989, DSJ, Created.
*/

{
	LABELEDLIST	CharSample;
	FEATURE_SET	FeatureSet;
	LIST		FeatureList;
	FILE		*File;
	char		Filename[MAXNAMESIZE];
	int		NumSamples;

	iterate (CharList)		// iterate thru all of the fonts
	{
		CharSample = (LABELEDLIST) first_node (CharList);

		// construct the full pathname for the current samples file
		strcpy (Filename, "");
		if (Directory != NULL)
		{
			strcat (Filename, Directory);
			strcat (Filename, "/");
		}
		strcat (Filename, FontName);
		strcat (Filename, "/");
		strcat (Filename, CharSample->Label);
		strcat (Filename, ".");
		strcat (Filename, PROGRAM_FEATURE_TYPE);
		printf ("\nWriting %s ...", Filename);

		/* if file does not exist, create a new one with an appropriate
		header; otherwise append samples to the existing file */
		File = fopen (Filename, "r");
		if (File == NULL)
		{
			File = Efopen (Filename, "w");
			WriteOldParamDesc
				(File, FeatureDefs.FeatureDesc[ShortNameToFeatureType (PROGRAM_FEATURE_TYPE)]);
		}
		else
		{
			fclose (File);
			File = Efopen (Filename, "a");
		}

		// append samples onto the file
		FeatureList = CharSample->List;
		NumSamples = 0;
		iterate (FeatureList)
		{
			if (NumSamples >= MaxNumSamples) break;

			FeatureSet = (FEATURE_SET) first_node (FeatureList);
			WriteFeatureSet (File, FeatureSet);
			NumSamples++;
		}
		fclose (File);
	}
}	/* WriteTrainingSamples */


/*----------------------------------------------------------------------------*/
void WriteClusteredTrainingSamples (
     char	*Directory,
     LIST	ProtoList,
	 CLUSTERER *Clusterer,
	 LABELEDLIST CharSample)

/*
**	Parameters:
**		Directory	directory to place sample files into
**	Globals:
**		MaxNumSamples	max number of samples per class to write
**	Operation:
**		This routine writes the specified samples into files which
**		are organized according to the font name and character name
**		of the samples.
**	Return: none
**	Exceptions: none
**	History: Fri Aug 18 16:17:06 1989, DSJ, Created.
*/

{
	FILE		*File;
	char		Filename[MAXNAMESIZE];

	strcpy (Filename, "");
	if (Directory != NULL)
	{
		strcat (Filename, Directory);
		strcat (Filename, "/");
	}
	strcat (Filename, FontName);
	strcat (Filename, "/");
	strcat (Filename, CharSample->Label);
	strcat (Filename, ".");
	strcat (Filename, PROGRAM_FEATURE_TYPE);
	strcat (Filename, ".p");
	printf ("\nWriting %s ...", Filename);
	File = Efopen (Filename, "w");
	WriteProtoList(File, Clusterer->SampleSize, Clusterer->ParamDesc,
		ProtoList, ShowSignificantProtos, ShowInsignificantProtos);
	fclose (File);

}	/* WriteClusteredTrainingSamples */

/*---------------------------------------------------------------------------*/
void WriteMergedTrainingSamples(
    char	*Directory,
	LIST ClassList)

{
	FILE		*File;
	char		Filename[MAXNAMESIZE];
	MERGE_CLASS MergeClass;

	iterate (ClassList)
	{
		MergeClass = (MERGE_CLASS) first_node (ClassList);
		strcpy (Filename, "");
		if (Directory != NULL)
		{
			strcat (Filename, Directory);
			strcat (Filename, "/");
		}
		strcat (Filename, "Merged/");
		strcat (Filename, MergeClass->Label);
		strcat (Filename, PROTO_SUFFIX);
		printf ("\nWriting Merged %s ...", Filename);
		File = Efopen (Filename, "w");
		WriteOldProtoFile (File, MergeClass->Class);
		fclose (File);

		strcpy (Filename, "");
		if (Directory != NULL)
		{
			strcat (Filename, Directory);
			strcat (Filename, "/");
		}
		strcat (Filename, "Merged/");
		strcat (Filename, MergeClass->Label);
		strcat (Filename, CONFIG_SUFFIX);
		printf ("\nWriting Merged %s ...", Filename);
		File = Efopen (Filename, "w");
		WriteOldConfigFile (File, MergeClass->Class);
		fclose (File);
	}

}	// WriteMergedTrainingSamples

/*--------------------------------------------------------------------------*/
void WriteMicrofeat(
    char	*Directory,
	LIST	ClassList)

{
	FILE		*File;
	char		Filename[MAXNAMESIZE];
	MERGE_CLASS MergeClass;

	strcpy (Filename, "");
	if (Directory != NULL)
	{
		strcat (Filename, Directory);
		strcat (Filename, "/");
	}
	strcat (Filename, "Microfeat");
	File = Efopen (Filename, "w");
	printf ("\nWriting Merged %s ...", Filename);
	iterate(ClassList)
	{
		MergeClass = (MERGE_CLASS) first_node (ClassList);
		WriteProtos(File, MergeClass);
		WriteConfigs(File, MergeClass->Class);
	}
	fclose (File);
} // WriteMicrofeat

/*---------------------------------------------------------------------------*/
void WriteProtos(
	FILE* File,
	MERGE_CLASS MergeClass)
{
	float Values[3];
	int i;
	PROTO Proto;

	fprintf(File, "%s\n", MergeClass->Label);
	fprintf(File, "%d\n", MergeClass->Class->NumProtos);
	for(i=0; i < (MergeClass->Class)->NumProtos; i++)
	{
		Proto = ProtoIn(MergeClass->Class,i);
		fprintf(File, "\t%8.4f %8.4f %8.4f %8.4f ", Proto->X, Proto->Y,
			Proto->Length, Proto->Angle);
		Values[0] = Proto->X;
		Values[1] = Proto->Y;
		Values[2] = Proto->Angle;
		Normalize(Values);
		fprintf(File, "%8.4f %8.4f %8.4f\n", Values[0], Values[1], Values[2]);
	}
} // WriteProtos

/*----------------------------------------------------------------------------*/
void WriteConfigs(
	FILE* File,
	CLASS_TYPE Class)
{
	BIT_VECTOR Config;
	int i, j, WordsPerConfig;

	WordsPerConfig = WordsInVectorOfSize(Class->NumProtos);
	fprintf(File, "%d %d\n", Class->NumConfigs, WordsPerConfig);
	for(i=0; i < Class->NumConfigs; i++)
	{
		Config = Class->Configurations[i];
		for(j=0; j < WordsPerConfig; j++)
			fprintf(File, "%08x ", Config[j]);
		fprintf(File, "\n");
	}
	fprintf(File, "\n");
} // WriteConfigs

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
	LABELEDLIST	CharSample;
	FEATURE_SET	FeatureSet;
	LIST		FeatureList;


// 	printf ("FreeTrainingSamples...\n");
	iterate (CharList) 		/* iterate thru all of the fonts */
	{
		CharSample = (LABELEDLIST) first_node (CharList);
		FeatureList = CharSample->List;
		iterate (FeatureList)	/* iterate thru all of the classes */
		{
			FeatureSet = (FEATURE_SET) first_node (FeatureList);
			FreeFeatureSet (FeatureSet);
		}
		FreeLabeledList (CharSample);
	}
	destroy (CharList);

}	/* FreeTrainingSamples */

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
		free (MergeClass);
	}
	destroy (ClassList);

}	/* FreeLabeledClassList */

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
     LABELEDLIST	CharSample)

/*
**	Parameters:
**		CharSample: LABELEDLIST that holds all the feature information for a
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
	FEATURE_DESC FeatureDesc = NULL;
//	PARAM_DESC* ParamDesc;

	FeatureDesc = FeatureDefs.FeatureDesc[ShortNameToFeatureType(PROGRAM_FEATURE_TYPE)];
	N = FeatureDesc->NumParams;
//	ParamDesc = ConvertToPARAMDESC(FeatureDesc->ParamDesc, N);
	Clusterer = MakeClusterer(N,FeatureDesc->ParamDesc);
//	free(ParamDesc);

	FeatureList = CharSample->List;
	CharID = 0;
	iterate(FeatureList)
	{
		if (CharID >= MaxNumSamples) break;

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

/*------------------------------------------------------------------------*/
LIST RemoveInsignificantProtos(
	LIST ProtoList,
	BOOL8 KeepSigProtos,
	BOOL8 KeepInsigProtos,
	int N)

{
	LIST NewProtoList = NIL;
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

/** SetUpForFloat2Int **************************************************/
void SetUpForFloat2Int(
	LIST LabeledClassList)
{
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
		MergeClass = (MERGE_CLASS) first_node (LabeledClassList);
		Class = &TrainingData[unicharset_mftraining.unichar_to_id(
                                          MergeClass->Label)];
		NumProtos = (MergeClass->Class)->NumProtos;
		NumConfigs = MergeClass->Class->NumConfigs;

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
void WritePFFMTable(INT_TEMPLATES Templates, const char* filename) {
  FILE* fp = Efopen(filename, "wb");
  /* then write out each class */
  for (int i = 0; i < Templates->NumClasses; i++) {
    int MaxLength = 0;
    INT_CLASS Class = Templates->Class[i];
    for (int ConfigId = 0; ConfigId < Class->NumConfigs; ConfigId++) {
      if (Class->ConfigLengths[ConfigId] > MaxLength)
        MaxLength = Class->ConfigLengths[ConfigId];
    }
    fprintf(fp, "%s %d\n", unicharset_mftraining.id_to_unichar(
                Templates->ClassIdFor[i]), MaxLength);
  }
  fclose(fp);
} // WritePFFMTable
