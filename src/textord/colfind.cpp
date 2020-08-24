///////////////////////////////////////////////////////////////////////
// File:        colfind.cpp
// Description: Class to hold BLOBNBOXs in a grid for fast access
//              to neighbours.
// Author:      Ray Smith
//
// (C) Copyright 2007, Google Inc.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "colfind.h"

#include "ccnontextdetect.h"
#include "colpartition.h"
#include "colpartitionset.h"
#include "equationdetectbase.h"
#include "linefind.h"
#include "normalis.h"
#include "strokewidth.h"
#include "blobbox.h"
#include "scrollview.h"
#include "tablefind.h"
#include "params.h"
#include "workingpartset.h"

#include <algorithm>

namespace tesseract {

// When assigning columns, the max number of misfit grid rows/ColPartitionSets
// that can be ignored.
const int kMaxIncompatibleColumnCount = 2;
// Max fraction of mean_column_gap_ for the gap between two partitions within a
// column to allow them to merge.
const double kHorizontalGapMergeFraction = 0.5;
// Minimum gutter width as a fraction of gridsize
const double kMinGutterWidthGrid = 0.5;
// Max multiple of a partition's median size as a distance threshold for
// adding noise blobs.
const double kMaxDistToPartSizeRatio = 1.5;

#ifndef GRAPHICS_DISABLED
static BOOL_VAR(textord_tabfind_show_initial_partitions,
                false, "Show partition bounds");
static BOOL_VAR(textord_tabfind_show_reject_blobs,
                false, "Show blobs rejected as noise");
static INT_VAR(textord_tabfind_show_partitions, 0,
              "Show partition bounds, waiting if >1 (ScrollView)");
static BOOL_VAR(textord_tabfind_show_columns, false, "Show column bounds (ScrollView)");
static BOOL_VAR(textord_tabfind_show_blocks, false, "Show final block bounds (ScrollView)");
#endif
static BOOL_VAR(textord_tabfind_find_tables, true, "run table detection");

#ifndef GRAPHICS_DISABLED
ScrollView* ColumnFinder::blocks_win_ = nullptr;
#endif

// Gridsize is an estimate of the text size in the image. A suitable value
// is in TO_BLOCK::line_size after find_components has been used to make
// the blobs.
// bleft and tright are the bounds of the image (or rectangle) being processed.
// vlines is a (possibly empty) list of TabVector and vertical_x and y are
// the sum logical vertical vector produced by LineFinder::FindVerticalLines.
ColumnFinder::ColumnFinder(int gridsize,
                           const ICOORD& bleft, const ICOORD& tright,
                           int resolution, bool cjk_script,
                           double aligned_gap_fraction,
                           TabVector_LIST* vlines, TabVector_LIST* hlines,
                           int vertical_x, int vertical_y)
  : TabFind(gridsize, bleft, tright, vlines, vertical_x, vertical_y,
            resolution),
    cjk_script_(cjk_script),
    min_gutter_width_(static_cast<int>(kMinGutterWidthGrid * gridsize)),
    mean_column_gap_(tright.x() - bleft.x()),
    tabfind_aligned_gap_fraction_(aligned_gap_fraction),
    deskew_(0.0f, 0.0f),
    reskew_(1.0f, 0.0f), rotation_(1.0f, 0.0f), rerotate_(1.0f, 0.0f),
    text_rotation_(0.0f, 0.0f),
    best_columns_(nullptr), stroke_width_(nullptr),
    part_grid_(gridsize, bleft, tright), nontext_map_(nullptr),
    projection_(resolution),
    denorm_(nullptr), input_blobs_win_(nullptr), equation_detect_(nullptr) {
  TabVector_IT h_it(&horizontal_lines_);
  h_it.add_list_after(hlines);
}

ColumnFinder::~ColumnFinder() {
  column_sets_.delete_data_pointers();
  delete [] best_columns_;
  delete stroke_width_;
  delete input_blobs_win_;
  pixDestroy(&nontext_map_);
  while (denorm_ != nullptr) {
    DENORM* dead_denorm = denorm_;
    denorm_ = const_cast<DENORM*>(denorm_->predecessor());
    delete dead_denorm;
  }

  // The ColPartitions are destroyed automatically, but any boxes in
  // the noise_parts_ list are owned and need to be deleted explicitly.
  ColPartition_IT part_it(&noise_parts_);
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition* part = part_it.data();
    part->DeleteBoxes();
  }
  // Likewise any boxes in the good_parts_ list need to be deleted.
  // These are just the image parts. Text parts have already given their
  // boxes on to the TO_BLOCK, and have empty lists.
  part_it.set_to_list(&good_parts_);
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition* part = part_it.data();
    part->DeleteBoxes();
  }
  // Also, any blobs on the image_bblobs_ list need to have their cblobs
  // deleted. This only happens if there has been an early return from
  // FindColumns, as in a normal return, the blobs go into the grid and
  // end up in noise_parts_, good_parts_ or the output blocks.
  BLOBNBOX_IT bb_it(&image_bblobs_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    BLOBNBOX* bblob = bb_it.data();
    delete bblob->cblob();
  }
}

// Performs initial processing on the blobs in the input_block:
// Setup the part_grid, stroke_width_, nontext_map.
// Obvious noise blobs are filtered out and used to mark the nontext_map_.
// Initial stroke-width analysis is used to get local text alignment
// direction, so the textline projection_ map can be setup.
// On return, IsVerticallyAlignedText may be called (now optionally) to
// determine the gross textline alignment of the page.
void ColumnFinder::SetupAndFilterNoise(PageSegMode pageseg_mode,
                                       Pix* photo_mask_pix,
                                       TO_BLOCK* input_block) {
  part_grid_.Init(gridsize(), bleft(), tright());
  delete stroke_width_;
  stroke_width_ = new StrokeWidth(gridsize(), bleft(), tright());
  min_gutter_width_ = static_cast<int>(kMinGutterWidthGrid * gridsize());
  input_block->ReSetAndReFilterBlobs();
  #ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_blocks) {
    input_blobs_win_ = MakeWindow(0, 0, "Filtered Input Blobs");
    input_block->plot_graded_blobs(input_blobs_win_);
  }
  #endif // !GRAPHICS_DISABLED
  SetBlockRuleEdges(input_block);
  pixDestroy(&nontext_map_);
  // Run a preliminary strokewidth neighbour detection on the medium blobs.
  stroke_width_->SetNeighboursOnMediumBlobs(input_block);
  CCNonTextDetect nontext_detect(gridsize(), bleft(), tright());
  // Remove obvious noise and make the initial non-text map.
  nontext_map_ = nontext_detect.ComputeNonTextMask(textord_debug_tabfind,
                                                   photo_mask_pix, input_block);
  stroke_width_->FindTextlineDirectionAndFixBrokenCJK(pageseg_mode, cjk_script_,
                                                      input_block);
  // Clear the strokewidth grid ready for rotation or leader finding.
  stroke_width_->Clear();
}

// Tests for vertical alignment of text (returning true if so), and generates
// a list of blobs of moderate aspect ratio, in the most frequent writing
// direction (in osd_blobs) for orientation and script detection to test
// the character orientation.
// block is the single block for the whole page or rectangle to be OCRed.
// Note that the vertical alignment may be due to text whose writing direction
// is vertical, like say Japanese, or due to text whose writing direction is
// horizontal but whose text appears vertically aligned because the image is
// not the right way up.
bool ColumnFinder::IsVerticallyAlignedText(double find_vertical_text_ratio,
                                           TO_BLOCK* block,
                                           BLOBNBOX_CLIST* osd_blobs) {
  return stroke_width_->TestVerticalTextDirection(find_vertical_text_ratio,
                                                  block, osd_blobs);
}

