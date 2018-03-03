/**********************************************************************
 * File:        memry.cpp  (Formerly memory.c)
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

#include          "memry.h"
#include          <stdlib.h>

// With improvements in OS memory allocators, internal memory management
// is no longer required, so all these functions now map to their malloc
// family equivalents.

// TODO(rays) further cleanup by redirecting calls to new and creating proper
// constructors.

char *alloc_string(int32_t count) {
  // Round up the amount allocated to a multiple of 4
  return static_cast<char*>(malloc((count + 3) & ~3));
}

void free_string(char *string) {
  free(string);
}

void *alloc_mem(int32_t count) {
  return malloc(static_cast<size_t>(count));
}

void free_mem(void *oldchunk) {
  free(oldchunk);
}
