/******************************************************************************
 ** Filename:   mftraining.c
 ** Purpose:    Separates training pages into files for each character.
 **             Strips from files only the features and there parameters of
 **             the feature type mf.
 ** Author:     Dan Johnson
 ** Revisment:  Christy Russon
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

#define _USE_MATH_DEFINES // for M_PI
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include <cmath> // for M_PI
#include <cstdio>
#include <cstring>

#include "classify.h"
#include "cluster.h"
#include "clusttool.h"
#include "commontraining.h"
#include "featdefs.h"
#include "fontinfo.h"
#include "indexmapbidi.h"
#include "intproto.h"
#include "mastertrainer.h"
#include "mergenf.h"
#include "mf.h"
#include "ocrfeatures.h"
#include "oldlist.h"
#include "protos.h"
#include "shapetable.h"
#include "tprintf.h"
#include "unicity_table.h"

using namespace tesseract;

/*----------------------------------------------------------------------------
            Public Code
-----------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
static void DisplayProtoList(const char *ch, LIST protolist) {
  auto window = std::make_unique<ScrollView>("Char samples", 50, 200, 520, 520, 260, 260, true);
  LIST proto = protolist;
  iterate(proto) {
    auto *prototype = reinterpret_cast<PROTOTYPE *>(proto->first_node());
    if (prototype->Significant) {
      window->Pen(ScrollView::GREEN);
    } else if (prototype->NumSamples == 0) {
      window->Pen(ScrollView::BLUE);
    } else if (prototype->Merged) {
      window->Pen(ScrollView::MAGENTA);
    } else {
      window->Pen(ScrollView::RED);
    }
    float x = CenterX(prototype->Mean);
    float y = CenterY(prototype->Mean);
    double angle = OrientationOf(prototype->Mean) * 2 * M_PI;
    auto dx = static_cast<float>(LengthOf(prototype->Mean) * cos(angle) / 2);
    auto dy = static_cast<float>(LengthOf(prototype->Mean) * sin(angle) / 2);
    window->SetCursor((x - dx) * 256, (y - dy) * 256);
    window->DrawTo((x + dx) * 256, (y + dy) * 256);
    auto prototypeNumSamples = prototype->NumSamples;
    if (prototype->Significant) {
      tprintf("Green proto at (%g,%g)+(%g,%g) %d samples\n", x, y, dx, dy, prototypeNumSamples);
    } else if (prototype->NumSamples > 0 && !prototype->Merged) {
      tprintf("Red proto at (%g,%g)+(%g,%g) %d samples\n", x, y, dx, dy, prototypeNumSamples);
    }
  }
  window->Update();
}
#endif // !GRAPHICS_DISABLED

// Helper to run clustering on a single config.
// Mostly copied from the old mftraining, but with renamed variables.
static LIST ClusterOneConfig(int shape_id, const char *class_label, LIST mf_classes,
                             const ShapeTable &shape_table, MasterTrainer *trainer) {
  int num_samples;
  CLUSTERER *clusterer =
      trainer->SetupForClustering(shape_table, feature_defs, shape_id, &num_samples);
  Config.MagicSamples = num_samples;
  LIST proto_list = ClusterSamples(clusterer, &Config);
  CleanUpUnusedData(proto_list);

  // Merge protos where reasonable to make more of them significant by
  // representing almost all samples of the class/font.
  MergeInsignificantProtos(proto_list, class_label, clusterer, &Config);
#ifndef GRAPHICS_DISABLED
  if (strcmp(FLAGS_test_ch.c_str(), class_label) == 0) {
    DisplayProtoList(FLAGS_test_ch.c_str(), proto_list);
  }
#endif // !GRAPHICS_DISABLED
  // Delete the protos that will not be used in the inttemp output file.
  proto_list = RemoveInsignificantProtos(proto_list, true, false, clusterer->SampleSize);
  FreeClusterer(clusterer);
  MERGE_CLASS merge_class = FindClass(mf_classes, class_label);
  if (merge_class == nullptr) {
    merge_class = new MERGE_CLASS_NODE(class_label);
    mf_classes = push(mf_classes, merge_class);
  }
  int config_id = AddConfigToClass(merge_class->Class);
  merge_class->Class->font_set.push_back(shape_id);
  LIST proto_it = proto_list;
  iterate(proto_it) {
    auto *prototype = reinterpret_cast<PROTOTYPE *>(proto_it->first_node());
    // See if proto can be approximated by existing proto.
    int p_id = FindClosestExistingProto(merge_class->Class, merge_class->NumMerged, prototype);
    if (p_id == NO_PROTO) {
      // Need to make a new proto, as it doesn't match anything.
      p_id = AddProtoToClass(merge_class->Class);
      MakeNewFromOld(ProtoIn(merge_class->Class, p_id), prototype);
      merge_class->NumMerged[p_id] = 1;
    } else {
      PROTO_STRUCT dummy_proto;
      MakeNewFromOld(&dummy_proto, prototype);
      // Merge with the similar proto.
      ComputeMergedProto(ProtoIn(merge_class->Class, p_id), &dummy_proto,
                         static_cast<float>(merge_class->NumMerged[p_id]), 1.0,
                         ProtoIn(merge_class->Class, p_id));
      merge_class->NumMerged[p_id]++;
    }
    AddProtoToConfig(p_id, merge_class->Class->Configurations[config_id]);
  }
  FreeProtoList(&proto_list);
  return mf_classes;
}

// Helper to setup the config map.
// Setup an index mapping from the shapes in the shape table to the classes
// that will be trained. In keeping with the original design, each shape
// with the same list of unichars becomes a different class and the configs
// represent the different combinations of fonts.
static void SetupConfigMap(ShapeTable *shape_table, IndexMapBiDi *config_map) {
  int num_configs = shape_table->NumShapes();
  config_map->Init(num_configs, true);
  config_map->Setup();
  for (int c1 = 0; c1 < num_configs; ++c1) {
    // Only process ids that are not already merged.
    if (config_map->SparseToCompact(c1) == c1) {
      Shape *shape1 = shape_table->MutableShape(c1);
      // Find all the subsequent shapes that are equal.
      for (int c2 = c1 + 1; c2 < num_configs; ++c2) {
        if (shape_table->MutableShape(c2)->IsEqualUnichars(shape1)) {
          config_map->Merge(c1, c2);
        }
      }
    }
  }
  config_map->CompleteMerges();
}

/**
 * This program reads in a text file consisting of feature
 * samples from a training page in the following format:
 * @verbatim
      FontName UTF8-char-str xmin ymin xmax ymax page-number
       NumberOfFeatureTypes(N)
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
 * The result of this program is a binary inttemp file used by
 * the OCR engine.
 * @param  argc  number of command line arguments
 * @param  argv  array of command line arguments
 * @return 0 if no error occurred
 */
