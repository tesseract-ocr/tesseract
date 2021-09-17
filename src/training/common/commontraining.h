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

#ifndef TESSERACT_TRAINING_COMMONTRAINING_H_
#define TESSERACT_TRAINING_COMMONTRAINING_H_

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "commandlineflags.h"
#include "export.h"
#include "tprintf.h"

#include <tesseract/baseapi.h>

#include <memory>

namespace tesseract {

TESS_COMMON_TRAINING_API
void ParseArguments(int *argc, char ***argv);

// Check whether the shared tesseract library is the right one.
// This function must be inline because otherwise it would be part of
// the shared library, so it could not compare the versions.
static inline void CheckSharedLibraryVersion() {
#ifdef HAVE_CONFIG_H
  if (!!strcmp(TESSERACT_VERSION_STR, TessBaseAPI::Version())) {
    tprintf(
        "ERROR: shared library version mismatch (was %s, expected %s\n"
        "Did you use a wrong shared tesseract library?\n",
        TessBaseAPI::Version(), TESSERACT_VERSION_STR);
    exit(1);
  }
#endif
}

} // namespace tesseract

#ifndef DISABLED_LEGACY_ENGINE

#  include "cluster.h"
#  include "featdefs.h"
#  include "intproto.h"
#  include "oldlist.h"

namespace tesseract {

class Classify;
class MasterTrainer;
class ShapeTable;

//////////////////////////////////////////////////////////////////////////////
// Globals ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TESS_COMMON_TRAINING_API
extern FEATURE_DEFS_STRUCT feature_defs;

// Must be defined in the file that "implements" commonTraining facilities.
TESS_COMMON_TRAINING_API
extern CLUSTERCONFIG Config;

//////////////////////////////////////////////////////////////////////////////
// Structs ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
struct LABELEDLISTNODE {
  /// This constructor allocates a new, empty labeled list and gives
  /// it the specified label.
  /// @param Label label for new list
  LABELEDLISTNODE(const char *label) : Label(label) {
  }
  std::string Label;
  int SampleCount = 0;
  int font_sample_count = 0;
  LIST List = nullptr;
};
using LABELEDLIST = LABELEDLISTNODE *;

struct MERGE_CLASS_NODE {
  MERGE_CLASS_NODE(const char * label) : Label(label), Class(NewClass(MAX_NUM_PROTOS, MAX_NUM_CONFIGS)) {
  }
  std::string Label;
  int NumMerged[MAX_NUM_PROTOS];
  tesseract::CLASS_TYPE Class;
};
using MERGE_CLASS = MERGE_CLASS_NODE *;

//////////////////////////////////////////////////////////////////////////////
// Functions /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Helper loads shape table from the given file.
ShapeTable *LoadShapeTable(const std::string &file_prefix);
// Helper to write the shape_table.
TESS_COMMON_TRAINING_API
void WriteShapeTable(const std::string &file_prefix, const ShapeTable &shape_table);

// Creates a MasterTraininer and loads the training data into it:
// Initializes feature_defs and IntegerFX.
// Loads the shape_table if shape_table != nullptr.
// Loads initial unicharset from -U command-line option.
// If FLAGS_input_trainer is set, loads the majority of data from there, else:
//   Loads font info from -F option.
//   Loads xheights from -X option.
//   Loads samples from .tr files in remaining command-line args.
//   Deletes outliers and computes canonical samples.
//   If FLAGS_output_trainer is set, saves the trainer for future use.
// Computes canonical and cloud features.
// If shape_table is not nullptr, but failed to load, make a fake flat one,
// as shape clustering was not run.
TESS_COMMON_TRAINING_API
std::unique_ptr<MasterTrainer> LoadTrainingData(const char *const *filelist, bool replication,
                                                ShapeTable **shape_table, std::string &file_prefix);

LABELEDLIST FindList(tesseract::LIST List, const std::string &Label);

TESS_COMMON_TRAINING_API
void ReadTrainingSamples(const tesseract::FEATURE_DEFS_STRUCT &feature_defs,
                         const char *feature_name, int max_samples,
                         tesseract::UNICHARSET *unicharset, FILE *file,
                         tesseract::LIST *training_samples);

void WriteTrainingSamples(const tesseract::FEATURE_DEFS_STRUCT &FeatureDefs, char *Directory,
                          tesseract::LIST CharList, const char *program_feature_type);

TESS_COMMON_TRAINING_API
void FreeTrainingSamples(tesseract::LIST CharList);

TESS_COMMON_TRAINING_API
void FreeLabeledList(LABELEDLIST LabeledList);

TESS_COMMON_TRAINING_API
void FreeLabeledClassList(tesseract::LIST ClassListList);

TESS_COMMON_TRAINING_API
tesseract::CLUSTERER *SetUpForClustering(const tesseract::FEATURE_DEFS_STRUCT &FeatureDefs,
                                         LABELEDLIST CharSample, const char *program_feature_type);

TESS_COMMON_TRAINING_API
tesseract::LIST RemoveInsignificantProtos(tesseract::LIST ProtoList, bool KeepSigProtos,
                                          bool KeepInsigProtos, int N);

TESS_COMMON_TRAINING_API
void CleanUpUnusedData(tesseract::LIST ProtoList);

TESS_COMMON_TRAINING_API
void MergeInsignificantProtos(tesseract::LIST ProtoList, const char *label,
                              tesseract::CLUSTERER *Clusterer, tesseract::CLUSTERCONFIG *Config);

TESS_COMMON_TRAINING_API
MERGE_CLASS FindClass(tesseract::LIST List, const std::string &Label);

TESS_COMMON_TRAINING_API
tesseract::CLASS_STRUCT *SetUpForFloat2Int(const tesseract::UNICHARSET &unicharset,
                                           tesseract::LIST LabeledClassList);

void Normalize(float *Values);

TESS_COMMON_TRAINING_API
void FreeNormProtoList(tesseract::LIST CharList);

TESS_COMMON_TRAINING_API
void AddToNormProtosList(tesseract::LIST *NormProtoList, tesseract::LIST ProtoList, const std::string &CharName);

TESS_COMMON_TRAINING_API
int NumberOfProtos(tesseract::LIST ProtoList, bool CountSigProtos, bool CountInsigProtos);

void allocNormProtos();

} // namespace tesseract

#endif // def DISABLED_LEGACY_ENGINE

#endif // TESSERACT_TRAINING_COMMONTRAINING_H_
