/******************************************************************************
 **	Filename:	kdtree.c
 **	Purpose:	Routines for managing K-D search trees
 **	Author:		Dan Johnson
 **	History:	3/10/89, DSJ, Created.
 **			5/23/89, DSJ, Added circular feature capability.
 **			7/13/89, DSJ, Made tree nodes invisible to outside.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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
#include <setjmp.h>

#define Magnitude(X)    ((X) < 0 ? -(X) : (X))
#define MIN(A,B)    ((A) < (B) ? (A) : (B))
#define NodeFound(N,K,D)  (( (N)->Key == (K) ) && ( (N)->Data == (D) ))

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
#define MINSEARCH -MAX_FLOAT32
#define MAXSEARCH MAX_FLOAT32

static int NumberOfNeighbors;
static inT16 N;                  /* number of dimensions in the kd tree */

static FLOAT32 *QueryPoint;
static int MaxNeighbors;
static FLOAT32 Radius;
static int Furthest;
static char **Neighbor;
static FLOAT32 *Distance;

static int MaxDimension = 0;
static FLOAT32 *SBMin;
static FLOAT32 *SBMax;
static FLOAT32 *LBMin;
static FLOAT32 *LBMax;

static PARAM_DESC *KeyDesc;

static jmp_buf QuickExit;

static void_proc WalkAction;

// Helper function to find the next essential dimension in a cycle.
static int NextLevel(int level) {
  do {
    ++level;
    if (level >= N)
      level = 0;
  } while (KeyDesc[level].NonEssential);
  return level;
}

/// Helper function to find the previous essential dimension in a cycle.
static int PrevLevel(int level) {
  do {
    --level;
    if (level < 0)
      level = N - 1;
  } while (KeyDesc[level].NonEssential);
  return level;
}

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine allocates and returns a new K-D tree data
 * structure.  It also reallocates the small and large
 * search region boxes if they are not large enough to
 * accomodate the size of the new K-D tree.  KeyDesc is
 * an array of key descriptors that indicate which dimensions
 * are circular and, if they are circular, what the range is.
 *
 * Globals:
 * - MaxDimension	largest # of dimensions in any K-D tree
 * - SBMin		small search region box
 * - SBMax
 * - LBMin		large search region box
 * - LBMax
 * - Key		description of key dimensions
 *
 * @param KeySize # of dimensions in the K-D tree
 * @param KeyDesc array of params to describe key dimensions
 *
 * @return Pointer to new K-D tree
 * @note Exceptions: None
 * @note History: 3/13/89, DSJ, Created.
 */
KDTREE *
MakeKDTree (inT16 KeySize, PARAM_DESC KeyDesc[]) {
  int i;
  void *NewMemory;
  KDTREE *KDTree;

  if (KeySize > MaxDimension) {
    NewMemory = Emalloc (KeySize * 4 * sizeof (FLOAT32));
    if (MaxDimension > 0) {
      memfree ((char *) SBMin);
      memfree ((char *) SBMax);
      memfree ((char *) LBMin);
      memfree ((char *) LBMax);
    }
    SBMin = (FLOAT32 *) NewMemory;
    SBMax = SBMin + KeySize;
    LBMin = SBMax + KeySize;
    LBMax = LBMin + KeySize;
  }

  KDTree =
    (KDTREE *) Emalloc (sizeof (KDTREE) +
    (KeySize - 1) * sizeof (PARAM_DESC));
  for (i = 0; i < KeySize; i++) {
    KDTree->KeyDesc[i].NonEssential = KeyDesc[i].NonEssential;
    KDTree->KeyDesc[i].Circular = KeyDesc[i].Circular;
    if (KeyDesc[i].Circular) {
      KDTree->KeyDesc[i].Min = KeyDesc[i].Min;
      KDTree->KeyDesc[i].Max = KeyDesc[i].Max;
      KDTree->KeyDesc[i].Range = KeyDesc[i].Max - KeyDesc[i].Min;
      KDTree->KeyDesc[i].HalfRange = KDTree->KeyDesc[i].Range / 2;
      KDTree->KeyDesc[i].MidRange = (KeyDesc[i].Max + KeyDesc[i].Min) / 2;
    }
    else {
      KDTree->KeyDesc[i].Min = MINSEARCH;
      KDTree->KeyDesc[i].Max = MAXSEARCH;
    }
  }
  KDTree->KeySize = KeySize;
  KDTree->Root.Left = NULL;
  KDTree->Root.Right = NULL;
  return (KDTree);
}                                /* MakeKDTree */


