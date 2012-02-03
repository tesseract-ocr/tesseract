///////////////////////////////////////////////////////////////////////
// File:        equationdetect.cpp
// Description: Helper classes to detect equations.
// Author:      Zongyi (Joe) Liu (joeliu@google.com)
// Created:     Fri Aug 31 11:13:01 PST 2011
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#include "mathfix.h"
#endif

#ifdef __MINGW32__
#include <limits.h>
#endif

#include <float.h>

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "equationdetect.h"

#include "bbgrid.h"
#include "classify.h"
#include "colpartition.h"
#include "colpartitiongrid.h"
#include "colpartitionset.h"
#include "helpers.h"
#include "ratngs.h"
#include "tesseractclass.h"

// Config variables.
BOOL_VAR(equationdetect_save_bi_image, false, "Save input bi image");
BOOL_VAR(equationdetect_save_spt_image, false, "Save special character image");
BOOL_VAR(equationdetect_save_seed_image, false, "Save the seed image");
BOOL_VAR(equationdetect_save_merged_image, false, "Save the merged image");

namespace tesseract {

///////////////////////////////////////////////////////////////////////////
// Utility ColParition sort functions.
///////////////////////////////////////////////////////////////////////////
static int SortCPByTopReverse(const void* p1, const void* p2) {
  const ColPartition* cp1 = *reinterpret_cast<ColPartition* const*>(p1);
  const ColPartition* cp2 = *reinterpret_cast<ColPartition* const*>(p2);
  ASSERT_HOST(cp1 != NULL && cp2 != NULL);
  const TBOX &box1(cp1->bounding_box()), &box2(cp2->bounding_box());
  return box2.top() - box1.top();
}

static int SortCPByBottom(const void* p1, const void* p2) {
  const ColPartition* cp1 = *reinterpret_cast<ColPartition* const*>(p1);
  const ColPartition* cp2 = *reinterpret_cast<ColPartition* const*>(p2);
  ASSERT_HOST(cp1 != NULL && cp2 != NULL);
  const TBOX &box1(cp1->bounding_box()), &box2(cp2->bounding_box());
  return box1.bottom() - box2.bottom();
}

static int SortCPByHeight(const void* p1, const void* p2) {
  const ColPartition* cp1 = *reinterpret_cast<ColPartition* const*>(p1);
  const ColPartition* cp2 = *reinterpret_cast<ColPartition* const*>(p2);
  ASSERT_HOST(cp1 != NULL && cp2 != NULL);
  const TBOX &box1(cp1->bounding_box()), &box2(cp2->bounding_box());
  return box1.height() - box2.height();
}

// TODO(joeliu): we may want to parameterize these constants.
const float kMathDigitDensityTh1 = 0.25;
const float kMathDigitDensityTh2 = 0.1;
const float kMathItalicDensityTh = 0.5;
const float kUnclearDensityTh = 0.25;
const int kSeedBlobsCountTh = 10;
const int kLeftIndentAlignmentCountTh = 1;

// Returns true if PolyBlockType is of text type or equation type.
inline bool IsTextOrEquationType(PolyBlockType type) {
  return PTIsTextType(type) || type == PT_EQUATION;
}

inline bool IsLeftIndented(const EquationDetect::IndentType type) {
  return type == EquationDetect::LEFT_INDENT ||
      type == EquationDetect::BOTH_INDENT;
}

inline bool IsRightIndented(const EquationDetect::IndentType type) {
  return type == EquationDetect::RIGHT_INDENT ||
      type == EquationDetect::BOTH_INDENT;
}

EquationDetect::EquationDetect(const char* equ_datapath,
                               const char* equ_name) {
  const char* default_name = "equ";
  if (equ_name == NULL) {
    equ_name = default_name;
  }
  equ_tesseract_ = lang_tesseract_ = NULL;
  resolution_ = 0;
  page_count_ = 0;

  // Construct equ_tesseract_.
  equ_tesseract_ = new Tesseract();
  if (equ_tesseract_->init_tesseract(equ_datapath, equ_name,
                                     OEM_TESSERACT_ONLY)) {
    tprintf("Warning: equation region detection requested,"
            " but %s failed to load from %s\n", equ_name, equ_datapath);
    delete equ_tesseract_;
    equ_tesseract_ = NULL;
  }

  cps_super_bbox_ = NULL;
}

EquationDetect::~EquationDetect() {
  if (equ_tesseract_) {
    delete (equ_tesseract_);
  }
  if (cps_super_bbox_) {
    delete(cps_super_bbox_);
  }
}

void EquationDetect::SetLangTesseract(Tesseract* lang_tesseract) {
  lang_tesseract_ = lang_tesseract;
}

void EquationDetect::SetResolution(const int resolution) {
  resolution_ = resolution;
}

int EquationDetect::LabelSpecialText(TO_BLOCK* to_block) {
  if (to_block == NULL) {
    tprintf("Warning: input to_block is NULL!\n");
    return -1;
  }

  GenericVector<BLOBNBOX_LIST*> blob_lists;
  blob_lists.push_back(&(to_block->blobs));
  blob_lists.push_back(&(to_block->large_blobs));
  for (int i = 0; i < blob_lists.size(); ++i) {
    BLOBNBOX_IT bbox_it(blob_lists[i]);
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
         bbox_it.forward()) {
      bbox_it.data()->set_special_text_type(BSTT_NONE);
    }
  }

  return 0;
}

void EquationDetect::IdentifySpecialText(
    BLOBNBOX *blobnbox, const int height_th) {
  ASSERT_HOST(blobnbox != NULL);
  if (blobnbox->bounding_box().height() < height_th && height_th > 0) {
    // For small blob, we simply set to BSTT_NONE.
    blobnbox->set_special_text_type(BSTT_NONE);
    return;
  }

  BLOB_CHOICE_LIST ratings_equ, ratings_lang;
  C_BLOB* blob = blobnbox->cblob();
  TBLOB* tblob = TBLOB::PolygonalCopy(blob);
  const TBOX& box = tblob->bounding_box();

  // Normalize the blob. Set the origin to the place we want to be the
  // bottom-middle, and scaling is to make the height the x-height.
  float scaling = static_cast<float>(kBlnXHeight) / box.height();
  DENORM denorm;
  float x_orig = (box.left() + box.right()) / 2.0f, y_orig = box.bottom();
  denorm.SetupNormalization(NULL, NULL, NULL, NULL, NULL, 0,
                            x_orig, y_orig, scaling, scaling,
                            0.0f, static_cast<float>(kBlnBaselineOffset));
  TBLOB* normed_blob = new TBLOB(*tblob);
  normed_blob->Normalize(denorm);
  equ_tesseract_->AdaptiveClassifier(normed_blob, denorm, &ratings_equ, NULL);
  lang_tesseract_->AdaptiveClassifier(normed_blob, denorm, &ratings_lang, NULL);
  delete normed_blob;
  delete tblob;

  // Get the best choice from ratings_lang and rating_equ. As the choice in the
  // list has already been sorted by the certainty, we simply use the first
  // choice.
  BLOB_CHOICE *lang_choice = NULL, *equ_choice = NULL;
  if (ratings_lang.length() > 0) {
    BLOB_CHOICE_IT choice_it(&ratings_lang);
    lang_choice = choice_it.data();
  }
  if (ratings_equ.length() > 0) {
    BLOB_CHOICE_IT choice_it(&ratings_equ);
    equ_choice = choice_it.data();
  }

  float lang_score = lang_choice ? lang_choice->certainty() : -FLT_MAX;
  float equ_score = equ_choice ? equ_choice->certainty() : -FLT_MAX;

  const float kConfScoreTh = -5.0f, kConfDiffTh = 1.8;
  // The scores here are negative, so the max/min == fabs(min/max).
  // float ratio = fmax(lang_score, equ_score) / fmin(lang_score, equ_score);
  float diff = fabs(lang_score - equ_score);
  BlobSpecialTextType type = BSTT_NONE;

  // Classification.
  if (fmax(lang_score, equ_score) < kConfScoreTh) {
    // If both score are very small, then mark it as unclear.
    type = BSTT_UNCLEAR;
  } else if (diff > kConfDiffTh && equ_score > lang_score) {
    // If equ_score is significantly higher, then we classify this character as
    // math symbol.
    type = BSTT_MATH;
  } else if (lang_choice) {
    // For other cases: lang_score is similar or significantly higher.
    type = EstimateTypeForUnichar(
        lang_tesseract_->unicharset, lang_choice->unichar_id());
  }

  if (type == BSTT_NONE && lang_tesseract_->get_fontinfo_table().get(
      lang_choice->fontinfo_id()).is_italic()) {
    // For text symbol, we still check if it is italic.
    blobnbox->set_special_text_type(BSTT_ITALIC);
  } else {
    blobnbox->set_special_text_type(type);
  }
}

