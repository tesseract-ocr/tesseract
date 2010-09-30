///////////////////////////////////////////////////////////////////////
// File:        colfind.cpp
// Description: Class to hold BLOBNBOXs in a grid for fast access
//              to neighbours.
// Author:      Ray Smith
// Created:     Wed Jun 06 17:22:01 PDT 2007
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "colfind.h"
#include "colpartition.h"
#include "colpartitionset.h"
#include "linefind.h"
#include "strokewidth.h"
#include "blobbox.h"
#include "scrollview.h"
#include "tessvars.h"
#include "varable.h"
#include "workingpartset.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

namespace tesseract {

// Minimum width to be considered when making columns.
const int kMinColumnWidth = 100;
// When assigning columns, the max number of misfits that can be ignored.
const int kMaxIncompatibleColumnCount = 2;
// Max vertical distance of neighbouring ColPartition for it to be a partner.
const double kMaxPartitionSpacing = 1.75;
// Min fraction of ColPartition height to be overlapping for margin purposes.
const double kMarginOverlapFraction = 0.25;
// Max fraction of mean_column_gap_ for the gap between two partitions within a
// column to allow them to merge.
const double kHorizontalGapMergeFraction = 0.5;
// Min fraction of grid size to not be considered likely noise.
const double kMinNonNoiseFraction = 0.5;
// Search radius to use for finding large neighbours of smaller blobs.
const int kSmallBlobSearchRadius = 2;

BOOL_VAR(textord_tabfind_show_strokewidths, false, "Show stroke widths");
BOOL_VAR(textord_tabfind_show_initial_partitions,
         false, "Show partition bounds");
INT_VAR(textord_tabfind_show_partitions, 0,
        "Show partition bounds, waiting if >1");
BOOL_VAR(textord_tabfind_show_columns, false, "Show column bounds");
BOOL_VAR(textord_tabfind_show_blocks, false, "Show final block bounds");

ScrollView* ColumnFinder::blocks_win_ = NULL;

// Gridsize is an estimate of the text size in the image. A suitable value
// is in TO_BLOCK::line_size after find_components has been used to make
// the blobs.
// bleft and tright are the bounds of the image (or rectangle) being processed.
// vlines is a (possibly empty) list of TabVector and vertical_x and y are
// the sum logical vertical vector produced by LineFinder::FindVerticalLines.
ColumnFinder::ColumnFinder(int gridsize,
                           const ICOORD& bleft, const ICOORD& tright,
                           TabVector_LIST* vlines, TabVector_LIST* hlines,
                           int vertical_x, int vertical_y)
  : TabFind(gridsize, bleft, tright, vlines, vertical_x, vertical_y),
    mean_column_gap_(tright.x() - bleft.x()),
    global_median_xheight_(0), global_median_ledding_(0),
    reskew_(1.0f, 0.0f), rerotate_(1.0f, 0.0f),
    best_columns_(NULL) {
  TabVector_IT h_it(&horizontal_lines_);
  h_it.add_list_after(hlines);
}

// Templated helper function used to create destructor callbacks for the
// BBGrid::ClearGridData() method.
template <typename T> void DeleteObject(T *object) {
  delete object;
}

ColumnFinder::~ColumnFinder() {
  column_sets_.delete_data_pointers();
  if (best_columns_ != NULL) {
    delete [] best_columns_;
  }
  // ColPartitions and ColSegments created by this class for storage in grids
  // need to be deleted explicitly.
  clean_part_grid_.ClearGridData(&DeleteObject<ColPartition>);
  col_seg_grid_.ClearGridData(&DeleteObject<ColSegment>);

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

// Finds the text and image blocks, returning them in the blocks and to_blocks
// lists. (Each TO_BLOCK points to the basic BLOCK and adds more information.)
// If boxa and pixa are not NULL, they are assumed to be the output of
// ImageFinder::FindImages, and are used to generate image blocks.
// The input boxa and pixa are destroyed.
// Imageheight and resolution should be the pixel height and resolution in
// pixels per inch of the original image.
// The input block is the result of a call to find_components, and contains
// the blobs found in the image. These blobs will be removed and placed
// in the output blocks, while unused ones will be deleted.
// If single_column is true, the input is treated as single column, but
// it is still divided into blocks of equal line spacing/text size.
// Returns -1 if the user requested retry with more debug info.
int ColumnFinder::FindBlocks(int imageheight, int resolution,
                             bool single_column, TO_BLOCK* block,
                             Boxa* boxa, Pixa* pixa,
                             BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks) {
#ifdef HAVE_LIBLEPT
  if (boxa != NULL) {
    // Convert the boxa/pixa to fake blobs aligned on the grid.
    ExtractImageBlobs(imageheight, boxa, pixa);
    boxaDestroy(&boxa);
    pixaDestroy(&pixa);
  }
#endif
  // Decide which large blobs should be included in the grid as potential
  // characters.
  // A subsidiary grid used to decide which large blobs to use.
  StrokeWidth* stroke_width = new StrokeWidth(gridsize(), bleft(), tright());
  stroke_width->InsertBlobs(block, this);
  if (textord_tabfind_show_strokewidths) {
    stroke_width->DisplayGoodBlobs("GoodStrokewidths", NULL);
  }
  stroke_width->MoveGoodLargeBlobs(resolution, block);
  delete stroke_width;

  if (single_column) {
    // No tab stops needed. Just the grid that FindTabVectors makes.
    DontFindTabVectors(resolution, &image_bblobs_, block, &reskew_);
  } else {
    // Find the tab stops.
    FindTabVectors(resolution, &horizontal_lines_, &image_bblobs_, block,
                   &reskew_, &rerotate_);
  }

  // Find the columns.
  if (MakeColumnPartitions() == 0)
    return 0;  // This is an empty page.
  // Make the initial column candidates from the part_sets_.
  MakeColumnCandidates(single_column);
  if (textord_debug_tabfind)
    PrintColumnCandidates("Column candidates");
  // Improve the column candidates against themselves.
  ImproveColumnCandidates(&column_sets_, &column_sets_);
  if (textord_debug_tabfind)
    PrintColumnCandidates("Improved columns");
  // Improve the column candidates using the part_sets_.
  ImproveColumnCandidates(&part_sets_, &column_sets_);
  if (textord_debug_tabfind)
    PrintColumnCandidates("Final Columns");
  // Divide the page into sections of uniform column layout.
  AssignColumns();
  if (textord_tabfind_show_columns) {
    DisplayColumnBounds(&part_sets_);
  }
  ComputeMeanColumnGap();

  // Refill the grid using rectangular spreading, and get the benefit
  // of the completed tab vectors marking the rule edges of each blob.
  Clear();
  InsertBlobList(false, false, false, &image_bblobs_, true, this);
  InsertBlobList(true, true, false, &block->blobs, true, this);

  // Insert all the remaining small and noise blobs into the grid and also
  // make an unknown partition for each. Ownership is taken by the grid.
  InsertSmallBlobsAsUnknowns(true, &block->small_blobs);
  InsertSmallBlobsAsUnknowns(true, &block->noise_blobs);

  // Ownership of the ColPartitions moves from part_sets_ to part_grid_ here,
  // and ownership of the BLOBNBOXes moves to the ColPartitions.
  // (They were previously owned by the block or the image_bblobs list,
  // but they both gave up ownership to the grid at the InsertBlobList above.)
  MovePartitionsToGrid();
  // Split and merge the partitions by looking at local neighbours.
  GridSplitPartitions();
  // Resolve unknown partitions by adding to an existing partition, fixing
  // the type, or declaring them noise.
  GridFindMargins();
  ListFindMargins(&unknown_parts_);
  GridInsertUnknowns();
  GridMergePartitions();
  // Add horizontal line separators as partitions.
  GridInsertHLinePartitions();
  // Recompute margins based on a local neighbourhood search.
  GridFindMargins();
  SetPartitionTypes();
  if (textord_tabfind_show_initial_partitions) {
    ScrollView* part_win = MakeWindow(100, 300, "InitialPartitions");
    part_grid_.DisplayBoxes(part_win);
    DisplayTabVectors(part_win);
  }

  // Copy cleaned partitions from part_grid_ to clean_part_grid_ and
  // insert dot-like noise into period_grid_
  GetCleanPartitions(block);

  // Get Table Regions
  LocateTables();

  // Build the partitions into chains that belong in the same block and
  // refine into one-to-one links, then smooth the types within each chain.
  FindPartitionPartners();
  RefinePartitionPartners();
  SmoothPartnerRuns();
  if (textord_tabfind_show_partitions) {
    ScrollView* window = MakeWindow(400, 300, "Partitions");
    if (textord_debug_images)
      window->Image(AlignedBlob::textord_debug_pix().string(),
                    image_origin().x(), image_origin().y());
    part_grid_.DisplayBoxes(window);
    if (!textord_debug_printable)
      DisplayTabVectors(window);
    if (window != NULL && textord_tabfind_show_partitions > 1) {
      delete window->AwaitEvent(SVET_DESTROY);
    }
  }
  part_grid_.AssertNoDuplicates();
  // Ownership of the ColPartitions moves from part_grid_ to good_parts_ and
  // noise_parts_ here. In text blocks, ownership of the BLOBNBOXes moves
  // from the ColPartitions to the output TO_BLOCK. In non-text, the
  // BLOBNBOXes stay with the ColPartitions and get deleted in the destructor.
  TransformToBlocks(blocks, to_blocks);
  if (textord_debug_tabfind) {
    tprintf("Found %d blocks, %d to_blocks\n",
            blocks->length(), to_blocks->length());
  }

  DisplayBlocks(blocks);
//  MoveSmallBlobs(&block->small_blobs, to_blocks);
//  MoveSmallBlobs(&block->noise_blobs, to_blocks);
//  MoveSmallBlobs(&period_blobs_, to_blocks);
  RotateAndReskewBlocks(to_blocks);
  int result = 0;
  if (blocks_win_ != NULL) {
    bool waiting = false;
    do {
      waiting = false;
      SVEvent* event = blocks_win_->AwaitEvent(SVET_ANY);
      if (event->type == SVET_INPUT && event->parameter != NULL) {
        if (*event->parameter == 'd')
          result = -1;
        else
          blocks->clear();
      } else if (event->type == SVET_DESTROY) {
        blocks_win_ = NULL;
      } else {
        waiting = true;
      }
      delete event;
    } while (waiting);
  }
  return result;
}

//////////////// PRIVATE CODE /////////////////////////

// Displays the blob and block bounding boxes in a window called Blocks.
void ColumnFinder::DisplayBlocks(BLOCK_LIST* blocks) {
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_blocks) {
    if (blocks_win_ == NULL)
      blocks_win_ = MakeWindow(700, 300, "Blocks");
    else
      blocks_win_->Clear();
    if (textord_debug_images)
      blocks_win_->Image(AlignedBlob::textord_debug_pix().string(),
                         image_origin().x(), image_origin().y());
    else
      DisplayBoxes(blocks_win_);
    BLOCK_IT block_it(blocks);
    int serial = 1;
    for (block_it.mark_cycle_pt(); !block_it.cycled_list();
         block_it.forward()) {
      BLOCK* block = block_it.data();
      block->plot(blocks_win_, serial++,
                  textord_debug_printable ? ScrollView::BLUE
                                          : ScrollView::GREEN);
    }
    blocks_win_->Update();
  }
#endif
}

// Displays the column edges at each grid y coordinate defined by
// best_columns_.
void ColumnFinder::DisplayColumnBounds(PartSetVector* sets) {
#ifndef GRAPHICS_DISABLED
  ScrollView* col_win = MakeWindow(50, 300, "Columns");
  if (textord_debug_images)
    col_win->Image(AlignedBlob::textord_debug_pix().string(),
                   image_origin().x(), image_origin().y());
  else
    DisplayBoxes(col_win);
  col_win->Pen(textord_debug_printable ? ScrollView::BLUE : ScrollView::GREEN);
  for (int i = 0; i < gridheight_; ++i) {
    ColPartitionSet* columns = best_columns_[i];
    if (columns != NULL)
      columns->DisplayColumnEdges(i * gridsize_, (i + 1) * gridsize_, col_win);
  }
#endif
}

// Converts the arrays of Box/Pix to a list of C_OUTLINE, and then to blobs.
// The output is a list of C_BLOBs for the images, but the C_OUTLINEs
// contain no data.
void ColumnFinder::ExtractImageBlobs(int image_height, Boxa* boxa, Pixa* pixa) {
#ifdef HAVE_LIBLEPT
  BLOBNBOX_IT bb_it(&image_bblobs_);
  // Iterate the connected components in the image regions mask.
  int nboxes = boxaGetCount(boxa);
  for (int i = 0; i < nboxes; ++i) {
    l_int32 x, y, width, height;
    boxaGetBoxGeometry(boxa, i, &x, &y, &width, &height);
    Pix* pix = pixaGetPix(pixa, i, L_CLONE);
    // Special case set in FindImages:
    // The image is a rectangle if its width doesn't match the box width.
    bool rectangle = width != pixGetWidth(pix);
    // For each grid cell in the pix, find the bounding box of the black
    // pixels within the cell.
    int grid_xmin, grid_ymin, grid_xmax, grid_ymax;
    GridCoords(x, image_height - (y + height), &grid_xmin, &grid_ymin);
    GridCoords(x + width - 1, image_height - 1 - y, &grid_xmax, &grid_ymax);
    for (int grid_y = grid_ymin; grid_y <= grid_ymax; ++grid_y) {
      for (int grid_x = grid_xmin; grid_x <= grid_xmax; ++grid_x) {
        // Compute bounds of grid cell in sub-image.
        int x_start = grid_x * gridsize_ + bleft_.x() - x;
        int y_end = image_height - (grid_y * gridsize_ + bleft_.y()) - y;
        int x_end = x_start + gridsize_;
        int y_start = y_end - gridsize_;
        ImageFinder::BoundsWithinRect(pix, &x_start, &y_start, &x_end, &y_end);
        // If the box is not degenerate, make a blob.
        if (x_end > x_start && y_end > y_start) {
          C_OUTLINE_LIST outlines;
          C_OUTLINE_IT ol_it = &outlines;
          // Make a C_OUTLINE from the bounds. This is a bit of a hack,
          // as there is no outline, just a bounding box, but with some very
          // small changes to coutln.cpp, it works nicely.
          ICOORD top_left(x_start + x, image_height - (y_start + y));
          ICOORD bot_right(x_end + x, image_height - (y_end + y));
          CRACKEDGE startpt;
          startpt.pos = top_left;
          C_OUTLINE* outline = new C_OUTLINE(&startpt, top_left, bot_right, 0);
          ol_it.add_after_then_move(outline);
          C_BLOB* blob = new C_BLOB(&outlines);
          // Although a BLOBNBOX doesn't normally think it owns the blob,
          // these will all end up in a ColPartition, which deletes the
          // C_BLOBs of all its BLOBNBOXes in its destructor to match the
          // fact that the rest get moved to a block later.
          BLOBNBOX* bblob = new BLOBNBOX(blob);
          bblob->set_region_type(rectangle ? BRT_RECTIMAGE : BRT_POLYIMAGE);
          bb_it.add_after_then_move(bblob);
        }
      }
    }
    pixDestroy(&pix);
  }
#endif  // HAVE_LIBLEPT
}

////// Functions involved in making the initial ColPartitions. /////

// Creates the initial ColPartitions, and puts them in a ColPartitionSet
// for each grid y coordinate, storing the ColPartitionSets in part_sets_.
// After creating the ColPartitonSets, attempts to merge them where they
// overlap and unique the BLOBNBOXes within.
// The return value is the number of ColPartitionSets made.
int ColumnFinder::MakeColumnPartitions() {
  part_sets_.reserve(gridheight_);
  for (int grid_y = 0; grid_y < gridheight_; ++grid_y) {
    ColPartitionSet* line_set = PartitionsAtGridY(grid_y);
    part_sets_.push_back(line_set);
  }
  // Now merge neighbouring partitions that overlap significantly.
  int part_set_count = 0;
  for (int i = 0; i < gridheight_; ++i) {
    ColPartitionSet* line_set = part_sets_.get(i);
    if (line_set == NULL)
      continue;
    bool merged_any = false;
    for (int j = i + 1; j < gridheight_; ++j) {
      ColPartitionSet* line_set2 = part_sets_.get(j);
      if (line_set2 == NULL)
        continue;
      if (line_set->MergeOverlaps(line_set2, WidthCB())) {
        merged_any = true;
        if (line_set2->Empty()) {
          delete line_set2;
          part_sets_.set(NULL, j);
        }
        if (line_set->Empty()) {
          delete line_set;
          part_sets_.set(NULL, i);
          merged_any = false;
          break;
        }
      } else {
        break;
      }
    }
    if (merged_any)
      --i;  // Try this one again.
    else
      ++part_set_count;
  }
  return part_set_count;
}

// Partition the BLOBNBOXES horizontally at the given grid y, creating a
// ColPartitionSet which is returned. NULL is returned if there are no
// BLOBNBOXES at the given grid y.
ColPartitionSet* ColumnFinder::PartitionsAtGridY(int grid_y) {
  ColPartition_LIST partitions;
  ColPartition_IT part_it(&partitions);
  // Setup a search of all the grid cells at the given y.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rectsearch(this);
  int y = grid_y * gridsize_ + bleft_.y();
  ICOORD botleft(bleft_.x(), y);
  ICOORD topright(tright_.x(), y + gridsize_ - 1);
  TBOX line_box(botleft, topright);
  rectsearch.StartRectSearch(line_box);
  BLOBNBOX* bbox = rectsearch.NextRectSearch();
  // Each iteration round this while loop finds a set of boxes between
  // tabvectors (or a change of aligned_text type) and places them in
  // a ColPartition.
  int page_edge = line_box.right() + kColumnWidthFactor;
  int prev_margin = line_box.left() - kColumnWidthFactor;
  // Runs of unknown blobs (not certainly text or image) go in a special
  // unk_part, following the same rules as known blobs, but need a
  // separate set of variables to hold the margin/edge information.
  ColPartition_IT unk_part_it(&unknown_parts_);
  ColPartition* unk_partition = NULL;
  TabVector* unk_right_line = NULL;
  int unk_right_margin = page_edge;
  int unk_prev_margin = prev_margin;
  bool unk_edge_is_left = false;
  while (bbox != NULL) {
    TBOX box = bbox->bounding_box();
    if (WithinTestRegion(2, box.left(), box.bottom()))
      tprintf("Starting partition on grid y=%d with box (%d,%d)->(%d,%d)\n",
              grid_y, box.left(), box.bottom(), box.right(), box.top());
    if (box.left() < prev_margin + 1 && textord_debug_bugs) {
      tprintf("Starting box too far left at %d vs %d:",
              box.left(), prev_margin + 1);
      part_it.data()->Print();
    }
    int right_margin = page_edge;
    BlobRegionType start_type = bbox->region_type();
    if (start_type == BRT_NOISE) {
      // Ignore blobs that have been overruled by image blobs.
      // TODO(rays) Possible place to fix inverse text.
      bbox = rectsearch.NextRectSearch();
      continue;
    }
    if (start_type == BRT_UNKNOWN) {
      // Keep unknown blobs in a special partition.
      ProcessUnknownBlob(page_edge, bbox, &unk_partition, &unk_part_it,
                         &unk_right_line, &unk_right_margin,
                         &unk_prev_margin, &unk_edge_is_left);
      bbox = rectsearch.NextRectSearch();
      continue;
    }
    if (unk_partition != NULL)
      unk_prev_margin = CompletePartition(false, page_edge,
                                          unk_right_line, &unk_right_margin,
                                          &unk_partition, &unk_part_it);
    TabVector* right_line = NULL;
    bool edge_is_left = false;
    ColPartition* partition = StartPartition(start_type, prev_margin + 1, bbox,
                                             &right_line, &right_margin,
                                             &edge_is_left);
    // Search for the right edge of this partition.
    while ((bbox = rectsearch.NextRectSearch()) != NULL) {
      TBOX box = bbox->bounding_box();
      int left = box.left();
      int right = box.right();
      int edge = edge_is_left ? left : right;
      BlobRegionType next_type = bbox->region_type();
      if (next_type == BRT_NOISE)
        continue;
      if (next_type == BRT_UNKNOWN) {
        // Keep unknown blobs in a special partition.
        ProcessUnknownBlob(page_edge, bbox, &unk_partition, &unk_part_it,
                           &unk_right_line, &unk_right_margin,
                           &unk_prev_margin, &unk_edge_is_left);
        continue;  // Deal with them later.
      }
      if (unk_partition != NULL)
        unk_prev_margin = CompletePartition(false, page_edge,
                                            unk_right_line, &unk_right_margin,
                                            &unk_partition, &unk_part_it);
      if (ColPartition::TypesMatch(next_type, start_type) &&
          edge < right_margin) {
        // Still within the region and it is still the same type.
        partition->AddBox(bbox);
      } else {
        // This is the first blob in the next set. It gives us the absolute
        // max right coord of the block. (The right margin.)
        right_margin = left - 1;
        if (WithinTestRegion(2, box.left(), box.bottom()))
          tprintf("Box (%d,%d)->(%d,%d) ended partition at %d\n",
                  box.left(), box.bottom(), box.right(), box.top(),
                  right_margin);
        break;
      }
    }
    prev_margin = CompletePartition(bbox == NULL, page_edge,
                                    right_line, &right_margin,
                                    &partition, &part_it);
  }
  if (unk_partition != NULL)
    CompletePartition(true, page_edge, unk_right_line, &unk_right_margin,
                      &unk_partition, &unk_part_it);
  return partitions.empty() ? NULL : new ColPartitionSet(&partitions);
}

// Insert the blobs in the given list into the main grid and for
// each one also make it a separate unknown partition.
// If filter is true, use only the blobs that are above a threshold in
// size or a non-isolated.
void ColumnFinder::InsertSmallBlobsAsUnknowns(bool filter,
                                              BLOBNBOX_LIST* blobs) {
  double noise_blob_size = gridsize() * kMinNonNoiseFraction;
  ColPartition_IT unk_part_it(&unknown_parts_);
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    TBOX box = blob->bounding_box();
    bool good_blob = !filter ||
                     box.width() > noise_blob_size ||
                     box.height() > noise_blob_size;
    if (!good_blob) {
      // Search the vicinity for a bigger blob.
      GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> radsearch(this);
      radsearch.StartRadSearch((box.left() + box.right()) / 2,
                               (box.bottom() + box.top()) / 2,
                               kSmallBlobSearchRadius);
      BLOBNBOX* neighbour;
      while ((neighbour = radsearch.NextRadSearch()) != NULL) {
        TBOX nbox = neighbour->bounding_box();
        // Neighbours must be bigger than the noise size limit to prevent
        // the seed effect of starting with one noise object near a real
        // object, and it then allowing all its neighbours to be accepted.
        if (nbox.height() > noise_blob_size || nbox.width() > noise_blob_size)
          break;
      }
      if (neighbour != NULL)
        good_blob = true;
    }
    if (good_blob) {
      blob_it.extract();
      blob->set_noise_flag(true);
      InsertBlob(true, true, false, blob, this);
      if (WithinTestRegion(2, box.left(), box.bottom()))
        tprintf("Starting small partition with box (%d,%d)->(%d,%d)\n",
                box.left(), box.bottom(), box.right(), box.top());

      int unk_right_margin = tright().x();
      TabVector* unk_right_line = NULL;
      bool unk_edge_is_left = false;
      ColPartition* unk_partition = StartPartition(BRT_TEXT, bleft().x(), blob,
                                                   &unk_right_line,
                                                   &unk_right_margin,
                                                   &unk_edge_is_left);
      CompletePartition(false, tright().x(), unk_right_line,
                        &unk_right_margin, &unk_partition, &unk_part_it);
    }
  }
}

