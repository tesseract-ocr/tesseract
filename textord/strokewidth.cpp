///////////////////////////////////////////////////////////////////////
// File:        strokewidth.cpp
// Description: Subclass of BBGrid to find uniformity of strokewidth.
// Author:      Ray Smith
// Created:     Mon Mar 31 16:17:01 PST 2008
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "strokewidth.h"
#include "blobbox.h"
#include "colpartition.h"
#include "colpartitiongrid.h"
#include "statistc.h"
#include "tabfind.h"
#include "tordmain.h"  // For SetBlobStrokeWidth.

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

namespace tesseract {

INT_VAR(textord_tabfind_show_strokewidths, 0, "Show stroke widths");
BOOL_VAR(textord_tabfind_only_strokewidths, false, "Only run stroke widths");
double_VAR(textord_strokewidth_minsize, 0.25,
           "Min multiple of linesize for medium-sized blobs");
double_VAR(textord_strokewidth_maxsize, 4.0,
           "Max multiple of linesize for medium-sized blobs");
BOOL_VAR(textord_tabfind_vertical_text, true, "Enable vertical detection");
BOOL_VAR(textord_tabfind_force_vertical_text, false,
         "Force using vertical text page mode");
BOOL_VAR(textord_tabfind_vertical_horizontal_mix, true,
         "find horizontal lines such as headers in vertical page mode");
double_VAR(textord_tabfind_vertical_text_ratio, 0.5,
           "Fraction of textlines deemed vertical to use vertical page mode");

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
// Min multiple of diacritic height that a neighbour must be to be a
// convincing base character.
const int kMinDiacriticSizeRatio = 2;
// Radius of a search for diacritics in grid units.
const int kSearchRadius = 2;
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
// Max number of neighbour small objects per squared gridsize before a grid
// cell becomes image.
const double kMaxSmallNeighboursPerPix = 3.0 / 128;
// Aspect ratio filter for OSD.
const float kSizeRatioToReject = 2.0;
/** Maximum height in inches of the largest possible text. */
const double kMaxTextSize = 2.0;

StrokeWidth::StrokeWidth(int gridsize,
                         const ICOORD& bleft, const ICOORD& tright)
  : BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>(gridsize, bleft, tright),
    noise_density_(NULL) {
  leaders_win_ = NULL;
  widths_win_ = NULL;
  initial_widths_win_ = NULL;
}

StrokeWidth::~StrokeWidth() {
  delete noise_density_;
  if (widths_win_ != NULL) {
    delete widths_win_->AwaitEvent(SVET_DESTROY);
    if (textord_tabfind_only_strokewidths)
      exit(0);
    delete widths_win_;
  }
  delete leaders_win_;
  delete initial_widths_win_;
}

// Types all the blobs as vertical or horizontal text or unknown and
// returns true if the majority are vertical.
// If the blobs are rotated, it is necessary to call CorrectForRotation
// after rotating everything, otherwise the work done here will be enough.
// If cjk_merge is true, it will attempt to merge broken cjk characters.
// If osd_blobs is not null, a list of blobs from the dominant textline
// direction are returned for use in orientation and script detection.
bool StrokeWidth::TestVerticalTextDirection(bool cjk_merge, TO_BLOCK* block,
                                            TabFind* line_grid,
                                            BLOBNBOX_CLIST* osd_blobs) {
  // Reset all blobs to initial state and filter by size.
  ReFilterBlobs(block);
  // Compute the noise density in the grid.
  ComputeNoiseDensity(block, line_grid);
  // Setup the grid with the remaining blobs
  InsertBlobs(block, line_grid);
  // Repair broken CJK characters if needed.
  if (cjk_merge)
    FixBrokenCJK(&block->blobs, line_grid);
  if (textord_tabfind_force_vertical_text) return true;
  // Grade blobs by inspection of neighbours.
  FindTextlineFlowDirection(false);
  if (!textord_tabfind_vertical_text) return false;

  int vertical_boxes = 0;
  int horizontal_boxes = 0;
  // Count vertical bboxes in the grid.
  BlobGridSearch gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* blob;
  BLOBNBOX_CLIST vertical_blobs;
  BLOBNBOX_CLIST horizontal_blobs;
  BLOBNBOX_CLIST nondescript_blobs;
  BLOBNBOX_C_IT v_it(&vertical_blobs);
  BLOBNBOX_C_IT h_it(&horizontal_blobs);
  BLOBNBOX_C_IT n_it(&nondescript_blobs);
  while ((blob = gsearch.NextFullSearch()) != NULL) {
    const TBOX& box = blob->bounding_box();
    float y_x = static_cast<float>(box.height()) / box.width();
    float x_y = 1.0f / y_x;
    // Select a >= 1.0 ratio
    float ratio = x_y > y_x ? x_y : y_x;
    // If the aspect ratio is small and we want them for osd, save the blob.
    bool ok_blob = ratio <= kSizeRatioToReject && osd_blobs != NULL;
    if (blob->UniquelyVertical()) {
      ++vertical_boxes;
      if (ok_blob) v_it.add_after_then_move(blob);
    } else if (blob->UniquelyHorizontal()) {
      ++horizontal_boxes;
      if (ok_blob) h_it.add_after_then_move(blob);
    } else if (ok_blob) {
      n_it.add_after_then_move(blob);
    }
  }
  if (textord_debug_tabfind)
    tprintf("TextDir hbox=%d vs vbox=%d, %dH, %dV, %dN osd blobs\n",
            horizontal_boxes, vertical_boxes,
            horizontal_blobs.length(), vertical_blobs.length(),
            nondescript_blobs.length());
  if (osd_blobs != NULL && vertical_boxes == 0 && horizontal_boxes == 0) {
    // Only nondescript blobs available, so return those.
    BLOBNBOX_C_IT osd_it(osd_blobs);
    osd_it.add_list_after(&nondescript_blobs);
    return false;
  }
  int min_vert_boxes = static_cast<int>((vertical_boxes + horizontal_boxes) *
                                        textord_tabfind_vertical_text_ratio);
  if (vertical_boxes >= min_vert_boxes) {
    if (osd_blobs != NULL) {
      BLOBNBOX_C_IT osd_it(osd_blobs);
      osd_it.add_list_after(&vertical_blobs);
    }
    return true;
  } else {
    if (osd_blobs != NULL) {
      BLOBNBOX_C_IT osd_it(osd_blobs);
      osd_it.add_list_after(&horizontal_blobs);
    }
    return false;
  }
}

// Corrects the data structures for the given rotation.
void StrokeWidth::CorrectForRotation(const FCOORD& rotation, TO_BLOCK* block) {
  noise_density_->Rotate(rotation);
  Init(noise_density_->gridsize(), noise_density_->bleft(),
       noise_density_->tright());
  // Reset all blobs to initial state and filter by size.
  // Since they have rotated, the list they belong on could have changed.
  ReFilterBlobs(block);
}

// Finds leader partitions and inserts them into the give grid.
void StrokeWidth::FindLeaderPartitions(TO_BLOCK* block, TabFind* line_grid) {
  Clear();
  // Find and isolate leaders in the noise list.
  ColPartition_LIST leader_parts;
  FindLeadersAndMarkNoise(true, block, line_grid, &leader_parts);
  // Setup the grid with the remaining blobs
  InsertBlobs(block, line_grid);
  // Mark blobs that have leader neighbours.
  for (ColPartition_IT it(&leader_parts); !it.empty(); it.forward()) {
    ColPartition* part = it.extract();
    MarkLeaderNeighbours(part, true);
    MarkLeaderNeighbours(part, false);
    delete part;
  }
}

static void PrintBoxWidths(BLOBNBOX* neighbour) {
  TBOX nbox = neighbour->bounding_box();
  tprintf("Box (%d,%d)->(%d,%d): h-width=%.1f, v-width=%.1f p-width=%1.f\n",
          nbox.left(), nbox.bottom(), nbox.right(), nbox.top(),
          neighbour->horz_stroke_width(), neighbour->vert_stroke_width(),
          2.0 * neighbour->cblob()->area()/neighbour->cblob()->perimeter());
}

/** Handles a click event in a display window. */
void StrokeWidth::HandleClick(int x, int y) {
  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>::HandleClick(x, y);
  // Run a radial search for blobs that overlap.
  BlobGridSearch radsearch(this);
  radsearch.StartRadSearch(x, y, 1);
  BLOBNBOX* neighbour;
  FCOORD click(static_cast<float>(x), static_cast<float>(y));
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (nbox.contains(click) && neighbour->cblob() != NULL) {
      PrintBoxWidths(neighbour);
      if (neighbour->neighbour(BND_LEFT) != NULL)
        PrintBoxWidths(neighbour->neighbour(BND_LEFT));
      if (neighbour->neighbour(BND_RIGHT) != NULL)
        PrintBoxWidths(neighbour->neighbour(BND_RIGHT));
      if (neighbour->neighbour(BND_ABOVE) != NULL)
        PrintBoxWidths(neighbour->neighbour(BND_ABOVE));
      if (neighbour->neighbour(BND_BELOW) != NULL)
        PrintBoxWidths(neighbour->neighbour(BND_BELOW));
      int gaps[BND_COUNT];
      neighbour->NeighbourGaps(gaps);
      tprintf("Left gap=%d, right=%d, above=%d, below=%d, horz=%d, vert=%d\n"
              "Good=    %d        %d        %d        %d\n",
              gaps[BND_LEFT], gaps[BND_RIGHT],
              gaps[BND_ABOVE], gaps[BND_BELOW],
              neighbour->horz_possible(),
              neighbour->vert_possible(),
              neighbour->good_stroke_neighbour(BND_LEFT),
              neighbour->good_stroke_neighbour(BND_RIGHT),
              neighbour->good_stroke_neighbour(BND_ABOVE),
              neighbour->good_stroke_neighbour(BND_BELOW));
      break;
    }
  }
}

