/**********************************************************************
 * File:        memblk.c  (Formerly memblock.c)
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

#include          "mfcpch.h"     //precompiled headers
#include          <stdlib.h>
#include          <string.h>
#include          "stderr.h"
#include          "memryerr.h"
#include          "hashfn.h"
#include          "tprintf.h"
#include          "memry.h"
#include          "memblk.h"
#ifdef __UNIX__
#include          <signal.h>
#endif

class UWREC
{
  public:
    unsigned cur_frsize;         //frame size
    unsigned cursp;              //stack
    unsigned currls;             //pc space
    unsigned currlo;             //pc offset
    unsigned curdp;              //data pointer
    unsigned toprp;              //rp
    unsigned topmrp;             //mrp
    unsigned topsr0;             //sr0
    unsigned topsr4;             //sr4
    unsigned r3;                 //gr3
    unsigned cur_r19;            //gr19
};

MEMUNION *free_block = NULL;     //head of freelist

#define EXTERN

EXTERN MEM_ALLOCATOR big_mem;
EXTERN MEM_ALLOCATOR main_mem;
                                 //heads of freelists
EXTERN MEMUNION *free_structs[MAX_STRUCTS];
                                 //number issued
EXTERN inT32 structs_in_use[MAX_STRUCTS];
                                 //number issued
EXTERN inT32 blocks_in_use[MAX_STRUCTS];
                                 //head of block lists
EXTERN MEMUNION *struct_blocks[MAX_STRUCTS];
EXTERN const char *owner_names[MAX_STRUCTS][MAX_CLASSES];
EXTERN inT32 owner_counts[MAX_STRUCTS][MAX_CLASSES];
                                 //no of names
EXTERN inT16 name_counts[MAX_STRUCTS];
EXTERN inT32 free_struct_blocks; //no of free blocks

EXTERN INT_VAR (mem_mallocdepth, 0, "Malloc stack depth to trace");
EXTERN INT_VAR (mem_mallocbits, 8, "Log 2 of hash table size");
EXTERN INT_VAR (mem_freedepth, 0, "Free stack dpeth to trace");
EXTERN INT_VAR (mem_freebits, 8, "Log 2 of hash table size");
EXTERN INT_VAR (mem_countbuckets, 16, "No of buckets for histogram");
EXTERN INT_VAR (mem_checkfreq, 0, "Calls to alloc_mem between owner counts");

/**********************************************************************
 * MEM_ALLOCATOR::MEM_ALLOCATOR
 *
 * Constructor for a memory allocator.
 **********************************************************************/

void
MEM_ALLOCATOR::init (            //initialize
void *(*ext_malloc) (inT32),     //external source
void (*ext_free) (void *),       //external free
inT32 firstsize,                 //size of first block
inT32 lastsize,                  //size of last block
inT32 maxchunk                   //biggest request
) {
  blockcount = 0;
  malloc_serial = 0;
  topblock = NULL;
  currblock = NULL;
  callers = NULL;
  malloc = ext_malloc;
  free = ext_free;
  maxsize = lastsize;
  biggestblock = maxchunk;
  totalmem = 0;
  memsize = firstsize;
  malloc_div_ratio = 1;
  malloc_minor_serial = 0;
  malloc_auto_count = 0;
  call_bits = 0;
  entries = 0;
}


/**********************************************************************
 * MEM_ALLOCATOR::hash_caller
 *
 * Generate a hash code for a caller, setup the tables if necessary.
 **********************************************************************/

uinT16 MEM_ALLOCATOR::hash_caller(            //get hash code
                                  void *addr  //return address
                                 ) {
  inT32 index;                   //index to table
  inT32 initial_hash;            //initial index

  if (callers == NULL)
    init_callers();  //setup table
                                 //get hash code
  initial_hash = hash (call_bits, &addr, sizeof (addr));
  if (initial_hash == 0)
    initial_hash = 1;
  index = initial_hash;
  if (callers[index].caller != NULL && callers[index].caller != addr) {
    do {
      index++;
      if (index >= entries)
        index = 1;
    }
    while (callers[index].caller != NULL
      && callers[index].caller != addr && index != initial_hash);
    if (index == initial_hash)
      index = 0;
  }
  if (callers[index].caller == NULL) {
    if (index != 0)
      callers[index].caller = addr;
    if (callers[index].free_list == NULL)
                                 //setup free table
      callers[index].init_freeers ();
  }
  return (uinT16) index;
}


/**********************************************************************
 * MALLOC_CALL::count_freeer
 *
 * Generate a hash code for a freeer, setup the tables if necessary.
 * Then count the call.
 **********************************************************************/

