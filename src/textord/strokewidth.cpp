///////////////////////////////////////////////////////////////////////
// File:        strokewidth.cpp
// Description: Subclass of BBGrid to find uniformity of strokewidth.
// Author:      Ray Smith
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "strokewidth.h"

#include <algorithm>
#include <cmath>

#include "blobbox.h"
#include "colpartition.h"
#include "colpartitiongrid.h"
#include "helpers.h" // for IntCastRounded
#include "imagefind.h"
#include "linlsq.h"
#include "statistc.h"
#include "tabfind.h"
#include "textlineprojection.h"
#include "tordmain.h" // For SetBlobStrokeWidth.

namespace tesseract {

#ifndef GRAPHICS_DISABLED
static INT_VAR(textord_tabfind_show_strokewidths, 0, "Show stroke widths (ScrollView)");
#else
static INT_VAR(textord_tabfind_show_strokewidths, 0, "Show stroke widths");
#endif
static BOOL_VAR(textord_tabfind_only_strokewidths, false, "Only run stroke widths");

/** Allowed proportional change in stroke width to be the same font. */
const double kStrokeWidthFractionTolerance = 0.125;
/**
 * Allowed constant change in stroke width to be the same font.
 * Really 1.5 pixels.
 */
const double kStrokeWidthTolerance = 1.5;
// Same but for CJK we are a bit more generous.
const double kStrokeWidthFractionCJK = 0.25;
const double kStrokeWidthCJK = 2.0;
// Radius in grid cells of search for broken CJK. Doesn't need to be very
// large as the grid size should be about the size of a character anyway.
const int kCJKRadius = 2;
// Max distance fraction of size to join close but broken CJK characters.
const double kCJKBrokenDistanceFraction = 0.25;
// Max number of components in a broken CJK character.
const int kCJKMaxComponents = 8;
// Max aspect ratio of CJK broken characters when put back together.
const double kCJKAspectRatio = 1.25;
// Max increase in aspect ratio of CJK broken characters when merged.
const double kCJKAspectRatioIncrease = 1.0625;
// Max multiple of the grid size that will be used in computing median CJKsize.
const int kMaxCJKSizeRatio = 5;
// Min fraction of blobs broken CJK to iterate and run it again.
const double kBrokenCJKIterationFraction = 0.125;
// Multiple of gridsize as x-padding for a search box for diacritic base
// characters.
const double kDiacriticXPadRatio = 7.0;
// Multiple of gridsize as y-padding for a search box for diacritic base
// characters.
const double kDiacriticYPadRatio = 1.75;
// Min multiple of diacritic height that a neighbour must be to be a
// convincing base character.
const double kMinDiacriticSizeRatio = 1.0625;
// Max multiple of a textline's median height as a threshold for the sum of
// a diacritic's farthest x and y distances (gap + size).
const double kMaxDiacriticDistanceRatio = 1.25;
// Max x-gap between a diacritic and its base char as a fraction of the height
// of the base char (allowing other blobs to fill the gap.)
const double kMaxDiacriticGapToBaseCharHeight = 1.0;
// Ratio between longest side of a line and longest side of a character.
// (neighbor_min > blob_min * kLineTrapShortest &&
//  neighbor_max < blob_max / kLineTrapLongest)
// => neighbor is a grapheme and blob is a line.
const int kLineTrapLongest = 4;
// Ratio between shortest side of a line and shortest side of a character.
const int kLineTrapShortest = 2;
// Max aspect ratio of the total box before CountNeighbourGaps
// decides immediately based on the aspect ratio.
const int kMostlyOneDirRatio = 3;
// Aspect ratio for a blob to be considered as line residue.
const double kLineResidueAspectRatio = 8.0;
// Padding ratio for line residue search box.
const int kLineResiduePadRatio = 3;
// Min multiple of neighbour size for a line residue to be genuine.
const double kLineResidueSizeRatio = 1.75;
// Aspect ratio filter for OSD.
const float kSizeRatioToReject = 2.0;
// Expansion factor for search box for good neighbours.
const double kNeighbourSearchFactor = 2.5;
// Factor of increase of overlap when adding diacritics to make an image noisy.
const double kNoiseOverlapGrowthFactor = 4.0;
// Fraction of the image size to add overlap when adding diacritics for an
// image to qualify as noisy.
const double kNoiseOverlapAreaFactor = 1.0 / 512;

StrokeWidth::StrokeWidth(int gridsize, const ICOORD &bleft, const ICOORD &tright)
    : BlobGrid(gridsize, bleft, tright)
    , nontext_map_(nullptr)
    , projection_(nullptr)
    , denorm_(nullptr)
    , grid_box_(bleft, tright)
    , rerotation_(1.0f, 0.0f) {
}

StrokeWidth::~StrokeWidth() {
#ifndef GRAPHICS_DISABLED
  if (widths_win_ != nullptr) {
    widths_win_->AwaitEvent(SVET_DESTROY);
    if (textord_tabfind_only_strokewidths) {
      exit(0);
    }
    delete widths_win_;
  }
  delete leaders_win_;
  delete initial_widths_win_;
  delete chains_win_;
  delete textlines_win_;
  delete smoothed_win_;
  delete diacritics_win_;
#endif
}

// Sets the neighbours member of the medium-sized blobs in the block.
// Searches on 4 sides of each blob for similar-sized, similar-strokewidth
// blobs and sets pointers to the good neighbours.
void StrokeWidth::SetNeighboursOnMediumBlobs(TO_BLOCK *block) {
  // Run a preliminary strokewidth neighbour detection on the medium blobs.
  InsertBlobList(&block->blobs);
  BLOBNBOX_IT blob_it(&block->blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    SetNeighbours(false, false, blob_it.data());
  }
  Clear();
}

// Sets the neighbour/textline writing direction members of the medium
// and large blobs with optional repair of broken CJK characters first.
// Repair of broken CJK is needed here because broken CJK characters
// can fool the textline direction detection algorithm.
void StrokeWidth::FindTextlineDirectionAndFixBrokenCJK(PageSegMode pageseg_mode, bool cjk_merge,
                                                       TO_BLOCK *input_block) {
  // Setup the grid with the remaining (non-noise) blobs.
  InsertBlobs(input_block);
  // Repair broken CJK characters if needed.
  while (cjk_merge && FixBrokenCJK(input_block)) {
  }
  // Grade blobs by inspection of neighbours.
  FindTextlineFlowDirection(pageseg_mode, false);
  // Clear the grid ready for rotation or leader finding.
  Clear();
}

// Helper to collect and count horizontal and vertical blobs from a list.
static void CollectHorizVertBlobs(BLOBNBOX_LIST *input_blobs, int *num_vertical_blobs,
                                  int *num_horizontal_blobs, BLOBNBOX_CLIST *vertical_blobs,
                                  BLOBNBOX_CLIST *horizontal_blobs,
                                  BLOBNBOX_CLIST *nondescript_blobs) {
  BLOBNBOX_C_IT v_it(vertical_blobs);
  BLOBNBOX_C_IT h_it(horizontal_blobs);
  BLOBNBOX_C_IT n_it(nondescript_blobs);
  BLOBNBOX_IT blob_it(input_blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    const TBOX &box = blob->bounding_box();
    float y_x = static_cast<float>(box.height()) / box.width();
    float x_y = 1.0f / y_x;
    // Select a >= 1.0 ratio
    float ratio = x_y > y_x ? x_y : y_x;
    // If the aspect ratio is small and we want them for osd, save the blob.
    bool ok_blob = ratio <= kSizeRatioToReject;
    if (blob->UniquelyVertical()) {
      ++*num_vertical_blobs;
      if (ok_blob) {
        v_it.add_after_then_move(blob);
      }
    } else if (blob->UniquelyHorizontal()) {
      ++*num_horizontal_blobs;
      if (ok_blob) {
        h_it.add_after_then_move(blob);
      }
    } else if (ok_blob) {
      n_it.add_after_then_move(blob);
    }
  }
}

// Types all the blobs as vertical or horizontal text or unknown and
// returns true if the majority are vertical.
// If the blobs are rotated, it is necessary to call CorrectForRotation
// after rotating everything, otherwise the work done here will be enough.
// If osd_blobs is not null, a list of blobs from the dominant textline
// direction are returned for use in orientation and script detection.
bool StrokeWidth::TestVerticalTextDirection(double find_vertical_text_ratio, TO_BLOCK *block,
                                            BLOBNBOX_CLIST *osd_blobs) {
  int vertical_boxes = 0;
  int horizontal_boxes = 0;
  // Count vertical normal and large blobs.
  BLOBNBOX_CLIST vertical_blobs;
  BLOBNBOX_CLIST horizontal_blobs;
  BLOBNBOX_CLIST nondescript_blobs;
  CollectHorizVertBlobs(&block->blobs, &vertical_boxes, &horizontal_boxes, &vertical_blobs,
                        &horizontal_blobs, &nondescript_blobs);
  CollectHorizVertBlobs(&block->large_blobs, &vertical_boxes, &horizontal_boxes, &vertical_blobs,
                        &horizontal_blobs, &nondescript_blobs);
  if (textord_debug_tabfind) {
    tprintf("TextDir hbox=%d vs vbox=%d, %dH, %dV, %dN osd blobs\n", horizontal_boxes,
            vertical_boxes, horizontal_blobs.length(), vertical_blobs.length(),
            nondescript_blobs.length());
  }
  if (osd_blobs != nullptr && vertical_boxes == 0 && horizontal_boxes == 0) {
    // Only nondescript blobs available, so return those.
    BLOBNBOX_C_IT osd_it(osd_blobs);
    osd_it.add_list_after(&nondescript_blobs);
    return false;
  }
  int min_vert_boxes =
      static_cast<int>((vertical_boxes + horizontal_boxes) * find_vertical_text_ratio);
  if (vertical_boxes >= min_vert_boxes) {
    if (osd_blobs != nullptr) {
      BLOBNBOX_C_IT osd_it(osd_blobs);
      osd_it.add_list_after(&vertical_blobs);
    }
    return true;
  } else {
    if (osd_blobs != nullptr) {
      BLOBNBOX_C_IT osd_it(osd_blobs);
      osd_it.add_list_after(&horizontal_blobs);
    }
    return false;
  }
}

// Corrects the data structures for the given rotation.
void StrokeWidth::CorrectForRotation(const FCOORD &rotation, ColPartitionGrid *part_grid) {
  Init(part_grid->gridsize(), part_grid->bleft(), part_grid->tright());
  grid_box_ = TBOX(bleft(), tright());
  rerotation_.set_x(rotation.x());
  rerotation_.set_y(-rotation.y());
}

// Finds leader partitions and inserts them into the given part_grid.
void StrokeWidth::FindLeaderPartitions(TO_BLOCK *block, ColPartitionGrid *part_grid) {
  Clear();
  // Find and isolate leaders in the noise list.
  ColPartition_LIST leader_parts;
  FindLeadersAndMarkNoise(block, &leader_parts);
  // Setup the strokewidth grid with the block's remaining (non-noise) blobs.
  InsertBlobList(&block->blobs);
  // Mark blobs that have leader neighbours.
  for (ColPartition_IT it(&leader_parts); !it.empty(); it.forward()) {
    ColPartition *part = it.extract();
    part->ClaimBoxes();
    MarkLeaderNeighbours(part, LR_LEFT);
    MarkLeaderNeighbours(part, LR_RIGHT);
    part_grid->InsertBBox(true, true, part);
  }
}

// Finds and marks noise those blobs that look like bits of vertical lines
// that would otherwise screw up layout analysis.
void StrokeWidth::RemoveLineResidue(ColPartition_LIST *big_part_list) {
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  // For every vertical line-like bbox in the grid, search its neighbours
  // to find the tallest, and if the original box is taller by sufficient
  // margin, then call it line residue and delete it.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    TBOX box = bbox->bounding_box();
    if (box.height() < box.width() * kLineResidueAspectRatio) {
      continue;
    }
    // Set up a rectangle search around the blob to find the size of its
    // neighbours.
    int padding = box.height() * kLineResiduePadRatio;
    TBOX search_box = box;
    search_box.pad(padding, padding);
    bool debug = AlignedBlob::WithinTestRegion(2, box.left(), box.bottom());
    // Find the largest object in the search box not equal to bbox.
    BlobGridSearch rsearch(this);
    int max_height = 0;
    BLOBNBOX *n;
    rsearch.StartRectSearch(search_box);
    while ((n = rsearch.NextRectSearch()) != nullptr) {
      if (n == bbox) {
        continue;
      }
      TBOX nbox = n->bounding_box();
      if (nbox.height() > max_height) {
        max_height = nbox.height();
      }
    }
    if (debug) {
      tprintf("Max neighbour size=%d for candidate line box at:", max_height);
      box.print();
    }
    if (max_height * kLineResidueSizeRatio < box.height()) {
#ifndef GRAPHICS_DISABLED
      if (leaders_win_ != nullptr) {
        // We are debugging, so display deleted in pink blobs in the same
        // window that we use to display leader detection.
        leaders_win_->Pen(ScrollView::PINK);
        leaders_win_->Rectangle(box.left(), box.bottom(), box.right(), box.top());
      }
#endif // !GRAPHICS_DISABLED
      ColPartition::MakeBigPartition(bbox, big_part_list);
    }
  }
}

