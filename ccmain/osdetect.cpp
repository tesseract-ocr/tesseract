///////////////////////////////////////////////////////////////////////
// File:        osdetect.cpp
// Description: Orientation and script detection.
// Author:      Samuel Charron
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

#include "strngs.h"
#include "blobbox.h"
#include "blread.h"
#include "tordmain.h"
#include "ratngs.h"
#include "oldlist.h"
#include "adaptmatch.h"
#include "tstruct.h"
#include "expandblob.h"
#include "tesseractclass.h"
#include "qrsequence.h"

extern IMAGE page_image;

const int kMinCharactersToTry = 50;
const int kMaxCharactersToTry = 5 * kMinCharactersToTry;

const float kSizeRatioToReject = 2.0;

const float kOrientationAcceptRatio = 1.3;
const float kScriptAcceptRatio = 1.3;

const float kHanRatioInKorean = 0.7;
const float kHanRatioInJapanese = 0.3;

const float kLatinRationInFraktur = 0.7;

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

CLISTIZEH(BLOBNBOX);
CLISTIZE(BLOBNBOX);

// Find connected components in the page and process a subset until finished or
// a stopping criterion is met.
// Returns true if the page was successfully processed.
bool orientation_and_script_detection(STRING& filename,
                                      OSResults* osr,
                                      tesseract::Tesseract* tess) {
  STRING name = filename;        //truncated name
  const char *lastdot;           //of name
  TO_BLOCK_LIST land_blocks, port_blocks;
  BLOCK_LIST blocks;
  TBOX page_box;

  lastdot = strrchr (name.string (), '.');
  if (lastdot != NULL)
    name[lastdot-name.string()] = '\0';
  if (!read_unlv_file(name, page_image.get_xsize(), page_image.get_ysize(),
                     &blocks))
    FullPageBlock(page_image.get_xsize(), page_image.get_ysize(), &blocks);
  find_components(&blocks, &land_blocks, &port_blocks, &page_box);
  return os_detect(&port_blocks, osr, tess);
}

// Filter and sample the blobs.
// Returns true if the page was successfully processed, or false if the page had
// too few characters to be reliable
bool os_detect(TO_BLOCK_LIST* port_blocks, OSResults* osr,
               tesseract::Tesseract* tess) {
  int blobs_total = 0;
  OSResults osr_;
  if (osr == NULL)
    osr = &osr_;

  osr->unicharset = &tess->unicharset;
  OrientationDetector o(osr);
  ScriptDetector s(osr, tess);

  TO_BLOCK_IT block_it;
  block_it.set_to_list(port_blocks);

  BLOBNBOX_CLIST filtered_list;
  BLOBNBOX_C_IT filtered_it(&filtered_list);

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward ()) {
    TO_BLOCK* block = block_it.data();
    BLOBNBOX_IT bbox_it;
    bbox_it.set_to_list(&block->blobs);
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
      if (box.height() < 10) continue;
      filtered_it.add_to_end(bbox);
    }
  }
  if (filtered_it.length() > 0)
    filtered_it.move_to_first();

  int real_max = MIN(filtered_it.length(), kMaxCharactersToTry);
   printf("Total blobs found = %d\n", blobs_total);
   printf("Number of blobs post-filtering = %d\n", filtered_it.length());
   printf("Number of blobs to try = %d\n", real_max);

  // If there are too few characters, skip this page entirely.
  if (real_max < kMinCharactersToTry / 2) {
    printf("Too few characters. Skipping this page\n");
    return false;
  }

  BLOBNBOX** blobs = new BLOBNBOX*[filtered_it.length()];
  int number_of_blobs = 0;
  for (filtered_it.mark_cycle_pt (); !filtered_it.cycled_list ();
       filtered_it.forward ()) {
    blobs[number_of_blobs++] = (BLOBNBOX*)filtered_it.data();
  }
  QRSequenceGenerator sequence(number_of_blobs);
  for (int i = 0; i < real_max; ++i) {
    if (os_detect_blob(blobs[sequence.GetVal()], &o, &s, osr, tess)
        && i > kMinCharactersToTry) {
      break;
    }
  }
  delete [] blobs;

  // Make sure the best_result is up-to-date
  int orientation = o.get_orientation();
  s.update_best_script(orientation);
  return true;
}