// Helper function to divide the input blobs over noise, small, medium
// and large lists. Blobs small in height and (small in width or large in width)
// go in the noise list. Dash (-) candidates go in the small list, and
// medium and large are by height.
// SIDE-EFFECT: reset all blobs to initial state by calling Init().
static void SizeFilterBlobs(int min_height, int max_height,
                            BLOBNBOX_LIST* src_list,
                            BLOBNBOX_LIST* noise_list,
                            BLOBNBOX_LIST* small_list,
                            BLOBNBOX_LIST* medium_list,
                            BLOBNBOX_LIST* large_list) {
  BLOBNBOX_IT noise_it(noise_list);
  BLOBNBOX_IT small_it(small_list);
  BLOBNBOX_IT medium_it(medium_list);
  BLOBNBOX_IT large_it(large_list);
  for (BLOBNBOX_IT src_it(src_list); !src_it.empty(); src_it.forward()) {
    BLOBNBOX* blob = src_it.extract();
    blob->ReInit();
    int width = blob->bounding_box().width();
    int height = blob->bounding_box().height();
    if (height < min_height  &&
        (width < min_height || width > max_height))
      noise_it.add_after_then_move(blob);
    else if (height > max_height)
      large_it.add_after_then_move(blob);
    else if (height < min_height)
      small_it.add_after_then_move(blob);
    else
      medium_it.add_after_then_move(blob);
  }
}

