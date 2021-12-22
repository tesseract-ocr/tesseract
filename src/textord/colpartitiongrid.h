///////////////////////////////////////////////////////////////////////
// File:        colpartitiongrid.h
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

#ifndef TESSERACT_TEXTORD_COLPARTITIONGRID_H_
#define TESSERACT_TEXTORD_COLPARTITIONGRID_H_

#include "bbgrid.h"
#include "colpartition.h"
#include "colpartitionset.h"

namespace tesseract {

class TabFind;

// ColPartitionGrid is a BBGrid of ColPartition.
// It collects functions that work on the grid.
class TESS_API ColPartitionGrid
    : public BBGrid<ColPartition, ColPartition_CLIST, ColPartition_C_IT> {
public:
  ColPartitionGrid() = default;
  ColPartitionGrid(int gridsize, const ICOORD &bleft, const ICOORD &tright);

  ~ColPartitionGrid() override = default;

  // Handles a click event in a display window.
  void HandleClick(int x, int y) override;

  // Merges ColPartitions in the grid that look like they belong in the same
  // textline.
  // For all partitions in the grid, calls the box_cb permanent callback
  // to compute the search box, searches the box, and if a candidate is found,
  // calls the confirm_cb to check any more rules. If the confirm_cb returns
  // true, then the partitions are merged.
  // Both callbacks are deleted before returning.
  void Merges(const std::function<bool(ColPartition *, TBOX *)> &box_cb,
              const std::function<bool(const ColPartition *,
                                       const ColPartition *)> &confirm_cb);

  // For the given partition, calls the box_cb permanent callback
  // to compute the search box, searches the box, and if a candidate is found,
  // calls the confirm_cb to check any more rules. If the confirm_cb returns
  // true, then the partitions are merged.
  // Returns true if the partition is consumed by one or more merges.
  bool MergePart(const std::function<bool(ColPartition *, TBOX *)> &box_cb,
                 const std::function<bool(const ColPartition *,
                                          const ColPartition *)> &confirm_cb,
                 ColPartition *part);

  // Computes and returns the total overlap of all partitions in the grid.
  // If overlap_grid is non-null, it is filled with a grid that holds empty
  // partitions representing the union of all overlapped partitions.
  int ComputeTotalOverlap(ColPartitionGrid **overlap_grid);

  // Finds all the ColPartitions in the grid that overlap with the given
  // box and returns them SortByBoxLeft(ed) and uniqued in the given list.
  // Any partition equal to not_this (may be nullptr) is excluded.
  void FindOverlappingPartitions(const TBOX &box, const ColPartition *not_this,
                                 ColPartition_CLIST *parts);

  // Finds and returns the best candidate ColPartition to merge with part,
  // selected from the candidates list, based on the minimum increase in
  // pairwise overlap among all the partitions overlapped by the combined box.
  // If overlap_increase is not nullptr then it returns the increase in overlap
  // that would result from the merge.
  // See colpartitiongrid.cpp for a diagram.
  ColPartition *BestMergeCandidate(
      const ColPartition *part, ColPartition_CLIST *candidates, bool debug,
      const std::function<bool(const ColPartition *, const ColPartition *)>
          &confirm_cb,
      int *overlap_increase);

  // Split partitions where it reduces overlap between their bounding boxes.
  // ColPartitions are after all supposed to be a partitioning of the blobs
  // AND of the space on the page!
  // Blobs that cause overlaps get removed, put in individual partitions
  // and added to the big_parts list. They are most likely characters on
  // 2 textlines that touch, or something big like a dropcap.
  void SplitOverlappingPartitions(ColPartition_LIST *big_parts);

  // Filters partitions of source_type by looking at local neighbours.
  // Where a majority of neighbours have a text type, the partitions are
  // changed to text, where the neighbours have image type, they are changed
  // to image, and partitions that have no definite neighbourhood type are
  // left unchanged.
  // im_box and rerotation are used to map blob coordinates onto the
  // nontext_map, which is used to prevent the spread of text neighbourhoods
  // into images.
  // Returns true if anything was changed.
  bool GridSmoothNeighbours(BlobTextFlowType source_type, Image nontext_map,
                            const TBOX &im_box, const FCOORD &rerotation);

  // Reflects the grid and its colpartitions in the y-axis, assuming that
  // all blob boxes have already been done.
  void ReflectInYAxis();

  // Rotates the grid and its colpartitions by the given angle, assuming that
  // all blob boxes have already been done.
  void Deskew(const FCOORD &deskew);

  // Transforms the grid of partitions to the output blocks, putting each
  // partition into a separate block. We don't really care about the order,
  // as we just want to get as much text as possible without trying to organize
  // it into proper blocks or columns.
  void ExtractPartitionsAsBlocks(BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks);

  // Sets the left and right tabs of the partitions in the grid.
  void SetTabStops(TabFind *tabgrid);

  // Makes the ColPartSets and puts them in the PartSetVector ready
  // for finding column bounds. Returns false if no partitions were found.
  // Each ColPartition in the grid is placed in a single ColPartSet based
  // on the bottom-left of its bounding box.
  bool MakeColPartSets(PartSetVector *part_sets);

  // Makes a single ColPartitionSet consisting of a single ColPartition that
  // represents the total horizontal extent of the significant content on the
  // page. Used for the single column setting in place of automatic detection.
  // Returns nullptr if the page is empty of significant content.
  ColPartitionSet *MakeSingleColumnSet(WidthCallback cb);

  // Mark the BLOBNBOXes in each partition as being owned by that partition.
  void ClaimBoxes();

  // Retypes all the blobs referenced by the partitions in the grid.
  // Image blobs are sliced on the grid boundaries to give the tab finder
  // a better handle on the edges of the images, and the actual blobs are
  // returned in the im_blobs list, as they are not owned by the block.
  void ReTypeBlobs(BLOBNBOX_LIST *im_blobs);

  // The boxes within the partitions have changed (by deskew) so recompute
  // the bounds of all the partitions and reinsert them into the grid.
  void RecomputeBounds(int gridsize, const ICOORD &bleft, const ICOORD &tright,
                       const ICOORD &vertical);

  // Improves the margins of the ColPartitions in the grid by calling
  // FindPartitionMargins on each.
  void GridFindMargins(ColPartitionSet **best_columns);

  // Improves the margins of the ColPartitions in the list by calling
  // FindPartitionMargins on each.
  void ListFindMargins(ColPartitionSet **best_columns,
                       ColPartition_LIST *parts);

  // Deletes all the partitions in the grid after disowning all the blobs.
  void DeleteParts();

  // Deletes all the partitions in the grid that are of type BRT_UNKNOWN and
  // all the blobs in them.
  void DeleteUnknownParts(TO_BLOCK *block);

  // Deletes all the partitions in the grid that are NOT of flow type
  // BTFT_LEADER.
  void DeleteNonLeaderParts();

  // Finds and marks text partitions that represent figure captions.
  void FindFigureCaptions();

  //////// Functions that manipulate ColPartitions in the grid     ///////
  //////// to find chains of partner partitions of the same type.  ///////
  // For every ColPartition in the grid, finds its upper and lower neighbours.
  void FindPartitionPartners();
  // Finds the best partner in the given direction for the given partition.
  // Stores the result with AddPartner.
  void FindPartitionPartners(bool upper, ColPartition *part);
  // Finds the best partner in the given direction for the given partition.
  // Stores the result with AddPartner.
  void FindVPartitionPartners(bool to_the_left, ColPartition *part);
  // For every ColPartition with multiple partners in the grid, reduces the
  // number of partners to 0 or 1. If get_desperate is true, goes to more
  // desperate merge methods to merge flowing text before breaking partnerships.
  void RefinePartitionPartners(bool get_desperate);

private:
  // Finds and returns a list of candidate ColPartitions to merge with part.
  // The candidates must overlap search_box, and when merged must not
  // overlap any other partitions that are not overlapped by each individually.
  void FindMergeCandidates(const ColPartition *part, const TBOX &search_box,
                           bool debug, ColPartition_CLIST *candidates);

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
  bool SmoothRegionType(Image nontext_map, const TBOX &im_box,
                        const FCOORD &rerotation, bool debug,
                        ColPartition *part);
  // Executes the search for SmoothRegionType in a single direction.
  // Creates a bounding box that is padded in all directions except direction,
  // and searches it for other partitions. Finds the nearest collection of
  // partitions that makes a decisive result (if any) and returns the type
  // and the distance of the collection. If there are any pixels in the
  // nontext_map, then the decision is biased towards image.
  BlobRegionType SmoothInOneDirection(BlobNeighbourDir direction,
                                      Image nontext_map, const TBOX &im_box,
                                      const FCOORD &rerotation, bool debug,
                                      const ColPartition &part,
                                      int *best_distance);
  // Counts the partitions in the given search_box by appending the gap
  // distance (scaled by dist_scaling) of the part from the base_part to the
  // vector of the appropriate type for the partition. Prior to return, the
  // vectors in the dists array are sorted in increasing order.
  // dists must be an array of vectors of size NPT_COUNT.
  void AccumulatePartDistances(const ColPartition &base_part,
                               const ICOORD &dist_scaling,
                               const TBOX &search_box, Image nontext_map,
                               const TBOX &im_box, const FCOORD &rerotation,
                               bool debug, std::vector<int> *dists);

  // Improves the margins of the ColPartition by searching for
  // neighbours that vertically overlap significantly.
  void FindPartitionMargins(ColPartitionSet *columns, ColPartition *part);

  // Starting at x, and going in the specified direction, up to x_limit, finds
  // the margin for the given y range by searching sideways,
  // and ignoring not_this.
  int FindMargin(int x, bool right_to_left, int x_limit, int y_bottom,
                 int y_top, const ColPartition *not_this);
};

} // namespace tesseract.

#endif // TESSERACT_TEXTORD_COLPARTITIONGRID_H_