void MALLOC_CALL::count_freeer(            //count calls to free
                               void *addr  //return address
                              ) {
  inT32 entries;                 //entries in table
  inT32 index;                   //index to table
  inT32 initial_hash;            //initial index

  if (free_list == NULL)
    init_freeers();  //setup table
  entries = 1 << free_bits;
                                 //get hash code
  initial_hash = hash (free_bits, &addr, sizeof (addr));
  if (initial_hash == 0)
    initial_hash = 1;
  index = initial_hash;
  if (free_list[index].freeer != NULL && free_list[index].freeer != addr) {
    do {
      index++;
      if (index >= entries)
        index = 1;
    }
    while (free_list[index].freeer != NULL
      && free_list[index].freeer != addr && index != initial_hash);
    if (index == initial_hash)
      index = 0;
  }
  if (free_list[index].freeer == NULL && index != 0) {
    free_list[index].freeer = addr;
  }
  free_list[index].count++;      //count them
}


/**********************************************************************
 * MEM_ALLOCATOR::init_callers
 *
 * Initialize the callers hash table.
 **********************************************************************/

void MEM_ALLOCATOR::init_callers() {  //setup hash table
  inT32 depth = mem_mallocdepth;

  mem_mallocdepth.set_value (0); //can't register it
  call_bits = mem_mallocbits;
  entries = 1 << call_bits;
                                 //make an array
  callers = new MALLOC_CALL[entries];
  mem_mallocdepth.set_value (depth);
}


/**********************************************************************
 * MALLOC_CALL::init_freeers
 *
 * Initialize the freeers hash table.
 **********************************************************************/

void MALLOC_CALL::init_freeers() {  //setup hash table
  inT32 entries;                 //entries in table
  inT32 depth = mem_mallocdepth;

  mem_mallocdepth.set_value (0); //can't register it
  free_bits = mem_freebits;
  entries = 1 << free_bits;
                                 //make an array
  free_list = new FREE_CALL[entries];
  mem_mallocdepth.set_value (depth);
}


/**********************************************************************
 * MEM_ALLOCATOR::reduce_counts
 *
 * Divide all ages by 2 to get a log use of counts.
 **********************************************************************/

void MEM_ALLOCATOR::reduce_counts() {  //divide by 2
  MEMBLOCK *block;               //current block
  MEMUNION *chunk;               //current chunk
  inT32 chunksize;               //size of chunk
  inT32 blockindex;              //index of block

  check_mem ("Reducing counts", JUSTCHECKS);
  for (blockindex = 0; blockindex < blockcount; blockindex++) {
                                 //current block
    block = &memblocks[blockindex];
                                 //scan all chunks
    for (chunk = block->blockstart; chunk != block->blockend; chunk += chunksize) {
      chunksize = chunk->size;   //size of chunk
      if (chunksize < 0)
        chunksize = -chunksize;  //absolute size
      chunk->age /= 2;           //divide ages
    }
  }
}


/**********************************************************************
 * MEM_ALLOCATOR::display_counts
 *
 * Send counts of outstanding blocks to stderr.
 **********************************************************************/