// Processes a single blob to estimate script and orientation.
// Return true if estimate of orientation and script satisfies stopping
// criteria.
bool os_detect_blob(BLOBNBOX* bbox, OrientationDetector* o,
                    ScriptDetector* s, OSResults* osr,
                    tesseract::Tesseract* tess) {
  C_BLOB*   blob = bbox->cblob();
  TBOX      box = blob->bounding_box();

  int       x_mid = (box.left() + box.right()) / 2.0f;
  int       y_mid = (box.bottom() + box.top()) / 2.0f;

  PBLOB     pblob(blob, box.height());

  BLOB_CHOICE_LIST ratings[4];
  // Test the 4 orientations
  for (int i = 0; i < 4; ++i) {
    // normalize the blob
    pblob.move(FCOORD(-x_mid, -box.bottom()));
    pblob.scale(static_cast<float>(bln_x_height) / box.height());
    pblob.move(FCOORD(0.0f, bln_baseline_offset));

    {
      // List of choices given by the classifier
      TBLOB *tessblob;               //converted blob
      TEXTROW tessrow;               //dummy row

      tess_cn_matching.set_value(true); // turn it on
      tess_bn_matching.set_value(false);
      //convert blob
      tessblob = make_tess_blob (&pblob, TRUE);
      //make dummy row
      make_tess_row(NULL, &tessrow);
      //classify
      tess->AdaptiveClassifier (tessblob, NULL, &tessrow, ratings + i, NULL);
      free_blob(tessblob);
    }
    // undo normalize
    pblob.move(FCOORD(0.0f, -bln_baseline_offset));
    pblob.scale(1.0f / (static_cast<float>(bln_x_height) / box.height()));
    pblob.move(FCOORD(x_mid, box.bottom()));

    // center the blob
    pblob.move(FCOORD(-x_mid, -y_mid));

    // Rotate it
    pblob.rotate();

    // Re-compute the mid
    box = pblob.bounding_box();
    x_mid = (box.left() + box.right()) / 2;
    y_mid = (box.top() + box.bottom()) / 2;

    // re-center in the new mid
    pblob.move(FCOORD(x_mid, y_mid));
  }

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
  for (int i = 0; i < 4; ++i) {
    BLOB_CHOICE_IT choice_it;
    choice_it.set_to_list(scores + i);

    if (!choice_it.empty()) {
      osr_->orientations[i] += (100 + choice_it.data()->certainty());
    }
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

void OrientationDetector::update_best_orientation() {
  float first = osr_->orientations[0];
  float second = osr_->orientations[1];

  if (first < second) {
    second = first;
    first = osr_->orientations[1];
  }

  osr_->best_result.orientation = 0;
  osr_->best_result.oconfidence = 0;

  for (int i = 0; i < 4; ++i) {
    if (osr_->orientations[i] > first) {
      second = first;
      first = osr_->orientations[i];
      osr_->best_result.orientation = i;
    } else if (osr_->orientations[i] > second) {
      second = osr_->orientations[i];
    }
  }

  osr_->best_result.oconfidence =
      (first / second - 1.0) / (kOrientationAcceptRatio - 1.0);
}

int OrientationDetector::get_orientation() {
  update_best_orientation();
  return osr_->best_result.orientation;
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
    int prev_config = -1;
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
        prev_config = choice->config();
      } else if (-choice->certainty() < prev_score + kNonAmbiguousMargin) {
        script_count++;
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
      osr_->scripts_na[i][prev_id] += 1;

      // Workaround for Fraktur
      if (prev_id == latin_id_) {
        int font_set_id = tess_->PreTrainedTemplates->
            Class[prev_class_id]->font_set_id;
        if (font_set_id >= 0 && prev_config >= 0) {
          FontInfo fi = tess_->get_fontinfo_table().get(
              tess_->get_fontset_table().get(font_set_id).configs[prev_config]);
          //printf("Font: %s i:%i b:%i f:%i s:%i k:%i (%s)\n", fi.name,
          //       fi.is_italic(), fi.is_bold(), fi.is_fixed_pitch(),
          //       fi.is_serif(), fi.is_fraktur(),
          //       prev_unichar);
          if (fi.is_fraktur()) {
            osr_->scripts_na[i][prev_id] -= 1;
            osr_->scripts_na[i][fraktur_id_] += 1;
          }
        }
      }

      // Update Japanese / Korean pseudo-scripts
      if (prev_id == katakana_id_)
        osr_->scripts_na[i][japanese_id_] += 1;
      if (prev_id == hiragana_id_)
        osr_->scripts_na[i][japanese_id_] += 1;
      if (prev_id == hangul_id_)
        osr_->scripts_na[i][korean_id_] += 1;
      if (prev_id == han_id_)
        osr_->scripts_na[i][korean_id_] += kHanRatioInKorean;
      if (prev_id == han_id_)
        osr_->scripts_na[i][japanese_id_] += kHanRatioInJapanese;
    }
  }  // iterate over each orientation
}

bool ScriptDetector::must_stop(int orientation) {
  update_best_script(orientation);
  return osr_->best_result.sconfidence > 1;
}


void ScriptDetector::update_best_script(int orientation) {
  float first = -1;
  float second = -1;

  // i = 1 -> ignore Common scripts
  for (int i = 1; i < kMaxNumberOfScripts; ++i) {
    if (osr_->scripts_na[orientation][i] > first) {
      osr_->best_result.script =
          tess_->unicharset.get_script_from_script_id(i);
      second = first;
      first = osr_->scripts_na[orientation][i];
    } else if (osr_->scripts_na[orientation][i] > second) {
      second = osr_->scripts_na[orientation][i];
    }
  }

  osr_->best_result.sconfidence =
      (first / second - 1.0) / (kOrientationAcceptRatio - 1.0);
}