// Types all the blobs as vertical text or horizontal text or unknown and
// puts them into initial ColPartitions in the supplied part_grid.
// rerotation determines how to get back to the image coordinates from the
// blob coordinates (since they may have been rotated for vertical text).
// block is the single block for the whole page or rectangle to be OCRed.
// nontext_pix (full-size), is a binary mask used to prevent merges across
// photo/text boundaries. It is not kept beyond this function.
// denorm provides a mapping back to the image from the current blob
// coordinate space.
// projection provides a measure of textline density over the image and
// provides functions to assist with diacritic detection. It should be a
// pointer to a new TextlineProjection, and will be setup here.
// part_grid is the output grid of textline partitions.
// Large blobs that cause overlap are put in separate partitions and added
// to the big_parts list.
void StrokeWidth::GradeBlobsIntoPartitions(PageSegMode pageseg_mode, const FCOORD &rerotation,
                                           TO_BLOCK *block, Image nontext_pix, const DENORM *denorm,
                                           bool cjk_script, TextlineProjection *projection,
                                           BLOBNBOX_LIST *diacritic_blobs,
                                           ColPartitionGrid *part_grid,
                                           ColPartition_LIST *big_parts) {
  nontext_map_ = nontext_pix;
  projection_ = projection;
  denorm_ = denorm;
  // Clear and re Insert to take advantage of the tab stops in the blobs.
  Clear();
  // Setup the strokewidth grid with the remaining non-noise, non-leader blobs.
  InsertBlobs(block);

  // Run FixBrokenCJK() again if the page is CJK.
  if (cjk_script) {
    FixBrokenCJK(block);
  }
  FindTextlineFlowDirection(pageseg_mode, false);
  projection_->ConstructProjection(block, rerotation, nontext_map_);
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_strokewidths) {
    ScrollView *line_blobs_win = MakeWindow(0, 0, "Initial textline Blobs");
    projection_->PlotGradedBlobs(&block->blobs, line_blobs_win);
    projection_->PlotGradedBlobs(&block->small_blobs, line_blobs_win);
  }
#endif
  projection_->MoveNonTextlineBlobs(&block->blobs, &block->noise_blobs);
  projection_->MoveNonTextlineBlobs(&block->small_blobs, &block->noise_blobs);
  // Clear and re Insert to take advantage of the removed diacritics.
  Clear();
  InsertBlobs(block);
  FCOORD skew;
  FindTextlineFlowDirection(pageseg_mode, true);
  PartitionFindResult r = FindInitialPartitions(pageseg_mode, rerotation, true, block,
                                                diacritic_blobs, part_grid, big_parts, &skew);
  if (r == PFR_NOISE) {
    tprintf("Detected %d diacritics\n", diacritic_blobs->length());
    // Noise was found, and removed.
    Clear();
    InsertBlobs(block);
    FindTextlineFlowDirection(pageseg_mode, true);
    r = FindInitialPartitions(pageseg_mode, rerotation, false, block, diacritic_blobs, part_grid,
                              big_parts, &skew);
  }
  nontext_map_ = nullptr;
  projection_ = nullptr;
  denorm_ = nullptr;
}

static void PrintBoxWidths(BLOBNBOX *neighbour) {
  const TBOX &nbox = neighbour->bounding_box();
  tprintf("Box (%d,%d)->(%d,%d): h-width=%.1f, v-width=%.1f p-width=%1.f\n", nbox.left(),
          nbox.bottom(), nbox.right(), nbox.top(), neighbour->horz_stroke_width(),
          neighbour->vert_stroke_width(),
          2.0 * neighbour->cblob()->area() / neighbour->cblob()->perimeter());
}

/** Handles a click event in a display window. */
void StrokeWidth::HandleClick(int x, int y) {
  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>::HandleClick(x, y);
  // Run a radial search for blobs that overlap.
  BlobGridSearch radsearch(this);
  radsearch.StartRadSearch(x, y, 1);
  BLOBNBOX *neighbour;
  FCOORD click(static_cast<float>(x), static_cast<float>(y));
  while ((neighbour = radsearch.NextRadSearch()) != nullptr) {
    TBOX nbox = neighbour->bounding_box();
    if (nbox.contains(click) && neighbour->cblob() != nullptr) {
      PrintBoxWidths(neighbour);
      if (neighbour->neighbour(BND_LEFT) != nullptr) {
        PrintBoxWidths(neighbour->neighbour(BND_LEFT));
      }
      if (neighbour->neighbour(BND_RIGHT) != nullptr) {
        PrintBoxWidths(neighbour->neighbour(BND_RIGHT));
      }
      if (neighbour->neighbour(BND_ABOVE) != nullptr) {
        PrintBoxWidths(neighbour->neighbour(BND_ABOVE));
      }
      if (neighbour->neighbour(BND_BELOW) != nullptr) {
        PrintBoxWidths(neighbour->neighbour(BND_BELOW));
      }
      int gaps[BND_COUNT];
      neighbour->NeighbourGaps(gaps);
      tprintf(
          "Left gap=%d, right=%d, above=%d, below=%d, horz=%d, vert=%d\n"
          "Good=    %d        %d        %d        %d\n",
          gaps[BND_LEFT], gaps[BND_RIGHT], gaps[BND_ABOVE], gaps[BND_BELOW],
          neighbour->horz_possible(), neighbour->vert_possible(),
          neighbour->good_stroke_neighbour(BND_LEFT), neighbour->good_stroke_neighbour(BND_RIGHT),
          neighbour->good_stroke_neighbour(BND_ABOVE), neighbour->good_stroke_neighbour(BND_BELOW));
      break;
    }
  }
}

// Detects and marks leader dots/dashes.
//    Leaders are horizontal chains of small or noise blobs that look
//    monospace according to ColPartition::MarkAsLeaderIfMonospaced().
// Detected leaders become the only occupants of the block->small_blobs list.
// Non-leader small blobs get moved to the blobs list.
// Non-leader noise blobs remain singletons in the noise list.
// All small and noise blobs in high density regions are marked BTFT_NONTEXT.
// block is the single block for the whole page or rectangle to be OCRed.
// leader_parts is the output.
void StrokeWidth::FindLeadersAndMarkNoise(TO_BLOCK *block, ColPartition_LIST *leader_parts) {
  InsertBlobList(&block->small_blobs);
  InsertBlobList(&block->noise_blobs);
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  // For every bbox in the grid, set its neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SetNeighbours(true, false, bbox);
  }
  ColPartition_IT part_it(leader_parts);
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    if (bbox->flow() == BTFT_NONE) {
      if (bbox->neighbour(BND_RIGHT) == nullptr && bbox->neighbour(BND_LEFT) == nullptr) {
        continue;
      }
      // Put all the linked blobs into a ColPartition.
      auto *part = new ColPartition(BRT_UNKNOWN, ICOORD(0, 1));
      BLOBNBOX *blob;
      for (blob = bbox; blob != nullptr && blob->flow() == BTFT_NONE;
           blob = blob->neighbour(BND_RIGHT)) {
        part->AddBox(blob);
      }
      for (blob = bbox->neighbour(BND_LEFT); blob != nullptr && blob->flow() == BTFT_NONE;
           blob = blob->neighbour(BND_LEFT)) {
        part->AddBox(blob);
      }
      if (part->MarkAsLeaderIfMonospaced()) {
        part_it.add_after_then_move(part);
      } else {
        delete part;
      }
    }
  }
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_strokewidths) {
    leaders_win_ = DisplayGoodBlobs("LeaderNeighbours", 0, 0);
  }
