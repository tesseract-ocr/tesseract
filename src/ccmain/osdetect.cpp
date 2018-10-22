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

#include <algorithm>
#include <cmath>        // for std::fabs
#include <memory>

#include "osdetect.h"

#include "blobbox.h"
#include "blread.h"
#include "colfind.h"
#include "fontinfo.h"
#include "imagefind.h"
#include "linefind.h"
#include "oldlist.h"
#include "qrsequence.h"
#include "ratngs.h"
#include "strngs.h"
#include "tabvector.h"
#include "tesseractclass.h"
#include "textord.h"

const float kSizeRatioToReject = 2.0;
const int kMinAcceptableBlobHeight = 10;

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

int OSResults::get_best_script(int orientation_id) const {
  int max_id = -1;
  for (int j = 0; j < kMaxNumberOfScripts; ++j) {
    const char *script = unicharset->get_script_from_script_id(j);
    if (strcmp(script, "Common") && strcmp(script, "NULL")) {
      if (max_id == -1 ||
          scripts_na[orientation_id][j] > scripts_na[orientation_id][max_id])
        max_id = j;
    }
  }
  return max_id;
}

// Print the script scores for all possible orientations.
void OSResults::print_scores(void) const {
  for (int i = 0; i < 4; ++i) {
    tprintf("Orientation id #%d", i);
    print_scores(i);
  }
}

// Print the script scores for the given candidate orientation.
void OSResults::print_scores(int orientation_id) const {
  for (int j = 0; j < kMaxNumberOfScripts; ++j) {
    if (scripts_na[orientation_id][j]) {
      tprintf("%12s\t: %f\n", unicharset->get_script_from_script_id(j),
             scripts_na[orientation_id][j]);
    }
  }
}

// Accumulate scores with given OSResults instance and update the best script.
void OSResults::accumulate(const OSResults& osr) {
  for (int i = 0; i < 4; ++i) {
    orientations[i] += osr.orientations[i];
    for (int j = 0; j < kMaxNumberOfScripts; ++j)
      scripts_na[i][j] += osr.scripts_na[i][j];
  }
  unicharset = osr.unicharset;
  update_best_orientation();
  update_best_script(best_result.orientation_id);
}

// Detect and erase horizontal/vertical lines and picture regions from the
// image, so that non-text blobs are removed from consideration.
static void remove_nontext_regions(tesseract::Tesseract *tess,
                                   BLOCK_LIST *blocks,
                                   TO_BLOCK_LIST *to_blocks) {
  Pix *pix = tess->pix_binary();
  ASSERT_HOST(pix != nullptr);
  int vertical_x = 0;
  int vertical_y = 1;
  tesseract::TabVector_LIST v_lines;
  tesseract::TabVector_LIST h_lines;
  int resolution;
  if (kMinCredibleResolution > pixGetXRes(pix)) {
    resolution = kMinCredibleResolution;
    tprintf("Warning. Invalid resolution %d dpi. Using %d instead.\n",
            pixGetXRes(pix), resolution);
  } else {
    resolution = pixGetXRes(pix);
  }

  tesseract::LineFinder::FindAndRemoveLines(resolution, false, pix,
                                            &vertical_x, &vertical_y,
                                            nullptr, &v_lines, &h_lines);
  Pix* im_pix = tesseract::ImageFind::FindImages(pix, nullptr);
  if (im_pix != nullptr) {
    pixSubtract(pix, pix, im_pix);
    pixDestroy(&im_pix);
  }
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
  if (lastdot != nullptr)
    name[lastdot-name.string()] = '\0';

  ASSERT_HOST(tess->pix_binary() != nullptr)
  int width = pixGetWidth(tess->pix_binary());
  int height = pixGetHeight(tess->pix_binary());

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
    if (to_block->block->pdblk.poly_block() &&
        !to_block->block->pdblk.poly_block()->IsText()) continue;
    BLOBNBOX_IT bbox_it;
    bbox_it.set_to_list(&to_block->blobs);
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list ();
         bbox_it.forward ()) {
      BLOBNBOX* bbox = bbox_it.data();
      C_BLOB*   blob = bbox->cblob();
      TBOX      box = blob->bounding_box();
      ++blobs_total;

      // Catch illegal value of box width and avoid division by zero.
      if (box.width() == 0) continue;
      // TODO: Can height and width be negative? If not, remove fabs.
      float y_x = std::fabs((box.height() * 1.0f) / box.width());
      float x_y = 1.0f / y_x;
      // Select a >= 1.0 ratio
      float ratio = x_y > y_x ? x_y : y_x;
      // Blob is ambiguous
      if (ratio > kSizeRatioToReject) continue;
      if (box.height() < kMinAcceptableBlobHeight) continue;
      filtered_it.add_to_end(bbox);
    }
  }
  return os_detect_blobs(nullptr, &filtered_list, osr, tess);
}

