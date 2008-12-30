/**********************************************************************
 * File:        memry.c  (Formerly memory.c)
 * Description: Memory allocation with builtin safety checks.
 * Author:					Ray Smith
 * Created:					Wed Jan 22 09:43:33 GMT 1992
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

#include          "mfcpch.h"
#include          <stdlib.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "stderr.h"
#include          "tprintf.h"
#include          "memblk.h"
#include          "memry.h"

//#define COUNTING_CLASS_STRUCTURES

/**********************************************************************
 * new
 *
 * Replace global new to get at memory leaks etc.
 **********************************************************************/
/*
void*						operator new(				//allocate memory
size_t						size						//amount to allocate
)
{
  if (size==0)
  {
    err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
      ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
      "Zero requested of new");
    size=1;
  }
  return alloc_big_mem(size);
}

void						operator delete(			//free memory
void*						addr						//mem to free
)
{
  free_big_mem(addr);
}*/

/**********************************************************************
 * check_mem
 *
 * Check consistency of all memory controlled by alloc_mem.
 **********************************************************************/

DLLSYM void check_mem(                     //check consistency
                      const char *string,  //context message
                      inT8 level           //level of check
                     ) {
  big_mem.check (string, level);
  main_mem.check (string, level);
  check_structs(level);
}


/**********************************************************************
 * alloc_string
 *
 * Allocate space for a string.  The space can only be used for chars as
 * it is not aligned.  Allocation is guaranteed to be fast and not cause
 * fragmentation for small strings (upto 10*worst alignment).  Calls for
 * larger strings will be satisfied with alloc_mem.
 * Use free_string to free the space from alloc_string.
 **********************************************************************/

DLLSYM char *alloc_string(             //allocate string
                          inT32 count  //no of chars required
                         ) {
#ifdef RAYS_MALLOC
  char *string;                  //allocated string

  if (count < 1 || count > MAX_CHUNK) {
    tprintf ("Invalid size %d requested of alloc_string", count);
    return NULL;
  }

  count++;                       //add size byte
  if (count <= MAX_STRUCTS * sizeof (MEMUNION)) {
    string = (char *) alloc_struct (count, "alloc_string");
    //get a fast structure
    if (string == NULL) {
      tprintf ("No memory for alloc_string");
      return NULL;
    }
    string[0] = (inT8) count;    //save its length
  }
  else {
                                 //get a big block
    string = (char *) alloc_mem (count);
    if (string == NULL) {
      tprintf ("No memory for alloc_string");
      return NULL;
    }
    string[0] = 0;               //mark its id
  }
  return &string[1];             //string for user
#else
  // Round up the amount allocated to a multiple of 4
  return static_cast<char*>(malloc((count + 3) & ~3));
#endif
}


/**********************************************************************
 * free_string
 *
 * Free a string allocated by alloc_string.
 **********************************************************************/

DLLSYM void free_string(              //free a string
                        char *string  //string to free
                       ) {
#ifdef RAYS_MALLOC
  if (((ptrdiff_t) string & 3) == 1) { //one over word
    string--;                    //get id marker
    if (*string == 0) {
      free_mem(string);  //generally free it
      return;
    }
    else if (*string <= MAX_STRUCTS * sizeof (MEMUNION)) {
                                 //free structure
      free_struct (string, *string, "alloc_string");
      return;
    }
  }
  tprintf ("Non-string given to free_string");
#else
  free(string);
#endif
}


/**********************************************************************
 * alloc_struct
 *
 * Allocate space for a structure.  This function is designed to be
 * fast and fragmentation free for arbitrary combinations of small
 * objects. (Upto 40 bytes in length.)
 * It can be used for any size of object up to 512K, but you must use
 * free_struct to release the memory it gives.  alloc_mem is better
 * for arbitrary data blocks of large size (>40 bytes.)
 * alloc_struct always aborts if the allocation fails.
 **********************************************************************/