BlobSpecialTextType EquationDetect::EstimateTypeForUnichar(
    const UNICHARSET& unicharset, const UNICHAR_ID id) const {
  STRING s = unicharset.id_to_unichar(id);
  if (unicharset.get_isalpha(id)) {
    return BSTT_NONE;
  }

  if (unicharset.get_ispunctuation(id)) {
    // Exclude some special texts that are likely to be confused as math symbol.
    static GenericVector<UNICHAR_ID> ids_to_exclude;
    if (ids_to_exclude.empty()) {
      static const STRING kCharsToEx[] = {"'", "`", "\"", "\\", ",", ".",
          "〈", "〉", "《", "》", "」", "「", ""};
      int i = 0;
      while (kCharsToEx[i] != "") {
        ids_to_exclude.push_back(
            unicharset.unichar_to_id(kCharsToEx[i++].string()));
      }
      ids_to_exclude.sort();
    }
    return ids_to_exclude.bool_binary_search(id) ? BSTT_NONE : BSTT_MATH;
  }

  // Check if it is digit. In addition to the isdigit attribute, we also check
  // if this character belongs to those likely to be confused with a digit.
  static const STRING kDigitsChars = "|";
  if (unicharset.get_isdigit(id) ||
      (s.length() == 1 && kDigitsChars.contains(s[0]))) {
    return BSTT_DIGIT;
  } else  {
    return BSTT_MATH;
  }
}

void EquationDetect::IdentifySpecialText() {
  // Set configuration for Tesseract::AdaptiveClassifier.
  equ_tesseract_->tess_cn_matching.set_value(true);  // turn it on
  equ_tesseract_->tess_bn_matching.set_value(false);

  // Set the multiplier to zero for lang_tesseract_ to improve the accuracy.
  int classify_class_pruner = lang_tesseract_->classify_class_pruner_multiplier;
  int classify_integer_matcher =
      lang_tesseract_->classify_integer_matcher_multiplier;
  lang_tesseract_->classify_class_pruner_multiplier.set_value(0);
  lang_tesseract_->classify_integer_matcher_multiplier.set_value(0);

  ColPartitionGridSearch gsearch(part_grid_);
  ColPartition *part = NULL;
  gsearch.StartFullSearch();
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (!IsTextOrEquationType(part->type())) {
      continue;
    }
    IdentifyBlobsToSkip(part);
    BLOBNBOX_C_IT bbox_it(part->boxes());
    // Compute the height threshold.
    GenericVector<int> blob_heights;
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
         bbox_it.forward()) {
      if (bbox_it.data()->special_text_type() != BSTT_SKIP) {
        blob_heights.push_back(bbox_it.data()->bounding_box().height());
      }
    }
    blob_heights.sort();
    int height_th =  blob_heights[blob_heights.size() / 2] / 3 * 2;
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
         bbox_it.forward()) {
      if (bbox_it.data()->special_text_type() != BSTT_SKIP) {
        IdentifySpecialText(bbox_it.data(), height_th);
      }
    }
  }

  // Set the multiplier values back.
  lang_tesseract_->classify_class_pruner_multiplier.set_value(
      classify_class_pruner);
  lang_tesseract_->classify_integer_matcher_multiplier.set_value(
      classify_integer_matcher);

  if (equationdetect_save_spt_image) {  // For debug.
    STRING outfile;
    GetOutputTiffName("_spt", &outfile);
    PaintSpecialTexts(outfile);
  }
}

void EquationDetect::IdentifyBlobsToSkip(ColPartition* part) {
  ASSERT_HOST(part);
  BLOBNBOX_C_IT blob_it(part->boxes());

  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    // At this moment, no blob should have been joined.
    ASSERT_HOST(!blob_it.data()->joined_to_prev());
  }
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->joined_to_prev() || blob->special_text_type() == BSTT_SKIP) {
      continue;
    }
    TBOX blob_box = blob->bounding_box();

    // Search if any blob can be merged into blob. If found, then we mark all
    // these blobs as BSTT_SKIP.
    BLOBNBOX_C_IT blob_it2 = blob_it;
    bool found = false;
    while (!blob_it2.at_last()) {
      BLOBNBOX* nextblob = blob_it2.forward();
      const TBOX& nextblob_box = nextblob->bounding_box();
      if (nextblob_box.left() >= blob_box.right()) {
        break;
      }
      const float kWidthR = 0.4, kHeightR = 0.3;
      bool xoverlap = blob_box.major_x_overlap(nextblob_box),
          yoverlap = blob_box.y_overlap(nextblob_box);
      float widthR = static_cast<float>(
          MIN(nextblob_box.width(), blob_box.width())) /
          MAX(nextblob_box.width(), blob_box.width());
      float heightR = static_cast<float>(
          MIN(nextblob_box.height(), blob_box.height())) /
          MAX(nextblob_box.height(), blob_box.height());

      if (xoverlap && yoverlap && widthR > kWidthR && heightR > kHeightR) {
        // Found one, set nextblob type and recompute blob_box.
        found = true;
        nextblob->set_special_text_type(BSTT_SKIP);
        blob_box += nextblob_box;
      }
    }
    if (found) {
      blob->set_special_text_type(BSTT_SKIP);
    }
  }
}

