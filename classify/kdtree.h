/******************************************************************************
 **	Filename:	kdtree.h
 **	Purpose:	Definition of K-D tree access routines.
 **	Author:		Dan Johnson
 **	History:	3/11/89, DSJ, Created.
 **			5/23/89, DSJ, Added circular feature capability.
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
#ifndef   KDTREE_H
#define   KDTREE_H

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "host.h"
#include "cutil.h"
#include "ocrfeatures.h"

/**
NOTE:  All circular parameters of all keys must be in the range

Min <= Param < Max

where Min and Max are specified in the KeyDesc parameter passed to
MakeKDTree.  All KD routines assume that this is true and will not operate
correctly if circular parameters outside the specified range are used.
*/

struct KDNODE {
  FLOAT32 *Key;                  /**< search key */
  void *Data;                    /**< data that corresponds to key */
  FLOAT32 BranchPoint;           /**< needed to make deletes work efficiently */
  FLOAT32 LeftBranch;            /**< used to optimize search pruning */
  FLOAT32 RightBranch;           /**< used to optimize search pruning */
  struct KDNODE *Left;           /**< ptrs for KD tree structure */
  struct KDNODE *Right;
};

struct KDTREE {
  inT16 KeySize;                 /* number of dimensions in the tree */
  KDNODE Root;                   /* Root.Left points to actual root node */
  PARAM_DESC KeyDesc[1];         /* description of each dimension */
};

/*----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
#define RootOf(T)   ((T)->Root.Left->Data)

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
KDTREE *MakeKDTree(inT16 KeySize, const PARAM_DESC KeyDesc[]);

void KDStore(KDTREE *Tree, FLOAT32 *Key, void *Data);

void KDDelete(KDTREE * Tree, FLOAT32 Key[], void *Data);

void KDNearestNeighborSearch(
    KDTREE *Tree, FLOAT32 Query[], int QuerySize, FLOAT32 MaxDistance,
    int *NumberOfResults, void **NBuffer, FLOAT32 DBuffer[]);

void KDWalk(KDTREE *Tree, void_proc Action, void *context);

void FreeKDTree(KDTREE *Tree);

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
KDNODE *MakeKDNode(KDTREE *tree, FLOAT32 Key[], void *Data, int Index);

void FreeKDNode(KDNODE *Node);

FLOAT32 DistanceSquared(int k, PARAM_DESC *dim, FLOAT32 p1[], FLOAT32 p2[]);

FLOAT32 TESS_API ComputeDistance(int k, PARAM_DESC *dim, FLOAT32 p1[], FLOAT32 p2[]);

int QueryInSearch(KDTREE *tree);

void Walk(KDTREE *tree, void_proc action, void *context,
          KDNODE *SubTree, inT32 Level);

void InsertNodes(KDTREE *tree, KDNODE *nodes);

void FreeSubTree(KDNODE *SubTree);
#endif