DLLSYM void *
alloc_struct (                   //allocate memory
inT32 count,                     //no of chars required
#if defined COUNTING_CLASS_STRUCTURES
const char *name                 //name of type
#else
const char *                     //name of type
#endif
) {
#ifdef RAYS_MALLOC
  MEMUNION *element;             //current element
  MEMUNION *returnelement;       //return value
  inT32 struct_count;            //no of required structs
  inT32 blocksize;               //no of structs in block
  inT32 index;                   //index to structure

  if (count < 1 || count > MAX_CHUNK) {
    tprintf ("Invalid size %d requested of alloc_struct", count);
    return NULL;
  }

  //      tprintf("Allocating structure of size %d\n",count);
                                 //no of MEMUNIONS-1
  struct_count = (count - 1) / sizeof (MEMUNION);
  if (struct_count < MAX_STRUCTS) {
                                 //can do fixed sizes
    #ifdef COUNTING_CLASS_STRUCTURES
    if (name != NULL) {
      index = identify_struct_owner (struct_count, name);
      if (index < MAX_CLASSES)
        owner_counts[struct_count][index]++;
    }
    #endif
                                 //head of freelist
    returnelement = free_structs[struct_count];
    if (returnelement == NULL) {
                                 //need a new block
                                 //get one
      element = (MEMUNION *) new_struct_block ();
      if (element == NULL) {
        tprintf ("No memory to satisfy request for %d", (int) count);
        return NULL;
      }
                                 //add to block list
      element->ptr = struct_blocks[struct_count];
      struct_blocks[struct_count] = element;
      blocks_in_use[struct_count]++;
      element++;                 //free cell
      returnelement = element;   //going to return 1st
      blocksize = STRUCT_BLOCK_SIZE / (struct_count + 1) - 1;

      for (index = 1; index < blocksize; index++) {
                                 //make links
        element->ptr = element + struct_count + 1;
        element += struct_count + 1;
      }
      element->ptr = NULL;       //end of freelist
    }
                                 //new free one
    free_structs[struct_count] = returnelement->ptr;
                                 //count number issued
    structs_in_use[struct_count]++;
  }
  else {
                                 //just get some
    returnelement = (MEMUNION *) alloc_mem (count);
    if (returnelement == NULL) {
      tprintf ("No memory to satisfy request for %d", (int) count);
      return NULL;
    }
  }
  return returnelement;          //free cell
#else
  return malloc(count);
#endif
}


/**********************************************************************
 * free_struct
 *
 * Free memory allocated by alloc_struct.  The size must be supplied.
 **********************************************************************/

