/******************************************************************************
 **  Filename:  kdtree.cpp
 **  Purpose:   Routines for managing K-D search trees
 **  Author:    Dan Johnson
 **  History:  3/10/89, DSJ, Created.
 **      5/23/89, DSJ, Added circular feature capability.
 **      7/13/89, DSJ, Made tree nodes invisible to outside.
 **
 **  (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "kdtree.h"
#include "const.h"
#include "emalloc.h"
#include "freelist.h"
#include <stdio.h>
#include <math.h>

#define Magnitude(X)    ((X) < 0 ? -(X) : (X))
#define NodeFound(N,K,D)  (( (N)->Key == (K) ) && ( (N)->Data == (D) ))

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
#define MINSEARCH -MAX_FLOAT32
#define MAXSEARCH MAX_FLOAT32

// Helper function to find the next essential dimension in a cycle.
static int NextLevel(KDTREE *tree, int level) {
  do {
    ++level;
    if (level >= tree->KeySize)
      level = 0;
  } while (tree->KeyDesc[level].NonEssential);
  return level;
}

//-----------------------------------------------------------------------------
/**  Store the k smallest-keyed key-value pairs. */
template<typename Key, typename Value>
class MinK {
 public:
  MinK(Key max_key, int k);
  ~MinK();

  struct Element {
    Element() {}
    Element(const Key& k, const Value& v) : key(k), value(v) {}

    Key key;
    Value value;
  };

  bool insert(Key k, Value v);
  const Key& max_insertable_key();

  int elements_count() { return elements_count_; }
  const Element* elements() { return elements_; }

 private:
  const Key max_key_;  //< the maximum possible Key
  Element* elements_;  //< unsorted array of elements
  int elements_count_;  //< the number of results collected so far
  int k_;  //< the number of results we want from the search
  int max_index_;  //< the index of the result with the largest key
};

template<typename Key, typename Value>
MinK<Key, Value>::MinK(Key max_key, int k) :
  max_key_(max_key), elements_count_(0), k_(k < 1 ? 1 : k), max_index_(0) {
  elements_ = new Element[k_];
}

template<typename Key, typename Value>
MinK<Key, Value>::~MinK() {
  delete []elements_;
}

template<typename Key, typename Value>
const Key& MinK<Key, Value>::max_insertable_key() {
  if (elements_count_ < k_)
    return max_key_;
  return elements_[max_index_].key;
}

