/**********************************************************************
 * File:        edgblob.c  (Formerly edgeloop.c)
 * Description: Functions to clean up an outline before approximation.
 * Author:		Ray Smith
 * Created:		Tue Mar 26 16:56:25 GMT 1991
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
//#include                                      "dirtab.h"
#include          "scanedg.h"
#include          "drawedg.h"
#include          "edgloop.h"
#include          "edgblob.h"

#define EXTERN

EXTERN INT_VAR (edges_children_per_grandchild, 10,
"Importance ratio for chucking outlines");
EXTERN INT_VAR (edges_children_count_limit, 45, "Max holes allowed in blob");
EXTERN BOOL_VAR (edges_children_fix, FALSE,
"Remove boxy parents of char-like children");
EXTERN INT_VAR (edges_min_nonhole, 12,
"Min pixels for potential char in box");
EXTERN INT_VAR (edges_patharea_ratio, 40,
"Max lensq/area for acceptable child outline");
EXTERN double_VAR (edges_childarea, 0.5,
"Max area fraction of child outline");
EXTERN double_VAR (edges_boxarea, 0.875,
"Min area fraction of grandchild for box");

/**********************************************************************
 * OL_BUCKETS::OL_BUCKETS
 *
 * Construct an array of buckets for associating outlines into blobs.
 **********************************************************************/

OL_BUCKETS::OL_BUCKETS (
////constructor
ICOORD bleft,                    //corners
ICOORD tright):         bl (bleft), tr (tright) {

  bxdim = (tright.x () - bleft.x ()) / BUCKETSIZE + 1;
  bydim = (tright.y () - bleft.y ()) / BUCKETSIZE + 1;
                                 //make array
  buckets = new C_OUTLINE_LIST[bxdim * bydim];
  index = 0;
}


/**********************************************************************
 * OL_BUCKETS::operator(
 *
 * Return a pointer to a list of C_OUTLINEs corresponding to the
 * given pixel coordinates.
 **********************************************************************/

C_OUTLINE_LIST *
OL_BUCKETS::operator () (        //array access
inT16 x,                         //image coords
inT16 y) {
  return &buckets[(y - bl.y ()) / BUCKETSIZE * bxdim +
    (x - bl.x ()) / BUCKETSIZE];
}


/**********************************************************************
 * OL_BUCKETS::count_children
 *
 * Find number of descendants of this outline.
 **********************************************************************/

inT32 OL_BUCKETS::count_children(                     //recursive count
                                 C_OUTLINE *outline,  //parent outline
                                 inT32 max_count      //max output
                                ) {
  BOOL8 parent_box;              //could it be boxy
  inT16 xmin, xmax;              //coord limits
  inT16 ymin, ymax;
  inT16 xindex, yindex;          //current bucket
  C_OUTLINE *child;              //current child
  inT32 child_count;             //no of children
  inT32 grandchild_count;        //no of grandchildren
  inT32 parent_area;             //potential box
  FLOAT32 max_parent_area;       //potential box
  inT32 child_area;              //current child
  inT32 child_length;            //current child
  TBOX olbox;
  C_OUTLINE_IT child_it;         //search iterator

  olbox = outline->bounding_box ();
  xmin = (olbox.left () - bl.x ()) / BUCKETSIZE;
  xmax = (olbox.right () - bl.x ()) / BUCKETSIZE;
  ymin = (olbox.bottom () - bl.y ()) / BUCKETSIZE;
  ymax = (olbox.top () - bl.y ()) / BUCKETSIZE;
  child_count = 0;
  grandchild_count = 0;
  parent_area = 0;
  max_parent_area = 0;
  parent_box = TRUE;
  for (yindex = ymin; yindex <= ymax; yindex++) {
    for (xindex = xmin; xindex <= xmax; xindex++) {
      child_it.set_to_list (&buckets[yindex * bxdim + xindex]);
      if (child_it.empty ())
        continue;
      for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
      child_it.forward ()) {
        child = child_it.data ();
        if (child != outline && *child < *outline) {
          child_count++;
          if (child_count <= max_count) {
            int max_grand = (max_count - child_count) /
                            edges_children_per_grandchild;
            if (max_grand > 0)
              grandchild_count += count_children (child, max_grand) *
                                  edges_children_per_grandchild;
            else
              grandchild_count += count_children(child, 1);
          }
          if (child_count + grandchild_count > max_count) {
            /*						err.log(RESULT_OKAY,E_LOC,ERR_OCR,
                            ERR_SCROLLING,ERR_CONTINUE,ERR_DEBUG,
                            "Discarding parent with child count=%d, gc=%d",
                            child_count,grandchild_count);*/
            return child_count + grandchild_count;
          }
          if (parent_area == 0) {
            parent_area = outline->outer_area ();
            if (parent_area < 0)
              parent_area = -parent_area;
            max_parent_area = outline->bounding_box ().width ()
              * outline->bounding_box ().height () * edges_boxarea;
            if (parent_area < max_parent_area)
              parent_box = FALSE;
          }
          if (parent_box
            && (!edges_children_fix
            || child->bounding_box ().height () >
          edges_min_nonhole) /**/) {
            child_area = child->outer_area ();
            if (child_area < 0)
              child_area = -child_area;
            if (edges_children_fix) {
              if (parent_area - child_area < max_parent_area) {
                parent_box = FALSE;
                continue;
              }
              if (grandchild_count > 0) {
                /*								err.log(RESULT_OKAY,E_LOC,ERR_OCR,
                                    ERR_SCROLLING,ERR_CONTINUE,ERR_DEBUG,
                                    "Discarding parent of area %d, child area=%d, max%g with gc=%d",
                                    parent_area,child_area,max_parent_area,grandchild_count);*/
                return max_count + 1;
              }
              child_length = child->pathlength ();
              if (child_length * child_length >
              child_area * edges_patharea_ratio) {
                /*/								err.log(RESULT_OKAY,E_LOC,ERR_OCR,
                                    ERR_SCROLLING,ERR_CONTINUE,ERR_DEBUG,
                                    "Discarding parent of area %d, child area=%d, max%g with child length=%d",
                                    parent_area,child_area,max_parent_area,child_length);*/
                return max_count + 1;
              }
            }
            if (child_area < child->bounding_box ().width ()
              * child->bounding_box ().height () *
            edges_childarea) {
              /*							err.log(RESULT_OKAY,E_LOC,ERR_OCR,
                                ERR_SCROLLING,ERR_CONTINUE,ERR_DEBUG,
                                "Discarding parent of area %d, child area=%d, max%g with child rect=%d",
                                parent_area,child_area,max_parent_area,child->bounding_box().width()
                                *child->bounding_box().height()); */
              return max_count + 1;
            }
          }
        }
      }
    }
  }
  return child_count + grandchild_count;
}


