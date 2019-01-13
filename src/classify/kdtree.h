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
#ifndef   KDTREE_H
#define   KDTREE_H

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "cutil.h"      // for void_proc
#include "host.h"
#include "ocrfeatures.h"

/**
NOTE:  All circular parameters of all keys must be in the range

Min <= Param < Max

where Min and Max are specified in the KeyDesc parameter passed to
MakeKDTree.  All KD routines assume that this is true and will not operate
correctly if circular parameters outside the specified range are used.
*/

struct KDNODE {
  float *Key;                  /**< search key */
  void *Data;                  /**< data that corresponds to key */
  float BranchPoint;           /**< needed to make deletes work efficiently */
  float LeftBranch;            /**< used to optimize search pruning */
  float RightBranch;           /**< used to optimize search pruning */
  struct KDNODE *Left;         /**< ptrs for KD tree structure */
  struct KDNODE *Right;
};

struct KDTREE {
  int16_t KeySize;               /* number of dimensions in the tree */
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
KDTREE *MakeKDTree(int16_t KeySize, const PARAM_DESC KeyDesc[]);

void KDStore(KDTREE *Tree, float *Key, void *Data);

void KDDelete(KDTREE * Tree, float Key[], void *Data);

void KDNearestNeighborSearch(
    KDTREE *Tree, float Query[], int QuerySize, float MaxDistance,
    int *NumberOfResults, void **NBuffer, float DBuffer[]);

void KDWalk(KDTREE *Tree, void_proc Action, void *context);

void FreeKDTree(KDTREE *Tree);

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
KDNODE *MakeKDNode(KDTREE *tree, float Key[], void *Data, int Index);

void FreeKDNode(KDNODE *Node);

float DistanceSquared(int k, PARAM_DESC *dim, float p1[], float p2[]);

float ComputeDistance(int k, PARAM_DESC *dim, float p1[], float p2[]);

int QueryInSearch(KDTREE *tree);

void Walk(KDTREE *tree, void_proc action, void *context,
          KDNODE *SubTree, int32_t Level);

void InsertNodes(KDTREE *tree, KDNODE *nodes);

void FreeSubTree(KDNODE *SubTree);
#endif
