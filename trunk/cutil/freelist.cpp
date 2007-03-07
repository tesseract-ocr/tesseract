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
#include "danerror.h"
#include "callcpp.h"

#include <memory.h>

static int mem_alloc_counter = 0;

/**********************************************************************
 * memalloc_p
 *
 * Memory allocator with protection.
 **********************************************************************/
int *memalloc_p(int size) {
  mem_alloc_counter++;
  if (!size)
    DoError (0, "Allocation of 0 bytes");
  return ((int *) c_alloc_mem_p (size));
}


/**********************************************************************
 * memalloc
 *
 * Memory allocator with protection.
 **********************************************************************/
int *memalloc(int size) {
  mem_alloc_counter++;
  return ((int *) c_alloc_mem (size));
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
  newbuf = (int *) c_alloc_mem (size);
  memcpy(newbuf, ptr, shiftsize);
  c_free_mem(ptr);
  return newbuf;
}


/**********************************************************************
 * memfree
 *
 * Memory allocator with protection.
 **********************************************************************/
void memfree(void *element) {
  if (element) {
    c_free_mem(element);
    mem_alloc_counter--;
  }
  else {
    cprintf ("%d MEM_ALLOC's used\n", mem_alloc_counter);
    DoError (0, "Memfree of NULL pointer");
  }
}


/**********************************************************************
 * mem_tidy
 *
 * Do nothing.
 **********************************************************************/
void mem_tidy(int level) {
  c_check_mem ("Old tidy", level);
}
