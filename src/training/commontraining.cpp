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

#define _USE_MATH_DEFINES       // for M_PI
#include "commontraining.h"
#include <algorithm>
#include <cmath>                // for M_PI

#ifdef DISABLED_LEGACY_ENGINE

#include "params.h"
#include "tessopt.h"
#include "tprintf.h"

INT_PARAM_FLAG(debug_level, 0, "Level of Trainer debugging");
INT_PARAM_FLAG(load_images, 0, "Load images with tr files");
STRING_PARAM_FLAG(configfile, "", "File to load more configs from");
STRING_PARAM_FLAG(D, "", "Directory to write output files to");
STRING_PARAM_FLAG(F, "font_properties", "File listing font properties");
STRING_PARAM_FLAG(X, "", "File listing font xheights");
STRING_PARAM_FLAG(U, "unicharset", "File to load unicharset from");
STRING_PARAM_FLAG(O, "", "File to write unicharset to");
STRING_PARAM_FLAG(output_trainer, "", "File to write trainer to");
STRING_PARAM_FLAG(test_ch, "", "UTF8 test character string");

/**
 * This routine parses the command line arguments that were
 * passed to the program and uses them to set relevant
 * training-related global parameters.
 *
 * Globals:
 * - Config  current clustering parameters
 * @param argc number of command line arguments to parse
 * @param argv command line arguments
 * @note Exceptions: Illegal options terminate the program.
 */
void ParseArguments(int* argc, char ***argv) {
  STRING usage;
  if (*argc) {
    usage += (*argv)[0];
    usage += " -v | --version | ";
    usage += (*argv)[0];
  }
  usage += " [.tr files ...]";
  tesseract::ParseCommandLineFlags(usage.c_str(), argc, argv, true);
}

#else

#include "allheaders.h"
#include "ccutil.h"
#include "classify.h"
#include "cluster.h"
#include "clusttool.h"
#include "emalloc.h"
#include "featdefs.h"
#include "fontinfo.h"
#include "intfeaturespace.h"
#include "mastertrainer.h"
#include "mf.h"
#include "oldlist.h"
#include "params.h"
#include "shapetable.h"
#include "tessdatamanager.h"
#include "tessopt.h"
#include "tprintf.h"
#include "unicity_table.h"

using tesseract::CCUtil;
using tesseract::IntFeatureSpace;
using tesseract::ParamUtils;
using tesseract::ShapeTable;

// Global Variables.

// global variable to hold configuration parameters to control clustering
// -M 0.625   -B 0.05   -I 1.0   -C 1e-6.
CLUSTERCONFIG Config = { elliptical, 0.625, 0.05, 1.0, 1e-6, 0 };
FEATURE_DEFS_STRUCT feature_defs;
static CCUtil ccutil;

INT_PARAM_FLAG(debug_level, 0, "Level of Trainer debugging");
static INT_PARAM_FLAG(load_images, 0, "Load images with tr files");
static STRING_PARAM_FLAG(configfile, "", "File to load more configs from");
STRING_PARAM_FLAG(D, "", "Directory to write output files to");
STRING_PARAM_FLAG(F, "font_properties", "File listing font properties");
STRING_PARAM_FLAG(X, "", "File listing font xheights");
STRING_PARAM_FLAG(U, "unicharset", "File to load unicharset from");
STRING_PARAM_FLAG(O, "", "File to write unicharset to");
STRING_PARAM_FLAG(output_trainer, "", "File to write trainer to");
STRING_PARAM_FLAG(test_ch, "", "UTF8 test character string");
static DOUBLE_PARAM_FLAG(clusterconfig_min_samples_fraction, Config.MinSamples,
                         "Min number of samples per proto as % of total");
static DOUBLE_PARAM_FLAG(clusterconfig_max_illegal, Config.MaxIllegal,
                         "Max percentage of samples in a cluster which have more"
                         " than 1 feature in that cluster");
static DOUBLE_PARAM_FLAG(clusterconfig_independence, Config.Independence,
                         "Desired independence between dimensions");
static DOUBLE_PARAM_FLAG(clusterconfig_confidence, Config.Confidence,
                         "Desired confidence in prototypes created");