#endif
  // Move any non-leaders from the small to the blobs list, as they are
  // most likely dashes or broken characters.
  BLOBNBOX_IT blob_it(&block->blobs);
  BLOBNBOX_IT small_it(&block->small_blobs);
  for (small_it.mark_cycle_pt(); !small_it.cycled_list(); small_it.forward()) {
    BLOBNBOX *blob = small_it.data();
    if (blob->flow() != BTFT_LEADER) {
      if (blob->flow() == BTFT_NEIGHBOURS) {
        blob->set_flow(BTFT_NONE);
      }
      blob->ClearNeighbours();
      blob_it.add_to_end(small_it.extract());
    }
  }
  // Move leaders from the noise list to the small list, leaving the small
  // list exclusively leaders, so they don't get processed further,
  // and the remaining small blobs all in the noise list.
  BLOBNBOX_IT noise_it(&block->noise_blobs);
  for (noise_it.mark_cycle_pt(); !noise_it.cycled_list(); noise_it.forward()) {
    BLOBNBOX *blob = noise_it.data();
    if (blob->flow() == BTFT_LEADER || blob->joined_to_prev()) {
      small_it.add_to_end(noise_it.extract());
    } else if (blob->flow() == BTFT_NEIGHBOURS) {
      blob->set_flow(BTFT_NONE);
      blob->ClearNeighbours();
    }
  }
  // Clear the grid as we don't want the small stuff hanging around in it.
  Clear();
}

/** Inserts the block blobs (normal and large) into this grid.
 * Blobs remain owned by the block. */
void StrokeWidth::InsertBlobs(TO_BLOCK *block) {
  InsertBlobList(&block->blobs);
  InsertBlobList(&block->large_blobs);
}

// Checks the left or right side of the given leader partition and sets the
// (opposite) leader_on_right or leader_on_left flags for blobs
// that are next to the given side of the given leader partition.
void StrokeWidth::MarkLeaderNeighbours(const ColPartition *part, LeftOrRight side) {
  const TBOX &part_box = part->bounding_box();
  BlobGridSearch blobsearch(this);
  // Search to the side of the leader for the nearest neighbour.
  BLOBNBOX *best_blob = nullptr;
  int best_gap = 0;
  blobsearch.StartSideSearch(side == LR_LEFT ? part_box.left() : part_box.right(),
                             part_box.bottom(), part_box.top());
  BLOBNBOX *blob;
  while ((blob = blobsearch.NextSideSearch(side == LR_LEFT)) != nullptr) {
    const TBOX &blob_box = blob->bounding_box();
    if (!blob_box.y_overlap(part_box)) {
      continue;
    }
    int x_gap = blob_box.x_gap(part_box);
    if (x_gap > 2 * gridsize()) {
      break;
    } else if (best_blob == nullptr || x_gap < best_gap) {
      best_blob = blob;
      best_gap = x_gap;
    }
  }
  if (best_blob != nullptr) {
    if (side == LR_LEFT) {
      best_blob->set_leader_on_right(true);
    } else {
      best_blob->set_leader_on_left(true);
    }
#ifndef GRAPHICS_DISABLED
    if (leaders_win_ != nullptr) {
      leaders_win_->Pen(side == LR_LEFT ? ScrollView::RED : ScrollView::GREEN);
      const TBOX &blob_box = best_blob->bounding_box();
      leaders_win_->Rectangle(blob_box.left(), blob_box.bottom(), blob_box.right(), blob_box.top());
    }
#endif // !GRAPHICS_DISABLED
  }
}

// Helper to compute the UQ of the square-ish CJK characters.
static int UpperQuartileCJKSize(int gridsize, BLOBNBOX_LIST *blobs) {
  STATS sizes(0, gridsize * kMaxCJKSizeRatio - 1);
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    int width = blob->bounding_box().width();
    int height = blob->bounding_box().height();
    if (width <= height * kCJKAspectRatio && height < width * kCJKAspectRatio) {
      sizes.add(height, 1);
    }
  }
  return static_cast<int>(sizes.ile(0.75f) + 0.5);
}

// Fix broken CJK characters, using the fake joined blobs mechanism.
// Blobs are really merged, ie the master takes all the outlines and the
// others are deleted.
// Returns true if sufficient blobs are merged that it may be worth running
// again, due to a better estimate of character size.
bool StrokeWidth::FixBrokenCJK(TO_BLOCK *block) {
  BLOBNBOX_LIST *blobs = &block->blobs;
  int median_height = UpperQuartileCJKSize(gridsize(), blobs);
  int max_dist = static_cast<int>(median_height * kCJKBrokenDistanceFraction);
  int max_height = static_cast<int>(median_height * kCJKAspectRatio);
  int num_fixed = 0;
  BLOBNBOX_IT blob_it(blobs);

  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    if (blob->cblob() == nullptr || blob->cblob()->out_list()->empty()) {
      continue;
    }
    TBOX bbox = blob->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(3, bbox.left(), bbox.bottom());
    if (debug) {
      tprintf("Checking for Broken CJK (max size=%d):", max_height);
      bbox.print();
    }
    // Generate a list of blobs that overlap or are near enough to merge.
    BLOBNBOX_CLIST overlapped_blobs;
    AccumulateOverlaps(blob, debug, max_height, max_dist, &bbox, &overlapped_blobs);
    if (!overlapped_blobs.empty()) {
      // There are overlapping blobs, so qualify them as being satisfactory
      // before removing them from the grid and replacing them with the union.
      // The final box must be roughly square.
      if (bbox.width() > bbox.height() * kCJKAspectRatio ||
          bbox.height() > bbox.width() * kCJKAspectRatio) {
        if (debug) {
          tprintf("Bad final aspectratio:");
          bbox.print();
        }
        continue;
      }
      // There can't be too many blobs to merge.
      if (overlapped_blobs.length() >= kCJKMaxComponents) {
        if (debug) {
          tprintf("Too many neighbours: %d\n", overlapped_blobs.length());
        }
        continue;
      }
      // The strokewidths must match amongst the join candidates.
      BLOBNBOX_C_IT n_it(&overlapped_blobs);
      for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
        BLOBNBOX *neighbour = nullptr;
        neighbour = n_it.data();
        if (!blob->MatchingStrokeWidth(*neighbour, kStrokeWidthFractionCJK, kStrokeWidthCJK)) {
          break;
        }
      }
      if (!n_it.cycled_list()) {
        if (debug) {
          tprintf("Bad stroke widths:");
          PrintBoxWidths(blob);
        }
        continue; // Not good enough.
      }

      // Merge all the candidates into blob.
      // We must remove blob from the grid and reinsert it after merging
      // to maintain the integrity of the grid.
      RemoveBBox(blob);
      // Everything else will be calculated later.
      for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
        BLOBNBOX *neighbour = n_it.data();
        RemoveBBox(neighbour);
        // Mark empty blob for deletion.
        neighbour->set_region_type(BRT_NOISE);
        blob->really_merge(neighbour);
        if (rerotation_.x() != 1.0f || rerotation_.y() != 0.0f) {
          blob->rotate_box(rerotation_);
        }
      }
      InsertBBox(true, true, blob);
      ++num_fixed;
      if (debug) {
        tprintf("Done! Final box:");
        bbox.print();
      }
    }
  }
  // Count remaining blobs.
  int num_remaining = 0;
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    if (blob->cblob() != nullptr && !blob->cblob()->out_list()->empty()) {
      ++num_remaining;
    }
  }
  // Permanently delete all the marked blobs after first removing all
  // references in the neighbour members.
  block->DeleteUnownedNoise();
  return num_fixed > num_remaining * kBrokenCJKIterationFraction;
}

// Helper function to determine whether it is reasonable to merge the
// bbox and the nbox for repairing broken CJK.
// The distance apart must not exceed max_dist, the combined size must
// not exceed max_size, and the aspect ratio must either improve or at
// least not get worse by much.
static bool AcceptableCJKMerge(const TBOX &bbox, const TBOX &nbox, bool debug, int max_size,
                               int max_dist, int *x_gap, int *y_gap) {
  *x_gap = bbox.x_gap(nbox);
  *y_gap = bbox.y_gap(nbox);
  TBOX merged(nbox);
  merged += bbox;
  if (debug) {
    tprintf("gaps = %d, %d, merged_box:", *x_gap, *y_gap);
    merged.print();
  }
  if (*x_gap <= max_dist && *y_gap <= max_dist && merged.width() <= max_size &&
      merged.height() <= max_size) {
    // Close enough to call overlapping. Check aspect ratios.
    double old_ratio = static_cast<double>(bbox.width()) / bbox.height();
    if (old_ratio < 1.0) {
      old_ratio = 1.0 / old_ratio;
    }
    double new_ratio = static_cast<double>(merged.width()) / merged.height();
    if (new_ratio < 1.0) {
      new_ratio = 1.0 / new_ratio;
    }
    if (new_ratio <= old_ratio * kCJKAspectRatioIncrease) {
      return true;
    }
  }
  return false;
}

