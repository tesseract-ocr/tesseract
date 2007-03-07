/**********************************************************************
 * File:        memryerr.h        (Formerly: memerr.h)
 * Description: File defining error messages for memory functions.
 * Author:      Ray Smith
 * Created:     Mon Aug 13 12:51:03 BST 1990
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

#ifndef           MEMRYERR_H
#define           MEMRYERR_H

#include          "errcode.h"

const ERRCODE MEMTOOBIG = "Memory request too big";
const ERRCODE NOMOREBLOCKS = "Max total memory blocks exceeded";
const ERRCODE NOMOREMEM = "No more memory available from malloc";
const ERRCODE FREENULLPTR = "Attempt to free memory NULL pointer";
const ERRCODE NOTMALLOCMEM =
"Attempt to free memory not belonging to memalloc";
const ERRCODE FREEILLEGALPTR = "Pointer or memory corrupted";
const ERRCODE FREEFREEDBLOCK = "Memory block already marked free";
const ERRCODE BADMEMCHUNKS = "Inconsistency in memory chunks";
const ERRCODE BADSTRUCTCOUNT = "Memory incorrect freelist length";
const ERRCODE NEGATIVE_USED_STRUCTS =
"Negative number of used memory structures";
const ERRCODE NOTASTRING = "Illegal pointer for memory strfree";
#endif
