///////////////////////////////////////////////////////////////////////
// File:        colpartitiongrid.cpp
// Description: Class collecting code that acts on a BBGrid of ColPartitions.
// Author:      Ray Smith
//
// (C) Copyright 2009, Google Inc.
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

#include "colpartitiongrid.h"
#include "colpartitionset.h"
#include "imagefind.h"

#include <algorithm>
#include <utility>

namespace tesseract {

// Max pad factor used to search the neighbourhood of a partition to smooth
// partition types.
const int kMaxPadFactor = 6;
// Max multiple of size (min(height, width)) for the distance of the nearest
// neighbour for the change of type to be used.
const int kMaxNeighbourDistFactor = 4;
// Maximum number of lines in a credible figure caption.
const int kMaxCaptionLines = 7;
// Min ratio between biggest and smallest gap to bound a caption.
const double kMinCaptionGapRatio = 2.0;
// Min ratio between biggest gap and mean line height to bound a caption.
const double kMinCaptionGapHeightRatio = 0.5;
// Min fraction of ColPartition height to be overlapping for margin purposes.
const double kMarginOverlapFraction = 0.25;
// Size ratio required to consider an unmerged overlapping partition to be big.
const double kBigPartSizeRatio = 1.75;
// Fraction of gridsize to allow arbitrary overlap between partitions.
const double kTinyEnoughTextlineOverlapFraction = 0.25;
// Max vertical distance of neighbouring ColPartition as a multiple of
// partition height for it to be a partner.
// TODO(rays) fix the problem that causes a larger number to not work well.
// The value needs to be larger as sparse text blocks in a page that gets
// marked as single column will not find adjacent lines as partners, and
// will merge horizontally distant, but aligned lines. See rep.4B3 p5.
// The value needs to be small because double-spaced legal docs written
// in a single column, but justified courier have widely spaced lines
// that need to get merged before they partner-up with the lines above
// and below. See legal.3B5 p13/17. Neither of these should depend on
// the value of kMaxPartitionSpacing to be successful, and ColPartition
// merging needs attention to fix this problem.
const double kMaxPartitionSpacing = 1.75;
// Margin by which text has to beat image or vice-versa to make a firm
// decision in GridSmoothNeighbour.
const int kSmoothDecisionMargin = 4;

ColPartitionGrid::ColPartitionGrid(int gridsize, const ICOORD &bleft,
                                   const ICOORD &tright)
    : BBGrid<ColPartition, ColPartition_CLIST, ColPartition_C_IT>(
          gridsize, bleft, tright) {}

// Handles a click event in a display window.
void ColPartitionGrid::HandleClick(int x, int y) {
  BBGrid<ColPartition, ColPartition_CLIST, ColPartition_C_IT>::HandleClick(x,
                                                                           y);
  // Run a radial search for partitions that overlap.
  ColPartitionGridSearch radsearch(this);
  radsearch.SetUniqueMode(true);
  radsearch.StartRadSearch(x, y, 1);
  ColPartition *neighbour;
  FCOORD click(x, y);
  while ((neighbour = radsearch.NextRadSearch()) != nullptr) {
    const TBOX &nbox = neighbour->bounding_box();
    if (nbox.contains(click)) {
      tprintf("Block box:");
      neighbour->bounding_box().print();
      neighbour->Print();
    }
  }
}

// Merges ColPartitions in the grid that look like they belong in the same
// textline.
// For all partitions in the grid, calls the box_cb permanent callback
// to compute the search box, searches the box, and if a candidate is found,
// calls the confirm_cb to check any more rules. If the confirm_cb returns
// true, then the partitions are merged.
// Both callbacks are deleted before returning.
void ColPartitionGrid::Merges(
    const std::function<bool(ColPartition *, TBOX *)> &box_cb,
    const std::function<bool(const ColPartition *, const ColPartition *)>
        &confirm_cb) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (MergePart(box_cb, confirm_cb, part)) {
      gsearch.RepositionIterator();
    }
  }
}

// For the given partition, calls the box_cb permanent callback
// to compute the search box, searches the box, and if a candidate is found,
// calls the confirm_cb to check any more rules. If the confirm_cb returns
// true, then the partitions are merged.
// Returns true if the partition is consumed by one or more merges.
bool ColPartitionGrid::MergePart(
    const std::function<bool(ColPartition *, TBOX *)> &box_cb,
    const std::function<bool(const ColPartition *, const ColPartition *)>
        &confirm_cb,
    ColPartition *part) {
  if (part->IsUnMergeableType()) {
    return false;
  }
  bool any_done = false;
  // Repeatedly merge part while we find a best merge candidate that works.
  bool merge_done = false;
  do {
    merge_done = false;
    TBOX box = part->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(2, box.left(), box.bottom());
    if (debug) {
      tprintf("Merge candidate:");
      box.print();
    }
    // Set up a rectangle search bounded by the part.
    if (!box_cb(part, &box)) {
      continue;
    }
    // Create a list of merge candidates.
    ColPartition_CLIST merge_candidates;
    FindMergeCandidates(part, box, debug, &merge_candidates);
    // Find the best merge candidate based on minimal overlap increase.
    int overlap_increase;
    ColPartition *neighbour = BestMergeCandidate(part, &merge_candidates, debug,
                                                 confirm_cb, &overlap_increase);
    if (neighbour != nullptr && overlap_increase <= 0) {
      if (debug) {
        tprintf("Merging:hoverlap=%d, voverlap=%d, OLI=%d\n",
                part->HCoreOverlap(*neighbour), part->VCoreOverlap(*neighbour),
                overlap_increase);
      }
      // Looks like a good candidate so merge it.
      RemoveBBox(neighbour);
      // We will modify the box of part, so remove it from the grid, merge
      // it and then re-insert it into the grid.
      RemoveBBox(part);
      part->Absorb(neighbour, nullptr);
      InsertBBox(true, true, part);
      merge_done = true;
      any_done = true;
    } else if (neighbour != nullptr) {
      if (debug) {
        tprintf("Overlapped when merged with increase %d: ", overlap_increase);
        neighbour->bounding_box().print();
      }
    } else if (debug) {
      tprintf("No candidate neighbour returned\n");
    }
  } while (merge_done);
  return any_done;
}

// Returns true if the given part and merge candidate might believably
// be part of a single text line according to the default rules.
// In general we only want to merge partitions that look like they
// are on the same text line, ie their median limits overlap, but we have
// to make exceptions for diacritics and stray punctuation.
static bool OKMergeCandidate(const ColPartition *part,
                             const ColPartition *candidate, bool debug) {
  const TBOX &part_box = part->bounding_box();
  if (candidate == part) {
    return false; // Ignore itself.
  }
  if (!part->TypesMatch(*candidate) || candidate->IsUnMergeableType()) {
    return false; // Don't mix inappropriate types.
  }

  const TBOX &c_box = candidate->bounding_box();
  if (debug) {
    tprintf("Examining merge candidate:");
    c_box.print();
  }
  // Candidates must be within a reasonable distance.
  if (candidate->IsVerticalType() || part->IsVerticalType()) {
    int h_dist = -part->HCoreOverlap(*candidate);
    if (h_dist >= std::max(part_box.width(), c_box.width()) / 2) {
      if (debug) {
        tprintf("Too far away: h_dist = %d\n", h_dist);
      }
      return false;
    }
  } else {
    // Coarse filter by vertical distance between partitions.
    int v_dist = -part->VCoreOverlap(*candidate);
    if (v_dist >= std::max(part_box.height(), c_box.height()) / 2) {
      if (debug) {
        tprintf("Too far away: v_dist = %d\n", v_dist);
      }
      return false;
    }
    // Candidates must either overlap in median y,
    // or part or candidate must be an acceptable diacritic.
    if (!part->VSignificantCoreOverlap(*candidate) &&
        !part->OKDiacriticMerge(*candidate, debug) &&
        !candidate->OKDiacriticMerge(*part, debug)) {
      if (debug) {
        tprintf("Candidate fails overlap and diacritic tests!\n");
      }
      return false;
    }
  }
  return true;
}

