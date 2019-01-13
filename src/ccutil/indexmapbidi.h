///////////////////////////////////////////////////////////////////////
// File:        indexmapbidi.h
// Description: Bi-directional mapping between a sparse and compact space.
// Author:      rays@google.com (Ray Smith)
// Created:     Tue Apr 06 11:33:59 PDT 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TESSERACT_CCUTIL_INDEXMAPBIDI_H_
#define TESSERACT_CCUTIL_INDEXMAPBIDI_H_

#include <cstdio>
#include "genericvector.h"

namespace tesseract {

class IndexMapBiDi;

// Bidirectional one-to-one mapping between a sparse and a compact discrete
// space. Many entries in the sparse space are unmapped, but those that are
// mapped have a 1-1 mapping to (and from) the compact space, where all
// values are used. This is useful for forming subsets of larger collections,
// such as subsets of character sets, or subsets of binary feature spaces.
//
// This base class provides basic functionality with binary search for the
// SparseToCompact mapping to save memory.
// For a faster inverse mapping, or to allow a many-to-one mapping, use
// IndexMapBiDi below.
// NOTE: there are currently no methods to setup an IndexMap on its own!
// It must be initialized by copying from an IndexMapBiDi or by DeSerialize.
class IndexMap {
 public:
  virtual ~IndexMap();

  // SparseToCompact takes a sparse index to an index in the compact space.
  // Uses a binary search to find the result. For faster speed use
  // IndexMapBiDi, but that takes more memory.
  virtual int SparseToCompact(int sparse_index) const;

  // CompactToSparse takes a compact index to the corresponding index in the
  // sparse space.
  int CompactToSparse(int compact_index) const {
    return compact_map_[compact_index];
  }
  // The size of the sparse space.
  virtual int SparseSize() const {
    return sparse_size_;
  }
  // The size of the compact space.
  int CompactSize() const {
    return compact_map_.size();
  }

  // Copy from the input.
  void CopyFrom(const IndexMap& src);
  void CopyFrom(const IndexMapBiDi& src);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

 protected:
  // The sparse space covers integers in the range [0, sparse_size_-1].
  int32_t sparse_size_;
  // The compact space covers integers in the range [0, compact_map_.size()-1].
  // Each element contains the corresponding sparse index.
  GenericVector<int32_t> compact_map_;
};

// Bidirectional many-to-one mapping between a sparse and a compact discrete
// space. As with IndexMap, many entries may be unmapped, but unlike IndexMap,
// of those that are, many may be mapped to the same compact index.
// If the map is many-to-one, it is not possible to directly obtain all the
// sparse indices that map to a single compact index.
// This map is time- rather than space-efficient. It stores the entire sparse
// space.
// IndexMapBiDi may be initialized in one of 3 ways:
// 1. Init(size, true);
//    Setup();
//    Sets a complete 1:1 mapping with no unmapped elements.
// 2. Init(size, false);
//    for ... SetMap(index, true);
//    Setup();
//    Specifies precisely which sparse indices are mapped. The mapping is 1:1.
// 3. Either of the above, followed by:
//    for ... Merge(index1, index2);
//    CompleteMerges();
//    Allows a many-to-one mapping by merging compact space indices.
class IndexMapBiDi : public IndexMap {
 public:
  virtual ~IndexMapBiDi();

  // Top-level init function in a single call to initialize a map to select
  // a single contiguous subrange [start, end) of the sparse space to be mapped
  // 1 to 1 to the compact space, with all other elements of the sparse space
  // left unmapped.
  // No need to call Setup after this.
  void InitAndSetupRange(int sparse_size, int start, int end);

  // Initializes just the sparse_map_ to the given size with either all
  // forward indices mapped (all_mapped = true) or none (all_mapped = false).
  // Call Setup immediately after, or make calls to SetMap first to adjust the
  // mapping and then call Setup before using the map.
  void Init(int size, bool all_mapped);
  // Sets a given index in the sparse_map_ to be mapped or not.
  void SetMap(int sparse_index, bool mapped);
  // Sets up the sparse_map_ and compact_map_ properly after Init and
  // some calls to SetMap. Assumes an ordered 1-1 map from set indices
  // in the sparse space to the compact space.
  void Setup();

  // Merges the two compact space indices. May be called many times, but
  // the merges must be concluded by a call to CompleteMerges.
  // Returns true if a merge was actually performed.
  bool Merge(int compact_index1, int compact_index2);
  // Returns true if the given compact index has been deleted.
  bool IsCompactDeleted(int index) const {
    return MasterCompactIndex(index) < 0;
  }
  // Completes one or more Merge operations by further compacting the
  // compact space.
  void CompleteMerges();

  // SparseToCompact takes a sparse index to an index in the compact space.
  virtual int SparseToCompact(int sparse_index) const {
    return sparse_map_[sparse_index];
  }
  // The size of the sparse space.
  virtual int SparseSize() const {
    return sparse_map_.size();
  }

  // Copy from the input.
  void CopyFrom(const IndexMapBiDi& src);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Bulk calls to SparseToCompact.
  // Maps the given array of sparse indices to an array of compact indices.
  // Assumes the input is sorted. The output indices are sorted and uniqued.
  // Return value is the number of "missed" features, being features that
  // don't map to the compact feature space.
  int MapFeatures(const GenericVector<int>& sparse,
                  GenericVector<int>* compact) const;

 private:
  // Returns the master compact index for a given compact index.
  // During a multiple merge operation, several compact indices may be
  // combined, so we need to be able to find the master of all.
  int MasterCompactIndex(int compact_index) const {
    while (compact_index >= 0 &&
           sparse_map_[compact_map_[compact_index]] != compact_index)
      compact_index = sparse_map_[compact_map_[compact_index]];
    return compact_index;
  }

  // Direct look-up of the compact index for each element in sparse space.
  GenericVector<int32_t> sparse_map_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CCUTIL_INDEXMAPBIDI_H_