// Reorganize the blob lists with a different definition of small, medium
// and large, compared to the original definition.
// Height is still the primary filter key, but medium width blobs of small
// height become small, and very wide blobs of small height stay noise, along
// with small dot-shaped blobs.
void StrokeWidth::ReFilterBlobs(TO_BLOCK* block) {
  int min_height =
    static_cast<int>(textord_strokewidth_minsize * block->line_size + 0.5);
  int max_height =
    static_cast<int>(textord_strokewidth_maxsize * block->line_size + 0.5);
  BLOBNBOX_LIST noise_list;
  BLOBNBOX_LIST small_list;
  BLOBNBOX_LIST medium_list;
  BLOBNBOX_LIST large_list;
  SizeFilterBlobs(min_height, max_height, &block->blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &block->large_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &block->small_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &block->noise_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  BLOBNBOX_IT blob_it(&block->blobs);
  blob_it.add_list_after(&medium_list);
  blob_it.set_to_list(&block->large_blobs);
  blob_it.add_list_after(&large_list);
  blob_it.set_to_list(&block->small_blobs);
  blob_it.add_list_after(&small_list);
  blob_it.set_to_list(&block->noise_blobs);
  blob_it.add_list_after(&noise_list);
}

// Computes the noise_density_ by summing the number of elements in a
// neighbourhood of each grid cell.
void StrokeWidth::ComputeNoiseDensity(TO_BLOCK* block, TabFind* line_grid) {
  // Run a preliminary strokewidth neighbour detection on the medium blobs.
  line_grid->InsertBlobList(true, true, false, &block->blobs, false, this);
  BLOBNBOX_IT blob_it(&block->blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    SetNeighbours(false, blob_it.data());
  }
  // Remove blobs with a good strokewidth neighbour from the grid.
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->GoodTextBlob() > 0)
      RemoveBBox(blob);
    blob->ClearNeighbours();
  }
  // Insert the smaller blobs into the grid.
  line_grid->InsertBlobList(true, true, false, &block->small_blobs,
                            false, this);
  line_grid->InsertBlobList(true, true, false, &block->noise_blobs,
                            false, this);
  if (noise_density_ != NULL)
    delete noise_density_;
  IntGrid* cell_counts = CountCellElements();
  noise_density_ = cell_counts->NeighbourhoodSum();
  delete cell_counts;
  // Clear the grid as we don't want the small stuff hanging around in it.
  Clear();
}

// Detects and marks leader dots/dashes.
//    Leaders are horizontal chains of small or noise blobs that look
//    monospace according to ColPartition::MarkAsLeaderIfMonospaced().
// Detected leaders become the only occupants of small_blobs list.
// Non-leader small blobs get moved to the blobs list.
// Non-leader noise blobs remain singletons in the noise list.
// All small and noise blobs in high density regions are marked BTFT_NONTEXT.
void StrokeWidth::FindLeadersAndMarkNoise(bool final, TO_BLOCK* block,
                                          TabFind* line_grid,
                                          ColPartition_LIST* leader_parts) {
  line_grid->InsertBlobList(true, true, false, &block->small_blobs,
                            false, this);
  line_grid->InsertBlobList(true, true, false, &block->noise_blobs,
                            false, this);
  int max_noise_count =
      static_cast<int>(kMaxSmallNeighboursPerPix * gridsize() * gridsize());
  BlobGridSearch gsearch(this);
  BLOBNBOX* bbox;
  // For every bbox in the grid, set its neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    int noise_count = noise_density_->GridCellValue(gsearch.GridX(),
                                                    gsearch.GridY());
    if (noise_count <= max_noise_count) {
      SetNeighbours(true, bbox);
    } else {
      bbox->set_flow(BTFT_NONTEXT);
    }
  }
  ColPartition_IT part_it(leader_parts);
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    if (bbox->flow() == BTFT_NONE) {
      if (bbox->neighbour(BND_RIGHT) == NULL &&
          bbox->neighbour(BND_LEFT) == NULL)
        continue;
      // Put all the linked blobs into a ColPartition.
      ColPartition* part = new ColPartition(BRT_UNKNOWN, ICOORD(0, 1));
      BLOBNBOX* blob;
      for (blob = bbox; blob != NULL && blob->flow() == BTFT_NONE;
           blob = blob->neighbour(BND_RIGHT))
        part->AddBox(blob);
      for (blob = bbox->neighbour(BND_LEFT); blob != NULL &&
           blob->flow() == BTFT_NONE;
           blob = blob->neighbour(BND_LEFT))
        part->AddBox(blob);
      if (part->MarkAsLeaderIfMonospaced())
        part_it.add_after_then_move(part);
      else
        delete part;
    }
  }
  if (textord_tabfind_show_strokewidths && final) {
    leaders_win_ = DisplayGoodBlobs("LeaderNeighbours", 0, 0);
  }
  // Move any non-leaders from the small to the blobs list, as they are
  // most likely dashes or broken characters.
  BLOBNBOX_IT blob_it(&block->blobs);
  BLOBNBOX_IT small_it(&block->small_blobs);
  for (small_it.mark_cycle_pt(); !small_it.cycled_list(); small_it.forward()) {
    BLOBNBOX* blob = small_it.data();
    if (blob->flow() != BTFT_LEADER) {
      if (blob->flow() == BTFT_NEIGHBOURS)
        blob->set_flow(BTFT_NONE);
      blob->ClearNeighbours();
      blob_it.add_to_end(small_it.extract());
    }
  }
  // Move leaders from the noise list to the small list, leaving the small
  // list exclusively leaders, so they don't get processed further,
  // and the remaining small blobs all in the noise list.
  BLOBNBOX_IT noise_it(&block->noise_blobs);
  for (noise_it.mark_cycle_pt(); !noise_it.cycled_list(); noise_it.forward()) {
    BLOBNBOX* blob = noise_it.data();
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

/** Puts the block blobs (normal and large) into the grid. */
void StrokeWidth::InsertBlobs(TO_BLOCK* block, TabFind* line_grid) {
  // Insert the blobs into this grid using the separator lines in line_grid.
  line_grid->InsertBlobList(true, true, false, &block->blobs, false, this);
  line_grid->InsertBlobList(true, true, true, &block->large_blobs,
                            false, this);
}

// Sets the leader_on_left or leader_on_right flags for blobs
// that are next to one end of the given leader partition.
// If left_of_part is true, then look at the left side of the partition for
// blobs on which to set the leader_on_right flag.
void StrokeWidth::MarkLeaderNeighbours(const ColPartition* part,
                                       bool left_of_part) {
  const TBOX& part_box = part->bounding_box();
  BlobGridSearch blobsearch(this);
  // Search to the side of the leader for the nearest neighbour.
  BLOBNBOX* best_blob = NULL;
  int best_gap = 0;
  blobsearch.StartSideSearch(left_of_part ? part_box.left() : part_box.right(),
                             part_box.bottom(), part_box.top());
  BLOBNBOX* blob;
  while ((blob = blobsearch.NextSideSearch(left_of_part)) != NULL) {
    const TBOX& blob_box = blob->bounding_box();
    if (!blob_box.y_overlap(part_box))
      continue;
    int x_gap = blob_box.x_gap(part_box);
    if (x_gap > 2 * gridsize()) {
      break;
    } else if (best_blob == NULL || x_gap < best_gap) {
      best_blob = blob;
      best_gap = x_gap;
    }
  }
  if (best_blob != NULL) {
    if (left_of_part)
      best_blob->set_leader_on_right(true);
    else
      best_blob->set_leader_on_left(true);
    if (leaders_win_ != NULL) {
      leaders_win_->Pen(left_of_part ? ScrollView::RED : ScrollView::GREEN);
      const TBOX& blob_box = best_blob->bounding_box();
      leaders_win_->Rectangle(blob_box.left(), blob_box.bottom(),
                              blob_box.right(), blob_box.top());
    }
  }
}

// Helper to compute the UQ of the square-ish CJK charcters.
static int UpperQuartileCJKSize(int gridsize, BLOBNBOX_LIST* blobs) {
  STATS sizes(0, gridsize * kMaxCJKSizeRatio);
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    int width = blob->bounding_box().width();
    int height = blob->bounding_box().height();
    if (width <= height * kCJKAspectRatio && height < width * kCJKAspectRatio)
      sizes.add(height, 1);
  }
  return static_cast<int>(sizes.ile(0.75f) + 0.5);
}

