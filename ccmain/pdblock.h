/**********************************************************************
 * File:        pdblock.h  (Formerly pdblk.h)
 * Description: Page block class definition.
 * Author:					Ray Smith
 * Created:					Thu Mar 14 17:32:01 GMT 1991
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

#ifndef           PDBLOCK_H
#define           PDBLOCK_H

#include          "img.h"
#include          "strngs.h"
#include          "polyblk.h"

#include          "hpddef.h"     //must be last (handpd.dll)

class DLLSYM PDBLK;              //forward decl

CLISTIZEH (PDBLK)
class DLLSYM PDBLK               //page block
{
  friend class BLOCK_RECT_IT;    //block iterator

  public:
    PDBLK() {  //empty constructor
      hand_poly = NULL;
      index_ = 0;
    }
    PDBLK(             //simple constructor
          inT16 xmin,  //bottom left
          inT16 ymin,
          inT16 xmax,  //top right
          inT16 ymax);

    void set_sides(                         //set vertex lists
                   ICOORDELT_LIST *left,    //list of left vertices
                   ICOORDELT_LIST *right);  //list of right vertices

    ~PDBLK () {                  //destructor
      if (hand_poly) delete hand_poly;
    }

    POLY_BLOCK *poly_block() {
      return hand_poly;
    }
    void set_poly_block(  //set the poly block
                        POLY_BLOCK *blk) {
      hand_poly = blk;
    }
    void bounding_box(                            //get box
                      ICOORD &bottom_left,        //bottom left
                      ICOORD &top_right) const {  //topright
      bottom_left = box.botleft ();
      top_right = box.topright ();
    }
                                 //get real box
    const TBOX &bounding_box() const {
      return box;
    }

    int index() const {
      return index_;
    }
    void set_index(int value) {
      index_ = value;
    }

    BOOL8 contains(  //is pt inside block
                   ICOORD pt);

    void move(                    // reposition block
              const ICOORD vec);  // by vector

    void plot(                 //draw histogram
              ScrollView* window,   //window to draw in
              inT32 serial,    //serial number
              ScrollView::Color colour);  //colour to draw in

    void show(                 //show image
              IMAGE *image,    //image to show
              ScrollView* window);  //window to show in

    PDBLK & operator= (          //assignment
      const PDBLK & source);     //from this

  protected:
    POLY_BLOCK *hand_poly;       //wierd as well
    ICOORDELT_LIST leftside;     //left side vertices
    ICOORDELT_LIST rightside;    //right side vertices
    TBOX box;                     //bounding box
    int index_;                  // Serial number of this block.
};

class DLLSYM BLOCK_RECT_IT       //rectangle iterator
{
  public:
    BLOCK_RECT_IT(                 //constructor
                  PDBLK *blkptr);  //block to iterate

                                 //start (new) block
    NEWDELETE2 (BLOCK_RECT_IT) void set_to_block (
      PDBLK * blkptr);           //block to iterate

    void start_block();  //start iteration

    void forward();  //next rectangle

    BOOL8 cycled_rects() {  //test end
      return left_it.cycled_list () && right_it.cycled_list ();
    }

    void bounding_box(                   //current rectangle
                      ICOORD &bleft,     //bottom left
                      ICOORD &tright) {  //top right
                                 //bottom left
      bleft = ICOORD (left_it.data ()->x (), ymin);
                                 //top right
      tright = ICOORD (right_it.data ()->x (), ymax);
    }

  private:
    inT16 ymin;                  //bottom of rectangle
    inT16 ymax;                  //top of rectangle
    PDBLK *block;                //block to iterate
    ICOORDELT_IT left_it;        //boundary iterators
    ICOORDELT_IT right_it;
};

class DLLSYM BLOCK_LINE_IT       //rectangle iterator
{
  public:
    BLOCK_LINE_IT (              //constructor
      PDBLK * blkptr)            //from block
    :rect_it (blkptr) {
      block = blkptr;            //remember block
    }

                                 //start (new) block
    NEWDELETE2 (BLOCK_LINE_IT) void set_to_block (
    PDBLK * blkptr) {            //block to start
      block = blkptr;            //remember block
                                 //set iterator
      rect_it.set_to_block (blkptr);
    }

    inT16 get_line(               //get a line
                   inT16 y,       //line to get
                   inT16 &xext);  //output extent

  private:
    PDBLK * block;               //block to iterate
    BLOCK_RECT_IT rect_it;       //rectangle iterator
};

int decreasing_top_order(  //
                         const void *row1,
                         const void *row2);
#endif
