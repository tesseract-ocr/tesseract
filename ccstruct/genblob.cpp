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

#include "mfcpch.h"
#include          "stepblob.h"
#include          "polyblob.h"
#include          "genblob.h"

/**********************************************************************
 *  blob_comparator()
 *
 *  Blob comparator used to sort a blob list so that blobs are in increasing
 *  order of left edge.
 **********************************************************************/

int blob_comparator(                     //sort blobs
                    const void *blob1p,  //ptr to ptr to blob1
                    const void *blob2p   //ptr to ptr to blob2
                   ) {
  PBLOB *blob1 = *(PBLOB **) blob1p;
  PBLOB *blob2 = *(PBLOB **) blob2p;

  return blob1->bounding_box ().left () - blob2->bounding_box ().left ();
}


/**********************************************************************
 *  c_blob_comparator()
 *
 *  Blob comparator used to sort a blob list so that blobs are in increasing
 *  order of left edge.
 **********************************************************************/

int c_blob_comparator(                     //sort blobs
                      const void *blob1p,  //ptr to ptr to blob1
                      const void *blob2p   //ptr to ptr to blob2
                     ) {
  C_BLOB *blob1 = *(C_BLOB **) blob1p;
  C_BLOB *blob2 = *(C_BLOB **) blob2p;

  return blob1->bounding_box ().left () - blob2->bounding_box ().left ();
}


/**********************************************************************
 *  gblob_bounding_box()
 *
 *  Return the bounding box of a generic blob.
 **********************************************************************/

TBOX gblob_bounding_box(                 //Get bounding box
                       PBLOB *blob,     //generic blob
                       BOOL8 polygonal  //is blob polygonal?
                      ) {
  if (polygonal)
    return blob->bounding_box ();
  else
    return ((C_BLOB *) blob)->bounding_box ();
}


/**********************************************************************
 *  gblob_sort_list()
 *
 *  Sort a generic blob list into order of bounding box left edge
 **********************************************************************/

void gblob_sort_list(                        //Sort a gblob list
                     PBLOB_LIST *blob_list,  //generic blob list
                     BOOL8 polygonal         //is list polygonal?
                    ) {
  PBLOB_IT b_it;
  C_BLOB_IT c_it;

  if (polygonal) {
    b_it.set_to_list (blob_list);
    b_it.sort (blob_comparator);
  }
  else {
    c_it.set_to_list ((C_BLOB_LIST *) blob_list);
    c_it.sort (c_blob_comparator);
  }
}


/**********************************************************************
 *  gblob_out_list()
 *
 *  Return the generic outline list of a generic blob.
 **********************************************************************/

OUTLINE_LIST *gblob_out_list(                 //Get outline list
                             PBLOB *blob,     //generic blob
                             BOOL8 polygonal  //is blob polygonal?
                            ) {
  if (polygonal)
    return blob->out_list ();
  else
    return (OUTLINE_LIST *) ((C_BLOB *) blob)->out_list ();
}


/**********************************************************************
 *  goutline_bounding_box()
 *
 *  Return the bounding box of a generic outline.
 **********************************************************************/

TBOX goutline_bounding_box(                   //Get bounding box
                          OUTLINE *outline,  //generic outline
                          BOOL8 polygonal    //is outline polygonal?
                         ) {
  if (polygonal)
    return outline->bounding_box ();
  else
    return ((C_OUTLINE *) outline)->bounding_box ();
}