// Helper function for PartitionsAtGridY, with a long argument list.
// This bbox is of unknown type, so it is added to an unk_partition.
// If the edge is past the unk_right_margin then unk_partition has to be
// completed and a new one made. See CompletePartition and StartPartition
// for the other args.
void ColumnFinder::ProcessUnknownBlob(int page_edge,
                                      BLOBNBOX* bbox,
                                      ColPartition** unk_partition,
                                      ColPartition_IT* unk_part_it,
                                      TabVector** unk_right_line,
                                      int* unk_right_margin,
                                      int* unk_prev_margin,
                                      bool* unk_edge_is_left) {
  if (*unk_partition != NULL) {
    const TBOX& box = bbox->bounding_box();
    int edge = *unk_edge_is_left ? box.left() : box.right();
    if (edge >= *unk_right_margin)
      *unk_prev_margin = CompletePartition(false, page_edge,
                                           *unk_right_line, unk_right_margin,
                                           unk_partition, unk_part_it);
  }
  if (*unk_partition == NULL) {
    *unk_partition = StartPartition(BRT_TEXT, *unk_prev_margin + 1, bbox,
                                    unk_right_line,
                                    unk_right_margin,
                                    unk_edge_is_left);
  } else {
    (*unk_partition)->AddBox(bbox);
  }
}