/**
 * This routine parses the command line arguments that were
 * passed to the program and uses them to set relevant
 * training-related global parameters.
 *
 * Globals:
 * - Config  current clustering parameters
 * @param argc number of command line arguments to parse
 * @param argv command line arguments
 */
void ParseArguments(int* argc, char ***argv) {
  STRING usage;
  if (*argc) {
    usage += (*argv)[0];
    usage += " -v | --version | ";
    usage += (*argv)[0];
  }
  usage += " [.tr files ...]";
  tesseract::ParseCommandLineFlags(usage.c_str(), argc, argv, true);
  // Record the index of the first non-flag argument to 1, since we set
  // remove_flags to true when parsing the flags.
  tessoptind = 1;
  // Set some global values based on the flags.
  Config.MinSamples =
          std::max(0.0, std::min(1.0, double(FLAGS_clusterconfig_min_samples_fraction)));
  Config.MaxIllegal =
          std::max(0.0, std::min(1.0, double(FLAGS_clusterconfig_max_illegal)));
  Config.Independence =
          std::max(0.0, std::min(1.0, double(FLAGS_clusterconfig_independence)));
  Config.Confidence =
          std::max(0.0, std::min(1.0, double(FLAGS_clusterconfig_confidence)));
  // Set additional parameters from config file if specified.
  if (!FLAGS_configfile.empty()) {
    tesseract::ParamUtils::ReadParamsFile(
        FLAGS_configfile.c_str(),
        tesseract::SET_PARAM_CONSTRAINT_NON_INIT_ONLY,
        ccutil.params());
  }
}

