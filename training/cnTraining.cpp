/******************************************************************************
**	Filename:	cnTraining.cpp
**	Purpose:	Generates a normproto and pffmtable.
**	Author:		Dan Johnson
**	Revisment:	Christy Russon
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
#include "general.h"
#include "clusttool.h"
#include "cluster.h"
#include "name2char.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "unichar.h"

#define MAXNAMESIZE	80
#define MAX_NUM_SAMPLES	10000
#define PROGRAM_FEATURE_TYPE "cn"
#define MINSD (1.0f / 64.0f)

int	row_number;						/* cjn: fixes link problem */

typedef struct
{
  char		*Label;
  int       SampleCount;
  LIST		List;
}
LABELEDLISTNODE, *LABELEDLIST;

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

void ReadTrainingSamples (
     FILE	*File,
	 LIST* TrainingSamples);

LABELEDLIST FindList (
     LIST	List,
     char	*Label);

LABELEDLIST NewLabeledList (
     char	*Label);

void WriteTrainingSamples (
     char	*Directory,
     LIST	CharList);

void WriteNormProtos (
     char	*Directory,
     LIST	LabeledProtoList,
	 CLUSTERER *Clusterer);

void FreeTrainingSamples (
     LIST	CharList);

void FreeNormProtoList (
     LIST	CharList);

void FreeLabeledList (
     LABELEDLIST	LabeledList);

CLUSTERER *SetUpForClustering(
     LABELEDLIST	CharSample);
/*
PARAMDESC *ConvertToPARAMDESC(
	PARAM_DESC* Param_Desc,
	int N);
*/
void AddToNormProtosList(
	LIST* NormProtoList,
	LIST ProtoList,
	char* CharName);

void WriteProtos(
     FILE	*File,
     uinT16	N,
     LIST	ProtoList,
     BOOL8	WriteSigProtos,
     BOOL8	WriteInsigProtos);

int NumberOfProtos(
	LIST ProtoList,
    BOOL8	CountSigProtos,
    BOOL8	CountInsigProtos);

/**----------------------------------------------------------------------------
		  		Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
static char FontName[MAXNAMESIZE];
/* globals used for parsing command line arguments */
static char	*Directory = NULL;
static int	MaxNumSamples = MAX_NUM_SAMPLES;
static int	Argc;
static char	**Argv;

/* globals used to control what information is saved in the output file */
static BOOL8		ShowAllSamples = FALSE;
static BOOL8		ShowSignificantProtos = TRUE;
static BOOL8		ShowInsignificantProtos = FALSE;

/* global variable to hold configuration parameters to control clustering */
//-M 0.025   -B 0.05   -I 0.8   -C 1e-3
static CLUSTERCONFIG	Config =
{
  elliptical, 0.025, 0.05, 0.8, 1e-3, 0
};

static FLOAT32 RoundingAccuracy = 0.0;

/**----------------------------------------------------------------------------
							Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
int main (
     int	argc,
     char	**argv)

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
**		It then appends these samples into a separate file for each
**		character.  The name of the file is
**
**			DirectoryName/FontName/CharName.FeatureTypeName
**
**		The DirectoryName can be specified via a command
**		line argument.  If not specified, it defaults to the
**		current directory.  The format of the resulting files is:
**
**			NumberOfFeatures(M)
**			   Feature1
**			   ...
**			   FeatureM
**			NumberOfFeatures(M)
**			...
**
**		The output files each have a header which describes the
**		type of feature which the file contains.  This header is
**		in the format required by the clusterer.  A command line
**		argument can also be used to specify that only the first
**		N samples of each class should be used.
**	Return: none
**	Exceptions: none
**	History: Fri Aug 18 08:56:17 1989, DSJ, Created.
*/

