/**********************************************************************
 * File:        memry.h  (Formerly memory.h)
 * Description: Header file for basic memory allocation/deallocation.
 * Author:      Ray Smith
 * Created:     Tue May  8 16:03:48 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef           MEMRY_H
#define           MEMRY_H

#include          <stddef.h>
#include          "host.h"

// allocate string
extern char *alloc_string(inT32 count);
// free a string.
extern void free_string(char *string);
// allocate memory
extern void *alloc_struct(inT32 count, const char *name = NULL);
// free a structure.
extern void free_struct(void *deadstruct, inT32, const char *name = NULL);
// get some memory
extern void *alloc_mem(inT32 count);
// get some memory initialized to 0.
extern void *alloc_big_zeros(inT32 count);
// free mem from alloc_mem
extern void free_mem(void *oldchunk);
// free mem from alloc_big_zeros
extern void free_big_mem(void *oldchunk);

#endif