// Rotates the blobs and the TabVectors so that the gross writing direction
// (text lines) are horizontal and lines are read down the page.
// Applied rotation stored in rotation_.
// A second rotation is calculated for application during recognition to
// make the rotated blobs upright for recognition.
// Subsequent rotation stored in text_rotation_.
//
// Arguments:
//   vertical_text_lines true if the text lines are vertical.
//   recognition_rotation [0..3] is the number of anti-clockwise 90 degree
//   rotations from osd required for the text to be upright and readable.
void ColumnFinder::CorrectOrientation(TO_BLOCK* block,
                                      bool vertical_text_lines,
                                      int recognition_rotation) {
  const FCOORD anticlockwise90(0.0f, 1.0f);
  const FCOORD clockwise90(0.0f, -1.0f);
  const FCOORD rotation180(-1.0f, 0.0f);
  const FCOORD norotation(1.0f, 0.0f);

  text_rotation_ = norotation;
  // Rotate the page to make the text upright, as implied by
  // recognition_rotation.
  rotation_ = norotation;
  if (recognition_rotation == 1) {
    rotation_ = anticlockwise90;
  } else if (recognition_rotation == 2) {
    rotation_ = rotation180;
  } else if (recognition_rotation == 3) {
    rotation_ = clockwise90;
  }
  // We infer text writing direction to be vertical if there are several
  // vertical text lines detected, and horizontal if not. But if the page
  // orientation was determined to be 90 or 270 degrees, the true writing
  // direction is the opposite of what we inferred.
  if (recognition_rotation & 1) {
    vertical_text_lines = !vertical_text_lines;
  }
  // If we still believe the writing direction is vertical, we use the
  // convention of rotating the page ccw 90 degrees to make the text lines
  // horizontal, and mark the blobs for rotation cw 90 degrees for
  // classification so that the text order is correct after recognition.
  if (vertical_text_lines) {
    rotation_.rotate(anticlockwise90);
    text_rotation_.rotate(clockwise90);
  }
  // Set rerotate_ to the inverse of rotation_.
  rerotate_ = FCOORD(rotation_.x(), -rotation_.y());
  if (rotation_.x() != 1.0f || rotation_.y() != 0.0f) {
    // Rotate all the blobs and tab vectors.
    RotateBlobList(rotation_, &block->large_blobs);
    RotateBlobList(rotation_, &block->blobs);
    RotateBlobList(rotation_, &block->small_blobs);
    RotateBlobList(rotation_, &block->noise_blobs);
    TabFind::ResetForVerticalText(rotation_, rerotate_, &horizontal_lines_,
                                  &min_gutter_width_);
    part_grid_.Init(gridsize(), bleft(), tright());
    // Reset all blobs to initial state and filter by size.
    // Since they have rotated, the list they belong on could have changed.
    block->ReSetAndReFilterBlobs();
    SetBlockRuleEdges(block);
    stroke_width_->CorrectForRotation(rerotate_, &part_grid_);
  }
  if (textord_debug_tabfind) {
    tprintf("Vertical=%d, orientation=%d, final rotation=(%f, %f)+(%f,%f)\n",
            vertical_text_lines, recognition_rotation,
            rotation_.x(), rotation_.y(),
            text_rotation_.x(), text_rotation_.y());
  }
  // Setup the denormalization.
  ASSERT_HOST(denorm_ == nullptr);
  denorm_ = new DENORM;
  denorm_->SetupNormalization(nullptr, &rotation_, nullptr,
                              0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
}

// Finds blocks of text, image, rule line, table etc, returning them in the
// blocks and to_blocks
// (Each TO_BLOCK points to the basic BLOCK and adds more information.)
// Image blocks are generated by a combination of photo_mask_pix (which may
// NOT be nullptr) and the rejected text found during preliminary textline
// finding.
// The input_block is the result of a call to find_components, and contains
// the blobs found in the image or rectangle to be OCRed. These blobs will be
// removed and placed in the output blocks, while unused ones will be deleted.
// If single_column is true, the input is treated as single column, but
// it is still divided into blocks of equal line spacing/text size.
// scaled_color is scaled down by scaled_factor from the input color image,
// and may be nullptr if the input was not color.
// grey_pix is optional, but if present must match the photo_mask_pix in size,
// and must be a *real* grey image instead of binary_pix * 255.
// thresholds_pix is expected to be present iff grey_pix is present and
// can be an integer factor reduction of the grey_pix. It represents the
// thresholds that were used to create the binary_pix from the grey_pix.
// If diacritic_blobs is non-null, then diacritics/noise blobs, that would
// confuse layout analysis by causing textline overlap, are placed there,
// with the expectation that they will be reassigned to words later and
// noise/diacriticness determined via classification.
// Returns -1 if the user hits the 'd' key in the blocks window while running
// in debug mode, which requests a retry with more debug info.
int ColumnFinder::FindBlocks(PageSegMode pageseg_mode, Pix* scaled_color,
                             int scaled_factor, TO_BLOCK* input_block,
                             Pix* photo_mask_pix, Pix* thresholds_pix,
                             Pix* grey_pix, DebugPixa* pixa_debug,
                             BLOCK_LIST* blocks, BLOBNBOX_LIST* diacritic_blobs,
                             TO_BLOCK_LIST* to_blocks) {
  pixOr(photo_mask_pix, photo_mask_pix, nontext_map_);
  stroke_width_->FindLeaderPartitions(input_block, &part_grid_);
  stroke_width_->RemoveLineResidue(&big_parts_);
  FindInitialTabVectors(nullptr, min_gutter_width_, tabfind_aligned_gap_fraction_,
                        input_block);
  SetBlockRuleEdges(input_block);
  stroke_width_->GradeBlobsIntoPartitions(
      pageseg_mode, rerotate_, input_block, nontext_map_, denorm_, cjk_script_,
      &projection_, diacritic_blobs, &part_grid_, &big_parts_);
  if (!PSM_SPARSE(pageseg_mode)) {
    ImageFind::FindImagePartitions(photo_mask_pix, rotation_, rerotate_,
                                   input_block, this, pixa_debug, &part_grid_,
                                   &big_parts_);
    ImageFind::TransferImagePartsToImageMask(rerotate_, &part_grid_,
                                             photo_mask_pix);
    ImageFind::FindImagePartitions(photo_mask_pix, rotation_, rerotate_,
                                   input_block, this, pixa_debug, &part_grid_,
                                   &big_parts_);
  }
  part_grid_.ReTypeBlobs(&image_bblobs_);
  TidyBlobs(input_block);
  Reset();
  // TODO(rays) need to properly handle big_parts_.
  ColPartition_IT p_it(&big_parts_);
  for (p_it.mark_cycle_pt(); !p_it.cycled_list(); p_it.forward())
    p_it.data()->DisownBoxesNoAssert();
  big_parts_.clear();
  delete stroke_width_;
  stroke_width_ = nullptr;
  // Compute the edge offsets whether or not there is a grey_pix. It is done
  // here as the c_blobs haven't been touched by rotation or anything yet,
  // so no denorm is required, yet the text has been separated from image, so
  // no time is wasted running it on image blobs.
  input_block->ComputeEdgeOffsets(thresholds_pix, grey_pix);

  // A note about handling right-to-left scripts (Hebrew/Arabic):
  // The columns must be reversed and come out in right-to-left instead of
  // the normal left-to-right order. Because the left-to-right ordering
  // is implicit in many data structures, it is simpler to fool the algorithms
  // into thinking they are dealing with left-to-right text.
  // To do this, we reflect the needed data in the y-axis and then reflect
  // the blocks back after they have been created. This is a temporary
  // arrangement that is confined to this function only, so the reflection
  // is completely invisible in the output blocks.
  // The only objects reflected are:
  // The vertical separator lines that have already been found;
  // The bounding boxes of all BLOBNBOXES on all lists on the input_block
  // plus the image_bblobs. The outlines are not touched, since they are
  // not looked at.
  bool input_is_rtl = input_block->block->right_to_left();
  if (input_is_rtl) {
    // Reflect the vertical separator lines (member of TabFind).
    ReflectInYAxis();
    // Reflect the blob boxes.
    ReflectForRtl(input_block, &image_bblobs_);
    part_grid_.ReflectInYAxis();
  }

  if (!PSM_SPARSE(pageseg_mode)) {
    if (!PSM_COL_FIND_ENABLED(pageseg_mode)) {
      // No tab stops needed. Just the grid that FindTabVectors makes.
      DontFindTabVectors(&image_bblobs_, input_block, &deskew_, &reskew_);
    } else {
      SetBlockRuleEdges(input_block);
      // Find the tab stops, estimate skew, and deskew the tabs, blobs and
      // part_grid_.
      FindTabVectors(&horizontal_lines_, &image_bblobs_, input_block,
                     min_gutter_width_, tabfind_aligned_gap_fraction_,
                     &part_grid_, &deskew_, &reskew_);
      // Add the deskew to the denorm_.
      auto* new_denorm = new DENORM;
      new_denorm->SetupNormalization(nullptr, &deskew_, denorm_,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
      denorm_ = new_denorm;
    }
    SetBlockRuleEdges(input_block);
    part_grid_.SetTabStops(this);

    // Make the column_sets_.
    if (!MakeColumns(false)) {
      tprintf("Empty page!!\n");
      part_grid_.DeleteParts();
      return 0;  // This is an empty page.
    }

    // Refill the grid using rectangular spreading, and get the benefit
    // of the completed tab vectors marking the rule edges of each blob.
    Clear();
    #ifndef GRAPHICS_DISABLED
    if (textord_tabfind_show_reject_blobs) {
      ScrollView* rej_win = MakeWindow(500, 300, "Rejected blobs");
      input_block->plot_graded_blobs(rej_win);
    }
    #endif // !GRAPHICS_DISABLED
    InsertBlobsToGrid(false, false, &image_bblobs_, this);
    InsertBlobsToGrid(true, true, &input_block->blobs, this);

    part_grid_.GridFindMargins(best_columns_);
    // Split and merge the partitions by looking at local neighbours.
    GridSplitPartitions();
    // Resolve unknown partitions by adding to an existing partition, fixing
    // the type, or declaring them noise.
    part_grid_.GridFindMargins(best_columns_);
    GridMergePartitions();
    // Insert any unused noise blobs that are close enough to an appropriate
    // partition.
    InsertRemainingNoise(input_block);
    // Add horizontal line separators as partitions.
    GridInsertHLinePartitions();
    GridInsertVLinePartitions();
    // Recompute margins based on a local neighbourhood search.
    part_grid_.GridFindMargins(best_columns_);
    SetPartitionTypes();
  }
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_initial_partitions) {
    ScrollView* part_win = MakeWindow(100, 300, "InitialPartitions");
    part_grid_.DisplayBoxes(part_win);
    DisplayTabVectors(part_win);
  }
#endif
  if (!PSM_SPARSE(pageseg_mode)) {
    if (equation_detect_) {
      equation_detect_->FindEquationParts(&part_grid_, best_columns_);
    }
    if (textord_tabfind_find_tables) {
      TableFinder table_finder;
      table_finder.Init(gridsize(), bleft(), tright());
      table_finder.set_resolution(resolution_);
      table_finder.set_left_to_right_language(
          !input_block->block->right_to_left());
      // Copy cleaned partitions from part_grid_ to clean_part_grid_ and
      // insert dot-like noise into period_grid_
      table_finder.InsertCleanPartitions(&part_grid_, input_block);
      // Get Table Regions
      table_finder.LocateTables(&part_grid_, best_columns_, WidthCB(), reskew_);
    }
    GridRemoveUnderlinePartitions();
    part_grid_.DeleteUnknownParts(input_block);

    // Build the partitions into chains that belong in the same block and
    // refine into one-to-one links, then smooth the types within each chain.
    part_grid_.FindPartitionPartners();
    part_grid_.FindFigureCaptions();
    part_grid_.RefinePartitionPartners(true);
    SmoothPartnerRuns();

    #ifndef GRAPHICS_DISABLED
    if (textord_tabfind_show_partitions) {
      ScrollView* window = MakeWindow(400, 300, "Partitions");
      if (window != nullptr) {
        part_grid_.DisplayBoxes(window);
        if (!textord_debug_printable)
          DisplayTabVectors(window);
        if (window != nullptr && textord_tabfind_show_partitions > 1) {
          delete window->AwaitEvent(SVET_DESTROY);
        }
      }
    }
    #endif // !GRAPHICS_DISABLED
    part_grid_.AssertNoDuplicates();
  }
  // Ownership of the ColPartitions moves from part_sets_ to part_grid_ here,
  // and ownership of the BLOBNBOXes moves to the ColPartitions.
  // (They were previously owned by the block or the image_bblobs list.)
  ReleaseBlobsAndCleanupUnused(input_block);
  // Ownership of the ColPartitions moves from part_grid_ to good_parts_ and
  // noise_parts_ here. In text blocks, ownership of the BLOBNBOXes moves
  // from the ColPartitions to the output TO_BLOCK. In non-text, the
  // BLOBNBOXes stay with the ColPartitions and get deleted in the destructor.
  if (PSM_SPARSE(pageseg_mode))
    part_grid_.ExtractPartitionsAsBlocks(blocks, to_blocks);
  else
    TransformToBlocks(blocks, to_blocks);
  if (textord_debug_tabfind) {
    tprintf("Found %d blocks, %d to_blocks\n",
            blocks->length(), to_blocks->length());
  }

#ifndef GRAPHICS_DISABLED
  DisplayBlocks(blocks);
#endif
  RotateAndReskewBlocks(input_is_rtl, to_blocks);
  int result = 0;
  #ifndef GRAPHICS_DISABLED
  if (blocks_win_ != nullptr) {
    bool waiting = false;
    do {
      waiting = false;
      SVEvent* event = blocks_win_->AwaitEvent(SVET_ANY);
      if (event->type == SVET_INPUT && event->parameter != nullptr) {
        if (*event->parameter == 'd')
          result = -1;
        else
          blocks->clear();
      } else if (event->type == SVET_DESTROY) {
        blocks_win_ = nullptr;
      } else {
        waiting = true;
      }
      delete event;
    } while (waiting);
  }
  #endif // !GRAPHICS_DISABLED
  return result;
}

// Get the rotation required to deskew, and its inverse rotation.
void ColumnFinder::GetDeskewVectors(FCOORD* deskew, FCOORD* reskew) {
  *reskew = reskew_;
  *deskew = reskew_;
  deskew->set_y(-deskew->y());
}

void ColumnFinder::SetEquationDetect(EquationDetectBase* detect) {
  equation_detect_ = detect;
}

//////////////// PRIVATE CODE /////////////////////////

#ifndef GRAPHICS_DISABLED

// Displays the blob and block bounding boxes in a window called Blocks.
void ColumnFinder::DisplayBlocks(BLOCK_LIST* blocks) {
  if (textord_tabfind_show_blocks) {
    if (blocks_win_ == nullptr)
      blocks_win_ = MakeWindow(700, 300, "Blocks");
    else
      blocks_win_->Clear();
    DisplayBoxes(blocks_win_);
    BLOCK_IT block_it(blocks);
    int serial = 1;
    for (block_it.mark_cycle_pt(); !block_it.cycled_list();
         block_it.forward()) {
      BLOCK* block = block_it.data();
      block->pdblk.plot(blocks_win_, serial++,
                  textord_debug_printable ? ScrollView::BLUE
                                          : ScrollView::GREEN);
    }
    blocks_win_->Update();
  }
}

// Displays the column edges at each grid y coordinate defined by
// best_columns_.
void ColumnFinder::DisplayColumnBounds(PartSetVector* sets) {
  ScrollView* col_win = MakeWindow(50, 300, "Columns");
  DisplayBoxes(col_win);
  col_win->Pen(textord_debug_printable ? ScrollView::BLUE : ScrollView::GREEN);
  for (int i = 0; i < gridheight_; ++i) {
    ColPartitionSet* columns = best_columns_[i];
    if (columns != nullptr)
      columns->DisplayColumnEdges(i * gridsize_, (i + 1) * gridsize_, col_win);
  }
}

#endif // !GRAPHICS_DISABLED

// Sets up column_sets_ (the determined column layout at each horizontal
// slice). Returns false if the page is empty.
bool ColumnFinder::MakeColumns(bool single_column) {
  // The part_sets_ are a temporary structure used during column creation,
  // and is a vector of ColPartitionSets, representing ColPartitions found
  // at horizontal slices through the page.
  PartSetVector part_sets;
  if (!single_column) {
    if (!part_grid_.MakeColPartSets(&part_sets))
      return false;  // Empty page.
    ASSERT_HOST(part_grid_.gridheight() == gridheight_);
    // Try using only the good parts first.
    bool good_only = true;
    do {
      for (int i = 0; i < gridheight_; ++i) {
        ColPartitionSet* line_set = part_sets.get(i);
        if (line_set != nullptr && line_set->LegalColumnCandidate()) {
          ColPartitionSet* column_candidate = line_set->Copy(good_only);
          if (column_candidate != nullptr)
            column_candidate->AddToColumnSetsIfUnique(&column_sets_, WidthCB());
        }
      }
      good_only = !good_only;
    } while (column_sets_.empty() && !good_only);
    if (textord_debug_tabfind)
      PrintColumnCandidates("Column candidates");
    // Improve the column candidates against themselves.
    ImproveColumnCandidates(&column_sets_, &column_sets_);
    if (textord_debug_tabfind)
      PrintColumnCandidates("Improved columns");
    // Improve the column candidates using the part_sets_.
    ImproveColumnCandidates(&part_sets, &column_sets_);
  }
  ColPartitionSet* single_column_set =
      part_grid_.MakeSingleColumnSet(WidthCB());
  if (single_column_set != nullptr) {
    // Always add the single column set as a backup even if not in
    // single column mode.
    single_column_set->AddToColumnSetsIfUnique(&column_sets_, WidthCB());
  }
  if (textord_debug_tabfind)
    PrintColumnCandidates("Final Columns");
  bool has_columns = !column_sets_.empty();
  if (has_columns) {
    // Divide the page into sections of uniform column layout.
    bool any_multi_column = AssignColumns(part_sets);
#ifndef GRAPHICS_DISABLED
    if (textord_tabfind_show_columns) {
      DisplayColumnBounds(&part_sets);
    }
#endif
    ComputeMeanColumnGap(any_multi_column);
  }
  for (int i = 0; i < part_sets.size(); ++i) {
    ColPartitionSet* line_set = part_sets.get(i);
    if (line_set != nullptr) {
      line_set->RelinquishParts();
      delete line_set;
    }
  }
  return has_columns;
}

// Attempt to improve the column_candidates by expanding the columns
// and adding new partitions from the partition sets in src_sets.
// Src_sets may be equal to column_candidates, in which case it will
// use them as a source to improve themselves.
void ColumnFinder::ImproveColumnCandidates(PartSetVector* src_sets,
                                           PartSetVector* column_sets) {
  PartSetVector temp_cols;
  temp_cols.move(column_sets);
  if (src_sets == column_sets)
    src_sets = &temp_cols;
  int set_size = temp_cols.size();
  // Try using only the good parts first.
  bool good_only = true;
  do {
    for (int i = 0; i < set_size; ++i) {
      ColPartitionSet* column_candidate = temp_cols.get(i);
      ASSERT_HOST(column_candidate != nullptr);
      ColPartitionSet* improved = column_candidate->Copy(good_only);
      if (improved != nullptr) {
        improved->ImproveColumnCandidate(WidthCB(), src_sets);
        improved->AddToColumnSetsIfUnique(column_sets, WidthCB());
      }
    }
    good_only = !good_only;
  } while (column_sets->empty() && !good_only);
  if (column_sets->empty())
    column_sets->move(&temp_cols);
  else
    temp_cols.delete_data_pointers();
}

// Prints debug information on the column candidates.
void ColumnFinder::PrintColumnCandidates(const char* title) {
  int set_size =  column_sets_.size();
  tprintf("Found %d %s:\n", set_size, title);
  if (textord_debug_tabfind >= 3) {
    for (int i = 0; i < set_size; ++i) {
      ColPartitionSet* column_set = column_sets_.get(i);
      column_set->Print();
    }
  }
}

// Finds the optimal set of columns that cover the entire image with as
// few changes in column partition as possible.
// NOTE: this could be thought of as an optimization problem, but a simple
// greedy algorithm is used instead. The algorithm repeatedly finds the modal
// compatible column in an unassigned region and uses that with the extra
// tweak of extending the modal region over small breaks in compatibility.
// Where modal regions overlap, the boundary is chosen so as to minimize
// the cost in terms of ColPartitions not fitting an approved column.
// Returns true if any part of the page is multi-column.
bool ColumnFinder::AssignColumns(const PartSetVector& part_sets) {
  int set_count = part_sets.size();
  ASSERT_HOST(set_count == gridheight());
  // Allocate and init the best_columns_.
  best_columns_ = new ColPartitionSet*[set_count];
  for (int y = 0; y < set_count; ++y)
    best_columns_[y] = nullptr;
  int column_count = column_sets_.size();
  // column_set_costs[part_sets_ index][column_sets_ index] is
  // < INT32_MAX if the partition set is compatible with the column set,
  // in which case its value is the cost for that set used in deciding
  // which competing set to assign.
  // any_columns_possible[part_sets_ index] is true if any of
  // possible_column_sets[part_sets_ index][*] is < INT32_MAX.
  // assigned_costs[part_sets_ index] is set to the column_set_costs
  // of the assigned column_sets_ index or INT32_MAX if none is set.
  // On return the best_columns_ member is set.
  bool* any_columns_possible = new bool[set_count];
  int* assigned_costs = new int[set_count];
  int** column_set_costs = new int*[set_count];
  // Set possible column_sets to indicate whether each set is compatible
  // with each column.
  for (int part_i = 0; part_i < set_count; ++part_i) {
    ColPartitionSet* line_set = part_sets.get(part_i);
    bool debug = line_set != nullptr &&
                 WithinTestRegion(2, line_set->bounding_box().left(),
                                  line_set->bounding_box().bottom());
    column_set_costs[part_i] = new int[column_count];
    any_columns_possible[part_i] = false;
    assigned_costs[part_i] = INT32_MAX;
    for (int col_i = 0; col_i < column_count; ++col_i) {
      if (line_set != nullptr &&
          column_sets_.get(col_i)->CompatibleColumns(debug, line_set,
                                                     WidthCB())) {
        column_set_costs[part_i][col_i] =
            column_sets_.get(col_i)->UnmatchedWidth(line_set);
        any_columns_possible[part_i] = true;
      } else {
        column_set_costs[part_i][col_i] = INT32_MAX;
        if (debug)
          tprintf("Set id %d did not match at y=%d, lineset =%p\n",
                  col_i, part_i, line_set);
      }
    }
  }
  bool any_multi_column = false;
  // Assign a column set to each vertical grid position.
  // While there is an unassigned range, find its mode.
  int start, end;
  while (BiggestUnassignedRange(set_count, any_columns_possible,
                                &start, &end)) {
    if (textord_debug_tabfind >= 2)
      tprintf("Biggest unassigned range = %d- %d\n", start, end);
    // Find the modal column_set_id in the range.
    int column_set_id = RangeModalColumnSet(column_set_costs,
                                            assigned_costs, start, end);
    if (textord_debug_tabfind >= 2) {
      tprintf("Range modal column id = %d\n", column_set_id);
      column_sets_.get(column_set_id)->Print();
    }
    // Now find the longest run of the column_set_id in the range.
    ShrinkRangeToLongestRun(column_set_costs, assigned_costs,
                            any_columns_possible,
                            column_set_id, &start, &end);
    if (textord_debug_tabfind >= 2)
      tprintf("Shrunk range = %d- %d\n", start, end);
    // Extend the start and end past the longest run, while there are
    // only small gaps in compatibility that can be overcome by larger
    // regions of compatibility beyond.
    ExtendRangePastSmallGaps(column_set_costs, assigned_costs,
                             any_columns_possible,
                             column_set_id, -1, -1, &start);
    --end;
    ExtendRangePastSmallGaps(column_set_costs, assigned_costs,
                             any_columns_possible,
                             column_set_id, 1, set_count, &end);
    ++end;
    if (textord_debug_tabfind)
      tprintf("Column id %d applies to range = %d - %d\n",
              column_set_id, start, end);
    // Assign the column to the range, which now may overlap with other ranges.
    AssignColumnToRange(column_set_id, start, end, column_set_costs,
                        assigned_costs);
    if (column_sets_.get(column_set_id)->GoodColumnCount() > 1)
      any_multi_column = true;
  }
  // If anything remains unassigned, the whole lot is unassigned, so
  // arbitrarily assign id 0.
  if (best_columns_[0] == nullptr) {
    AssignColumnToRange(0, 0, gridheight_, column_set_costs, assigned_costs);
  }
  // Free memory.
  for (int i = 0; i < set_count; ++i) {
    delete [] column_set_costs[i];
  }
  delete [] assigned_costs;
  delete [] any_columns_possible;
  delete [] column_set_costs;
  return any_multi_column;
}

// Finds the biggest range in part_sets_ that has no assigned column, but
// column assignment is possible.
bool ColumnFinder::BiggestUnassignedRange(int set_count,
                                          const bool* any_columns_possible,
                                          int* best_start, int* best_end) {
  int best_range_size = 0;
  *best_start = set_count;
  *best_end = set_count;
  int end = set_count;
  for (int start = 0; start < gridheight_; start = end) {
    // Find the first unassigned index in start.
    while (start < set_count) {
      if (best_columns_[start] == nullptr && any_columns_possible[start])
        break;
      ++start;
    }
    // Find the first past the end and count the good ones in between.
    int range_size = 1;  // Number of non-null, but unassigned line sets.
    end = start + 1;
    while (end < set_count) {
      if (best_columns_[end] != nullptr)
        break;
      if (any_columns_possible[end])
        ++range_size;
      ++end;
    }
    if (start < set_count && range_size > best_range_size) {
      best_range_size = range_size;
      *best_start = start;
      *best_end = end;
    }
  }
  return *best_start < *best_end;
}

// Finds the modal compatible column_set_ index within the given range.
int ColumnFinder::RangeModalColumnSet(int** column_set_costs,
                                      const int* assigned_costs,
                                      int start, int end) {
  int column_count = column_sets_.size();
  STATS column_stats(0, column_count);
  for (int part_i = start; part_i < end; ++part_i) {
    for (int col_j = 0; col_j < column_count; ++col_j) {
      if (column_set_costs[part_i][col_j] < assigned_costs[part_i])
        column_stats.add(col_j, 1);
    }
  }
  ASSERT_HOST(column_stats.get_total() > 0);
  return column_stats.mode();
}

// Given that there are many column_set_id compatible columns in the range,
// shrinks the range to the longest contiguous run of compatibility, allowing
// gaps where no columns are possible, but not where competing columns are
// possible.
void ColumnFinder::ShrinkRangeToLongestRun(int** column_set_costs,
                                           const int* assigned_costs,
                                           const bool* any_columns_possible,
                                           int column_set_id,
                                           int* best_start, int* best_end) {
  // orig_start and orig_end are the maximum range we will look at.
  int orig_start = *best_start;
  int orig_end = *best_end;
  int best_range_size = 0;
  *best_start = orig_end;
  *best_end = orig_end;
  int end = orig_end;
  for (int start = orig_start; start < orig_end; start = end) {
    // Find the first possible
    while (start < orig_end) {
      if (column_set_costs[start][column_set_id] < assigned_costs[start] ||
          !any_columns_possible[start])
        break;
      ++start;
    }
    // Find the first past the end.
    end = start + 1;
    while (end < orig_end) {
      if (column_set_costs[end][column_set_id] >= assigned_costs[start] &&
          any_columns_possible[end])
          break;
      ++end;
    }
    if (start < orig_end && end - start > best_range_size) {
      best_range_size = end - start;
      *best_start = start;
      *best_end = end;
    }
  }
}

// Moves start in the direction of step, up to, but not including end while
// the only incompatible regions are no more than kMaxIncompatibleColumnCount
// in size, and the compatible regions beyond are bigger.
void ColumnFinder::ExtendRangePastSmallGaps(int** column_set_costs,
                                            const int* assigned_costs,
                                            const bool* any_columns_possible,
                                            int column_set_id,
                                            int step, int end, int* start) {
  if (textord_debug_tabfind > 2)
    tprintf("Starting expansion at %d, step=%d, limit=%d\n",
            *start, step, end);
  if (*start == end)
    return;  // Cannot be expanded.

  int barrier_size = 0;
  int good_size = 0;
  do {
    // Find the size of the incompatible barrier.
    barrier_size = 0;
    int i;
    for (i = *start + step; i != end; i += step) {
      if (column_set_costs[i][column_set_id] < assigned_costs[i])
        break;  // We are back on.
      // Locations where none are possible don't count.
      if (any_columns_possible[i])
        ++barrier_size;
    }
    if (textord_debug_tabfind > 2)
      tprintf("At %d, Barrier size=%d\n", i, barrier_size);
    if (barrier_size > kMaxIncompatibleColumnCount)
      return;  // Barrier too big.
    if (i == end) {
      // We can't go any further, but the barrier was small, so go to the end.
      *start = i - step;
      return;
    }
    // Now find the size of the good region on the other side.
    good_size = 1;
    for (i += step; i != end; i += step) {
      if (column_set_costs[i][column_set_id] < assigned_costs[i])
        ++good_size;
      else if (any_columns_possible[i])
        break;
    }
    if (textord_debug_tabfind > 2)
      tprintf("At %d, good size = %d\n", i, good_size);
    // If we had enough good ones we can extend the start and keep looking.
    if (good_size >= barrier_size)
      *start = i - step;
  } while (good_size >= barrier_size);
}

// Assigns the given column_set_id to the given range.
void ColumnFinder::AssignColumnToRange(int column_set_id, int start, int end,
                                       int** column_set_costs,
                                       int* assigned_costs) {
  ColPartitionSet* column_set = column_sets_.get(column_set_id);
  for (int i = start; i < end; ++i) {
    assigned_costs[i] = column_set_costs[i][column_set_id];
    best_columns_[i] = column_set;
  }
}

// Computes the mean_column_gap_.
void ColumnFinder::ComputeMeanColumnGap(bool any_multi_column) {
  int total_gap = 0;
  int total_width = 0;
  int gap_samples = 0;
  int width_samples = 0;
  for (int i = 0; i < gridheight_; ++i) {
    ASSERT_HOST(best_columns_[i] != nullptr);
    best_columns_[i]->AccumulateColumnWidthsAndGaps(&total_width,
                                                    &width_samples,
                                                    &total_gap,
                                                    &gap_samples);
  }
  mean_column_gap_ = any_multi_column && gap_samples > 0
      ? total_gap / gap_samples : width_samples > 0
      ? total_width / width_samples : 0;
}

//////// Functions that manipulate ColPartitions in the part_grid_ /////
//////// to split, merge, find margins, and find types.  //////////////

// Helper to delete all the deletable blobs on the list. Owned blobs are
// extracted from the list, but not deleted, leaving them owned by the owner().
static void ReleaseAllBlobsAndDeleteUnused(BLOBNBOX_LIST* blobs) {
  for (BLOBNBOX_IT blob_it(blobs); !blob_it.empty(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.extract();
    if (blob->owner() == nullptr) {
      delete blob->cblob();
      delete blob;
    }
  }
}

// Hoovers up all un-owned blobs and deletes them.
// The rest get released from the block so the ColPartitions can pass
// ownership to the output blocks.
void ColumnFinder::ReleaseBlobsAndCleanupUnused(TO_BLOCK* block) {
  ReleaseAllBlobsAndDeleteUnused(&block->blobs);
  ReleaseAllBlobsAndDeleteUnused(&block->small_blobs);
  ReleaseAllBlobsAndDeleteUnused(&block->noise_blobs);
  ReleaseAllBlobsAndDeleteUnused(&block->large_blobs);
  ReleaseAllBlobsAndDeleteUnused(&image_bblobs_);
}

// Splits partitions that cross columns where they have nothing in the gap.
void ColumnFinder::GridSplitPartitions() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* dont_repeat = nullptr;
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->blob_type() < BRT_UNKNOWN || part == dont_repeat)
      continue;  // Only applies to text partitions.
    ColPartitionSet* column_set = best_columns_[gsearch.GridY()];
    int first_col = -1;
    int last_col = -1;
    // Find which columns the partition spans.
    part->ColumnRange(resolution_, column_set, &first_col, &last_col);
    if (first_col > 0)
      --first_col;
    // Convert output column indices to physical column indices.
    first_col /= 2;
    last_col /= 2;
    // We will only consider cases where a partition spans two columns,
    // since a heading that spans more columns than that is most likely
    // genuine.
    if (last_col != first_col + 1)
      continue;
    // Set up a rectangle search x-bounded by the column gap and y by the part.
    int y = part->MidY();
    TBOX margin_box = part->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(2, margin_box.left(),
                                               margin_box.bottom());
    if (debug) {
      tprintf("Considering partition for GridSplit:");
      part->Print();
    }
    ColPartition* column = column_set->GetColumnByIndex(first_col);
    if (column == nullptr)
      continue;
    margin_box.set_left(column->RightAtY(y) + 2);
    column = column_set->GetColumnByIndex(last_col);
    if (column == nullptr)
      continue;
    margin_box.set_right(column->LeftAtY(y) - 2);
    // TODO(rays) Decide whether to keep rectangular filling or not in the
    // main grid and therefore whether we need a fancier search here.
    // Now run the rect search on the main blob grid.
    GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rectsearch(this);
    if (debug) {
      tprintf("Searching box (%d,%d)->(%d,%d)\n",
              margin_box.left(), margin_box.bottom(),
              margin_box.right(), margin_box.top());
      part->Print();
    }
    rectsearch.StartRectSearch(margin_box);
    BLOBNBOX* bbox;
    while ((bbox = rectsearch.NextRectSearch()) != nullptr) {
      if (bbox->bounding_box().overlap(margin_box))
        break;
    }
    if (bbox == nullptr) {
      // There seems to be nothing in the hole, so split the partition.
      gsearch.RemoveBBox();
      int x_middle = (margin_box.left() + margin_box.right()) / 2;
      if (debug) {
        tprintf("Splitting part at %d:", x_middle);
        part->Print();
      }
      ColPartition* split_part = part->SplitAt(x_middle);
      if (split_part != nullptr) {
        if (debug) {
          tprintf("Split result:");
          part->Print();
          split_part->Print();
        }
        part_grid_.InsertBBox(true, true, split_part);
      } else {
        // Split had no effect
        if (debug)
          tprintf("Split had no effect\n");
        dont_repeat = part;
      }
      part_grid_.InsertBBox(true, true, part);
      gsearch.RepositionIterator();
    } else if (debug) {
      tprintf("Part cannot be split: blob (%d,%d)->(%d,%d) in column gap\n",
              bbox->bounding_box().left(), bbox->bounding_box().bottom(),
              bbox->bounding_box().right(), bbox->bounding_box().top());
    }
  }
}

