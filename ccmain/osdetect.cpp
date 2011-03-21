///////////////////////////////////////////////////////////////////////
// File:        osdetect.cpp
// Description: Orientation and script detection.
// Author:      Samuel Charron
//              Ranjith Unnikrishnan
//
// (C) Copyright 2008, Google Inc.
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

#include "osdetect.h"

#include "blobbox.h"
#include "blread.h"
#include "colfind.h"
#include "imagefind.h"
#include "linefind.h"
#include "oldlist.h"
#include "qrsequence.h"
#include "ratngs.h"
#include "strngs.h"
#include "tabvector.h"
#include "tesseractclass.h"
#include "textord.h"

const int kMinCharactersToTry = 50;
const int kMaxCharactersToTry = 5 * kMinCharactersToTry;

const float kSizeRatioToReject = 2.0;
const int kMinAcceptableBlobHeight = 10;

const float kOrientationAcceptRatio = 1.3;
const float kScriptAcceptRatio = 1.3;

const float kHanRatioInKorean = 0.7;
const float kHanRatioInJapanese = 0.3;

const float kNonAmbiguousMargin = 1.0;

// General scripts
static const char* han_script = "Han";
static const char* latin_script = "Latin";
static const char* katakana_script = "Katakana";
static const char* hiragana_script = "Hiragana";
static const char* hangul_script = "Hangul";

// Pseudo-scripts Name
const char* ScriptDetector::korean_script_ = "Korean";
const char* ScriptDetector::japanese_script_ = "Japanese";
const char* ScriptDetector::fraktur_script_ = "Fraktur";

// Minimum believable resolution.
const int kMinCredibleResolution = 70;
// Default resolution used if input is not believable.
const int kDefaultResolution = 300;

void OSResults::update_best_orientation() {
  float first = orientations[0];
  float second = orientations[1];
  best_result.orientation_id = 0;
  if (orientations[0] < orientations[1]) {
    first = orientations[1];
    second = orientations[0];
    best_result.orientation_id = 1;
  }
  for (int i = 2; i < 4; ++i) {
    if (orientations[i] > first) {
      second = first;
      first = orientations[i];
      best_result.orientation_id = i;
    } else if (orientations[i] > second) {
      second = orientations[i];
    }
  }
  // Store difference of top two orientation scores.
  best_result.oconfidence = first - second;
}

void OSResults::set_best_orientation(int orientation_id) {
  best_result.orientation_id = orientation_id;
  best_result.oconfidence = 0;
}

void OSResults::update_best_script(int orientation) {
  // We skip index 0 to ignore the "Common" script.
  float first = scripts_na[orientation][1];
  float second = scripts_na[orientation][2];
  best_result.script_id = 1;
  if (scripts_na[orientation][1] < scripts_na[orientation][2]) {
    first = scripts_na[orientation][2];
    second = scripts_na[orientation][1];
    best_result.script_id = 2;
  }
  for (int i = 3; i < kMaxNumberOfScripts; ++i) {
    if (scripts_na[orientation][i] > first) {
      best_result.script_id = i;
      second = first;
      first = scripts_na[orientation][i];
    } else if (scripts_na[orientation][i] > second) {
      second = scripts_na[orientation][i];
    }
  }
  best_result.sconfidence =
      (first / second - 1.0) / (kScriptAcceptRatio - 1.0);
}

// Detect and erase horizontal/vertical lines and picture regions from the
// image, so that non-text blobs are removed from consideration.
void remove_nontext_regions(tesseract::Tesseract *tess, BLOCK_LIST *blocks,
                            TO_BLOCK_LIST *to_blocks) {
  Pix *pix = tess->pix_binary();
  ASSERT_HOST(pix != NULL);
  int vertical_x = 0;
  int vertical_y = 1;
  tesseract::TabVector_LIST v_lines;
  tesseract::TabVector_LIST h_lines;
  Boxa* boxa = NULL;
  Pixa* pixa = NULL;
  const int kMinCredibleResolution = 70;
  int resolution = (kMinCredibleResolution > pixGetXRes(pix)) ?
      kMinCredibleResolution : pixGetXRes(pix);

  tesseract::LineFinder::FindVerticalLines(resolution, pix, &vertical_x,
                                           &vertical_y, &v_lines);
  tesseract::LineFinder::FindHorizontalLines(resolution, pix, &h_lines);
  tesseract::ImageFinder::FindImages(pix, &boxa, &pixa);
  pixaDestroy(&pixa);
  boxaDestroy(&boxa);
  tess->mutable_textord()->find_components(tess->pix_binary(),
                                           blocks, to_blocks);
}