int main(int argc, char **argv) {
  tesseract::CheckSharedLibraryVersion();

  ParseArguments(&argc, &argv);

  ShapeTable *shape_table = nullptr;
  std::string file_prefix;
  // Load the training data.
  auto trainer = tesseract::LoadTrainingData(argv + 1, false, &shape_table, file_prefix);
  if (trainer == nullptr) {
    return EXIT_FAILURE; // Failed.
  }

  // Setup an index mapping from the shapes in the shape table to the classes
  // that will be trained. In keeping with the original design, each shape
  // with the same list of unichars becomes a different class and the configs
  // represent the different combinations of fonts.
  IndexMapBiDi config_map;
  SetupConfigMap(shape_table, &config_map);

  WriteShapeTable(file_prefix, *shape_table);
  // If the shape_table is flat, then either we didn't run shape clustering, or
  // it did nothing, so we just output the trainer's unicharset.
  // Otherwise shape_set will hold a fake unicharset with an entry for each
  // shape in the shape table, and we will output that instead.
  UNICHARSET shape_set;
  const UNICHARSET *unicharset = &trainer->unicharset();
  // If we ran shapeclustering (and it worked) then at least one shape will
  // have multiple unichars, so we have to build a fake unicharset.
  if (shape_table->AnyMultipleUnichars()) {
    unicharset = &shape_set;
    // Now build a fake unicharset for the compact shape space to keep the
    // output modules happy that we are doing things correctly.
    int num_shapes = config_map.CompactSize();
    for (int s = 0; s < num_shapes; ++s) {
      char shape_label[14];
      snprintf(shape_label, sizeof(shape_label), "sh%04d", s);
      shape_set.unichar_insert(shape_label);
    }
  }

  // Now train each config separately.
  int num_configs = shape_table->NumShapes();
  LIST mf_classes = NIL_LIST;
  for (int s = 0; s < num_configs; ++s) {
    int unichar_id, font_id;
    if (unicharset == &shape_set) {
      // Using fake unichar_ids from the config_map/shape_set.
      unichar_id = config_map.SparseToCompact(s);
    } else {
      // Get the real unichar_id from the shape table/unicharset.
      shape_table->GetFirstUnicharAndFont(s, &unichar_id, &font_id);
    }
    const char *class_label = unicharset->id_to_unichar(unichar_id);
    mf_classes = ClusterOneConfig(s, class_label, mf_classes, *shape_table, trainer.get());
  }
  std::string inttemp_file = file_prefix;
  inttemp_file += "inttemp";
  std::string pffmtable_file = std::move(file_prefix);
  pffmtable_file += "pffmtable";
  CLASS_STRUCT *float_classes = SetUpForFloat2Int(*unicharset, mf_classes);
  // Now write the inttemp and pffmtable.
  trainer->WriteInttempAndPFFMTable(trainer->unicharset(), *unicharset, *shape_table, float_classes,
                                    inttemp_file.c_str(), pffmtable_file.c_str());
  for (size_t c = 0; c < unicharset->size(); ++c) {
    FreeClassFields(&float_classes[c]);
  }
  delete[] float_classes;
  FreeLabeledClassList(mf_classes);
  delete shape_table;
  printf("Done!\n");
  if (!FLAGS_test_ch.empty()) {
    // If we are displaying debug window(s), wait for the user to look at them.
    printf("Hit return to exit...\n");
    while (getchar() != '\n') {
      ;
    }
  }
  return EXIT_SUCCESS;
} /* main */