// Creates and returns a new ColPartition of the given start_type
// and adds the given bbox to it.
// Also finds the left and right tabvectors that bound the textline, setting
// the members of the returned ColPartition appropriately:
// If the left tabvector is less constraining than the input left_margin
// (assumed to be the right edge of the previous partition), then the
// tabvector is ignored and the left_margin used instead.
// If the right tabvector is more constraining than the input *right_margin,
// (probably the right edge of the page), then the *right_margin is adjusted
// to use the tabvector.
// *edge_is_left is set to true if the right tabvector is good and used as the
// margin, so we can include blobs that overhang the tabvector in this
// partition.
ColPartition* ColumnFinder::StartPartition(BlobRegionType start_type,
                                           int left_margin, BLOBNBOX* bbox,
                                           TabVector** right_line,
                                           int* right_margin,
                                           bool* edge_is_left) {
  ColPartition* partition = new ColPartition(start_type, vertical_skew_);
  partition->AddBox(bbox);
  // Find the tabs that bound it.
  TBOX box = bbox->bounding_box();
  int mid_y = (box.bottom() + box.top()) / 2;
  TabVector* left_line = LeftTabForBox(box, true, false);
  // If the overlapping line is not a left tab, try for non-overlapping.
  if (left_line != NULL && !left_line->IsLeftTab())
    left_line = LeftTabForBox(box, false, false);
  if (left_line != NULL) {
    int left_x = left_line->XAtY(mid_y);
    left_x += left_line->IsLeftTab() ? -kColumnWidthFactor : 1;
    // If the left line is not a genuine left or is less constraining
    // than the previous blob, then don't store it in the partition.
    if (left_x < left_margin || !left_line->IsLeftTab())
      left_line = NULL;
    if (left_x > left_margin)
      left_margin = left_x;
    if (WithinTestRegion(2, box.left(), box.bottom()))
      tprintf("Left x =%d, left margin = %d\n",
              left_x, left_margin);
  }
  partition->set_left_margin(left_margin);
  *right_line = RightTabForBox(box, true, false);
  // If the overlapping line is not a right tab, try for non-overlapping.
  if (*right_line != NULL && !(*right_line)->IsRightTab())
    *right_line = RightTabForBox(box, false, false);
  *edge_is_left = false;
  if (*right_line != NULL) {
    int right_x = (*right_line)->XAtY(box.bottom());
    if (right_x < *right_margin) {
      *right_margin = right_x;
      if ((*right_line)->IsRightTab())
        *edge_is_left = true;
    }
    if (WithinTestRegion(3, box.left(), box.bottom()))
      tprintf("Right x =%d, right_max = %d\n",
              right_x, *right_margin);
  }
  partition->set_right_margin(*right_margin);
  partition->ComputeLimits();
  partition->SetLeftTab(left_line);
  partition->SetRightTab(*right_line);
  return partition;
}

