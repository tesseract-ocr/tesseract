///////////////////////////////////////////////////////////////////////
// File:        indexmapbidi.cpp
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

#include "indexmapbidi.h"

namespace tesseract {

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
IndexMap::~IndexMap() = default;

// SparseToCompact takes a sparse index to an index in the compact space.
// Uses a binary search to find the result. For faster speed use
// IndexMapBiDi, but that takes more memory.
int IndexMap::SparseToCompact(int sparse_index) const {
  int result = compact_map_.binary_search(sparse_index);
  return compact_map_[result] == sparse_index ? result : -1;
}

// Copy from the input.
void IndexMap::CopyFrom(const IndexMap& src) {
  sparse_size_ = src.sparse_size_;
  compact_map_ = src.compact_map_;
}
void IndexMap::CopyFrom(const IndexMapBiDi& src) {
  sparse_size_ = src.SparseSize();
  compact_map_ = src.compact_map_;
}

// Writes to the given file. Returns false in case of error.
bool IndexMap::Serialize(FILE* fp) const {
  return tesseract::Serialize(fp, &sparse_size_) && compact_map_.Serialize(fp);
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool IndexMap::DeSerialize(bool swap, FILE* fp) {
  uint32_t sparse_size;
  if (!tesseract::DeSerialize(fp, &sparse_size)) return false;
  if (swap)
    ReverseN(&sparse_size, sizeof(sparse_size));
  // Arbitrarily limit the number of elements to protect against bad data.
  if (sparse_size > UINT16_MAX) return false;
  sparse_size_ = sparse_size;
  return compact_map_.DeSerialize(swap, fp);
}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
IndexMapBiDi::~IndexMapBiDi() = default;

// Top-level init function in a single call to initialize a map to select
// a single contiguous subrange [start, end) of the sparse space to be mapped
// 1 to 1 to the compact space, with all other elements of the sparse space
// left unmapped.
// No need to call Setup after this.
void IndexMapBiDi::InitAndSetupRange(int sparse_size, int start, int end) {
  Init(sparse_size, false);
  for (int i = start; i < end; ++i)
    SetMap(i, true);
  Setup();
}

// Initializes just the sparse_map_ to the given size with either all
// forward indices mapped (all_mapped = true) or none (all_mapped = false).
// Call Setup immediately after, or make calls to SetMap first to adjust the
// mapping and then call Setup before using the map.
void IndexMapBiDi::Init(int size, bool all_mapped) {
  sparse_map_.init_to_size(size, -1);
  if (all_mapped) {
    for (int i = 0; i < size; ++i)
      sparse_map_[i] = i;
  }
}

// Sets a given index in the sparse_map_ to be mapped or not.
void IndexMapBiDi::SetMap(int sparse_index, bool mapped) {
  sparse_map_[sparse_index] = mapped ? 0 : -1;
}

// Sets up the sparse_map_ and compact_map_ properly after Init and
// some calls to SetMap. Assumes an ordered 1-1 map from set indices
// in the forward map to the compact space.
void IndexMapBiDi::Setup() {
  int compact_size = 0;
  for (int i = 0; i < sparse_map_.size(); ++i) {
    if (sparse_map_[i] >= 0) {
      sparse_map_[i] = compact_size++;
    }
  }
  compact_map_.init_to_size(compact_size, -1);
  for (int i = 0; i < sparse_map_.size(); ++i) {
    if (sparse_map_[i] >= 0) {
      compact_map_[sparse_map_[i]] = i;
    }
  }
  sparse_size_ = sparse_map_.size();
}

// Copy from the input.
void IndexMapBiDi::CopyFrom(const IndexMapBiDi& src) {
  sparse_map_ = src.sparse_map_;
  compact_map_ = src.compact_map_;
  sparse_size_ = sparse_map_.size();
}

// Merges the two compact space indices. May be called many times, but
// the merges must be concluded by a call to CompleteMerges.
// Returns true if a merge was actually performed.
bool IndexMapBiDi::Merge(int compact_index1, int compact_index2) {
  // Find the current master index for index1 and index2.
  compact_index1 = MasterCompactIndex(compact_index1);
  compact_index2 = MasterCompactIndex(compact_index2);
  // Be sure that index1 < index2.
  if (compact_index1 > compact_index2) {
    int tmp = compact_index1;
    compact_index1 = compact_index2;
    compact_index2 = tmp;
  } else if (compact_index1 == compact_index2) {
    return false;
  }
  // To save iterating over all sparse_map_ entries, simply make the master
  // entry for index2 point to index1.
  // This leaves behind a potential chain of parents that needs to be chased,
  // as above.
  sparse_map_[compact_map_[compact_index2]] = compact_index1;
  if (compact_index1 >= 0)
    compact_map_[compact_index2] = compact_map_[compact_index1];
  return true;
}

// Completes one or more Merge operations by further compacting the
// compact space. Unused compact space indices are removed, and the used
// ones above shuffled down to fill the gaps.
// Example:
// Input sparse_map_: (x indicates -1)
// x x 0 x 2 x x 4 x 0 x 2 x
// Output sparse_map_:
// x x 0 x 1 x x 2 x 0 x 1 x
// Output compact_map_:
// 2 4 7.
void IndexMapBiDi::CompleteMerges() {
  // Ensure each sparse_map_entry contains a master compact_map_ index.
  int compact_size = 0;
  for (int i = 0; i < sparse_map_.size(); ++i) {
    int compact_index = MasterCompactIndex(sparse_map_[i]);
    sparse_map_[i] = compact_index;
    if (compact_index >= compact_size)
      compact_size = compact_index + 1;
  }
  // Re-generate the compact_map leaving holes for unused indices.
  compact_map_.init_to_size(compact_size, -1);
  for (int i = 0; i < sparse_map_.size(); ++i) {
    if (sparse_map_[i] >= 0) {
      if (compact_map_[sparse_map_[i]] == -1)
        compact_map_[sparse_map_[i]] = i;
    }
  }
  // Compact the compact_map, leaving tmp_compact_map saying where each
  // index went to in the compacted map.
  GenericVector<int32_t> tmp_compact_map;
  tmp_compact_map.init_to_size(compact_size, -1);
  compact_size = 0;
  for (int i = 0; i < compact_map_.size(); ++i) {
    if (compact_map_[i] >= 0) {
      tmp_compact_map[i] = compact_size;
      compact_map_[compact_size++] = compact_map_[i];
    }
  }
  compact_map_.truncate(compact_size);
  // Now modify the entries in the sparse map to point to the new locations.
  for (int i = 0; i < sparse_map_.size(); ++i) {
    if (sparse_map_[i] >= 0) {
      sparse_map_[i] = tmp_compact_map[sparse_map_[i]];
    }
  }
}

// Writes to the given file. Returns false in case of error.
bool IndexMapBiDi::Serialize(FILE* fp) const {
  if (!IndexMap::Serialize(fp)) return false;
  // Make a vector containing the rest of the map. If the map is many-to-one
  // then each additional sparse entry needs to be stored.
  // Normally we store only the compact map to save space.
  GenericVector<int32_t> remaining_pairs;
  for (int i = 0; i < sparse_map_.size(); ++i) {
    if (sparse_map_[i] >= 0 && compact_map_[sparse_map_[i]] != i) {
      remaining_pairs.push_back(i);
      remaining_pairs.push_back(sparse_map_[i]);
    }
  }
  if (!remaining_pairs.Serialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool IndexMapBiDi::DeSerialize(bool swap, FILE* fp) {
  if (!IndexMap::DeSerialize(swap, fp)) return false;
  GenericVector<int32_t> remaining_pairs;
  if (!remaining_pairs.DeSerialize(swap, fp)) return false;
  sparse_map_.init_to_size(sparse_size_, -1);
  for (int i = 0; i < compact_map_.size(); ++i) {
    sparse_map_[compact_map_[i]] = i;
  }
  for (int i = 0; i < remaining_pairs.size(); ++i) {
    int sparse_index = remaining_pairs[i++];
    sparse_map_[sparse_index] = remaining_pairs[i];
  }
  return true;
}

// Bulk calls to SparseToCompact.
// Maps the given array of sparse indices to an array of compact indices.
// Assumes the input is sorted. The output indices are sorted and uniqued.
// Return value is the number of "missed" features, being features that
// don't map to the compact feature space.
int IndexMapBiDi::MapFeatures(const GenericVector<int>& sparse,
                              GenericVector<int>* compact) const {
  compact->truncate(0);
  int num_features = sparse.size();
  int missed_features = 0;
  int prev_good_feature = -1;
  for (int f = 0; f < num_features; ++f) {
    int feature = sparse_map_[sparse[f]];
    if (feature >= 0) {
      if (feature != prev_good_feature) {
        compact->push_back(feature);
        prev_good_feature = feature;
      }
    } else {
      ++missed_features;
    }
  }
  return missed_features;
}

}  // namespace tesseract.