// Collect blobs that overlap or are within max_dist of the input bbox.
// Return them in the list of blobs and expand the bbox to be the union
// of all the boxes. not_this is excluded from the search, as are blobs
// that cause the merged box to exceed max_size in either dimension.
void StrokeWidth::AccumulateOverlaps(const BLOBNBOX *not_this, bool debug, int max_size,
                                     int max_dist, TBOX *bbox, BLOBNBOX_CLIST *blobs) {
  // While searching, nearests holds the nearest failed blob in each
  // direction. When we have a nearest in each of the 4 directions, then
  // the search is over, and at this point the final bbox must not overlap
  // any of the nearests.
  BLOBNBOX *nearests[BND_COUNT];
  for (auto &nearest : nearests) {
    nearest = nullptr;
  }
  int x = (bbox->left() + bbox->right()) / 2;
  int y = (bbox->bottom() + bbox->top()) / 2;
  // Run a radial search for blobs that overlap or are sufficiently close.
  BlobGridSearch radsearch(this);
  radsearch.StartRadSearch(x, y, kCJKRadius);
  BLOBNBOX *neighbour;
  while ((neighbour = radsearch.NextRadSearch()) != nullptr) {
    if (neighbour == not_this) {
      continue;
    }
    TBOX nbox = neighbour->bounding_box();
    int x_gap, y_gap;
    if (AcceptableCJKMerge(*bbox, nbox, debug, max_size, max_dist, &x_gap, &y_gap)) {
      // Close enough to call overlapping. Merge boxes.
      *bbox += nbox;
      blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, neighbour);
      if (debug) {
        tprintf("Added:");
        nbox.print();
      }
      // Since we merged, search the nearests, as some might now me mergeable.
      for (int dir = 0; dir < BND_COUNT; ++dir) {
        if (nearests[dir] == nullptr) {
          continue;
        }
        nbox = nearests[dir]->bounding_box();
        if (AcceptableCJKMerge(*bbox, nbox, debug, max_size, max_dist, &x_gap, &y_gap)) {
          // Close enough to call overlapping. Merge boxes.
          *bbox += nbox;
          blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, nearests[dir]);
          if (debug) {
            tprintf("Added:");
            nbox.print();
          }
          nearests[dir] = nullptr;
          dir = -1; // Restart the search.
        }
      }
    } else if (x_gap < 0 && x_gap <= y_gap) {
      // A vertical neighbour. Record the nearest.
      BlobNeighbourDir dir = nbox.top() > bbox->top() ? BND_ABOVE : BND_BELOW;
      if (nearests[dir] == nullptr || y_gap < bbox->y_gap(nearests[dir]->bounding_box())) {
        nearests[dir] = neighbour;
      }
    } else if (y_gap < 0 && y_gap <= x_gap) {
      // A horizontal neighbour. Record the nearest.
      BlobNeighbourDir dir = nbox.left() > bbox->left() ? BND_RIGHT : BND_LEFT;
      if (nearests[dir] == nullptr || x_gap < bbox->x_gap(nearests[dir]->bounding_box())) {
        nearests[dir] = neighbour;
      }
    }
    // If all nearests are non-null, then we have finished.
    if (nearests[BND_LEFT] && nearests[BND_RIGHT] && nearests[BND_ABOVE] && nearests[BND_BELOW]) {
      break;
    }
  }
  // Final overlap with a nearest is not allowed.
  for (auto &nearest : nearests) {
    if (nearest == nullptr) {
      continue;
    }
    const TBOX &nbox = nearest->bounding_box();
    if (debug) {
      tprintf("Testing for overlap with:");
      nbox.print();
    }
    if (bbox->overlap(nbox)) {
      blobs->shallow_clear();
      if (debug) {
        tprintf("Final box overlaps nearest\n");
      }
      return;
    }
  }
}

// For each blob in this grid, Finds the textline direction to be horizontal
// or vertical according to distance to neighbours and 1st and 2nd order
// neighbours. Non-text tends to end up without a definite direction.
// Result is setting of the neighbours and vert_possible/horz_possible
// flags in the BLOBNBOXes currently in this grid.
// This function is called more than once if page orientation is uncertain,
// so display_if_debugging is true on the final call to display the results.
void StrokeWidth::FindTextlineFlowDirection(PageSegMode pageseg_mode, bool display_if_debugging) {
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  // For every bbox in the grid, set its neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SetNeighbours(false, display_if_debugging, bbox);
  }
  // Where vertical or horizontal wins by a big margin, clarify it.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SimplifyObviousNeighbours(bbox);
  }
  // Now try to make the blobs only vertical or horizontal using neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    if (FindingVerticalOnly(pageseg_mode)) {
      bbox->set_vert_possible(true);
      bbox->set_horz_possible(false);
    } else if (FindingHorizontalOnly(pageseg_mode)) {
      bbox->set_vert_possible(false);
      bbox->set_horz_possible(true);
    } else {
      SetNeighbourFlows(bbox);
    }
  }
#ifndef GRAPHICS_DISABLED
  if ((textord_tabfind_show_strokewidths && display_if_debugging) ||
      textord_tabfind_show_strokewidths > 1) {
    initial_widths_win_ = DisplayGoodBlobs("InitialStrokewidths", 400, 0);
  }
#endif
  // Improve flow direction with neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SmoothNeighbourTypes(pageseg_mode, false, bbox);
  }
  // Now allow reset of firm values to fix renegades.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SmoothNeighbourTypes(pageseg_mode, true, bbox);
  }
  // Repeat.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    SmoothNeighbourTypes(pageseg_mode, true, bbox);
  }
#ifndef GRAPHICS_DISABLED
  if ((textord_tabfind_show_strokewidths && display_if_debugging) ||
      textord_tabfind_show_strokewidths > 1) {
    widths_win_ = DisplayGoodBlobs("ImprovedStrokewidths", 800, 0);
  }
#endif
}

// Sets the neighbours and good_stroke_neighbours members of the blob by
// searching close on all 4 sides.
// When finding leader dots/dashes, there is a slightly different rule for
// what makes a good neighbour.
void StrokeWidth::SetNeighbours(bool leaders, bool activate_line_trap, BLOBNBOX *blob) {
  int line_trap_count = 0;
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    line_trap_count += FindGoodNeighbour(bnd, leaders, blob);
  }
  if (line_trap_count > 0 && activate_line_trap) {
    // It looks like a line so isolate it by clearing its neighbours.
    blob->ClearNeighbours();
    const TBOX &box = blob->bounding_box();
    blob->set_region_type(box.width() > box.height() ? BRT_HLINE : BRT_VLINE);
  }
}

// Sets the good_stroke_neighbours member of the blob if it has a
// GoodNeighbour on the given side.
// Also sets the neighbour in the blob, whether or not a good one is found.
// Returns the number of blobs in the nearby search area that would lead us to
// believe that this blob is a line separator.
// Leaders get extra special lenient treatment.
int StrokeWidth::FindGoodNeighbour(BlobNeighbourDir dir, bool leaders, BLOBNBOX *blob) {
  // Search for neighbours that overlap vertically.
  TBOX blob_box = blob->bounding_box();
  bool debug = AlignedBlob::WithinTestRegion(2, blob_box.left(), blob_box.bottom());
  if (debug) {
    tprintf("FGN in dir %d for blob:", dir);
    blob_box.print();
  }
  int top = blob_box.top();
  int bottom = blob_box.bottom();
  int left = blob_box.left();
  int right = blob_box.right();
  int width = right - left;
  int height = top - bottom;

  // A trap to detect lines tests for the min dimension of neighbours
  // being larger than a multiple of the min dimension of the line
  // and the larger dimension being smaller than a fraction of the max
  // dimension of the line.
  int line_trap_max = std::max(width, height) / kLineTrapLongest;
  int line_trap_min = std::min(width, height) * kLineTrapShortest;
  int line_trap_count = 0;

  int min_good_overlap = (dir == BND_LEFT || dir == BND_RIGHT) ? height / 2 : width / 2;
  int min_decent_overlap = (dir == BND_LEFT || dir == BND_RIGHT) ? height / 3 : width / 3;
  if (leaders) {
    min_good_overlap = min_decent_overlap = 1;
  }

  int search_pad =
      static_cast<int>(sqrt(static_cast<double>(width * height)) * kNeighbourSearchFactor);
  if (gridsize() > search_pad) {
    search_pad = gridsize();
  }
  TBOX search_box = blob_box;
  // Pad the search in the appropriate direction.
  switch (dir) {
    case BND_LEFT:
      search_box.set_left(search_box.left() - search_pad);
      break;
    case BND_RIGHT:
      search_box.set_right(search_box.right() + search_pad);
      break;
    case BND_BELOW:
      search_box.set_bottom(search_box.bottom() - search_pad);
      break;
    case BND_ABOVE:
      search_box.set_top(search_box.top() + search_pad);
      break;
    case BND_COUNT:
      return 0;
  }

  BlobGridSearch rectsearch(this);
  rectsearch.StartRectSearch(search_box);
  BLOBNBOX *best_neighbour = nullptr;
  double best_goodness = 0.0;
  bool best_is_good = false;
  BLOBNBOX *neighbour;
  while ((neighbour = rectsearch.NextRectSearch()) != nullptr) {
    TBOX nbox = neighbour->bounding_box();
    if (neighbour == blob) {
      continue;
    }
    int mid_x = (nbox.left() + nbox.right()) / 2;
    if (mid_x < blob->left_rule() || mid_x > blob->right_rule()) {
      continue; // In a different column.
    }
    if (debug) {
      tprintf("Neighbour at:");
      nbox.print();
    }

    // Last-minute line detector. There is a small upper limit to the line
    // width accepted by the morphological line detector.
    int n_width = nbox.width();
    int n_height = nbox.height();
    if (std::min(n_width, n_height) > line_trap_min &&
        std::max(n_width, n_height) < line_trap_max) {
      ++line_trap_count;
    }
    // Heavily joined text, such as Arabic may have very different sizes when
    // looking at the maxes, but the heights may be almost identical, so check
    // for a difference in height if looking sideways or width vertically.
    if (TabFind::VeryDifferentSizes(std::max(n_width, n_height), std::max(width, height)) &&
        (((dir == BND_LEFT || dir == BND_RIGHT) && TabFind::DifferentSizes(n_height, height)) ||
         ((dir == BND_BELOW || dir == BND_ABOVE) && TabFind::DifferentSizes(n_width, width)))) {
      if (debug) {
        tprintf("Bad size\n");
      }
      continue; // Could be a different font size or non-text.
    }
    // Amount of vertical overlap between the blobs.
    int overlap;
    // If the overlap is along the short side of the neighbour, and it
    // is fully overlapped, then perp_overlap holds the length of the long
    // side of the neighbour. A measure to include hyphens and dashes as
    // legitimate neighbours.
    int perp_overlap;
    int gap;
    if (dir == BND_LEFT || dir == BND_RIGHT) {
      overlap = std::min(static_cast<int>(nbox.top()), top) -
                std::max(static_cast<int>(nbox.bottom()), bottom);
      if (overlap == nbox.height() && nbox.width() > nbox.height()) {
        perp_overlap = nbox.width();
      } else {
        perp_overlap = overlap;
      }
      gap = dir == BND_LEFT ? left - nbox.left() : nbox.right() - right;
      if (gap <= 0) {
        if (debug) {
          tprintf("On wrong side\n");
        }
        continue; // On the wrong side.
      }
      gap -= n_width;
    } else {
      overlap = std::min(static_cast<int>(nbox.right()), right) -
                std::max(static_cast<int>(nbox.left()), left);
      if (overlap == nbox.width() && nbox.height() > nbox.width()) {
        perp_overlap = nbox.height();
      } else {
        perp_overlap = overlap;
      }
      gap = dir == BND_BELOW ? bottom - nbox.bottom() : nbox.top() - top;
      if (gap <= 0) {
        if (debug) {
          tprintf("On wrong side\n");
        }
        continue; // On the wrong side.
      }
      gap -= n_height;
    }
    if (-gap > overlap) {
      if (debug) {
        tprintf("Overlaps wrong way\n");
      }
      continue; // Overlaps the wrong way.
    }
    if (perp_overlap < min_decent_overlap) {
      if (debug) {
        tprintf("Doesn't overlap enough\n");
      }
      continue; // Doesn't overlap enough.
    }
    bool bad_sizes =
        TabFind::DifferentSizes(height, n_height) && TabFind::DifferentSizes(width, n_width);
    bool is_good =
        overlap >= min_good_overlap && !bad_sizes &&
        blob->MatchingStrokeWidth(*neighbour, kStrokeWidthFractionTolerance, kStrokeWidthTolerance);
    // Best is a fuzzy combination of gap, overlap and is good.
    // Basically if you make one thing twice as good without making
    // anything else twice as bad, then it is better.
    if (gap < 1) {
      gap = 1;
    }
    double goodness = (1.0 + is_good) * overlap / gap;
    if (debug) {
      tprintf("goodness = %g vs best of %g, good=%d, overlap=%d, gap=%d\n", goodness, best_goodness,
              is_good, overlap, gap);
    }
    if (goodness > best_goodness) {
      best_neighbour = neighbour;
      best_goodness = goodness;
      best_is_good = is_good;
    }
  }
  blob->set_neighbour(dir, best_neighbour, best_is_good);
  return line_trap_count;
}