void MEM_ALLOCATOR::display_counts() {  //count up
  MEMBLOCK *block;               //current block
  MEMUNION *chunk;               //current chunk
  inT32 chunksize;               //size of chunk
  inT32 blockindex;              //index of block
  inT32 buckets;                 //required buckets
  inT32 bucketsize;              //no in each bucket
  inT32 callindex;               //index to callers
  inT32 freeindex;               //index to freeers
  inT32 freeentries;             //table size
  inT32 totalchunks;             //total chunk counts
  inT32 totalspace;              //total mem space
  inT32 totalpchunks;            //permanent chunks
  inT32 totalpspace;             //permanent space
  inT32 totalfrees;              //total free calls

  if (callers == NULL)
    return;                      //can't do anything
  check_mem ("Displaying counts", JUSTCHECKS);
  buckets = mem_countbuckets;
  bucketsize = (malloc_serial - 1) / buckets + 1;
  tprintf ("\nEach bucket covers %g counts.\n",
    (double) bucketsize * malloc_div_ratio);
  for (callindex = 0; callindex < entries; callindex++) {
    if (callers[callindex].free_list != NULL) {
      callers[callindex].counts =
        (inT32 *) malloc (buckets * 4 * sizeof (inT32));
      memset (callers[callindex].counts, 0,
        (size_t) (buckets * 4 * sizeof (inT32)));
    }
  }
  for (blockindex = 0; blockindex < blockcount; blockindex++) {
                                 //current block
    block = &memblocks[blockindex];
                                 //scan all chunks
    for (chunk = block->blockstart; chunk != block->topchunk; chunk += chunksize) {
      chunksize = chunk->size;   //size of chunk
      if (chunksize < 0) {
        chunksize = -chunksize;  //absolute size
        callindex = chunk->owner;
        if (callers[callindex].counts != NULL) {
          callers[callindex].counts[chunk->age / bucketsize * 4]++;
          callers[callindex].counts[chunk->age / bucketsize * 4 +
            1] += chunksize;
        }
      }
    }
                                 //scan all chunks
    for (; chunk != block->blockend; chunk += chunksize) {
      chunksize = chunk->size;   //size of chunk
      if (chunksize < 0) {
        chunksize = -chunksize;  //absolute size
        callindex = chunk->owner;
        if (callers[callindex].counts != NULL) {
          callers[callindex].counts[chunk->age / bucketsize * 4 +
            2]++;
          callers[callindex].counts[chunk->age / bucketsize * 4 +
            3] += chunksize;
        }
      }
    }
  }
  for (callindex = 0; callindex < entries; callindex++) {
    if (callers[callindex].counts != NULL) {
      for (totalspace = 0, totalchunks = 0, totalpspace =
        0, totalpchunks = 0, freeindex = 0; freeindex < buckets;
      freeindex++) {
        totalchunks += callers[callindex].counts[freeindex * 4];
        totalspace += callers[callindex].counts[freeindex * 4 + 1];
        totalpchunks += callers[callindex].counts[freeindex * 4 + 2];
        totalpspace += callers[callindex].counts[freeindex * 4 + 3];
      }
      freeentries = 1 << callers[callindex].free_bits;
      for (totalfrees = 0, freeindex = 0; freeindex < freeentries;
        freeindex++)
      totalfrees += callers[callindex].free_list[freeindex].count;
      if (totalspace != 0 || totalfrees != 0) {
        tprintf ("alloc_mem at %d : total held=%d(%d), frees=%d.\n",
          callers[callindex].caller,
          totalchunks, totalspace * sizeof (MEMUNION),
          totalfrees);
      }
      if (totalspace > 0) {
        for (freeindex = 0; freeindex < buckets; freeindex++) {
          tprintf ("%d(%d) ",
            callers[callindex].counts[freeindex * 4],
            callers[callindex].counts[freeindex * 4 +
            1] * sizeof (MEMUNION));
        }
        tprintf ("\n");
      }
      if (totalfrees != 0) {
        tprintf ("Calls to free : ");
        for (freeindex = 0; freeindex < freeentries; freeindex++) {
          if (callers[callindex].free_list[freeindex].count != 0)
            tprintf ("%d : %d ",
              callers[callindex].free_list[freeindex].freeer,
              callers[callindex].free_list[freeindex].count);
        }
        tprintf ("\n");
      }
      if (totalpspace != 0) {
        tprintf ("alloc_mem_p at %d : total held=%d(%d).\n",
          callers[callindex].caller,
          totalpchunks, totalpspace * sizeof (MEMUNION));
        for (freeindex = 0; freeindex < buckets; freeindex++) {
          tprintf ("%d(%d) ",
            callers[callindex].counts[freeindex * 4 + 2],
            callers[callindex].counts[freeindex * 4 +
            3] * sizeof (MEMUNION));
        }
        tprintf ("\n");
      }
      free (callers[callindex].counts);
      callers[callindex].counts = NULL;
    }
  }
}


/**********************************************************************
 * MEM_ALLOCATOR::check
 *
 * Check consistency of all memory controlled by this allocator.
 **********************************************************************/