// Find connected components in the page and process a subset until finished or
// a stopping criterion is met.
// Returns the number of blobs used in making the estimate. 0 implies failure.
int orientation_and_script_detection(STRING& filename,
                                     OSResults* osr,
                                     tesseract::Tesseract* tess) {
  STRING name = filename;        //truncated name
  const char *lastdot;           //of name
  TBOX page_box;

  lastdot = strrchr (name.string (), '.');
  if (lastdot != NULL)
    name[lastdot-name.string()] = '\0';

  ASSERT_HOST(tess->pix_binary() != NULL)
  int width = pixGetWidth(tess->pix_binary());
  int height = pixGetHeight(tess->pix_binary());
  int resolution = pixGetXRes(tess->pix_binary());
  // Zero resolution messes up the algorithms, so make sure it is credible.
  if (resolution < kMinCredibleResolution)
    resolution = kDefaultResolution;

  BLOCK_LIST blocks;
  if (!read_unlv_file(name, width, height, &blocks))
    FullPageBlock(width, height, &blocks);

  // Try to remove non-text regions from consideration.
  TO_BLOCK_LIST land_blocks, port_blocks;
  remove_nontext_regions(tess, &blocks, &port_blocks);

  if (port_blocks.empty()) {
    // page segmentation did not succeed, so we need to find_components first.
    tess->mutable_textord()->find_components(tess->pix_binary(),
                                             &blocks, &port_blocks);
  } else {
    page_box.set_left(0);
    page_box.set_bottom(0);
    page_box.set_right(width);
    page_box.set_top(height);
    // Filter_blobs sets up the TO_BLOCKs the same as find_components does.
    tess->mutable_textord()->filter_blobs(page_box.topright(),
                                          &port_blocks, true);
  }

  return os_detect(&port_blocks, osr, tess);
}

// Filter and sample the blobs.
// Returns a non-zero number of blobs if the page was successfully processed, or
// zero if the page had too few characters to be reliable
int os_detect(TO_BLOCK_LIST* port_blocks, OSResults* osr,
              tesseract::Tesseract* tess) {
  int blobs_total = 0;
  TO_BLOCK_IT block_it;
  block_it.set_to_list(port_blocks);

  BLOBNBOX_CLIST filtered_list;
  BLOBNBOX_C_IT filtered_it(&filtered_list);

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward ()) {
    TO_BLOCK* to_block = block_it.data();
    if (to_block->block->poly_block() &&
        !to_block->block->poly_block()->IsText()) continue;
    BLOBNBOX_IT bbox_it;
    bbox_it.set_to_list(&to_block->blobs);
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list ();
         bbox_it.forward ()) {
      BLOBNBOX* bbox = bbox_it.data();
      C_BLOB*   blob = bbox->cblob();
      TBOX      box = blob->bounding_box();
      ++blobs_total;

      float y_x = fabs((box.height() * 1.0) / box.width());
      float x_y = 1.0f / y_x;
      // Select a >= 1.0 ratio
      float ratio = x_y > y_x ? x_y : y_x;
      // Blob is ambiguous
      if (ratio > kSizeRatioToReject) continue;
      if (box.height() < kMinAcceptableBlobHeight) continue;
      filtered_it.add_to_end(bbox);
    }
  }
  return os_detect_blobs(&filtered_list, osr, tess);
}

// Detect orientation and script from a list of blobs.
// Returns a non-zero number of blobs if the list was successfully processed, or
// zero if the list had too few characters to be reliable
int os_detect_blobs(BLOBNBOX_CLIST* blob_list, OSResults* osr,
                    tesseract::Tesseract* tess) {
  OSResults osr_;
  if (osr == NULL)
    osr = &osr_;

  osr->unicharset = &tess->unicharset;
  OrientationDetector o(osr);
  ScriptDetector s(osr, tess);

  BLOBNBOX_C_IT filtered_it(blob_list);
  int real_max = MIN(filtered_it.length(), kMaxCharactersToTry);
  // printf("Total blobs found = %d\n", blobs_total);
  // printf("Number of blobs post-filtering = %d\n", filtered_it.length());
  // printf("Number of blobs to try = %d\n", real_max);

  // If there are too few characters, skip this page entirely.
  if (real_max < kMinCharactersToTry / 2) {
    printf("Too few characters. Skipping this page\n");
    return 0;
  }

  BLOBNBOX** blobs = new BLOBNBOX*[filtered_it.length()];
  int number_of_blobs = 0;
  for (filtered_it.mark_cycle_pt (); !filtered_it.cycled_list ();
       filtered_it.forward ()) {
    blobs[number_of_blobs++] = (BLOBNBOX*)filtered_it.data();
  }
  QRSequenceGenerator sequence(number_of_blobs);
  int num_blobs_evaluated = 0;
  for (int i = 0; i < real_max; ++i) {
    if (os_detect_blob(blobs[sequence.GetVal()], &o, &s, osr, tess)
        && i > kMinCharactersToTry) {
      break;
    }
    ++num_blobs_evaluated;
  }
  delete [] blobs;

  // Make sure the best_result is up-to-date
  int orientation = o.get_orientation();
  osr->update_best_script(orientation);
  return num_blobs_evaluated;
}

