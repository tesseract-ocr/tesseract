/**********************************************************************
 * File:        ocrblock.h  (Formerly block.h)
 * Description: Page block class definition.
 * Author:		Ray Smith
 * Created:		Thu Mar 14 17:32:01 GMT 1991
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

#ifndef           OCRBLOCK_H
#define           OCRBLOCK_H

#include          "img.h"
#include          "ocrrow.h"
#include          "pageblk.h"
#include          "pdblock.h"

class BLOCK;                     //forward decl

ELISTIZEH_S (BLOCK)
class BLOCK:public ELIST_LINK, public PDBLK
//page block
{
  friend class BLOCK_RECT_IT;    //block iterator

                                 //block label
  friend void scan_hpd_blocks(const char *name,
                              PAGE_BLOCK_LIST *page_blocks,  //head of full pag
                              inT32 &block_no,               //no of blocks
                              BLOCK_IT *block_it);
  friend BOOL8 read_vec_file(              //read uscan output
                             STRING name,  //basename of file
                             inT32 xsize,  //page size //output list
                             inT32 ysize,
                             BLOCK_LIST *blocks);
  friend BOOL8 read_pd_file(              //read uscan output
                            STRING name,  //basename of file
                            inT32 xsize,  //page size //output list
                            inT32 ysize,
                            BLOCK_LIST *blocks);

  public:
    BLOCK() {  //empty constructor
      hand_block = NULL;
      hand_poly = NULL;
    }
    BLOCK(                   //simple constructor
          const char *name,  //filename
          BOOL8 prop,        //proportional
          inT16 kern,        //kerning
          inT16 space,       //spacing
          inT16 xmin,        //bottom left
          inT16 ymin,
          inT16 xmax,        //top right
          inT16 ymax);

    //      void                                            set_sides(                                                 //set vertex lists
    //              ICOORDELT_LIST       *left,                        //list of left vertices
    //              ICOORDELT_LIST       *right);                      //list of right vertices

    ~BLOCK () {                  //destructor
    }

    void set_stats(                   //set space size etc.
                   BOOL8 prop,        //proportional
                   inT16 kern,        //inter char size
                   inT16 space,       //inter word size
                   inT16 ch_pitch) {  //pitch if fixed
      proportional = prop;
      kerning = (inT8) kern;
      spacing = space;
      pitch = ch_pitch;
    }
    void set_xheight(  //set char size
                     inT32 height) {
      xheight = height;
    }
    void set_font_class(  //set font class
                        inT16 font) {
      font_class = font;
    }
    //      TEXT_REGION*                            text_region()
    //      {
    //              return hand_block;
    //      }
    //      POLY_BLOCK*                                     poly_block()
    //      {
    //              return hand_poly;
    //      }
    BOOL8 prop() const {  //return proportional
      return proportional;
    }
    inT32 fixed_pitch() const {  //return pitch
      return pitch;
    }
    inT16 kern() const {  //return kerning
      return kerning;
    }
    inT16 font() const {  //return font class
      return font_class;
    }
    inT16 space() const {  //return spacing
      return spacing;
    }
    const char *name() const {  //return filename
      return filename.string ();
    }
    inT32 x_height() const {  //return xheight
      return xheight;
    }
    float cell_over_xheight() const {
      return cell_over_xheight_;
    }
    void set_cell_over_xheight(float ratio) {
      cell_over_xheight_ = ratio;
    }
    ROW_LIST *row_list() {  //get rows
      return &rows;
    }
    C_BLOB_LIST *blob_list() {  //get blobs
      return &c_blobs;
    }
    C_BLOB_LIST *reject_blobs() {
      return &rej_blobs;
    }
    //      void                                                    bounding_box(                                           //get box
    //              ICOORD&                                 bottom_left,                                            //bottom left
    //              ICOORD&                                 top_right) const                                        //topright
    //      {
    //              bottom_left=box.botleft();
    //              top_right=box.topright();
    //      }
    //      const TBOX&                                      bounding_box() const                            //get real box
    //      {
    //              return box;
    //      }

    //      BOOL8                                                   contains(                                                       //is pt inside block
    //              ICOORD                                  pt);

    //      void                                                    move(                                                                   // reposition block
    //         const ICOORD                 vec);                                                                   // by vector

    void sort_rows();  //decreasing y order

    void compress();  //shrink white space

    void check_pitch();  //check proportional

    void compress(                    //shrink white space
                  const ICOORD vec);  //and move by vector

    void print(              //print summary/table
               FILE *fp,     //file to print on
               BOOL8 dump);  //dump whole table

    //      void                                                    plot(                                                                   //draw histogram
    //              WINDOW                                  window,                                                         //window to draw in
    //              inT32                                           serial,                                                         //serial number
    //              COLOUR                                  colour);                                                                //colour to draw in

    //      void                                                    show(                                                                   //show image
    //              IMAGE                                           *image,                                                         //image to show
    //              WINDOW                                  window);                                                                //window to show in

    void prep_serialise() {  //set ptrs to counts
      filename.prep_serialise ();
      rows.prep_serialise ();
      c_blobs.prep_serialise ();
      rej_blobs.prep_serialise ();
      leftside.prep_serialise ();
      rightside.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      filename.dump (f);
      rows.dump (f);
      c_blobs.dump (f);
      rej_blobs.dump (f);
      leftside.dump (f);
      rightside.dump (f);
      if (hand_block != NULL)
        hand_block->serialise (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      filename.de_dump (f);
      rows.de_dump (f);
      c_blobs.de_dump (f);
      rej_blobs.de_dump (f);
      leftside.de_dump (f);
      rightside.de_dump (f);
      if (hand_block != NULL)
        hand_block = TEXT_REGION::de_serialise (f);
    }

                                 //assignment
    make_serialise (BLOCK) BLOCK & operator= (
      const BLOCK & source);     //from this

  private:
    BOOL8 proportional;          //proportional
    inT8 kerning;                //inter blob gap
    inT16 spacing;               //inter word gap
    inT16 pitch;                 //pitch of non-props
    inT16 font_class;            //correct font class
    inT32 xheight;               //height of chars
    float cell_over_xheight_;    // Ratio of cell height to xheight.
    STRING filename;             //name of block
    //      TEXT_REGION*                            hand_block;                                                     //if it exists
    //      POLY_BLOCK*                                     hand_poly;                                                      //wierd as well
    ROW_LIST rows;               //rows in block
    C_BLOB_LIST c_blobs;         //before textord
    C_BLOB_LIST rej_blobs;       //duff stuff
    //      ICOORDELT_LIST                          leftside;                                                       //left side vertices
    //      ICOORDELT_LIST                          rightside;                                                      //right side vertices
    //      TBOX                                                     box;                                                                    //bounding box
};

int decreasing_top_order(  //
                         const void *row1,
                         const void *row2);
#endif