// Helper function to compute the increase in overlap of the parts list of
// Colpartitions with the combination of merge1 and merge2, compared to
// the overlap with them uncombined.
// An overlap is not counted if passes the OKMergeOverlap test with ok_overlap
// as the pixel overlap limit. merge1 and merge2 must both be non-nullptr.
static int IncreaseInOverlap(const ColPartition *merge1,
                             const ColPartition *merge2, int ok_overlap,
                             ColPartition_CLIST *parts) {
  ASSERT_HOST(merge1 != nullptr && merge2 != nullptr);
  int total_area = 0;
  ColPartition_C_IT it(parts);
  TBOX merged_box(merge1->bounding_box());
  merged_box += merge2->bounding_box();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    if (part == merge1 || part == merge2) {
      continue;
    }
    TBOX part_box = part->bounding_box();
    // Compute the overlap of the merged box with part.
    int overlap_area = part_box.intersection(merged_box).area();
    if (overlap_area > 0 &&
        !part->OKMergeOverlap(*merge1, *merge2, ok_overlap, false)) {
      total_area += overlap_area;
      // Subtract the overlap of merge1 and merge2 individually.
      overlap_area = part_box.intersection(merge1->bounding_box()).area();
      if (overlap_area > 0) {
        total_area -= overlap_area;
      }
      TBOX intersection_box = part_box.intersection(merge2->bounding_box());
      overlap_area = intersection_box.area();
      if (overlap_area > 0) {
        total_area -= overlap_area;
        // Add back the 3-way area.
        intersection_box &= merge1->bounding_box(); // In-place intersection.
        overlap_area = intersection_box.area();
        if (overlap_area > 0) {
          total_area += overlap_area;
        }
      }
    }
  }
  return total_area;
}

// Helper function to test that each partition in candidates is either a
// good diacritic merge with part or an OK merge candidate with all others
// in the candidates list.
// ASCII Art Scenario:
// We sometimes get text such as "join-this" where the - is actually a long
// dash culled from a standard set of extra characters that don't match the
// font of the text. This makes its strokewidth not match and forms a broken
// set of 3 partitions for "join", "-" and "this" and the dash may slightly
// overlap BOTH words.
// -------  -------
// |     ====     |
// -------  -------
// The standard merge rule: "you can merge 2 partitions as long as there is
// no increase in overlap elsewhere" fails miserably here. Merge any pair
// of partitions and the combined box overlaps more with the third than
// before. To allow the merge, we need to consider whether it is safe to
// merge everything, without merging separate text lines. For that we need
// everything to be an OKMergeCandidate (which is supposed to prevent
// separate text lines merging), but this is hard for diacritics to satisfy,
// so an alternative to being OKMergeCandidate with everything is to be an
// OKDiacriticMerge with part as the base character.
static bool TestCompatibleCandidates(const ColPartition &part, bool debug,
                                     ColPartition_CLIST *candidates) {
  ColPartition_C_IT it(candidates);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *candidate = it.data();
    if (!candidate->OKDiacriticMerge(part, false)) {
      ColPartition_C_IT it2(it);
      for (it2.mark_cycle_pt(); !it2.cycled_list(); it2.forward()) {
        ColPartition *candidate2 = it2.data();
        if (candidate2 != candidate &&
            !OKMergeCandidate(candidate, candidate2, false)) {
          if (debug) {
            tprintf("NC overlap failed:Candidate:");
            candidate2->bounding_box().print();
            tprintf("fails to be a good merge with:");
            candidate->bounding_box().print();
          }
          return false;
        }
      }
    }
  }
  return true;
}

// Computes and returns the total overlap of all partitions in the grid.
// If overlap_grid is non-null, it is filled with a grid that holds empty
// partitions representing the union of all overlapped partitions.
int ColPartitionGrid::ComputeTotalOverlap(ColPartitionGrid **overlap_grid) {
  int total_overlap = 0;
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    ColPartition_CLIST neighbors;
    const TBOX &part_box = part->bounding_box();
    FindOverlappingPartitions(part_box, part, &neighbors);
    ColPartition_C_IT n_it(&neighbors);
    bool any_part_overlap = false;
    for (n_it.mark_cycle_pt(); !n_it.cycled_list(); n_it.forward()) {
      const TBOX &n_box = n_it.data()->bounding_box();
      int overlap = n_box.intersection(part_box).area();
      if (overlap > 0 && overlap_grid != nullptr) {
        if (*overlap_grid == nullptr) {
          *overlap_grid = new ColPartitionGrid(gridsize(), bleft(), tright());
        }
        (*overlap_grid)->InsertBBox(true, true, n_it.data()->ShallowCopy());
        if (!any_part_overlap) {
          (*overlap_grid)->InsertBBox(true, true, part->ShallowCopy());
        }
      }
      any_part_overlap = true;
      total_overlap += overlap;
    }
  }
  return total_overlap;
}

// Finds all the ColPartitions in the grid that overlap with the given
// box and returns them SortByBoxLeft(ed) and uniqued in the given list.
// Any partition equal to not_this (may be nullptr) is excluded.
void ColPartitionGrid::FindOverlappingPartitions(const TBOX &box,
                                                 const ColPartition *not_this,
                                                 ColPartition_CLIST *parts) {
  ColPartitionGridSearch rsearch(this);
  rsearch.StartRectSearch(box);
  ColPartition *part;
  while ((part = rsearch.NextRectSearch()) != nullptr) {
    if (part != not_this) {
      parts->add_sorted(SortByBoxLeft<ColPartition>, true, part);
    }
  }
}

// Finds and returns the best candidate ColPartition to merge with part,
// selected from the candidates list, based on the minimum increase in
// pairwise overlap among all the partitions overlapped by the combined box.
// If overlap_increase is not nullptr then it returns the increase in overlap
// that would result from the merge.
// confirm_cb is a permanent callback that (if non-null) will be used to
// confirm the validity of a proposed merge candidate before selecting it.
//
// ======HOW MERGING WORKS======
// The problem:
// We want to merge all the parts of a textline together, but avoid merging
// separate textlines. Diacritics, i dots, punctuation, and broken characters
// are examples of small bits that need merging with the main textline.
// Drop-caps and descenders in one line that touch ascenders in the one below
// are examples of cases where we don't want to merge.
//
// The solution:
// Merges that increase overlap among other partitions are generally bad.
// Those that don't increase overlap (much) and minimize the total area
// seem to be good.
//
// Ascii art example:
// The text:
// groggy descenders
// minimum ascenders
// The boxes: The === represents a small box near or overlapping the lower box.
// -----------------
// |               |
// -----------------
// -===-------------
// |               |
// -----------------
// In considering what to do with the small === box, we find the 2 larger
// boxes as neighbours and possible merge candidates, but merging with the
// upper box increases overlap with the lower box, whereas merging with the
// lower box does not increase overlap.
// If the small === box didn't overlap either to start with, total area
// would be minimized by merging with the nearer (lower) box.
//
// This is a simple example. In reality, we have to allow some increase
// in overlap, or tightly spaced text would end up in bits.
ColPartition *ColPartitionGrid::BestMergeCandidate(
    const ColPartition *part, ColPartition_CLIST *candidates, bool debug,
    const std::function<bool(const ColPartition *, const ColPartition *)>
        &confirm_cb,
    int *overlap_increase) {
  if (overlap_increase != nullptr) {
    *overlap_increase = 0;
  }
  if (candidates->empty()) {
    return nullptr;
  }
  int ok_overlap =
      static_cast<int>(kTinyEnoughTextlineOverlapFraction * gridsize() + 0.5);
  // The best neighbour to merge with is the one that causes least
  // total pairwise overlap among all the neighbours.
  // If more than one offers the same total overlap, choose the one
  // with the least total area.
  const TBOX &part_box = part->bounding_box();
  ColPartition_C_IT it(candidates);
  ColPartition *best_candidate = nullptr;
  // Find the total combined box of all candidates and the original.
  TBOX full_box(part_box);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *candidate = it.data();
    full_box += candidate->bounding_box();
  }
  // Keep valid neighbours in a list.
  ColPartition_CLIST neighbours;
  // Now run a rect search of the merged box for overlapping neighbours, as
  // we need anything that might be overlapped by the merged box.
  FindOverlappingPartitions(full_box, part, &neighbours);
  if (debug) {
    tprintf("Finding best merge candidate from %d, %d neighbours for box:",
            candidates->length(), neighbours.length());
    part_box.print();
  }
  // If the best increase in overlap is positive, then we also check the
  // worst non-candidate overlap. This catches the case of multiple good
  // candidates that overlap each other when merged. If the worst
  // non-candidate overlap is better than the best overlap, then return
  // the worst non-candidate overlap instead.
  ColPartition_CLIST non_candidate_neighbours;
  non_candidate_neighbours.set_subtract(SortByBoxLeft<ColPartition>, true,
                                        &neighbours, candidates);
  int worst_nc_increase = 0;
  int best_increase = INT32_MAX;
  int best_area = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *candidate = it.data();
    if (confirm_cb != nullptr && !confirm_cb(part, candidate)) {
      if (debug) {
        tprintf("Candidate not confirmed:");
        candidate->bounding_box().print();
      }
      continue;
    }
    int increase = IncreaseInOverlap(part, candidate, ok_overlap, &neighbours);
    const TBOX &cand_box = candidate->bounding_box();
    if (best_candidate == nullptr || increase < best_increase) {
      best_candidate = candidate;
      best_increase = increase;
      best_area = cand_box.bounding_union(part_box).area() - cand_box.area();
      if (debug) {
        tprintf("New best merge candidate has increase %d, area %d, over box:",
                increase, best_area);
        full_box.print();
        candidate->Print();
      }
    } else if (increase == best_increase) {
      int area = cand_box.bounding_union(part_box).area() - cand_box.area();
      if (area < best_area) {
        best_area = area;
        best_candidate = candidate;
      }
    }
    increase = IncreaseInOverlap(part, candidate, ok_overlap,
                                 &non_candidate_neighbours);
    if (increase > worst_nc_increase) {
      worst_nc_increase = increase;
    }
  }
  if (best_increase > 0) {
    // If the worst non-candidate increase is less than the best increase
    // including the candidates, then all the candidates can merge together
    // and the increase in outside overlap would be less, so use that result,
    // but only if each candidate is either a good diacritic merge with part,
    // or an ok merge candidate with all the others.
    // See TestCompatibleCandidates for more explanation and a picture.
    if (worst_nc_increase < best_increase &&
        TestCompatibleCandidates(*part, debug, candidates)) {
      best_increase = worst_nc_increase;
    }
  }
  if (overlap_increase != nullptr) {
    *overlap_increase = best_increase;
  }
  return best_candidate;
}