/**********************************************************************
 * OL_BUCKETS::extract_children
 *
 * Find number of descendants of this outline.
 **********************************************************************/

void OL_BUCKETS::extract_children(                     //recursive count
                                  C_OUTLINE *outline,  //parent outline
                                  C_OUTLINE_IT *it     //destination iterator
                                 ) {
  inT16 xmin, xmax;              //coord limits
  inT16 ymin, ymax;
  inT16 xindex, yindex;          //current bucket
  TBOX olbox;
  C_OUTLINE_IT child_it;         //search iterator

  olbox = outline->bounding_box ();
  xmin = (olbox.left () - bl.x ()) / BUCKETSIZE;
  xmax = (olbox.right () - bl.x ()) / BUCKETSIZE;
  ymin = (olbox.bottom () - bl.y ()) / BUCKETSIZE;
  ymax = (olbox.top () - bl.y ()) / BUCKETSIZE;
  for (yindex = ymin; yindex <= ymax; yindex++) {
    for (xindex = xmin; xindex <= xmax; xindex++) {
      child_it.set_to_list (&buckets[yindex * bxdim + xindex]);
      for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
      child_it.forward ()) {
        if (*child_it.data () < *outline) {
          it->add_after_then_move (child_it.extract ());
        }
      }
    }
  }
}


/**********************************************************************
 * extract_edges
 *
 * Run the edge detector over the block and return a list of blobs.
 **********************************************************************/

void extract_edges(                 //find blobs
#ifndef GRAPHICS_DISABLED
                   ScrollView* window,   //window for output
#endif
                   IMAGE *image,    //image to scan
                   IMAGE *t_image,  //thresholded image
                   ICOORD page_tr,  //corner of page
                   BLOCK *block     //block to scan
                  ) {
  ICOORD bleft;                  //block box
  ICOORD tright;
  C_OUTLINE_LIST outlines;       //outlines in block
                                 //iterator
  C_OUTLINE_IT out_it = &outlines;

#ifndef GRAPHICS_DISABLED
  get_outlines (window, image, t_image, page_tr, (PDBLK *) block, &out_it);
#else
  get_outlines (image, t_image, page_tr, (PDBLK *) block, &out_it);
#endif
                                 //block box
  block->bounding_box (bleft, tright);
                                 //make blobs
  outlines_to_blobs(block, bleft, tright, &outlines);

}


/**********************************************************************
 * outlines_to_blobs
 *
 * Gather together outlines into blobs using the usual bucket sort.
 **********************************************************************/

