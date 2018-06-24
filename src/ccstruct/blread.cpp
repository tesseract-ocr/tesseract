/**********************************************************************
 * File:        blread.cpp  (Formerly pdread.c)
 * Description: Friend function of BLOCK to read the uscan pd file.
 * Author:      Ray Smith
 * Created:     Mon Mar 18 14:39:00 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include "blread.h"
#include <cstdio>       // for fclose, fopen, FILE
#include "host.h"       // for TRUE
#include "ocrblock.h"   // for BLOCK_IT, BLOCK, BLOCK_LIST (ptr only)
#include "scanutils.h"  // for tfscanf

#define UNLV_EXT  ".uzn"  // unlv zone file

/**********************************************************************
 * read_unlv_file
 *
 * Read a whole unlv zone file to make a list of blocks.
 **********************************************************************/

bool read_unlv_file(                    //print list of sides
                     STRING name,        //basename of file
                     int32_t xsize,        //image size
                     int32_t ysize,        //image size
                     BLOCK_LIST *blocks  //output list
                    ) {
  FILE *pdfp;                    //file pointer
  BLOCK *block;                  //current block
  int x;                         //current top-down coords
  int y;
  int width;                     //of current block
  int height;
  BLOCK_IT block_it = blocks;    //block iterator

  name += UNLV_EXT;              //add extension
  if ((pdfp = fopen (name.string (), "rb")) == nullptr) {
    return false;                //didn't read one
  } else {
    while (tfscanf(pdfp, "%d %d %d %d %*s", &x, &y, &width, &height) >= 4) {
                                 //make rect block
      block = new BLOCK (name.string (), TRUE, 0, 0,
                         (int16_t) x, (int16_t) (ysize - y - height),
                         (int16_t) (x + width), (int16_t) (ysize - y));
                                 //on end of list
      block_it.add_to_end (block);
    }
    fclose(pdfp);
  }
  return true;
}

void FullPageBlock(int width, int height, BLOCK_LIST *blocks) {
  BLOCK_IT block_it(blocks);
  BLOCK* block = new BLOCK("", TRUE, 0, 0, 0, 0, width, height);
  block_it.add_to_end(block);
}