// Completes the given partition, and adds it to the given iterator.
// The right_margin on input is the left edge of the next blob if there is
// one. The right tab vector plus a margin is used as the right margin if
// it is more constraining than the next blob, but if there are no more
// blobs, we want the right margin to make it to the page edge.
// The return value is the next left margin, being the right edge of the
// bounding box of blobs.
int ColumnFinder::CompletePartition(bool no_more_blobs,
                                    int page_edge,
                                    TabVector* right_line,
                                    int* right_margin,
                                    ColPartition** partition,
                                    ColPartition_IT* part_it) {
  ASSERT_HOST(partition !=NULL && *partition != NULL);
  // If we have a right line, it is possible that its edge is more
  // constraining than the next blob.
  if (right_line != NULL && right_line->IsRightTab()) {
    int mid_y = (*partition)->MidY();
    int right_x = right_line->XAtY(mid_y) + kColumnWidthFactor;
    if (right_x < *right_margin)
      *right_margin = right_x;
    else if (no_more_blobs)
      *right_margin = MAX(right_x, page_edge);
    else if (right_line->XAtY(mid_y) > *right_margin)
      right_line = NULL;
  } else {
    right_line = NULL;
  }
  // Now we can complete the partition and add it to the list.
  (*partition)->set_right_margin(*right_margin);
  (*partition)->ComputeLimits();
  (*partition)->SetRightTab(right_line);
  (*partition)->SetColumnGoodness(WidthCB());
  part_it->add_after_then_move(*partition);
  // Setup ready to start the next one.
  *right_margin = page_edge;
  int next_left_margin = (*partition)->bounding_box().right();
  *partition = NULL;
  return next_left_margin;
}