// Helper to remove the given box from the given partition, put it in its
// own partition, and add to the partition list.
static void RemoveBadBox(BLOBNBOX *box, ColPartition *part,
                         ColPartition_LIST *part_list) {
  part->RemoveBox(box);
  ColPartition::MakeBigPartition(box, part_list);
}

// Split partitions where it reduces overlap between their bounding boxes.
// ColPartitions are after all supposed to be a partitioning of the blobs
// AND of the space on the page!
// Blobs that cause overlaps get removed, put in individual partitions
// and added to the big_parts list. They are most likely characters on
// 2 textlines that touch, or something big like a dropcap.
void ColPartitionGrid::SplitOverlappingPartitions(
    ColPartition_LIST *big_parts) {
  int ok_overlap =
      static_cast<int>(kTinyEnoughTextlineOverlapFraction * gridsize() + 0.5);
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    // Set up a rectangle search bounded by the part.
    const TBOX &box = part->bounding_box();
    ColPartitionGridSearch rsearch(this);
    rsearch.SetUniqueMode(true);
    rsearch.StartRectSearch(box);
    int unresolved_overlaps = 0;

    ColPartition *neighbour;
    while ((neighbour = rsearch.NextRectSearch()) != nullptr) {
      if (neighbour == part) {
        continue;
      }
      const TBOX &neighbour_box = neighbour->bounding_box();
      if (neighbour->OKMergeOverlap(*part, *part, ok_overlap, false) &&
          part->OKMergeOverlap(*neighbour, *neighbour, ok_overlap, false)) {
        continue; // The overlap is OK both ways.
      }

      // If removal of the biggest box from either partition eliminates the
      // overlap, and it is much bigger than the box left behind, then
      // it is either a drop-cap, an inter-line join, or some junk that
      // we don't want anyway, so put it in the big_parts list.
      if (!part->IsSingleton()) {
        BLOBNBOX *excluded = part->BiggestBox();
        TBOX shrunken = part->BoundsWithoutBox(excluded);
        if (!shrunken.overlap(neighbour_box) &&
            excluded->bounding_box().height() >
                kBigPartSizeRatio * shrunken.height()) {
          // Removing the biggest box fixes the overlap, so do it!
          gsearch.RemoveBBox();
          RemoveBadBox(excluded, part, big_parts);
          InsertBBox(true, true, part);
          gsearch.RepositionIterator();
          break;
        }
      } else if (box.contains(neighbour_box)) {
        ++unresolved_overlaps;
        continue; // No amount of splitting will fix it.
      }
      if (!neighbour->IsSingleton()) {
        BLOBNBOX *excluded = neighbour->BiggestBox();
        TBOX shrunken = neighbour->BoundsWithoutBox(excluded);
        if (!shrunken.overlap(box) &&
            excluded->bounding_box().height() >
                kBigPartSizeRatio * shrunken.height()) {
          // Removing the biggest box fixes the overlap, so do it!
          rsearch.RemoveBBox();
          RemoveBadBox(excluded, neighbour, big_parts);
          InsertBBox(true, true, neighbour);
          gsearch.RepositionIterator();
          break;
        }
      }
      int part_overlap_count = part->CountOverlappingBoxes(neighbour_box);
      int neighbour_overlap_count = neighbour->CountOverlappingBoxes(box);
      ColPartition *right_part = nullptr;
      if (neighbour_overlap_count <= part_overlap_count ||
          part->IsSingleton()) {
        // Try to split the neighbour to reduce overlap.
        BLOBNBOX *split_blob = neighbour->OverlapSplitBlob(box);
        if (split_blob != nullptr) {
          rsearch.RemoveBBox();
          right_part = neighbour->SplitAtBlob(split_blob);
          InsertBBox(true, true, neighbour);
          ASSERT_HOST(right_part != nullptr);
        }
      } else {
        // Try to split part to reduce overlap.
        BLOBNBOX *split_blob = part->OverlapSplitBlob(neighbour_box);
        if (split_blob != nullptr) {
          gsearch.RemoveBBox();
          right_part = part->SplitAtBlob(split_blob);
          InsertBBox(true, true, part);
          ASSERT_HOST(right_part != nullptr);
        }
      }
      if (right_part != nullptr) {
        InsertBBox(true, true, right_part);
        gsearch.RepositionIterator();
        rsearch.RepositionIterator();
        break;
      }
    }
    if (unresolved_overlaps > 2 && part->IsSingleton()) {
      // This part is no good so just add to big_parts.
      RemoveBBox(part);
      ColPartition_IT big_it(big_parts);
      part->set_block_owned(true);
      big_it.add_to_end(part);
      gsearch.RepositionIterator();
    }
  }
}

// Filters partitions of source_type by looking at local neighbours.
// Where a majority of neighbours have a text type, the partitions are
// changed to text, where the neighbours have image type, they are changed
// to image, and partitions that have no definite neighbourhood type are
// left unchanged.
// im_box and rerotation are used to map blob coordinates onto the
// nontext_map, which is used to prevent the spread of text neighbourhoods
// into images.
// Returns true if anything was changed.
bool ColPartitionGrid::GridSmoothNeighbours(BlobTextFlowType source_type,
                                            Image nontext_map,
                                            const TBOX &im_box,
                                            const FCOORD &rotation) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  bool any_changed = false;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->flow() != source_type ||
        BLOBNBOX::IsLineType(part->blob_type())) {
      continue;
    }
    const TBOX &box = part->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(2, box.left(), box.bottom());
    if (SmoothRegionType(nontext_map, im_box, rotation, debug, part)) {
      any_changed = true;
    }
  }
  return any_changed;
}

// Reflects the grid and its colpartitions in the y-axis, assuming that
// all blob boxes have already been done.
void ColPartitionGrid::ReflectInYAxis() {
  ColPartition_LIST parts;
  ColPartition_IT part_it(&parts);
  // Iterate the ColPartitions in the grid to extract them.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part_it.add_after_then_move(part);
  }
  ICOORD bot_left(-tright().x(), bleft().y());
  ICOORD top_right(-bleft().x(), tright().y());
  // Reinitializing the grid with reflected coords also clears all the
  // pointers, so parts will now own the ColPartitions. (Briefly).
  Init(gridsize(), bot_left, top_right);
  for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
    part = part_it.extract();
    part->ReflectInYAxis();
    InsertBBox(true, true, part);
  }
}