// Helper to get a list of 1st-order neighbours.
static void ListNeighbours(const BLOBNBOX *blob, BLOBNBOX_CLIST *neighbours) {
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX *neighbour = blob->neighbour(bnd);
    if (neighbour != nullptr) {
      neighbours->add_sorted(SortByBoxLeft<BLOBNBOX>, true, neighbour);
    }
  }
}

// Helper to get a list of 1st and 2nd order neighbours.
static void List2ndNeighbours(const BLOBNBOX *blob, BLOBNBOX_CLIST *neighbours) {
  ListNeighbours(blob, neighbours);
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX *neighbour = blob->neighbour(bnd);
    if (neighbour != nullptr) {
      ListNeighbours(neighbour, neighbours);
    }
  }
}

// Helper to get a list of 1st, 2nd and 3rd order neighbours.
static void List3rdNeighbours(const BLOBNBOX *blob, BLOBNBOX_CLIST *neighbours) {
  List2ndNeighbours(blob, neighbours);
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX *neighbour = blob->neighbour(bnd);
    if (neighbour != nullptr) {
      List2ndNeighbours(neighbour, neighbours);
    }
  }
}

// Helper to count the evidence for verticalness or horizontalness
// in a list of neighbours.
static void CountNeighbourGaps(bool debug, BLOBNBOX_CLIST *neighbours, int *pure_h_count,
                               int *pure_v_count) {
  if (neighbours->length() <= kMostlyOneDirRatio) {
    return;
  }
  BLOBNBOX_C_IT it(neighbours);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    int h_min, h_max, v_min, v_max;
    blob->MinMaxGapsClipped(&h_min, &h_max, &v_min, &v_max);
    if (debug) {
      tprintf("Hgaps [%d,%d], vgaps [%d,%d]:", h_min, h_max, v_min, v_max);
    }
    if (h_max < v_min || blob->leader_on_left() || blob->leader_on_right()) {
      // Horizontal gaps are clear winners. Count a pure horizontal.
      ++*pure_h_count;
      if (debug) {
        tprintf("Horz at:");
      }
    } else if (v_max < h_min) {
      // Vertical gaps are clear winners. Clear a pure vertical.
      ++*pure_v_count;
      if (debug) {
        tprintf("Vert at:");
      }
    } else {
      if (debug) {
        tprintf("Neither at:");
      }
    }
    if (debug) {
      blob->bounding_box().print();
    }
  }
}

// Makes the blob to be only horizontal or vertical where evidence
// is clear based on gaps of 2nd order neighbours, or definite individual
// blobs.
void StrokeWidth::SetNeighbourFlows(BLOBNBOX *blob) {
  if (blob->DefiniteIndividualFlow()) {
    return;
  }
  bool debug =
      AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(), blob->bounding_box().bottom());
  if (debug) {
    tprintf("SetNeighbourFlows (current flow=%d, type=%d) on:", blob->flow(), blob->region_type());
    blob->bounding_box().print();
  }
  BLOBNBOX_CLIST neighbours;
  List3rdNeighbours(blob, &neighbours);
  // The number of pure horizontal and vertical neighbours.
  int pure_h_count = 0;
  int pure_v_count = 0;
  CountNeighbourGaps(debug, &neighbours, &pure_h_count, &pure_v_count);
  if (debug) {
    HandleClick(blob->bounding_box().left() + 1, blob->bounding_box().bottom() + 1);
    tprintf("SetFlows: h_count=%d, v_count=%d\n", pure_h_count, pure_v_count);
  }
  if (!neighbours.empty()) {
    blob->set_vert_possible(true);
    blob->set_horz_possible(true);
    if (pure_h_count > 2 * pure_v_count) {
      // Horizontal gaps are clear winners. Clear vertical neighbours.
      blob->set_vert_possible(false);
    } else if (pure_v_count > 2 * pure_h_count) {
      // Vertical gaps are clear winners. Clear horizontal neighbours.
      blob->set_horz_possible(false);
    }
  } else {
    // Lonely blob. Can't tell its flow direction.
    blob->set_vert_possible(false);
    blob->set_horz_possible(false);
  }
}

// Helper to count the number of horizontal and vertical blobs in a list.
static void CountNeighbourTypes(BLOBNBOX_CLIST *neighbours, int *pure_h_count, int *pure_v_count) {
  BLOBNBOX_C_IT it(neighbours);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    if (blob->UniquelyHorizontal()) {
      ++*pure_h_count;
    }
    if (blob->UniquelyVertical()) {
      ++*pure_v_count;
    }
  }
}

// Nullify the neighbours in the wrong directions where the direction
// is clear-cut based on a distance margin. Good for isolating vertical
// text from neighbouring horizontal text.
void StrokeWidth::SimplifyObviousNeighbours(BLOBNBOX *blob) {
  // Case 1: We have text that is likely several characters, blurry and joined
  //         together.
  if ((blob->bounding_box().width() > 3 * blob->area_stroke_width() &&
       blob->bounding_box().height() > 3 * blob->area_stroke_width())) {
    // The blob is complex (not stick-like).
    if (blob->bounding_box().width() > 4 * blob->bounding_box().height()) {
      // Horizontal conjoined text.
      blob->set_neighbour(BND_ABOVE, nullptr, false);
      blob->set_neighbour(BND_BELOW, nullptr, false);
      return;
    }
    if (blob->bounding_box().height() > 4 * blob->bounding_box().width()) {
      // Vertical conjoined text.
      blob->set_neighbour(BND_LEFT, nullptr, false);
      blob->set_neighbour(BND_RIGHT, nullptr, false);
      return;
    }
  }

  // Case 2: This blob is likely a single character.
  int margin = gridsize() / 2;
  int h_min, h_max, v_min, v_max;
  blob->MinMaxGapsClipped(&h_min, &h_max, &v_min, &v_max);
  if ((h_max + margin < v_min && h_max < margin / 2) || blob->leader_on_left() ||
      blob->leader_on_right()) {
    // Horizontal gaps are clear winners. Clear vertical neighbours.
    blob->set_neighbour(BND_ABOVE, nullptr, false);
    blob->set_neighbour(BND_BELOW, nullptr, false);
  } else if (v_max + margin < h_min && v_max < margin / 2) {
    // Vertical gaps are clear winners. Clear horizontal neighbours.
    blob->set_neighbour(BND_LEFT, nullptr, false);
    blob->set_neighbour(BND_RIGHT, nullptr, false);
  }
}

// Smoothes the vertical/horizontal type of the blob based on the
// 2nd-order neighbours. If reset_all is true, then all blobs are
// changed. Otherwise, only ambiguous blobs are processed.
void StrokeWidth::SmoothNeighbourTypes(PageSegMode pageseg_mode, bool reset_all, BLOBNBOX *blob) {
  if ((blob->vert_possible() && blob->horz_possible()) || reset_all) {
    // There are both horizontal and vertical so try to fix it.
    BLOBNBOX_CLIST neighbours;
    List2ndNeighbours(blob, &neighbours);
    // The number of pure horizontal and vertical neighbours.
    int pure_h_count = 0;
    int pure_v_count = 0;
    CountNeighbourTypes(&neighbours, &pure_h_count, &pure_v_count);
    if (AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(),
                                      blob->bounding_box().bottom())) {
      HandleClick(blob->bounding_box().left() + 1, blob->bounding_box().bottom() + 1);
      tprintf("pure_h=%d, pure_v=%d\n", pure_h_count, pure_v_count);
    }
    if (pure_h_count > pure_v_count && !FindingVerticalOnly(pageseg_mode)) {
      // Horizontal gaps are clear winners. Clear vertical neighbours.
      blob->set_vert_possible(false);
      blob->set_horz_possible(true);
    } else if (pure_v_count > pure_h_count && !FindingHorizontalOnly(pageseg_mode)) {
      // Vertical gaps are clear winners. Clear horizontal neighbours.
      blob->set_horz_possible(false);
      blob->set_vert_possible(true);
    }
  } else if (AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(),
                                           blob->bounding_box().bottom())) {
    HandleClick(blob->bounding_box().left() + 1, blob->bounding_box().bottom() + 1);
    tprintf("Clean on pass 3!\n");
  }
}

// Partition creation. Accumulates vertical and horizontal text chains,
// puts the remaining blobs in as unknowns, and then merges/splits to
// minimize overlap and smoothes the types with neighbours and the color
// image if provided. rerotation is used to rotate the coordinate space
// back to the nontext_map_ image.
// If find_problems is true, detects possible noise pollution by the amount
// of partition overlap that is created by the diacritics. If excessive, the
// noise is separated out into diacritic blobs, and PFR_NOISE is returned.
// [TODO(rays): if the partition overlap is caused by heavy skew, deskews
// the components, saves the skew_angle and returns PFR_SKEW.] If the return
// is not PFR_OK, the job is incomplete, and FindInitialPartitions must be
// called again after cleaning up the partly done work.
PartitionFindResult StrokeWidth::FindInitialPartitions(
    PageSegMode pageseg_mode, const FCOORD &rerotation, bool find_problems, TO_BLOCK *block,
    BLOBNBOX_LIST *diacritic_blobs, ColPartitionGrid *part_grid, ColPartition_LIST *big_parts,
    FCOORD *skew_angle) {
  if (!FindingHorizontalOnly(pageseg_mode)) {
    FindVerticalTextChains(part_grid);
  }
  if (!FindingVerticalOnly(pageseg_mode)) {
    FindHorizontalTextChains(part_grid);
  }
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_strokewidths) {
    chains_win_ = MakeWindow(0, 400, "Initial text chains");
    part_grid->DisplayBoxes(chains_win_);
    projection_->DisplayProjection();
  }
