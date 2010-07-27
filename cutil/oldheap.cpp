/******************************************************************************
 **	Filename:	heap.c
 **	Purpose:	Routines for managing heaps (smallest at root)
 **	Author:		Dan Johnson
 **	History:	3/13/89, DSJ, Created.
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
#include "oldheap.h"
#include "freelist.h"
#include "danerror.h"
#include "emalloc.h"
#include <stdio.h>

#define FATHER(N) ((N)>>1)
#define LEFTSON(N)  ((N)<<1)
#define RIGHTSON(N) ((N)<<1 + 1)

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine creates and initializes a new heap data
 * structure containing Size elements.  In actuality, Size + 1
 * elements are allocated.  The first element, element 0, is
 * unused, this makes the index arithmetic easier.
 *
 * Globals:
 * - None
 *
 * @param Size maximum number of entries in the heap
 * @return Pointer to the new heap.
 * @note Exceptions: None
 * @note History: 3/13/89, DSJ, Created.
 */
HEAP *MakeHeap(int Size) {
  HEAP *NewHeap;

  NewHeap = (HEAP *) Emalloc (sizeof (HEAP) + Size * sizeof (HEAPENTRY));

  NewHeap->Size = Size;
  NewHeap->FirstFree = 1;
  return (NewHeap);
}                                /* MakeHeap */


/*---------------------------------------------------------------------------*/
/**
 * This routine removes the top item on the heap and places
 * its contents into Key and Data.
 *
 * Globals:
 * - None
 *
 * @param Heap ptr to heap whose top is to be removed and returned
 * @param Key place to put key of top heap item
 * @param Data place to put data of top heap item
 *
 * @return OK if top entry returned, EMPTY if heap is empty
 * @note Exceptions: None
 * @note History: 5/10/91, DSJ, Created (Modified from GetTopOfHeap).
 */
int HeapPop(HEAP *Heap, FLOAT32 *Key, void *out_ptr) {
  inT32 Hole;
  FLOAT32 HoleKey;
  inT32 Son;
  void **Data = (void **) out_ptr;

  if (Heap->FirstFree <= 1)
    return (EMPTY);

  *Key = Heap->Entry[1].Key;
  *Data = Heap->Entry[1].Data;

  Heap->FirstFree--;

  /* imagine the hole at the root is filled with the last entry in the heap */
  HoleKey = Heap->Entry[Heap->FirstFree].Key;
  Hole = 1;

                                 /* while hole has 2 sons */
  while ((Son = LEFTSON (Hole)) < Heap->FirstFree) {
    /* find the son with the smallest key */
    if (Heap->Entry[Son].Key > Heap->Entry[Son + 1].Key)
      Son++;

    /* if key for hole is greater than key for son, sift hole down */
    if (HoleKey > Heap->Entry[Son].Key) {
      Heap->Entry[Hole].Key = Heap->Entry[Son].Key;
      Heap->Entry[Hole].Data = Heap->Entry[Son].Data;
      Hole = Son;
    }
    else
      break;
  }
  Heap->Entry[Hole].Key = HoleKey;
  Heap->Entry[Hole].Data = Heap->Entry[Heap->FirstFree].Data;
  return (OK);
}                                /* HeapPop */


/**
 * HeapPopWorst
 *
 * Remove the largest item from the heap.
 *
 * @param Heap ptr to heap whose top is to be removed and returned
 * @param Key place to put key of top heap item
 * @param Data place to put data of top heap item
 */
int HeapPopWorst(HEAP *Heap, FLOAT32 *Key, void *out_ptr) {
  inT32 Index;                   /*current index */
  inT32 Hole;
  FLOAT32 HoleKey;
  inT32 Father;
  void *HoleData;
  void **Data = (void **) out_ptr;

  if (Heap->FirstFree <= 1)
    return (EMPTY);

  HoleKey = Heap->Entry[1].Key;
  Hole = 1;
  Heap->FirstFree--;
  for (Index = Heap->FirstFree, Father = FATHER (Index); Index > Father;
    Index--)
  if (Heap->Entry[Index].Key > HoleKey) {
                                 /*find biggest */
    HoleKey = Heap->Entry[Index].Key;
    Hole = Index;
  }
  *Key = HoleKey;
  *Data = Heap->Entry[Hole].Data;

  HoleKey = Heap->Entry[Heap->FirstFree].Key;
  Heap->Entry[Hole].Key = HoleKey;
  HoleData = Heap->Entry[Heap->FirstFree].Data;
  Heap->Entry[Hole].Data = HoleData;

  /* now sift last entry to its rightful place */
  Father = FATHER (Hole);        /*father of hole */
  while (Hole > 1 && Heap->Entry[Father].Key > HoleKey) {
                                 /*swap entries */
    Heap->Entry[Hole].Key = Heap->Entry[Father].Key;
    Heap->Entry[Hole].Data = Heap->Entry[Father].Data;
    Heap->Entry[Father].Data = HoleData;
    Heap->Entry[Father].Key = HoleKey;
    Hole = Father;
    Father = FATHER (Hole);
  }
  return (OK);
}                                /* HeapPop */