namespace tesseract {
// Helper loads shape table from the given file.
ShapeTable* LoadShapeTable(const STRING& file_prefix) {
  ShapeTable* shape_table = nullptr;
  STRING shape_table_file = file_prefix;
  shape_table_file += kShapeTableFileSuffix;
  TFile shape_fp;
  if (shape_fp.Open(shape_table_file.c_str(), nullptr)) {
    shape_table = new ShapeTable;
    if (!shape_table->DeSerialize(&shape_fp)) {
      delete shape_table;
      shape_table = nullptr;
      tprintf("Error: Failed to read shape table %s\n",
              shape_table_file.c_str());
    } else {
      int num_shapes = shape_table->NumShapes();
      tprintf("Read shape table %s of %d shapes\n",
              shape_table_file.c_str(), num_shapes);
    }
  } else {
    tprintf("Warning: No shape table file present: %s\n",
            shape_table_file.c_str());
  }
  return shape_table;
}

// Helper to write the shape_table.
void WriteShapeTable(const STRING& file_prefix, const ShapeTable& shape_table) {
  STRING shape_table_file = file_prefix;
  shape_table_file += kShapeTableFileSuffix;
  FILE* fp = fopen(shape_table_file.c_str(), "wb");
  if (fp != nullptr) {
    if (!shape_table.Serialize(fp)) {
      fprintf(stderr, "Error writing shape table: %s\n",
              shape_table_file.c_str());
    }
    fclose(fp);
  } else {
    fprintf(stderr, "Error creating shape table: %s\n",
            shape_table_file.c_str());
  }
}

/**
 * Creates a MasterTrainer and loads the training data into it:
 * Initializes feature_defs and IntegerFX.
 * Loads the shape_table if shape_table != nullptr.
 * Loads initial unicharset from -U command-line option.
 * If FLAGS_T is set, loads the majority of data from there, else:
 *  - Loads font info from -F option.
 *  - Loads xheights from -X option.
 *  - Loads samples from .tr files in remaining command-line args.
 *  - Deletes outliers and computes canonical samples.
 *  - If FLAGS_output_trainer is set, saves the trainer for future use.
 *    TODO: Who uses that? There is currently no code which reads it.
 * Computes canonical and cloud features.
 * If shape_table is not nullptr, but failed to load, make a fake flat one,
 * as shape clustering was not run.
 */
MasterTrainer* LoadTrainingData(int argc, const char* const * argv,
                                bool replication,
                                ShapeTable** shape_table,
                                STRING* file_prefix) {
  InitFeatureDefs(&feature_defs);
  InitIntegerFX();
  *file_prefix = "";
  if (!FLAGS_D.empty()) {
    *file_prefix += FLAGS_D.c_str();
    *file_prefix += "/";
  }
  // If we are shape clustering (nullptr shape_table) or we successfully load
  // a shape_table written by a previous shape clustering, then
  // shape_analysis will be true, meaning that the MasterTrainer will replace
  // some members of the unicharset with their fragments.
  bool shape_analysis = false;
  if (shape_table != nullptr) {
    *shape_table = LoadShapeTable(*file_prefix);
    if (*shape_table != nullptr) shape_analysis = true;
  } else {
    shape_analysis = true;
  }
  MasterTrainer* trainer = new MasterTrainer(NM_CHAR_ANISOTROPIC,
                                             shape_analysis,
                                             replication,
                                             FLAGS_debug_level);
  IntFeatureSpace fs;
  fs.Init(kBoostXYBuckets, kBoostXYBuckets, kBoostDirBuckets);
  trainer->LoadUnicharset(FLAGS_U.c_str());
  // Get basic font information from font_properties.
  if (!FLAGS_F.empty()) {
    if (!trainer->LoadFontInfo(FLAGS_F.c_str())) {
      delete trainer;
      return nullptr;
    }
  }
  if (!FLAGS_X.empty()) {
    if (!trainer->LoadXHeights(FLAGS_X.c_str())) {
      delete trainer;
      return nullptr;
    }
  }
  trainer->SetFeatureSpace(fs);
  const char* page_name;
  // Load training data from .tr files on the command line.
  while ((page_name = GetNextFilename(argc, argv)) != nullptr) {
    tprintf("Reading %s ...\n", page_name);
    trainer->ReadTrainingSamples(page_name, feature_defs, false);

    // If there is a file with [lang].[fontname].exp[num].fontinfo present,
    // read font spacing information in to fontinfo_table.
    int pagename_len = strlen(page_name);
    char* fontinfo_file_name = new char[pagename_len + 7];
    strncpy(fontinfo_file_name, page_name, pagename_len - 2);   // remove "tr"
    strcpy(fontinfo_file_name + pagename_len - 2, "fontinfo");  // +"fontinfo"
    trainer->AddSpacingInfo(fontinfo_file_name);
    delete[] fontinfo_file_name;

    // Load the images into memory if required by the classifier.
    if (FLAGS_load_images) {
      STRING image_name = page_name;
      // Chop off the tr and replace with tif. Extension must be tif!
      image_name.truncate_at(image_name.length() - 2);
      image_name += "tif";
      trainer->LoadPageImages(image_name.c_str());
    }
  }
  trainer->PostLoadCleanup();
  // Write the master trainer if required.
  if (!FLAGS_output_trainer.empty()) {
    FILE* fp = fopen(FLAGS_output_trainer.c_str(), "wb");
    if (fp == nullptr) {
      tprintf("Can't create saved trainer data!\n");
    } else {
      trainer->Serialize(fp);
      fclose(fp);
    }
  }
  trainer->PreTrainingSetup();
  if (!FLAGS_O.empty() &&
      !trainer->unicharset().save_to_file(FLAGS_O.c_str())) {
    fprintf(stderr, "Failed to save unicharset to file %s\n", FLAGS_O.c_str());
    delete trainer;
    return nullptr;
  }
  if (shape_table != nullptr) {
    // If we previously failed to load a shapetable, then shape clustering
    // wasn't run so make a flat one now.
    if (*shape_table == nullptr) {
      *shape_table = new ShapeTable;
      trainer->SetupFlatShapeTable(*shape_table);
      tprintf("Flat shape table summary: %s\n",
              (*shape_table)->SummaryStr().c_str());
    }
    (*shape_table)->set_unicharset(trainer->unicharset());
  }
  return trainer;
}

}  // namespace tesseract.

/*---------------------------------------------------------------------------*/
/**
 * This routine returns the next command line argument.  If
 * there are no remaining command line arguments, it returns
 * nullptr.  This routine should only be called after all option
 * arguments have been parsed and removed with ParseArguments.
 *
 * Globals:
 * - tessoptind defined by tessopt sys call
 * @return Next command line argument or nullptr.
 */
const char *GetNextFilename(int argc, const char* const * argv) {
  if (tessoptind < argc)
    return argv[tessoptind++];
  else
    return nullptr;
} /* GetNextFilename */