// Fix broken CJK characters, using the fake joined blobs mechanism.
// Blobs are really merged, ie the master takes all the outlines and the
// others are deleted.
void StrokeWidth::FixBrokenCJK(BLOBNBOX_LIST* blobs, TabFind* line_grid) {
  int median_height = UpperQuartileCJKSize(gridsize(), blobs);
  int max_dist = static_cast<int>(median_height * kCJKBrokenDistanceFraction);
  int max_size = static_cast<int>(median_height * kCJKAspectRatio);
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->cblob() == NULL || blob->cblob()->out_list()->empty())
      continue;
    TBOX bbox = blob->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(3, bbox.left(),
                                               bbox.bottom());
    if (debug) {
      tprintf("Checking for Broken CJK (max size=%d):", max_size);
      bbox.print();
    }
    // Generate a list of blobs that overlap or are near enough to merge.
    BLOBNBOX_CLIST overlapped_blobs;
    AccumulateOverlaps(blob, debug, max_size, max_dist,
                       &bbox, &overlapped_blobs);
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
        if (debug)
          tprintf("Too many neighbours: %d\n", overlapped_blobs.length());
        continue;
      }
      // The strokewidths must match amongst the join candidates.
      BLOBNBOX_C_IT n_it(&overlapped_blobs);
      for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
        BLOBNBOX* neighbour = NULL;
        neighbour = n_it.data();
        if (!blob->MatchingStrokeWidth(*neighbour, kStrokeWidthFractionCJK,
                                       kStrokeWidthCJK))
          break;
      }
      if (!n_it.cycled_list()) {
        if (debug) {
          tprintf("Bad stroke widths:");
          PrintBoxWidths(blob);
        }
        continue;  // Not good enough.
      }

      // Merge all the candidates into blob.
      // We must remove blob from the grid and reinsert it after merging
      // to maintain the integrity of the grid.
      RemoveBBox(blob);
      // Everything else will be calculated later.
      for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
        BLOBNBOX* neighbour = n_it.data();
        RemoveBBox(neighbour);
        blob->really_merge(neighbour);
      }
      line_grid->InsertBlob(true, true, false, blob, this);
      if (debug) {
        tprintf("Done! Final box:");
        bbox.print();
      }
    }
  }
  // Permanently delete all the empty shell blobs that contain no outlines.
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->cblob() == NULL || blob->cblob()->out_list()->empty()) {
      if (blob->cblob() != NULL)
        delete blob->cblob();
      delete blob_it.extract();
    }
  }
}

// Helper function to determine whether it is reasonable to merge the
// bbox and the nbox for repairing broken CJK.
// The distance apart must not exceed max_dist, the combined size must
// not exceed max_size, and the aspect ratio must either improve or at
// least not get worse by much.
static bool AcceptableCJKMerge(const TBOX& bbox, const TBOX& nbox,
                               bool debug, int max_size, int max_dist,
                               int* x_gap, int* y_gap) {
  *x_gap = bbox.x_gap(nbox);
  *y_gap = bbox.y_gap(nbox);
  TBOX merged(nbox);
  merged += bbox;
  if (debug) {
    tprintf("gaps = %d, %d, merged_box:", *x_gap, *y_gap);
    merged.print();
  }
  if (*x_gap <= max_dist && *y_gap <= max_dist &&
      merged.width() <= max_size && merged.height() <= max_size) {
    // Close enough to call overlapping. Check aspect ratios.
    double old_ratio = static_cast<double>(bbox.width()) / bbox.height();
    if (old_ratio < 1.0) old_ratio = 1.0 / old_ratio;
    double new_ratio = static_cast<double>(merged.width()) / merged.height();
    if (new_ratio < 1.0) new_ratio = 1.0 / new_ratio;
    if (new_ratio <= old_ratio * kCJKAspectRatioIncrease)
      return true;
  }
  return false;
}