// Transforms the grid of partitions to the output blocks, putting each
// partition into a separate block. We don't really care about the order,
// as we just want to get as much text as possible without trying to organize
// it into proper blocks or columns.
// TODO(rays) some kind of sort function would be useful and probably better
// than the default here, which is to sort by order of the grid search.
void ColPartitionGrid::ExtractPartitionsAsBlocks(BLOCK_LIST *blocks,
                                                 TO_BLOCK_LIST *to_blocks) {
  TO_BLOCK_IT to_block_it(to_blocks);
  BLOCK_IT block_it(blocks);
  // All partitions will be put on this list and deleted on return.
  ColPartition_LIST parts;
  ColPartition_IT part_it(&parts);
  // Iterate the ColPartitions in the grid to extract them.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part_it.add_after_then_move(part);
    // The partition has to be at least vaguely like text.
    BlobRegionType blob_type = part->blob_type();
    if (BLOBNBOX::IsTextType(blob_type) ||
        (blob_type == BRT_UNKNOWN && part->boxes_count() > 1)) {
      PolyBlockType type =
          blob_type == BRT_VERT_TEXT ? PT_VERTICAL_TEXT : PT_FLOWING_TEXT;
      // Get metrics from the row that will be used for the block.
      TBOX box = part->bounding_box();
      int median_width = part->median_width();
      int median_height = part->median_height();
      // Turn the partition into a TO_ROW.
      TO_ROW *row = part->MakeToRow();
      if (row == nullptr) {
        // This partition is dead.
        part->DeleteBoxes();
        continue;
      }
      auto *block = new BLOCK("", true, 0, 0, box.left(), box.bottom(),
                              box.right(), box.top());
      block->pdblk.set_poly_block(new POLY_BLOCK(box, type));
      auto *to_block = new TO_BLOCK(block);
      TO_ROW_IT row_it(to_block->get_rows());
      row_it.add_after_then_move(row);
      // We haven't differentially rotated vertical and horizontal text at
      // this point, so use width or height as appropriate.
      if (blob_type == BRT_VERT_TEXT) {
        to_block->line_size = static_cast<float>(median_width);
        to_block->line_spacing = static_cast<float>(box.width());
        to_block->max_blob_size = static_cast<float>(box.width() + 1);
      } else {
        to_block->line_size = static_cast<float>(median_height);
        to_block->line_spacing = static_cast<float>(box.height());
        to_block->max_blob_size = static_cast<float>(box.height() + 1);
      }
      if (to_block->line_size == 0) {
        to_block->line_size = 1;
      }
      block_it.add_to_end(block);
      to_block_it.add_to_end(to_block);
    } else {
      // This partition is dead.
      part->DeleteBoxes();
    }
  }
  Clear();
  // Now it is safe to delete the ColPartitions as parts goes out of scope.
}

// Rotates the grid and its colpartitions by the given angle, assuming that
// all blob boxes have already been done.
void ColPartitionGrid::Deskew(const FCOORD &deskew) {
  ColPartition_LIST parts;
  ColPartition_IT part_it(&parts);
  // Iterate the ColPartitions in the grid to extract them.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part_it.add_after_then_move(part);
  }
  // Rebuild the grid to the new size.
  TBOX grid_box(bleft_, tright_);
  grid_box.rotate_large(deskew);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
  // Reinitializing the grid with rotated coords also clears all the
  // pointers, so parts will now own the ColPartitions. (Briefly).
  for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
    part = part_it.extract();
    part->ComputeLimits();
    InsertBBox(true, true, part);
  }
}

// Sets the left and right tabs of the partitions in the grid.
void ColPartitionGrid::SetTabStops(TabFind *tabgrid) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    const TBOX &part_box = part->bounding_box();
    TabVector *left_line = tabgrid->LeftTabForBox(part_box, true, false);
    // If the overlapping line is not a left tab, try for non-overlapping.
    if (left_line != nullptr && !left_line->IsLeftTab()) {
      left_line = tabgrid->LeftTabForBox(part_box, false, false);
    }
    if (left_line != nullptr && left_line->IsLeftTab()) {
      part->SetLeftTab(left_line);
    }
    TabVector *right_line = tabgrid->RightTabForBox(part_box, true, false);
    if (right_line != nullptr && !right_line->IsRightTab()) {
      right_line = tabgrid->RightTabForBox(part_box, false, false);
    }
    if (right_line != nullptr && right_line->IsRightTab()) {
      part->SetRightTab(right_line);
    }
    part->SetColumnGoodness(tabgrid->WidthCB());
  }
}

// Makes the ColPartSets and puts them in the PartSetVector ready
// for finding column bounds. Returns false if no partitions were found.
bool ColPartitionGrid::MakeColPartSets(PartSetVector *part_sets) {
  auto *part_lists = new ColPartition_LIST[gridheight()];
  part_sets->reserve(gridheight());
  // Iterate the ColPartitions in the grid to get parts onto lists for the
  // y bottom of each.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  bool any_parts_found = false;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    BlobRegionType blob_type = part->blob_type();
    if (blob_type != BRT_NOISE &&
        (blob_type != BRT_UNKNOWN || !part->boxes()->singleton())) {
      int grid_x, grid_y;
      const TBOX &part_box = part->bounding_box();
      GridCoords(part_box.left(), part_box.bottom(), &grid_x, &grid_y);
      ColPartition_IT part_it(&part_lists[grid_y]);
      part_it.add_to_end(part);
      any_parts_found = true;
    }
  }
  if (any_parts_found) {
    for (int grid_y = 0; grid_y < gridheight(); ++grid_y) {
      ColPartitionSet *line_set = nullptr;
      if (!part_lists[grid_y].empty()) {
        line_set = new ColPartitionSet(&part_lists[grid_y]);
      }
      part_sets->push_back(line_set);
    }
  }
  delete[] part_lists;
  return any_parts_found;
}

// Makes a single ColPartitionSet consisting of a single ColPartition that
// represents the total horizontal extent of the significant content on the
// page. Used for the single column setting in place of automatic detection.
// Returns nullptr if the page is empty of significant content.
ColPartitionSet *ColPartitionGrid::MakeSingleColumnSet(WidthCallback cb) {
  ColPartition *single_column_part = nullptr;
  // Iterate the ColPartitions in the grid to get parts onto lists for the
  // y bottom of each.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    BlobRegionType blob_type = part->blob_type();
    if (blob_type != BRT_NOISE &&
        (blob_type != BRT_UNKNOWN || !part->boxes()->singleton())) {
      // Consider for single column.
      BlobTextFlowType flow = part->flow();
      if ((blob_type == BRT_TEXT &&
           (flow == BTFT_STRONG_CHAIN || flow == BTFT_CHAIN ||
            flow == BTFT_LEADER || flow == BTFT_TEXT_ON_IMAGE)) ||
          blob_type == BRT_RECTIMAGE || blob_type == BRT_POLYIMAGE) {
        if (single_column_part == nullptr) {
          single_column_part = part->ShallowCopy();
          single_column_part->set_blob_type(BRT_TEXT);
          // Copy the tabs from itself to properly setup the margins.
          single_column_part->CopyLeftTab(*single_column_part, false);
          single_column_part->CopyRightTab(*single_column_part, false);
        } else {
          if (part->left_key() < single_column_part->left_key()) {
            single_column_part->CopyLeftTab(*part, false);
          }
          if (part->right_key() > single_column_part->right_key()) {
            single_column_part->CopyRightTab(*part, false);
          }
        }
      }
    }
  }
  if (single_column_part != nullptr) {
    // Make a ColPartitionSet out of the single_column_part as a candidate
    // for the single column case.
    single_column_part->SetColumnGoodness(cb);
    return new ColPartitionSet(single_column_part);
  }
  return nullptr;
}

// Mark the BLOBNBOXes in each partition as being owned by that partition.
void ColPartitionGrid::ClaimBoxes() {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part->ClaimBoxes();
  }
}