/*---------------------------------------------------------------------------*/
void KDStore(KDTREE *Tree, FLOAT32 *Key, void *Data) {
/**
 * This routine stores Data in the K-D tree specified by Tree
 * using Key as an access key.
 *
 * @param Tree		K-D tree in which data is to be stored
 * @param Key		ptr to key by which data can be retrieved
 * @param Data		ptr to data to be stored in the tree
 *
 * Globals:
 * - N		dimension of the K-D tree
 * - KeyDesc		descriptions of tree dimensions
 * - StoreCount	debug variables for performance tests
 * - StoreUniqueCount
 * - StoreProbeCount
 *
 * @note Exceptions: none
 * @note History:	3/10/89, DSJ, Created.
 *			7/13/89, DSJ, Changed return to void.
 */
  int Level;
  KDNODE *Node;
  KDNODE **PtrToNode;

  N = Tree->KeySize;
  KeyDesc = &(Tree->KeyDesc[0]);
  PtrToNode = &(Tree->Root.Left);
  Node = *PtrToNode;
  Level = NextLevel(-1);
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
    Level = NextLevel(Level);
    Node = *PtrToNode;
  }

  *PtrToNode = MakeKDNode (Key, (char *) Data, Level);
}                                /* KDStore */


/*---------------------------------------------------------------------------*/
/**
 * This routine deletes a node from Tree.  The node to be
 * deleted is specified by the Key for the node and the Data
 * contents of the node.  These two pointers must be identical
 * to the pointers that were used for the node when it was
 * originally stored in the tree.  A node will be deleted from
 * the tree only if its key and data pointers are identical
 * to Key and Data respectively.  The empty space left in the tree
 * is filled by pulling a leaf up from the bottom of one of
 * the subtrees of the node being deleted.  The leaf node will
 * be pulled from left subtrees whenever possible (this was
 * an arbitrary decision).  No attempt is made to pull the leaf
 * from the deepest subtree (to minimize length).  The branch
 * point for the replacement node is changed to be the same as
 * the branch point of the deleted node.  This keeps us from
 * having to rearrange the tree every time we delete a node.
 * Also, the LeftBranch and RightBranch numbers of the
 * replacement node are set to be the same as the deleted node.
 * The makes the delete easier and more efficient, but it may
 * make searches in the tree less efficient after many nodes are
 * deleted.  If the node specified by Key and Data does not
 * exist in the tree, then nothing is done.
 *
 * Globals:
 * - N		dimension of the K-D tree
 * - KeyDesc		description of each dimension
 * - DeleteCount	debug variables for performance tests
 * - DeleteProbeCount
 *
 * @param Tree K-D tree to delete node from
 * @param Key key of node to be deleted
 * @param Data data contents of node to be deleted
 *
 * @note Exceptions: none
 *
 * @note History:	3/13/89, DSJ, Created.
 *    		        7/13/89, DSJ, Specify node indirectly by key and data.
 */
