/**********************************************************************
 * File:        blread.cpp  (Formerly pdread.c)
 * Description: Friend function of BLOCK to read the uscan pd file.
 * Author:		Ray Smith
 * Created:		Mon Mar 18 14:39:00 GMT 1991
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
#include          <stdlib.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "scanutils.h"
#include          "fileerr.h"
#include          "imgtiff.h"
#include          "pdclass.h"
#include          "rwpoly.h"
#include          "blread.h"

#define PD_EXT        ".pd"
#define VEC_EXT       ".vec"     //accupage file
#define HPD_EXT       ".bl"      //hand pd file
                                 //unlv zone file
#define UNLV_EXT            ".uzn"
#define BLOCK_EXPANSION   8      //boundary expansion
#define EXTERN

EXTERN BOOL_EVAR (ignore_weird_blocks, TRUE, "Don't read weird blocks");

static TBOX convert_vec_block(                        //make non-rect block
                             VEC_ENTRY *entries,     //vectors
                             uinT16 entry_count,     //no of entries
                             inT32 ysize,            //image size
                             ICOORDELT_IT *left_it,  //block sides
                             ICOORDELT_IT *right_it);

/**********************************************************************
 * BLOCK::read_pd_file
 *
 * Read a whole pd file to make a list of blocks, or return false.
 **********************************************************************/

BOOL8 read_pd_file(                    //print list of sides
                   STRING name,        //basename of file
                   inT32 xsize,        //image size
                   inT32 ysize,        //image size
                   BLOCK_LIST *blocks  //output list
                  ) {
  FILE *pdfp;                    //file pointer
  BLOCK *block;                  //current block
  inT32 block_count;             //no of blocks
  inT32 junk_count;              //no of junks to read
  inT32 junks[4];                //junk elements
  inT32 vertex_count;            //boundary vertices
  inT32 xcoord;                  //current coords
  inT32 ycoord;
  inT32 prevx;                   //previous coords
  inT32 prevy;
  BLOCK_IT block_it = blocks;    //block iterator
  ICOORDELT_LIST dummy;          //for constructor
  ICOORDELT_IT left_it = &dummy; //iterator
  ICOORDELT_IT right_it = &dummy;//iterator

  if (read_hpd_file (name, xsize, ysize, blocks))
    return TRUE;                 //succeeded
  if (read_vec_file (name, xsize, ysize, blocks))
    return TRUE;                 //succeeded
  if (read_unlv_file (name, xsize, ysize, blocks))
    return TRUE;                 //succeeded
  name += PD_EXT;                //add extension
  if ((pdfp = fopen (name.string (), "r")) == NULL) {
                                 //make rect block
    return FALSE;                //didn't read one
  }
  else {
    if (fread (&block_count, sizeof (block_count), 1, pdfp) != 1)
      READFAILED.error ("read_pd_file", EXIT, "Block count");
    tprintf ("%d blocks in .pd file.\n", block_count);
    while (block_count > 0) {
      if (fread (&junk_count, sizeof (junk_count), 1, pdfp) != 1)
        READFAILED.error ("read_pd_file", EXIT, "Junk count");
      if (fread (&vertex_count, sizeof (vertex_count), 1, pdfp) != 1)
        READFAILED.error ("read_pd_file", EXIT, "Vertex count");
      block = new BLOCK;         //make a block
                                 //on end of list
      block_it.add_to_end (block);
      left_it.set_to_list (&block->leftside);
      right_it.set_to_list (&block->rightside);

                                 //read a pair
      get_pd_vertex (pdfp, xsize, ysize, &block->box, xcoord, ycoord);
      vertex_count -= 2;         //count read ones
      prevx = xcoord;
      do {
        if (xcoord == prevx) {
          if (!right_it.empty ()) {
            if (right_it.data ()->x () <= xcoord + BLOCK_EXPANSION)
              right_it.data ()->set_y (right_it.data ()->y () +
                BLOCK_EXPANSION);
            else
              right_it.data ()->set_y (right_it.data ()->y () -
                BLOCK_EXPANSION);
          }
          right_it.
            add_before_then_move (new
            ICOORDELT (xcoord + BLOCK_EXPANSION,
            ycoord));
        }
        prevx = xcoord;          //remember previous
        prevy = ycoord;
        get_pd_vertex (pdfp, xsize, ysize, &block->box, xcoord, ycoord);
        vertex_count -= 2;       //count read ones
      }
      while (ycoord <= prevy);
      right_it.data ()->set_y (right_it.data ()->y () - BLOCK_EXPANSION);

                                 //start of left
      left_it.add_to_end (new ICOORDELT (prevx - BLOCK_EXPANSION, prevy - BLOCK_EXPANSION));

      do {
        prevx = xcoord;          //remember previous
        get_pd_vertex (pdfp, xsize, ysize, &block->box, xcoord, ycoord);
        vertex_count -= 2;
        if (xcoord != prevx && vertex_count > 0) {
          if (xcoord > prevx)
            left_it.
              add_to_end (new
              ICOORDELT (xcoord - BLOCK_EXPANSION,
              ycoord + BLOCK_EXPANSION));
          else
            left_it.
              add_to_end (new
              ICOORDELT (xcoord - BLOCK_EXPANSION,
              ycoord - BLOCK_EXPANSION));
        }
        else if (vertex_count == 0)
          left_it.add_to_end (new ICOORDELT (prevx - BLOCK_EXPANSION,
              ycoord + BLOCK_EXPANSION));
      }
      while (vertex_count > 0);  //until all read

      while (junk_count > 0) {
        if (fread (junks, sizeof (inT32), 4, pdfp) != 4)
          READFAILED.error ("read_pd_file", EXIT, "Junk coords");
        junk_count--;
      }
      block_count--;             //count read blocks
    }
  }
  fclose(pdfp);
  return TRUE;                   //read one
}