int EquationDetect::FindEquationParts(
    ColPartitionGrid* part_grid, ColPartitionSet** best_columns) {
  if (!equ_tesseract_ || !lang_tesseract_) {
    tprintf("Warning: equ_tesseract_/lang_tesseract_ is NULL!\n");
    return -1;
  }
  if (!part_grid || !best_columns) {
    tprintf("part_grid/best_columns is NULL!!\n");
    return -1;
  }
  cp_seeds_.clear();
  part_grid_ = part_grid;
  best_columns_ = best_columns;
  resolution_ = lang_tesseract_->source_resolution();
  STRING outfile;
  page_count_++;

  if (equationdetect_save_bi_image) {
    GetOutputTiffName("_bi", &outfile);
    pixWrite(outfile.string(), lang_tesseract_->pix_binary(), IFF_TIFF_G4);
  }

  // Pass 0: Compute special text type for blobs.
  IdentifySpecialText();

  // Pass 1: Merge parts by overlap.
  MergePartsByLocation();

  // Pass 2: compute the math blob density and find the seed partition.
  IdentifySeedParts();
  // We still need separate seed into block seed and inline seed partition.
  IdentifyInlineParts();

  if (equationdetect_save_seed_image) {
    GetOutputTiffName("_seed", &outfile);
    PaintColParts(outfile);
  }

  // Pass 3: expand block equation seeds.
  while (!cp_seeds_.empty()) {
    GenericVector<ColPartition*> seeds_expanded;
    for (int i = 0; i < cp_seeds_.size(); ++i) {
      if (ExpandSeed(cp_seeds_[i])) {
        // If this seed is expanded, then we add it into seeds_expanded. Note
        // this seed has been removed from part_grid_ if it is expanded.
        seeds_expanded.push_back(cp_seeds_[i]);
      }
    }
    // Add seeds_expanded back into part_grid_ and reset cp_seeds_.
    for (int i = 0; i < seeds_expanded.size(); ++i) {
      InsertPartAfterAbsorb(seeds_expanded[i]);
    }
    cp_seeds_ = seeds_expanded;
  }

  // Pass 4: find math block satellite text partitions and merge them.
  ProcessMathBlockSatelliteParts();

  if (equationdetect_save_merged_image) {  // For debug.
    GetOutputTiffName("_merged", &outfile);
    PaintColParts(outfile);
  }

  return 0;
}

void EquationDetect::MergePartsByLocation() {
  while (true) {
    ColPartition* part = NULL;
    // partitions that have been updated.
    GenericVector<ColPartition*> parts_updated;
    ColPartitionGridSearch gsearch(part_grid_);
    gsearch.StartFullSearch();
    while ((part = gsearch.NextFullSearch()) != NULL) {
      if (!IsTextOrEquationType(part->type())) {
        continue;
      }
      GenericVector<ColPartition*> parts_to_merge;
      SearchByOverlap(part, &parts_to_merge);
      if (parts_to_merge.empty()) {
        continue;
      }

      // Merge parts_to_merge with part, and remove them from part_grid_.
      part_grid_->RemoveBBox(part);
      for (int i = 0; i < parts_to_merge.size(); ++i) {
        ASSERT_HOST(parts_to_merge[i] != NULL && parts_to_merge[i] != part);
        part->Absorb(parts_to_merge[i], NULL);
      }
      gsearch.RepositionIterator();

      parts_updated.push_back(part);
    }

    if (parts_updated.empty()) {  // Exit the loop
      break;
    }

    // Re-insert parts_updated into part_grid_.
    for (int i = 0; i < parts_updated.size(); ++i) {
      InsertPartAfterAbsorb(parts_updated[i]);
    }
  }
}

void EquationDetect::SearchByOverlap(
    ColPartition* seed,
    GenericVector<ColPartition*>* parts_overlap) {
  ASSERT_HOST(seed != NULL && parts_overlap != NULL);
  if (!IsTextOrEquationType(seed->type())) {
    return;
  }
  ColPartitionGridSearch search(part_grid_);
  const TBOX& seed_box(seed->bounding_box());
  const int kRadNeighborCells = 30;
  search.StartRadSearch((seed_box.left() + seed_box.right()) / 2,
                        (seed_box.top() + seed_box.bottom()) / 2,
                        kRadNeighborCells);
  search.SetUniqueMode(true);

  // Search iteratively.
  ColPartition *part;
  GenericVector<ColPartition*> parts;
  const float kLargeOverlapTh = 0.95;
  const float kEquXOverlap = 0.4, kEquYOverlap = 0.5;
  while ((part = search.NextRadSearch()) != NULL) {
    if (part == seed || !IsTextOrEquationType(part->type())) {
      continue;
    }
    const TBOX& part_box(part->bounding_box());
    bool merge = false;

    float x_overlap_fraction = part_box.x_overlap_fraction(seed_box),
        y_overlap_fraction = part_box.y_overlap_fraction(seed_box);

    // If part is large overlapped with seed, then set merge to true.
    if (x_overlap_fraction >= kLargeOverlapTh &&
        y_overlap_fraction >= kLargeOverlapTh) {
      merge = true;
    } else if (seed->type() == PT_EQUATION &&
               IsTextOrEquationType(part->type())) {
      if ((x_overlap_fraction > kEquXOverlap && y_overlap_fraction > 0.0) ||
          (x_overlap_fraction > 0.0 && y_overlap_fraction > kEquYOverlap)) {
        merge = true;
      }
    }

    if (merge) {  // Remove the part from search and put it into parts.
      search.RemoveBBox();
      parts_overlap->push_back(part);
    }
  }
}

void EquationDetect::InsertPartAfterAbsorb(ColPartition* part) {
  ASSERT_HOST(part);

  // Before insert part back into part_grid_, we will need re-compute some
  // of its attributes such as first_column_, last_column_. However, we still
  // want to preserve its type.
  BlobTextFlowType flow_type = part->flow();
  PolyBlockType part_type = part->type();
  BlobRegionType blob_type = part->blob_type();

  // Call SetPartitionType to re-compute the attributes of part.
  const TBOX& part_box(part->bounding_box());
  int grid_x, grid_y;
  part_grid_->GridCoords(
      part_box.left(), part_box.bottom(), &grid_x, &grid_y);
  part->SetPartitionType(resolution_, best_columns_[grid_y]);

  // Reset the types back.
  part->set_type(part_type);
  part->set_blob_type(blob_type);
  part->set_flow(flow_type);
  part->SetBlobTypes();

  // Insert into part_grid_.
  part_grid_->InsertBBox(true, true, part);
}