// Makes an ordered list of candidates to partition the width of the page
// into columns using the part_sets_.
// See AddToColumnSetsIfUnique for the ordering.
// If single_column, then it just makes a single page-wide fake column.
void ColumnFinder::MakeColumnCandidates(bool single_column) {
  if (!single_column) {
    // Try using only the good parts first.
    bool good_only = true;
    do {
      for (int i = 0; i < gridheight_; ++i) {
        ColPartitionSet* line_set = part_sets_.get(i);
        if (line_set != NULL && line_set->LegalColumnCandidate()) {
          ColPartitionSet* column_candidate = line_set->Copy(good_only);
          if (column_candidate != NULL)
            column_candidate->AddToColumnSetsIfUnique(&column_sets_, WidthCB());
        }
      }
      good_only = !good_only;
    } while (column_sets_.empty() && !good_only);
  }
  if (column_sets_.empty()) {
    // The page contains only image or is single column.
    // Make a fake page-wide column.
    ColPartition* fake_part = new ColPartition(BRT_TEXT, vertical_skew_);
    fake_part->set_left_margin(bleft_.x());
    fake_part->set_right_margin(tright_.x());
    fake_part->ComputeLimits();
    fake_part->SetColumnGoodness(WidthCB());
    ColPartitionSet* column_candidate = new ColPartitionSet(fake_part);
    column_candidate->AddToColumnSetsIfUnique(&column_sets_, WidthCB());
  }
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
      ASSERT_HOST(column_candidate != NULL);
      ColPartitionSet* improved = column_candidate->Copy(good_only);
      if (improved != NULL) {
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
void ColumnFinder::AssignColumns() {
  int set_count = part_sets_.size();
  ASSERT_HOST(set_count == gridheight());
  // Allocate and init the best_columns_.
  best_columns_ = new ColPartitionSet*[set_count];
  for (int y = 0; y < set_count; ++y)
    best_columns_[y] = NULL;
  int column_count = column_sets_.size();
  // possible_column_sets[part_sets_ index][column_sets_ index] is
  // true if the partition set is compatible with the column set.
  // assigned_column_sets[part_sets_ index][column_sets_ index] is true
  // if the partition set has been assigned the column. (Multiple bits
  // true is possible.)
  // any_columns_possible[part_sets_ index] is true if any of
  // possible_column_sets[part_sets_ index][*] is true.
  // On return the best_columns_ member is set.
  bool* any_columns_possible = new bool[set_count];
  bool** possible_column_sets = new bool*[set_count];
  bool** assigned_column_sets = new bool*[set_count];
  // Set possible column_sets to indicate whether each set is compatible
  // with each column.
  for (int part_i = 0; part_i < set_count; ++part_i) {
    ColPartitionSet* line_set = part_sets_.get(part_i);
    bool debug = line_set != NULL &&
                 WithinTestRegion(2, line_set->bounding_box().left(),
                                  line_set->bounding_box().bottom());
    possible_column_sets[part_i] = new bool[column_count];
    assigned_column_sets[part_i] = new bool[column_count];
    any_columns_possible[part_i] = false;
    for (int col_i = 0; col_i < column_count; ++col_i) {
      assigned_column_sets[part_i][col_i] = false;
      if (line_set != NULL &&
          column_sets_.get(col_i)->CompatibleColumns(debug, line_set,
                                                     WidthCB())) {
        possible_column_sets[part_i][col_i] = true;
        any_columns_possible[part_i] = true;
      } else {
        possible_column_sets[part_i][col_i] = false;
      }
    }
  }
  // Assign a column set to each vertical grid position.
  // While there is an unassigned range, find its mode.
  int start, end;
  while (BiggestUnassignedRange(any_columns_possible, &start, &end)) {
    if (textord_debug_tabfind >= 2)
      tprintf("Biggest unassigned range = %d- %d\n", start, end);
    // Find the modal column_set_id in the range.
    int column_set_id = RangeModalColumnSet(possible_column_sets, start, end);
    if (textord_debug_tabfind >= 2) {
      tprintf("Range modal column id = %d\n", column_set_id);
      column_sets_.get(column_set_id)->Print();
    }
    // Now find the longest run of the column_set_id in the range.
    ShrinkRangeToLongestRun(possible_column_sets, any_columns_possible,
                            column_set_id, &start, &end);
    if (textord_debug_tabfind >= 2)
      tprintf("Shrunk range = %d- %d\n", start, end);
    // Extend the start and end past the longest run, while there are
    // only small gaps in compatibility that can be overcome by larger
    // regions of compatibility beyond.
    ExtendRangePastSmallGaps(possible_column_sets, any_columns_possible,
                             column_set_id, -1, -1, &start);
    --end;
    ExtendRangePastSmallGaps(possible_column_sets, any_columns_possible,
                             column_set_id, 1, set_count, &end);
    ++end;
    if (textord_debug_tabfind)
      tprintf("Column id %d applies to range = %d - %d\n",
              column_set_id, start, end);
    // Assign the column to the range, which now may overlap with other ranges.
    AssignColumnToRange(column_set_id, start, end,
                        assigned_column_sets);
  }
  // If anything remains unassigned, the whole lot is unassigned, so
  // arbitrarily assign id 0.
  if (best_columns_[0] == NULL) {
    AssignColumnToRange(0, 0, gridheight_, assigned_column_sets);
  }
  // Free memory.
  for (int i = 0; i < set_count; ++i) {
    delete [] possible_column_sets[i];
    delete [] assigned_column_sets[i];
  }
  delete [] any_columns_possible;
  delete [] possible_column_sets;
  delete [] assigned_column_sets;
  // TODO(rays) Now resolve overlapping assignments.
}

// Finds the biggest range in part_sets_ that has no assigned column, but
// column assignment is possible.
bool ColumnFinder::BiggestUnassignedRange(const bool* any_columns_possible,
                                          int* best_start, int* best_end) {
  int set_count = part_sets_.size();
  int best_range_size = 0;
  *best_start = set_count;
  *best_end = set_count;
  int end = set_count;
  for (int start = 0; start < gridheight_; start = end) {
    // Find the first unassigned index in start.
    while (start < set_count) {
      if (best_columns_[start] == NULL && any_columns_possible[start])
        break;
      ++start;
    }
    // Find the first past the end and count the good ones in between.
    int range_size = 1;  // Number of non-null, but unassigned line sets.
    end = start + 1;
    while (end < set_count) {
      if (best_columns_[end] != NULL)
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
int ColumnFinder::RangeModalColumnSet(bool** possible_column_sets,
                                      int start, int end) {
  int column_count = column_sets_.size();
  STATS column_stats(0, column_count);
  for (int part_i = start; part_i < end; ++part_i) {
    for (int col_j = 0; col_j < column_count; ++col_j) {
      if (possible_column_sets[part_i][col_j])
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
void ColumnFinder::ShrinkRangeToLongestRun(bool** possible_column_sets,
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
      if (possible_column_sets[start][column_set_id] ||
          !any_columns_possible[start])
        break;
      ++start;
    }
    // Find the first past the end.
    end = start + 1;
    while (end < orig_end) {
      if (!possible_column_sets[end][column_set_id] &&
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

// Moves start in the direction of step, upto, but not including end while
// the only incompatible regions are no more than kMaxIncompatibleColumnCount
// in size, and the compatible regions beyond are bigger.
void ColumnFinder::ExtendRangePastSmallGaps(bool** possible_column_sets,
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
      if (possible_column_sets[i][column_set_id])
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
      if (possible_column_sets[i][column_set_id])
        ++good_size;
      else if (any_columns_possible[i])
        break;
    }
    if (textord_debug_tabfind > 2)
      tprintf("At %d, good size = %d\n", i, good_size);
    // If we had enough good ones we can extend the start and keep looking.
    if (good_size > barrier_size)
      *start = i - step;
  } while (good_size > barrier_size);
}

// Assigns the given column_set_id to the given range.
void ColumnFinder::AssignColumnToRange(int column_set_id, int start, int end,
                                       bool** assigned_column_sets) {
  ColPartitionSet* column_set = column_sets_.get(column_set_id);
  for (int i = start; i < end; ++i) {
    assigned_column_sets[i][column_set_id] = true;
    best_columns_[i] = column_set;
  }
}

// Computes the mean_column_gap_.
void ColumnFinder::ComputeMeanColumnGap() {
  int total_gap = 0;
  int total_width = 0;
  int gap_samples = 0;
  int width_samples = 0;
  for (int i = 0; i < gridheight_; ++i) {
    ASSERT_HOST(best_columns_[i] != NULL);
    best_columns_[i]->AccumulateColumnWidthsAndGaps(&total_width,
                                                    &width_samples,
                                                    &total_gap,
                                                    &gap_samples);
  }
  mean_column_gap_ = gap_samples > 0 ? total_gap / gap_samples
                                     : total_width / width_samples;
}

//////// Functions that manipulate ColPartitions in the part_grid_ /////
//////// to split, merge, find margins, and find types.  //////////////

// Removes the ColPartitions from part_sets_, the ColPartitionSets that
// contain them, and puts them in the part_grid_ after ensuring that no
// BLOBNBOX is owned by more than one of them.
void ColumnFinder::MovePartitionsToGrid() {
  // Remove the parts from the part_sets_ and put them in the parts list.
  part_grid_.Init(gridsize(), bleft(), tright());
  ColPartition_LIST parts;
  for (int i = 0; i < gridheight_; ++i) {
    ColPartitionSet* line_set = part_sets_.get(i);
    if (line_set != NULL) {
      line_set->ReturnParts(&parts);
      delete line_set;
      part_sets_.set(NULL, i);
    }
  }
  // Make each part claim ownership of its own boxes uniquely.
  ColPartition_IT it(&parts);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* part = it.data();
    part->ClaimBoxes(WidthCB());
  }
  // Unknowns must be uniqued too.
  it.set_to_list(&unknown_parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* part = it.data();
    part->ClaimBoxes(WidthCB());
  }
  // Put non-empty parts into the grid and delete empty ones.
  for (it.set_to_list(&parts); !it.empty(); it.forward()) {
    ColPartition* part = it.extract();
    if (part->IsEmpty())
      delete part;
    else
      part_grid_.InsertBBox(true, true, part);
  }
}

// Splits partitions that cross columns where they have nothing in the gap.
void ColumnFinder::GridSplitPartitions() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* dont_repeat = NULL;
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->blob_type() < BRT_UNKNOWN || part == dont_repeat)
      continue;  // Only applies to text partitions.
    ColPartitionSet* column_set = best_columns_[gsearch.GridY()];
    int first_col = -1;
    int last_col = -1;
    // Find which columns the partition spans.
    part->ColumnRange(column_set, &first_col, &last_col);
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
    if (textord_debug_tabfind) {
      tprintf("Considering partition for GridSplit:");
      part->Print();
    }
    // Set up a rectangle search x-bounded by the column gap and y by the part.
    int y = part->MidY();
    TBOX margin_box = part->bounding_box();
    ColPartition* column = column_set->GetColumnByIndex(first_col);
    if (column == NULL)
      continue;
    margin_box.set_left(column->RightAtY(y) + 2);
    column = column_set->GetColumnByIndex(last_col);
    if (column == NULL)
      continue;
    margin_box.set_right(column->LeftAtY(y) - 2);
    // TODO(rays) Decide whether to keep rectangular filling or not in the
    // main grid and therefore whether we need a fancier search here.
    // Now run the rect search on the main blob grid.
    GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rectsearch(this);
    if (textord_debug_tabfind) {
      tprintf("Searching box (%d,%d)->(%d,%d)\n",
              margin_box.left(), margin_box.bottom(),
              margin_box.right(), margin_box.top());
      part->Print();
    }
    rectsearch.StartRectSearch(margin_box);
    BLOBNBOX* bbox;
    while ((bbox = rectsearch.NextRectSearch()) != NULL) {
      if (bbox->bounding_box().overlap(margin_box))
        break;
    }
    if (bbox == NULL) {
      // There seems to be nothing in the hole, so split the partition.
      gsearch.RemoveBBox();
      int x_middle = (margin_box.left() + margin_box.right()) / 2;
      if (textord_debug_tabfind) {
        tprintf("Splitting part at %d:", x_middle);
        part->Print();
      }
      ColPartition* split_part = part->SplitAt(x_middle);
      if (split_part != NULL) {
        if (textord_debug_tabfind) {
          tprintf("Split result:");
          part->Print();
          split_part->Print();
        }
        part_grid_.InsertBBox(true, true, split_part);
      } else {
        // Split had no effect
        if (textord_debug_tabfind)
          tprintf("Split had no effect\n");
        dont_repeat = part;
      }
      part_grid_.InsertBBox(true, true, part);
      gsearch.RepositionIterator();
    } else if (textord_debug_tabfind) {
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
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet* columns = best_columns_[gsearch.GridY()];
    TBOX box = part->bounding_box();
    int y = part->MidY();
    ColPartition* left_column = columns->ColumnContaining(box.left(), y);
    ColPartition* right_column = columns->ColumnContaining(box.right(), y);
    if (left_column == NULL || right_column != left_column)
      continue;
    box.set_left(left_column->LeftAtY(y));
    box.set_right(right_column->RightAtY(y));
    // Now run the rect search.
    bool modified_box = false;
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rsearch(&part_grid_);
    rsearch.StartRectSearch(box);
    ColPartition* neighbour;

    while ((neighbour = rsearch.NextRectSearch()) != NULL) {
      if (neighbour == part)
        continue;
      const TBOX& neighbour_box = neighbour->bounding_box();
      if (neighbour_box.right() < box.left() ||
          neighbour_box.left() > box.right())
        continue;  // Not within the same column.
      if (part->VOverlaps(*neighbour) && part->TypesMatch(*neighbour)) {
        // There is vertical overlap and the gross types match, but only
        // merge if the horizontal gap is small enough, as one of the
        // partitions may be a figure caption within a column.
        // If there is only one column, then the mean_column_gap_ is large
        // enough to allow almost any merge, by being the mean column width.
        const TBOX& part_box = part->bounding_box();
        int h_gap = MAX(part_box.left(), neighbour_box.left()) -
                    MIN(part_box.right(), neighbour_box.right());
        if (h_gap < mean_column_gap_ * kHorizontalGapMergeFraction ||
            part_box.width() < mean_column_gap_ ||
            neighbour_box.width() < mean_column_gap_) {
          if (textord_debug_tabfind) {
            tprintf("Running grid-based merge between:\n");
            part->Print();
            neighbour->Print();
          }
          rsearch.RemoveBBox();
          gsearch.RepositionIterator();
          part->Absorb(neighbour, WidthCB());
          modified_box = true;
        }
      }
    }
    if (modified_box) {
      // We modified the box of part, so re-insert it into the grid.
      // This does no harm in the current cell, as it already exists there,
      // but it needs to exist in all the cells covered by its bounding box,
      // or it will never be found by a full search.
      // Because the box has changed, it has to be removed first, otherwise
      // add_sorted may fail to keep a single copy of the pointer.
      gsearch.RemoveBBox();
      part_grid_.InsertBBox(true, true, part);
      gsearch.RepositionIterator();
    }
  }
}

// Helper function to compute the total pairwise area overlap of a list of
// Colpartitions. If box_this matches an element in the list, the test_box
// is used in place of its box.
static int TotalPairwiseOverlap(const ColPartition* box_this,
                                const TBOX& test_box,
                                ColPartition_CLIST* parts) {
  if (parts->singleton())
    return 0;
  int total_area = 0;
  for (ColPartition_C_IT it(parts); !it.at_last(); it.forward()) {
    ColPartition* part = it.data();
    TBOX part_box = part == box_this ? test_box : part->bounding_box();
    ColPartition_C_IT it2(it);
    for (it2.forward(); !it2.at_first(); it2.forward()) {
      ColPartition* part2 = it2.data();
      TBOX part_box2 = part2 == box_this ? test_box : part2->bounding_box();
      total_area += part_box.intersection(part_box2).area();
    }
  }
  return total_area;
}

// Helper function to compute the total area of a list of Colpartitions.
// If box_this matches an element in the list, the test_box
// is used in place of its box.
static int TotalArea(const ColPartition* box_this, const TBOX& test_box,
                     ColPartition_CLIST* parts) {
  int total_area = 0;
  ColPartition_C_IT it(parts);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* part = it.data();
    TBOX part_box = part == box_this ? test_box : part->bounding_box();
    total_area += part_box.area();
  }
  return total_area;
}

// Resolves unknown partitions from the unknown_parts_ list by merging them
// with a close neighbour, inserting them into the grid with a known type,
// or declaring them to be noise.
void ColumnFinder::GridInsertUnknowns() {
  ColPartition_IT noise_it(&noise_parts_);
  for (ColPartition_IT it(&unknown_parts_); !it.empty(); it.forward()) {
    ColPartition* part = it.extract();
    if (part->IsEmpty()) {
      // Claiming ownership left this one empty.
      delete part;
      continue;
    }
    const TBOX& part_box = part->bounding_box();
    int left_limit = LeftEdgeForBox(part_box, false, false);
    int right_limit = RightEdgeForBox(part_box, false, false);
    TBOX search_box = part_box;
    int y = part->MidY();
    int grid_x, grid_y;
    GridCoords(part_box.left(), y, &grid_x, &grid_y);
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet* columns = best_columns_[grid_y];
    int first_col = -1;
    int last_col = -1;
    // Find which columns the partition spans.
    part->ColumnRange(columns, &first_col, &last_col);
    // Convert output column indices to physical column indices.
    // Twiddle with first and last_col to get the desired effect with
    // in-between columns:
    // As returned by ColumnRange, the indices are even for in-betweens and
    // odd for real columns (being 2n+1 the real column index).
    // Subtract 1 from first col, so we can use left edge of first_col/2 if it
    // is even, and the right edge of first_col/2 if it is odd.
    // With last_col unchanged, we can use the right edge of last_col/2 if it
    // is odd and the left edge of last_col/2 if it is even.
    // with first_col, we have to special-case to pretend that the first
    // in-between is actually the first column, and with last_col, we have to
    // pretend that the last in-between is actually the last column.
    if (first_col > 0)
      --first_col;
    ColPartition* first_column = columns->GetColumnByIndex(first_col / 2);
    ColPartition* last_column = columns->GetColumnByIndex(last_col / 2);
    if (last_column == NULL && last_col > first_col + 1)
      last_column = columns->GetColumnByIndex(last_col / 2 - 1);
    // Do not accept the case of both being in the first or last in-between.
    if (last_col > 0 && first_column != NULL && last_column != NULL) {
      search_box.set_left((first_col & 1) ? first_column->RightAtY(y)
                                          : first_column->LeftAtY(y));
      search_box.set_right((last_col & 1) ? last_column->RightAtY(y)
                                          : last_column->LeftAtY(y));
      // Expand the search vertically.
      int height = search_box.height();
      search_box.set_top(search_box.top() + height);
      search_box.set_bottom(search_box.bottom() - height);
      // Keep valid neighbours in a list.
      ColPartition_CLIST neighbours;
      // Now run the rect search.
      GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rsearch(&part_grid_);
      rsearch.StartRectSearch(search_box);
      ColPartition* neighbour;
      while ((neighbour = rsearch.NextRectSearch()) != NULL) {
        const TBOX& n_box = neighbour->bounding_box();
        if (n_box.left() > right_limit || n_box.right() < left_limit)
          continue;  // Other side of a tab vector.
        if (neighbour->blob_type() == BRT_RECTIMAGE) {
          continue;  // Rectangle images aren't allowed to acquire anything.
        }
        // We can't merge with a partition where it would go beyond the margin
        // of the partition.
        if ((part_box.left() < neighbour->left_margin() ||
            part_box.right() > neighbour->right_margin()) &&
            !n_box.contains(part_box)) {
          continue;  // This would create an overlap with another partition.
        }
        // Candidates must be within a reasonable vertical distance.
        int v_dist = -part->VOverlap(*neighbour);
        if (v_dist >= MAX(part_box.height(), n_box.height()) / 2)
          continue;
        // Unique elements as they arrive.
        neighbours.add_sorted(SortByBoxLeft<ColPartition>, true, neighbour);
      }
      // The best neighbour to merge with is the one that causes least
      // total pairwise overlap among all the candidates.
      // If more than one offers the same total overlap, choose the one
      // with the least total area.
      ColPartition* best_neighbour = NULL;
      ColPartition_C_IT n_it(&neighbours);
      if (neighbours.singleton()) {
        best_neighbour = n_it.data();
      } else if (!neighbours.empty()) {
        int best_overlap = MAX_INT32;
        int best_area = 0;
        for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
          neighbour = n_it.data();
          TBOX merged_box = neighbour->bounding_box();
          merged_box += part_box;
          int overlap = TotalPairwiseOverlap(neighbour, merged_box,
                                             &neighbours);
          if (best_neighbour == NULL || overlap < best_overlap) {
            best_neighbour = neighbour;
            best_overlap = overlap;
            best_area = TotalArea(neighbour, merged_box, &neighbours);
          } else if (overlap == best_overlap) {
            int area = TotalArea(neighbour, merged_box, &neighbours);
            if (area < best_area) {
              best_area = area;
              best_neighbour = neighbour;
            }
          }
        }
      }
      if (best_neighbour != NULL) {
        // It was close enough to an existing partition to merge it.
        if (textord_debug_tabfind) {
          tprintf("Merging unknown partition:\n");
          part->Print();
          best_neighbour->Print();
        }
        // Because the box is about to be resized, it must be removed and
        // then re-inserted to prevent duplicates in the grid lists.
        part_grid_.RemoveBBox(best_neighbour);
        best_neighbour->Absorb(part, WidthCB());
        // We modified the box of best_neighbour, so re-insert it into the grid.
        part_grid_.InsertBBox(true, true, best_neighbour);
      } else {
        // It was inside a column, so just add it to the grid.
        if (textord_debug_tabfind)
          tprintf("Inserting unknown partition:\n");
        part_grid_.InsertBBox(true, true, part);
      }
    } else {
      if (textord_debug_tabfind) {
        tprintf("Unknown partition at (%d,%d)->(%d,%d) not in any column\n",
                part_box.left(), part_box.bottom(), part_box.right(),
                part_box.top());
        tprintf("first_col = %d->%p, last_col=%d->%p\n",
                first_col, first_column, last_col, last_column);
      }
      noise_it.add_to_end(part);
    }
  }
}

// Add horizontal line separators as partitions.
void ColumnFinder::GridInsertHLinePartitions() {
  TabVector_IT hline_it(&horizontal_lines_);
  for (hline_it.mark_cycle_pt(); !hline_it.cycled_list(); hline_it.forward()) {
    TabVector* hline = hline_it.data();
    int top = MAX(hline->startpt().y(), hline->endpt().y());
    int bottom = MIN(hline->startpt().y(), hline->endpt().y());
    if (top == bottom) {
      if (bottom > 0)
        --bottom;
      else
        ++top;
    }
    ColPartition* part = new ColPartition(vertical_skew_,
                                          hline->startpt().x(), bottom,
                                          hline->endpt().x(), top);
    part_grid_.InsertBBox(true, true, part);
  }
}

// Improves the margins of the ColPartitions in the grid by calling
// FindPartitionMargins on each.
void ColumnFinder::GridFindMargins() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet* columns = best_columns_[gsearch.GridY()];
    FindPartitionMargins(columns, part);
  }
}

// Improves the margins of the ColPartitions in the list by calling
// FindPartitionMargins on each.
void ColumnFinder::ListFindMargins(ColPartition_LIST* parts) {
  ColPartition_IT part_it(parts);
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition* part = part_it.data();
    TBOX part_box = part->bounding_box();
    // Get the columns from the y grid coord.
    int grid_x, grid_y;
    GridCoords(part_box.left(), part_box.bottom(), &grid_x, &grid_y);
    ColPartitionSet* columns = best_columns_[grid_y];
    FindPartitionMargins(columns, part);
  }
}