{
	char	*PageName;
	FILE	*TrainingPage;
	LIST	CharList = NIL;
	CLUSTERER	*Clusterer = NULL;
	LIST		ProtoList = NIL;
	LIST		NormProtoList = NIL;
	LIST pCharList;
	LABELEDLIST CharSample;

	ParseArguments (argc, argv);
	while ((PageName = GetNextFilename()) != NULL)
	{
		printf ("Reading %s ...\n", PageName);
		TrainingPage = Efopen (PageName, "r");
		ReadTrainingSamples (TrainingPage, &CharList);
		fclose (TrainingPage);
		//WriteTrainingSamples (Directory, CharList);
	}
        printf("Clustering ...\n");
	pCharList = CharList;
	iterate(pCharList)
	{
          //Cluster
          CharSample = (LABELEDLIST) first_node (pCharList);
          //printf ("\nClustering %s ...", CharSample->Label);
          Clusterer = SetUpForClustering(CharSample);
          float SavedMinSamples = Config.MinSamples;
          Config.MagicSamples = CharSample->SampleCount;
          while (Config.MinSamples > 0.001) {
            ProtoList = ClusterSamples(Clusterer, &Config);
            if (NumberOfProtos(ProtoList, 1, 0) > 0)
              break;
            else {
              Config.MinSamples *= 0.95;
              printf("0 significant protos for %s."
                     " Retrying clustering with MinSamples = %f%%\n",
                     CharSample->Label, Config.MinSamples);
            }
          }
          Config.MinSamples = SavedMinSamples;
          AddToNormProtosList(&NormProtoList, ProtoList, CharSample->Label);
	}
	FreeTrainingSamples (CharList);
	WriteNormProtos (Directory, NormProtoList, Clusterer);
	FreeClusterer(Clusterer);
	FreeProtoList(&ProtoList);
	FreeNormProtoList(NormProtoList);
	printf ("\n");
  return 0;
}	// main


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
      sscanf(tessoptarg,"%d", &ParametersRead);
			ShowInsignificantProtos = ParametersRead;
			break;
		case 'p':
      sscanf(tessoptarg,"%d", &ParametersRead);
			ShowSignificantProtos = ParametersRead;
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
			else if ( RoundingAccuracy > 0.01 ) RoundingAccuracy = 0.01;
			else if ( RoundingAccuracy < 0.0 ) RoundingAccuracy = 0.0;
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
}	/* ParseArguments */

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
void ReadTrainingSamples (
     FILE	*File,
	 LIST* TrainingSamples)

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
	char		unichar[UNICHAR_LEN + 1];
	LABELEDLIST	CharSample;
	FEATURE_SET	FeatureSamples;
	CHAR_DESC	CharDesc;
	int		Type, i;

	while (fscanf (File, "%s %s", FontName, unichar) == 2) {
          CharSample = FindList (*TrainingSamples, unichar);
          if (CharSample == NULL) {
            CharSample = NewLabeledList (unichar);
            *TrainingSamples = push (*TrainingSamples, CharSample);
          }
          CharDesc = ReadCharDescription (File);
          Type = ShortNameToFeatureType(PROGRAM_FEATURE_TYPE);
          FeatureSamples = CharDesc->FeatureSets[Type];
          for (int feature = 0; feature < FeatureSamples->NumFeatures; ++feature) {
            FEATURE f = FeatureSamples->Features[feature];
            for (int dim =0; dim < f->Type->NumParams; ++dim)
              f->Params[dim] += UniformRandomNumber(-MINSD, MINSD);
          }
          CharSample->List = push (CharSample->List, FeatureSamples);
          CharSample->SampleCount++;
          for (i = 0; i < CharDesc->NumFeatureSets; i++)
            if (Type != i)
              FreeFeatureSet(CharDesc->FeatureSets[i]);
          free (CharDesc);
        }
}	// ReadTrainingSamples

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

	LabeledList = (LABELEDLIST) (char*)Emalloc (sizeof (LABELEDLISTNODE));
	LabeledList->Label = (char*)Emalloc (strlen (Label)+1);
	strcpy (LabeledList->Label, Label);
	LabeledList->List = NIL;
    LabeledList->SampleCount = 0;
	return (LabeledList);

}	/* NewLabeledList */

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
		strcat (Filename, "Merged");
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
			//if (NumSamples >= MaxNumSamples) break;

			FeatureSet = (FEATURE_SET) first_node (FeatureList);
			WriteFeatureSet (File, FeatureSet);
			NumSamples++;
		}
		fclose (File);
	}
}	/* WriteTrainingSamples */