// Merges partitions where there is vertical overlap, within a single column,
// and the horizontal gap is small enough.
void ColumnFinder::GridMergePartitions() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->IsUnMergeableType())
      continue;
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet* columns = best_columns_[gsearch.GridY()];
    TBOX box = part->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(1, box.left(), box.bottom());
    if (debug) {
      tprintf("Considering part for merge at:");
      part->Print();
    }
    int y = part->MidY();
    ColPartition* left_column = columns->ColumnContaining(box.left(), y);
    ColPartition* right_column = columns->ColumnContaining(box.right(), y);
    if (left_column == nullptr || right_column != left_column) {
      if (debug)
        tprintf("In different columns\n");
      continue;
    }
    box.set_left(left_column->LeftAtY(y));
    box.set_right(right_column->RightAtY(y));
    // Now run the rect search.
    bool modified_box = false;
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rsearch(&part_grid_);
    rsearch.SetUniqueMode(true);
    rsearch.StartRectSearch(box);
    ColPartition* neighbour;

    while ((neighbour = rsearch.NextRectSearch()) != nullptr) {
      if (neighbour == part || neighbour->IsUnMergeableType())
        continue;
      const TBOX& neighbour_box = neighbour->bounding_box();
      if (debug) {
        tprintf("Considering merge with neighbour at:");
        neighbour->Print();
      }
      if (neighbour_box.right() < box.left() ||
          neighbour_box.left() > box.right())
        continue;  // Not within the same column.
      if (part->VSignificantCoreOverlap(*neighbour) &&
          part->TypesMatch(*neighbour)) {
        // There is vertical overlap and the gross types match, but only
        // merge if the horizontal gap is small enough, as one of the
        // partitions may be a figure caption within a column.
        // If there is only one column, then the mean_column_gap_ is large
        // enough to allow almost any merge, by being the mean column width.
        const TBOX& part_box = part->bounding_box();
        // Don't merge if there is something else in the way. Use the margin
        // to decide, and check both to allow a bit of overlap.
        if (neighbour_box.left() > part->right_margin() &&
            part_box.right() < neighbour->left_margin())
          continue;  // Neighbour is too far to the right.
        if (neighbour_box.right() < part->left_margin() &&
            part_box.left() > neighbour->right_margin())
          continue;  // Neighbour is too far to the left.
        int h_gap = std::max(part_box.left(), neighbour_box.left()) -
                std::min(part_box.right(), neighbour_box.right());
        if (h_gap < mean_column_gap_ * kHorizontalGapMergeFraction ||
            part_box.width() < mean_column_gap_ ||
            neighbour_box.width() < mean_column_gap_) {
          if (debug) {
            tprintf("Running grid-based merge between:\n");
            part->Print();
            neighbour->Print();
          }
          rsearch.RemoveBBox();
          if (!modified_box) {
            // We are going to modify part, so remove it and re-insert it after.
            gsearch.RemoveBBox();
            rsearch.RepositionIterator();
            modified_box = true;
          }
          part->Absorb(neighbour, WidthCB());
        } else if (debug) {
          tprintf("Neighbour failed hgap test\n");
        }
      } else if (debug) {
        tprintf("Neighbour failed overlap or typesmatch test\n");
      }
    }
    if (modified_box) {
      // We modified the box of part, so re-insert it into the grid.
      // This does no harm in the current cell, as it already exists there,
      // but it needs to exist in all the cells covered by its bounding box,
      // or it will never be found by a full search.
      // Because the box has changed, it has to be removed first, otherwise
      // add_sorted may fail to keep a single copy of the pointer.
      part_grid_.InsertBBox(true, true, part);
      gsearch.RepositionIterator();
    }
  }
}