// Collect blobs that overlap or are within max_dist of the input bbox.
// Return them in the list of blobs and expand the bbox to be the union
// of all the boxes. not_this is excluded from the search, as are blobs
// that cause the merged box to exceed max_size in either dimension.
void StrokeWidth::AccumulateOverlaps(const BLOBNBOX* not_this, bool debug,
                                     int max_size, int max_dist,
                                     TBOX* bbox, BLOBNBOX_CLIST* blobs) {
  // While searching, nearests holds the nearest failed blob in each
  // direction. When we have a nearest in each of the 4 directions, then
  // the search is over, and at this point the final bbox must not overlap
  // any of the nearests.
  BLOBNBOX* nearests[BND_COUNT];
  for (int i = 0; i < BND_COUNT; ++i) {
    nearests[i] = NULL;
  }
  int x = (bbox->left() + bbox->right()) / 2;
  int y = (bbox->bottom() + bbox->top()) / 2;
  // Run a radial search for blobs that overlap or are sufficiently close.
  BlobGridSearch radsearch(this);
  radsearch.StartRadSearch(x, y, kCJKRadius);
  BLOBNBOX* neighbour;
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    if (neighbour == not_this) continue;
    TBOX nbox = neighbour->bounding_box();
    int x_gap, y_gap;
    if (AcceptableCJKMerge(*bbox, nbox, debug, max_size, max_dist,
                           &x_gap, &y_gap)) {
      // Close enough to call overlapping. Merge boxes.
      *bbox += nbox;
      blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, neighbour);
      if (debug) {
        tprintf("Added:");
        nbox.print();
      }
      // Since we merged, search the nearests, as some might now me mergeable.
      for (int dir = 0; dir < BND_COUNT; ++dir) {
        if (nearests[dir] == NULL) continue;
        nbox = nearests[dir]->bounding_box();
        if (AcceptableCJKMerge(*bbox, nbox, debug, max_size,
                               max_dist, &x_gap, &y_gap)) {
          // Close enough to call overlapping. Merge boxes.
          *bbox += nbox;
          blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, nearests[dir]);
          if (debug) {
            tprintf("Added:");
            nbox.print();
          }
          nearests[dir] = NULL;
          dir = -1;  // Restart the search.
        }
      }
    } else if (x_gap < 0 && x_gap <= y_gap) {
      // A vertical neighbour. Record the nearest.
      BlobNeighbourDir dir = nbox.top() > bbox->top() ? BND_ABOVE : BND_BELOW;
      if (nearests[dir] == NULL ||
          y_gap < bbox->y_gap(nearests[dir]->bounding_box())) {
        nearests[dir] = neighbour;
      }
    } else if (y_gap < 0 && y_gap <= x_gap) {
      // A horizontal neighbour. Record the nearest.
      BlobNeighbourDir dir = nbox.left() > bbox->left() ? BND_RIGHT : BND_LEFT;
      if (nearests[dir] == NULL ||
          x_gap < bbox->x_gap(nearests[dir]->bounding_box())) {
        nearests[dir] = neighbour;
      }
    }
    // If all nearests are non-null, then we have finished.
    if (nearests[BND_LEFT] && nearests[BND_RIGHT] &&
        nearests[BND_ABOVE] && nearests[BND_BELOW])
      break;
  }
  // Final overlap with a nearest is not allowed.
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    if (nearests[dir] == NULL) continue;
    const TBOX& nbox = nearests[dir]->bounding_box();
    if (debug) {
      tprintf("Testing for overlap with:");
      nbox.print();
    }
    if (bbox->overlap(nbox)) {
      blobs->shallow_clear();
      if (debug)
        tprintf("Final box overlaps nearest\n");
      return;
    }
  }
}

// Finds the textline direction to be horizontal or vertical according
// to distance to neighbours and 1st and 2nd order neighbours.
// Non-text tends to end up without a definite direction.
void StrokeWidth::FindTextlineFlowDirection(bool final) {
  int max_noise_count =
      static_cast<int>(kMaxSmallNeighboursPerPix * gridsize() * gridsize());
  BlobGridSearch gsearch(this);
  BLOBNBOX* bbox;
  // For every bbox in the grid, set its neighbours, unless in a noisy area.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    int noise_count = noise_density_->GridCellValue(gsearch.GridX(),
                                                    gsearch.GridY());
    if (noise_count <= max_noise_count) {
      SetNeighbours(false, bbox);
    } else {
      // The noise density is so high, that it must be non-text.
      bbox->set_flow(BTFT_NONTEXT);
    }
  }
  // Where vertical or horizontal wins by a big margin, clarify it.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SimplifyObviousNeighbours(bbox);
  }
  // Now try to make the blobs only vertical or horizontal using neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SetNeighbourFlows(bbox);
  }
  if ((textord_tabfind_show_strokewidths  && final) ||
      textord_tabfind_show_strokewidths > 1) {
    initial_widths_win_ = DisplayGoodBlobs("InitialStrokewidths", 400, 0);
  }
  // Improve flow direction with neighbours.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SmoothNeighbourTypes(bbox, false);
  }
  // Now allow reset of firm values to fix renegades.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SmoothNeighbourTypes(bbox, true);
  }
  // Repeat.
  gsearch.StartFullSearch();
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SmoothNeighbourTypes(bbox, true);
  }
  if ((textord_tabfind_show_strokewidths  && final) ||
      textord_tabfind_show_strokewidths > 1) {
    widths_win_ = DisplayGoodBlobs("ImprovedStrokewidths", 800, 0);
  }
}