/*---------------------------------------------------------------------------*/
/**
 * This routine searches through a list of labeled lists to find
 * a list with the specified label.  If a matching labeled list
 * cannot be found, nullptr is returned.
 * @param List list to search
 * @param Label label to search for
 * @return Labeled list with the specified label or nullptr.
 * @note Globals: none
 */
LABELEDLIST FindList(LIST List, char* Label) {
  LABELEDLIST LabeledList;

  iterate (List)
  {
    LabeledList = reinterpret_cast<LABELEDLIST>first_node (List);
    if (strcmp (LabeledList->Label, Label) == 0)
      return (LabeledList);
  }
  return (nullptr);

} /* FindList */

/*---------------------------------------------------------------------------*/
/**
 * This routine allocates a new, empty labeled list and gives
 * it the specified label.
 * @param Label label for new list
 * @return New, empty labeled list.
 * @note Globals: none
 */
LABELEDLIST NewLabeledList(const char* Label) {
  LABELEDLIST LabeledList;

  LabeledList = static_cast<LABELEDLIST>(Emalloc (sizeof (LABELEDLISTNODE)));
  LabeledList->Label = static_cast<char*>(Emalloc (strlen (Label)+1));
  strcpy (LabeledList->Label, Label);
  LabeledList->List = NIL_LIST;
  LabeledList->SampleCount = 0;
  LabeledList->font_sample_count = 0;
  return (LabeledList);

} /* NewLabeledList */

/*---------------------------------------------------------------------------*/
// TODO(rays) This is now used only by cntraining. Convert cntraining to use
// the new method or get rid of it entirely.
/**
 * This routine reads training samples from a file and
 * places them into a data structure which organizes the
 * samples by FontName and CharName.  It then returns this
 * data structure.
 * @param file open text file to read samples from
 * @param feature_definitions
 * @param feature_name
 * @param max_samples
 * @param unicharset
 * @param training_samples
 */
void ReadTrainingSamples(const FEATURE_DEFS_STRUCT& feature_definitions,
                         const char *feature_name, int max_samples,
                         UNICHARSET* unicharset,
                         FILE* file, LIST* training_samples) {
  char    buffer[2048];
  char    unichar[UNICHAR_LEN + 1];
  LABELEDLIST char_sample;
  FEATURE_SET feature_samples;
  CHAR_DESC char_desc;
  uint32_t feature_type =
    ShortNameToFeatureType(feature_definitions, feature_name);

  // Zero out the font_sample_count for all the classes.
  LIST it = *training_samples;
  iterate(it) {
    char_sample = reinterpret_cast<LABELEDLIST>(first_node(it));
    char_sample->font_sample_count = 0;
  }

  while (fgets(buffer, 2048, file) != nullptr) {
    if (buffer[0] == '\n')
      continue;

    sscanf(buffer, "%*s %s", unichar);
    if (unicharset != nullptr && !unicharset->contains_unichar(unichar)) {
      unicharset->unichar_insert(unichar);
      if (unicharset->size() > MAX_NUM_CLASSES) {
        tprintf("Error: Size of unicharset in training is "
                "greater than MAX_NUM_CLASSES\n");
        exit(1);
      }
    }
    char_sample = FindList(*training_samples, unichar);
    if (char_sample == nullptr) {
      char_sample = NewLabeledList(unichar);
      *training_samples = push(*training_samples, char_sample);
    }
    char_desc = ReadCharDescription(feature_definitions, file);
    feature_samples = char_desc->FeatureSets[feature_type];
    if (char_sample->font_sample_count < max_samples || max_samples <= 0) {
      char_sample->List = push(char_sample->List, feature_samples);
      char_sample->SampleCount++;
      char_sample->font_sample_count++;
    } else {
      FreeFeatureSet(feature_samples);
    }
    for (size_t i = 0; i < char_desc->NumFeatureSets; i++) {
      if (feature_type != i)
        FreeFeatureSet(char_desc->FeatureSets[i]);
    }
    free(char_desc);
  }
}  // ReadTrainingSamples


/*---------------------------------------------------------------------------*/
/**
 * This routine deallocates all of the space allocated to
 * the specified list of training samples.
 * @param CharList list of all fonts in document
 */
