/******************************************************************************
 ** Filename:   kdtree.h
 ** Purpose:    Definition of K-D tree access routines.
 ** Author:     Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *****************************************************************************/

#ifndef KDTREE_H
#define KDTREE_H

#include "ocrfeatures.h"

namespace tesseract {

/**
NOTE:  All circular parameters of all keys must be in the range

Min <= Param < Max

where Min and Max are specified in the KeyDesc parameter passed to
MakeKDTree.  All KD routines assume that this is true and will not operate
correctly if circular parameters outside the specified range are used.
*/

struct ClusteringContext;
struct CLUSTER;
struct KDTREE;

using kdwalk_proc = void (*)(ClusteringContext *context, CLUSTER *Cluster, int32_t Level);

struct KDNODE {
  /// This routine allocates memory for a new K-D tree node
  /// and places the specified Key and Data into it.  The
  /// left and right subtree pointers for the node are
  /// initialized to empty subtrees.
  /// @param tree  The tree to create the node for
  /// @param Key  Access key for new node in KD tree
  /// @param Data  ptr to data to be stored in new node
  /// @param Index  index of Key to branch on
  KDNODE() = default;
  KDNODE(KDTREE *tree, float key[], CLUSTER *data, int Index);
  ~KDNODE() {
    delete Left;
    delete Right;
  }

  float *Key;          /**< search key */
  CLUSTER *Data;       /**< data that corresponds to key */
  float BranchPoint;   /**< needed to make deletes work efficiently */
  float LeftBranch;    /**< used to optimize search pruning */
  float RightBranch;   /**< used to optimize search pruning */
  KDNODE *Left;        /**< ptrs for KD tree structure */
  KDNODE *Right;
};

struct KDTREE {
  KDTREE(size_t n) : KeySize(n), KeyDesc(n) {
  }

  // The destructor frees all memory which is allocated to the
  // specified KD-tree.  This includes the data structure for
  // the kd-tree itself plus the data structures for each node
  // in the tree.  It does not include the Key and Data items
  // which are pointed to by the nodes.  This memory is left
  // untouched.
  ~KDTREE() {
  }

  // TODO: KeySize might be replaced by KeyDesc.size().
  int16_t KeySize = 0;   // number of dimensions in the tree
  KDNODE Root;           // Root.Left points to actual root node
  std::vector<PARAM_DESC> KeyDesc; // description of each dimension
};

inline KDNODE::KDNODE(KDTREE *tree, float key[], CLUSTER *data, int Index) {
  Key = key;
  Data = data;
  BranchPoint = Key[Index];
  LeftBranch = tree->KeyDesc[Index].Min;
  RightBranch = tree->KeyDesc[Index].Max;
  Left = nullptr;
  Right = nullptr;
}

/*----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
#define RootOf(T) ((T)->Root.Left->Data)

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
KDTREE *MakeKDTree(int16_t KeySize, const PARAM_DESC KeyDesc[]);

void KDStore(KDTREE *Tree, float *Key, CLUSTER *Data);

void KDDelete(KDTREE *Tree, float Key[], void *Data);

void KDNearestNeighborSearch(KDTREE *Tree, float Query[], int QuerySize, float MaxDistance,
                             int *NumberOfResults, void **NBuffer, float DBuffer[]);

void KDWalk(KDTREE *Tree, kdwalk_proc Action, ClusteringContext *context);

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/

float DistanceSquared(int k, PARAM_DESC *dim, float p1[], float p2[]);

TESS_API
float ComputeDistance(int k, PARAM_DESC *dim, float p1[], float p2[]);

int QueryInSearch(KDTREE *tree);

void Walk(KDTREE *tree, kdwalk_proc action, ClusteringContext *context, KDNODE *SubTree, int32_t Level);

void InsertNodes(KDTREE *tree, KDNODE *nodes);

} // namespace tesseract

#endif