// Processes a single blob to estimate script and orientation.
// Return true if estimate of orientation and script satisfies stopping
// criteria.
bool os_detect_blob(BLOBNBOX* bbox, OrientationDetector* o,
                    ScriptDetector* s, OSResults* osr,
                    tesseract::Tesseract* tess) {
  tess->tess_cn_matching.set_value(true); // turn it on
  tess->tess_bn_matching.set_value(false);
  C_BLOB* blob = bbox->cblob();
  TBLOB* tblob = TBLOB::PolygonalCopy(blob);
  TBOX box = tblob->bounding_box();
  FCOORD current_rotation(1.0f, 0.0f);
  FCOORD rotation90(0.0f, 1.0f);
  BLOB_CHOICE_LIST ratings[4];
  // Test the 4 orientations
  for (int i = 0; i < 4; ++i) {
    // Normalize the blob. Set the origin to the place we want to be the
    // bottom-middle after rotation.
    // Scaling is to make the rotated height the x-height.
    float scaling = static_cast<float>(kBlnXHeight) / box.height();
    float x_origin = (box.left() + box.right()) / 2.0f;
    float y_origin = (box.bottom() + box.top()) / 2.0f;
    if (i == 0 || i == 2) {
      // Rotation is 0 or 180.
      y_origin = i == 0 ? box.bottom() : box.top();
    } else {
      // Rotation is 90 or 270.
      scaling = static_cast<float>(kBlnXHeight) / box.width();
      x_origin = i == 1 ? box.left() : box.right();
    }
    DENORM denorm;
    denorm.SetupNormalization(NULL, NULL, &current_rotation, NULL, NULL, 0,
                              x_origin, y_origin, scaling, scaling,
                              0.0f, static_cast<float>(kBlnBaselineOffset));
    TBLOB* rotated_blob = new TBLOB(*tblob);
    rotated_blob->Normalize(denorm);
    tess->set_denorm(&denorm);
    tess->AdaptiveClassifier(rotated_blob, ratings + i, NULL);
    delete rotated_blob;
    current_rotation.rotate(rotation90);
  }
  delete tblob;

  bool stop = o->detect_blob(ratings);
  s->detect_blob(ratings);
  int orientation = o->get_orientation();
  stop = s->must_stop(orientation) && stop;
  return stop;
}


OrientationDetector::OrientationDetector(OSResults* osr) {
  osr_ = osr;
}

// Score the given blob and return true if it is now sure of the orientation
// after adding this block.
bool OrientationDetector::detect_blob(BLOB_CHOICE_LIST* scores) {
  float blob_o_score[4] = {0.0, 0.0, 0.0, 0.0};
  float total_blob_o_score = 0.0;

  for (int i = 0; i < 4; ++i) {
    BLOB_CHOICE_IT choice_it;
    choice_it.set_to_list(scores + i);
    if (!choice_it.empty()) {
      // The certainty score ranges between [-20,0]. This is converted here to
      // [0,1], with 1 indicating best match.
      blob_o_score[i] = 1 + 0.05 * choice_it.data()->certainty();
      total_blob_o_score += blob_o_score[i];
    }
  }
  // Normalize the orientation scores for the blob and use them to
  // update the aggregated orientation score.
  for (int i = 0; total_blob_o_score != 0 && i < 4; ++i) {
    osr_->orientations[i] += log(blob_o_score[i] / total_blob_o_score);
  }

  float first = -1;
  float second = -1;

  int idx = -1;
  for (int i = 0; i < 4; ++i) {
    if (osr_->orientations[i] > first) {
      idx = i;
      second = first;
      first = osr_->orientations[i];
    } else if (osr_->orientations[i] > second) {
      second = osr_->orientations[i];
    }
  }

  return first / second > kOrientationAcceptRatio;
}

int OrientationDetector::get_orientation() {
  osr_->update_best_orientation();
  return osr_->best_result.orientation_id;
}