#endif
  if (find_problems) {
    // TODO(rays) Do something to find skew, set skew_angle and return if there
    // is some.
  }
  part_grid->SplitOverlappingPartitions(big_parts);
  EasyMerges(part_grid);
  RemoveLargeUnusedBlobs(block, part_grid, big_parts);
  TBOX grid_box(bleft(), tright());
  while (part_grid->GridSmoothNeighbours(BTFT_CHAIN, nontext_map_, grid_box, rerotation)) {
    ;
  }
  while (part_grid->GridSmoothNeighbours(BTFT_NEIGHBOURS, nontext_map_, grid_box, rerotation)) {
    ;
  }
  int pre_overlap = part_grid->ComputeTotalOverlap(nullptr);
  TestDiacritics(part_grid, block);
  MergeDiacritics(block, part_grid);
  if (find_problems && diacritic_blobs != nullptr &&
      DetectAndRemoveNoise(pre_overlap, grid_box, block, part_grid, diacritic_blobs)) {
    return PFR_NOISE;
  }
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_strokewidths) {
    textlines_win_ = MakeWindow(400, 400, "GoodTextline blobs");
    part_grid->DisplayBoxes(textlines_win_);
    diacritics_win_ = DisplayDiacritics("Diacritics", 0, 0, block);
  }
#endif
  PartitionRemainingBlobs(pageseg_mode, part_grid);
  part_grid->SplitOverlappingPartitions(big_parts);
  EasyMerges(part_grid);
  while (part_grid->GridSmoothNeighbours(BTFT_CHAIN, nontext_map_, grid_box, rerotation)) {
    ;
  }
  while (part_grid->GridSmoothNeighbours(BTFT_NEIGHBOURS, nontext_map_, grid_box, rerotation)) {
    ;
  }
  // Now eliminate strong stuff in a sea of the opposite.
  while (part_grid->GridSmoothNeighbours(BTFT_STRONG_CHAIN, nontext_map_, grid_box, rerotation)) {
    ;
  }
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_strokewidths) {
    smoothed_win_ = MakeWindow(800, 400, "Smoothed blobs");
    part_grid->DisplayBoxes(smoothed_win_);
  }
#endif
  return PFR_OK;
}

// Detects noise by a significant increase in partition overlap from
// pre_overlap to now, and removes noise from the union of all the overlapping
// partitions, placing the blobs in diacritic_blobs. Returns true if any noise
// was found and removed.
bool StrokeWidth::DetectAndRemoveNoise(int pre_overlap, const TBOX &grid_box, TO_BLOCK *block,
                                       ColPartitionGrid *part_grid,
                                       BLOBNBOX_LIST *diacritic_blobs) {
  ColPartitionGrid *noise_grid = nullptr;
  int post_overlap = part_grid->ComputeTotalOverlap(&noise_grid);
  if (pre_overlap == 0) {
    pre_overlap = 1;
  }
  BLOBNBOX_IT diacritic_it(diacritic_blobs);
  if (noise_grid != nullptr) {
    if (post_overlap > pre_overlap * kNoiseOverlapGrowthFactor &&
        post_overlap > grid_box.area() * kNoiseOverlapAreaFactor) {
      // This is noisy enough to fix.
#ifndef GRAPHICS_DISABLED
      if (textord_tabfind_show_strokewidths) {
        ScrollView *noise_win = MakeWindow(1000, 500, "Noise Areas");
        noise_grid->DisplayBoxes(noise_win);
      }
#endif
      part_grid->DeleteNonLeaderParts();
      BLOBNBOX_IT blob_it(&block->noise_blobs);
      ColPartitionGridSearch rsearch(noise_grid);
      for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
        BLOBNBOX *blob = blob_it.data();
        blob->ClearNeighbours();
        if (!blob->IsDiacritic() || blob->owner() != nullptr) {
          continue; // Not a noise candidate.
        }
        TBOX search_box(blob->bounding_box());
        search_box.pad(gridsize(), gridsize());
        rsearch.StartRectSearch(search_box);
        ColPartition *part = rsearch.NextRectSearch();
        if (part != nullptr) {
          // Consider blob as possible noise.
          blob->set_owns_cblob(true);
          blob->compute_bounding_box();
          diacritic_it.add_after_then_move(blob_it.extract());
        }
      }
      noise_grid->DeleteParts();
      delete noise_grid;
      return true;
    }
    noise_grid->DeleteParts();
    delete noise_grid;
  }
  return false;
}

// Helper verifies that blob's neighbour in direction dir is good to add to a
// vertical text chain by returning the neighbour if it is not null, not owned,
// and not uniquely horizontal, as well as its neighbour in the opposite
// direction is blob.
static BLOBNBOX *MutualUnusedVNeighbour(const BLOBNBOX *blob, BlobNeighbourDir dir) {
  BLOBNBOX *next_blob = blob->neighbour(dir);
  if (next_blob == nullptr || next_blob->owner() != nullptr || next_blob->UniquelyHorizontal()) {
    return nullptr;
  }
  if (next_blob->neighbour(DirOtherWay(dir)) == blob) {
    return next_blob;
  }
  return nullptr;
}

// Finds vertical chains of text-like blobs and puts them in ColPartitions.
void StrokeWidth::FindVerticalTextChains(ColPartitionGrid *part_grid) {
  // A PageSegMode that forces vertical textlines with the current rotation.
  PageSegMode pageseg_mode =
      rerotation_.y() == 0.0f ? PSM_SINGLE_BLOCK_VERT_TEXT : PSM_SINGLE_COLUMN;
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    // Only process boxes that have no horizontal hope and have not yet
    // been included in a chain.
    BLOBNBOX *blob;
    if (bbox->owner() == nullptr && bbox->UniquelyVertical() &&
        (blob = MutualUnusedVNeighbour(bbox, BND_ABOVE)) != nullptr) {
      // Put all the linked blobs into a ColPartition.
      auto *part = new ColPartition(BRT_VERT_TEXT, ICOORD(0, 1));
      part->AddBox(bbox);
      while (blob != nullptr) {
        part->AddBox(blob);
        blob = MutualUnusedVNeighbour(blob, BND_ABOVE);
      }
      blob = MutualUnusedVNeighbour(bbox, BND_BELOW);
      while (blob != nullptr) {
        part->AddBox(blob);
        blob = MutualUnusedVNeighbour(blob, BND_BELOW);
      }
      CompletePartition(pageseg_mode, part, part_grid);
    }
  }
}

// Helper verifies that blob's neighbour in direction dir is good to add to a
// horizontal text chain by returning the neighbour if it is not null, not
// owned, and not uniquely vertical, as well as its neighbour in the opposite
// direction is blob.
static BLOBNBOX *MutualUnusedHNeighbour(const BLOBNBOX *blob, BlobNeighbourDir dir) {
  BLOBNBOX *next_blob = blob->neighbour(dir);
  if (next_blob == nullptr || next_blob->owner() != nullptr || next_blob->UniquelyVertical()) {
    return nullptr;
  }
  if (next_blob->neighbour(DirOtherWay(dir)) == blob) {
    return next_blob;
  }
  return nullptr;
}

// Finds horizontal chains of text-like blobs and puts them in ColPartitions.
void StrokeWidth::FindHorizontalTextChains(ColPartitionGrid *part_grid) {
  // A PageSegMode that forces horizontal textlines with the current rotation.
  PageSegMode pageseg_mode =
      rerotation_.y() == 0.0f ? PSM_SINGLE_COLUMN : PSM_SINGLE_BLOCK_VERT_TEXT;
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    BLOBNBOX *blob;
    if (bbox->owner() == nullptr && bbox->UniquelyHorizontal() &&
        (blob = MutualUnusedHNeighbour(bbox, BND_RIGHT)) != nullptr) {
      // Put all the linked blobs into a ColPartition.
      auto *part = new ColPartition(BRT_TEXT, ICOORD(0, 1));
      part->AddBox(bbox);
      while (blob != nullptr) {
        part->AddBox(blob);
        blob = MutualUnusedHNeighbour(blob, BND_RIGHT);
      }
      blob = MutualUnusedHNeighbour(bbox, BND_LEFT);
      while (blob != nullptr) {
        part->AddBox(blob);
        blob = MutualUnusedVNeighbour(blob, BND_LEFT);
      }
      CompletePartition(pageseg_mode, part, part_grid);
    }
  }
}

// Finds diacritics and saves their base character in the blob.
// The objective is to move all diacritics to the noise_blobs list, so
// they don't mess up early textline finding/merging, or force splits
// on textlines that overlap a bit. Blobs that become diacritics must be
// either part of no ColPartition (nullptr owner) or in a small partition in
// which ALL the blobs are diacritics, in which case the partition is
// exploded (deleted) back to its blobs.
void StrokeWidth::TestDiacritics(ColPartitionGrid *part_grid, TO_BLOCK *block) {
  BlobGrid small_grid(gridsize(), bleft(), tright());
  small_grid.InsertBlobList(&block->noise_blobs);
  small_grid.InsertBlobList(&block->blobs);
  int medium_diacritics = 0;
  int small_diacritics = 0;
  BLOBNBOX_IT small_it(&block->noise_blobs);
  for (small_it.mark_cycle_pt(); !small_it.cycled_list(); small_it.forward()) {
    BLOBNBOX *blob = small_it.data();
    if (blob->owner() == nullptr && !blob->IsDiacritic() && DiacriticBlob(&small_grid, blob)) {
      ++small_diacritics;
    }
  }
  BLOBNBOX_IT blob_it(&block->blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    if (blob->IsDiacritic()) {
      small_it.add_to_end(blob_it.extract());
      continue; // Already a diacritic.
    }
    ColPartition *part = blob->owner();
    if (part == nullptr && DiacriticBlob(&small_grid, blob)) {
      ++medium_diacritics;
      RemoveBBox(blob);
      small_it.add_to_end(blob_it.extract());
    } else if (part != nullptr && !part->block_owned() && part->boxes_count() < 3) {
      // We allow blobs in small partitions to become diacritics if ALL the
      // blobs in the partition qualify as we can then cleanly delete the
      // partition, turn all the blobs in it to diacritics and they can be
      // merged into the base character partition more easily than merging
      // the partitions.
      BLOBNBOX_C_IT box_it(part->boxes());
      for (box_it.mark_cycle_pt();
           !box_it.cycled_list() && DiacriticBlob(&small_grid, box_it.data()); box_it.forward()) {
        ;
      }
      if (box_it.cycled_list()) {
        // They are all good.
        while (!box_it.empty()) {
          // Liberate the blob from its partition so it can be treated
          // as a diacritic and merged explicitly with the base part.
          // The blob is really owned by the block. The partition "owner"
          // is nulled to allow the blob to get merged with its base character
          // partition.
          BLOBNBOX *box = box_it.extract();
          box->set_owner(nullptr);
          box_it.forward();
          ++medium_diacritics;
          // We remove the blob from the grid so it isn't found by subsequent
          // searches where we might not want to include diacritics.
          RemoveBBox(box);
        }
        // We only move the one blob to the small list here, but the others
        // all get moved by the test at the top of the loop.
        small_it.add_to_end(blob_it.extract());
        part_grid->RemoveBBox(part);
        delete part;
      }
    } else if (AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(),
                                             blob->bounding_box().bottom())) {
      tprintf("Blob not available to be a diacritic at:");
      blob->bounding_box().print();
    }
  }
  if (textord_tabfind_show_strokewidths) {
    tprintf("Found %d small diacritics, %d medium\n", small_diacritics, medium_diacritics);
  }
}