/**********************************************************************
 * get_pd_vertex
 *
 * Read a pair of coords, invert the y and clip to image limits.
 * Also update the bounding box.
 *
 * Read a whole pd file to make a list of blocks, or use the whole page.
 **********************************************************************/

void get_pd_vertex(                //get new vertex
                   FILE *pdfp,     //file to read
                   inT32 xsize,    //image size
                   inT32 ysize,    //image size
                   TBOX *box,       //bounding box
                   inT32 &xcoord,  //output coords
                   inT32 &ycoord) {
  TBOX new_coord;                 //expansion box

                                 //get new coords
  if (fread (&xcoord, sizeof (xcoord), 1, pdfp) != 1)
    READFAILED.error ("read_pd_file", EXIT, "Xcoord");
  if (fread (&ycoord, sizeof (ycoord), 1, pdfp) != 1)
    READFAILED.error ("read_pd_file", EXIT, "Xcoord");
  ycoord = ysize - ycoord;       //invert y
  if (xcoord < BLOCK_EXPANSION)
    xcoord = BLOCK_EXPANSION;    //clip to limits
  if (xcoord > xsize - BLOCK_EXPANSION)
    xcoord = xsize - BLOCK_EXPANSION;
  if (ycoord < BLOCK_EXPANSION)
    ycoord = BLOCK_EXPANSION;
  if (ycoord > ysize - BLOCK_EXPANSION)
    ycoord = ysize - BLOCK_EXPANSION;

  new_coord =
    TBOX (ICOORD (xcoord - BLOCK_EXPANSION, ycoord - BLOCK_EXPANSION),
    ICOORD (xcoord + BLOCK_EXPANSION, ycoord + BLOCK_EXPANSION));
  (*box) += new_coord;
}


/**********************************************************************
 * BLOCK::read_hpd_file
 *
 * Read a whole hpd file to make a list of blocks.
 * Return FALSE if the .vec fiel cannot be found
 **********************************************************************/

BOOL8 read_hpd_file(                    //print list of sides
                    STRING name,        //basename of file
                    inT32 xsize,        //image size
                    inT32 ysize,        //image size
                    BLOCK_LIST *blocks  //output list
                   ) {
  FILE *pdfp;                    //file pointer
  PAGE_BLOCK_LIST *page_blocks;
  inT32 block_no;                //no of blocks
  BLOCK_IT block_it = blocks;    //block iterator

  name += HPD_EXT;               //add extension
  if ((pdfp = fopen (name.string (), "r")) == NULL) {
    return FALSE;                //can't find it
  }
  fclose(pdfp);
  page_blocks = read_poly_blocks (name.string ());
  block_no = 0;
  scan_hpd_blocks (name.string (), page_blocks, block_no, &block_it);
  tprintf ("Text region count=%d\n", block_no);
  return TRUE;                   //read one
}