// Inserts remaining noise blobs into the most applicable partition if any.
// If there is no applicable partition, then the blobs are deleted.
void ColumnFinder::InsertRemainingNoise(TO_BLOCK* block) {
  BLOBNBOX_IT blob_it(&block->noise_blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->owner() != nullptr) continue;
    TBOX search_box(blob->bounding_box());
    bool debug = WithinTestRegion(2, search_box.left(), search_box.bottom());
    search_box.pad(gridsize(), gridsize());
    // Setup a rectangle search to find the best partition to merge with.
    ColPartitionGridSearch rsearch(&part_grid_);
    rsearch.SetUniqueMode(true);
    rsearch.StartRectSearch(search_box);
    ColPartition* part;
    ColPartition* best_part = nullptr;
    int best_distance = 0;
    while ((part = rsearch.NextRectSearch()) != nullptr) {
      if (part->IsUnMergeableType())
        continue;
      int distance = projection_.DistanceOfBoxFromPartition(
          blob->bounding_box(), *part, denorm_, debug);
      if (best_part == nullptr || distance < best_distance) {
        best_part = part;
        best_distance = distance;
      }
    }
    if (best_part != nullptr &&
        best_distance < kMaxDistToPartSizeRatio * best_part->median_height()) {
      // Close enough to merge.
      if (debug) {
        tprintf("Adding noise blob with distance %d, thr=%g:box:",
                best_distance,
                kMaxDistToPartSizeRatio * best_part->median_height());
        blob->bounding_box().print();
        tprintf("To partition:");
        best_part->Print();
      }
      part_grid_.RemoveBBox(best_part);
      best_part->AddBox(blob);
      part_grid_.InsertBBox(true, true, best_part);
      blob->set_owner(best_part);
      blob->set_flow(best_part->flow());
      blob->set_region_type(best_part->blob_type());
    } else {
      // Mark the blob for deletion.
      blob->set_region_type(BRT_NOISE);
    }
  }
  // Delete the marked blobs, clearing neighbour references.
  block->DeleteUnownedNoise();
}

