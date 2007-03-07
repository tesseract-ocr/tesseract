/**********************************************************************
 * File:        txtregn.h  (Formerly text_region.h)
 * Description: Text region within a polygonal block
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

#ifndef           TXTREGN_H
#define           TXTREGN_H

#include          <stdio.h>
#include          "elst.h"
#include          "hpdsizes.h"
#include          "polyblk.h"
#include          "debugwin.h"

#include          "hpddef.h"     //must be last (handpd.dll)

#define REGION_COLOUR CYAN
#define SUBREGION_COLOUR GREEN

class DLLSYM TEXT_REGION;        //forward decl

ELISTIZEH_S (TEXT_REGION)
class DLLSYM TEXT_REGION:public ELIST_LINK, public POLY_BLOCK
//text REGION
{
  public:
    TEXT_REGION() { 
    }                            //empty constructor
    TEXT_REGION(  //simple constructor
                INT32 idno,
                ICOORDELT_LIST *points,
                TEXT_REGION_LIST *child);

    TEXT_REGION(  //simple constructor
                INT32 idno,
                ICOORDELT_LIST *points);

    TEXT_REGION(  //constructor
                INT32 idno,
                ICOORDELT_LIST *points,
                INT8 hor,
                INT8 tex,
                INT8 ser,
                INT8 pro,
                INT8 nor,
                INT8 upr,
                INT8 sol,
                INT8 bla,
                INT8 und,
                INT8 dro);

    ~TEXT_REGION () {            //destructor
    }

    void set_id_no(INT32 new_id) { 
      id_number = new_id;
    }

    INT32 id_no() { 
      return id_number;
    }

    INT32 nregions() { 
      return txt_regions.length ();
    }

    BOOL8 is_prop() const {  //test proportional
      return !proportional;      //stored negatively
    }

    void set_prop(BOOL8 prop) { 
      if (prop)
        proportional = 0;
      else
        proportional = 1;
    }

    void add_a_region(TEXT_REGION *newchild); 

                                 //get children
    TEXT_REGION_LIST *regions() { 
      return &txt_regions;
    }

    void set_attrs(INT8 hor,
                   INT8 tex,
                   INT8 ser,
                   INT8 pro,
                   INT8 nor,
                   INT8 upr,
                   INT8 sol,
                   INT8 bla,
                   INT8 und,
                   INT8 dro);

    void show_attrs(DEBUG_WIN *f); 

    void rotate(  //rotate it
                FCOORD rotation);
    void move(                //move it
              ICOORD shift);  //vector

    void prep_serialise() {  //set ptrs to counts
      POLY_BLOCK::prep_serialise(); 
      txt_regions.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      POLY_BLOCK::dump(f); 
      txt_regions.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      POLY_BLOCK::de_dump(f); 
      txt_regions.de_dump (f);
    }

                                 //serialise to ascii
    make_serialise (TEXT_REGION) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    INT32 id_number;             //unique id
    INT8 horizontal;             //horizontal, vertical, skewed
    INT8 text;                   //text, table, form
    INT8 serif;                  //serif, sansserif
    INT8 proportional;           //proportional, fixed
    INT8 normal;                 //normal, bold
    INT8 upright;                //upright, italic
    INT8 solid;                  //solid, outline
    INT8 black;                  //black, coloured, white,
    INT8 underlined;             //not underlined, underlined
    INT8 dropcaps;               //not dropcaps, dropcaps

    TEXT_REGION_LIST txt_regions;
};
#endif
