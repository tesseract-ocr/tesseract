///////////////////////////////////////////////////////////////////////
// File:        colpartitionrid.h
// Description: Class collecting code that acts on a BBGrid of ColPartitions.
// Author:      Ray Smith
// Created:     Mon Oct 05 08:42:01 PDT 2009
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

#include "colpartitiongrid.h"
#include "colpartitionset.h"

namespace tesseract {

BOOL_VAR(textord_tabfind_show_color_fit, false, "Show stroke widths");

// Maximum number of lines in a credible figure caption.
const int kMaxCaptionLines = 7;
// Min ratio between biggest and smallest gap to bound a caption.
const double kMinCaptionGapRatio = 2.0;
// Min ratio between biggest gap and mean line height to bound a caption.
const double kMinCaptionGapHeightRatio = 0.5;
// Min fraction of ColPartition height to be overlapping for margin purposes.
const double kMarginOverlapFraction = 0.25;
// Fraction of gridsize to allow arbitrary overlap between partitions.
const double kTinyEnoughTextlineOverlapFraction = 0.25;
// Max vertical distance of neighbouring ColPartition as a multiple of
// partition height for it to be a partner.
// TODO(rays) determine and write here why a larger number doesn't work well.
const double kMaxPartitionSpacing = 1.75;

ColPartitionGrid::ColPartitionGrid() {
}
ColPartitionGrid::ColPartitionGrid(int gridsize,
                                   const ICOORD& bleft, const ICOORD& tright)
  : BBGrid<ColPartition, ColPartition_CLIST, ColPartition_C_IT>(gridsize,
                                                                bleft, tright) {
}

ColPartitionGrid::~ColPartitionGrid() {
}

// Handles a click event in a display window.
void ColPartitionGrid::HandleClick(int x, int y) {
  BBGrid<ColPartition,
         ColPartition_CLIST, ColPartition_C_IT>::HandleClick(x, y);
  // Run a radial search for partitions that overlap.
  ColPartitionGridSearch radsearch(this);
  radsearch.SetUniqueMode(true);
  radsearch.StartRadSearch(x, y, 1);
  ColPartition* neighbour;
  FCOORD click(x, y);
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (nbox.contains(click)) {
      tprintf("Block box:");
      neighbour->bounding_box().print();
      neighbour->Print();
    }
  }
}

// Returns true if the given part and merge candidate might believably
// be part of a single text line according to the default rules.
// In general we only want to merge partitions that look like they
// are on the same text line, ie their median limits overlap, but we have
// to make exceptions for diacritics and stray punctuation.
static bool OKMergeCandidate(const ColPartition* part,
                             const ColPartition* candidate,
                             bool debug) {
  const TBOX& part_box = part->bounding_box();
  if (candidate == part)
    return false;  // Ignore itself.
  if (!part->TypesMatch(*candidate) || candidate->IsUnMergeableType())
    return false;  // Don't mix inappropriate types.

  const TBOX& c_box = candidate->bounding_box();
  if (debug) {
    tprintf("Examining merge candidate:");
    c_box.print();
  }
  // Candidates must be within a reasonable distance.
  if (candidate->IsVerticalType() || part->IsVerticalType()) {
    int h_dist = -part->HOverlap(*candidate);
    if (h_dist >= MAX(part_box.width(), c_box.width()) / 2) {
      if (debug)
        tprintf("Too far away: h_dist = %d\n", h_dist);
      return false;
    }
  } else {
    // Coarse filter by vertical distance between partitions.
    int v_dist = -part->VOverlap(*candidate);
    if (v_dist >= MAX(part_box.height(), c_box.height()) / 2) {
      if (debug)
        tprintf("Too far away: v_dist = %d\n", v_dist);
      return false;
    }
    // Candidates must either overlap in median y,
    // or part or candidate must be an acceptable diacritic.
    if (!part->VOverlaps(*candidate) &&
        !part->OKDiacriticMerge(*candidate, debug) &&
        !candidate->OKDiacriticMerge(*part, debug)) {
      if (debug)
        tprintf("Candidate fails overlap and diacritic tests!\n");
      return false;
    }
  }
  return true;
}

