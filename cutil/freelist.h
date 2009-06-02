/* -*-C-*-
 ********************************************************************************
 *
 * File:        freelist.h  (Formerly freelist.h)
 * Description:  Memory allocator
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 30 13:50:28 1990
 * Modified:     Mon Dec 10 15:15:25 1990 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 *********************************************************************************/

#ifndef FREELIST_H
#define FREELIST_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int *memalloc(int size);

int *memrealloc(void *ptr, int size, int oldsize);

void memfree(void *element);

void mem_tidy(int level);

#endif