void EquationDetect::IdentifySeedParts() {
  ColPartitionGridSearch gsearch(part_grid_);
  ColPartition *part = NULL;
  gsearch.StartFullSearch();

  GenericVector<ColPartition*> seeds1, seeds2;
  // The left coordinates of indented text partitions.
  GenericVector<int> indented_texts_left;
  // The foreground density of text partitions.
  GenericVector<float> texts_foreground_density;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (!IsTextOrEquationType(part->type())) {
      continue;
    }
    part->ComputeSpecialBlobsDensity();
    bool blobs_check = CheckSeedBlobsCount(part);
    const int kTextBlobsTh = 20;

    if (CheckSeedDensity(kMathDigitDensityTh1, kMathDigitDensityTh2, part) &&
        blobs_check) {
      // Passed high density threshold test, save into seeds1.
      seeds1.push_back(part);
    } else {
      IndentType indent = IsIndented(part);
      if (IsLeftIndented(indent) && blobs_check &&
          CheckSeedDensity(kMathDigitDensityTh2, kMathDigitDensityTh2, part)) {
        // Passed low density threshold test and is indented, save into seeds2.
        seeds2.push_back(part);
      } else if (!IsRightIndented(indent) &&
                 part->boxes_count() > kTextBlobsTh) {
        // This is likely to be a text part, save the features.
        const TBOX&box = part->bounding_box();
        if (IsLeftIndented(indent)) {
          indented_texts_left.push_back(box.left());
        }
        texts_foreground_density.push_back(ComputeForegroundDensity(box));
      }
    }
  }

  // Sort the features collected from text regions.
  indented_texts_left.sort();
  texts_foreground_density.sort();
  float foreground_density_th = 0.15;  // Default value.
  if (!texts_foreground_density.empty()) {
    // Use the median of the texts_foreground_density.
    foreground_density_th = 0.8 * texts_foreground_density[
        texts_foreground_density.size() / 2];
  }

  for (int i = 0; i < seeds1.size(); ++i) {
    const TBOX& box = seeds1[i]->bounding_box();
    if (CheckSeedFgDensity(foreground_density_th, seeds1[i]) &&
        !(IsLeftIndented(IsIndented(seeds1[i])) &&
          CountAlignment(indented_texts_left, box.left()) >=
          kLeftIndentAlignmentCountTh)) {
      // Mark as PT_EQUATION type.
      seeds1[i]->set_type(PT_EQUATION);
      cp_seeds_.push_back(seeds1[i]);
    } else {  // Mark as PT_INLINE_EQUATION type.
      seeds1[i]->set_type(PT_INLINE_EQUATION);
    }
  }

  for (int i = 0; i < seeds2.size(); ++i) {
    if (CheckForSeed2(indented_texts_left, foreground_density_th, seeds2[i])) {
      seeds2[i]->set_type(PT_EQUATION);
      cp_seeds_.push_back(seeds2[i]);
    }
  }
}

float EquationDetect::ComputeForegroundDensity(const TBOX& tbox) {
#if LIBLEPT_MINOR_VERSION < 69 && LIBLEPT_MAJOR_VERSION <= 1
  // This will disable the detector because no seed will be identified.
  return 1.0f;
#else
  Pix *pix_bi = lang_tesseract_->pix_binary();
  int pix_height = pixGetHeight(pix_bi);
  Box* box = boxCreate(tbox.left(), pix_height - tbox.top(),
                       tbox.width(), tbox.height());
  Pix *pix_sub = pixClipRectangle(pix_bi, box, NULL);
  l_float32 fract;
  pixForegroundFraction(pix_sub, &fract);
  pixDestroy(&pix_sub);
  boxDestroy(&box);

  return fract;
#endif
}

bool EquationDetect::CheckSeedFgDensity(const float density_th,
                                        ColPartition* part) {
  ASSERT_HOST(part);

  // Split part horizontall, and check for each sub part.
  GenericVector<TBOX> sub_boxes;
  SplitCPHorLite(part, &sub_boxes);
  float parts_passed = 0.0;
  for (int i = 0; i < sub_boxes.size(); ++i) {
    float density = ComputeForegroundDensity(sub_boxes[i]);
    if (density < density_th) {
      parts_passed++;
    }
  }

  // If most sub parts passed, then we return true.
  const float kSeedPartRatioTh = 0.3;
  bool retval = (parts_passed / sub_boxes.size() >= kSeedPartRatioTh);

  return retval;
}

void EquationDetect::SplitCPHor(ColPartition* part,
     GenericVector<ColPartition*>* parts_splitted) {
  ASSERT_HOST(part && parts_splitted);
  if (part->median_width() == 0 || part->boxes_count() == 0) {
    return;
  }

  // Make a copy of part, and reset parts_splitted.
  ColPartition* right_part = part->CopyButDontOwnBlobs();
  parts_splitted->delete_data_pointers();
  parts_splitted->clear();

  const double kThreshold = part->median_width() * 3.0;
  bool found_split = true;
  while (found_split) {
    found_split = false;
    BLOBNBOX_C_IT box_it(right_part->boxes());
    // Blobs are sorted left side first. If blobs overlap,
    // the previous blob may have a "more right" right side.
    // Account for this by always keeping the largest "right"
    // so far.
    int previous_right = MIN_INT32;

    // Look for the next split in the partition.
    for (box_it.mark_cycle_pt(); !box_it.cycled_list(); box_it.forward()) {
      const TBOX& box = box_it.data()->bounding_box();
      if (previous_right != MIN_INT32 &&
          box.left() - previous_right > kThreshold) {
        // We have a split position. Split the partition in two pieces.
        // Insert the left piece in the grid and keep processing the right.
        int mid_x = (box.left() + previous_right) / 2;
        ColPartition* left_part = right_part;
        right_part = left_part->SplitAt(mid_x);

        parts_splitted->push_back(left_part);
        left_part->ComputeSpecialBlobsDensity();
        found_split = true;
        break;
      }

      // The right side of the previous blobs.
      previous_right = MAX(previous_right, box.right());
    }
  }

  // Add the last piece.
  right_part->ComputeSpecialBlobsDensity();
  parts_splitted->push_back(right_part);
}

void EquationDetect::SplitCPHorLite(ColPartition* part,
     GenericVector<TBOX>* splitted_boxes) {
  ASSERT_HOST(part && splitted_boxes);
  splitted_boxes->clear();
  if (part->median_width() == 0) {
    return;
  }

  const double kThreshold = part->median_width() * 3.0;

  // Blobs are sorted left side first. If blobs overlap,
  // the previous blob may have a "more right" right side.
  // Account for this by always keeping the largest "right"
  // so far.
  TBOX union_box;
  int previous_right = MIN_INT32;
  BLOBNBOX_C_IT box_it(part->boxes());
  for (box_it.mark_cycle_pt(); !box_it.cycled_list(); box_it.forward()) {
    const TBOX& box = box_it.data()->bounding_box();
    if (previous_right != MIN_INT32 &&
        box.left() - previous_right > kThreshold) {
      // We have a split position.
      splitted_boxes->push_back(union_box);
      previous_right = MIN_INT32;
    }
    if (previous_right == MIN_INT32) {
      union_box = box;
    } else {
      union_box += box;
    }
    // The right side of the previous blobs.
    previous_right = MAX(previous_right, box.right());
  }

  // Add the last piece.
  if (previous_right != MIN_INT32) {
    splitted_boxes->push_back(union_box);
  }
}

bool EquationDetect::CheckForSeed2(
    const GenericVector<int>& indented_texts_left,
    const float foreground_density_th,
    ColPartition* part) {
  ASSERT_HOST(part);
  const TBOX& box = part->bounding_box();

  // Check if it is aligned with any indented_texts_left.
  if (!indented_texts_left.empty() &&
      CountAlignment(indented_texts_left, box.left()) >=
      kLeftIndentAlignmentCountTh) {
      return false;
  }

  // Check the foreground density.
  if (ComputeForegroundDensity(box) > foreground_density_th) {
    return false;
  }

  return true;
}

