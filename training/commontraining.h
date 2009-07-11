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

#ifndef TESSERACT_TRAINING_COMMONTRAINING_H__
#define TESSERACT_TRAINING_COMMONTRAINING_H__

#include "oldlist.h"
#include "cluster.h"
#include "intproto.h"


//////////////////////////////////////////////////////////////////////////////
// Macros ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define MAXNAMESIZE     80
#define MINSD_ANGLE     (1.0f / 64.0f)


//////////////////////////////////////////////////////////////////////////////
// Globals ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
extern BOOL8 ShowSignificantProtos;
extern BOOL8 ShowInsignificantProtos;
extern BOOL8 ShowAllSamples;

// Must be defined in the file that "implements" commonTraining facilities.
extern CLUSTERCONFIG Config;
extern FLOAT32 RoundingAccuracy;

extern char CTFontName[MAXNAMESIZE];
// globals used for parsing command line arguments
extern char *Directory;

extern const char* test_ch;

extern const char *InputUnicharsetFile;
extern const char *OutputUnicharsetFile;

extern const char *InputFontInfoFile;

// The unicharset used during training
extern UNICHARSET unicharset_training;

//////////////////////////////////////////////////////////////////////////////
// Structs ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
  char  *Label;
  int   SampleCount;
  LIST  List;
}
LABELEDLISTNODE, *LABELEDLIST;

typedef struct
{
  char* Label;
  int   NumMerged[MAX_NUM_PROTOS];
  CLASS_TYPE Class;
}MERGE_CLASS_NODE;
typedef MERGE_CLASS_NODE* MERGE_CLASS;


//////////////////////////////////////////////////////////////////////////////
// Functions /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ParseArguments(
    int         argc,
    char        **argv);

char *GetNextFilename(int Argc, char** argv);

LABELEDLIST FindList(
    LIST        List,
    char        *Label);

LABELEDLIST NewLabeledList(
    const char  *Label);

void WriteTrainingSamples(
    char        *Directory,
    LIST        CharList,
    const char  *program_feature_type);

void FreeTrainingSamples(
    LIST        CharList);

void FreeLabeledList(
    LABELEDLIST LabeledList);

void FreeLabeledClassList(
    LIST        ClassListList);

CLUSTERER *SetUpForClustering(
    LABELEDLIST CharSample,
    const char  *program_feature_type);

LIST RemoveInsignificantProtos(
    LIST        ProtoList,
    BOOL8       KeepSigProtos,
    BOOL8       KeepInsigProtos,
    int         N);

void CleanUpUnusedData(
    LIST        ProtoList);

void MergeInsignificantProtos(
    LIST        ProtoList,
    const char  *label,
    CLUSTERER   *Clusterer,
    CLUSTERCONFIG *Config);

MERGE_CLASS FindClass(
    LIST        List,
    char        *Label);

MERGE_CLASS NewLabeledClass(
    char        *Label);

void FreeTrainingSamples(
    LIST        CharList);

void SetUpForFloat2Int(
    LIST        LabeledClassList);

void Normalize(
    float       *Values);

void FreeNormProtoList(
    LIST        CharList);

void AddToNormProtosList(
    LIST*       NormProtoList,
    LIST        ProtoList,
    char        *CharName);

int NumberOfProtos(
    LIST        ProtoList,
    BOOL8       CountSigProtos,
    BOOL8       CountInsigProtos);


void allocNormProtos();
#endif  // TESSERACT_TRAINING_COMMONTRAINING_H__