/**********************************************************************
 * BLOCK::scan_hpd_blocks
 *
 * Read a whole hpd file to make a list of blocks.
 * Return FALSE if the .vec fiel cannot be found
 **********************************************************************/

void scan_hpd_blocks(                               //print list of sides
                     const char *name,              //block label
                     PAGE_BLOCK_LIST *page_blocks,  //head of full pag
                     inT32 &block_no,               //no of blocks
                     BLOCK_IT *block_it             //block iterator
                    ) {
  BLOCK *block;                  //current block
                                 //page blocks
  PAGE_BLOCK_IT pb_it = page_blocks;
  PAGE_BLOCK *current_block;
  TEXT_REGION_IT tr_it;
  TEXT_BLOCK *tb;
  TEXT_REGION *tr;
  TBOX *block_box;                //from text region

  for (pb_it.mark_cycle_pt (); !pb_it.cycled_list (); pb_it.forward ()) {
    current_block = pb_it.data ();
    if (current_block->type () == PB_TEXT) {
      tb = (TEXT_BLOCK *) current_block;
      if (!tb->regions ()->empty ()) {
        tr_it.set_to_list (tb->regions ());
        for (tr_it.mark_cycle_pt ();
        !tr_it.cycled_list (); tr_it.forward ()) {
          block_no++;
          tr = tr_it.data ();
          block_box = tr->bounding_box ();
          block = new BLOCK (name, TRUE, 0, 0,
            block_box->left (), block_box->bottom (),
            block_box->right (), block_box->top ());
          block->hand_block = tr;
          block->hand_poly = tr;
          block_it->add_after_then_move (block);
        }
      }
    }
    else if (current_block->type () == PB_WEIRD
      && !ignore_weird_blocks
    && ((WEIRD_BLOCK *) current_block)->id_no () > 0) {
      block_no++;
      block_box = current_block->bounding_box ();
      block = new BLOCK (name, TRUE, 0, 0,
        block_box->left (), block_box->bottom (),
        block_box->right (), block_box->top ());
      block->hand_block = NULL;
      block->hand_poly = current_block;
      block_it->add_after_then_move (block);
    }
    if (!current_block->child ()->empty ())
      scan_hpd_blocks (name, current_block->child (), block_no, block_it);
  }
}





/**********************************************************************
 * BLOCK::read_vec_file
 *
 * Read a whole vec file to make a list of blocks.
 * Return FALSE if the .vec fiel cannot be found
 **********************************************************************/