int EquationDetect::CountAlignment(
    const GenericVector<int>& sorted_vec, const int val) const {
  if (sorted_vec.empty()) {
    return 0;
  }
  const int kDistTh = static_cast<int>(roundf(0.03 * resolution_));
  int pos = sorted_vec.binary_search(val), count = 0;

  // Search left side.
  int index = pos;
  while (index >= 0 && abs(val - sorted_vec[index--]) < kDistTh) {
    count++;
  }

  // Search right side.
  index = pos + 1;
  while (index < sorted_vec.size() && sorted_vec[index++] - val < kDistTh) {
    count++;
  }

  return count;
}

void EquationDetect::IdentifyInlineParts() {
  ComputeCPsSuperBBox();
  IdentifyInlinePartsHorizontal();
  int textparts_linespacing = EstimateTextPartLineSpacing();
  IdentifyInlinePartsVertical(true, textparts_linespacing);
  IdentifyInlinePartsVertical(false, textparts_linespacing);
}

void EquationDetect::ComputeCPsSuperBBox() {
  ColPartitionGridSearch gsearch(part_grid_);
  ColPartition *part = NULL;
  gsearch.StartFullSearch();
  if (cps_super_bbox_) {
    delete cps_super_bbox_;
  }
  cps_super_bbox_ = new TBOX();
  while ((part = gsearch.NextFullSearch()) != NULL) {
    (*cps_super_bbox_) += part->bounding_box();
  }
}

void EquationDetect::IdentifyInlinePartsHorizontal() {
  ASSERT_HOST(cps_super_bbox_);
  GenericVector<ColPartition*> new_seeds;
  const int kMarginDiffTh = IntCastRounded(
      0.5 * lang_tesseract_->source_resolution());
  const int kGapTh = static_cast<int>(roundf(
      1.0 * lang_tesseract_->source_resolution()));
  ColPartitionGridSearch search(part_grid_);
  search.SetUniqueMode(true);
  // The center x coordinate of the cp_super_bbox_.
  int cps_cx = cps_super_bbox_->left() + cps_super_bbox_->width() / 2;
  for (int i = 0; i < cp_seeds_.size(); ++i) {
    ColPartition* part = cp_seeds_[i];
    const TBOX& part_box(part->bounding_box());
    int left_margin = part_box.left() - cps_super_bbox_->left(),
        right_margin = cps_super_bbox_->right() - part_box.right();
    bool right_to_left;
    if (left_margin + kMarginDiffTh < right_margin &&
        left_margin < kMarginDiffTh) {
      // part is left aligned, so we search if it has any right neighbor.
      search.StartSideSearch(
          part_box.right(), part_box.top(), part_box.bottom());
      right_to_left = false;
    } else if (left_margin > cps_cx) {
      // part locates on the right half on image, so search if it has any left
      // neighbor.
      search.StartSideSearch(
          part_box.left(), part_box.top(), part_box.bottom());
      right_to_left = true;
    } else {  // part is not an inline equation.
      new_seeds.push_back(part);
      continue;
    }
    ColPartition* neighbor = NULL;
    bool side_neighbor_found = false;
    while ((neighbor = search.NextSideSearch(right_to_left)) != NULL) {
      const TBOX& neighbor_box(neighbor->bounding_box());
      if (!IsTextOrEquationType(neighbor->type()) ||
          part_box.x_gap(neighbor_box) > kGapTh ||
          !part_box.major_y_overlap(neighbor_box) ||
          part_box.major_x_overlap(neighbor_box)) {
        continue;
      }
      // We have found one. Set the side_neighbor_found flag.
      side_neighbor_found = true;
      break;
    }
    if (!side_neighbor_found) {  // Mark part as PT_INLINE_EQUATION.
      part->set_type(PT_INLINE_EQUATION);
    } else {
      // Check the geometric feature of neighbor.
      const TBOX& neighbor_box(neighbor->bounding_box());
      if (neighbor_box.width() > part_box.width() &&
          neighbor->type() != PT_EQUATION) {  // Mark as PT_INLINE_EQUATION.
        part->set_type(PT_INLINE_EQUATION);
      } else {  // part is not an inline equation type.
        new_seeds.push_back(part);
      }
    }
  }

  // Reset the cp_seeds_ using the new_seeds.
  cp_seeds_ = new_seeds;
}

int EquationDetect::EstimateTextPartLineSpacing() {
  ColPartitionGridSearch gsearch(part_grid_);

  // Get the y gap between text partitions;
  ColPartition *current = NULL, *prev = NULL;
  gsearch.StartFullSearch();
  GenericVector<int> ygaps;
  while ((current = gsearch.NextFullSearch()) != NULL) {
    if (!PTIsTextType(current->type())) {
      continue;
    }
    if (prev != NULL) {
      const TBOX &current_box = current->bounding_box();
      const TBOX &prev_box = prev->bounding_box();
      // prev and current should be x major overlap and non y overlap.
      if (current_box.major_x_overlap(prev_box) &&
          !current_box.y_overlap(prev_box)) {
        int gap = current_box.y_gap(prev_box);
        if (gap < MIN(current_box.height(), prev_box.height())) {
          // The gap should be smaller than the height of the bounding boxes.
          ygaps.push_back(gap);
        }
      }
    }
    prev = current;
  }

  if (ygaps.size() < 8) {  // We do not have enough data.
    return -1;
  }

  // Compute the line spacing from ygaps: use the mean of the first half.
  ygaps.sort();
  int spacing = 0, count;
  for (count = 0; count < ygaps.size() / 2; count++) {
    spacing += ygaps[count];
  }
  return spacing / count;
}

void EquationDetect::IdentifyInlinePartsVertical(
    const bool top_to_bottom, const int textparts_linespacing) {
  if (cp_seeds_.empty()) {
    return;
  }

  // Sort cp_seeds_.
  if (top_to_bottom) {  // From top to bottom.
    cp_seeds_.sort(&SortCPByTopReverse);
  } else {  // From bottom to top.
    cp_seeds_.sort(&SortCPByBottom);
  }

  GenericVector<ColPartition*> new_seeds;
  for (int i = 0; i < cp_seeds_.size(); ++i) {
    ColPartition* part = cp_seeds_[i];
    // If we sort cp_seeds_ from top to bottom, then for each cp_seeds_, we look
    // for its top neighbors, so that if two/more inline regions are connected
    // to each other, then we will identify the top one, and then use it to
    // identify the bottom one.
    if (IsInline(!top_to_bottom, textparts_linespacing, part)) {
      part->set_type(PT_INLINE_EQUATION);
    } else {
      new_seeds.push_back(part);
    }
  }
  cp_seeds_ = new_seeds;
}