void
KDDelete (KDTREE * Tree, FLOAT32 Key[], void *Data) {
  int Level;
  KDNODE *Current;
  KDNODE *Father;
  KDNODE *Replacement;
  KDNODE *FatherReplacement;

  /* initialize search at root of tree */
  N = Tree->KeySize;
  KeyDesc = &(Tree->KeyDesc[0]);
  Father = &(Tree->Root);
  Current = Father->Left;
  Level = NextLevel(-1);

  /* search tree for node to be deleted */
  while ((Current != NULL) && (!NodeFound (Current, Key, Data))) {
    Father = Current;
    if (Key[Level] < Current->BranchPoint)
      Current = Current->Left;
    else
      Current = Current->Right;

    Level = NextLevel(Level);
  }

  if (Current != NULL) {         /* if node to be deleted was found */
    Replacement = Current;
    FatherReplacement = Father;

    /* search for replacement node (a leaf under node to be deleted */
    while (TRUE) {
      if (Replacement->Left != NULL) {
        FatherReplacement = Replacement;
        Replacement = Replacement->Left;
      }
      else if (Replacement->Right != NULL) {
        FatherReplacement = Replacement;
        Replacement = Replacement->Right;
      }
      else
        break;

      Level = NextLevel(Level);
    }

    /* compute level of replacement node's father */
    Level = PrevLevel(Level);

    /* disconnect replacement node from it's father */
    if (FatherReplacement->Left == Replacement) {
      FatherReplacement->Left = NULL;
      FatherReplacement->LeftBranch = KeyDesc[Level].Min;
    }
    else {
      FatherReplacement->Right = NULL;
      FatherReplacement->RightBranch = KeyDesc[Level].Max;
    }

    /* replace deleted node with replacement (unless they are the same) */
    if (Replacement != Current) {
      Replacement->BranchPoint = Current->BranchPoint;
      Replacement->LeftBranch = Current->LeftBranch;
      Replacement->RightBranch = Current->RightBranch;
      Replacement->Left = Current->Left;
      Replacement->Right = Current->Right;

      if (Father->Left == Current)
        Father->Left = Replacement;
      else
        Father->Right = Replacement;
    }
    FreeKDNode(Current);
  }
}                                /* KDDelete */


/*---------------------------------------------------------------------------*/
int
KDNearestNeighborSearch (KDTREE * Tree,
FLOAT32 Query[],
int QuerySize,
FLOAT32 MaxDistance,
void *NBuffer, FLOAT32 DBuffer[]) {
/*
 **	Parameters:
 **		Tree		ptr to K-D tree to be searched
 **		Query		ptr to query key (point in D-space)
 **		QuerySize	number of nearest neighbors to be found
 **		MaxDistance	all neighbors must be within this distance
 **		NBuffer		ptr to QuerySize buffer to hold nearest neighbors
 **		DBuffer		ptr to QuerySize buffer to hold distances
 **					from nearest neighbor to query point
 **	Globals:
 **		NumberOfNeighbors	# of neighbors found so far
 **		N			# of features in each key
 **		KeyDesc			description of tree dimensions
 **		QueryPoint		point in D-space to find neighbors of
 **		MaxNeighbors		maximum # of neighbors to find
 **		Radius			current distance of furthest neighbor
 **		Furthest		index of furthest neighbor
 **		Neighbor		buffer of current neighbors
 **		Distance		buffer of neighbor distances
 **		SBMin			lower extent of small search region
 **		SBMax			upper extent of small search region
 **		LBMin			lower extent of large search region
 **		LBMax			upper extent of large search region
 **		QuickExit		quick exit from recursive search
 **	Operation:
 **		This routine searches the K-D tree specified by Tree and
 **		finds the QuerySize nearest neighbors of Query.  All neighbors
 **		must be within MaxDistance of Query.  The data contents of
 **		the nearest neighbors
 **		are placed in NBuffer and their distances from Query are
 **		placed in DBuffer.
 **	Return: Number of nearest neighbors actually found
 **	Exceptions: none
 **	History:
 **		3/10/89, DSJ, Created.
 **		7/13/89, DSJ, Return contents of node instead of node itself.
 */
  int i;

  NumberOfNeighbors = 0;
  N = Tree->KeySize;
  KeyDesc = &(Tree->KeyDesc[0]);
  QueryPoint = Query;
  MaxNeighbors = QuerySize;
  Radius = MaxDistance;
  Furthest = 0;
  Neighbor = (char **) NBuffer;
  Distance = DBuffer;

  for (i = 0; i < N; i++) {
    SBMin[i] = KeyDesc[i].Min;
    SBMax[i] = KeyDesc[i].Max;
    LBMin[i] = KeyDesc[i].Min;
    LBMax[i] = KeyDesc[i].Max;
  }

  if (Tree->Root.Left != NULL) {
    if (setjmp (QuickExit) == 0)
      Search (0, Tree->Root.Left);
  }
  return (NumberOfNeighbors);
}                                /* KDNearestNeighborSearch */