// Detect orientation and script from a list of blobs.
// Returns a non-zero number of blobs if the list was successfully processed, or
// zero if the list had too few characters to be reliable.
// If allowed_scripts is non-null and non-empty, it is a list of scripts that
// constrains both orientation and script detection to consider only scripts
// from the list.
int os_detect_blobs(const GenericVector<int>* allowed_scripts,
                    BLOBNBOX_CLIST* blob_list, OSResults* osr,
                    tesseract::Tesseract* tess) {
  OSResults osr_;
  int minCharactersToTry = tess->min_characters_to_try;
  int maxCharactersToTry = 5 * minCharactersToTry;
  if (osr == nullptr)
    osr = &osr_;

  osr->unicharset = &tess->unicharset;
  OrientationDetector o(allowed_scripts, osr);
  ScriptDetector s(allowed_scripts, osr, tess);

  BLOBNBOX_C_IT filtered_it(blob_list);
  int real_max = std::min(filtered_it.length(), maxCharactersToTry);
  // tprintf("Total blobs found = %d\n", blobs_total);
  // tprintf("Number of blobs post-filtering = %d\n", filtered_it.length());
  // tprintf("Number of blobs to try = %d\n", real_max);

  // If there are too few characters, skip this page entirely.
  if (real_max < minCharactersToTry / 2) {
    tprintf("Too few characters. Skipping this page\n");
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
        && i > minCharactersToTry) {
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
  TBLOB* tblob = TBLOB::PolygonalCopy(tess->poly_allow_detailed_fx, blob);
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
    std::unique_ptr<TBLOB> rotated_blob(new TBLOB(*tblob));
    rotated_blob->Normalize(nullptr, &current_rotation, nullptr,
                            x_origin, y_origin, scaling, scaling,
                            0.0f, static_cast<float>(kBlnBaselineOffset),
                            false, nullptr);
    tess->AdaptiveClassifier(rotated_blob.get(), ratings + i);
    current_rotation.rotate(rotation90);
  }
  delete tblob;

  bool stop = o->detect_blob(ratings);
  s->detect_blob(ratings);
  int orientation = o->get_orientation();
  stop = s->must_stop(orientation) && stop;
  return stop;
}


OrientationDetector::OrientationDetector(
    const GenericVector<int>* allowed_scripts, OSResults* osr) {
  osr_ = osr;
  allowed_scripts_ = allowed_scripts;
}

// Score the given blob and return true if it is now sure of the orientation
// after adding this block.
bool OrientationDetector::detect_blob(BLOB_CHOICE_LIST* scores) {
  float blob_o_score[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float total_blob_o_score = 0.0f;

  for (int i = 0; i < 4; ++i) {
    BLOB_CHOICE_IT choice_it(scores + i);
    if (!choice_it.empty()) {
      BLOB_CHOICE* choice = nullptr;
      if (allowed_scripts_ != nullptr && !allowed_scripts_->empty()) {
        // Find the top choice in an allowed script.
        for (choice_it.mark_cycle_pt(); !choice_it.cycled_list() &&
             choice == nullptr; choice_it.forward()) {
          int choice_script = choice_it.data()->script_id();
          int s = 0;
          for (s = 0; s < allowed_scripts_->size(); ++s) {
            if ((*allowed_scripts_)[s] == choice_script) {
              choice = choice_it.data();
              break;
            }
          }
        }
      } else {
        choice = choice_it.data();
      }
      if (choice != nullptr) {
        // The certainty score ranges between [-20,0]. This is converted here to
        // [0,1], with 1 indicating best match.
        blob_o_score[i] = 1 + 0.05 * choice->certainty();
        total_blob_o_score += blob_o_score[i];
      }
    }
  }
  if (total_blob_o_score == 0.0) return false;
  // Fill in any blanks with the worst score of the others. This is better than
  // picking an arbitrary probability for it and way better than -inf.
  float worst_score = 0.0f;
  int num_good_scores = 0;
  for (int i = 0; i < 4; ++i) {
    if (blob_o_score[i] > 0.0f) {
      ++num_good_scores;
      if (worst_score == 0.0f || blob_o_score[i] < worst_score)
        worst_score = blob_o_score[i];
    }
  }
  if (num_good_scores == 1) {
    // Lower worst if there is only one.
    worst_score /= 2.0f;
  }
  for (int i = 0; i < 4; ++i) {
    if (blob_o_score[i] == 0.0f) {
      blob_o_score[i] = worst_score;
      total_blob_o_score += worst_score;
    }
  }
  // Normalize the orientation scores for the blob and use them to
  // update the aggregated orientation score.
  for (int i = 0; total_blob_o_score != 0 && i < 4; ++i) {
    osr_->orientations[i] += log(blob_o_score[i] / total_blob_o_score);
  }

  // TODO(ranjith) Add an early exit test, based on min_orientation_margin,
  // as used in pagesegmain.cpp.
  return false;
}

int OrientationDetector::get_orientation() {
  osr_->update_best_orientation();
  return osr_->best_result.orientation_id;
}


ScriptDetector::ScriptDetector(const GenericVector<int>* allowed_scripts,
                               OSResults* osr, tesseract::Tesseract* tess) {
  osr_ = osr;
  tess_ = tess;
  allowed_scripts_ = allowed_scripts;
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
    int prev_fontinfo_id = -1;
    const char* prev_unichar = "";
    const char* unichar = "";

    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
         choice_it.forward()) {
      BLOB_CHOICE* choice = choice_it.data();
      int id = choice->script_id();
      if (allowed_scripts_ != nullptr && !allowed_scripts_->empty()) {
        // Check that the choice is in an allowed script.
        int s = 0;
        for (s = 0; s < allowed_scripts_->size(); ++s) {
          if ((*allowed_scripts_)[s] == id) break;
        }
        if (s == allowed_scripts_->size()) continue;  // Not found in list.
      }
      // Script already processed before.
      if (done[id]) continue;
      done[id] = true;

      unichar = tess_->unicharset.id_to_unichar(choice->unichar_id());
      // Save data from the first match
      if (prev_score < 0) {
        prev_score = -choice->certainty();
        script_count = 1;
        prev_id = id;
        prev_unichar = unichar;
        prev_fontinfo_id = choice->fontinfo_id();
      } else if (-choice->certainty() < prev_score + kNonAmbiguousMargin) {
        ++script_count;
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
          const tesseract::FontInfo &fi =
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
      if (prev_id == han_id_) {
        osr_->scripts_na[i][korean_id_] += kHanRatioInKorean;
        osr_->scripts_na[i][japanese_id_] += kHanRatioInJapanese;
      }
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
int OrientationIdToValue(const int& id) {
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
