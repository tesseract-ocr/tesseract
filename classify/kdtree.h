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
#include "general.h"
#include "cutil.h"
#include "ocrfeatures.h"

/**
NOTE:  All circular parameters of all keys must be in the range

Min <= Param < Max

where Min and Max are specified in the KeyDesc parameter passed to
MakeKDTree.  All KD routines assume that this is true and will not operate
correctly if circular parameters outside the specified range are used.
*/

typedef struct kdnode
{
  FLOAT32 *Key;                  /**< search key */
  char *Data;                    /**< data that corresponds to key */
  FLOAT32 BranchPoint;           /**< needed to make deletes work efficiently */
  FLOAT32 LeftBranch;            /**< used to optimize search pruning */
  FLOAT32 RightBranch;           /**< used to optimize search pruning */
  struct kdnode *Left;           /**< ptr for KD tree structure */
  struct kdnode *Right;          /**< ptr for KD tree structure */
}


KDNODE;

typedef struct
{
  inT16 KeySize;                 /* number of dimensions in the tree */
  KDNODE Root;                   /* Root.Left points to actual root node */
  PARAM_DESC KeyDesc[1];         /* description of each dimension */
}


KDTREE;

/** used for walking thru KD trees */
typedef enum {                   
  preorder, postorder, endorder, leaf
}


VISIT;

/*----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
#define RootOf(T)   ((T)->Root.Left->Data)

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
KDTREE *MakeKDTree (inT16 KeySize, PARAM_DESC KeyDesc[]);

void KDStore(KDTREE *Tree, FLOAT32 *Key, void *Data);

void KDDelete (KDTREE * Tree, FLOAT32 Key[], void *Data);

int KDNearestNeighborSearch (KDTREE * Tree,
FLOAT32 Query[],
int QuerySize,
FLOAT32 MaxDistance,
void *NBuffer, FLOAT32 DBuffer[]);

void KDWalk(KDTREE *Tree, void_proc Action);

void FreeKDTree(KDTREE *Tree);

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
int Equal (FLOAT32 Key1[], FLOAT32 Key2[]);

KDNODE *MakeKDNode (FLOAT32 Key[], char *Data, int Index);

void FreeKDNode(KDNODE *Node);

void Search(int Level, KDNODE *SubTree);

FLOAT32 ComputeDistance (register int N,
register PARAM_DESC Dim[],
register FLOAT32 p1[], register FLOAT32 p2[]);

void FindMaxDistance();

int QueryIntersectsSearch();

int QueryInSearch();

void Walk(KDNODE *SubTree, inT32 Level);

void FreeSubTree(KDNODE *SubTree);
#endif