void outlines_to_blobs(               //find blobs
                       BLOCK *block,  //block to scan
                       ICOORD bleft,  //block box //outlines in block
                       ICOORD tright,
                       C_OUTLINE_LIST *outlines) {
                                 //make buckets
  OL_BUCKETS buckets(bleft, tright);

  fill_buckets(outlines, &buckets);
  empty_buckets(block, &buckets);
}


/**********************************************************************
 * fill_buckets
 *
 * Run the edge detector over the block and return a list of blobs.
 **********************************************************************/

void fill_buckets(                           //find blobs
                  C_OUTLINE_LIST *outlines,  //outlines in block
                  OL_BUCKETS *buckets        //output buckets
                 ) {
  TBOX ol_box;                    //outline box
  C_OUTLINE_IT out_it = outlines;//iterator
  C_OUTLINE_IT bucket_it;        //iterator in bucket
  C_OUTLINE *outline;            //current outline

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.extract (); //take off list
                                 //get box
    ol_box = outline->bounding_box ();
    bucket_it.set_to_list ((*buckets) (ol_box.left (), ol_box.bottom ()));
    bucket_it.add_to_end (outline);
  }
}


/**********************************************************************
 * empty_buckets
 *
 * Run the edge detector over the block and return a list of blobs.
 **********************************************************************/

void empty_buckets(                     //find blobs
                   BLOCK *block,        //block to scan
                   OL_BUCKETS *buckets  //output buckets
                  ) {
  BOOL8 good_blob;               //healthy blob
  C_OUTLINE_LIST outlines;       //outlines in block
                                 //iterator
  C_OUTLINE_IT out_it = &outlines;
  C_OUTLINE_IT bucket_it = buckets->start_scan ();
  C_OUTLINE_IT parent_it;        //parent outline
  C_BLOB *blob;                  //new blob
  C_BLOB_IT good_blobs = block->blob_list ();
  C_BLOB_IT junk_blobs = block->reject_blobs ();

  while (!bucket_it.empty ()) {
    out_it.set_to_list (&outlines);
    do {
      parent_it = bucket_it;     //find outermost
      do
      bucket_it.forward ();
      while (!bucket_it.at_first ()
        && !(*parent_it.data () < *bucket_it.data ()));
    }
    while (!bucket_it.at_first ());

                                 //move to new list
    out_it.add_after_then_move (parent_it.extract ());
    good_blob = capture_children (buckets, &junk_blobs, &out_it);
    blob = new C_BLOB (&outlines);
    if (good_blob)
      good_blobs.add_after_then_move (blob);
    else
      junk_blobs.add_after_then_move (blob);

    bucket_it.set_to_list (buckets->scan_next ());
  }
}


/**********************************************************************
 * capture_children
 *
 * Find all neighbouring outlines that are children of this outline
 * and either move them to the output list or declare this outline
 * illegal and return FALSE.
 **********************************************************************/

BOOL8 capture_children(                       //find children
                       OL_BUCKETS *buckets,   //bucket sort clanss
                       C_BLOB_IT *reject_it,  //dead grandchildren
                       C_OUTLINE_IT *blob_it  //output outlines
                      ) {
  C_OUTLINE *outline;            //master outline
  inT32 child_count;             //no of children

  outline = blob_it->data ();
  child_count = buckets->count_children (outline, edges_children_count_limit);
  if (child_count > edges_children_count_limit)
    return FALSE;
  if (child_count == 0)
    return TRUE;
                                 //get single level
  buckets->extract_children (outline, blob_it);
#if 0
  C_BLOB *blob;                  //reject
  C_OUTLINE *child;              //child under test
  C_OUTLINE_LIST r_list;         //rejects
  C_OUTLINE_IT r_it;             //iterator
  C_OUTLINE_IT test_it;          //for grandchildren
  BOOL8 anydone;                 //anything canned
  if (child_count == 1)
    return TRUE;
  do {
    anydone = FALSE;
    blob_it->move_to_first ();
    for (blob_it->mark_cycle_pt (); !blob_it->cycled_list ();
    blob_it->forward ()) {
      child = blob_it->data ();
      if (child != outline) {
        for (test_it = *blob_it, test_it.mark_cycle_pt ();
        !test_it.cycled_list (); test_it.forward ()) {
          if (test_it.data () != child && *test_it.data () < *child) {
            r_it.set_to_list (&r_list);
            r_it.add_after_then_move (test_it.extract ());
                                 //turn to blob
            blob = new C_BLOB (&r_list);
            reject_it->add_after_then_move (blob);
            anydone = TRUE;
          }
        }
        if (anydone)
          break;                 //got to reatart
      }
    }
  }
  while (anydone);               //got to restart
#endif
  return TRUE;
}