// Searches this grid for an appropriately close and sized neighbour of the
// given [small] blob. If such a blob is found, the diacritic base is saved
// in the blob and true is returned.
// The small_grid is a secondary grid that contains the small/noise objects
// that are not in this grid, but may be useful for determining a connection
// between blob and its potential base character. (See DiacriticXGapFilled.)
bool StrokeWidth::DiacriticBlob(BlobGrid *small_grid, BLOBNBOX *blob) {
  if (BLOBNBOX::UnMergeableType(blob->region_type()) || blob->region_type() == BRT_VERT_TEXT) {
    return false;
  }
  TBOX small_box(blob->bounding_box());
  bool debug = AlignedBlob::WithinTestRegion(2, small_box.left(), small_box.bottom());
  if (debug) {
    tprintf("Testing blob for diacriticness at:");
    small_box.print();
  }
  int x = (small_box.left() + small_box.right()) / 2;
  int y = (small_box.bottom() + small_box.top()) / 2;
  int grid_x, grid_y;
  GridCoords(x, y, &grid_x, &grid_y);
  int height = small_box.height();
  // Setup a rectangle search to find its nearest base-character neighbour.
  // We keep 2 different best candidates:
  // best_x_overlap is a category of base characters that have an overlap in x
  // (like a acute) in which we look for the least y-gap, computed using the
  // projection to favor base characters in the same textline.
  // best_y_overlap is a category of base characters that have no x overlap,
  // (nominally a y-overlap is preferrecd but not essential) in which we
  // look for the least weighted sum of x-gap and y-gap, with x-gap getting
  // a lower weight to catch quotes at the end of a textline.
  // NOTE that x-gap and y-gap are measured from the nearest side of the base
  // character to the FARTHEST side of the diacritic to allow small diacritics
  // to be a reasonable distance away, but not big diacritics.
  BLOBNBOX *best_x_overlap = nullptr;
  BLOBNBOX *best_y_overlap = nullptr;
  int best_total_dist = 0;
  int best_y_gap = 0;
  TBOX best_xbox;
  // TODO(rays) the search box could be setup using the projection as a guide.
  TBOX search_box(small_box);
  int x_pad = IntCastRounded(gridsize() * kDiacriticXPadRatio);
  int y_pad = IntCastRounded(gridsize() * kDiacriticYPadRatio);
  search_box.pad(x_pad, y_pad);
  BlobGridSearch rsearch(this);
  rsearch.SetUniqueMode(true);
  int min_height = height * kMinDiacriticSizeRatio;
  rsearch.StartRectSearch(search_box);
  BLOBNBOX *neighbour;
  while ((neighbour = rsearch.NextRectSearch()) != nullptr) {
    if (BLOBNBOX::UnMergeableType(neighbour->region_type()) || neighbour == blob ||
        neighbour->owner() == blob->owner()) {
      continue;
    }
    TBOX nbox = neighbour->bounding_box();
    if (neighbour->owner() == nullptr || neighbour->owner()->IsVerticalType() ||
        (neighbour->flow() != BTFT_CHAIN && neighbour->flow() != BTFT_STRONG_CHAIN)) {
      if (debug) {
        tprintf("Neighbour not strong enough:");
        nbox.print();
      }
      continue; // Diacritics must be attached to strong text.
    }
    if (nbox.height() < min_height) {
      if (debug) {
        tprintf("Neighbour not big enough:");
        nbox.print();
      }
      continue; // Too small to be the base character.
    }
    int x_gap = small_box.x_gap(nbox);
    int y_gap = small_box.y_gap(nbox);
    int total_distance = projection_->DistanceOfBoxFromBox(small_box, nbox, true, denorm_, debug);
    if (debug) {
      tprintf("xgap=%d, y=%d, total dist=%d\n", x_gap, y_gap, total_distance);
    }
    if (total_distance > neighbour->owner()->median_height() * kMaxDiacriticDistanceRatio) {
      if (debug) {
        tprintf("Neighbour with median size %d too far away:", neighbour->owner()->median_height());
        neighbour->bounding_box().print();
      }
      continue; // Diacritics must not be too distant.
    }
    if (x_gap <= 0) {
      if (debug) {
        tprintf("Computing reduced box for :");
        nbox.print();
      }
      int left = small_box.left() - small_box.width();
      int right = small_box.right() + small_box.width();
      nbox = neighbour->BoundsWithinLimits(left, right);
      y_gap = small_box.y_gap(nbox);
      if (best_x_overlap == nullptr || y_gap < best_y_gap) {
        best_x_overlap = neighbour;
        best_xbox = nbox;
        best_y_gap = y_gap;
        if (debug) {
          tprintf("New best:");
          nbox.print();
        }
      } else if (debug) {
        tprintf("Shrunken box doesn't win:");
        nbox.print();
      }
    } else if (blob->ConfirmNoTabViolation(*neighbour)) {
      if (best_y_overlap == nullptr || total_distance < best_total_dist) {
        if (debug) {
          tprintf("New best y overlap:");
          nbox.print();
        }
        best_y_overlap = neighbour;
        best_total_dist = total_distance;
      } else if (debug) {
        tprintf("New y overlap box doesn't win:");
        nbox.print();
      }
    } else if (debug) {
      tprintf("Neighbour wrong side of a tab:");
      nbox.print();
    }
  }
  if (best_x_overlap != nullptr &&
      (best_y_overlap == nullptr || best_xbox.major_y_overlap(best_y_overlap->bounding_box()))) {
    blob->set_diacritic_box(best_xbox);
    blob->set_base_char_blob(best_x_overlap);
    if (debug) {
      tprintf("DiacriticBlob OK! (x-overlap:");
      small_box.print();
      best_xbox.print();
    }
    return true;
  }
  if (best_y_overlap != nullptr &&
      DiacriticXGapFilled(small_grid, small_box, best_y_overlap->bounding_box()) &&
      NoNoiseInBetween(small_box, best_y_overlap->bounding_box())) {
    blob->set_diacritic_box(best_y_overlap->bounding_box());
    blob->set_base_char_blob(best_y_overlap);
    if (debug) {
      tprintf("DiacriticBlob OK! (y-overlap:");
      small_box.print();
      best_y_overlap->bounding_box().print();
    }
    return true;
  }
  if (debug) {
    tprintf("DiacriticBlob fails:");
    small_box.print();
    tprintf("Best x+y gap = %d, y = %d\n", best_total_dist, best_y_gap);
    if (best_y_overlap != nullptr) {
      tprintf("XGapFilled=%d, NoiseBetween=%d\n",
              DiacriticXGapFilled(small_grid, small_box, best_y_overlap->bounding_box()),
              NoNoiseInBetween(small_box, best_y_overlap->bounding_box()));
    }
  }
  return false;
}

// Returns true if there is no gap between the base char and the diacritic
// bigger than a fraction of the height of the base char:
// Eg: line end.....'
// The quote is a long way from the end of the line, yet it needs to be a
// diacritic. To determine that the quote is not part of an image, or
// a different text block, we check for other marks in the gap between
// the base char and the diacritic.
//                          '<--Diacritic
// |---------|
// |         |<-toobig-gap->
// | Base    |<ok gap>
// |---------|        x<-----Dot occupying gap
// The grid is const really.
bool StrokeWidth::DiacriticXGapFilled(BlobGrid *grid, const TBOX &diacritic_box,
                                      const TBOX &base_box) {
  // Since most gaps are small, use an iterative algorithm to search the gap.
  int max_gap = IntCastRounded(base_box.height() * kMaxDiacriticGapToBaseCharHeight);
  TBOX occupied_box(base_box);
  int diacritic_gap;
  while ((diacritic_gap = diacritic_box.x_gap(occupied_box)) > max_gap) {
    TBOX search_box(occupied_box);
    if (diacritic_box.left() > search_box.right()) {
      // We are looking right.
      search_box.set_left(search_box.right());
      search_box.set_right(search_box.left() + max_gap);
    } else {
      // We are looking left.
      search_box.set_right(search_box.left());
      search_box.set_left(search_box.left() - max_gap);
    }
    BlobGridSearch rsearch(grid);
    rsearch.StartRectSearch(search_box);
    BLOBNBOX *neighbour;
    while ((neighbour = rsearch.NextRectSearch()) != nullptr) {
      const TBOX &nbox = neighbour->bounding_box();
      if (nbox.x_gap(diacritic_box) < diacritic_gap) {
        if (nbox.left() < occupied_box.left()) {
          occupied_box.set_left(nbox.left());
        }
        if (nbox.right() > occupied_box.right()) {
          occupied_box.set_right(nbox.right());
        }
        break;
      }
    }
    if (neighbour == nullptr) {
      return false; // Found a big gap.
    }
  }
  return true; // The gap was filled.
}

// Merges diacritics with the ColPartition of the base character blob.
void StrokeWidth::MergeDiacritics(TO_BLOCK *block, ColPartitionGrid *part_grid) {
  BLOBNBOX_IT small_it(&block->noise_blobs);
  for (small_it.mark_cycle_pt(); !small_it.cycled_list(); small_it.forward()) {
    BLOBNBOX *blob = small_it.data();
    if (blob->base_char_blob() != nullptr) {
      ColPartition *part = blob->base_char_blob()->owner();
      // The base character must be owned by a partition and that partition
      // must not be on the big_parts list (not block owned).
      if (part != nullptr && !part->block_owned() && blob->owner() == nullptr &&
          blob->IsDiacritic()) {
        // The partition has to be removed from the grid and reinserted
        // because its bounding box may change.
        part_grid->RemoveBBox(part);
        part->AddBox(blob);
        blob->set_region_type(part->blob_type());
        blob->set_flow(part->flow());
        blob->set_owner(part);
        part_grid->InsertBBox(true, true, part);
      }
      // Set all base chars to nullptr before any blobs get deleted.
      blob->set_base_char_blob(nullptr);
    }
  }
}