void FreeTrainingSamples(LIST CharList) {
  LABELEDLIST char_sample;
  FEATURE_SET FeatureSet;
  LIST FeatureList;

  LIST nodes = CharList;
  iterate(CharList) { /* iterate through all of the fonts */
    char_sample = reinterpret_cast<LABELEDLIST>first_node(CharList);
    FeatureList = char_sample->List;
    iterate(FeatureList) { /* iterate through all of the classes */
      FeatureSet = reinterpret_cast<FEATURE_SET>first_node(FeatureList);
      FreeFeatureSet(FeatureSet);
    }
    FreeLabeledList(char_sample);
  }
  destroy(nodes);
}  /* FreeTrainingSamples */

/*---------------------------------------------------------------------------*/
/**
 * This routine deallocates all of the memory consumed by
 * a labeled list.  It does not free any memory which may be
 * consumed by the items in the list.
 * @param LabeledList labeled list to be freed
 * @note Globals: none
 */
void FreeLabeledList(LABELEDLIST LabeledList) {
  destroy(LabeledList->List);
  free(LabeledList->Label);
  free(LabeledList);
}  /* FreeLabeledList */

/*---------------------------------------------------------------------------*/
/**
 * This routine reads samples from a LABELEDLIST and enters
 * those samples into a clusterer data structure.  This
 * data structure is then returned to the caller.
 * @param char_sample: LABELEDLIST that holds all the feature information for a
 * @param FeatureDefs
 * @param program_feature_type
 * given character.
 * @return Pointer to new clusterer data structure.
 * @note Globals: None
 */
CLUSTERER *SetUpForClustering(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              LABELEDLIST char_sample,
                              const char* program_feature_type) {
  uint16_t N;
  int i, j;
  float* Sample = nullptr;
  CLUSTERER *Clusterer;
  int32_t CharID;
  LIST FeatureList = nullptr;
  FEATURE_SET FeatureSet = nullptr;

  int32_t desc_index =
      ShortNameToFeatureType(FeatureDefs, program_feature_type);
  N = FeatureDefs.FeatureDesc[desc_index]->NumParams;
  Clusterer = MakeClusterer(N, FeatureDefs.FeatureDesc[desc_index]->ParamDesc);

  FeatureList = char_sample->List;
  CharID = 0;
  iterate(FeatureList) {
    FeatureSet = reinterpret_cast<FEATURE_SET>first_node(FeatureList);
    for (i = 0; i < FeatureSet->MaxNumFeatures; i++) {
      if (Sample == nullptr) Sample = static_cast<float*>(Emalloc(N * sizeof(float)));
      for (j = 0; j < N; j++)
        Sample[j] = FeatureSet->Features[i]->Params[j];
      MakeSample (Clusterer, Sample, CharID);
    }
    CharID++;
  }
  free(Sample);
  return Clusterer;

} /* SetUpForClustering */

/*------------------------------------------------------------------------*/
void MergeInsignificantProtos(LIST ProtoList, const char* label,
                              CLUSTERER* Clusterer,
                              CLUSTERCONFIG* clusterconfig) {
  PROTOTYPE* Prototype;
  bool debug = strcmp(FLAGS_test_ch.c_str(), label) == 0;

  LIST pProtoList = ProtoList;
  iterate(pProtoList) {
    Prototype = reinterpret_cast<PROTOTYPE *>first_node (pProtoList);
    if (Prototype->Significant || Prototype->Merged)
      continue;
    float best_dist = 0.125;
    PROTOTYPE* best_match = nullptr;
    // Find the nearest alive prototype.
    LIST list_it = ProtoList;
    iterate(list_it) {
      PROTOTYPE* test_p = reinterpret_cast<PROTOTYPE *>first_node (list_it);
      if (test_p != Prototype && !test_p->Merged) {
        float dist = ComputeDistance(Clusterer->SampleSize,
                                     Clusterer->ParamDesc,
                                     Prototype->Mean, test_p->Mean);
        if (dist < best_dist) {
          best_match = test_p;
          best_dist = dist;
        }
      }
    }
    if (best_match != nullptr && !best_match->Significant) {
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
      Prototype->Merged = true;
    } else if (best_match != nullptr) {
      if (debug)
        tprintf("Red proto at %g,%g matched a green one at %g,%g\n",
                Prototype->Mean[0], Prototype->Mean[1],
                best_match->Mean[0], best_match->Mean[1]);
      Prototype->Merged = true;
    }
  }
  // Mark significant those that now have enough samples.
  int min_samples =
    static_cast<int32_t>(clusterconfig->MinSamples * Clusterer->NumChar);
  pProtoList = ProtoList;
  iterate(pProtoList) {
    Prototype = reinterpret_cast<PROTOTYPE *>first_node (pProtoList);
    // Process insignificant protos that do not match a green one
    if (!Prototype->Significant && Prototype->NumSamples >= min_samples &&
        !Prototype->Merged) {
      if (debug)
        tprintf("Red proto at %g,%g becoming green\n",
                Prototype->Mean[0], Prototype->Mean[1]);
      Prototype->Significant = true;
    }
  }
} /* MergeInsignificantProtos */