void MEM_ALLOCATOR::check(                     //check consistency
                          const char *string,  //context message
                          inT8 level           //level of check
                         ) {
  MEMBLOCK *block;               //current block
  MEMUNION *chunk;               //current chunk
  MEMUNION *prevchunk;           //previous chunk
  inT32 chunksize;               //size of chunk
  inT32 usedcount;               //no of used chunks
  inT32 usedsize;                //size of used chunks
  inT32 freecount;               //no of free chunks
  inT32 freesize;                //size of free chunks
  inT32 biggest;                 //biggest free chunk
  inT32 totusedcount;            //no of used chunks
  inT32 totusedsize;             //size of used chunks
  inT32 totfreecount;            //no of free chunks
  inT32 totfreesize;             //size of free chunks
  inT32 totbiggest;              //biggest free chunk
  inT32 totblocksize;            //total size of blocks
  inT32 chunkindex;              //index of chunk
  inT32 blockindex;              //index of block

  if (level >= MEMCHECKS)
    tprintf ("\nMEM_ALLOCATOR::check:at '%s'\n", string);
  totusedcount = 0;              //grand totals
  totusedsize = 0;
  totfreecount = 0;
  totfreesize = 0;
  totbiggest = 0;
  totblocksize = 0;
  for (blockindex = 0; blockindex < blockcount; blockindex++) {
                                 //current block
    block = &memblocks[blockindex];
    if (level >= MEMCHECKS)
      tprintf ("Block %d:0x%x-0x%x, size=%d, top=0x%x, l=%d, u=%d\n",
        blockindex, block->blockstart, block->blockend,
        (block->blockend - block->blockstart) * sizeof (MEMUNION),
        block->topchunk, block->lowerspace, block->upperspace);
    usedcount = usedsize = 0;    //zero counters
    freecount = freesize = 0;    //zero counters
    biggest = 0;
                                 //scan all chunks
    for (chunkindex = 0, prevchunk = NULL, chunk = block->blockstart; chunk != block->blockend; chunkindex++, chunk += chunksize) {
      chunksize = chunk->size;   //size of chunk
      if (chunksize < 0)
        chunksize = -chunksize;  //absolute size
      if (level >= FULLMEMCHECKS) {
        tprintf ("%5d=%8d%c caller=%d, age=%d ", (int) chunkindex,
          chunksize * sizeof (MEMUNION),
          chunk->size < 0 ? 'U' : 'F', chunk->owner, chunk->age);
        if (chunkindex % 5 == 4)
          tprintf ("\n");
      }
                                 //illegal sizes
      if (chunksize == 0 || chunk->size == -1
                                 //out of bounds
        || chunk + chunksize - block->blockstart <= 0 || block->blockend - (chunk + chunksize) < 0)
        BADMEMCHUNKS.error ("check_mem", ABORT,
          "Block=%p, Prev chunk=%p, Chunk=%p, Size=%x",
          block, prevchunk, chunk,
          (int) chunk->size);

      else if (chunk->size < 0) {
        usedcount++;             //used block
        usedsize += chunksize;
      }
      else {
        freecount++;             //free block
        freesize += chunksize;
        if (chunksize > biggest)
          biggest = chunksize;
      }
      prevchunk = chunk;
    }
    if (level >= MEMCHECKS) {
      if (level >= FULLMEMCHECKS)
        tprintf ("\n");
      tprintf ("%d chunks in use, total size=%d bytes\n",
        (int) usedcount, usedsize * sizeof (MEMUNION));
      tprintf ("%d chunks free, total size=%d bytes\n",
        (int) freecount, freesize * sizeof (MEMUNION));
      tprintf ("Largest free fragment=%d bytes\n",
        biggest * sizeof (MEMUNION));
    }
    totusedcount += usedcount;   //grand totals
    totusedsize += usedsize;
    totfreecount += freecount;
    totfreesize += freesize;
    if (biggest > totbiggest)
      totbiggest = biggest;
    totblocksize += block->blockend - block->blockstart;
  }
  if (level >= MEMCHECKS) {
    tprintf ("%d total blocks in use, total size=%d bytes\n",
      blockcount, totblocksize * sizeof (MEMUNION));
    tprintf ("%d total chunks in use, total size=%d bytes\n",
      (int) totusedcount, totusedsize * sizeof (MEMUNION));
    tprintf ("%d total chunks free, total size=%d bytes\n",
      (int) totfreecount, totfreesize * sizeof (MEMUNION));
    tprintf ("Largest free fragment=%d bytes\n",
      totbiggest * sizeof (MEMUNION));
  }
  if (level >= MEMCHECKS)
    display_counts();
}


/**********************************************************************
 * MEM_ALLOCATOR::alloc_p
 *
 * Allocate permanent space which will never be returned.
 * This space is allocated from the top end of a memory block to
 * avoid the fragmentation which would result from alternate use
 * of alloc_mem for permanent and temporary blocks.
 **********************************************************************/

