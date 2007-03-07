/**********************************************************************
 * File:        pdclass.h  (Formerly pdstruct.h)
 * Description: Data structures for read_vec_file.
 * Author:		Ray Smith
 * Created:		Tue Nov  3 11:42:08 GMT 1992
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

#ifndef           PDCLASS_H
#define           PDCLASS_H

#include          "points.h"

struct VEC_HEADER
{
  INT32 filesize;                //bytes in file
  INT16 bytesize;                //sizeof a byte
  INT16 arraysize;               //no of blocks
  INT16 width;                   //of image
  INT16 height;
  INT16 res;                     //not set
  INT16 bpp;
};

struct BLOCK_HEADER
{
  UINT8 type;                    //block type
  UINT8 valid;                   //useable flag
  UINT8 charsize;                //blob size
  UINT8 downsamplerate;          //??
  UINT8 subtype;                 //??
  UINT8 temp;                    //??
  UINT16 offset;                 //index in vectors
  UINT16 order;                  //block number
  UINT16 entries;                //no of vectors
};

struct VEC_ENTRY
{
  ICOORD start;                  //start of vector
  ICOORD end;                    //in clockwise dir
};
#endif
