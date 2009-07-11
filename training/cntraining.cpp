/******************************************************************************
**	Filename:	cntraining.cpp
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
#include "commontraining.h"

#define PROGRAM_FEATURE_TYPE "cn"
#define MINSD (1.0f / 64.0f)

int	row_number;						/* cjn: fixes link problem */

/**----------------------------------------------------------------------------
					Public Function Prototypes
----------------------------------------------------------------------------**/
int main (
     int	argc,
     char	**argv);

/**----------------------------------------------------------------------------
					Private Function Prototypes
----------------------------------------------------------------------------**/
void ReadTrainingSamples (
     FILE	*File,
	 LIST* TrainingSamples);

void WriteNormProtos (
     char	*Directory,
     LIST	LabeledProtoList,
	 CLUSTERER *Clusterer);

/*
PARAMDESC *ConvertToPARAMDESC(
	PARAM_DESC* Param_Desc,
	int N);
*/

void WriteProtos(
     FILE	*File,
     uinT16	N,
     LIST	ProtoList,
     BOOL8	WriteSigProtos,
     BOOL8	WriteInsigProtos);

/**----------------------------------------------------------------------------
		  		Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* global variable to hold configuration parameters to control clustering */
//-M 0.025   -B 0.05   -I 0.8   -C 1e-3
CLUSTERCONFIG	Config =
{
  elliptical, 0.025, 0.05, 0.8, 1e-3, 0
};


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
	while ((PageName = GetNextFilename(argc, argv)) != NULL)
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
          Clusterer = SetUpForClustering(CharSample, PROGRAM_FEATURE_TYPE);
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
        if (Clusterer == NULL) // To avoid a SIGSEGV
          return 1;
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

	while (fscanf (File, "%s %s", CTFontName, unichar) == 2) {
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

/*----------------------------------------------------------------------------*/
void WriteNormProtos (
     char	*Directory,
     LIST	LabeledProtoList,
	 CLUSTERER *Clusterer)

/*
**	Parameters:
**		Directory	directory to place sample files into
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
