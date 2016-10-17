/******************************************************************************
**  Filename:  cntraining.cpp
**  Purpose:  Generates a normproto and pffmtable.
**  Author:    Dan Johnson
**  Revisment:  Christy Russon
**  History:     Fri Aug 18 08:53:50 1989, DSJ, Created.
**         5/25/90, DSJ, Adapted to multiple feature types.
**        Tuesday, May 17, 1998 Changes made to make feature specific and
**        simplify structures. First step in simplifying training process.
**
 **  (c) Copyright Hewlett-Packard Company, 1988.
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


/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "oldlist.h"
#include "efio.h"
#include "emalloc.h"
#include "featdefs.h"
#include "tessopt.h"
#include "ocrfeatures.h"
#include "clusttool.h"
#include "cluster.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "unichar.h"
#include "commontraining.h"

#define PROGRAM_FEATURE_TYPE "cn"

DECLARE_STRING_PARAM_FLAG(D);

/*----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------*/
int main (
     int  argc,
     char  **argv);

/*----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------*/

void WriteNormProtos (
     const char  *Directory,
     LIST  LabeledProtoList,
   CLUSTERER *Clusterer);

/*
PARAMDESC *ConvertToPARAMDESC(
  PARAM_DESC* Param_Desc,
  int N);
*/

void WriteProtos(
     FILE  *File,
     uinT16  N,
     LIST  ProtoList,
     BOOL8  WriteSigProtos,
     BOOL8  WriteInsigProtos);

/*----------------------------------------------------------------------------
          Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
/* global variable to hold configuration parameters to control clustering */
//-M 0.025   -B 0.05   -I 0.8   -C 1e-3
CLUSTERCONFIG  CNConfig =
{
  elliptical, 0.025, 0.05, 0.8, 1e-3, 0
};


/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
* This program reads in a text file consisting of feature
* samples from a training page in the following format:
* @verbatim
   FontName CharName NumberOfFeatureTypes(N)
      FeatureTypeName1 NumberOfFeatures(M)
         Feature1
         ...
         FeatureM
      FeatureTypeName2 NumberOfFeatures(M)
         Feature1
         ...
         FeatureM
      ...
      FeatureTypeNameN NumberOfFeatures(M)
         Feature1
         ...
         FeatureM
   FontName CharName ...
@endverbatim
* It then appends these samples into a separate file for each
* character.  The name of the file is
*
*   DirectoryName/FontName/CharName.FeatureTypeName
*
* The DirectoryName can be specified via a command
* line argument.  If not specified, it defaults to the
* current directory.  The format of the resulting files is:
* @verbatim
   NumberOfFeatures(M)
      Feature1
      ...
      FeatureM
   NumberOfFeatures(M)
   ...