// Improves the margins of the ColPartition by searching for
// neighbours that vertically overlap significantly.
void ColumnFinder::FindPartitionMargins(ColPartitionSet* columns,
                                        ColPartition* part) {
  // Set up a rectangle search x-bounded by the column and y by the part.
  ASSERT_HOST(columns != NULL);
  TBOX box = part->bounding_box();
  int y = part->MidY();
  // Initial left margin is based on the column, if there is one.
  ColPartition* column = columns->ColumnContaining(box.left(), y);
  int left_margin = column != NULL ? column->LeftAtY(y) : bleft_.x();
  left_margin -= kColumnWidthFactor;
  // Search for ColPartitions that reduce the margin.
  left_margin = FindMargin(box.left()+ box.height(), true, left_margin,
                           box.bottom(), box.top(), part);
  part->set_left_margin(left_margin);
  column = columns->ColumnContaining(box.right(), y);
  int right_margin = column != NULL ? column->RightAtY(y) : tright_.x();
  right_margin += kColumnWidthFactor;
  // Search for ColPartitions that reduce the margin.
  right_margin = FindMargin(box.right() - box.height(), false, right_margin,
                            box.bottom(), box.top(), part);
  part->set_right_margin(right_margin);
}

// Starting at x, and going in the specified direction, upto x_limit, finds
// the margin for the given y range by searching sideways,
// and ignoring not_this.
int ColumnFinder::FindMargin(int x, bool right_to_left, int x_limit,
                             int y_bottom, int y_top,
                             const ColPartition* not_this) {
  int height = y_top - y_bottom;
  int target_overlap = static_cast<int>(height * kMarginOverlapFraction);
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    side_search(&part_grid_);
  side_search.StartSideSearch(x, y_bottom, y_top);
  ColPartition* part;
  while ((part = side_search.NextSideSearch(right_to_left)) != NULL) {
    // Ignore itself.
    if (part == not_this)
      continue;
    // Must overlap by enough.
    TBOX box = part->bounding_box();
    int y_overlap = MIN(y_top, box.top()) - MAX(y_bottom, box.bottom());
    if (y_overlap < target_overlap)
      continue;
    // Must be going the right way.
    int x_edge = right_to_left ? box.right() : box.left();
    if ((x_edge < x) != right_to_left)
      continue;
    // If we have gone past x_limit, then x_limit will do.
    if ((x_edge < x_limit) == right_to_left)
      break;
    // It reduces x limit, so save the new one.
    x_limit = x_edge;
  }
  return x_limit;
}