// Helper makes a box from a horizontal line.
static TBOX BoxFromHLine(const TabVector* hline) {
  int top = std::max(hline->startpt().y(), hline->endpt().y());
  int bottom = std::min(hline->startpt().y(), hline->endpt().y());
  top += hline->mean_width();
  if (top == bottom) {
    if (bottom > 0)
      --bottom;
    else
      ++top;
  }
  return TBOX(hline->startpt().x(), bottom, hline->endpt().x(), top);
}

// Remove partitions that come from horizontal lines that look like
// underlines, but are not part of a table.
void ColumnFinder::GridRemoveUnderlinePartitions() {
  TabVector_IT hline_it(&horizontal_lines_);
  for (hline_it.mark_cycle_pt(); !hline_it.cycled_list(); hline_it.forward()) {
    TabVector* hline = hline_it.data();
    if (hline->intersects_other_lines())
      continue;
    TBOX line_box = BoxFromHLine(hline);
    TBOX search_box = line_box;
    search_box.pad(0, line_box.height());
    ColPartitionGridSearch part_search(&part_grid_);
    part_search.SetUniqueMode(true);
    part_search.StartRectSearch(search_box);
    ColPartition* covered;
    bool touched_table = false;
    bool touched_text = false;
    ColPartition* line_part = nullptr;
    while ((covered = part_search.NextRectSearch()) != nullptr) {
      if (covered->type() == PT_TABLE) {
        touched_table = true;
        break;
      } else if (covered->IsTextType()) {
        // TODO(rays) Add a list of underline sections to ColPartition.
        int text_bottom = covered->median_bottom();
        if (line_box.bottom() <= text_bottom && text_bottom <= search_box.top())
          touched_text = true;
      } else if (covered->blob_type() == BRT_HLINE &&
          line_box.contains(covered->bounding_box()) &&
          // not if same instance (identical to hline)
          !TBOX(covered->bounding_box()).contains(line_box)) {
        line_part = covered;
      }
    }
    if (line_part != nullptr && !touched_table && touched_text) {
      part_grid_.RemoveBBox(line_part);
      delete line_part;
    }
  }
}

