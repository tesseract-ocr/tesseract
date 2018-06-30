/**********************************************************************
 * File:        genblob.cpp  (Formerly gblob.c)
 * Description: Generic Blob processing routines
 * Author:      Phil Cheatle
 * Created:     Mon Nov 25 10:53:26 GMT 1991
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

#include          "genblob.h"
#include          "stepblob.h"

/**********************************************************************
 *  c_blob_comparator()
 *
 *  Blob comparator used to sort a blob list so that blobs are in increasing
 *  order of left edge.
 **********************************************************************/

int c_blob_comparator(                     // sort blobs
                      const void *blob1p,  // ptr to ptr to blob1
                      const void *blob2p   // ptr to ptr to blob2
                     ) {
  C_BLOB *blob1 = *(C_BLOB **) blob1p;
  C_BLOB *blob2 = *(C_BLOB **) blob2p;

  return blob1->bounding_box ().left () - blob2->bounding_box ().left ();
}