// Any blobs on the large_blobs list of block that are still unowned by a
// ColPartition, are probably drop-cap or vertically touching so the blobs
// are removed to the big_parts list and treated separately.
void StrokeWidth::RemoveLargeUnusedBlobs(TO_BLOCK *block, ColPartitionGrid *part_grid,
                                         ColPartition_LIST *big_parts) {
  BLOBNBOX_IT large_it(&block->large_blobs);
  for (large_it.mark_cycle_pt(); !large_it.cycled_list(); large_it.forward()) {
    BLOBNBOX *blob = large_it.data();
    ColPartition *big_part = blob->owner();
    if (big_part == nullptr) {
      // Large blobs should have gone into partitions by now if they are
      // genuine characters, so move any unowned ones out to the big parts
      // list. This will include drop caps and vertically touching characters.
      ColPartition::MakeBigPartition(blob, big_parts);
    }
  }
}

// All remaining unused blobs are put in individual ColPartitions.
void StrokeWidth::PartitionRemainingBlobs(PageSegMode pageseg_mode, ColPartitionGrid *part_grid) {
  BlobGridSearch gsearch(this);
  BLOBNBOX *bbox;
  int prev_grid_x = -1;
  int prev_grid_y = -1;
  BLOBNBOX_CLIST cell_list;
  BLOBNBOX_C_IT cell_it(&cell_list);
  bool cell_all_noise = true;
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    int grid_x = gsearch.GridX();
    int grid_y = gsearch.GridY();
    if (grid_x != prev_grid_x || grid_y != prev_grid_y) {
      // New cell. Process old cell.
      MakePartitionsFromCellList(pageseg_mode, cell_all_noise, part_grid, &cell_list);
      cell_it.set_to_list(&cell_list);
      prev_grid_x = grid_x;
      prev_grid_y = grid_y;
      cell_all_noise = true;
    }
    if (bbox->owner() == nullptr) {
      cell_it.add_to_end(bbox);
      if (bbox->flow() != BTFT_NONTEXT) {
        cell_all_noise = false;
      }
    } else {
      cell_all_noise = false;
    }
  }
  MakePartitionsFromCellList(pageseg_mode, cell_all_noise, part_grid, &cell_list);
}

// If combine, put all blobs in the cell_list into a single partition, otherwise
// put each one into its own partition.
void StrokeWidth::MakePartitionsFromCellList(PageSegMode pageseg_mode, bool combine,
                                             ColPartitionGrid *part_grid,
                                             BLOBNBOX_CLIST *cell_list) {
  if (cell_list->empty()) {
    return;
  }
  BLOBNBOX_C_IT cell_it(cell_list);
  if (combine) {
    BLOBNBOX *bbox = cell_it.extract();
    auto *part = new ColPartition(bbox->region_type(), ICOORD(0, 1));
    part->AddBox(bbox);
    part->set_flow(bbox->flow());
    for (cell_it.forward(); !cell_it.empty(); cell_it.forward()) {
      part->AddBox(cell_it.extract());
    }
    CompletePartition(pageseg_mode, part, part_grid);
  } else {
    for (; !cell_it.empty(); cell_it.forward()) {
      BLOBNBOX *bbox = cell_it.extract();
      auto *part = new ColPartition(bbox->region_type(), ICOORD(0, 1));
      part->set_flow(bbox->flow());
      part->AddBox(bbox);
      CompletePartition(pageseg_mode, part, part_grid);
    }
  }
}

// Helper function to finish setting up a ColPartition and insert into
// part_grid.
void StrokeWidth::CompletePartition(PageSegMode pageseg_mode, ColPartition *part,
                                    ColPartitionGrid *part_grid) {
  part->ComputeLimits();
  TBOX box = part->bounding_box();
  bool debug = AlignedBlob::WithinTestRegion(2, box.left(), box.bottom());
  int value = projection_->EvaluateColPartition(*part, denorm_, debug);
  // Override value if pageseg_mode disagrees.
  if (value > 0 && FindingVerticalOnly(pageseg_mode)) {
    value = part->boxes_count() == 1 ? 0 : -2;
  } else if (value < 0 && FindingHorizontalOnly(pageseg_mode)) {
    value = part->boxes_count() == 1 ? 0 : 2;
  }
  part->SetRegionAndFlowTypesFromProjectionValue(value);
  part->ClaimBoxes();
  part_grid->InsertBBox(true, true, part);
}

// Merge partitions where the merge appears harmless.
// As this
void StrokeWidth::EasyMerges(ColPartitionGrid *part_grid) {
  using namespace std::placeholders; // for _1, _2
  part_grid->Merges(std::bind(&StrokeWidth::OrientationSearchBox, this, _1, _2),
                    std::bind(&StrokeWidth::ConfirmEasyMerge, this, _1, _2));
}

// Compute a search box based on the orientation of the partition.
// Returns true if a suitable box can be calculated.
// Callback for EasyMerges.
bool StrokeWidth::OrientationSearchBox(ColPartition *part, TBOX *box) {
  if (part->IsVerticalType()) {
    box->set_top(box->top() + box->width());
    box->set_bottom(box->bottom() - box->width());
  } else {
    box->set_left(box->left() - box->height());
    box->set_right(box->right() + box->height());
  }
  return true;
}

// Merge confirmation callback for EasyMerges.
bool StrokeWidth::ConfirmEasyMerge(const ColPartition *p1, const ColPartition *p2) {
  ASSERT_HOST(p1 != nullptr && p2 != nullptr);
  ASSERT_HOST(!p1->IsEmpty() && !p2->IsEmpty());
  if ((p1->flow() == BTFT_NONTEXT && p2->flow() >= BTFT_CHAIN) ||
      (p1->flow() >= BTFT_CHAIN && p2->flow() == BTFT_NONTEXT)) {
    return false; // Don't merge confirmed image with text.
  }
  if ((p1->IsVerticalType() || p2->IsVerticalType()) && p1->HCoreOverlap(*p2) <= 0 &&
      ((!p1->IsSingleton() && !p2->IsSingleton()) ||
       !p1->bounding_box().major_overlap(p2->bounding_box()))) {
    return false; // Overlap must be in the text line.
  }
  if ((p1->IsHorizontalType() || p2->IsHorizontalType()) && p1->VCoreOverlap(*p2) <= 0 &&
      ((!p1->IsSingleton() && !p2->IsSingleton()) ||
       (!p1->bounding_box().major_overlap(p2->bounding_box()) &&
        !p1->OKDiacriticMerge(*p2, false) && !p2->OKDiacriticMerge(*p1, false)))) {
    return false; // Overlap must be in the text line.
  }
  if (!p1->ConfirmNoTabViolation(*p2)) {
    return false;
  }
  if (p1->flow() <= BTFT_NONTEXT && p2->flow() <= BTFT_NONTEXT) {
    return true;
  }
  return NoNoiseInBetween(p1->bounding_box(), p2->bounding_box());
}

// Returns true if there is no significant noise in between the boxes.
bool StrokeWidth::NoNoiseInBetween(const TBOX &box1, const TBOX &box2) const {
  return ImageFind::BlankImageInBetween(box1, box2, grid_box_, rerotation_, nontext_map_);
}

#ifndef GRAPHICS_DISABLED

/** Displays the blobs colored according to the number of good neighbours
 * and the vertical/horizontal flow.
 */
ScrollView *StrokeWidth::DisplayGoodBlobs(const char *window_name, int x, int y) {
  auto window = MakeWindow(x, y, window_name);
  // For every blob in the grid, display it.
  window->Brush(ScrollView::NONE);

  // For every bbox in the grid, display it.
  BlobGridSearch gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX *bbox;
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    const TBOX &box = bbox->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    int goodness = bbox->GoodTextBlob();
    BlobRegionType blob_type = bbox->region_type();
    if (bbox->UniquelyVertical()) {
      blob_type = BRT_VERT_TEXT;
    }
    if (bbox->UniquelyHorizontal()) {
      blob_type = BRT_TEXT;
    }
    BlobTextFlowType flow = bbox->flow();
    if (flow == BTFT_NONE) {
      if (goodness == 0) {
        flow = BTFT_NEIGHBOURS;
      } else if (goodness == 1) {
        flow = BTFT_CHAIN;
      } else {
        flow = BTFT_STRONG_CHAIN;
      }
    }
    window->Pen(BLOBNBOX::TextlineColor(blob_type, flow));
    window->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  window->Update();
  return window;
}

static void DrawDiacriticJoiner(const BLOBNBOX *blob, ScrollView *window) {
  const TBOX &blob_box(blob->bounding_box());
  int top = std::max(static_cast<int>(blob_box.top()), blob->base_char_top());
  int bottom = std::min(static_cast<int>(blob_box.bottom()), blob->base_char_bottom());
  int x = (blob_box.left() + blob_box.right()) / 2;
  window->Line(x, top, x, bottom);
}

// Displays blobs colored according to whether or not they are diacritics.
ScrollView *StrokeWidth::DisplayDiacritics(const char *window_name, int x, int y, TO_BLOCK *block) {
  auto window = MakeWindow(x, y, window_name);
  // For every blob in the grid, display it.
  window->Brush(ScrollView::NONE);

  BLOBNBOX_IT it(&block->blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    if (blob->IsDiacritic()) {
      window->Pen(ScrollView::GREEN);
      DrawDiacriticJoiner(blob, window);
    } else {
      window->Pen(blob->BoxColor());
    }
    const TBOX &box = blob->bounding_box();
    window->Rectangle(box.left(), box.bottom(), box.right(), box.top());
  }
  it.set_to_list(&block->noise_blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    if (blob->IsDiacritic()) {
      window->Pen(ScrollView::GREEN);
      DrawDiacriticJoiner(blob, window);
    } else {
      window->Pen(ScrollView::WHITE);
    }
    const TBOX &box = blob->bounding_box();
    window->Rectangle(box.left(), box.bottom(), box.right(), box.top());
  }
  window->Update();
  return window;
}

#endif // !GRAPHICS_DISABLED

} // namespace tesseract.
