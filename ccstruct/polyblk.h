/**********************************************************************
 * File:        polyblk.h  (Formerly poly_block.h)
 * Description: Polygonal blocks
 * Author:					Sheelagh Lloyd?
 * Created:
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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
#ifndef           POLYBLK_H
#define           POLYBLK_H

#include          "rect.h"
#include          "points.h"
#include          "scrollview.h"
#include          "elst.h"

#include          "hpddef.h"     //must be last (handpd.dll)

enum POLY_TYPE
{
  POLY_TEXT,                     // Text region
  POLY_PAGE,                     // Page block
  POLY_X                         // Don't care
};

class DLLSYM POLY_BLOCK          //poly block
{

  public:
    POLY_BLOCK() {
    }                            //empty constructor
    POLY_BLOCK(  //simple constructor
               ICOORDELT_LIST *points,
               POLY_TYPE t);
    ~POLY_BLOCK () {
    }                            //destructor

    TBOX *bounding_box() {  // access function
      return &box;
    }

    ICOORDELT_LIST *points() {  // access function
      return &vertices;
    }

    void compute_bb();

    POLY_TYPE isA() {
      return type;
    }

    void rotate(  //rotate it
                FCOORD rotation);
    void move(                //move it
              ICOORD shift);  //vector

    void plot(ScrollView* window, ScrollView::Color colour, inT32 num);

    void fill(ScrollView* window, ScrollView::Color colour);

    BOOL8 contains(  // is poly inside poly
                   POLY_BLOCK *poly);

    BOOL8 overlap(  // do polys overlap
                  POLY_BLOCK *poly);

    inT16 winding_number(                         // get winding number
                         const ICOORD &test_pt);  // around this point

    void prep_serialise() {
      vertices.prep_serialise ();
    }

    void dump(FILE *f) {
      vertices.dump (f);
    }

    void de_dump(FILE *f) {
      vertices.de_dump (f);
    }

                                 //convert to ascii
    make_serialise (POLY_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //convert from ascii
                          FILE *f);

  private:
    ICOORDELT_LIST vertices;     // vertices
    TBOX box;                     // bounding box
    POLY_TYPE type;              // Page block or
    // text region
};

class DLLSYM PB_LINE_IT          //line iterator
{
  public:
    PB_LINE_IT(  //constructor
               POLY_BLOCK *blkptr) {
      block = blkptr;
    }

    NEWDELETE2 (PB_LINE_IT) void set_to_block (POLY_BLOCK * blkptr) {
      block = blkptr;
    }

    ICOORDELT_LIST *get_line(inT16 y);

  private:
    POLY_BLOCK * block;
};
#endif