BOOL8 read_vec_file(                    //print list of sides
                    STRING name,        //basename of file
                    inT32 xsize,        //image size
                    inT32 ysize,        //image size
                    BLOCK_LIST *blocks  //output list
                   ) {
  FILE *pdfp;                    //file pointer
  BLOCK *block;                  //current block
  inT32 block_no;                //no of blocks
  inT32 block_index;             //current blocks
  inT32 vector_count;            //total vectors
  VEC_HEADER header;             //file header
  BLOCK_HEADER *vec_blocks;      //blocks from file
  VEC_ENTRY *vec_entries;        //vectors from file
  BLOCK_IT block_it = blocks;    //block iterator
  ICOORDELT_IT left_it;          //iterators
  ICOORDELT_IT right_it;

  name += VEC_EXT;               //add extension
  if ((pdfp = fopen (name.string (), "r")) == NULL) {
    return FALSE;                //can't find it
  }
  if (fread (&header, sizeof (header), 1, pdfp) != 1)
    READFAILED.error ("read_vec_file", EXIT, "Header");
                                 //from intel
  header.filesize = reverse32 (header.filesize);
  header.bytesize = reverse16 (header.bytesize);
  header.arraysize = reverse16 (header.arraysize);
  header.width = reverse16 (header.width);
  header.height = reverse16 (header.height);
  header.res = reverse16 (header.res);
  header.bpp = reverse16 (header.bpp);
  tprintf ("%d blocks in %s file:", header.arraysize, VEC_EXT);
  vector_count = header.filesize - header.arraysize * sizeof (BLOCK_HEADER);
  vector_count /= sizeof (VEC_ENTRY);
  vec_blocks =
    (BLOCK_HEADER *) alloc_mem (header.arraysize * sizeof (BLOCK_HEADER));
  vec_entries = (VEC_ENTRY *) alloc_mem (vector_count * sizeof (VEC_ENTRY));
  xsize = header.width;          //real image size
  ysize = header.height;
  if (fread (vec_blocks, sizeof (BLOCK_HEADER), header.arraysize, pdfp)
    != static_cast<size_t>(header.arraysize))
    READFAILED.error ("read_vec_file", EXIT, "Blocks");
  if (fread (vec_entries, sizeof (VEC_ENTRY), vector_count, pdfp)
    != static_cast<size_t>(vector_count))
    READFAILED.error ("read_vec_file", EXIT, "Vectors");
  for (block_index = 0; block_index < header.arraysize; block_index++) {
    vec_blocks[block_index].offset =
      reverse16 (vec_blocks[block_index].offset);
    vec_blocks[block_index].order =
      reverse16 (vec_blocks[block_index].order);
    vec_blocks[block_index].entries =
      reverse16 (vec_blocks[block_index].entries);
    vec_blocks[block_index].charsize =
      reverse16 (vec_blocks[block_index].charsize);
  }
  for (block_index = 0; block_index < vector_count; block_index++) {
    vec_entries[block_index].start =
      ICOORD (reverse16 (vec_entries[block_index].start.x ()),
      reverse16 (vec_entries[block_index].start.y ()));
    vec_entries[block_index].end =
      ICOORD (reverse16 (vec_entries[block_index].end.x ()),
      reverse16 (vec_entries[block_index].end.y ()));
  }
  for (block_no = 1; block_no <= header.arraysize; block_no++) {
    for (block_index = 0; block_index < header.arraysize; block_index++) {
      if (vec_blocks[block_index].order == block_no
      && vec_blocks[block_index].valid) {
        block = new BLOCK;
        left_it.set_to_list (&block->leftside);
        right_it.set_to_list (&block->rightside);
        block->box =
          convert_vec_block (&vec_entries
          [vec_blocks[block_index].offset],
          vec_blocks[block_index].entries, ysize,
          &left_it, &right_it);
        block->set_xheight (vec_blocks[block_index].charsize);
                                 //on end of list
        block_it.add_to_end (block);
        //                              tprintf("Block at (%d,%d)->(%d,%d) has index %d and order %d\n",
        //                                      block->box.left(),
        //                                      block->box.bottom(),
        //                                      block->box.right(),
        //                                      block->box.top(),
        //                                      block_index,vec_blocks[block_index].order);
      }
    }
  }
  free_mem(vec_blocks);
  free_mem(vec_entries);
  tprintf ("%d valid\n", block_it.length ());
  fclose(pdfp);
  return TRUE;                   //read one
}


/**********************************************************************
 * BLOCK::convert_vec_block
 *
 * Read a whole vec file to make a list of blocks.
 * Return FALSE if the .vec fiel cannot be found
 **********************************************************************/