/*----------------------------------------------------------------------------*/
void WriteNormProtos (
     char	*Directory,
     LIST	LabeledProtoList,
	 CLUSTERER *Clusterer)

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
	LABELEDLIST LabeledProto;
	int N;

	strcpy (Filename, "");
	if (Directory != NULL)
	{
		strcat (Filename, Directory);
		strcat (Filename, "/");
	}
	strcat (Filename, "normproto");
	printf ("\nWriting %s ...", Filename);
	File = Efopen (Filename, "w");
	fprintf(File,"%0d\n",Clusterer->SampleSize);
	WriteParamDesc(File,Clusterer->SampleSize,Clusterer->ParamDesc);
	iterate(LabeledProtoList)
	{
		LabeledProto = (LABELEDLIST) first_node (LabeledProtoList);
		N = NumberOfProtos(LabeledProto->List,
		ShowSignificantProtos, ShowInsignificantProtos);
                if (N < 1) {
                  printf ("\nError! Not enough protos for %s: %d protos"
                          " (%d significant protos"
                          ", %d insignificant protos)\n",
                          LabeledProto->Label, N,
                          NumberOfProtos(LabeledProto->List, 1, 0),
                          NumberOfProtos(LabeledProto->List, 0, 1));
                  exit(1);
                }
		fprintf(File, "\n%s %d\n", LabeledProto->Label, N);
		WriteProtos(File, Clusterer->SampleSize, LabeledProto->List,
			ShowSignificantProtos, ShowInsignificantProtos);
	}
	fclose (File);

}	// WriteNormProtos

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


	printf ("\nFreeTrainingSamples...");
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

/*-------------------------------------------------------------------------*/
void FreeNormProtoList (
     LIST	CharList)

{
	LABELEDLIST	CharSample;

	iterate (CharList) 		/* iterate thru all of the fonts */
	{
		CharSample = (LABELEDLIST) first_node (CharList);
		FreeLabeledList (CharSample);
	}
	destroy (CharList);

}	// FreeNormProtoList

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
	//ParamDesc = ConvertToPARAMDESC(FeatureDesc->ParamDesc, N);
	Clusterer = MakeClusterer(N,FeatureDesc->ParamDesc);
//	free(ParamDesc);

	FeatureList = CharSample->List;
	CharID = 0;
	iterate(FeatureList)
	{
		FeatureSet = (FEATURE_SET) first_node (FeatureList);
		for (i=0; i < FeatureSet->MaxNumFeatures; i++)
		{
			if (Sample == NULL)
				Sample = (FLOAT32 *)Emalloc(N * sizeof(FLOAT32));
			for (j=0; j < N; j++)
				if (RoundingAccuracy != 0.0)
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

/*-------------------------------------------------------------------------*/
void WriteProtos(
     FILE	*File,
     uinT16	N,
     LIST	ProtoList,
     BOOL8	WriteSigProtos,
     BOOL8	WriteInsigProtos)
{
	PROTOTYPE	*Proto;

	// write prototypes
	iterate(ProtoList)
	{
		Proto = (PROTOTYPE *) first_node ( ProtoList );
		if (( Proto->Significant && WriteSigProtos )	||
			( ! Proto->Significant && WriteInsigProtos ) )
			WritePrototype( File, N, Proto );
	}
}	// WriteProtos

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