@endverbatim
* The output files each have a header which describes the
* type of feature which the file contains.  This header is
* in the format required by the clusterer.  A command line
* argument can also be used to specify that only the first
* N samples of each class should be used.
* @param argc  number of command line arguments
* @param argv  array of command line arguments
* @return none
* @note Globals: none
* @note Exceptions: none
* @note History: Fri Aug 18 08:56:17 1989, DSJ, Created.
*/
int main(int  argc, char* argv[])
{
  // Set the global Config parameters before parsing the command line.
  Config = CNConfig;

  const char  *PageName;
  FILE  *TrainingPage;
  LIST  CharList = NIL_LIST;
  CLUSTERER  *Clusterer = NULL;
  LIST    ProtoList = NIL_LIST;
  LIST    NormProtoList = NIL_LIST;
  LIST pCharList;
  LABELEDLIST CharSample;
  FEATURE_DEFS_STRUCT FeatureDefs;
  InitFeatureDefs(&FeatureDefs);

  ParseArguments(&argc, &argv);
  int num_fonts = 0;
  while ((PageName = GetNextFilename(argc, argv)) != NULL) {
    printf("Reading %s ...\n", PageName);
    TrainingPage = Efopen(PageName, "rb");
    ReadTrainingSamples(FeatureDefs, PROGRAM_FEATURE_TYPE,
                        100, NULL, TrainingPage, &CharList);
    fclose(TrainingPage);
    ++num_fonts;
  }
  printf("Clustering ...\n");
  // To allow an individual font to form a separate cluster,
  // reduce the min samples:
  // Config.MinSamples = 0.5 / num_fonts;
  pCharList = CharList;
  iterate(pCharList) {
    //Cluster
    if (Clusterer)
       FreeClusterer(Clusterer);
    CharSample = (LABELEDLIST)first_node(pCharList);
    Clusterer =
      SetUpForClustering(FeatureDefs, CharSample, PROGRAM_FEATURE_TYPE);
    float SavedMinSamples = Config.MinSamples;
    // To disable the tendency to produce a single cluster for all fonts,
    // make MagicSamples an impossible to achieve number:
    // Config.MagicSamples = CharSample->SampleCount * 10;
    Config.MagicSamples = CharSample->SampleCount;
    while (Config.MinSamples > 0.001) {
      ProtoList = ClusterSamples(Clusterer, &Config);
      if (NumberOfProtos(ProtoList, 1, 0) > 0) {
        break;
      } else {
        Config.MinSamples *= 0.95;
        printf("0 significant protos for %s."
               " Retrying clustering with MinSamples = %f%%\n",
               CharSample->Label, Config.MinSamples);
      }
    }
    Config.MinSamples = SavedMinSamples;
    AddToNormProtosList(&NormProtoList, ProtoList, CharSample->Label);
  }
  FreeTrainingSamples(CharList);
  if (Clusterer == NULL) { // To avoid a SIGSEGV
    fprintf(stderr, "Error: NULL clusterer!\n");
    return 1;
  }
  WriteNormProtos(FLAGS_D.c_str(), NormProtoList, Clusterer);
  FreeNormProtoList(NormProtoList);
  FreeProtoList(&ProtoList);
  FreeClusterer(Clusterer);
  printf ("\n");
  return 0;
}  // main


/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
* This routine writes the specified samples into files which
* are organized according to the font name and character name
* of the samples.
* @param Directory  directory to place sample files into
* @param LabeledProtoList List of labeled protos
* @param Clusterer The CLUSTERER to use
* @return none
* @note Exceptions: none
* @note History: Fri Aug 18 16:17:06 1989, DSJ, Created.
*/
void WriteNormProtos (
     const char  *Directory,
     LIST  LabeledProtoList,
     CLUSTERER *Clusterer)
{
  FILE    *File;
  STRING Filename;
  LABELEDLIST LabeledProto;
  int N;

  Filename = "";
  if (Directory != NULL && Directory[0] != '\0')
  {
    Filename += Directory;
    Filename += "/";
  }
  Filename += "normproto";
  printf ("\nWriting %s ...", Filename.string());
  File = Efopen (Filename.string(), "wb");
  fprintf(File,"%0d\n",Clusterer->SampleSize);
  WriteParamDesc(File,Clusterer->SampleSize,Clusterer->ParamDesc);
  iterate(LabeledProtoList)
  {
    LabeledProto = (LABELEDLIST) first_node (LabeledProtoList);
    N = NumberOfProtos(LabeledProto->List, true, false);
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
    WriteProtos(File, Clusterer->SampleSize, LabeledProto->List, true, false);
  }
  fclose (File);

}  // WriteNormProtos

/*-------------------------------------------------------------------------*/
void WriteProtos(
     FILE  *File,
     uinT16  N,
     LIST  ProtoList,
     BOOL8  WriteSigProtos,
     BOOL8  WriteInsigProtos)
{
  PROTOTYPE  *Proto;

  // write prototypes
  iterate(ProtoList)
  {
    Proto = (PROTOTYPE *) first_node ( ProtoList );
    if (( Proto->Significant && WriteSigProtos )  ||
      ( ! Proto->Significant && WriteInsigProtos ) )
      WritePrototype( File, N, Proto );
  }
}  // WriteProtos