// Add horizontal line separators as partitions.
void ColumnFinder::GridInsertHLinePartitions() {
  TabVector_IT hline_it(&horizontal_lines_);
  for (hline_it.mark_cycle_pt(); !hline_it.cycled_list(); hline_it.forward()) {
    TabVector* hline = hline_it.data();
    TBOX line_box = BoxFromHLine(hline);
    ColPartition* part = ColPartition::MakeLinePartition(
        BRT_HLINE, vertical_skew_,
        line_box.left(), line_box.bottom(), line_box.right(), line_box.top());
    part->set_type(PT_HORZ_LINE);
    bool any_image = false;
    ColPartitionGridSearch part_search(&part_grid_);
    part_search.SetUniqueMode(true);
    part_search.StartRectSearch(line_box);
    ColPartition* covered;
    while ((covered = part_search.NextRectSearch()) != nullptr) {
      if (covered->IsImageType()) {
        any_image = true;
        break;
      }
    }
    if (!any_image)
      part_grid_.InsertBBox(true, true, part);
    else
      delete part;
  }
}

// Add horizontal line separators as partitions.
void ColumnFinder::GridInsertVLinePartitions() {
  TabVector_IT vline_it(dead_vectors());
  for (vline_it.mark_cycle_pt(); !vline_it.cycled_list(); vline_it.forward()) {
    TabVector* vline = vline_it.data();
    if (!vline->IsSeparator())
      continue;
    int left = std::min(vline->startpt().x(), vline->endpt().x());
    int right = std::max(vline->startpt().x(), vline->endpt().x());
    right += vline->mean_width();
    if (left == right) {
      if (left > 0)
        --left;
      else
        ++right;
    }
    ColPartition* part = ColPartition::MakeLinePartition(
        BRT_VLINE, vertical_skew_,
        left, vline->startpt().y(), right, vline->endpt().y());
    part->set_type(PT_VERT_LINE);
    bool any_image = false;
    ColPartitionGridSearch part_search(&part_grid_);
    part_search.SetUniqueMode(true);
    part_search.StartRectSearch(part->bounding_box());
    ColPartition* covered;
    while ((covered = part_search.NextRectSearch()) != nullptr) {
      if (covered->IsImageType()) {
        any_image = true;
        break;
      }
    }
    if (!any_image)
      part_grid_.InsertBBox(true, true, part);
    else
      delete part;
  }
}

