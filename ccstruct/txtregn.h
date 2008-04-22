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

#define REGION_COLOUR ScrollView::CYAN
#define SUBREGION_COLOUR ScrollView::GREEN

class DLLSYM TEXT_REGION;        //forward decl

ELISTIZEH_S (TEXT_REGION)
class DLLSYM TEXT_REGION:public ELIST_LINK, public POLY_BLOCK
//text REGION
{
  public:
    TEXT_REGION() {
    }                            //empty constructor
    TEXT_REGION(  //simple constructor
                inT32 idno,
                ICOORDELT_LIST *points,
                TEXT_REGION_LIST *child);

    TEXT_REGION(  //simple constructor
                inT32 idno,
                ICOORDELT_LIST *points);

    TEXT_REGION(  //constructor
                inT32 idno,
                ICOORDELT_LIST *points,
                inT8 hor,
                inT8 tex,
                inT8 ser,
                inT8 pro,
                inT8 nor,
                inT8 upr,
                inT8 sol,
                inT8 bla,
                inT8 und,
                inT8 dro);

    ~TEXT_REGION () {            //destructor
    }

    void set_id_no(inT32 new_id) {
      id_number = new_id;
    }

    inT32 id_no() {
      return id_number;
    }

    inT32 nregions() {
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

    void set_attrs(inT8 hor,
                   inT8 tex,
                   inT8 ser,
                   inT8 pro,
                   inT8 nor,
                   inT8 upr,
                   inT8 sol,
                   inT8 bla,
                   inT8 und,
                   inT8 dro);

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
    inT32 id_number;             //unique id
    inT8 horizontal;             //horizontal, vertical, skewed
    inT8 text;                   //text, table, form
    inT8 serif;                  //serif, sansserif
    inT8 proportional;           //proportional, fixed
    inT8 normal;                 //normal, bold
    inT8 upright;                //upright, italic
    inT8 solid;                  //solid, outline
    inT8 black;                  //black, coloured, white,
    inT8 underlined;             //not underlined, underlined
    inT8 dropcaps;               //not dropcaps, dropcaps

    TEXT_REGION_LIST txt_regions;
};
#endif
