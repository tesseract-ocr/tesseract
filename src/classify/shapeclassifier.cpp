// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapeclassifier.cpp
// Description: Base interface class for classifiers that return a
//              shape index.
// Author:      Ray Smith
//
// (C) Copyright 2011, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "shapeclassifier.h"
#include <tesseract/genericvector.h>
#include "scrollview.h"
#include "shapetable.h"
#include "svmnode.h"
#include "trainingsample.h"
#include "tprintf.h"

namespace tesseract {

// Classifies the given [training] sample, writing to results.
// See shapeclassifier.h for a full description.
// Default implementation calls the ShapeRating version.
int ShapeClassifier::UnicharClassifySample(
    const TrainingSample& sample, Pix* page_pix, int debug,
    UNICHAR_ID keep_this, GenericVector<UnicharRating>* results) {
  results->truncate(0);
  GenericVector<ShapeRating> shape_results;
  int num_shape_results = ClassifySample(sample, page_pix, debug, keep_this,
                                         &shape_results);
  const ShapeTable* shapes = GetShapeTable();
  GenericVector<int> unichar_map;
  unichar_map.init_to_size(shapes->unicharset().size(), -1);
  for (int r = 0; r < num_shape_results; ++r) {
    shapes->AddShapeToResults(shape_results[r], &unichar_map, results);
  }
  return results->size();
}

// Classifies the given [training] sample, writing to results.
// See shapeclassifier.h for a full description.
// Default implementation aborts.
int ShapeClassifier::ClassifySample(const TrainingSample& sample, Pix* page_pix,
                           int debug, int keep_this,
                           GenericVector<ShapeRating>* results) {
  ASSERT_HOST("Must implement ClassifySample!" == nullptr);
  return 0;
}

// Returns the shape that contains unichar_id that has the best result.
// If result is not nullptr, it is set with the shape_id and rating.
// Does not need to be overridden if ClassifySample respects the keep_this
// rule.
int ShapeClassifier::BestShapeForUnichar(const TrainingSample& sample,
                                         Pix* page_pix, UNICHAR_ID unichar_id,
                                         ShapeRating* result) {
  GenericVector<ShapeRating> results;
  const ShapeTable* shapes = GetShapeTable();
  int num_results = ClassifySample(sample, page_pix, 0, unichar_id, &results);
  for (int r = 0; r < num_results; ++r) {
    if (shapes->GetShape(results[r].shape_id).ContainsUnichar(unichar_id)) {
      if (result != nullptr)
        *result = results[r];
      return results[r].shape_id;
    }
  }
  return -1;
}

// Provides access to the UNICHARSET that this classifier works with.
// Only needs to be overridden if GetShapeTable() can return nullptr.
const UNICHARSET& ShapeClassifier::GetUnicharset() const {
  return GetShapeTable()->unicharset();
}

#ifndef GRAPHICS_DISABLED

// Visual debugger classifies the given sample, displays the results and
// solicits user input to display other classifications. Returns when
// the user has finished with debugging the sample.
// Probably doesn't need to be overridden if the subclass provides
// DisplayClassifyAs.
void ShapeClassifier::DebugDisplay(const TrainingSample& sample,
                                   Pix* page_pix,
                                   UNICHAR_ID unichar_id) {
  static ScrollView* terminator = nullptr;
  if (terminator == nullptr) {
    terminator = new ScrollView("XIT", 0, 0, 50, 50, 50, 50, true);
  }
  ScrollView* debug_win = CreateFeatureSpaceWindow("ClassifierDebug", 0, 0);
  // Provide a right-click menu to choose the class.
  auto* popup_menu = new SVMenuNode();
  popup_menu->AddChild("Choose class to debug", 0, "x", "Class to debug");
  popup_menu->BuildMenu(debug_win, false);
  // Display the features in green.
  const INT_FEATURE_STRUCT* features = sample.features();
  uint32_t num_features = sample.num_features();
  for (uint32_t f = 0; f < num_features; ++f) {
    RenderIntFeature(debug_win, &features[f], ScrollView::GREEN);
  }
  debug_win->Update();
  GenericVector<UnicharRating> results;
  // Debug classification until the user quits.
  const UNICHARSET& unicharset = GetUnicharset();
  SVEvent* ev;
  SVEventType ev_type;
  do {
    PointerVector<ScrollView> windows;
    if (unichar_id >= 0) {
      tprintf("Debugging class %d = %s\n",
              unichar_id, unicharset.id_to_unichar(unichar_id));
      UnicharClassifySample(sample, page_pix, 1, unichar_id, &results);
      DisplayClassifyAs(sample, page_pix, unichar_id, 1, &windows);
    } else {
      tprintf("Invalid unichar_id: %d\n", unichar_id);
      UnicharClassifySample(sample, page_pix, 1, -1, &results);
    }
    if (unichar_id >= 0) {
      tprintf("Debugged class %d = %s\n",
              unichar_id, unicharset.id_to_unichar(unichar_id));
    }
    tprintf("Right-click in ClassifierDebug window to choose debug class,");
    tprintf(" Left-click or close window to quit...\n");
    UNICHAR_ID old_unichar_id;
    do {
      old_unichar_id = unichar_id;
      ev = debug_win->AwaitEvent(SVET_ANY);
      ev_type = ev->type;
      if (ev_type == SVET_POPUP) {
        if (unicharset.contains_unichar(ev->parameter)) {
          unichar_id = unicharset.unichar_to_id(ev->parameter);
        } else {
          tprintf("Char class '%s' not found in unicharset", ev->parameter);
        }
      }
      delete ev;
    } while (unichar_id == old_unichar_id &&
             ev_type != SVET_CLICK && ev_type != SVET_DESTROY);
  } while (ev_type != SVET_CLICK && ev_type != SVET_DESTROY);
  delete debug_win;
}

#endif // !GRAPHICS_DISABLED

// Displays classification as the given shape_id. Creates as many windows
// as it feels fit, using index as a guide for placement. Adds any created
// windows to the windows output and returns a new index that may be used
// by any subsequent classifiers. Caller waits for the user to view and
// then destroys the windows by clearing the vector.
int ShapeClassifier::DisplayClassifyAs(
    const TrainingSample& sample, Pix* page_pix,
    UNICHAR_ID unichar_id, int index,
    PointerVector<ScrollView>* windows) {
  // Does nothing in the default implementation.
  return index;
}

// Prints debug information on the results.
void ShapeClassifier::UnicharPrintResults(
    const char* context, const GenericVector<UnicharRating>& results) const {
  tprintf("%s\n", context);
  for (int i = 0; i < results.size(); ++i) {
    tprintf("%g: c_id=%d=%s", results[i].rating, results[i].unichar_id,
            GetUnicharset().id_to_unichar(results[i].unichar_id));
    if (!results[i].fonts.empty()) {
      tprintf(" Font Vector:");
      for (int f = 0; f < results[i].fonts.size(); ++f) {
        tprintf(" %d", results[i].fonts[f].fontinfo_id);
      }
    }
    tprintf("\n");
  }
}
void ShapeClassifier::PrintResults(
    const char* context, const GenericVector<ShapeRating>& results) const {
  tprintf("%s\n", context);
  for (int i = 0; i < results.size(); ++i) {
    tprintf("%g:", results[i].rating);
    if (results[i].joined)
      tprintf("[J]");
    if (results[i].broken)
      tprintf("[B]");
    tprintf(" %s\n", GetShapeTable()->DebugStr(results[i].shape_id).c_str());
  }
}

// Removes any result that has all its unichars covered by a better choice,
// regardless of font.
void ShapeClassifier::FilterDuplicateUnichars(
    GenericVector<ShapeRating>* results) const {
  GenericVector<ShapeRating> filtered_results;
  // Copy results to filtered results and knock out duplicate unichars.
  const ShapeTable* shapes = GetShapeTable();
  for (int r = 0; r < results->size(); ++r) {
    if (r > 0) {
      const Shape& shape_r = shapes->GetShape((*results)[r].shape_id);
      int c;
      for (c = 0; c < shape_r.size(); ++c) {
        int unichar_id = shape_r[c].unichar_id;
        int s;
        for (s = 0; s < r; ++s) {
          const Shape& shape_s = shapes->GetShape((*results)[s].shape_id);
          if (shape_s.ContainsUnichar(unichar_id))
            break;  // We found unichar_id.
        }
        if (s == r)
          break;  // We didn't find unichar_id.
      }
      if (c == shape_r.size())
        continue;  // We found all the unichar ids in previous answers.
    }
    filtered_results.push_back((*results)[r]);
  }
  *results = filtered_results;
}

}  // namespace tesseract.