/*---------------------------------------------------------------------------*/
void KDWalk(KDTREE *Tree, void_proc Action) {
/*
 **	Parameters:
 **		Tree	ptr to K-D tree to be walked
 **		Action	ptr to function to be executed at each node
 **	Globals:
 **		WalkAction	action to be performed at every node
 **	Operation:
 **		This routine stores the desired action in a global
 **		variable and starts a recursive walk of Tree.  The walk
 **		is started at the root node.
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		3/13/89, DSJ, Created.
 */
  WalkAction = Action;
  if (Tree->Root.Left != NULL)
    Walk (Tree->Root.Left, NextLevel(-1));
}                                /* KDWalk */


/*---------------------------------------------------------------------------*/
void FreeKDTree(KDTREE *Tree) {
/*
 **	Parameters:
 **		Tree	tree data structure to be released
 **	Globals: none
 **	Operation:
 **		This routine frees all memory which is allocated to the
 **		specified KD-tree.  This includes the data structure for
 **		the kd-tree itself plus the data structures for each node
 **		in the tree.  It does not include the Key and Data items
 **		which are pointed to by the nodes.  This memory is left
 **		untouched.
 **	Return: none
 **	Exceptions: none
 **	History:
 **		5/26/89, DSJ, Created.
 */
  FreeSubTree (Tree->Root.Left);
  memfree(Tree);
}                                /* FreeKDTree */


