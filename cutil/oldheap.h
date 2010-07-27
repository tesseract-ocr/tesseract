/******************************************************************************
 **	Filename:	heap.h
 **	Purpose:	Definition of heap access routines.
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
#ifndef   HEAP_H
#define   HEAP_H

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "general.h"
#include "cutil.h"

#define HEAPFULL      3000

#define OK          0
#define EMPTY -1

typedef struct
{
  FLOAT32 Key;
  void *Data;
}


HEAPENTRY;

typedef struct
{
  inT32 Size;
  inT32 FirstFree;
  HEAPENTRY Entry[1];
}


HEAP;

/*-----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
#define           FreeHeap(H) memfree(H)
#define           MaxSizeOfHeap(H)  (H->Size)
#define           SizeOfHeap(H)   (H->FirstFree - 1)
#define           InitHeap(H)   (H->FirstFree = 1)
#define           HeapFull(H)   ((H)->FirstFree > (H)->Size)
#define           HeapEmpty(H)    ((H)->FirstFree <= 1)

/* macros for accessing elements in heap by index.  The indicies vary from
  0 to SizeOfHeap-1.  No bounds checking is done.  Elements accessed in
  this manner are in random order relative to the Key values.  These
  macros should never be used as the LHS of an assignment statement as this
  will corrupt the heap.*/
#define           HeapKeyFor(H,E)   ((H)->Entry[(E)+1].Key)
#define           HeapDataFor(H,E)  ((H)->Entry[(E)+1].Data)

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
HEAP *MakeHeap(int Size);

int HeapPop(HEAP *Heap, FLOAT32 *Key, void *out_ptr);

int HeapPopWorst(HEAP *Heap, FLOAT32 *Key, void *out_ptr);

void HeapPush(HEAP *Heap, FLOAT32 Key, void *Data);

void HeapStore(HEAP *Heap, HEAPENTRY *Entry);

int GetTopOfHeap(HEAP *Heap, HEAPENTRY *Entry);

void FreeHeapData(HEAP *Heap, void_dest destructor);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define        _ARGS(s) s
#else
# define        _ARGS(s) ()
#endif*/

/* heap.c
HEAP *MakeHeap
    _ARGS((int Size));

int HeapPop
    _ARGS((HEAP *Heap,
  FLOAT32 *Key,
  char **Data));

int HeapPopWorst
    _ARGS((HEAP *Heap,
  FLOAT32 *Key,
  char **Data));

void HeapPush
    _ARGS((HEAP *Heap,
  FLOAT32 Key,
  char *Data));

void HeapStore
    _ARGS((HEAP *Heap,
  HEAPENTRY *Entry));

int GetTopOfHeap
    _ARGS((HEAP *Heap,
  HEAPENTRY *Entry));

void FreeHeapData
    _ARGS((HEAP *Heap,
  void (*Deallocator )()));

#undef _ARGS
*/
#endif