DLLSYM void
free_struct (                    //free a structure
void *deadstruct,                //structure to free
inT32 count,                     //no of bytes
#if defined COUNTING_CLASS_STRUCTURES
const char *name                 //name of type
#else
const char *                     //name of type
#endif
) {
#ifdef RAYS_MALLOC
  MEMUNION *end_element;         //current element
  MEMUNION *element;             //current element
  MEMUNION *prev_element;        //previous element
  MEMUNION *prev_block;          //previous element
  MEMUNION *nextblock;           //next block in list
  MEMUNION *block;               //next block in list
  inT32 struct_count;            //no of required structs
  inT32 index;                   //to structure counts

  if (count < 1 || count > MAX_CHUNK) {
    tprintf ("Invalid size %d requested of free_struct", count);
    return;
  }

  //      tprintf("Freeing structure of size %d\n",count);
                                 //no of MEMUNIONS-1
  struct_count = (count - 1) / sizeof (MEMUNION);

  if (deadstruct == NULL) {
                                 //not really legal
    check_struct(MEMCHECKS, count);
  }
  else {
    if (struct_count < MAX_STRUCTS) {
                                 //can do fixed sizes
      #ifdef COUNTING_CLASS_STRUCTURES
      if (name != NULL) {
        index = identify_struct_owner (struct_count, name);
        if (index < MAX_CLASSES) {
          owner_counts[struct_count][index]--;
          ASSERT_HOST (owner_counts[struct_count][index] >= 0);
        }
      }
      #endif
      element = (MEMUNION *) deadstruct;
                                 //add to freelist
      element->ptr = free_structs[struct_count];
      free_structs[struct_count] = element;
                                 //one less in use
      structs_in_use[struct_count]--;
      if (structs_in_use[struct_count] == 0) {
        index = 0;
        for (element = struct_blocks[struct_count];
        element != NULL; element = nextblock) {
                                 //traverse and destroy
          nextblock = element->ptr;
                                 //free all the blocks
          old_struct_block(element);
          index++;
        }
                                 //none left any more
        struct_blocks[struct_count] = NULL;
                                 //no free structs
        free_structs[struct_count] = NULL;
        blocks_in_use[struct_count] = 0;
      }
      else if (structs_in_use[struct_count] < 0) {
        tprintf ("Negative number of structs of size %d in use",
          (int) count);
      }
      else if (structs_in_use[struct_count] < blocks_in_use[struct_count]) {
        prev_block = NULL;
        for (block = struct_blocks[struct_count];
        block != NULL; block = nextblock) {
          nextblock = block;
          index = STRUCT_BLOCK_SIZE / (struct_count + 1) - 1;
          end_element = block + STRUCT_BLOCK_SIZE;
          for (element = free_structs[struct_count];
          element != NULL; element = element->ptr) {
            if (element > nextblock && element < end_element) {
              index--;
              if (index == 0)
                break;
            }
          }
          if (index == 0) {
            index = STRUCT_BLOCK_SIZE / (struct_count + 1) - 1;
            for (element =
              free_structs[struct_count], prev_element = NULL;
            element != NULL; element = element->ptr) {
              if (element > nextblock && element < end_element) {
                index--;
                if (prev_element != NULL)
                  prev_element->ptr = element->ptr;
                else
                  free_structs[struct_count] = element->ptr;
                if (index == 0)
                  break;
              }
              else
                prev_element = element;
            }
            if (prev_block != NULL)
              prev_block->ptr = block->ptr;
            else
              struct_blocks[struct_count] = block->ptr;
            nextblock = block->ptr;
            blocks_in_use[struct_count]--;
                                 //free all the blocks
            old_struct_block(block);
          }
          else {
            prev_block = block;
                                 //traverse and destroy
            nextblock = block->ptr;
          }
        }
      }
    }
    else
      free_mem(deadstruct);  //free directly
  }
#else
  free(deadstruct);
#endif
}


/**********************************************************************
 * alloc_mem_p
 *
 * Allocate permanent space which will never be returned.
 * This space is allocated from the top end of a memory block to
 * avoid the fragmentation which would result from alternate use
 * of alloc_mem for permanent and temporary blocks.
 **********************************************************************/

//#ifdef __UNIX__
//#pragma OPT_LEVEL 0
//#endif
DLLSYM void *alloc_mem_p(             //allocate permanent space
                         inT32 count  //block size to allocate
                        ) {
  #ifdef RAYS_MALLOC
  #ifdef TESTING_BIGSTUFF
  if (main_mem.biggestblock == 0)
    main_mem.init (alloc_big_mem, free_big_mem,
      FIRSTSIZE, LASTSIZE, MAX_CHUNK);
  #else
  if (main_mem.biggestblock == 0)
    main_mem.init ((void *(*)(inT32)) malloc, free,
      FIRSTSIZE, LASTSIZE, MAX_CHUNK);
  #endif
  if (mem_mallocdepth > 0)
    return main_mem.alloc_p (count, trace_caller (mem_mallocdepth));
  else
    return main_mem.alloc_p (count, NULL);
  #else
  return malloc ((size_t) count);
  #endif
}


