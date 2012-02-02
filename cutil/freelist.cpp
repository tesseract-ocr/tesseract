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

#include <stdlib.h>


// With improvements in OS memory allocators, internal memory management is
// no longer required, so these functions all map to their malloc-family
// equivalents.


int *memalloc(int size) {
  return static_cast<int*>(malloc(static_cast<size_t>(size)));
}

int *memrealloc(void *ptr, int size, int oldsize) {
  return static_cast<int*>(realloc(ptr, static_cast<size_t>(size)));
}

void memfree(void *element) {
  free(element);
}