// For every ColPartition in the grid, sets its type based on position
// in the columns.
void ColumnFinder::SetPartitionTypes() {
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    part->SetPartitionType(best_columns_[gsearch.GridY()]);
  }
}

//////// Functions that manipulate ColPartitions in the part_grid_ /////
//////// to find chains of partner partitions of the same type.  ///////

// For every ColPartition in the grid, finds its upper and lower neighbours.
void ColumnFinder::FindPartitionPartners() {
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    FindPartitionPartners(true, part);
    FindPartitionPartners(false, part);
  }
}

// Finds the best partner in the given direction for the given partition.
// Stores the result with AddPartner.
void ColumnFinder::FindPartitionPartners(bool upper, ColPartition* part) {
  if (part->type() == PT_NOISE)
    return;  // Noise is not allowed to partner anything.
  const TBOX& box = part->bounding_box();
  int top = part->median_top();
  int bottom = part->median_bottom();
  int height = top - bottom;
  int mid_y = (bottom + top) / 2;
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    vsearch(&part_grid_);
  // Search down for neighbour below
  vsearch.StartVerticalSearch(box.left(), box.right(), part->MidY());
  ColPartition* neighbour;
  ColPartition* best_neighbour = NULL;
  int best_dist = MAX_INT32;
  while ((neighbour = vsearch.NextVerticalSearch(!upper)) != NULL) {
    if (neighbour == part || neighbour->type() == PT_NOISE)
      continue;  // Noise is not allowed to partner anything.
    int neighbour_bottom = neighbour->median_bottom();
    int neighbour_top = neighbour->median_top();
    int neighbour_y = (neighbour_bottom + neighbour_top) / 2;
    if (upper != (neighbour_y > mid_y))
      continue;
    if (!part->HOverlaps(*neighbour) && !part->HCompatible(*neighbour))
      continue;
    if (!part->TypesMatch(*neighbour)) {
      if (best_neighbour == NULL)
        best_neighbour = neighbour;
      continue;
    }
    int dist = upper ? neighbour_bottom - top : bottom - neighbour_top;
    if (dist <= kMaxPartitionSpacing * height) {
      if (dist < best_dist) {
        best_dist = dist;
        best_neighbour = neighbour;
      }
    } else {
      break;
    }
  }
  if (best_neighbour != NULL)
    part->AddPartner(upper, best_neighbour);
}

