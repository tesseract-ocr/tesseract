/**************************************************************************
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
**************************************************************************/
#include "freelist.h"

#include <memory.h>

#include "danerror.h"
#include "memry.h"
#include "tprintf.h"

static int mem_alloc_counter = 0;


/**********************************************************************
 * memalloc
 *
 * Memory allocator with protection.
 **********************************************************************/
int *memalloc(int size) {
  mem_alloc_counter++;
  return ((int *) alloc_mem (size));
}


/**********************************************************************
 * memrealloc
 *
 * Memory allocator with protection.
 **********************************************************************/
int *memrealloc(void *ptr, int size, int oldsize) {
  int shiftsize;
  int *newbuf;

  shiftsize = size > oldsize ? oldsize : size;
  newbuf = (int *) alloc_mem (size);
  memcpy(newbuf, ptr, shiftsize);
  free_mem(ptr);
  return newbuf;
}


/**********************************************************************
 * memfree
 *
 * Memory allocator with protection.
 **********************************************************************/
void memfree(void *element) {
  if (element) {
    free_mem(element);
    mem_alloc_counter--;
  }
  else {
    tprintf ("%d MEM_ALLOC's used\n", mem_alloc_counter);
    DoError (0, "Memfree of NULL pointer");
  }
}


/**********************************************************************
 * mem_tidy
 *
 * Do nothing.
 **********************************************************************/
void mem_tidy(int level) {
  check_mem ("Old tidy", level);
}
