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

#include "cluster.h"
#include "commandlineflags.h"
#include "featdefs.h"
#include "intproto.h"
#include "oldlist.h"

namespace tesseract {
class Classify;
class MasterTrainer;
class ShapeTable;
}

//////////////////////////////////////////////////////////////////////////////
// Globals ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

extern FEATURE_DEFS_STRUCT feature_defs;

// Must be defined in the file that "implements" commonTraining facilities.
extern CLUSTERCONFIG Config;

//////////////////////////////////////////////////////////////////////////////
// Structs ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
  char  *Label;
  int   SampleCount;
  int   font_sample_count;
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
void ParseArguments(int* argc, char*** argv);

namespace tesseract {

// Helper loads shape table from the given file.
ShapeTable* LoadShapeTable(const STRING& file_prefix);
// Helper to write the shape_table.
void WriteShapeTable(const STRING& file_prefix, const ShapeTable& shape_table);

// Creates a MasterTraininer and loads the training data into it:
// Initializes feature_defs and IntegerFX.
// Loads the shape_table if shape_table != NULL.
// Loads initial unicharset from -U command-line option.
// If FLAGS_input_trainer is set, loads the majority of data from there, else:
//   Loads font info from -F option.
//   Loads xheights from -X option.
//   Loads samples from .tr files in remaining command-line args.
//   Deletes outliers and computes canonical samples.
//   If FLAGS_output_trainer is set, saves the trainer for future use.
// Computes canonical and cloud features.
// If shape_table is not NULL, but failed to load, make a fake flat one,
// as shape clustering was not run.
MasterTrainer* LoadTrainingData(int argc, const char* const * argv,
                                bool replication,
                                ShapeTable** shape_table,
                                STRING* file_prefix);
}  // namespace tesseract.

const char *GetNextFilename(int argc, const char* const * argv);

LABELEDLIST FindList(
    LIST        List,
    char        *Label);

LABELEDLIST NewLabeledList(
    const char  *Label);

void ReadTrainingSamples(const FEATURE_DEFS_STRUCT& feature_defs,
                         const char *feature_name, int max_samples,
                         UNICHARSET* unicharset,
                         FILE* file, LIST* training_samples);

void WriteTrainingSamples(
    const FEATURE_DEFS_STRUCT &FeatureDefs,
    char *Directory,
    LIST CharList,
    const char  *program_feature_type);

void FreeTrainingSamples(
    LIST        CharList);

void FreeLabeledList(
    LABELEDLIST LabeledList);

void FreeLabeledClassList(
    LIST        ClassListList);

CLUSTERER *SetUpForClustering(
    const FEATURE_DEFS_STRUCT &FeatureDefs,
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
    const char        *Label);

MERGE_CLASS NewLabeledClass(
    const char        *Label);

void FreeTrainingSamples(
    LIST        CharList);

CLASS_STRUCT* SetUpForFloat2Int(const UNICHARSET& unicharset,
                                LIST LabeledClassList);

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