void *MEM_ALLOCATOR::alloc_p(              //permanent space
                             inT32 count,  //block size to allocate
                             void *caller  //ptr to caller
                            ) {
  MEMBLOCK *block;               //current block
  MEMUNION *chunk;               //current chunk

  if (count < 1 || count > biggestblock)
                                 //request too big
    MEMTOOBIG.error ("alloc_mem_p", ABORT, "%d", (int) count);

  count += sizeof (MEMUNION) - 1;//round up to word
  count /= sizeof (MEMUNION);
  count++;                       //and add one
  if (topblock == NULL) {
    topblock = new_block (count);//get first block
    currblock = topblock;
    if (topblock == NULL) {
      check_mem ("alloc_mem_p returning NULL", MEMCHECKS);
      return NULL;
    }
  }
  block = topblock;              //current block
  do {
    chunk = block->topchunk;
    if (chunk->size < count)
      block = block->next;       //try next block
  }
                                 //until all tried
  while (chunk->size < count && block != topblock);
  if (chunk->size < count) {     //still no good
    chunk = (MEMUNION *) alloc ((count - 1) * sizeof (MEMUNION), caller);
    //try last resort
    if (chunk != NULL)
      return chunk;
    check_mem ("alloc_mem_p returning NULL", MEMCHECKS);
    return NULL;
  }
  block->upperspace -= count;    //less above freechunk
  if (chunk->size > count) {
    chunk->size -= count;
    chunk += chunk->size;
  }
  chunk->size = -count;          //mark as in use
  if (mem_mallocdepth > 0) {
    set_owner(chunk, caller);
  }
  else {
    chunk->owner = 0;
    chunk->age = 0;
  }
  return chunk + 1;              //created chunk
}


/**********************************************************************
 * MEM_ALLOCATOR::alloc
 *
 * Return a pointer to a buffer of count bytes aligned for any type.
 **********************************************************************/

void *MEM_ALLOCATOR::alloc(              //get memory
                           inT32 count,  //no of bytes to get
                           void *caller  //ptr to caller
                          ) {
  MEMBLOCK *block;               //current block
  MEMUNION *chunk;               //current chunk
  inT32 chunksize;               //size of free chunk
  MEMUNION *chunkstart;          //start of free chunk

  if (count < 1 || count > biggestblock)
    MEMTOOBIG.error ("alloc_mem", ABORT, "%d", (int) count);
  //request too big

  count += sizeof (MEMUNION) - 1;//round up to word
  count /= sizeof (MEMUNION);
  count++;                       //and add one
  if (currblock == NULL) {
                                 //get first block
    currblock = new_block (count);
    topblock = currblock;
    if (currblock == NULL) {
      check_mem ("alloc_mem returning NULL", MEMCHECKS);
      return NULL;
    }
  }
  block = currblock;             //current block
  if (block->upperspace <= block->lowerspace) {
                                 //restart chunklist
    block->freechunk = block->blockstart;
    block->upperspace += block->lowerspace;
    block->lowerspace = 0;       //correct space counts
  }
  chunk = block->freechunk;      //current free chunk
  if (chunk->size < count) {     //big enough?
    do {
                                 //search for free chunk
      chunk = block->find_chunk (count);
      if (chunk->size < count)
        block = block->next;     //try next block
    }
                                 //until all tried
    while (chunk->size < count && block != currblock);
    if (chunk->size < count) {   //still no good
                                 //get a new block
      currblock = new_block (count);
      topblock = currblock;      //set perms here too
      if (currblock == NULL) {
        check_mem ("alloc_mem returning NULL", MEMCHECKS);
        return NULL;
      }
      block = currblock;
      chunk = block->freechunk;  //bound to be big enough
    }
  }
  chunkstart = chunk;            //start of chunk
  if (chunk == block->topchunk && chunk + count != block->blockend)
    block->topchunk += count;    //top has moved
  block->upperspace -= count;    //upper part used
  chunksize = chunk->size;       //size of free chunk
  chunk->size = -count;          //mark as used
  chunk += count;                //next free space
  totalmem -= count;             //no of free elements
  if (chunksize > count)         //bigger than exact?
                                 //remaining space
    chunk->size = chunksize - count;
  else if (chunk == block->blockend) {
    chunk = block->blockstart;   //restart block
    block->upperspace = block->lowerspace;
    block->lowerspace = 0;       //fix space counts
  }
  block->freechunk = chunk;      //next free block
  if (mem_mallocdepth > 0) {
    set_owner(chunkstart, caller);
  }
  else {
    chunkstart->owner = 0;
    chunkstart->age = 0;
  }
  chunkstart++;                  //start of block
  return chunkstart;             //pointer to block
}


/**********************************************************************
 * MEM_ALLOCATOR::set_owner
 *
 * Set the owner and time stamp of the block and check if needed.
 **********************************************************************/

void MEM_ALLOCATOR::set_owner(                       //get memory
                              MEMUNION *chunkstart,  //chunk to set
                              void *caller           //ptr to caller
                             ) {
  uinT16 callindex;              //hash code

  callindex = hash_caller (caller);
  chunkstart->owner = callindex;
                                 //store evidence
  chunkstart->age = malloc_serial;
  malloc_minor_serial++;
  if (malloc_minor_serial >= malloc_div_ratio) {
    malloc_minor_serial = 0;
    malloc_serial++;             //count calls
    if (malloc_serial == 0) {
                                 //wrap around
      reduce_counts();  //fix serial numbers
      malloc_serial = MAX_INT16 + 1;
                                 //all worth double
      malloc_div_ratio += malloc_div_ratio;
    }
  }
  malloc_auto_count++;
  if (mem_checkfreq > 0 && malloc_auto_count >= (uinT32) mem_checkfreq) {
    malloc_auto_count = 0;
    check_mem ("Auto check", MEMCHECKS);
  }
}