/*---------------------------------------------------------------------------*/
/** 
 * This routine stores Data into Heap and associates it
 * with Key.  The heap is
 * maintained in such a way that the item with the lowest key
 * is always at the top of the heap.
 *
 * Globals:
 * - None
 *
 * @param Heap ptr to heap to store new item in
 * @param Key numeric key associated with new item
 * @param Data ptr to data contents of new item
 *
 * @note Exceptions:
 * - HEAPFULL error if heap size is exceeded
 *
 * @note History: 5/10/91, DSJ, Created (Modified version of HeapStore).
 */
void HeapPush(HEAP *Heap, FLOAT32 Key, void *Data) {
  inT32 Item;
  inT32 Father;

  if (Heap->FirstFree > Heap->Size)
    DoError (HEAPFULL, "Heap size exceeded");

  Item = Heap->FirstFree;
  Heap->FirstFree++;
  while (Item != 1) {
    Father = FATHER (Item);
    if (Heap->Entry[Father].Key > Key) {
      Heap->Entry[Item].Key = Heap->Entry[Father].Key;
      Heap->Entry[Item].Data = Heap->Entry[Father].Data;
      Item = Father;
    }
    else
      break;
  }
  Heap->Entry[Item].Key = Key;
  Heap->Entry[Item].Data = Data;
}                                /* HeapPush */


/*---------------------------------------------------------------------------*/
/**
 * This routine stores Entry into Heap.  The heap is
 * maintained in such a way that the item with the lowest key
 * is always at the top of the heap.
 *
 * Globals:
 * - None
 *
 * @param Heap ptr to heap to store new item in
 * @param Entry ptr to item to be stored in Heap
 * @note Exceptions:
 * - HEAPFULL error if heap size is exceeded
 * @note History: 3/13/89, DSJ, Created.
 */
void HeapStore(HEAP *Heap, HEAPENTRY *Entry) {
  inT32 Item;
  inT32 Father;

  if (Heap->FirstFree > Heap->Size)
    DoError (HEAPFULL, "Heap size exceeded");

  Item = Heap->FirstFree;
  Heap->FirstFree++;
  while (Item != 1) {
    Father = FATHER (Item);
    if (Heap->Entry[Father].Key > Entry->Key) {
      Heap->Entry[Item].Key = Heap->Entry[Father].Key;
      Heap->Entry[Item].Data = Heap->Entry[Father].Data;
      Item = Father;
    }
    else
      break;
  }
  Heap->Entry[Item].Key = Entry->Key;
  Heap->Entry[Item].Data = Entry->Data;
}                                /* HeapStore */


/*---------------------------------------------------------------------------*/
/**
 * This routine removes the top item on the heap and copies its
 * contents into Entry.
 *
 * @param Heap ptr to heap whose top is to be removed and returned
 * @param Entry ptr to heap entry to be filled with top entry on Heap
 *
 * Globals:
 * - None
 *
 * @return OK if top entry returned, EMPTY if heap is empty
 * @note Exceptions: None
 * @note History: 3/13/89, DSJ, Created.
 */
int GetTopOfHeap(HEAP *Heap, HEAPENTRY *Entry) {
  inT32 Hole;
  FLOAT32 HoleKey;
  inT32 Son;

  if (Heap->FirstFree <= 1)
    return (EMPTY);

  Entry->Key = Heap->Entry[1].Key;
  Entry->Data = Heap->Entry[1].Data;

  Heap->FirstFree--;

  /* imagine the hole at the root is filled with the last entry in the heap */
  HoleKey = Heap->Entry[Heap->FirstFree].Key;
  Hole = 1;

                                 /* while hole has 2 sons */
  while ((Son = LEFTSON (Hole)) < Heap->FirstFree) {
    /* find the son with the smallest key */
    if (Heap->Entry[Son].Key > Heap->Entry[Son + 1].Key)
      Son++;

    /* if key for hole is greater than key for son, sift hole down */
    if (HoleKey > Heap->Entry[Son].Key) {
      Heap->Entry[Hole].Key = Heap->Entry[Son].Key;
      Heap->Entry[Hole].Data = Heap->Entry[Son].Data;
      Hole = Son;
    }
    else
      break;
  }
  Heap->Entry[Hole].Key = HoleKey;
  Heap->Entry[Hole].Data = Heap->Entry[Heap->FirstFree].Data;
  return (OK);
}                                /* GetTopOfHeap */


/*---------------------------------------------------------------------------*/
/**
 * This routine is similar to FreeHeap in that it
 * deallocates the memory consumed by the heap.  However, it
 * also calls Deallocator for each item in the heap so that
 * this data is also deallocated.
 *
 * @param Heap heap whose data is to be freed
 * @param Deallocator function to be used to deallocate data
 *
 * Globals: 
 * - None
 *
 * @note Exceptions: none
 * @note History: Tue May 15 08:52:04 1990, DSJ, Created.
 */
void FreeHeapData(HEAP *Heap, void_dest destructor) {
  HEAPENTRY Entry;

  while (GetTopOfHeap (Heap, &Entry) != EMPTY)
    destructor (Entry.Data);

  FreeHeap(Heap);
}                                /* FreeHeapData */
