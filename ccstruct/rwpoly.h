/**********************************************************************
 * File:        rwpoly.h  (Formerly rw_poly.h)
 * Description: latest version of manual page decomp tool
 * Author:      Sheelagh Lloyd
 * Created:     16:05 24/3/93
 *
 * This version constructs a list of blocks.
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef                    RWPOLY_H
#define                    RWPOLY_H

#include          <ctype.h>
#include          <math.h>
#ifdef __MSW32__
#include          <io.h>
#else
#include          <stdlib.h>
#endif
#include          "elst.h"
#include          "pageblk.h"
#include          "varable.h"

#include          "hpddef.h"     //must be last (handpd.dll)

DLLSYM void write_poly_blocks(FILE *blfile, PAGE_BLOCK_LIST *blocks); 

extern DLLSYM PAGE_BLOCK_LIST *page_block_list;
extern PAGE_BLOCK_IT page_block_it;
                                 //read file
DLLSYM PAGE_BLOCK_LIST *read_poly_blocks(const char *name  //file to read
                                        );
#endif