// For every ColPartition in the grid, sets its type based on position
// in the columns.
void ColumnFinder::SetPartitionTypes() {
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part->SetPartitionType(resolution_, best_columns_[gsearch.GridY()]);
  }
}

// Only images remain with multiple types in a run of partners.
// Sets the type of all in the group to the maximum of the group.
void ColumnFinder::SmoothPartnerRuns() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    ColPartition* partner = part->SingletonPartner(true);
    if (partner != nullptr) {
      if (partner->SingletonPartner(false) != part) {
        tprintf("Ooops! Partition:(%d partners)",
                part->upper_partners()->length());
        part->Print();
        tprintf("has singleton partner:(%d partners",
                partner->lower_partners()->length());
        partner->Print();
        tprintf("but its singleton partner is:");
        if (partner->SingletonPartner(false) == nullptr)
          tprintf("NULL\n");
        else
          partner->SingletonPartner(false)->Print();
      }
      ASSERT_HOST(partner->SingletonPartner(false) == part);
    } else if (part->SingletonPartner(false) != nullptr) {
      ColPartitionSet* column_set = best_columns_[gsearch.GridY()];
      int column_count = column_set->ColumnCount();
      part->SmoothPartnerRun(column_count * 2 + 1);
    }
  }
}

// Helper functions for TransformToBlocks.
// Add the part to the temp list in the correct order.
void ColumnFinder::AddToTempPartList(ColPartition* part,
                                     ColPartition_CLIST* temp_list) {
  int mid_y = part->MidY();
  ColPartition_C_IT it(temp_list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* test_part = it.data();
    if (part->type() == PT_NOISE || test_part->type() == PT_NOISE)
      continue;  // Noise stays in sequence.
    if (test_part == part->SingletonPartner(false))
      break;  // Insert before its lower partner.
    int neighbour_bottom = test_part->median_bottom();
    int neighbour_top = test_part->median_top();
    int neighbour_y = (neighbour_bottom + neighbour_top) / 2;
    if (neighbour_y < mid_y)
      break;  // part is above test_part so insert it.
    if (!part->HOverlaps(*test_part) && !part->WithinSameMargins(*test_part))
      continue;  // Incompatibles stay in order
  }
  if (it.cycled_list()) {
    it.add_to_end(part);
  } else {
    it.add_before_stay_put(part);
  }
}

// Add everything from the temp list to the work_set assuming correct order.
void ColumnFinder::EmptyTempPartList(ColPartition_CLIST* temp_list,
                                     WorkingPartSet_LIST* work_set) {
  ColPartition_C_IT it(temp_list);
  while (!it.empty()) {
    it.extract()->AddToWorkingSet(bleft_, tright_, resolution_,
                          &good_parts_, work_set);
    it.forward();
  }
}

// Transform the grid of partitions to the output blocks.
void ColumnFinder::TransformToBlocks(BLOCK_LIST* blocks,
                                     TO_BLOCK_LIST* to_blocks) {
  WorkingPartSet_LIST work_set;
  ColPartitionSet* column_set = nullptr;
  ColPartition_IT noise_it(&noise_parts_);
  // The temp_part_list holds a list of parts at the same grid y coord
  // so they can be added in the correct order. This prevents thin objects
  // like horizontal lines going before the text lines above them.
  ColPartition_CLIST temp_part_list;
  // Iterate the ColPartitions in the grid. It starts at the top
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  int prev_grid_y = -1;
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    int grid_y = gsearch.GridY();
    if (grid_y != prev_grid_y) {
      EmptyTempPartList(&temp_part_list, &work_set);
      prev_grid_y = grid_y;
    }
    if (best_columns_[grid_y] != column_set) {
      column_set = best_columns_[grid_y];
      // Every line should have a non-null best column.
      ASSERT_HOST(column_set != nullptr);
      column_set->ChangeWorkColumns(bleft_, tright_, resolution_,
                                    &good_parts_, &work_set);
      if (textord_debug_tabfind)
        tprintf("Changed column groups at grid index %d, y=%d\n",
                gsearch.GridY(), gsearch.GridY() * gridsize());
    }
    if (part->type() == PT_NOISE) {
      noise_it.add_to_end(part);
    } else {
      AddToTempPartList(part, &temp_part_list);
    }
  }
  EmptyTempPartList(&temp_part_list, &work_set);
  // Now finish all working sets and transfer ColPartitionSets to block_sets.
  WorkingPartSet_IT work_it(&work_set);
  while (!work_it.empty()) {
    WorkingPartSet* working_set = work_it.extract();
    working_set->ExtractCompletedBlocks(bleft_, tright_, resolution_,
                                        &good_parts_, blocks, to_blocks);
    delete working_set;
    work_it.forward();
  }
}