ScriptDetector::ScriptDetector(OSResults* osr, tesseract::Tesseract* tess) {
  osr_ = osr;
  tess_ = tess;
  katakana_id_ = tess_->unicharset.add_script(katakana_script);
  hiragana_id_ = tess_->unicharset.add_script(hiragana_script);
  han_id_ = tess_->unicharset.add_script(han_script);
  hangul_id_ = tess_->unicharset.add_script(hangul_script);
  japanese_id_ = tess_->unicharset.add_script(japanese_script_);
  korean_id_ = tess_->unicharset.add_script(korean_script_);
  latin_id_ = tess_->unicharset.add_script(latin_script);
  fraktur_id_ = tess_->unicharset.add_script(fraktur_script_);
}


// Score the given blob and return true if it is now sure of the script after
// adding this blob.
void ScriptDetector::detect_blob(BLOB_CHOICE_LIST* scores) {
  bool done[kMaxNumberOfScripts];
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < kMaxNumberOfScripts; ++j)
      done[j] = false;

    BLOB_CHOICE_IT choice_it;
    choice_it.set_to_list(scores + i);

    float prev_score = -1;
    int script_count = 0;
    int prev_id = -1;
    int prev_script;
    int prev_class_id = -1;
    int prev_fontinfo_id = -1;
    const char* prev_unichar = "";
    const char* unichar = "";
    float next_best_score = -1.0;
    int next_best_script_id = -1;
    const char* next_best_unichar = "";

    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
         choice_it.forward()) {
      BLOB_CHOICE* choice = choice_it.data();
      int id = choice->script_id();
      // Script already processed before.
      if (done[id]) continue;
      done[id] = true;

      unichar = tess_->unicharset.id_to_unichar(choice->unichar_id());
      // Save data from the first match
      if (prev_score < 0) {
        prev_score = -choice->certainty();
        script_count = 1;
        prev_id = id;
        prev_script = choice->script_id();
        prev_unichar = unichar;
        prev_class_id = choice->unichar_id();
        prev_fontinfo_id = choice->fontinfo_id();
      } else if (-choice->certainty() < prev_score + kNonAmbiguousMargin) {
        ++script_count;
        next_best_score = -choice->certainty();
        next_best_script_id = choice->script_id();
        next_best_unichar = tess_->unicharset.id_to_unichar(choice->unichar_id());
      }

      if (strlen(prev_unichar) == 1)
        if (unichar[0] >= '0' && unichar[0] <= '9')
          break;

      // if script_count is >= 2, character is ambiguous, skip other matches
      // since they are useless.
      if (script_count >= 2)
        break;
    }
    // Character is non ambiguous
    if (script_count == 1) {
      // Update the score of the winning script
      osr_->scripts_na[i][prev_id] += 1.0;

      // Workaround for Fraktur
      if (prev_id == latin_id_) {
        if (prev_fontinfo_id >= 0) {
          const FontInfo &fi =
              tess_->get_fontinfo_table().get(prev_fontinfo_id);
          //printf("Font: %s i:%i b:%i f:%i s:%i k:%i (%s)\n", fi.name,
          //       fi.is_italic(), fi.is_bold(), fi.is_fixed_pitch(),
          //       fi.is_serif(), fi.is_fraktur(),
          //       prev_unichar);
          if (fi.is_fraktur()) {
            osr_->scripts_na[i][prev_id] -= 1.0;
            osr_->scripts_na[i][fraktur_id_] += 1.0;
          }
        }
      }

      // Update Japanese / Korean pseudo-scripts
      if (prev_id == katakana_id_)
        osr_->scripts_na[i][japanese_id_] += 1.0;
      if (prev_id == hiragana_id_)
        osr_->scripts_na[i][japanese_id_] += 1.0;
      if (prev_id == hangul_id_)
        osr_->scripts_na[i][korean_id_] += 1.0;
      if (prev_id == han_id_)
        osr_->scripts_na[i][korean_id_] += kHanRatioInKorean;
      if (prev_id == han_id_)
        osr_->scripts_na[i][japanese_id_] += kHanRatioInJapanese;
    }
  }  // iterate over each orientation
}

bool ScriptDetector::must_stop(int orientation) {
  osr_->update_best_script(orientation);
  return osr_->best_result.sconfidence > 1;
}

// Helper method to convert an orientation index to its value in degrees.
// The value represents the amount of clockwise rotation in degrees that must be
// applied for the text to be upright (readable).
const int OrientationIdToValue(const int& id) {
  switch (id) {
    case 0:
      return 0;
    case 1:
      return 270;
    case 2:
      return 180;
    case 3:
      return 90;
    default:
      return -1;
  }
}