template<typename Key, typename Value>
bool MinK<Key, Value>::insert(Key key, Value value) {
  if (elements_count_ < k_) {
    elements_[elements_count_++] = Element(key, value);
    if (key > elements_[max_index_].key)
      max_index_ = elements_count_ - 1;
    return true;
  } else if (key < elements_[max_index_].key) {
    // evict the largest element.
    elements_[max_index_] = Element(key, value);
    // recompute max_index_
    for (int i = 0; i < elements_count_; i++) {
      if (elements_[i].key > elements_[max_index_].key)
        max_index_ = i;
    }
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------
/** Helper class for searching for the k closest points to query_point in tree. */
class KDTreeSearch {
 public:
  KDTreeSearch(KDTREE* tree, FLOAT32 *query_point, int k_closest);
  ~KDTreeSearch();

  /** Return the k nearest points' data. */
  void Search(int *result_count, FLOAT32 *distances, void **results);

 private:
  void SearchRec(int Level, KDNODE *SubTree);
  bool BoxIntersectsSearch(FLOAT32 *lower, FLOAT32 *upper);

  KDTREE *tree_;
  FLOAT32 *query_point_;
  MinK<FLOAT32, void *>* results_;
  FLOAT32 *sb_min_;  //< search box minimum
  FLOAT32 *sb_max_;  //< search box maximum
};

KDTreeSearch::KDTreeSearch(KDTREE* tree, FLOAT32 *query_point, int k_closest) :
    tree_(tree),
    query_point_(query_point) {
  results_ = new MinK<FLOAT32, void *>(MAXSEARCH, k_closest);
  sb_min_ = new FLOAT32[tree->KeySize];
  sb_max_ = new FLOAT32[tree->KeySize];
}

KDTreeSearch::~KDTreeSearch() {
  delete results_;
  delete[] sb_min_;
  delete[] sb_max_;
}

/// Locate the k_closest points to query_point_, and return their distances and
/// data into the given buffers.
void KDTreeSearch::Search(int *result_count,
                          FLOAT32 *distances,
                          void **results) {
  if (tree_->Root.Left == NULL) {
    *result_count = 0;
  } else {
    for (int i = 0; i < tree_->KeySize; i++) {
      sb_min_[i] = tree_->KeyDesc[i].Min;
      sb_max_[i] = tree_->KeyDesc[i].Max;
    }
    SearchRec(0, tree_->Root.Left);
    int count = results_->elements_count();
    *result_count = count;
    for (int j = 0; j < count; j++) {
      distances[j] = (FLOAT32) sqrt((FLOAT64)results_->elements()[j].key);
      results[j] = results_->elements()[j].value;
    }
  }
}

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/// @return a new KDTREE based on the specified parameters.
/// @param KeySize  # of dimensions in the K-D tree
/// @param KeyDesc  array of params to describe key dimensions
KDTREE *MakeKDTree(inT16 KeySize, const PARAM_DESC KeyDesc[]) {
  KDTREE *KDTree = (KDTREE *) Emalloc(
      sizeof(KDTREE) + (KeySize - 1) * sizeof(PARAM_DESC));
  for (int i = 0; i < KeySize; i++) {
    KDTree->KeyDesc[i].NonEssential = KeyDesc[i].NonEssential;
    KDTree->KeyDesc[i].Circular = KeyDesc[i].Circular;
    if (KeyDesc[i].Circular) {
      KDTree->KeyDesc[i].Min = KeyDesc[i].Min;
      KDTree->KeyDesc[i].Max = KeyDesc[i].Max;
      KDTree->KeyDesc[i].Range = KeyDesc[i].Max - KeyDesc[i].Min;
      KDTree->KeyDesc[i].HalfRange = KDTree->KeyDesc[i].Range / 2;
      KDTree->KeyDesc[i].MidRange = (KeyDesc[i].Max + KeyDesc[i].Min) / 2;
    } else {
      KDTree->KeyDesc[i].Min = MINSEARCH;
      KDTree->KeyDesc[i].Max = MAXSEARCH;
    }
  }
  KDTree->KeySize = KeySize;
  KDTree->Root.Left = NULL;
  KDTree->Root.Right = NULL;
  return KDTree;
}


/**
 * This routine stores Data in the K-D tree specified by Tree
 * using Key as an access key.
 *
 * @param Tree    K-D tree in which data is to be stored
 * @param Key    ptr to key by which data can be retrieved
 * @param Data    ptr to data to be stored in the tree
 *
 * @note Exceptions: none
 * @note History:  3/10/89, DSJ, Created.
 *      7/13/89, DSJ, Changed return to void.
 */
void KDStore(KDTREE *Tree, FLOAT32 *Key, void *Data) {
  int Level;
  KDNODE *Node;
  KDNODE **PtrToNode;

  PtrToNode = &(Tree->Root.Left);
  Node = *PtrToNode;
  Level = NextLevel(Tree, -1);
  while (Node != NULL) {
    if (Key[Level] < Node->BranchPoint) {
      PtrToNode = &(Node->Left);
      if (Key[Level] > Node->LeftBranch)
        Node->LeftBranch = Key[Level];
    }
    else {
      PtrToNode = &(Node->Right);
      if (Key[Level] < Node->RightBranch)
        Node->RightBranch = Key[Level];
    }
    Level = NextLevel(Tree, Level);
    Node = *PtrToNode;
  }

  *PtrToNode = MakeKDNode(Tree, Key, (void *) Data, Level);
}                                /* KDStore */


/**
 * This routine deletes a node from Tree.  The node to be	
 * deleted is specified by the Key for the node and the Data	
 * contents of the node.  These two pointers must be identical	
 * to the pointers that were used for the node when it was	
 * originally stored in the tree.  A node will be deleted from	
 * the tree only if its key and data pointers are identical	
 * to Key and Data respectively.  The tree is re-formed by removing
 * the affected subtree and inserting all elements but the root.
 *
 * @param Tree K-D tree to delete node from
 * @param Key key of node to be deleted
 * @param Data data contents of node to be deleted
 *
 * @note Exceptions: none
 *
 * @note History:  3/13/89, DSJ, Created.
 *                7/13/89, DSJ, Specify node indirectly by key and data.
 */
void
KDDelete (KDTREE * Tree, FLOAT32 Key[], void *Data) {
  int Level;
  KDNODE *Current;
  KDNODE *Father;

  /* initialize search at root of tree */
  Father = &(Tree->Root);
  Current = Father->Left;
  Level = NextLevel(Tree, -1);

  /* search tree for node to be deleted */
  while ((Current != NULL) && (!NodeFound (Current, Key, Data))) {
    Father = Current;
    if (Key[Level] < Current->BranchPoint)
      Current = Current->Left;
    else
      Current = Current->Right;

    Level = NextLevel(Tree, Level);
  }

  if (Current != NULL) {         /* if node to be deleted was found */
    if (Current == Father->Left) {
      Father->Left = NULL;
      Father->LeftBranch = Tree->KeyDesc[Level].Min;
    } else {
      Father->Right = NULL;
      Father->RightBranch = Tree->KeyDesc[Level].Max;
    }

    InsertNodes(Tree, Current->Left);
    InsertNodes(Tree, Current->Right);
    FreeSubTree(Current);
  }
}                                /* KDDelete */


/**
 * This routine searches the K-D tree specified by Tree and
 * finds the QuerySize nearest neighbors of Query.  All neighbors
 * must be within MaxDistance of Query.  The data contents of
 * the nearest neighbors
 * are placed in NBuffer and their distances from Query are
 * placed in DBuffer.
 * @param Tree    ptr to K-D tree to be searched
 * @param Query    ptr to query key (point in D-space)
 * @param QuerySize  number of nearest neighbors to be found
 * @param MaxDistance  all neighbors must be within this distance
 * @param NBuffer ptr to QuerySize buffer to hold nearest neighbors
 * @param DBuffer ptr to QuerySize buffer to hold distances
 *          from nearest neighbor to query point
 * @param NumberOfResults [out] Number of nearest neighbors actually found
 * @note Exceptions: none
 * @note History:
 * - 3/10/89, DSJ, Created.
 * - 7/13/89, DSJ, Return contents of node instead of node itself.
 */
void KDNearestNeighborSearch(
    KDTREE *Tree, FLOAT32 Query[], int QuerySize, FLOAT32 MaxDistance,
    int *NumberOfResults, void **NBuffer, FLOAT32 DBuffer[]) {
  KDTreeSearch search(Tree, Query, QuerySize);
  search.Search(NumberOfResults, DBuffer, NBuffer);
}


/*---------------------------------------------------------------------------*/
/** Walk a given Tree with action. */
void KDWalk(KDTREE *Tree, void_proc action, void *context) {
  if (Tree->Root.Left != NULL)
    Walk(Tree, action, context, Tree->Root.Left, NextLevel(Tree, -1));
}


/*---------------------------------------------------------------------------*/
/**
 * This routine frees all memory which is allocated to the
 * specified KD-tree.  This includes the data structure for
 * the kd-tree itself plus the data structures for each node
 * in the tree.  It does not include the Key and Data items
 * which are pointed to by the nodes.  This memory is left
 * untouched.
 * @param Tree  tree data structure to be released
 * @return none
 * @note Exceptions: none
 * @note History: 5/26/89, DSJ, Created.
 */
void FreeKDTree(KDTREE *Tree) {
  FreeSubTree(Tree->Root.Left);
  memfree(Tree);
}                                /* FreeKDTree */


/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine allocates memory for a new K-D tree node
 * and places the specified Key and Data into it.  The
 * left and right subtree pointers for the node are
 * initialized to empty subtrees.
 * @param tree  The tree to create the node for
 * @param Key  Access key for new node in KD tree
 * @param Data  ptr to data to be stored in new node
 * @param Index  index of Key to branch on
 * @return pointer to new K-D tree node
 * @note Exceptions: None
 * @note History: 3/11/89, DSJ, Created.
 */
KDNODE *MakeKDNode(KDTREE *tree, FLOAT32 Key[], void *Data, int Index) {
  KDNODE *NewNode;

  NewNode = (KDNODE *) Emalloc (sizeof (KDNODE));

  NewNode->Key = Key;
  NewNode->Data = Data;
  NewNode->BranchPoint = Key[Index];
  NewNode->LeftBranch = tree->KeyDesc[Index].Min;
  NewNode->RightBranch = tree->KeyDesc[Index].Max;
  NewNode->Left = NULL;
  NewNode->Right = NULL;

  return NewNode;
}                                /* MakeKDNode */


/*---------------------------------------------------------------------------*/
void FreeKDNode(KDNODE *Node) {
  memfree ((char *)Node);
}


/*---------------------------------------------------------------------------*/
/**
 * Recursively accumulate the k_closest points to query_point_ into results_.
 * @param Level  level in tree of sub-tree to be searched
 * @param SubTree  sub-tree to be searched
 */
void KDTreeSearch::SearchRec(int level, KDNODE *sub_tree) {
  if (level >= tree_->KeySize)
    level = 0;

  if (!BoxIntersectsSearch(sb_min_, sb_max_))
    return;

  results_->insert(DistanceSquared(tree_->KeySize, tree_->KeyDesc,
                                  query_point_, sub_tree->Key),
                   sub_tree->Data);

  if (query_point_[level] < sub_tree->BranchPoint) {
    if (sub_tree->Left != NULL) {
      FLOAT32 tmp = sb_max_[level];
      sb_max_[level] = sub_tree->LeftBranch;
      SearchRec(NextLevel(tree_, level), sub_tree->Left);
      sb_max_[level] = tmp;
    }
    if (sub_tree->Right != NULL) {
      FLOAT32 tmp = sb_min_[level];
      sb_min_[level] = sub_tree->RightBranch;
      SearchRec(NextLevel(tree_, level), sub_tree->Right);
      sb_min_[level] = tmp;
    }
  } else {
    if (sub_tree->Right != NULL) {
      FLOAT32 tmp = sb_min_[level];
      sb_min_[level] = sub_tree->RightBranch;
      SearchRec(NextLevel(tree_, level), sub_tree->Right);
      sb_min_[level] = tmp;
    }
    if (sub_tree->Left != NULL) {
      FLOAT32 tmp = sb_max_[level];
      sb_max_[level] = sub_tree->LeftBranch;
      SearchRec(NextLevel(tree_, level), sub_tree->Left);
      sb_max_[level] = tmp;
    }
  }
}


/*---------------------------------------------------------------------------*/
/** 
 *Returns the Euclidean distance squared between p1 and p2 for all essential
 * dimensions.
 * @param k      keys are in k-space
 * @param dim    dimension descriptions (essential, circular, etc)
 * @param p1,p2  two different points in K-D space
 */
FLOAT32 DistanceSquared(int k, PARAM_DESC *dim, FLOAT32 p1[], FLOAT32 p2[]) {
  FLOAT32 total_distance = 0;

  for (; k > 0; k--, p1++, p2++, dim++) {
    if (dim->NonEssential)
      continue;

    FLOAT32 dimension_distance = *p1 - *p2;

    /* if this dimension is circular - check wraparound distance */
    if (dim->Circular) {
      dimension_distance = Magnitude(dimension_distance);
      FLOAT32 wrap_distance = dim->Max - dim->Min - dimension_distance;
      dimension_distance = MIN(dimension_distance, wrap_distance);
    }

    total_distance += dimension_distance * dimension_distance;
  }
  return total_distance;
}

FLOAT32 ComputeDistance(int k, PARAM_DESC *dim, FLOAT32 p1[], FLOAT32 p2[]) {
  return sqrt(DistanceSquared(k, dim, p1, p2));
}

/*---------------------------------------------------------------------------*/
/// Return whether the query region (the smallest known circle about
/// query_point_ containing results->k_ points) intersects the box specified
/// between lower and upper.  For circular dimensions, we also check the point
/// one wrap distance away from the query.
bool KDTreeSearch::BoxIntersectsSearch(FLOAT32 *lower, FLOAT32 *upper) {
  FLOAT32 *query = query_point_;
  FLOAT64 total_distance = 0.0;
  FLOAT64 radius_squared =
      results_->max_insertable_key() * results_->max_insertable_key();
  PARAM_DESC *dim = tree_->KeyDesc;

  for (int i = tree_->KeySize; i > 0; i--, dim++, query++, lower++, upper++) {
    if (dim->NonEssential)
      continue;

    FLOAT32 dimension_distance;
    if (*query < *lower)
      dimension_distance = *lower - *query;
    else if (*query > *upper)
      dimension_distance = *query - *upper;
    else
      dimension_distance = 0;

    /* if this dimension is circular - check wraparound distance */
    if (dim->Circular) {
      FLOAT32 wrap_distance = MAX_FLOAT32;
      if (*query < *lower)
        wrap_distance = *query + dim->Max - dim->Min - *upper;
      else if (*query > *upper)
        wrap_distance = *lower - (*query - (dim->Max - dim->Min));
      dimension_distance = MIN(dimension_distance, wrap_distance);
    }

    total_distance += dimension_distance * dimension_distance;
    if (total_distance >= radius_squared)
      return FALSE;
  }
  return TRUE;
}


/*---------------------------------------------------------------------------*/
/**
 * Walk a tree, calling action once on each node.
 *
 * Operation:
 *   This routine walks thru the specified sub_tree and invokes action
 *   action at each node as follows:
 *       action(context, data, level)
 *   data  the data contents of the node being visited,
 *   level is the level of the node in the tree with the root being level 0.
 * @param tree  root of the tree being walked.
 * @param action  action to be performed at every node
 * @param context  action's context
 * @param sub_tree  ptr to root of subtree to be walked
 * @param level  current level in the tree for this node
 */
void Walk(KDTREE *tree, void_proc action, void *context,
          KDNODE *sub_tree, inT32 level) {
  (*action)(context, sub_tree->Data, level);
  if (sub_tree->Left != NULL)
    Walk(tree, action, context, sub_tree->Left, NextLevel(tree, level));
  if (sub_tree->Right != NULL)
    Walk(tree, action, context, sub_tree->Right, NextLevel(tree, level));
}


/** Given a subtree nodes, insert all of its elements into tree. */
void InsertNodes(KDTREE *tree, KDNODE *nodes) {
  if (nodes == NULL)
    return;

  KDStore(tree, nodes->Key, nodes->Data);
  InsertNodes(tree, nodes->Left);
  InsertNodes(tree, nodes->Right);
}

/** Free all of the nodes of a sub tree. */
void FreeSubTree(KDNODE *sub_tree) {
  if (sub_tree != NULL) {
    FreeSubTree(sub_tree->Left);
    FreeSubTree(sub_tree->Right);
    memfree(sub_tree);
  }
}