bool EquationDetect::IsInline(const bool search_bottom,
                              const int textparts_linespacing,
                              ColPartition* part) {
  ASSERT_HOST(part != NULL);
  // Look for its nearest vertical neighbor that hardly overlaps in y but
  // largely overlaps in x.
  ColPartitionGridSearch search(part_grid_);
  ColPartition *neighbor = NULL;
  const TBOX& part_box(part->bounding_box());
  const float kYGapRatioTh = 1.0;

  if (search_bottom) {
    search.StartVerticalSearch(part_box.left(), part_box.right(),
                               part_box.bottom());
  } else {
    search.StartVerticalSearch(part_box.left(), part_box.right(),
                               part_box.top());
  }
  search.SetUniqueMode(true);
  while ((neighbor = search.NextVerticalSearch(search_bottom)) != NULL) {
    const TBOX& neighbor_box(neighbor->bounding_box());
    if (part_box.y_gap(neighbor_box) > kYGapRatioTh *
        MIN(part_box.height(), neighbor_box.height())) {
      // Finished searching.
      break;
    }
    if (!PTIsTextType(neighbor->type())) {
      continue;
    }

    // Check if neighbor and part is inline similar.
    const float kHeightRatioTh = 0.5;
    const int kYGapTh = textparts_linespacing > 0 ?
        textparts_linespacing + static_cast<int>(roundf(0.02 * resolution_)):
        static_cast<int>(roundf(0.05 * resolution_));  // Default value.
    if (part_box.x_overlap(neighbor_box) &&  // Location feature.
        part_box.y_gap(neighbor_box) <= kYGapTh &&  // Line spacing.
        // Geo feature.
        static_cast<float>(MIN(part_box.height(), neighbor_box.height())) /
        MAX(part_box.height(), neighbor_box.height()) > kHeightRatioTh) {
      return true;
    }
  }

  return false;
}

bool EquationDetect::CheckSeedBlobsCount(ColPartition* part) {
  if (!part) {
    return false;
  }
  const int kSeedMathBlobsCount = 2;
  const int kSeedMathDigitBlobsCount = 5;

  int blobs = part->boxes_count(),
      math_blobs = part->SpecialBlobsCount(BSTT_MATH),
      digit_blobs = part->SpecialBlobsCount(BSTT_DIGIT);
  if (blobs < kSeedBlobsCountTh || math_blobs <= kSeedMathBlobsCount ||
      math_blobs + digit_blobs <= kSeedMathDigitBlobsCount) {
    return false;
  }

  return true;
}

bool EquationDetect::CheckSeedDensity(
    const float math_density_high,
    const float math_density_low,
    const ColPartition* part) const {
  ASSERT_HOST(part);
  float math_digit_density = part->SpecialBlobsDensity(BSTT_MATH)
      + part->SpecialBlobsDensity(BSTT_DIGIT);
  float italic_density = part->SpecialBlobsDensity(BSTT_ITALIC);
  if (math_digit_density > math_density_high) {
    return true;
  }
  if (math_digit_density + italic_density > kMathItalicDensityTh &&
      math_digit_density > math_density_low) {
    return true;
  }

  return false;
}

EquationDetect::IndentType EquationDetect::IsIndented(ColPartition* part) {
  ASSERT_HOST(part);

  ColPartitionGridSearch search(part_grid_);
  ColPartition *neighbor = NULL;
  const TBOX& part_box(part->bounding_box());
  const int kXGapTh = static_cast<int>(roundf(0.5 * resolution_));
  const int kRadiusTh = static_cast<int>(roundf(3.0 * resolution_));
  const int kYGapTh = static_cast<int>(roundf(0.5 * resolution_));

  // Here we use a simple approximation algorithm: from the center of part, We
  // perform the radius search, and check if we can find a neighboring parition
  // that locates on the top/bottom left of part.
  search.StartRadSearch((part_box.left() + part_box.right()) / 2,
      (part_box.top() + part_box.bottom()) / 2, kRadiusTh);
  search.SetUniqueMode(true);
  bool left_indented = false, right_indented = false;
  while ((neighbor = search.NextRadSearch()) != NULL &&
         (!left_indented || !right_indented)) {
    if (neighbor == part) {
      continue;
    }
    const TBOX& neighbor_box(neighbor->bounding_box());

    if (part_box.major_y_overlap(neighbor_box) &&
        part_box.x_gap(neighbor_box) < kXGapTh) {
      // When this happens, it is likely part is a fragment of an
      // over-segmented colpartition. So we return false.
      return NO_INDENT;
    }

    if (!IsTextOrEquationType(neighbor->type())) {
      continue;
    }

    // The neighbor should be above/below part, and overlap in x direction.
    if (!part_box.x_overlap(neighbor_box) || part_box.y_overlap(neighbor_box)) {
      continue;
    }

    if (part_box.y_gap(neighbor_box) < kYGapTh) {
      int left_gap = part_box.left() - neighbor_box.left();
      int right_gap = neighbor_box.right() - part_box.right();
      if (left_gap > kXGapTh) {
        left_indented = true;
      }
      if (right_gap > kXGapTh) {
        right_indented = true;
      }
    }
  }

  if (left_indented && right_indented) {
    return BOTH_INDENT;
  }
  if (left_indented) {
    return LEFT_INDENT;
  }
  if (right_indented) {
    return RIGHT_INDENT;
  }
  return NO_INDENT;
}

bool EquationDetect::ExpandSeed(ColPartition* seed) {
  if (seed == NULL ||  // This seed has been absorbed by other seeds.
      seed->IsVerticalType()) {  // We skip vertical type right now.
    return false;
  }

  // Expand in four directions.
  GenericVector<ColPartition*> parts_to_merge;
  ExpandSeedHorizontal(true, seed, &parts_to_merge);
  ExpandSeedHorizontal(false, seed, &parts_to_merge);
  ExpandSeedVertical(true, seed, &parts_to_merge);
  ExpandSeedVertical(false, seed, &parts_to_merge);
  SearchByOverlap(seed, &parts_to_merge);

  if (parts_to_merge.empty()) {  // We don't find any partition to merge.
    return false;
  }

  // Merge all partitions in parts_to_merge with seed. We first remove seed
  // from part_grid_ as its bounding box is going to expand. Then we add it
  // back after it aborbs all parts_to_merge parititions.
  part_grid_->RemoveBBox(seed);
  for (int i = 0; i < parts_to_merge.size(); ++i) {
    ColPartition* part = parts_to_merge[i];
    if (part->type() == PT_EQUATION) {
      // If part is in cp_seeds_, then we mark it as NULL so that we won't
      // process it again.
      for (int j = 0; j < cp_seeds_.size(); ++j) {
        if (part == cp_seeds_[j]) {
          cp_seeds_[j] = NULL;
          break;
        }
      }
    }

    // part has already been removed from part_grid_ in function
    // ExpandSeedHorizontal/ExpandSeedVertical.
    seed->Absorb(part, NULL);
  }

  return true;
}