// For every ColPartition with multiple partners in the grid, reduces the
// number of partners to 0 or 1.
void ColumnFinder::RefinePartitionPartners() {
  // Refine in type order so that chasing multple partners can be done
  // before eliminating type mis-matching partners.
  for (int type = PT_UNKNOWN + 1; type <= PT_COUNT; type++) {
    // Iterate the ColPartitions in the grid.
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      gsearch(&part_grid_);
    gsearch.StartFullSearch();
    ColPartition* part;
    while ((part = gsearch.NextFullSearch()) != NULL) {
      part->RefinePartners(static_cast<PolyBlockType>(type));
    }
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
  while ((part = gsearch.NextFullSearch()) != NULL) {
    ColPartition* partner = part->SingletonPartner(true);
    if (partner != NULL) {
      ASSERT_HOST(partner->SingletonPartner(false) == part);
    } else if (part->SingletonPartner(false) != NULL) {
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
    if (!part->HOverlaps(*test_part) && !part->HCompatible(*test_part))
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
  ColPartitionSet* column_set = NULL;
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
  while ((part = gsearch.NextFullSearch()) != NULL) {
    int grid_y = gsearch.GridY();
    if (grid_y != prev_grid_y) {
      EmptyTempPartList(&temp_part_list, &work_set);
      prev_grid_y = grid_y;
    }
    if (best_columns_[grid_y] != column_set) {
      column_set = best_columns_[grid_y];
      // Every line should have a non-null best column.
      ASSERT_HOST(column_set != NULL);
      column_set->ChangeWorkColumns(bleft_, tright_, resolution_,
                                    &good_parts_, &work_set);
      if (textord_debug_tabfind)
        tprintf("Changed column groups at grid index %d\n", gsearch.GridY());
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

// Reskew the completed blocks to put them back to the original coords.
// (Blob outlines are not corrected for skew.)
// Rotate blobs and blocks individually so text line direction is
// horizontal. Record appropriate inverse transformations and required
// classifier transformation in the blocks.
void ColumnFinder::RotateAndReskewBlocks(TO_BLOCK_LIST* blocks) {
  TO_BLOCK_IT it(blocks);
  int block_index = 1;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TO_BLOCK* to_block = it.data();
    BLOCK* block = to_block->block;
    block->set_index(block_index++);
    BLOBNBOX_IT blob_it(&to_block->blobs);
    // ColPartition::MakeBlock stored the inverse rotation that must be
    // applied to small vertical blocks to go back to the original image
    // coords at the end of recognition, but did not actually do any rotations,
    // so now blocks must actually be rotated to make them horizontal by the
    // inverse of that stored inverse rotation. This is of course a no-op
    // for normal blocks.
    FCOORD block_rotation = block->re_rotation();
    block_rotation.set_y(-block_rotation.y());
    block->poly_block()->rotate(block_rotation);
    // The final stored inverse coordinate rotation (block->re_rotation_)
    // is the sum of rerotate_ (for gross vertical pages) and the current
    // block->re_rotation_ (for small vertical text regions).
    // We will execute the inverse of that on all the blobs.
    FCOORD blob_rotation = block->re_rotation();
    blob_rotation.rotate(rerotate_);
    block->set_re_rotation(blob_rotation);
    blob_rotation.set_y(-blob_rotation.y());
    // TODO(rays) determine classify rotation by orientation detection.
    // In the mean time, it works for Chinese and English photo credits
    // to set a classify rotation to the stored block rerotation only if
    // the block rotation to do (before skew) is 0.
    if (block_rotation.y() == 0.0f) {
      block->set_classify_rotation(block->re_rotation());
    }
    // Blocks must also be rotated back by the skew angle.
    block->rotate(reskew_);
    // Save the skew in the block.
    block->set_skew(reskew_);
    // Rotate all the blobs if needed and recompute the bounding boxes.
    // Compute the block median blob width and height as we go.
    STATS widths(0, block->bounding_box().width());
    STATS heights(0, block->bounding_box().height());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      BLOBNBOX* blob = blob_it.data();
      if (blob_rotation.y() != 0.0f) {
        blob->cblob()->rotate(blob_rotation);
      }
      blob->compute_bounding_box();
      widths.add(blob->bounding_box().width(), 1);
      heights.add(blob->bounding_box().height(), 1);
    }
    block->set_median_size(static_cast<int>(widths.median() + 0.5),
                           static_cast<int>(heights.median() + 0.5));
    if (textord_debug_tabfind > 1)
      tprintf("Block median size = (%d, %d)\n",
              block->median_size().x(), block->median_size().y());
  }
}

// TransformToBlocks leaves all the small and noise blobs untouched in the
// source TO_BLOCK. MoveSmallBlobs moves them into the main blobs list of
// the block from the to_blocks list that contains them.
// TODO(rays) This is inefficient with a large number of blocks. A more
// efficient implementation is possible using a BBGrid.
void ColumnFinder::MoveSmallBlobs(BLOBNBOX_LIST* bblobs,
                                  TO_BLOCK_LIST* to_blocks) {
  for (BLOBNBOX_IT bb_it(bblobs); !bb_it.empty(); bb_it.forward()) {
    BLOBNBOX* bblob = bb_it.extract();
    const TBOX& bbox = bblob->bounding_box();
    // Find the centre of the blob.
    ICOORD centre = bbox.botleft();
    centre += bbox.topright();
    centre /= 2;
    // Find the TO_BLOCK that contains the centre and put the blob in
    // its main blobs list.
    TO_BLOCK_IT to_it(to_blocks);
    for (to_it.mark_cycle_pt(); !to_it.cycled_list(); to_it.forward()) {
      TO_BLOCK* to_block = to_it.data();
      BLOCK* block = to_block->block;
      if (block->contains(centre)) {
        BLOBNBOX_IT blob_it(&to_block->blobs);
        blob_it.add_to_end(bblob);
        bblob = NULL;
        break;
      }
    }
    if (bblob != NULL) {
      delete bblob->cblob();
      delete bblob;
    }
  }
}

}  // namespace tesseract.