/*-----------------------------------------------------------------------------*/
void CleanUpUnusedData(
    LIST ProtoList)
{
  PROTOTYPE* Prototype;

  iterate(ProtoList)
  {
    Prototype = reinterpret_cast<PROTOTYPE *>first_node (ProtoList);
    free(Prototype->Variance.Elliptical);
    Prototype->Variance.Elliptical = nullptr;
    free(Prototype->Magnitude.Elliptical);
    Prototype->Magnitude.Elliptical = nullptr;
    free(Prototype->Weight.Elliptical);
    Prototype->Weight.Elliptical = nullptr;
  }
}

/*------------------------------------------------------------------------*/
LIST RemoveInsignificantProtos(
    LIST ProtoList,
    bool KeepSigProtos,
    bool KeepInsigProtos,
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
    Proto = reinterpret_cast<PROTOTYPE *>first_node (pProtoList);
    if ((Proto->Significant && KeepSigProtos) ||
        (!Proto->Significant && KeepInsigProtos))
    {
      NewProto = static_cast<PROTOTYPE *>(Emalloc(sizeof(PROTOTYPE)));

      NewProto->Mean = static_cast<float *>(Emalloc(N * sizeof(float)));
      NewProto->Significant = Proto->Significant;
      NewProto->Style = Proto->Style;
      NewProto->NumSamples = Proto->NumSamples;
      NewProto->Cluster = nullptr;
      NewProto->Distrib = nullptr;

      for (i=0; i < N; i++)
        NewProto->Mean[i] = Proto->Mean[i];
      if (Proto->Variance.Elliptical != nullptr) {
        NewProto->Variance.Elliptical = static_cast<float *>(Emalloc(N * sizeof(float)));
        for (i=0; i < N; i++)
          NewProto->Variance.Elliptical[i] = Proto->Variance.Elliptical[i];
      }
      else
        NewProto->Variance.Elliptical = nullptr;
      //---------------------------------------------
      if (Proto->Magnitude.Elliptical != nullptr) {
        NewProto->Magnitude.Elliptical = static_cast<float *>(Emalloc(N * sizeof(float)));
        for (i=0; i < N; i++)
          NewProto->Magnitude.Elliptical[i] = Proto->Magnitude.Elliptical[i];
      }
      else
        NewProto->Magnitude.Elliptical = nullptr;
      //------------------------------------------------
      if (Proto->Weight.Elliptical != nullptr) {
        NewProto->Weight.Elliptical = static_cast<float *>(Emalloc(N * sizeof(float)));
        for (i=0; i < N; i++)
          NewProto->Weight.Elliptical[i] = Proto->Weight.Elliptical[i];
      }
      else
        NewProto->Weight.Elliptical = nullptr;

      NewProto->TotalMagnitude = Proto->TotalMagnitude;
      NewProto->LogMagnitude = Proto->LogMagnitude;
      NewProtoList = push_last(NewProtoList, NewProto);
    }
  }
  FreeProtoList(&ProtoList);
  return (NewProtoList);
} /* RemoveInsignificantProtos */

/*----------------------------------------------------------------------------*/
MERGE_CLASS FindClass(LIST List, const char* Label) {
  MERGE_CLASS MergeClass;

  iterate (List)
  {
    MergeClass = reinterpret_cast<MERGE_CLASS>first_node (List);
    if (strcmp (MergeClass->Label, Label) == 0)
      return (MergeClass);
  }
  return (nullptr);

} /* FindClass */