/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int
Equal (FLOAT32 Key1[], FLOAT32 Key2[]) {
/*
 **	Parameters:
 **		Key1,Key2	search keys to be compared for equality
 **	Globals:
 **		N		number of parameters per key
 **	Operation:
 **		This routine returns TRUE if Key1 = Key2.
 **	Return:
 **		TRUE if Key1 = Key2, else FALSE.
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  int i;

  for (i = N; i > 0; i--, Key1++, Key2++)
    if (*Key1 != *Key2)
      return (FALSE);
  return (TRUE);
}                                /* Equal */


/*---------------------------------------------------------------------------*/
KDNODE *
MakeKDNode (FLOAT32 Key[], char *Data, int Index) {
/*
 **	Parameters:
 **		Key	Access key for new node in KD tree
 **		Data	ptr to data to be stored in new node
 **		Index	index of Key to branch on
 **	Globals:
 **		KeyDesc	descriptions of key dimensions
 **	Operation:
 **		This routine allocates memory for a new K-D tree node
 **		and places the specified Key and Data into it.  The
 **		left and right subtree pointers for the node are
 **		initialized to empty subtrees.
 **	Return:
 **		pointer to new K-D tree node
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  KDNODE *NewNode;

  NewNode = (KDNODE *) Emalloc (sizeof (KDNODE));

  NewNode->Key = Key;
  NewNode->Data = Data;
  NewNode->BranchPoint = Key[Index];
  NewNode->LeftBranch = KeyDesc[Index].Min;
  NewNode->RightBranch = KeyDesc[Index].Max;
  NewNode->Left = NULL;
  NewNode->Right = NULL;

  return (NewNode);
}                                /* MakeKDNode */


/*---------------------------------------------------------------------------*/
void FreeKDNode(KDNODE *Node) {
/*
 **	Parameters:
 **		Node	ptr to node data structure to be freed
 **	Globals:
 **		None
 **	Operation:
 **		This routine frees up the memory allocated to Node.
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		3/13/89, DSJ, Created.
 */
  memfree ((char *) Node);
}                                /* FreeKDNode */


/*---------------------------------------------------------------------------*/
void Search(int Level, KDNODE *SubTree) {
/*
 **	Parameters:
 **		Level		level in tree of sub-tree to be searched
 **		SubTree		sub-tree to be searched
 **	Globals:
 **		NumberOfNeighbors	# of neighbors found so far
 **		N			# of features in each key
 **		KeyDesc			description of key dimensions
 **		QueryPoint		point in D-space to find neighbors of
 **		MaxNeighbors		maximum # of neighbors to find
 **		Radius			current distance of furthest neighbor
 **		Furthest		index of furthest neighbor
 **		Neighbor		buffer of current neighbors
 **		Distance		buffer of neighbor distances
 **		SBMin			lower extent of small search region
 **		SBMax			upper extent of small search region
 **		LBMin			lower extent of large search region
 **		LBMax			upper extent of large search region
 **		QuickExit		quick exit from recursive search
 **	Operation:
 **		This routine searches SubTree for those entries which are
 **		possibly among the MaxNeighbors nearest neighbors of the
 **		QueryPoint and places their data in the Neighbor buffer and
 **		their distances from QueryPoint in the Distance buffer.
 **	Return: none
 **	Exceptions: none
 **	History:
 **		3/11/89, DSJ, Created.
 **		7/13/89, DSJ, Save node contents, not node, in neighbor buffer
 */
  FLOAT32 d;
  FLOAT32 OldSBoxEdge;
  FLOAT32 OldLBoxEdge;

  if (Level >= N)
    Level = 0;

  d = ComputeDistance (N, KeyDesc, QueryPoint, SubTree->Key);
  if (d < Radius) {
    if (NumberOfNeighbors < MaxNeighbors) {
      Neighbor[NumberOfNeighbors] = SubTree->Data;
      Distance[NumberOfNeighbors] = d;
      NumberOfNeighbors++;
      if (NumberOfNeighbors == MaxNeighbors)
        FindMaxDistance();
    }
    else {
      Neighbor[Furthest] = SubTree->Data;
      Distance[Furthest] = d;
      FindMaxDistance();
    }
  }
  if (QueryPoint[Level] < SubTree->BranchPoint) {
    OldSBoxEdge = SBMax[Level];
    SBMax[Level] = SubTree->LeftBranch;
    OldLBoxEdge = LBMax[Level];
    LBMax[Level] = SubTree->RightBranch;
    if (SubTree->Left != NULL)
      Search (NextLevel(Level), SubTree->Left);
    SBMax[Level] = OldSBoxEdge;
    LBMax[Level] = OldLBoxEdge;
    OldSBoxEdge = SBMin[Level];
    SBMin[Level] = SubTree->RightBranch;
    OldLBoxEdge = LBMin[Level];
    LBMin[Level] = SubTree->LeftBranch;
    if ((SubTree->Right != NULL) && QueryIntersectsSearch ())
      Search (NextLevel(Level), SubTree->Right);
    SBMin[Level] = OldSBoxEdge;
    LBMin[Level] = OldLBoxEdge;
  }
  else {
    OldSBoxEdge = SBMin[Level];
    SBMin[Level] = SubTree->RightBranch;
    OldLBoxEdge = LBMin[Level];
    LBMin[Level] = SubTree->LeftBranch;
    if (SubTree->Right != NULL)
      Search (NextLevel(Level), SubTree->Right);
    SBMin[Level] = OldSBoxEdge;
    LBMin[Level] = OldLBoxEdge;
    OldSBoxEdge = SBMax[Level];
    SBMax[Level] = SubTree->LeftBranch;
    OldLBoxEdge = LBMax[Level];
    LBMax[Level] = SubTree->RightBranch;
    if ((SubTree->Left != NULL) && QueryIntersectsSearch ())
      Search (NextLevel(Level), SubTree->Left);
    SBMax[Level] = OldSBoxEdge;
    LBMax[Level] = OldLBoxEdge;
  }
  if (QueryInSearch ())
    longjmp (QuickExit, 1);
}                                /* Search */


/*---------------------------------------------------------------------------*/
FLOAT32
ComputeDistance (register int N,
register PARAM_DESC Dim[],
register FLOAT32 p1[], register FLOAT32 p2[]) {
/*
 **	Parameters:
 **		N		number of dimensions in K-D space
 **		Dim		descriptions of each dimension
 **		p1,p2		two different points in K-D space
 **	Globals:
 **		None
 **	Operation:
 **		This routine computes the euclidian distance
 **		between p1 and p2 in K-D space (an N dimensional space).
 **	Return:
 **		Distance between p1 and p2.
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  register FLOAT32 TotalDistance;
  register FLOAT32 DimensionDistance;
  FLOAT32 WrapDistance;

  TotalDistance = 0;
  for (; N > 0; N--, p1++, p2++, Dim++) {
    if (Dim->NonEssential)
      continue;

    DimensionDistance = *p1 - *p2;

    /* if this dimension is circular - check wraparound distance */
    if (Dim->Circular) {
      DimensionDistance = Magnitude (DimensionDistance);
      WrapDistance = Dim->Max - Dim->Min - DimensionDistance;
      DimensionDistance = MIN (DimensionDistance, WrapDistance);
    }

    TotalDistance += DimensionDistance * DimensionDistance;
  }
  return ((FLOAT32) sqrt ((FLOAT64) TotalDistance));
}                                /* ComputeDistance */


/*---------------------------------------------------------------------------*/
void FindMaxDistance() {
/*
 **	Parameters:
 **		None
 **	Globals:
 **		MaxNeighbors		maximum # of neighbors to find
 **		Radius			current distance of furthest neighbor
 **		Furthest		index of furthest neighbor
 **		Distance		buffer of neighbor distances
 **	Operation:
 **		This routine searches the Distance buffer for the maximum
 **		distance, places this distance in Radius, and places the
 **		index of this distance in Furthest.
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  int i;

  Radius = Distance[Furthest];
  for (i = 0; i < MaxNeighbors; i++) {
    if (Distance[i] > Radius) {
      Radius = Distance[i];
      Furthest = i;
    }
  }
}                                /* FindMaxDistance */


/*---------------------------------------------------------------------------*/
int QueryIntersectsSearch() {
/*
 **	Parameters:
 **		None
 **	Globals:
 **		N			# of features in each key
 **		KeyDesc			descriptions of each dimension
 **		QueryPoint		point in D-space to find neighbors of
 **		Radius			current distance of furthest neighbor
 **		SBMin			lower extent of small search region
 **		SBMax			upper extent of small search region
 **	Operation:
 **		This routine returns TRUE if the query region intersects
 **		the current smallest search region.  The query region is
 **		the circle of radius Radius centered at QueryPoint.
 **		The smallest search region is the box (in N dimensions)
 **		whose edges in each dimension are specified by SBMin and SBMax.
 **		In the case of circular dimensions, we must also check the
 **		point which is one wrap-distance away from the query to
 **		see if it would intersect the search region.
 **	Return:
 **		TRUE if query region intersects search region, else FALSE
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  register int i;
  register FLOAT32 *Query;
  register FLOAT32 *Lower;
  register FLOAT32 *Upper;
  register FLOAT64 TotalDistance;
  register FLOAT32 DimensionDistance;
  register FLOAT64 RadiusSquared;
  register PARAM_DESC *Dim;
  register FLOAT32 WrapDistance;

  RadiusSquared = Radius * Radius;
  Query = QueryPoint;
  Lower = SBMin;
  Upper = SBMax;
  TotalDistance = 0.0;
  Dim = KeyDesc;
  for (i = N; i > 0; i--, Dim++, Query++, Lower++, Upper++) {
    if (Dim->NonEssential)
      continue;

    if (*Query < *Lower)
      DimensionDistance = *Lower - *Query;
    else if (*Query > *Upper)
      DimensionDistance = *Query - *Upper;
    else
      DimensionDistance = 0;

    /* if this dimension is circular - check wraparound distance */
    if (Dim->Circular) {
      if (*Query < *Lower)
        WrapDistance = *Query + Dim->Max - Dim->Min - *Upper;
      else if (*Query > *Upper)
        WrapDistance = *Lower - (*Query - (Dim->Max - Dim->Min));
      else
        WrapDistance = MAX_FLOAT32;

      DimensionDistance = MIN (DimensionDistance, WrapDistance);
    }

    TotalDistance += DimensionDistance * DimensionDistance;
    if (TotalDistance >= RadiusSquared)
      return (FALSE);
  }
  return (TRUE);
}                                /* QueryIntersectsSearch */


/*---------------------------------------------------------------------------*/
int QueryInSearch() {
/*
 **	Parameters:
 **		None
 **	Globals:
 **		N			# of features in each key
 **		KeyDesc			descriptions of each dimension
 **		QueryPoint		point in D-space to find neighbors of
 **		Radius			current distance of furthest neighbor
 **		LBMin			lower extent of large search region
 **		LBMax			upper extent of large search region
 **	Operation:
 **		This routine returns TRUE if the current query region is
 **		totally contained in the current largest search region.
 **		The query region is the circle of
 **		radius Radius centered at QueryPoint.  The search region is
 **		the box (in N dimensions) whose edges in each
 **		dimension are specified by LBMin and LBMax.
 **	Return:
 **		TRUE if query region is inside search region, else FALSE
 **	Exceptions:
 **		None
 **	History:
 **		3/11/89, DSJ, Created.
 */
  register int i;
  register FLOAT32 *Query;
  register FLOAT32 *Lower;
  register FLOAT32 *Upper;
  register PARAM_DESC *Dim;

  Query = QueryPoint;
  Lower = LBMin;
  Upper = LBMax;
  Dim = KeyDesc;

  for (i = N - 1; i >= 0; i--, Dim++, Query++, Lower++, Upper++) {
    if (Dim->NonEssential)
      continue;

    if ((*Query < *Lower + Radius) || (*Query > *Upper - Radius))
      return (FALSE);
  }
  return (TRUE);
}                                /* QueryInSearch */


/*---------------------------------------------------------------------------*/
void Walk(KDNODE *SubTree, inT32 Level) {
/*
 **	Parameters:
 **		SubTree		ptr to root of subtree to be walked
 **		Level		current level in the tree for this node
 **	Globals:
 **		WalkAction	action to be performed at every node
 **	Operation:
 **		This routine walks thru the specified SubTree and invokes
 **		WalkAction at each node.  WalkAction is invoked with three
 **		arguments as follows:
 **			WalkAction( NodeData, Order, Level )
 **		Data is the data contents of the node being visited,
 **		Order is either preorder,
 **		postorder, endorder, or leaf depending on whether this is
 **		the 1st, 2nd, or 3rd time a node has been visited, or
 **		whether the node is a leaf.  Level is the level of the node in
 **		the tree with the root being level 0.
 **	Return: none
 **	Exceptions: none
 **	History:
 **		3/13/89, DSJ, Created.
 **		7/13/89, DSJ, Pass node contents, not node, to WalkAction().
 */
  if ((SubTree->Left == NULL) && (SubTree->Right == NULL))
    (*WalkAction) (SubTree->Data, leaf, Level);
  else {
    (*WalkAction) (SubTree->Data, preorder, Level);
    if (SubTree->Left != NULL)
      Walk (SubTree->Left, NextLevel(Level));
    (*WalkAction) (SubTree->Data, postorder, Level);
    if (SubTree->Right != NULL)
      Walk (SubTree->Right, NextLevel(Level));
    (*WalkAction) (SubTree->Data, endorder, Level);
  }
}                                /* Walk */


/*---------------------------------------------------------------------------*/
void FreeSubTree(KDNODE *SubTree) {
/*
 **	Parameters:
 **		SubTree		ptr to root node of sub-tree to be freed
 **	Globals: none
 **	Operation:
 **		This routine recursively frees the memory allocated to
 **		to the specified subtree.
 **	Return: none
 **	Exceptions: none
 **	History: 7/13/89, DSJ, Created.
 */
  if (SubTree != NULL) {
    FreeSubTree (SubTree->Left);
    FreeSubTree (SubTree->Right);
    memfree(SubTree);
  }
}                                /* FreeSubTree */
