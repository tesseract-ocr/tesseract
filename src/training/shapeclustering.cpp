// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//  Filename: shapeclustering.cpp
//  Purpose:  Generates a master shape table to merge similarly-shaped
//            training data of whole, partial or multiple characters.
//  Author:   Ray Smith

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef GOOGLE_TESSERACT
#include "base/commandlineflags.h"
#endif
#include "commontraining.h"
#include "mastertrainer.h"
#include "params.h"
#include "strngs.h"

static INT_PARAM_FLAG(display_cloud_font, -1,
                      "Display cloud of this font, canonical_class1");
static INT_PARAM_FLAG(display_canonical_font, -1,
                      "Display canonical sample of this font, canonical_class2");
static STRING_PARAM_FLAG(canonical_class1, "", "Class to show ambigs for");
static STRING_PARAM_FLAG(canonical_class2, "", "Class to show ambigs for");

// Loads training data, if requested displays debug information, otherwise
// creates the master shape table by shape clustering and writes it to a file.
// If FLAGS_display_cloud_font is set, then the cloud features of
// FLAGS_canonical_class1/FLAGS_display_cloud_font are shown in green ON TOP
// OF the red canonical features of FLAGS_canonical_class2/
// FLAGS_display_canonical_font, so as to show which canonical features are
// NOT in the cloud.
// Otherwise, if FLAGS_canonical_class1 is set, prints a table of font-wise
// cluster distances between FLAGS_canonical_class1 and FLAGS_canonical_class2.
int main(int argc, char **argv) {
  tesseract::CheckSharedLibraryVersion();

  ParseArguments(&argc, &argv);

  STRING file_prefix;
  tesseract::MasterTrainer* trainer =
      tesseract::LoadTrainingData(argc, argv, false, nullptr, &file_prefix);

  if (!trainer)
    return 1;

  if (FLAGS_display_cloud_font >= 0) {
#ifndef GRAPHICS_DISABLED
    trainer->DisplaySamples(FLAGS_canonical_class1.c_str(),
                            FLAGS_display_cloud_font,
                            FLAGS_canonical_class2.c_str(),
                            FLAGS_display_canonical_font);
#endif  // GRAPHICS_DISABLED
    return 0;
  } else if (!FLAGS_canonical_class1.empty()) {
    trainer->DebugCanonical(FLAGS_canonical_class1.c_str(),
                            FLAGS_canonical_class2.c_str());
    return 0;
  }
  trainer->SetupMasterShapes();
  WriteShapeTable(file_prefix, trainer->master_shapes());
  delete trainer;

  return 0;
} /* main */