// Sets the neighbours and good_stroke_neighbours members of the blob by
// searching close on all 4 sides.
// When finding leader dots/dashes, there is a slightly different rule for
// what makes a good neighbour.
void StrokeWidth::SetNeighbours(bool leaders, BLOBNBOX* blob) {
  int line_trap_count = 0;
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    BlobNeighbourDir bnd = static_cast<BlobNeighbourDir>(dir);
    line_trap_count += FindGoodNeighbour(bnd, leaders, blob);
  }
  if (line_trap_count > 0) {
    // It looks like a line so isolate it by clearing its neighbours.
    blob->ClearNeighbours();
    const TBOX& box = blob->bounding_box();
    blob->set_region_type(box.width() > box.height() ? BRT_HLINE : BRT_VLINE);
  }
}


// Sets the good_stroke_neighbours member of the blob if it has a
// GoodNeighbour on the given side.
// Also sets the neighbour in the blob, whether or not a good one is found.
// Returns the number of blobs in the nearby search area that would lead us to
// believe that this blob is a line separator.
// Leaders get extra special lenient treatment.
int StrokeWidth::FindGoodNeighbour(BlobNeighbourDir dir, bool leaders,
                                   BLOBNBOX* blob) {
  // Search for neighbours that overlap vertically.
  TBOX blob_box = blob->bounding_box();
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
  int line_trap_max = MAX(width, height) / kLineTrapLongest;
  int line_trap_min = MIN(width, height) * kLineTrapShortest;
  int line_trap_count = 0;

  int min_good_overlap = (dir == BND_LEFT || dir == BND_RIGHT)
                       ? height / 2 : width / 2;
  int min_decent_overlap = (dir == BND_LEFT || dir == BND_RIGHT)
                       ? height / 3 : width / 3;
  if (leaders)
    min_good_overlap = min_decent_overlap = 1;

  int search_pad = static_cast<int>(sqrt(static_cast<double>(width * height)));
  if (gridsize() > search_pad)
    search_pad = gridsize();
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
  BLOBNBOX* best_neighbour = NULL;
  double best_goodness = 0.0;
  bool best_is_good = false;
  BLOBNBOX* neighbour;
  while ((neighbour = rectsearch.NextRectSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (neighbour == blob)
      continue;
    int mid_x = (nbox.left() + nbox.right()) / 2;
    if (mid_x < blob->left_rule() || mid_x > blob->right_rule())
      continue;  // In a different column.

    // Last-minute line detector. There is a small upper limit to the line
    // width accepted by the morphological line detector.
    int n_width = nbox.width();
    int n_height = nbox.height();
    if (MIN(n_width, n_height) > line_trap_min &&
        MAX(n_width, n_height) < line_trap_max)
      ++line_trap_count;
    if (TabFind::VeryDifferentSizes(MAX(n_width, n_height),
                                    MAX(width, height)))
      continue;  // Could be a different font size or non-text.
    // Amount of vertical overlap between the blobs.
    int overlap;
    // If the overlap is along the short side of the neighbour, and it
    // is fully overlapped, then perp_overlap holds the length of the long
    // side of the neighbour. A measure to include hyphens and dashes as
    // legitimate neighbours.
    int perp_overlap;
    int gap;
    if (dir == BND_LEFT || dir == BND_RIGHT) {
      overlap = MIN(nbox.top(), top) - MAX(nbox.bottom(), bottom);
      if (overlap == nbox.height() && nbox.width() > nbox.height())
        perp_overlap = nbox.width();
      else
        perp_overlap = overlap;
      gap = dir == BND_LEFT ? left - nbox.left() : nbox.right() - right;
      if (gap <= 0)
        continue;  // On the wrong side.
      gap -= n_width;
    } else {
      overlap = MIN(nbox.right(), right) - MAX(nbox.left(), left);
      if (overlap == nbox.width() && nbox.height() > nbox.width())
        perp_overlap = nbox.height();
      else
        perp_overlap = overlap;
      gap = dir == BND_BELOW ? bottom - nbox.bottom() : nbox.top() - top;
      if (gap <= 0)
        continue;  // On the wrong side.
      gap -= n_height;
    }
    if (-gap > overlap)
      continue;  // Overlaps the wrong way.
    if (perp_overlap < min_decent_overlap)
      continue;  // Doesn't overlap enough.
    bool bad_sizes = TabFind::DifferentSizes(height, n_height) &&
                     TabFind::DifferentSizes(width, n_width);
    bool is_good = overlap >= min_good_overlap && !bad_sizes &&
                   blob->MatchingStrokeWidth(*neighbour,
                                             kStrokeWidthFractionTolerance,
                                             kStrokeWidthTolerance);
    // Best is a fuzzy combination of gap, overlap and is good.
    // Basically if you make one thing twice as good without making
    // anything else twice as bad, then it is better.
    if (gap < 1) gap = 1;
    double goodness = (1.0 + is_good) * overlap / gap;
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
static void ListNeighbours(const BLOBNBOX* blob,
                           BLOBNBOX_CLIST* neighbours) {
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    BlobNeighbourDir bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX* neighbour = blob->neighbour(bnd);
    if (neighbour != NULL) {
      neighbours->add_sorted(SortByBoxLeft<BLOBNBOX>, true, neighbour);
    }
  }
}

// Helper to get a list of 1st and 2nd order neighbours.
static void List2ndNeighbours(const BLOBNBOX* blob,
                              BLOBNBOX_CLIST* neighbours) {
  ListNeighbours(blob, neighbours);
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    BlobNeighbourDir bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX* neighbour = blob->neighbour(bnd);
    if (neighbour != NULL) {
      ListNeighbours(neighbour, neighbours);
    }
  }
}

// Helper to get a list of 1st, 2nd and 3rd order neighbours.
static void List3rdNeighbours(const BLOBNBOX* blob,
                              BLOBNBOX_CLIST* neighbours) {
  List2ndNeighbours(blob, neighbours);
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    BlobNeighbourDir bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX* neighbour = blob->neighbour(bnd);
    if (neighbour != NULL) {
      List2ndNeighbours(neighbour, neighbours);
    }
  }
}