void EquationDetect::ExpandSeedHorizontal(
    const bool search_left,
    ColPartition* seed,
    GenericVector<ColPartition*>* parts_to_merge) {
  ASSERT_HOST(seed != NULL && parts_to_merge != NULL);
  const float kYOverlapTh = 0.6;
  const int kXGapTh = static_cast<int>(roundf(0.2 * resolution_));

  ColPartitionGridSearch search(part_grid_);
  const TBOX& seed_box(seed->bounding_box());
  int x = search_left ? seed_box.left() : seed_box.right();
  search.StartSideSearch(x, seed_box.bottom(), seed_box.top());
  search.SetUniqueMode(true);

  // Search iteratively.
  ColPartition *part = NULL;
  while ((part = search.NextSideSearch(search_left)) != NULL) {
    if (part == seed) {
      continue;
    }
    const TBOX& part_box(part->bounding_box());
    if (part_box.x_gap(seed_box) > kXGapTh) {  // Out of scope.
      break;
    }

    // Check part location.
    if ((part_box.left() >= seed_box.left() && search_left) ||
        (part_box.right() <= seed_box.right() && !search_left)) {
      continue;
    }

    if (part->type() != PT_EQUATION) {  // Non-equation type.
      // Skip PT_LINLINE_EQUATION and non text type.
      if (part->type() == PT_INLINE_EQUATION ||
          (!IsTextOrEquationType(part->type()) &&
           part->blob_type() != BRT_HLINE)) {
        continue;
      }
      // For other types, it should be the near small neighbor of seed.
      if (!IsNearSmallNeighbor(seed_box, part_box) ||
          !CheckSeedNeighborDensity(part)) {
        continue;
      }
    } else {  // Equation type, check the y overlap.
      if (part_box.y_overlap_fraction(seed_box) < kYOverlapTh &&
          seed_box.y_overlap_fraction(part_box) < kYOverlapTh) {
        continue;
      }
    }

    // Passed the check, delete it from search and add into parts_to_merge.
    search.RemoveBBox();
    parts_to_merge->push_back(part);
  }
}

void EquationDetect::ExpandSeedVertical(
    const bool search_bottom,
    ColPartition* seed,
    GenericVector<ColPartition*>* parts_to_merge) {
  ASSERT_HOST(seed != NULL && parts_to_merge != NULL &&
              cps_super_bbox_ != NULL);
  const float kXOverlapTh = 0.4;
  const int kYGapTh = static_cast<int>(roundf(0.2 * resolution_));

  ColPartitionGridSearch search(part_grid_);
  const TBOX& seed_box(seed->bounding_box());
  int y = search_bottom ? seed_box.bottom() : seed_box.top();
  search.StartVerticalSearch(
      cps_super_bbox_->left(), cps_super_bbox_->right(), y);
  search.SetUniqueMode(true);

  // Search iteratively.
  ColPartition *part = NULL;
  GenericVector<ColPartition*> parts;
  int skipped_min_top = INT_MAX, skipped_max_bottom = -1;
  while ((part = search.NextVerticalSearch(search_bottom)) != NULL) {
    if (part == seed) {
      continue;
    }
    const TBOX& part_box(part->bounding_box());

    if (part_box.y_gap(seed_box) > kYGapTh) {  // Out of scope.
      break;
    }

    // Check part location.
    if ((part_box.bottom() >= seed_box.bottom() && search_bottom) ||
        (part_box.top() <= seed_box.top() && !search_bottom)) {
      continue;
    }

    bool skip_part = false;
    if (part->type() != PT_EQUATION) {  // Non-equation type.
      // Skip PT_LINLINE_EQUATION and non text type.
      if (part->type() == PT_INLINE_EQUATION ||
          (!IsTextOrEquationType(part->type()) &&
           part->blob_type() != BRT_HLINE)) {
        skip_part = true;
      } else if (!IsNearSmallNeighbor(seed_box, part_box) ||
          !CheckSeedNeighborDensity(part)) {
        // For other types, it should be the near small neighbor of seed.
        skip_part = true;
      }
    } else {  // Equation type, check the x overlap.
      if (part_box.x_overlap_fraction(seed_box) < kXOverlapTh &&
          seed_box.x_overlap_fraction(part_box) < kXOverlapTh) {
        skip_part = true;
      }
    }
    if (skip_part) {
      if (part->type() != PT_EQUATION) {
        if (skipped_min_top > part_box.top()) {
          skipped_min_top = part_box.top();
        }
        if (skipped_max_bottom < part_box.bottom()) {
          skipped_max_bottom = part_box.bottom();
        }
      }
    } else {
      parts.push_back(part);
    }
  }

  // For every part in parts, we need verify it is not above skipped_min_top
  // when search top, or not below skipped_max_bottom when search bottom. I.e.,
  // we will skip a part if it looks like:
  //             search bottom      |         search top
  // seed:     ******************   | part:    **********
  // skipped: xxx                   | skipped:  xxx
  // part:       **********         | seed:    ***********
  for (int i = 0; i < parts.size(); i++) {
    const TBOX& part_box(parts[i]->bounding_box());
    if ((search_bottom && part_box.top() <= skipped_max_bottom) ||
        (!search_bottom && part_box.bottom() >= skipped_min_top)) {
      continue;
    }
    // Add parts[i] into parts_to_merge, and delete it from part_grid_.
    parts_to_merge->push_back(parts[i]);
    part_grid_->RemoveBBox(parts[i]);
  }
}

bool EquationDetect::IsNearSmallNeighbor(const TBOX& seed_box,
                                         const TBOX& part_box) const {
  const int kXGapTh = static_cast<int>(roundf(0.25 * resolution_));
  const int kYGapTh = static_cast<int>(roundf(0.05 * resolution_));

  // Check geometric feature.
  if (part_box.height() > seed_box.height() ||
      part_box.width() > seed_box.width()) {
    return false;
  }

  // Check overlap and distance.
  if ((!part_box.major_x_overlap(seed_box) ||
       part_box.y_gap(seed_box) > kYGapTh) &&
      (!part_box.major_y_overlap(seed_box) ||
       part_box.x_gap(seed_box) > kXGapTh)) {
    return false;
  }

  return true;
}

bool EquationDetect::CheckSeedNeighborDensity(const ColPartition* part) const {
  ASSERT_HOST(part);
  if (part->boxes_count() < kSeedBlobsCountTh) {
    // Too few blobs, skip the check.
    return true;
  }

  // We check the math blobs density and the unclear blobs density.
  if (part->SpecialBlobsDensity(BSTT_MATH) +
      part->SpecialBlobsDensity(BSTT_DIGIT) > kMathDigitDensityTh1 ||
      part->SpecialBlobsDensity(BSTT_UNCLEAR) > kUnclearDensityTh) {
    return true;
  }

  return false;
}

void EquationDetect::ProcessMathBlockSatelliteParts() {
  // Iterate over part_grid_, and find all parts that are text type but not
  // equation type.
  ColPartition *part = NULL;
  GenericVector<ColPartition*> text_parts;
  ColPartitionGridSearch gsearch(part_grid_);
  gsearch.StartFullSearch();
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() == PT_FLOWING_TEXT || part->type() == PT_HEADING_TEXT) {
      text_parts.push_back(part);
    }
  }
  if (text_parts.empty()) {
    return;
  }

  // Compute the medium height of the text_parts.
  text_parts.sort(&SortCPByHeight);
  const TBOX& text_box = text_parts[text_parts.size() / 2]->bounding_box();
  int med_height = text_box.height();
  if (text_parts.size() % 2 == 0 && text_parts.size() > 1) {
    const TBOX& text_box =
        text_parts[text_parts.size() / 2 - 1]->bounding_box();
    med_height = static_cast<int>(roundf(
        0.5 * (text_box.height() + med_height)));
  }

  // Iterate every text_parts and check if it is a math block satellite.
  for (int i = 0; i < text_parts.size(); ++i) {
    const TBOX& text_box(text_parts[i]->bounding_box());
    if (text_box.height() > med_height) {
      continue;
    }
    GenericVector<ColPartition*> math_blocks;
    if (!IsMathBlockSatellite(text_parts[i], &math_blocks)) {
      continue;
    }

    // Found. merge text_parts[i] with math_blocks.
    part_grid_->RemoveBBox(text_parts[i]);
    text_parts[i]->set_type(PT_EQUATION);
    for (int j = 0; j < math_blocks.size(); ++j) {
      part_grid_->RemoveBBox(math_blocks[j]);
      text_parts[i]->Absorb(math_blocks[j], NULL);
    }
    InsertPartAfterAbsorb(text_parts[i]);
  }
}