// Retypes all the blobs referenced by the partitions in the grid.
// Image blobs are found and returned in the im_blobs list, as they are not
// owned by the block.
void ColPartitionGrid::ReTypeBlobs(BLOBNBOX_LIST *im_blobs) {
  BLOBNBOX_IT im_blob_it(im_blobs);
  ColPartition_LIST dead_parts;
  ColPartition_IT dead_part_it(&dead_parts);
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    BlobRegionType blob_type = part->blob_type();
    BlobTextFlowType flow = part->flow();
    bool any_blobs_moved = false;
    if (blob_type == BRT_POLYIMAGE || blob_type == BRT_RECTIMAGE) {
      BLOBNBOX_C_IT blob_it(part->boxes());
      for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
        BLOBNBOX *blob = blob_it.data();
        im_blob_it.add_after_then_move(blob);
      }
    } else if (blob_type != BRT_NOISE) {
      // Make sure the blobs are marked with the correct type and flow.
      BLOBNBOX_C_IT blob_it(part->boxes());
      for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
        BLOBNBOX *blob = blob_it.data();
        if (blob->region_type() == BRT_NOISE) {
          // TODO(rays) Deprecated. Change this section to an assert to verify
          // and then delete.
          ASSERT_HOST(blob->cblob()->area() != 0);
          blob->set_owner(nullptr);
          blob_it.extract();
          any_blobs_moved = true;
        } else {
          blob->set_region_type(blob_type);
          if (blob->flow() != BTFT_LEADER) {
            blob->set_flow(flow);
          }
        }
      }
    }
    if (blob_type == BRT_NOISE || part->boxes()->empty()) {
      BLOBNBOX_C_IT blob_it(part->boxes());
      part->DisownBoxes();
      dead_part_it.add_to_end(part);
      gsearch.RemoveBBox();
      for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
        BLOBNBOX *blob = blob_it.data();
        if (blob->cblob()->area() == 0) {
          // Any blob with zero area is a fake image blob and should be deleted.
          delete blob->cblob();
          delete blob;
        }
      }
    } else if (any_blobs_moved) {
      gsearch.RemoveBBox();
      part->ComputeLimits();
      InsertBBox(true, true, part);
      gsearch.RepositionIterator();
    }
  }
}

// The boxes within the partitions have changed (by deskew) so recompute
// the bounds of all the partitions and reinsert them into the grid.
void ColPartitionGrid::RecomputeBounds(int gridsize, const ICOORD &bleft,
                                       const ICOORD &tright,
                                       const ICOORD &vertical) {
  ColPartition_LIST saved_parts;
  ColPartition_IT part_it(&saved_parts);
  // Iterate the ColPartitions in the grid to get parts onto a list.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part_it.add_to_end(part);
  }
  // Reinitialize grid to the new size.
  Init(gridsize, bleft, tright);
  // Recompute the bounds of the parts and put them back in the new grid.
  for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
    part = part_it.extract();
    part->set_vertical(vertical);
    part->ComputeLimits();
    InsertBBox(true, true, part);
  }
}

// Improves the margins of the ColPartitions in the grid by calling
// FindPartitionMargins on each.
// best_columns, which may be nullptr, is an array of pointers indicating the
// column set at each y-coordinate in the grid.
// best_columns is usually the best_columns_ member of ColumnFinder.
void ColPartitionGrid::GridFindMargins(ColPartitionSet **best_columns) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet *columns =
        best_columns != nullptr ? best_columns[gsearch.GridY()] : nullptr;
    FindPartitionMargins(columns, part);
    const TBOX &box = part->bounding_box();
    if (AlignedBlob::WithinTestRegion(2, box.left(), box.bottom())) {
      tprintf("Computed margins for part:");
      part->Print();
    }
  }
}

// Improves the margins of the ColPartitions in the list by calling
// FindPartitionMargins on each.
// best_columns, which may be nullptr, is an array of pointers indicating the
// column set at each y-coordinate in the grid.
// best_columns is usually the best_columns_ member of ColumnFinder.
void ColPartitionGrid::ListFindMargins(ColPartitionSet **best_columns,
                                       ColPartition_LIST *parts) {
  ColPartition_IT part_it(parts);
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition *part = part_it.data();
    ColPartitionSet *columns = nullptr;
    if (best_columns != nullptr) {
      const TBOX &part_box = part->bounding_box();
      // Get the columns from the y grid coord.
      int grid_x, grid_y;
      GridCoords(part_box.left(), part_box.bottom(), &grid_x, &grid_y);
      columns = best_columns[grid_y];
    }
    FindPartitionMargins(columns, part);
  }
}

// Deletes all the partitions in the grid after disowning all the blobs.
void ColPartitionGrid::DeleteParts() {
  ColPartition_LIST dead_parts;
  ColPartition_IT dead_it(&dead_parts);
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    part->DisownBoxes();
    dead_it.add_to_end(part); // Parts will be deleted on return.
  }
  Clear();
}

// Deletes all the partitions in the grid that are of type BRT_UNKNOWN and
// all the blobs in them.
void ColPartitionGrid::DeleteUnknownParts(TO_BLOCK *block) {
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->blob_type() == BRT_UNKNOWN) {
      gsearch.RemoveBBox();
      // Once marked, the blobs will be swept up by DeleteUnownedNoise.
      part->set_flow(BTFT_NONTEXT);
      part->set_blob_type(BRT_NOISE);
      part->SetBlobTypes();
      part->DisownBoxes();
      delete part;
    }
  }
  block->DeleteUnownedNoise();
}

// Deletes all the partitions in the grid that are NOT of flow type BTFT_LEADER.
void ColPartitionGrid::DeleteNonLeaderParts() {
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->flow() != BTFT_LEADER) {
      gsearch.RemoveBBox();
      if (part->ReleaseNonLeaderBoxes()) {
        InsertBBox(true, true, part);
        gsearch.RepositionIterator();
      } else {
        delete part;
      }
    }
  }
}

// Finds and marks text partitions that represent figure captions.
void ColPartitionGrid::FindFigureCaptions() {
  // For each image region find its best candidate text caption region,
  // if any and mark it as such.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->IsImageType()) {
      const TBOX &part_box = part->bounding_box();
      bool debug =
          AlignedBlob::WithinTestRegion(2, part_box.left(), part_box.bottom());
      ColPartition *best_caption = nullptr;
      int best_dist = 0;  // Distance to best_caption.
      int best_upper = 0; // Direction of best_caption.
      // Handle both lower and upper directions.
      for (int upper = 0; upper < 2; ++upper) {
        ColPartition_C_IT partner_it(upper ? part->upper_partners()
                                           : part->lower_partners());
        // If there are no image partners, then this direction is ok.
        for (partner_it.mark_cycle_pt(); !partner_it.cycled_list();
             partner_it.forward()) {
          ColPartition *partner = partner_it.data();
          if (partner->IsImageType()) {
            break;
          }
        }
        if (!partner_it.cycled_list()) {
          continue;
        }
        // Find the nearest totally overlapping text partner.
        for (partner_it.mark_cycle_pt(); !partner_it.cycled_list();
             partner_it.forward()) {
          ColPartition *partner = partner_it.data();
          if (!partner->IsTextType() || partner->type() == PT_TABLE) {
            continue;
          }
          const TBOX &partner_box = partner->bounding_box();
          if (debug) {
            tprintf("Finding figure captions for image part:");
            part_box.print();
            tprintf("Considering partner:");
            partner_box.print();
          }
          if (partner_box.left() >= part_box.left() &&
              partner_box.right() <= part_box.right()) {
            int dist = partner_box.y_gap(part_box);
            if (best_caption == nullptr || dist < best_dist) {
              best_dist = dist;
              best_caption = partner;
              best_upper = upper;
            }
          }
        }
      }
      if (best_caption != nullptr) {
        if (debug) {
          tprintf("Best caption candidate:");
          best_caption->bounding_box().print();
        }
        // We have a candidate caption. Qualify it as being separable from
        // any body text. We are looking for either a small number of lines
        // or a big gap that indicates a separation from the body text.
        int line_count = 0;
        int biggest_gap = 0;
        int smallest_gap = INT16_MAX;
        int total_height = 0;
        int mean_height = 0;
        ColPartition *end_partner = nullptr;
        ColPartition *next_partner = nullptr;
        for (ColPartition *partner = best_caption;
             partner != nullptr && line_count <= kMaxCaptionLines;
             partner = next_partner) {
          if (!partner->IsTextType()) {
            end_partner = partner;
            break;
          }
          ++line_count;
          total_height += partner->bounding_box().height();
          next_partner = partner->SingletonPartner(best_upper);
          if (next_partner != nullptr) {
            int gap =
                partner->bounding_box().y_gap(next_partner->bounding_box());
            if (gap > biggest_gap) {
              biggest_gap = gap;
              end_partner = next_partner;
              mean_height = total_height / line_count;
            } else if (gap < smallest_gap) {
              smallest_gap = gap;
            }
            // If the gap looks big compared to the text size and the smallest
            // gap seen so far, then we can stop.
            if (biggest_gap > mean_height * kMinCaptionGapHeightRatio &&
                biggest_gap > smallest_gap * kMinCaptionGapRatio) {
              break;
            }
          }
        }
        if (debug) {
          tprintf("Line count=%d, biggest gap %d, smallest%d, mean height %d\n",
                  line_count, biggest_gap, smallest_gap, mean_height);
          if (end_partner != nullptr) {
            tprintf("End partner:");
            end_partner->bounding_box().print();
          }
        }
        if (next_partner == nullptr && line_count <= kMaxCaptionLines) {
          end_partner = nullptr; // No gap, but line count is small.
        }
        if (line_count <= kMaxCaptionLines) {
          // This is a qualified caption. Mark the text as caption.
          for (ColPartition *partner = best_caption;
               partner != nullptr && partner != end_partner;
               partner = next_partner) {
            partner->set_type(PT_CAPTION_TEXT);
            partner->SetBlobTypes();
            if (debug) {
              tprintf("Set caption type for partition:");
              partner->bounding_box().print();
            }
            next_partner = partner->SingletonPartner(best_upper);
          }
        }
      }
    }
  }
}