/**********************************************************************
 * MEM_ALLOCATOR::dealloc
 *
 * Free a block allocated by alloc (or alloc_p).
 * It checks that the pointer is legal and maintains counts of the
 * amount of free memory above and below the current free pointer.
 **********************************************************************/

void MEM_ALLOCATOR::dealloc(                 //free memory
                            void *oldchunk,  //chunk to free
                            void *caller     //ptr to caller
                           ) {
  MEMUNION *chunk;               //current chunk
  MEMBLOCK *block;               //current block

  if (oldchunk == NULL)
    FREENULLPTR.error ("free_mem", ABORT, NULL);
  chunk = (MEMUNION *) oldchunk;
  block = currblock;             //current block
  if (block == NULL)
    NOTMALLOCMEM.error ("free_mem", ABORT, NULL);
  do {
    block = block->next;
  }
                                 //outside the block
  while ((chunk - block->blockstart < 0 || block->blockend - chunk <= 0)
    && block != currblock);

  if (chunk - block->blockstart < 0 || block->blockend - chunk <= 0)
                                 //in no block
    NOTMALLOCMEM.error ("free_mem", ABORT, NULL);

  chunk--;                       //point to size
  if (chunk->size == 0)
                                 //zero size
    FREEILLEGALPTR.error ("free_mem", ABORT, NULL);
  else if (chunk->size > 0)
                                 //already free
    FREEFREEDBLOCK.error ("free_mem", ABORT, NULL);
  chunk->size = -chunk->size;    //mark it free
  if (mem_freedepth > 0 && callers != NULL) {
                                 //count calls
    callers[chunk->owner].count_freeer (caller);
  }
  totalmem += chunk->size;       //total free memory
  if (chunk - block->freechunk < 0)
                                 //extra below
    block->lowerspace += chunk->size;
  else
                                 //extra above
    block->upperspace += chunk->size;
}


/**********************************************************************
 * MEM_ALLOCATOR::new_block
 *
 * Gets a new big block of memory from malloc for use by alloc_mem.
 **********************************************************************/

MEMBLOCK *MEM_ALLOCATOR::new_block(               //get new big block
                                   inT32 minsize  //minimum size
                                  ) {
  MEMBLOCK *newblock;            //new block

  if (blockcount >= MAXBLOCKS) {
                                 //can't have another
    NOMOREBLOCKS.error ("mem_new_block", TESSLOG, NULL);
    return NULL;
  }
  if (mem_checkfreq != 0) {
    tprintf ("\nGetting new block due to request size of %d",
      minsize * sizeof (MEMUNION));
    tprintf (" from %d from %d from %d from %d from %d\n",
      trace_caller (3), trace_caller (4), trace_caller (5),
      trace_caller (6), trace_caller (7));
    check_mem ("Getting new block", MEMCHECKS);
  }
                                 //get a new one
  newblock = &memblocks[blockcount++];
  while (memsize < minsize)
    memsize *= 4;                //go up in sizes
                                 //get a big block
  newblock->blockstart = (MEMUNION *)
    malloc (memsize * sizeof (MEMUNION));
  if (newblock->blockstart == NULL) {
    NOMOREMEM.error ("mem_new_block", TESSLOG, NULL);

    #ifdef __UNIX__
    raise(SIGTTOU);  //hangup for js
    #endif
    return NULL;
  }
                                 //end of block
  newblock->blockend = newblock->blockstart + memsize;
                                 //first free chunk
  newblock->freechunk = newblock->blockstart;
  newblock->topchunk = newblock->blockstart;
  newblock->lowerspace = 0;
  newblock->upperspace = memsize;//amount available
                                 //set pointer
  newblock->freechunk->size = memsize;
  newblock->freechunk->owner = 0;
  newblock->freechunk->age = 0;

  totalmem += memsize;           //total assigned mem

  if (memsize < maxsize)
    memsize *= 4;                //successively bigger
  if (currblock == NULL) {
    newblock->next = newblock;   //first block
  }
  else {
                                 //insert in list
    newblock->next = currblock->next;
    currblock->next = newblock;
  }
  return newblock;               //new block
}


/**********************************************************************
 * MEMBLOCK::find_chunk
 *
 * Find a chunk within the block which is big enough for the given request
 **********************************************************************/