/*---------------------------------------------------------------------------*/
MERGE_CLASS NewLabeledClass(const char* Label) {
  MERGE_CLASS MergeClass;

  MergeClass = new MERGE_CLASS_NODE;
  MergeClass->Label = static_cast<char*>(Emalloc (strlen (Label)+1));
  strcpy (MergeClass->Label, Label);
  MergeClass->Class = NewClass (MAX_NUM_PROTOS, MAX_NUM_CONFIGS);
  return (MergeClass);

} /* NewLabeledClass */

/*-----------------------------------------------------------------------------*/
/**
 * This routine deallocates all of the space allocated to
 * the specified list of training samples.
 * @param ClassList list of all fonts in document
 */
void FreeLabeledClassList(LIST ClassList) {
  MERGE_CLASS MergeClass;

  LIST nodes = ClassList;
  iterate(ClassList) /* iterate through all of the fonts */
  {
    MergeClass = reinterpret_cast<MERGE_CLASS>first_node (ClassList);
    free (MergeClass->Label);
    FreeClass(MergeClass->Class);
    delete MergeClass;
  }
  destroy(nodes);

} /* FreeLabeledClassList */

/* SetUpForFloat2Int */
CLASS_STRUCT* SetUpForFloat2Int(const UNICHARSET& unicharset,
                                LIST LabeledClassList) {
  MERGE_CLASS MergeClass;
  CLASS_TYPE Class;
  int NumProtos;
  int NumConfigs;
  int NumWords;
  int i, j;
  float Values[3];
  PROTO NewProto;
  PROTO OldProto;
  BIT_VECTOR NewConfig;
  BIT_VECTOR OldConfig;

  //  printf("Float2Int ...\n");

  CLASS_STRUCT* float_classes = new CLASS_STRUCT[unicharset.size()];
  iterate(LabeledClassList)
  {
    UnicityTableEqEq<int>   font_set;
    MergeClass = reinterpret_cast<MERGE_CLASS>first_node (LabeledClassList);
    Class = &float_classes[unicharset.unichar_to_id(MergeClass->Label)];
    NumProtos = MergeClass->Class->NumProtos;
    NumConfigs = MergeClass->Class->NumConfigs;
    font_set.move(&MergeClass->Class->font_set);
    Class->NumProtos = NumProtos;
    Class->MaxNumProtos = NumProtos;
    Class->Prototypes = static_cast<PROTO>(Emalloc (sizeof(PROTO_STRUCT) * NumProtos));
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
    Class->Configurations = static_cast<BIT_VECTOR*>(Emalloc (sizeof(BIT_VECTOR) * NumConfigs));
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
  return float_classes;
} // SetUpForFloat2Int

/*--------------------------------------------------------------------------*/
void Normalize (
    float  *Values)
{
  float Slope;
  float Intercept;
  float Normalizer;

  Slope      = tan(Values [2] * 2 * M_PI);
  Intercept  = Values [1] - Slope * Values [0];
  Normalizer = 1 / sqrt (Slope * Slope + 1.0);

  Values [0] = Slope * Normalizer;
  Values [1] = - Normalizer;
  Values [2] = Intercept * Normalizer;
} // Normalize

/*-------------------------------------------------------------------------*/
void FreeNormProtoList(LIST CharList)

{
  LABELEDLIST char_sample;

  LIST nodes = CharList;
  iterate(CharList) /* iterate through all of the fonts */
  {
    char_sample = reinterpret_cast<LABELEDLIST>first_node (CharList);
    FreeLabeledList (char_sample);
  }
  destroy(nodes);

}  // FreeNormProtoList

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
    Proto = reinterpret_cast<PROTOTYPE *>first_node (ProtoList);
    LabeledProtoList->List = push(LabeledProtoList->List, Proto);
  }
  *NormProtoList = push(*NormProtoList, LabeledProtoList);
}

/*---------------------------------------------------------------------------*/
int NumberOfProtos(LIST ProtoList, bool CountSigProtos,
                   bool CountInsigProtos) {
  int N = 0;
  iterate(ProtoList)
  {
    PROTOTYPE* Proto = reinterpret_cast<PROTOTYPE*>first_node(ProtoList);
    if ((Proto->Significant && CountSigProtos) ||
        (!Proto->Significant && CountInsigProtos))
      N++;
  }
  return(N);
}

#endif  // def DISABLED_LEGACY_ENGINE