//////// Functions that manipulate ColPartitions in the part_grid_ /////
//////// to find chains of partner partitions of the same type.  ///////

// For every ColPartition in the grid, finds its upper and lower neighbours.
void ColPartitionGrid::FindPartitionPartners() {
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    if (part->IsVerticalType()) {
      FindVPartitionPartners(true, part);
      FindVPartitionPartners(false, part);
    } else {
      FindPartitionPartners(true, part);
      FindPartitionPartners(false, part);
    }
  }
}

// Finds the best partner in the given direction for the given partition.
// Stores the result with AddPartner.
void ColPartitionGrid::FindPartitionPartners(bool upper, ColPartition *part) {
  if (part->type() == PT_NOISE) {
    return; // Noise is not allowed to partner anything.
  }
  const TBOX &box = part->bounding_box();
  int top = part->median_top();
  int bottom = part->median_bottom();
  int height = top - bottom;
  int mid_y = (bottom + top) / 2;
  ColPartitionGridSearch vsearch(this);
  // Search down for neighbour below
  vsearch.StartVerticalSearch(box.left(), box.right(), part->MidY());
  ColPartition *neighbour;
  ColPartition *best_neighbour = nullptr;
  int best_dist = INT32_MAX;
  while ((neighbour = vsearch.NextVerticalSearch(!upper)) != nullptr) {
    if (neighbour == part || neighbour->type() == PT_NOISE) {
      continue; // Noise is not allowed to partner anything.
    }
    int neighbour_bottom = neighbour->median_bottom();
    int neighbour_top = neighbour->median_top();
    int neighbour_y = (neighbour_bottom + neighbour_top) / 2;
    if (upper != (neighbour_y > mid_y)) {
      continue;
    }
    if (!part->HOverlaps(*neighbour) && !part->WithinSameMargins(*neighbour)) {
      continue;
    }
    if (!part->TypesMatch(*neighbour)) {
      if (best_neighbour == nullptr) {
        best_neighbour = neighbour;
      }
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
  if (best_neighbour != nullptr) {
    part->AddPartner(upper, best_neighbour);
  }
}

// Finds the best partner in the given direction for the given partition.
// Stores the result with AddPartner.
void ColPartitionGrid::FindVPartitionPartners(bool to_the_left,
                                              ColPartition *part) {
  if (part->type() == PT_NOISE) {
    return; // Noise is not allowed to partner anything.
  }
  const TBOX &box = part->bounding_box();
  int left = part->median_left();
  int right = part->median_right();
  int width = right >= left ? right - left : -1;
  int mid_x = (left + right) / 2;
  ColPartitionGridSearch hsearch(this);
  // Search left for neighbour to_the_left
  hsearch.StartSideSearch(mid_x, box.bottom(), box.top());
  ColPartition *neighbour;
  ColPartition *best_neighbour = nullptr;
  int best_dist = INT32_MAX;
  while ((neighbour = hsearch.NextSideSearch(to_the_left)) != nullptr) {
    if (neighbour == part || neighbour->type() == PT_NOISE) {
      continue; // Noise is not allowed to partner anything.
    }
    int neighbour_left = neighbour->median_left();
    int neighbour_right = neighbour->median_right();
    int neighbour_x = (neighbour_left + neighbour_right) / 2;
    if (to_the_left != (neighbour_x < mid_x)) {
      continue;
    }
    if (!part->VOverlaps(*neighbour)) {
      continue;
    }
    if (!part->TypesMatch(*neighbour)) {
      continue; // Only match to other vertical text.
    }
    int dist = to_the_left ? left - neighbour_right : neighbour_left - right;
    if (dist <= kMaxPartitionSpacing * width) {
      if (dist < best_dist || best_neighbour == nullptr) {
        best_dist = dist;
        best_neighbour = neighbour;
      }
    } else {
      break;
    }
  }
  // For vertical partitions, the upper partner is to the left, and lower is
  // to the right.
  if (best_neighbour != nullptr) {
    part->AddPartner(to_the_left, best_neighbour);
  }
}

// For every ColPartition with multiple partners in the grid, reduces the
// number of partners to 0 or 1. If get_desperate is true, goes to more
// desperate merge methods to merge flowing text before breaking partnerships.
void ColPartitionGrid::RefinePartitionPartners(bool get_desperate) {
  ColPartitionGridSearch gsearch(this);
  // Refine in type order so that chasing multiple partners can be done
  // before eliminating type mis-matching partners.
  for (int type = PT_UNKNOWN + 1; type <= PT_COUNT; type++) {
    // Iterate the ColPartitions in the grid.
    gsearch.StartFullSearch();
    ColPartition *part;
    while ((part = gsearch.NextFullSearch()) != nullptr) {
      part->RefinePartners(static_cast<PolyBlockType>(type), get_desperate,
                           this);
      // Iterator may have been messed up by a merge.
      gsearch.RepositionIterator();
    }
  }
}

// ========================== PRIVATE CODE ========================

// Finds and returns a list of candidate ColPartitions to merge with part.
// The candidates must overlap search_box, and when merged must not
// overlap any other partitions that are not overlapped by each individually.
void ColPartitionGrid::FindMergeCandidates(const ColPartition *part,
                                           const TBOX &search_box, bool debug,
                                           ColPartition_CLIST *candidates) {
  int ok_overlap =
      static_cast<int>(kTinyEnoughTextlineOverlapFraction * gridsize() + 0.5);
  const TBOX &part_box = part->bounding_box();
  // Now run the rect search.
  ColPartitionGridSearch rsearch(this);
  rsearch.SetUniqueMode(true);
  rsearch.StartRectSearch(search_box);
  ColPartition *candidate;
  while ((candidate = rsearch.NextRectSearch()) != nullptr) {
    if (!OKMergeCandidate(part, candidate, debug)) {
      continue;
    }
    const TBOX &c_box = candidate->bounding_box();
    // Candidate seems to be a potential merge with part. If one contains
    // the other, then the merge is a no-brainer. Otherwise, search the
    // combined box to see if anything else is inappropriately overlapped.
    if (!part_box.contains(c_box) && !c_box.contains(part_box)) {
      // Search the combined rectangle to see if anything new is overlapped.
      // This is a preliminary test designed to quickly weed-out poor
      // merge candidates that would create a big list of overlapped objects
      // for the squared-order overlap analysis. Eg. vertical and horizontal
      // line-like objects that overlap real text when merged:
      // || ==========================
      // ||
      // ||  r e a l  t e x t
      // ||
      // ||
      TBOX merged_box(part_box);
      merged_box += c_box;
      ColPartitionGridSearch msearch(this);
      msearch.SetUniqueMode(true);
      msearch.StartRectSearch(merged_box);
      ColPartition *neighbour;
      while ((neighbour = msearch.NextRectSearch()) != nullptr) {
        if (neighbour == part || neighbour == candidate) {
          continue; // Ignore itself.
        }
        if (neighbour->OKMergeOverlap(*part, *candidate, ok_overlap, false)) {
          continue; // This kind of merge overlap is OK.
        }
        TBOX n_box = neighbour->bounding_box();
        // The overlap is OK if:
        // * the n_box already overlapped the part or the candidate OR
        // * the n_box is a suitable merge with either part or candidate
        if (!n_box.overlap(part_box) && !n_box.overlap(c_box) &&
            !OKMergeCandidate(part, neighbour, false) &&
            !OKMergeCandidate(candidate, neighbour, false)) {
          break;
        }
      }
      if (neighbour != nullptr) {
        if (debug) {
          tprintf(
              "Combined box overlaps another that is not OK despite"
              " allowance of %d:",
              ok_overlap);
          neighbour->bounding_box().print();
          tprintf("Reason:");
          OKMergeCandidate(part, neighbour, true);
          tprintf("...and:");
          OKMergeCandidate(candidate, neighbour, true);
          tprintf("Overlap:");
          neighbour->OKMergeOverlap(*part, *candidate, ok_overlap, true);
        }
        continue;
      }
    }
    if (debug) {
      tprintf("Adding candidate:");
      candidate->bounding_box().print();
    }
    // Unique elements as they arrive.
    candidates->add_sorted(SortByBoxLeft<ColPartition>, true, candidate);
  }
}

// Smoothes the region type/flow type of the given part by looking at local
// neighbours and the given image mask. Searches a padded rectangle with the
// padding truncated on one size of the part's box in turn for each side,
// using the result (if any) that has the least distance to all neighbours
// that contribute to the decision. This biases in favor of rectangular
// regions without completely enforcing them.
// If a good decision cannot be reached, the part is left unchanged.
// im_box and rerotation are used to map blob coordinates onto the
// nontext_map, which is used to prevent the spread of text neighbourhoods
// into images.
// Returns true if the partition was changed.
bool ColPartitionGrid::SmoothRegionType(Image nontext_map, const TBOX &im_box,
                                        const FCOORD &rerotation, bool debug,
                                        ColPartition *part) {
  const TBOX &part_box = part->bounding_box();
  if (debug) {
    tprintf("Smooothing part at:");
    part_box.print();
  }
  BlobRegionType best_type = BRT_UNKNOWN;
  int best_dist = INT32_MAX;
  int max_dist = std::min(part_box.width(), part_box.height());
  max_dist = std::max(max_dist * kMaxNeighbourDistFactor, gridsize() * 2);
  // Search with the pad truncated on each side of the box in turn.
  bool any_image = false;
  bool all_image = true;
  for (int d = 0; d < BND_COUNT; ++d) {
    int dist;
    auto dir = static_cast<BlobNeighbourDir>(d);
    BlobRegionType type = SmoothInOneDirection(dir, nontext_map, im_box,
                                               rerotation, debug, *part, &dist);
    if (debug) {
      tprintf("Result in dir %d = %d at dist %d\n", dir, type, dist);
    }
    if (type != BRT_UNKNOWN && dist < best_dist) {
      best_dist = dist;
      best_type = type;
    }
    if (type == BRT_POLYIMAGE) {
      any_image = true;
    } else {
      all_image = false;
    }
  }
  if (best_dist > max_dist) {
    return false; // Too far away to set the type with it.
  }
  if (part->flow() == BTFT_STRONG_CHAIN && !all_image) {
    return false; // We are not modifying it.
  }
  BlobRegionType new_type = part->blob_type();
  BlobTextFlowType new_flow = part->flow();
  if (best_type == BRT_TEXT && !any_image) {
    new_flow = BTFT_STRONG_CHAIN;
    new_type = BRT_TEXT;
  } else if (best_type == BRT_VERT_TEXT && !any_image) {
    new_flow = BTFT_STRONG_CHAIN;
    new_type = BRT_VERT_TEXT;
  } else if (best_type == BRT_POLYIMAGE) {
    new_flow = BTFT_NONTEXT;
    new_type = BRT_UNKNOWN;
  }
  if (new_type != part->blob_type() || new_flow != part->flow()) {
    part->set_flow(new_flow);
    part->set_blob_type(new_type);
    part->SetBlobTypes();
    if (debug) {
      tprintf("Modified part:");
      part->Print();
    }
    return true;
  } else {
    return false;
  }
}

// Sets up a search box based on the part_box, padded in all directions
// except direction. Also setup dist_scaling to weight x,y distances according
// to the given direction.
static void ComputeSearchBoxAndScaling(BlobNeighbourDir direction,
                                       const TBOX &part_box, int min_padding,
                                       TBOX *search_box, ICOORD *dist_scaling) {
  *search_box = part_box;
  // Generate a pad value based on the min dimension of part_box, but at least
  // min_padding and then scaled by kMaxPadFactor.
  int padding = std::min(part_box.height(), part_box.width());
  padding = std::max(padding, min_padding);
  padding *= kMaxPadFactor;
  search_box->pad(padding, padding);
  // Truncate the box in the appropriate direction and make the distance
  // metric slightly biased in the truncated direction.
  switch (direction) {
    case BND_LEFT:
      search_box->set_left(part_box.left());
      *dist_scaling = ICOORD(2, 1);
      break;
    case BND_BELOW:
      search_box->set_bottom(part_box.bottom());
      *dist_scaling = ICOORD(1, 2);
      break;
    case BND_RIGHT:
      search_box->set_right(part_box.right());
      *dist_scaling = ICOORD(2, 1);
      break;
    case BND_ABOVE:
      search_box->set_top(part_box.top());
      *dist_scaling = ICOORD(1, 2);
      break;
    default:
      ASSERT_HOST(false);
  }
}

// Local enum used by SmoothInOneDirection and AccumulatePartDistances
// for the different types of partition neighbour.
enum NeighbourPartitionType {
  NPT_HTEXT,      // Definite horizontal text.
  NPT_VTEXT,      // Definite vertical text.
  NPT_WEAK_HTEXT, // Weakly horizontal text. Counts as HTEXT for HTEXT, but
                  // image for image and VTEXT.
  NPT_WEAK_VTEXT, // Weakly vertical text. Counts as VTEXT for VTEXT, but
                  // image for image and HTEXT.
  NPT_IMAGE,      // Defininte non-text.
  NPT_COUNT       // Number of array elements.
};

// Executes the search for SmoothRegionType in a single direction.
// Creates a bounding box that is padded in all directions except direction,
// and searches it for other partitions. Finds the nearest collection of
// partitions that makes a decisive result (if any) and returns the type
// and the distance of the collection. If there are any pixels in the
// nontext_map, then the decision is biased towards image.
BlobRegionType ColPartitionGrid::SmoothInOneDirection(
    BlobNeighbourDir direction, Image nontext_map, const TBOX &im_box,
    const FCOORD &rerotation, bool debug, const ColPartition &part,
    int *best_distance) {
  // Set up a rectangle search bounded by the part.
  const TBOX &part_box = part.bounding_box();
  TBOX search_box;
  ICOORD dist_scaling;
  ComputeSearchBoxAndScaling(direction, part_box, gridsize(), &search_box,
                             &dist_scaling);
  bool image_region = ImageFind::CountPixelsInRotatedBox(
                          search_box, im_box, rerotation, nontext_map) > 0;
  std::vector<int> dists[NPT_COUNT];
  AccumulatePartDistances(part, dist_scaling, search_box, nontext_map, im_box,
                          rerotation, debug, dists);
  // By iteratively including the next smallest distance across the vectors,
  // (as in a merge sort) we can use the vector indices as counts of each type
  // and find the nearest set of objects that give us a definite decision.
  unsigned counts[NPT_COUNT];
  memset(counts, 0, sizeof(counts));
  // If there is image in the search box, tip the balance in image's favor.
  int image_bias = image_region ? kSmoothDecisionMargin / 2 : 0;
  BlobRegionType text_dir = part.blob_type();
  BlobTextFlowType flow_type = part.flow();
  int min_dist = 0;
  do {
    // Find the minimum new entry across the vectors
    min_dist = INT32_MAX;
    for (int i = 0; i < NPT_COUNT; ++i) {
      if (counts[i] < dists[i].size() && dists[i][counts[i]] < min_dist) {
        min_dist = dists[i][counts[i]];
      }
    }
    // Step all the indices/counts forward to include min_dist.
    for (int i = 0; i < NPT_COUNT; ++i) {
      while (counts[i] < dists[i].size() && dists[i][counts[i]] <= min_dist) {
        ++counts[i];
      }
    }
    *best_distance = min_dist;
    if (debug) {
      tprintf("Totals: htext=%u+%u, vtext=%u+%u, image=%u+%u, at dist=%d\n",
              counts[NPT_HTEXT], counts[NPT_WEAK_HTEXT], counts[NPT_VTEXT],
              counts[NPT_WEAK_VTEXT], counts[NPT_IMAGE], image_bias, min_dist);
    }
    // See if we have a decision yet.
    auto image_count = counts[NPT_IMAGE];
    auto htext_score = counts[NPT_HTEXT] + counts[NPT_WEAK_HTEXT] -
                       (image_count + counts[NPT_WEAK_VTEXT]);
    auto vtext_score = counts[NPT_VTEXT] + counts[NPT_WEAK_VTEXT] -
                       (image_count + counts[NPT_WEAK_HTEXT]);
    if (image_count > 0 && image_bias - htext_score >= kSmoothDecisionMargin &&
        image_bias - vtext_score >= kSmoothDecisionMargin) {
      *best_distance = dists[NPT_IMAGE][0];
      if (!dists[NPT_WEAK_VTEXT].empty() &&
          *best_distance > dists[NPT_WEAK_VTEXT][0]) {
        *best_distance = dists[NPT_WEAK_VTEXT][0];
      }
      if (!dists[NPT_WEAK_HTEXT].empty() &&
          *best_distance > dists[NPT_WEAK_HTEXT][0]) {
        *best_distance = dists[NPT_WEAK_HTEXT][0];
      }
      return BRT_POLYIMAGE;
    }
    if ((text_dir != BRT_VERT_TEXT || flow_type != BTFT_CHAIN) &&
        counts[NPT_HTEXT] > 0 && htext_score >= kSmoothDecisionMargin) {
      *best_distance = dists[NPT_HTEXT][0];
      return BRT_TEXT;
    } else if ((text_dir != BRT_TEXT || flow_type != BTFT_CHAIN) &&
               counts[NPT_VTEXT] > 0 && vtext_score >= kSmoothDecisionMargin) {
      *best_distance = dists[NPT_VTEXT][0];
      return BRT_VERT_TEXT;
    }
  } while (min_dist < INT32_MAX);
  return BRT_UNKNOWN;
}

// Counts the partitions in the given search_box by appending the gap
// distance (scaled by dist_scaling) of the part from the base_part to the
// vector of the appropriate type for the partition. Prior to return, the
// vectors in the dists array are sorted in increasing order.
// The nontext_map (+im_box, rerotation) is used to make text invisible if
// there is non-text in between.
// dists must be an array of vectors of size NPT_COUNT.
void ColPartitionGrid::AccumulatePartDistances(
    const ColPartition &base_part, const ICOORD &dist_scaling,
    const TBOX &search_box, Image nontext_map, const TBOX &im_box,
    const FCOORD &rerotation, bool debug, std::vector<int> *dists) {
  const TBOX &part_box = base_part.bounding_box();
  ColPartitionGridSearch rsearch(this);
  rsearch.SetUniqueMode(true);
  rsearch.StartRectSearch(search_box);
  ColPartition *neighbour;
  // Search for compatible neighbours with a similar strokewidth, but not
  // on the other side of a tab vector.
  while ((neighbour = rsearch.NextRectSearch()) != nullptr) {
    if (neighbour->IsUnMergeableType() ||
        !base_part.ConfirmNoTabViolation(*neighbour) ||
        neighbour == &base_part) {
      continue;
    }
    TBOX nbox = neighbour->bounding_box();
    BlobRegionType n_type = neighbour->blob_type();
    if ((n_type == BRT_TEXT || n_type == BRT_VERT_TEXT) &&
        !ImageFind::BlankImageInBetween(part_box, nbox, im_box, rerotation,
                                        nontext_map)) {
      continue; // Text not visible the other side of image.
    }
    if (BLOBNBOX::IsLineType(n_type)) {
      continue; // Don't use horizontal lines as neighbours.
    }
    int x_gap = std::max(part_box.x_gap(nbox), 0);
    int y_gap = std::max(part_box.y_gap(nbox), 0);
    int n_dist = x_gap * dist_scaling.x() + y_gap * dist_scaling.y();
    if (debug) {
      tprintf("Part has x-gap=%d, y=%d, dist=%d at:", x_gap, y_gap, n_dist);
      nbox.print();
    }
    // Truncate the number of boxes, so text doesn't get too much advantage.
    int n_boxes = std::min(neighbour->boxes_count(), kSmoothDecisionMargin);
    BlobTextFlowType n_flow = neighbour->flow();
    std::vector<int> *count_vector = nullptr;
    if (n_flow == BTFT_STRONG_CHAIN) {
      if (n_type == BRT_TEXT) {
        count_vector = &dists[NPT_HTEXT];
      } else {
        count_vector = &dists[NPT_VTEXT];
      }
      if (debug) {
        tprintf("%s %d\n", n_type == BRT_TEXT ? "Htext" : "Vtext", n_boxes);
      }
    } else if ((n_type == BRT_TEXT || n_type == BRT_VERT_TEXT) &&
               (n_flow == BTFT_CHAIN || n_flow == BTFT_NEIGHBOURS)) {
      // Medium text counts as weak, and all else counts as image.
      if (n_type == BRT_TEXT) {
        count_vector = &dists[NPT_WEAK_HTEXT];
      } else {
        count_vector = &dists[NPT_WEAK_VTEXT];
      }
      if (debug) {
        tprintf("Weak %d\n", n_boxes);
      }
    } else {
      count_vector = &dists[NPT_IMAGE];
      if (debug) {
        tprintf("Image %d\n", n_boxes);
      }
    }
    if (count_vector != nullptr) {
      for (int i = 0; i < n_boxes; ++i) {
        count_vector->push_back(n_dist);
      }
    }
    if (debug) {
      neighbour->Print();
    }
  }
  for (int i = 0; i < NPT_COUNT; ++i) {
    std::sort(dists[i].begin(), dists[i].end());
  }
}

// Improves the margins of the part ColPartition by searching for
// neighbours that vertically overlap significantly.
// columns may be nullptr, and indicates the assigned column structure this
// is applicable to part.
void ColPartitionGrid::FindPartitionMargins(ColPartitionSet *columns,
                                            ColPartition *part) {
  // Set up a rectangle search x-bounded by the column and y by the part.
  TBOX box = part->bounding_box();
  int y = part->MidY();
  // Initial left margin is based on the column, if there is one.
  int left_margin = bleft().x();
  int right_margin = tright().x();
  if (columns != nullptr) {
    ColPartition *column = columns->ColumnContaining(box.left(), y);
    if (column != nullptr) {
      left_margin = column->LeftAtY(y);
    }
    column = columns->ColumnContaining(box.right(), y);
    if (column != nullptr) {
      right_margin = column->RightAtY(y);
    }
  }
  left_margin -= kColumnWidthFactor;
  right_margin += kColumnWidthFactor;
  // Search for ColPartitions that reduce the margin.
  left_margin = FindMargin(box.left() + box.height(), true, left_margin,
                           box.bottom(), box.top(), part);
  part->set_left_margin(left_margin);
  // Search for ColPartitions that reduce the margin.
  right_margin = FindMargin(box.right() - box.height(), false, right_margin,
                            box.bottom(), box.top(), part);
  part->set_right_margin(right_margin);
}

// Starting at x, and going in the specified direction, up to x_limit, finds
// the margin for the given y range by searching sideways,
// and ignoring not_this.
int ColPartitionGrid::FindMargin(int x, bool right_to_left, int x_limit,
                                 int y_bottom, int y_top,
                                 const ColPartition *not_this) {
  int height = y_top - y_bottom;
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch side_search(this);
  side_search.SetUniqueMode(true);
  side_search.StartSideSearch(x, y_bottom, y_top);
  ColPartition *part;
  while ((part = side_search.NextSideSearch(right_to_left)) != nullptr) {
    // Ignore itself.
    if (part == not_this) { // || part->IsLineType())
      continue;
    }
    // Must overlap by enough, based on the min of the heights, so
    // large partitions can't smash through small ones.
    TBOX box = part->bounding_box();
    int min_overlap = std::min(height, static_cast<int>(box.height()));
    min_overlap = static_cast<int>(min_overlap * kMarginOverlapFraction + 0.5);
    int y_overlap = std::min(y_top, static_cast<int>(box.top())) -
                    std::max(y_bottom, static_cast<int>(box.bottom()));
    if (y_overlap < min_overlap) {
      continue;
    }
    // Must be going the right way.
    int x_edge = right_to_left ? box.right() : box.left();
    if ((x_edge < x) != right_to_left) {
      continue;
    }
    // If we have gone past x_limit, then x_limit will do.
    if ((x_edge < x_limit) == right_to_left) {
      break;
    }
    // It reduces x limit, so save the new one.
    x_limit = x_edge;
  }
  return x_limit;
}

} // namespace tesseract.