// Helper reflects a list of blobs in the y-axis.
// Only reflects the BLOBNBOX bounding box. Not the blobs or outlines below.
static void ReflectBlobList(BLOBNBOX_LIST* bblobs) {
  BLOBNBOX_IT it(bblobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->reflect_box_in_y_axis();
  }
}

// Reflect the blob boxes (but not the outlines) in the y-axis so that
// the blocks get created in the correct RTL order. Reflects the blobs
// in the input_block and the bblobs list.
// The reflection is undone in RotateAndReskewBlocks by
// reflecting the blocks themselves, and then recomputing the blob bounding
// boxes.
void ColumnFinder::ReflectForRtl(TO_BLOCK* input_block, BLOBNBOX_LIST* bblobs) {
  ReflectBlobList(bblobs);
  ReflectBlobList(&input_block->blobs);
  ReflectBlobList(&input_block->small_blobs);
  ReflectBlobList(&input_block->noise_blobs);
  ReflectBlobList(&input_block->large_blobs);
  // Update the denorm with the reflection.
  auto* new_denorm = new DENORM;
  new_denorm->SetupNormalization(nullptr, nullptr, denorm_,
                                 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f);
  denorm_ = new_denorm;
}

// Helper fixes up blobs and cblobs to match the desired rotation,
// exploding multi-outline blobs back to single blobs and accumulating
// the bounding box widths and heights.
static void RotateAndExplodeBlobList(const FCOORD& blob_rotation,
                                     BLOBNBOX_LIST* bblobs,
                                     STATS* widths,
                                     STATS* heights) {
  BLOBNBOX_IT it(bblobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    C_BLOB* cblob = blob->cblob();
    C_OUTLINE_LIST* outlines = cblob->out_list();
    C_OUTLINE_IT ol_it(outlines);
    if (!outlines->singleton()) {
      // This blob has multiple outlines from CJK repair.
      // Explode the blob back into individual outlines.
      for (;!ol_it.empty(); ol_it.forward()) {
        C_OUTLINE* outline = ol_it.extract();
        BLOBNBOX* new_blob = BLOBNBOX::RealBlob(outline);
        // This blob will be revisited later since we add_after_stay_put here.
        // This means it will get rotated and have its width/height added to
        // the stats below.
        it.add_after_stay_put(new_blob);
      }
      it.extract();
      delete cblob;
      delete blob;
    } else {
      if (blob_rotation.x() != 1.0f || blob_rotation.y() != 0.0f) {
        cblob->rotate(blob_rotation);
      }
      blob->compute_bounding_box();
      widths->add(blob->bounding_box().width(), 1);
      heights->add(blob->bounding_box().height(), 1);
    }
  }
}

// Undo the deskew that was done in FindTabVectors, as recognition is done
// without correcting blobs or blob outlines for skew.
// Reskew the completed blocks to put them back to the original rotated coords
// that were created by CorrectOrientation.
// If the input_is_rtl, then reflect the blocks in the y-axis to undo the
// reflection that was done before FindTabVectors.
// Blocks that were identified as vertical text (relative to the rotated
// coordinates) are further rotated so the text lines are horizontal.
// blob polygonal outlines are rotated to match the position of the blocks
// that they are in, and their bounding boxes are recalculated to be accurate.
// Record appropriate inverse transformations and required
// classifier transformation in the blocks.
void ColumnFinder::RotateAndReskewBlocks(bool input_is_rtl,
                                         TO_BLOCK_LIST* blocks) {
  if (input_is_rtl) {
    // The skew is backwards because of the reflection.
    FCOORD tmp = deskew_;
    deskew_ = reskew_;
    reskew_ = tmp;
  }
  TO_BLOCK_IT it(blocks);
  int block_index = 1;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TO_BLOCK* to_block = it.data();
    BLOCK* block = to_block->block;
    // Blocks are created on the deskewed blob outlines in TransformToBlocks()
    // so we need to reskew them back to page coordinates.
    if (input_is_rtl) {
      block->reflect_polygon_in_y_axis();
    }
    block->rotate(reskew_);
    // Copy the right_to_left flag to the created block.
    block->set_right_to_left(input_is_rtl);
    // Save the skew angle in the block for baseline computations.
    block->set_skew(reskew_);
    block->pdblk.set_index(block_index++);
    FCOORD blob_rotation = ComputeBlockAndClassifyRotation(block);
    // Rotate all the blobs if needed and recompute the bounding boxes.
    // Compute the block median blob width and height as we go.
    STATS widths(0, block->pdblk.bounding_box().width());
    STATS heights(0, block->pdblk.bounding_box().height());
    RotateAndExplodeBlobList(blob_rotation, &to_block->blobs,
                             &widths, &heights);
    TO_ROW_IT row_it(to_block->get_rows());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      TO_ROW* row = row_it.data();
      RotateAndExplodeBlobList(blob_rotation, row->blob_list(),
                               &widths, &heights);
    }
    block->set_median_size(static_cast<int>(widths.median() + 0.5),
                           static_cast<int>(heights.median() + 0.5));
    if (textord_debug_tabfind >= 2)
      tprintf("Block median size = (%d, %d)\n",
              block->median_size().x(), block->median_size().y());
  }
}

// Computes the rotations for the block (to make textlines horizontal) and
// for the blobs (for classification) and sets the appropriate members
// of the given block.
// Returns the rotation that needs to be applied to the blobs to make
// them sit in the rotated block.
FCOORD ColumnFinder::ComputeBlockAndClassifyRotation(BLOCK* block) {
  // The text_rotation_ tells us the gross page text rotation that needs
  // to be applied for classification
  // TODO(rays) find block-level classify rotation by orientation detection.
  // In the mean time, assume that "up" for text printed in the minority
  // direction (PT_VERTICAL_TEXT) is perpendicular to the line of reading.
  // Accomplish this by zero-ing out the text rotation.  This covers the
  // common cases of image credits in documents written in Latin scripts
  // and page headings for predominantly vertically written CJK books.
  FCOORD classify_rotation(text_rotation_);
  FCOORD block_rotation(1.0f, 0.0f);
  if (block->pdblk.poly_block()->isA() == PT_VERTICAL_TEXT) {
    // Vertical text needs to be 90 degrees rotated relative to the rest.
    // If the rest has a 90 degree rotation already, use the inverse, making
    // the vertical text the original way up. Otherwise use 90 degrees
    // clockwise.
    if (rerotate_.x() == 0.0f)
      block_rotation = rerotate_;
    else
      block_rotation = FCOORD(0.0f, -1.0f);
    block->rotate(block_rotation);
    classify_rotation = FCOORD(1.0f, 0.0f);
  }
  block_rotation.rotate(rotation_);
  // block_rotation is now what we have done to the blocks. Now do the same
  // thing to the blobs, but save the inverse rotation in the block, as that
  // is what we need to DENORM back to the image coordinates.
  FCOORD blob_rotation(block_rotation);
  block_rotation.set_y(-block_rotation.y());
  block->set_re_rotation(block_rotation);
  block->set_classify_rotation(classify_rotation);
  if (textord_debug_tabfind) {
    tprintf("Blk %d, type %d rerotation(%.2f, %.2f), char(%.2f,%.2f), box:",
            block->pdblk.index(), block->pdblk.poly_block()->isA(),
            block->re_rotation().x(), block->re_rotation().y(),
            classify_rotation.x(), classify_rotation.y());
    block->pdblk.bounding_box().print();
  }
  return blob_rotation;
}

}  // namespace tesseract.