static TBOX convert_vec_block(                        //make non-rect block
                             VEC_ENTRY *entries,     //vectors
                             uinT16 entry_count,     //no of entries
                             inT32 ysize,            //image size
                             ICOORDELT_IT *left_it,  //block sides
                             ICOORDELT_IT *right_it) {
  TBOX block_box;                 //bounding box
  TBOX vec_box;                   //box of vec
  ICOORD box_point;              //expanded coord
  ICOORD shift_vec;              //for box expansion
  ICOORD prev_pt;                //previous coord
  ICOORD end_pt;                 //end of vector
  inT32 vertex_index;            //boundary vertices

  for (vertex_index = 0; vertex_index < entry_count; vertex_index++) {
    entries[vertex_index].start = ICOORD (entries[vertex_index].start.x (),
      ysize - 1 -
      entries[vertex_index].start.y ());
    entries[vertex_index].end =
      ICOORD (entries[vertex_index].end.x (),
      ysize - 1 - entries[vertex_index].end.y ());
    vec_box = TBOX (entries[vertex_index].start, entries[vertex_index].end);
    block_box += vec_box;        //find total bounds
  }

  for (vertex_index = 0; vertex_index < entry_count
    && (entries[vertex_index].start.y () != block_box.bottom ()
    || entries[vertex_index].end.y () != block_box.bottom ());
    vertex_index++);
  ASSERT_HOST (vertex_index < entry_count);
  prev_pt = entries[vertex_index].start;
  end_pt = entries[vertex_index].end;
  do {
    for (vertex_index = 0; vertex_index < entry_count
      && entries[vertex_index].start != end_pt; vertex_index++);
                                 //found start of vertical
    ASSERT_HOST (vertex_index < entry_count);
    box_point = entries[vertex_index].start;
    if (box_point.x () <= prev_pt.x ())
      shift_vec = ICOORD (-BLOCK_EXPANSION, -BLOCK_EXPANSION);
    else
      shift_vec = ICOORD (-BLOCK_EXPANSION, BLOCK_EXPANSION);
    left_it->add_to_end (new ICOORDELT (box_point + shift_vec));
    prev_pt = box_point;
    for (vertex_index = 0; vertex_index < entry_count
      && entries[vertex_index].start != end_pt; vertex_index++);
                                 //found horizontal
    ASSERT_HOST (vertex_index < entry_count);
    end_pt = entries[vertex_index].end;
  }
  while (end_pt.y () < block_box.top ());
  shift_vec = ICOORD (-BLOCK_EXPANSION, BLOCK_EXPANSION);
  left_it->add_to_end (new ICOORDELT (end_pt + shift_vec));

  for (vertex_index = 0; vertex_index < entry_count
    && (entries[vertex_index].start.y () != block_box.top ()
    || entries[vertex_index].end.y () != block_box.top ());
    vertex_index++);
  ASSERT_HOST (vertex_index < entry_count);
  prev_pt = entries[vertex_index].start;
  end_pt = entries[vertex_index].end;
  do {
    for (vertex_index = 0; vertex_index < entry_count
      && entries[vertex_index].start != end_pt; vertex_index++);
                                 //found start of vertical
    ASSERT_HOST (vertex_index < entry_count);
    box_point = entries[vertex_index].start;
    if (box_point.x () < prev_pt.x ())
      shift_vec = ICOORD (BLOCK_EXPANSION, -BLOCK_EXPANSION);
    else
      shift_vec = ICOORD (BLOCK_EXPANSION, BLOCK_EXPANSION);
    right_it->add_before_then_move (new ICOORDELT (box_point + shift_vec));
    prev_pt = box_point;
    for (vertex_index = 0; vertex_index < entry_count
      && entries[vertex_index].start != end_pt; vertex_index++);
                                 //found horizontal
    ASSERT_HOST (vertex_index < entry_count);
    end_pt = entries[vertex_index].end;
  }
  while (end_pt.y () > block_box.bottom ());
  shift_vec = ICOORD (BLOCK_EXPANSION, -BLOCK_EXPANSION);
  right_it->add_before_then_move (new ICOORDELT (end_pt + shift_vec));

  shift_vec = ICOORD (BLOCK_EXPANSION, BLOCK_EXPANSION);
  box_point = block_box.botleft () - shift_vec;
  end_pt = block_box.topright () + shift_vec;
  return TBOX (box_point, end_pt);
}


/**********************************************************************
 * read_unlv_file
 *
 * Read a whole unlv zone file to make a list of blocks.
 **********************************************************************/

BOOL8 read_unlv_file(                    //print list of sides
                     STRING name,        //basename of file
                     inT32 xsize,        //image size
                     inT32 ysize,        //image size
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
  if ((pdfp = fopen (name.string (), "r")) == NULL) {
    return FALSE;                //didn't read one
  }
  else {
    while (fscanf (pdfp, "%d %d %d %d %*s", &x, &y, &width, &height) >= 4) {
                                 //make rect block
      block = new BLOCK (name.string (), TRUE, 0, 0,
                         (inT16) x, (inT16) (ysize - y - height),
                         (inT16) (x + width), (inT16) (ysize - y));
                                 //on end of list
      block_it.add_to_end (block);
    }
    fclose(pdfp);
  }
  return true;
}