MEMUNION *MEMBLOCK::find_chunk(             //find free chunk
                               inT32 count  //size required
                              ) {
  MEMUNION *chunk;               //current chunk
  inT32 chunksize;               //size of free chunk
  MEMUNION *chunkstart;          //start of free chunk
  inT32 spaceshift;              //shift in lowerspace

  if (upperspace <= lowerspace) {
    freechunk = blockstart;      //restart chunklist
    upperspace += lowerspace;
    lowerspace = 0;              //correct space counts
  }
  chunk = freechunk;             //current free chunk
  if (chunk->size < count) {     //big enough?
    spaceshift = 0;
    do {
      while (chunk->size < 0) {  //find free chunk
        chunk -= chunk->size;    //skip forward
        if (chunk == blockend) {
          chunk = blockstart;    //restart block
                                 //gone back to start
          spaceshift = -lowerspace;
        }
        if (chunk == freechunk)
          return chunk;          //gone all round & failed
      }
      chunkstart = chunk;        //start of chunk
      chunksize = chunk->size;;
      chunk += chunk->size;
      while (chunk != blockend   //until end
      && chunk->size > 0) {      //or used
        chunksize += chunk->size;//coalesce free blocks
                                 //gone all round
        if (chunk == freechunk) {
                                 //ensure it is at end
          freechunk += chunk->size;
          upperspace -= chunk->size;
          lowerspace += chunk->size;
          spaceshift -= chunk->size;
        }
        if (chunk == topchunk)   //got back to end one
          topchunk = chunkstart; //end one bigger
        chunk += chunk->size;    //get next block
      }
                                 //new big block
      chunkstart->size = chunksize;
      if (chunksize < count)
        spaceshift += chunksize; //skipping free block
      if (chunk == blockend) {
        chunk = blockstart;      //back to start
        if (freechunk == blockend) {
          freechunk = blockstart;//so is freechunk
          upperspace += lowerspace;
          lowerspace = 0;
          spaceshift = 0;
        }
        else
                                 //so is shift
            spaceshift = -lowerspace;
      }
    }
    while (chunksize < count && chunk != freechunk);
    if (chunksize < count)
      return chunk;              //failed
    lowerspace += spaceshift;    //get space counts right
    upperspace -= spaceshift;
    freechunk = chunkstart;
    return chunkstart;           //success
  }
  return chunk;                  //easy
}


#ifdef __UNIX__
/**********************************************************************
 * trace_caller
 *
 * Return the return address of the caller at a given depth back.
 * 0 gives the return address of the caller to trace_caller.
 * S300 ONLY!!
 **********************************************************************/
//#pragma OPTIMIZE OFF                                                                                                  /*force link*/

void *trace_caller(             //trace stack
                   inT32 depth  //depth to trace
                  ) {
  #ifdef hp9000s800

  unsigned sp, pc, rp;           //registers
  UWREC rec1;                    //for unwinder
  UWREC rec2;

  sp = (unsigned) (&depth + 9);
  pc = *(int *) (sp - 20);
  rp = 0;
  get_pcspace(&rec1, pc);
  rec1.cur_frsize = 0xc0;
  rec1.currlo = pc & ~3;
  rec1.curdp = 0;
  rec1.toprp = rp;

  while (depth > 0) {
    if (U_get_previous_frame (&rec1, &rec2))
      return NULL;
    rec1.currlo = rec2.currlo;
    rec1.cur_frsize = rec2.cur_frsize;
    rec1.cursp = rec2.cursp;
    rec1.currls = rec2.currls;
    rec1.curdp = rec2.curdp;
    depth--;
  }
  return (void *) rec1.currlo;
  #else
  void *a6;                      //address register

  a6 = &depth - 2;
  while (depth > 0) {
    a6 = *(void **) a6;          //follow chain
    depth--;
  }
  return *((void **) a6 + 1);
  #endif
}


//#pragma OPTIMIZE ON

#else

// Fake procedure for non-UNIX
void *trace_caller(             //trace stack
                   inT32 depth  //depth to trace
                  ) {
  return NULL;
}
#endif

/**********************************************************************
 * identify_struct_owner
 *
 * Get an index into the table of owners of structures.
 * Implemented very inefficiently, but only a debug tool!
 **********************************************************************/

inT32 identify_struct_owner(                     //get table index
                            inT32 struct_count,  //cell size
                            const char *name     //name of type
                           ) {
  inT32 index;                   //index to structure

  for (index = 0; index < name_counts[struct_count]
    && strcmp (name, owner_names[struct_count][index]); index++);
  if (index < MAX_CLASSES) {
    if (index == name_counts[struct_count]) {
      name_counts[struct_count]++;
      owner_names[struct_count][index] = name;
      owner_counts[struct_count][index] = 0;
    }
  }
  return index;
}


