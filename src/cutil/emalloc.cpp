/**************************************************************************
 * Filename:    emalloc.cpp
 * Purpose:     Routines for trapping memory allocation errors.
 * Author:      Dan Johnson
**
**  (c) Copyright Hewlett-Packard Company, 1988.
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

#include "emalloc.h"
#include <cstdlib>
#include "errcode.h"    // for ASSERT_HOST

/**
 * This routine attempts to allocate the specified number of
 * bytes.  If the memory can be allocated, a pointer to the
 * memory is returned.  If the memory cannot be allocated, or
 * if the allocation request is negative or zero,
 * an error is trapped.
 * @param Size number of bytes of memory to be allocated
 * @return Pointer to allocated memory.
 */
void *Emalloc(int Size) {
  ASSERT_HOST(Size > 0);
  void* Buffer = malloc(Size);
  ASSERT_HOST(Buffer != nullptr);
  return Buffer;
}

void *Erealloc(void *ptr, int size) {
  ASSERT_HOST(size > 0 || (size == 0 && ptr != nullptr));
  void* Buffer = realloc(ptr, size);
  ASSERT_HOST(Buffer != nullptr || size == 0);
  return Buffer;
}

void Efree(void *ptr) {
  ASSERT_HOST(ptr != nullptr);
  free(ptr);
}