/**********************************************************************
 * alloc_mem
 *
 * Return a pointer to a buffer of count bytes aligned for any type.
 **********************************************************************/

DLLSYM void *alloc_mem(             //get some memory
                       inT32 count  //no of bytes to get
                      ) {
  #ifdef RAYS_MALLOC
  #ifdef TESTING_BIGSTUFF
  if (main_mem.biggestblock == 0)
    main_mem.init (alloc_big_mem, free_big_mem,
      FIRSTSIZE, LASTSIZE, MAX_CHUNK);
  #else
  if (main_mem.biggestblock == 0)
    main_mem.init ((void *(*)(inT32)) malloc, free,
      FIRSTSIZE, LASTSIZE, MAX_CHUNK);
  #endif
  if (mem_mallocdepth > 0)
    return main_mem.alloc (count, trace_caller (mem_mallocdepth));
  else
    return main_mem.alloc (count, NULL);
  #else
  return malloc ((size_t) count);
  #endif
}


/**********************************************************************
 * alloc_big_mem
 *
 * Return a pointer to a buffer of count bytes aligned for any type.
 **********************************************************************/

DLLSYM void *alloc_big_mem(             //get some memory
                           inT32 count  //no of bytes to get
                          ) {
  #ifdef TESTING_BIGSTUFF
  if (big_mem.biggestblock == 0)
    big_mem.init ((void *(*)(inT32)) malloc, free,
      BIGSIZE, BIGSIZE, MAX_BIGCHUNK);
  if (mem_mallocdepth > 0)
    return big_mem.alloc (count, trace_caller (mem_mallocdepth));
  else
    return big_mem.alloc (count, NULL);
  #else
  return malloc ((size_t) count);
  #endif
}


/**********************************************************************
 * alloc_big_zeros
 *
 * Return a pointer to a buffer of count bytes aligned for any type.
 **********************************************************************/

DLLSYM void *alloc_big_zeros(             //get some memory
                             inT32 count  //no of bytes to get
                            ) {
  #ifdef TESTING_BIGSTUFF
  if (big_mem.biggestblock == 0)
    big_mem.init ((void *(*)(inT32)) malloc, free,
      BIGSIZE, BIGSIZE, MAX_BIGCHUNK);
  void *buf;                     //return value

  if (mem_mallocdepth > 0)
    buf = big_mem.alloc (count, trace_caller (mem_mallocdepth));
  else
    buf = big_mem.alloc (count, NULL);
  memset (buf, 0, count);
  return buf;
  #else
  return calloc ((size_t) count, 1);
  #endif
}


/**********************************************************************
 * free_mem
 *
 * Free a block allocated by alloc_mem (or alloc_mem_p).
 * It checks that the pointer is legal and maintains counts of the
 * amount of free memory above and below the current free pointer.
 **********************************************************************/

DLLSYM void free_mem(                //free mem from alloc_mem
                     void *oldchunk  //chunk to free
                    ) {
  #ifdef RAYS_MALLOC
  if (mem_freedepth > 0 && main_mem.callers != NULL)
    main_mem.dealloc (oldchunk, trace_caller (mem_freedepth));
  else
    main_mem.dealloc (oldchunk, NULL);
  #else
  free(oldchunk);
  #endif
}


/**********************************************************************
 * free_big_mem
 *
 * Free a block allocated by alloc_big_mem.
 * It checks that the pointer is legal and maintains counts of the
 * amount of free memory above and below the current free pointer.
 **********************************************************************/

DLLSYM void free_big_mem(                //free mem from alloc_mem
                         void *oldchunk  //chunk to free
                        ) {
  #ifdef TESTING_BIGSTUFF
  if (mem_freedepth > 0 && main_mem.callers != NULL)
    big_mem.dealloc (oldchunk, trace_caller (mem_freedepth));
  else
    big_mem.dealloc (oldchunk, NULL);
  #else
  free(oldchunk);
  #endif
}
