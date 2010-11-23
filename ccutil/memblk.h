/**********************************************************************
 * File:        memblk.h  (Formerly memblock.h)
 * Description: Enhanced instrumented memory allocator implemented as a class.
 * Author:					Ray Smith
 * Created:					Tue Jan 21 17:13:39 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef           MEMBLK_H
#define           MEMBLK_H

#include          "params.h"

#define MAXBLOCKS     16         /*max allowed to grab */
#define MAX_STRUCTS     20       //no of units maintained
#define MAX_CLASSES     24       //max classes of each size
#define MAX_FREE_S_BLOCKS 10     //max free list before all freed
#define STRUCT_BLOCK_SIZE 2521
#define MAX_CHUNK     262144     //max single chunk
#define FIRSTSIZE     16384      //size of first block
#define LASTSIZE      262144     //biggest size to use
#define BIGSIZE       2100000    //size of big blocks
#define MAX_BIGCHUNK    20000000 //max chunk of big mem

//#define TESTING_BIGSTUFF                                                                                      //define for big tests
//#define COUNTING_CLASS_STRUCTURES

class MEMUNION
{
  public:
    union
    {
      MEMUNION *ptr;             //next chunk
      inT32 size;                //chunk size
    };
    uinT16 owner;                //owner of chunk
    uinT16 age;                  //age of chunk
};

class MEMBLOCK
{
  public:
    MEMUNION * blockstart;       /*start of block */
    MEMUNION *blockend;          /*end of block */
    MEMUNION *freechunk;         /*next free chunk */
    MEMUNION *topchunk;          /*top free chunk */
    MEMBLOCK *next;              /*next block in chain */
    inT32 upperspace;            /*space above freechunk */
    inT32 lowerspace;            /*space below freechunk */

    MEMUNION *find_chunk(               //find free chunk
                         inT32 count);  //size required
};

class FREE_CALL
{
  public:
    void *freeer;                //return addr
    inT32 count;                 //no of frees
    FREE_CALL() {  //constructor
      freeer = NULL;
      count = 0;
    }
};
class MALLOC_CALL
{
  public:
    void *caller;                //return addr
    FREE_CALL *free_list;        //freeer counts
    inT32 *counts;               //no of blocks
    inT32 free_bits;             //bits in free table

    MALLOC_CALL() {  //constructor
      caller = NULL;
      free_list = NULL;
      counts = NULL;
      free_bits = 0;
    }
    void count_freeer(              //check a structure
                      void *addr);  //return address

    void init_freeers();  //check a structure
};

class MEM_ALLOCATOR
{
  public:
    inT16 blockcount;            //blocks in use
    uinT16 malloc_serial;        //serial allocation
    MEMBLOCK *topblock;          //block for permanents
    MEMBLOCK *currblock;         //current block
    MALLOC_CALL *callers;        //hash table of callers
    void *(*malloc) (inT32);     //external allocator
    void (*free) (void *);       //external free
    inT32 maxsize;               //biggest block
    inT32 biggestblock;          //biggest chunk
    inT32 totalmem;              //total free memory
    inT32 memsize;               //current block size
    uinT32 malloc_div_ratio;     //scaling of malloc_serial
    uinT32 malloc_minor_serial;  //scaling counter
    uinT32 malloc_auto_count;    //counts auto checks
    inT32 call_bits;             //size of table
    inT32 entries;               //size of table
                                 //all memory blocks
    MEMBLOCK memblocks[MAXBLOCKS];

    void init (                  //initialize
      void *(*ext_malloc) (inT32),//external source
      void (*ext_free) (void *), //external free
      inT32 firstsize,           //size of first block
      inT32 lastsize,            //size of last block
      inT32 maxchunk);           //biggest request

    void *alloc(                //allocator
                inT32 size,     //size of chunk
                void *caller);  //ptr to caller
    void *alloc_p(                //allocator
                  inT32 size,     //size of chunk
                  void *caller);  //ptr to caller
    void dealloc(                //deallocator
                 void *ptr,      //mem to free
                 void *caller);  //ptr to caller
    void check(                     //check chunks
               const char *string,  //message
               inT8 level);         //amount of checks

    void reduce_counts();  //divide by 2
    void display_counts();  //count up
    MEMBLOCK *new_block(                 //get new big block
                        inT32 minsize);  //minimum size
    uinT16 hash_caller(              //check a structure
                       void *addr);  //return address

  private:
    void init_callers();  //check a structure
    void set_owner(                       //set owner & date
                   MEMUNION *chunkstart,  //chunk to set
                   void *caller);         //ptr to caller
};
extern MEM_ALLOCATOR big_mem;
extern MEM_ALLOCATOR main_mem;
                                 //heads of freelists
extern MEMUNION *free_structs[MAX_STRUCTS];
                                 //number issued
extern inT32 structs_in_use[MAX_STRUCTS];
                                 //number issued
extern inT32 blocks_in_use[MAX_STRUCTS];
                                 //head of block lists
extern MEMUNION *struct_blocks[MAX_STRUCTS];
extern inT32 owner_counts[MAX_STRUCTS][MAX_CLASSES];

extern INT_VAR_H (mem_mallocdepth, 0, "Malloc stack depth to trace");
extern INT_VAR_H (mem_mallocbits, 8, "Log 2 of hash table size");
extern INT_VAR_H (mem_freedepth, 0, "Free stack dpeth to trace");
extern INT_VAR_H (mem_freebits, 8, "Log 2 of hash table size");
extern INT_VAR_H (mem_countbuckets, 16, "No of buckets for histogram");
extern INT_VAR_H (mem_checkfreq, 0,
"Calls to alloc_mem between owner counts");

void *trace_caller(             //trace stack
                   inT32 depth  //depth to trace
                  );
inT32 identify_struct_owner(                     //get table index
                            inT32 struct_count,  //cell size
                            const char *name     //name of type
                           );
void check_struct(             //check a structure
                  inT8 level,  //print control
                  inT32 count  //no of bytes
                 );
void check_structs(            //count in use on structs
                   inT8 level  //print control
                  );
void *new_struct_block();  //allocate memory
void old_struct_block(                     //free a structure block
                      MEMUNION *deadblock  //block to free
                     );
#endif