bool EquationDetect::IsMathBlockSatellite(
    ColPartition* part, GenericVector<ColPartition*>* math_blocks) {
  ASSERT_HOST(part != NULL && math_blocks != NULL);
  math_blocks->clear();
  const TBOX& part_box(part->bounding_box());
  // Find the top/bottom nearest neighbor of part.
  ColPartition *neighbors[2];
  int y_gaps[2] = {INT_MAX, INT_MAX};
  // The horizontal boundary of the neighbors.
  int neighbors_left = INT_MAX, neighbors_right = 0;
  for (int i = 0; i < 2; ++i) {
    neighbors[i] = SearchNNVertical(i != 0, part);
    if (neighbors[i]) {
      const TBOX& neighbor_box = neighbors[i]->bounding_box();
      y_gaps[i] = neighbor_box.y_gap(part_box);
      if (neighbor_box.left() < neighbors_left) {
        neighbors_left = neighbor_box.left();
      }
      if (neighbor_box.right() > neighbors_right) {
        neighbors_right = neighbor_box.right();
      }
    }
  }
  if (neighbors[0] == neighbors[1]) {
    // This happens when part is inside neighbor.
    neighbors[1] = NULL;
    y_gaps[1] = INT_MAX;
  }

  // Check if part is within [neighbors_left, neighbors_right].
  if (part_box.left() < neighbors_left || part_box.right() > neighbors_right) {
    return false;
  }

  // Get the index of the near one in neighbors.
  int index = y_gaps[0] < y_gaps[1] ? 0 : 1;

  // Check the near one.
  if (IsNearMathNeighbor(y_gaps[index], neighbors[index])) {
    math_blocks->push_back(neighbors[index]);
  } else {
    // If the near one failed the check, then we skip checking the far one.
    return false;
  }

  // Check the far one.
  index = 1 - index;
  if (IsNearMathNeighbor(y_gaps[index], neighbors[index])) {
    math_blocks->push_back(neighbors[index]);
  }

  return true;
}

ColPartition* EquationDetect::SearchNNVertical(
    const bool search_bottom, const ColPartition* part) {
  ASSERT_HOST(part);
  ColPartition *nearest_neighbor = NULL, *neighbor = NULL;
  const int kYGapTh = static_cast<int>(roundf(resolution_ * 0.5));

  ColPartitionGridSearch search(part_grid_);
  search.SetUniqueMode(true);
  const TBOX& part_box(part->bounding_box());
  int y = search_bottom ? part_box.bottom() : part_box.top();
  search.StartVerticalSearch(part_box.left(), part_box.right(), y);
  int min_y_gap = INT_MAX;
  while ((neighbor = search.NextVerticalSearch(search_bottom)) != NULL) {
    if (neighbor == part || !IsTextOrEquationType(neighbor->type())) {
      continue;
    }
    const TBOX& neighbor_box(neighbor->bounding_box());
    int y_gap = neighbor_box.y_gap(part_box);
    if (y_gap > kYGapTh) {  // Out of scope.
      break;
    }
    if (!neighbor_box.major_x_overlap(part_box) ||
        (search_bottom && neighbor_box.bottom() > part_box.bottom()) ||
        (!search_bottom && neighbor_box.top() < part_box.top())) {
      continue;
    }
    if (y_gap < min_y_gap) {
      min_y_gap = y_gap;
      nearest_neighbor = neighbor;
    }
  }

  return nearest_neighbor;
}

bool EquationDetect::IsNearMathNeighbor(
    const int y_gap, const ColPartition *neighbor) const {
  if (!neighbor) {
    return false;
  }
  const int kYGapTh = static_cast<int>(roundf(resolution_ * 0.1));
  return neighbor->type() == PT_EQUATION && y_gap <= kYGapTh;
}

void EquationDetect::GetOutputTiffName(const char* name,
                                       STRING* image_name) const {
  ASSERT_HOST(image_name && name);
  char page[50];
  snprintf(page, sizeof(page), "%04d", page_count_);
  *image_name = STRING(lang_tesseract_->imagebasename) + page + name + ".tif";
}

void EquationDetect::PaintSpecialTexts(const STRING& outfile) const {
  Pix *pix = NULL, *pixBi = lang_tesseract_->pix_binary();
  pix = pixConvertTo32(pixBi);
  ColPartitionGridSearch gsearch(part_grid_);
  ColPartition* part = NULL;
  gsearch.StartFullSearch();
  while ((part = gsearch.NextFullSearch()) != NULL) {
    BLOBNBOX_C_IT blob_it(part->boxes());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      RenderSpecialText(pix, blob_it.data());
    }
  }

  pixWrite(outfile.string(), pix, IFF_TIFF_LZW);
  pixDestroy(&pix);
}

void EquationDetect::PaintColParts(const STRING& outfile) const {
  Pix *pix = pixConvertTo32(lang_tesseract_->BestPix());
  ColPartitionGridSearch gsearch(part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    const TBOX& tbox = part->bounding_box();
    Box *box = boxCreate(tbox.left(), pixGetHeight(pix) - tbox.top(),
                         tbox.width(), tbox.height());
    if (part->type() == PT_EQUATION) {
      pixRenderBoxArb(pix, box, 5, 255, 0, 0);
    } else if (part->type() == PT_INLINE_EQUATION) {
      pixRenderBoxArb(pix, box, 5, 0, 255, 0);
    } else {
      pixRenderBoxArb(pix, box, 5, 0, 0, 255);
    }
    boxDestroy(&box);
  }

  pixWrite(outfile.string(), pix, IFF_TIFF_LZW);
  pixDestroy(&pix);
}

void EquationDetect::PrintSpecialBlobsDensity(const ColPartition* part) const {
  ASSERT_HOST(part);
  TBOX box(part->bounding_box());
  int h = pixGetHeight(lang_tesseract_->BestPix());
  tprintf("Printing special blobs density values for ColParition (t=%d,b=%d) ",
          h - box.top(), h - box.bottom());
  box.print();
  tprintf("blobs count = %d, density = ", part->boxes_count());
  for (int i = 0; i < BSTT_COUNT; ++i) {
    BlobSpecialTextType type = static_cast<BlobSpecialTextType>(i);
    tprintf("%d:%f ", i, part->SpecialBlobsDensity(type));
  }
  tprintf("\n");
}

};  // namespace tesseract