/**********************************************************************
 * check_struct
 *
 * Check a particular structure size for consistency.
 **********************************************************************/

void check_struct(             //check a structure
                  inT8 level,  //print control
                  inT32 count  //no of bytes
                 ) {
  MEMUNION *element;             //current element
  MEMUNION *block;               //current block
  inT32 struct_count;            //no of required structs
  inT32 block_count;             //no of structure blocks
  inT32 free_count;              //size of freelist*/
  inT32 name_index;              //named holder
  inT32 named_total;             //total held by names

                                 //no of MEMUNIONS-1
  struct_count = (count - 1) / sizeof (MEMUNION);
  if (struct_count < 0 || struct_count >= MAX_STRUCTS)
                                 //request too big
    MEMTOOBIG.error ("check_struct", ABORT, "%d", (int) count);

  free_count = 0;                //size of freelist
                                 //count blocks
  for (block_count = 0, block = struct_blocks[struct_count]; block != NULL; block = block->ptr, block_count++);
  if (block_count > 0) {
                                 //scan freelist
    for (element = free_structs[struct_count]; element != NULL; element = element->ptr)
      free_count++;
    if (level >= MEMCHECKS) {
      tprintf ("No of structs of size %d in use=%d,",
        (int) count, (int) structs_in_use[struct_count]);
      tprintf (" %d free", free_count);
      tprintf (" in %d blocks, total space=%d\n",
        (int) block_count,
        block_count * STRUCT_BLOCK_SIZE * sizeof (MEMUNION));
      for (named_total = 0, name_index = 0;
      name_index < name_counts[struct_count]; name_index++) {
        tprintf ("No held by %s=%d\n",
          owner_names[struct_count][name_index],
          owner_counts[struct_count][name_index]);
        named_total += owner_counts[struct_count][name_index];
      }
      tprintf ("Total held by names=%d\n", named_total);
    }
  }
  if (structs_in_use[struct_count] + free_count
    != block_count * (STRUCT_BLOCK_SIZE / (struct_count + 1) - 1))
    BADSTRUCTCOUNT.error ("check_struct", ABORT, "%d+%d=%d",
      structs_in_use[struct_count], free_count,
      block_count * (STRUCT_BLOCK_SIZE /
      (struct_count + 1) - 1));
}


/**********************************************************************
 * check_structs
 *
 * Reports statistics on each maintained structure type by calling
 * free_struct(NULL) on each.  Only active structure types are reported.
 **********************************************************************/

void check_structs(            //count in use on structs
                   inT8 level  //print control
                  ) {
  inT8 index;                    //index to structs

  for (index = 1; index <= MAX_STRUCTS; index++)
                                 //check number allocated
    check_struct (level, index * sizeof (MEMUNION));
}


/**********************************************************************
 * new_struct_block
 *
 * Allocate space for a new block of structures.  The space is obtained
 * from alloc_mem, and a freelist of such blocks is maintained for when
 * the individual structure types get completely freed.
 **********************************************************************/

void *new_struct_block() {  //allocate memory
  MEMUNION *element;             //current element
  MEMUNION *returnelement;       //return value

  returnelement = free_block;
  if (returnelement == NULL) {
                                 //need a new block
    element =
      (MEMUNION *) alloc_mem_p (STRUCT_BLOCK_SIZE * sizeof (MEMUNION));
    if (element == NULL)
      return NULL;               //can't get more
    returnelement = element;     //going to return 1st
  }
  else {
                                 //new free one
    free_block = returnelement->ptr;
  }
  return returnelement;          //free cell
}


/**********************************************************************
 * old_struct_block
 *
 * Free memory allocated by new_struct_block.  The block is returned
 * to a freelist ready for a new call to new_struct_block.
 * This saves confusion over freeing "permanent" blocks, yet
 * allows them to be recycled for different structures.
 **********************************************************************/

void old_struct_block(                     //free a structure block
                      MEMUNION *deadblock  //block to free
                     ) {
  if (deadblock != NULL) {
    deadblock->ptr = free_block; //add to freelist
    free_block = deadblock;
    free_struct_blocks++;
  }
  if (free_struct_blocks > MAX_FREE_S_BLOCKS) {
    MEMUNION *next_block;        //next in list
    deadblock = free_block;
    do {
      next_block = deadblock->ptr;
      free_mem(deadblock);  //really free it
      deadblock = next_block;
    }
    while (deadblock != NULL);
    free_struct_blocks = 0;
    free_block = NULL;
  }
}