// Helper to count the evidence for verticalness or horizontalness
// in a list of neighbours.
static void CountNeighbourGaps(bool debug, BLOBNBOX_CLIST* neighbours,
                               int* pure_h_count, int* pure_v_count) {
  if (neighbours->length() <= kMostlyOneDirRatio)
    return;
  BLOBNBOX_C_IT it(neighbours);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    int h_min, h_max, v_min, v_max;
    blob->MinMaxGapsClipped(&h_min, &h_max, &v_min, &v_max);
    if (debug)
      tprintf("Hgaps [%d,%d], vgaps [%d,%d]:", h_min, h_max, v_min, v_max);
    if (h_max < v_min ||
        blob->leader_on_left() || blob->leader_on_right()) {
      // Horizontal gaps are clear winners. Count a pure horizontal.
      ++*pure_h_count;
      if (debug) tprintf("Horz at:");
    } else if (v_max < h_min) {
      // Vertical gaps are clear winners. Clear a pure vertical.
      ++*pure_v_count;
      if (debug) tprintf("Vert at:");
    } else {
      if (debug) tprintf("Neither at:");
    }
    if (debug)
      blob->bounding_box().print();
  }
}

// Makes the blob to be only horizontal or vertical where evidence
// is clear based on gaps of 2nd order neighbours, or definite individual
// blobs.
void StrokeWidth::SetNeighbourFlows(BLOBNBOX* blob) {
  if (blob->DefiniteIndividualFlow())
    return;
  bool debug = AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(),
                                             blob->bounding_box().bottom());
  if (debug) {
    tprintf("SetNeighbourFLows on:");
    blob->bounding_box().print();
  }
  BLOBNBOX_CLIST neighbours;
  List3rdNeighbours(blob, &neighbours);
  // The number of pure horizontal and vertical neighbours.
  int pure_h_count = 0;
  int pure_v_count = 0;
  CountNeighbourGaps(debug, &neighbours, &pure_h_count, &pure_v_count);
  if (debug) {
    HandleClick(blob->bounding_box().left() + 1,
                blob->bounding_box().bottom() + 1);
    tprintf("SetFlows: h_count=%d, v_count=%d\n",
            pure_h_count, pure_v_count);
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
static void CountNeighbourTypes(BLOBNBOX_CLIST* neighbours,
                                int* pure_h_count, int* pure_v_count) {
  BLOBNBOX_C_IT it(neighbours);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    if (blob->UniquelyHorizontal())
      ++*pure_h_count;
    if (blob->UniquelyVertical())
      ++*pure_v_count;
  }
}

// Nullify the neighbours in the wrong directions where the direction
// is clear-cut based on a distance margin. Good for isolating vertical
// text from neighbouring horizontal text.
void StrokeWidth::SimplifyObviousNeighbours(BLOBNBOX* blob) {
  // Case 1: We have text that is likely several characters, blurry and joined
  //         together.
  if ((blob->bounding_box().width() > 3 * blob->area_stroke_width() &&
       blob->bounding_box().height() > 3 * blob->area_stroke_width())) {
    // The blob is complex (not stick-like).
    if (blob->bounding_box().width() > 4 * blob->bounding_box().height()) {
      // Horizontal conjoined text.
      blob->set_neighbour(BND_ABOVE, NULL, false);
      blob->set_neighbour(BND_BELOW, NULL, false);
      return;
    }
    if (blob->bounding_box().height() > 4 * blob->bounding_box().width()) {
      // Vertical conjoined text.
      blob->set_neighbour(BND_LEFT, NULL, false);
      blob->set_neighbour(BND_RIGHT, NULL, false);
      return;
    }
  }

  // Case 2: This blob is likely a single character.
  int margin = gridsize() / 2;
  int h_min, h_max, v_min, v_max;
  blob->MinMaxGapsClipped(&h_min, &h_max, &v_min, &v_max);
  if ((h_max + margin < v_min && h_max < margin / 2) ||
      blob->leader_on_left() || blob->leader_on_right()) {
    // Horizontal gaps are clear winners. Clear vertical neighbours.
    blob->set_neighbour(BND_ABOVE, NULL, false);
    blob->set_neighbour(BND_BELOW, NULL, false);
  } else if (v_max + margin < h_min && v_max < margin / 2) {
    // Vertical gaps are clear winners. Clear horizontal neighbours.
    blob->set_neighbour(BND_LEFT, NULL, false);
    blob->set_neighbour(BND_RIGHT, NULL, false);
  }
}

