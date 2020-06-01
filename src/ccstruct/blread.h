/**********************************************************************
 * File:        blread.h  (Formerly pdread.h)
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

#ifndef BLREAD_H
#define BLREAD_H

#include <cstdint>      // for int32_t
#include <tesseract/strngs.h>     // for STRING

class BLOCK_LIST;

bool read_unlv_file(                    //print list of sides
                     STRING name,        //basename of file
                     int32_t xsize,        //image size
                     int32_t ysize,        //image size
                     BLOCK_LIST *blocks  //output list
                    );
void FullPageBlock(int width, int height, BLOCK_LIST *blocks);

#endif
