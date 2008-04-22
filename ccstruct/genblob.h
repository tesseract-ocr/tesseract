/**********************************************************************
 * File:        genblob.h  (Formerly gblob.h)
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

#ifndef           GENBLOB_H
#define           GENBLOB_H

#include          "polyblob.h"
#include          "hosthplb.h"
#include          "rect.h"
#include          "notdll.h"

int blob_comparator(                     //sort blobs
                    const void *blob1p,  //ptr to ptr to blob1
                    const void *blob2p   //ptr to ptr to blob2
                   );
int c_blob_comparator(                     //sort blobs
                      const void *blob1p,  //ptr to ptr to blob1
                      const void *blob2p   //ptr to ptr to blob2
                     );
TBOX gblob_bounding_box(                 //Get bounding box
                       PBLOB *blob,     //generic blob
                       BOOL8 polygonal  //is blob polygonal?
                      );
void gblob_sort_list(                        //Sort a gblob list
                     PBLOB_LIST *blob_list,  //generic blob list
                     BOOL8 polygonal         //is list polygonal?
                    );
OUTLINE_LIST *gblob_out_list(                 //Get outline list
                             PBLOB *blob,     //generic blob
                             BOOL8 polygonal  //is blob polygonal?
                            );
TBOX goutline_bounding_box(                   //Get bounding box
                          OUTLINE *outline,  //generic outline
                          BOOL8 polygonal    //is outline polygonal?
                         );
#endif