// Helper function to compute the increase in overlap of the parts list of
// Colpartitions with the combination of merge1 and merge2, compared to
// the overlap with them uncombined.
// An overlap is not counted if passes the OKMergeOverlap test with ok_overlap
// as the pixel overlap limit. merge1 and merge2 must both be non-NULL.
static int IncreaseInOverlap(const ColPartition* merge1,
                             const ColPartition* merge2,
                             int ok_overlap,
                             ColPartition_CLIST* parts) {
  ASSERT_HOST(merge1 != NULL && merge2 != NULL);
  int total_area = 0;
  ColPartition_C_IT it(parts);
  TBOX merged_box(merge1->bounding_box());
  merged_box += merge2->bounding_box();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* part = it.data();
    if (part == merge1 || part == merge2)
      continue;
    TBOX part_box = part->bounding_box();
    // Compute the overlap of the merged box with part.
    int overlap_area = part_box.intersection(merged_box).area();
    if (overlap_area > 0 && !part->OKMergeOverlap(*merge1, *merge2,
                                                  ok_overlap, false)) {
      total_area += overlap_area;
      // Subtract the overlap of merge1 and merge2 individually.
      overlap_area = part_box.intersection(merge1->bounding_box()).area();
      if (overlap_area > 0)
        total_area -= overlap_area;
      TBOX intersection_box = part_box.intersection(merge2->bounding_box());
      overlap_area = intersection_box.area();
      if (overlap_area > 0) {
        total_area -= overlap_area;
        // Add back the 3-way area.
        intersection_box -= merge1->bounding_box();
        overlap_area = intersection_box.area();
        if (overlap_area > 0)
          total_area += overlap_area;
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
static bool TestCompatibleCandidates(const ColPartition& part, bool debug,
                                     ColPartition_CLIST* candidates) {
  ColPartition_C_IT it(candidates);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* candidate = it.data();
    if (!candidate->OKDiacriticMerge(part, false)) {
      ColPartition_C_IT it2(it);
      for (it2.mark_cycle_pt(); !it2.cycled_list(); it2.forward()) {
        ColPartition* candidate2 = it2.data();
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

// Finds all the ColPartitions in the grid that overlap with the given
// box and returns them SortByBoxLeft(ed) and uniqued in the given list.
// Any partition equal to not_this (may be NULL) is excluded.
void ColPartitionGrid::FindOverlappingPartitions(const TBOX& box,
                                                 const ColPartition* not_this,
                                                 ColPartition_CLIST* parts) {
  ColPartitionGridSearch rsearch(this);
  rsearch.StartRectSearch(box);
  ColPartition* part;
  while ((part = rsearch.NextRectSearch()) != NULL) {
    if (part != not_this)
      parts->add_sorted(SortByBoxLeft<ColPartition>, true, part);
  }
}

// Finds and returns the best candidate ColPartition to merge with part,
// selected from the candidates list, based on the minimum increase in
// pairwise overlap among all the partitions overlapped by the combined box.
// If overlap_increase is not NULL then it returns the increase in overlap
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
ColPartition* ColPartitionGrid::BestMergeCandidate(
    const ColPartition* part, ColPartition_CLIST* candidates, bool debug,
    TessResultCallback2<bool, const ColPartition*, const ColPartition*>* confirm_cb,
    int* overlap_increase) {
  if (overlap_increase != NULL)
    *overlap_increase = 0;
  if (candidates->empty())
    return NULL;
  int ok_overlap =
      static_cast<int>(kTinyEnoughTextlineOverlapFraction * gridsize() + 0.5);
  // The best neighbour to merge with is the one that causes least
  // total pairwise overlap among all the neighbours.
  // If more than one offers the same total overlap, choose the one
  // with the least total area.
  const TBOX& part_box = part->bounding_box();
  ColPartition_C_IT it(candidates);
  ColPartition* best_candidate = NULL;
  // Find the total combined box of all candidates and the original.
  TBOX full_box(part_box);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* candidate = it.data();
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
  int best_increase = MAX_INT32;
  int best_area = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* candidate = it.data();
    if (confirm_cb != NULL && !confirm_cb->Run(part, candidate)) {
      if (debug) {
        tprintf("Candidate not confirmed:");
        candidate->bounding_box().print();
      }
      continue;
    }
    int increase = IncreaseInOverlap(part, candidate, ok_overlap, &neighbours);
    const TBOX& cand_box = candidate->bounding_box();
    if (best_candidate == NULL || increase < best_increase) {
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
    if (increase > worst_nc_increase)
      worst_nc_increase = increase;
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
  if (overlap_increase != NULL)
    *overlap_increase = best_increase;
  return best_candidate;
}

// Improves the margins of the ColPartitions in the grid by calling
// FindPartitionMargins on each.
// best_columns, which may be NULL, is an array of pointers indicating the
// column set at each y-coordinate in the grid.
// best_columns is usually the best_columns_ member of ColumnFinder.
void ColPartitionGrid::GridFindMargins(ColPartitionSet** best_columns) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // Set up a rectangle search x-bounded by the column and y by the part.
    ColPartitionSet* columns = best_columns != NULL
                             ? best_columns[gsearch.GridY()]
                             : NULL;
    FindPartitionMargins(columns, part);
    const TBOX& box = part->bounding_box();
    if (AlignedBlob::WithinTestRegion(2, box.left(), box.bottom())) {
      tprintf("Computed margins for part:");
      part->Print();
    }
  }
}

// Improves the margins of the ColPartitions in the list by calling
// FindPartitionMargins on each.
// best_columns, which may be NULL, is an array of pointers indicating the
// column set at each y-coordinate in the grid.
// best_columns is usually the best_columns_ member of ColumnFinder.
void ColPartitionGrid::ListFindMargins(ColPartitionSet** best_columns,
                                       ColPartition_LIST* parts) {
  ColPartition_IT part_it(parts);
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition* part = part_it.data();
    ColPartitionSet* columns = NULL;
    if (best_columns != NULL) {
      TBOX part_box = part->bounding_box();
      // Get the columns from the y grid coord.
      int grid_x, grid_y;
      GridCoords(part_box.left(), part_box.bottom(), &grid_x, &grid_y);
      columns = best_columns[grid_y];
    }
    FindPartitionMargins(columns, part);
  }
}

// Finds and marks text partitions that represent figure captions.
void ColPartitionGrid::FindFigureCaptions() {
  // For each image region find its best candidate text caption region,
  // if any and mark it as such.
  ColPartitionGridSearch gsearch(this);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->IsImageType()) {
      const TBOX& part_box = part->bounding_box();
      bool debug = AlignedBlob::WithinTestRegion(2, part_box.left(),
                                                 part_box.bottom());
      ColPartition* best_caption = NULL;
      int best_dist = 0;   // Distance to best_caption.
      int best_upper = 0;  // Direction of best_caption.
      // Handle both lower and upper directions.
      for (int upper = 0; upper < 2; ++upper) {
        ColPartition_C_IT partner_it(upper ? part->upper_partners()
                                           : part->lower_partners());
        // If there are no image partners, then this direction is ok.
        for (partner_it.mark_cycle_pt(); !partner_it.cycled_list();
             partner_it.forward()) {
          ColPartition* partner = partner_it.data();
          if (partner->IsImageType()) {
            break;
          }
        }
        if (!partner_it.cycled_list()) continue;
        // Find the nearest totally overlapping text partner.
        for (partner_it.mark_cycle_pt(); !partner_it.cycled_list();
             partner_it.forward()) {
          ColPartition* partner = partner_it.data();
          if (!partner->IsTextType()) continue;
          const TBOX& partner_box = partner->bounding_box();
          if (debug) {
            tprintf("Finding figure captions for image part:");
            part_box.print();
            tprintf("Considering partner:");
            partner_box.print();
          }
          if (partner_box.left() >= part_box.left() &&
              partner_box.right() <= part_box.right()) {
            int dist = partner_box.y_gap(part_box);
            if (best_caption == NULL || dist < best_dist) {
              best_dist = dist;
              best_caption = partner;
              best_upper = upper;
            }
          }
        }
      }
      if (best_caption != NULL) {
        if (debug) {
          tprintf("Best caption candidate:");
          best_caption->bounding_box().print();
        }
        // We have a candidate caption. Qualify it as being separable from
        // any body text. We are looking for either a small number of lines
        // or a big gap that indicates a separation from the body text.
        int line_count = 0;
        int biggest_gap = 0;
        int smallest_gap = MAX_INT16;
        int total_height = 0;
        int mean_height = 0;
        ColPartition* end_partner = NULL;
        ColPartition* next_partner = NULL;
        for (ColPartition* partner = best_caption; partner != NULL &&
             line_count <= kMaxCaptionLines;
             partner = next_partner) {
          if (!partner->IsTextType()) {
            end_partner = partner;
            break;
          }
          ++line_count;
          total_height += partner->bounding_box().height();
          next_partner = partner->SingletonPartner(best_upper);
          if (next_partner != NULL) {
            int gap = partner->bounding_box().y_gap(
                next_partner->bounding_box());
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
                biggest_gap > smallest_gap * kMinCaptionGapRatio)
              break;
          }
        }
        if (debug) {
          tprintf("Line count=%d, biggest gap %d, smallest%d, mean height %d\n",
                  line_count, biggest_gap, smallest_gap, mean_height);
          if (end_partner != NULL) {
            tprintf("End partner:");
            end_partner->bounding_box().print();
          }
        }
        if (next_partner == NULL && line_count <= kMaxCaptionLines)
          end_partner = NULL;  // No gap, but line count is small.
        if (line_count <= kMaxCaptionLines) {
          // This is a qualified caption. Mark the text as caption.
          for (ColPartition* partner = best_caption; partner != NULL &&
               partner != end_partner;
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
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    FindPartitionPartners(true, part);
    FindPartitionPartners(false, part);
  }
}

// Finds the best partner in the given direction for the given partition.
// Stores the result with AddPartner.
void ColPartitionGrid::FindPartitionPartners(bool upper, ColPartition* part) {
  if (part->type() == PT_NOISE)
    return;  // Noise is not allowed to partner anything.
  const TBOX& box = part->bounding_box();
  int top = part->median_top();
  int bottom = part->median_bottom();
  int height = top - bottom;
  int mid_y = (bottom + top) / 2;
  ColPartitionGridSearch vsearch(this);
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
// number of partners to 0 or 1. If get_desperate is true, goes to more
// desperate merge methods to merge flowing text before breaking partnerships.
void ColPartitionGrid::RefinePartitionPartners(bool get_desperate) {
  ColPartitionGridSearch gsearch(this);
  // Refine in type order so that chasing multiple partners can be done
  // before eliminating type mis-matching partners.
  for (int type = PT_UNKNOWN + 1; type <= PT_COUNT; type++) {
    // Iterate the ColPartitions in the grid.
    gsearch.StartFullSearch();
    ColPartition* part;
    while ((part = gsearch.NextFullSearch()) != NULL) {
      part->RefinePartners(static_cast<PolyBlockType>(type),
                           get_desperate, this);
      // Iterator may have been messed up by a merge.
      gsearch.RepositionIterator();
    }
  }
}


// ========================== PRIVATE CODE ========================

// Improves the margins of the part ColPartition by searching for
// neighbours that vertically overlap significantly.
// columns may be NULL, and indicates the assigned column structure this
// is applicable to part.
void ColPartitionGrid::FindPartitionMargins(ColPartitionSet* columns,
                                            ColPartition* part) {
  // Set up a rectangle search x-bounded by the column and y by the part.
  TBOX box = part->bounding_box();
  int y = part->MidY();
  // Initial left margin is based on the column, if there is one.
  int left_margin = bleft().x();
  int right_margin = tright().x();
  if (columns != NULL) {
    ColPartition* column = columns->ColumnContaining(box.left(), y);
    if (column != NULL)
      left_margin = column->LeftAtY(y);
    column = columns->ColumnContaining(box.right(), y);
    if (column != NULL)
      right_margin = column->RightAtY(y);
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

// Starting at x, and going in the specified direction, upto x_limit, finds
// the margin for the given y range by searching sideways,
// and ignoring not_this.
int ColPartitionGrid::FindMargin(int x, bool right_to_left, int x_limit,
                                 int y_bottom, int y_top,
                                 const ColPartition* not_this) {
  int height = y_top - y_bottom;
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch side_search(this);
  side_search.SetUniqueMode(true);
  side_search.StartSideSearch(x, y_bottom, y_top);
  ColPartition* part;
  while ((part = side_search.NextSideSearch(right_to_left)) != NULL) {
    // Ignore itself.
    if (part == not_this)  // || part->IsLineType())
      continue;
    // Must overlap by enough, based on the min of the heights, so
    // large partitions can't smash through small ones.
    TBOX box = part->bounding_box();
    int min_overlap = MIN(height, box.height());
    min_overlap = static_cast<int>(min_overlap * kMarginOverlapFraction + 0.5);
    int y_overlap = MIN(y_top, box.top()) - MAX(y_bottom, box.bottom());
    if (y_overlap < min_overlap)
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


}  // namespace tesseract.