// Smoothes the vertical/horizontal type of the blob based on the
// 2nd-order neighbours. If reset_all is true, then all blobs are
// changed. Otherwise, only ambiguous blobs are processed.
void StrokeWidth::SmoothNeighbourTypes(BLOBNBOX* blob, bool reset_all) {
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
      HandleClick(blob->bounding_box().left() + 1,
                  blob->bounding_box().bottom() + 1);
      tprintf("pure_h=%d, pure_v=%d\n",
              pure_h_count, pure_v_count);
    }
    if (pure_h_count > pure_v_count) {
      // Horizontal gaps are clear winners. Clear vertical neighbours.
      blob->set_vert_possible(false);
      blob->set_horz_possible(true);
    } else if (pure_v_count > pure_h_count) {
      // Vertical gaps are clear winners. Clear horizontal neighbours.
      blob->set_horz_possible(false);
      blob->set_vert_possible(true);
    }
  } else if (AlignedBlob::WithinTestRegion(2, blob->bounding_box().left(),
                                    blob->bounding_box().bottom())) {
    HandleClick(blob->bounding_box().left() + 1,
                blob->bounding_box().bottom() + 1);
    tprintf("Clean on pass 3!\n");
  }
}

// Puts the block blobs (normal and large) into the grid.
void StrokeWidth::InsertBlobsOld(TO_BLOCK* block, TabFind* line_grid) {
  // Insert the blobs into this grid using the separator lines in line_grid.
  line_grid->InsertBlobList(true, false, false, &block->blobs, false, this);
  line_grid->InsertBlobList(true, false, true, &block->large_blobs,
                            false, this);
}

/**
 * Moves the large blobs that have good stroke-width neighbours to the normal
 * blobs list.
 */
void StrokeWidth::MoveGoodLargeBlobs(int resolution, TO_BLOCK* block) {
  BLOBNBOX_IT large_it = &block->large_blobs;
  BLOBNBOX_IT blob_it = &block->blobs;
  int max_height = static_cast<int>(resolution * kMaxTextSize);
  int b_count = 0;
  for (large_it.mark_cycle_pt(); !large_it.cycled_list(); large_it.forward()) {
    BLOBNBOX* large_blob = large_it.data();
    if (large_blob->bounding_box().height() <= max_height &&
        GoodTextBlob(large_blob)) {
      blob_it.add_to_end(large_it.extract());
      ++b_count;
    }
  }
  if (textord_debug_tabfind) {
    tprintf("Moved %d large blobs to normal list\n",
            b_count);
  }
}

/** Displays the blobs colored according to the number of good neighbours
 * and the vertical/horizontal flow.
 */
ScrollView* StrokeWidth::DisplayGoodBlobs(const char* window_name,
                                          int x, int y) {
  ScrollView* window = NULL;
#ifndef GRAPHICS_DISABLED
  window = MakeWindow(x, y, window_name);
  // For every blob in the grid, display it.
  window->Brush(ScrollView::NONE);

  // For every bbox in the grid, display it.
  BlobGridSearch gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    TBOX box = bbox->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    int goodness = bbox->GoodTextBlob();
    BlobRegionType blob_type = bbox->region_type();
    if (bbox->UniquelyVertical())
      blob_type = BRT_VERT_TEXT;
    if (bbox->UniquelyHorizontal())
      blob_type = BRT_TEXT;
    BlobTextFlowType flow = bbox->flow();
    if (flow == BTFT_NONE) {
      if (goodness == 0)
        flow = BTFT_NEIGHBOURS;
      else if (goodness == 1)
        flow = BTFT_CHAIN;
      else
        flow = BTFT_STRONG_CHAIN;
    }
    window->Pen(BLOBNBOX::TextlineColor(blob_type, flow));
    window->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  window->Update();
#endif
  return window;
}

/**
 * Returns true if there is at least one side neighbour that has a similar
 * stroke width and is not on the other side of a rule line.
 */
bool StrokeWidth::GoodTextBlob(BLOBNBOX* blob) {
  double h_width = blob->horz_stroke_width();
  double v_width = blob->vert_stroke_width();
  // The perimeter-based width is used as a backup in case there is
  // no information in the blob.
  double p_width = 2.0f * blob->cblob()->area();
  p_width /= blob->cblob()->perimeter();
  double h_tolerance = h_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;
  double v_tolerance = v_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;
  double p_tolerance = p_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;

  // Run a radial search for neighbours that overlap.
  TBOX box = blob->bounding_box();
  int radius = box.height() / gridsize_ + 2;
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> radsearch(this);
  radsearch.StartRadSearch((box.left() + box.right()) / 2, box.bottom(),
                           radius);
  int top = box.top();
  int bottom = box.bottom();
  int min_overlap = (top - bottom) / 2;
  BLOBNBOX* neighbour;
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (neighbour == blob) {
      continue;
    }
    // In finding a suitable neighbour, do not cross rule lines.
    if (nbox.right() > blob->right_rule() || nbox.left() < blob->left_rule()) {
      continue;  // Can't use it.
    }
    int overlap = MIN(nbox.top(), top) - MAX(nbox.bottom(), bottom);
    if (overlap >= min_overlap &&
        !TabFind::DifferentSizes(box.height(), nbox.height())) {
      double n_h_width = neighbour->horz_stroke_width();
      double n_v_width = neighbour->vert_stroke_width();
      double n_p_width = 2.0f * neighbour->cblob()->area();
      n_p_width /= neighbour->cblob()->perimeter();
      bool h_zero = h_width == 0.0f || n_h_width == 0.0f;
      bool v_zero = v_width == 0.0f || n_v_width == 0.0f;
      bool h_ok = !h_zero && NearlyEqual(h_width, n_h_width, h_tolerance);
      bool v_ok = !v_zero && NearlyEqual(v_width, n_v_width, v_tolerance);
      bool p_ok = h_zero && v_zero &&
                  NearlyEqual(p_width, n_p_width, p_tolerance);
      // For a match, at least one of the horizontal and vertical widths
      // must match, and the other one must either match or be zero.
      // Only if both are zero will we look at the perimeter metric.
      if (p_ok || ((v_ok || h_ok) && (h_ok || h_zero) && (v_ok || v_zero))) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace tesseract.
