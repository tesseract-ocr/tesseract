/**********************************************************************
 * File:        txtregn.c  (Formerly text_region.c)
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

#include          "mfcpch.h"
#include          <ctype.h>
#include          <math.h>
#include          "txtregn.h"
#include          "labls.h"

#include          "hpddef.h"     //must be last (handpd.dll)

ELISTIZE_S (TEXT_REGION) TEXT_REGION::TEXT_REGION (inT32 idno, ICOORDELT_LIST * points, TEXT_REGION_LIST * child):POLY_BLOCK (points,
POLY_TEXT) {
  TEXT_REGION_IT
    c = &txt_regions;

  id_number = idno;
  txt_regions.clear ();
  c.move_to_first ();
  c.add_list_before (child);
}


TEXT_REGION::TEXT_REGION (inT32 idno, ICOORDELT_LIST * points):POLY_BLOCK (points,
POLY_TEXT) {
  id_number = idno;

  txt_regions.clear ();
}


TEXT_REGION::TEXT_REGION (       //constructor
inT32 idno, ICOORDELT_LIST * points, inT8 hor, inT8 tex, inT8 ser, inT8 pro, inT8 nor, inT8 upr, inT8 sol, inT8 bla, inT8 und, inT8 dro):POLY_BLOCK (points,
POLY_TEXT) {

  id_number = idno;
  horizontal = hor;
  text = tex;
  serif = ser;
  proportional = pro;
  normal = nor;
  upright = upr;
  solid = sol;
  black = bla;
  underlined = und;
  dropcaps = dro;

  txt_regions.clear ();
}


void TEXT_REGION::set_attrs(inT8 hor,
                            inT8 tex,
                            inT8 ser,
                            inT8 pro,
                            inT8 nor,
                            inT8 upr,
                            inT8 sol,
                            inT8 bla,
                            inT8 und,
                            inT8 dro) {

  horizontal = hor;
  text = tex;
  serif = ser;
  proportional = pro;
  normal = nor;
  upright = upr;
  solid = sol;
  black = bla;
  underlined = und;
  dropcaps = dro;
}


#include          "hpddef.h"     //must be last (handpd.dll)
void TEXT_REGION::show_attrs(  //Now uses tprintf instead
                             DEBUG_WIN *f) {
  TEXT_REGION_IT it = &txt_regions;

  if (id_number > -1) {
    f->
      dprintf
      ("Text region no. %d with attributes %s, %s, %s, %s, %s, %s, %s,	%s, %s, %s\n",
      id_number, tlabel[0][horizontal], tlabel[1][text],
      tlabel[2][serif], tlabel[3][proportional], tlabel[4][normal],
      tlabel[5][upright], tlabel[6][solid], tlabel[7][black],
      tlabel[8][underlined], tlabel[9][dropcaps]);
    if (!txt_regions.empty ()) {
      f->dprintf ("with text subregions\n");
      for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
        it.data ()->show_attrs (f);
      f->dprintf ("end of subregions\n");
    }
  }
  else
    f->
      dprintf
      ("Text subregion with attributes %s, %s, %s, %s, %s, %s, %s,	%s, %s, %s\n",
      tlabel[0][horizontal], tlabel[1][text], tlabel[2][serif],
      tlabel[3][proportional], tlabel[4][normal], tlabel[5][upright],
      tlabel[6][solid], tlabel[7][black], tlabel[8][underlined],
      tlabel[9][dropcaps]);
}


void TEXT_REGION::add_a_region(TEXT_REGION *newchild) {
  TEXT_REGION_IT c = &txt_regions;

  c.move_to_first ();
  c.add_to_end (newchild);
}


/**********************************************************************
 * TEXT_REGION::rotate
 *
 * Rotate the TEXT_REGION and its children
 **********************************************************************/

void TEXT_REGION::rotate(  //cos,sin
                         FCOORD rotation) {
                                 //sub block iterator
  TEXT_REGION_IT child_it = &txt_regions;
  TEXT_REGION *child;            //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->rotate (rotation);
  }
  POLY_BLOCK::rotate(rotation);
}


/**********************************************************************
 * TEXT_REGION::move
 *
 * Move the TEXT_REGION and its children
 **********************************************************************/

void TEXT_REGION::move(ICOORD shift  //amount to move
                      ) {
                                 //sub block iterator
  TEXT_REGION_IT child_it = &txt_regions;
  TEXT_REGION *child;            //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->move (shift);
  }
  POLY_BLOCK::move(shift);
}


/**********************************************************************
 * TEXT_REGION::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void TEXT_REGION::serialise_asc(         //convert to ascii
                                FILE *f  //file to use
                               ) {
  ((POLY_BLOCK *) this)->serialise_asc (f);
  serialise_INT32(f, id_number);  //unique id
                                 //horizontal, vertical, skewed
  serialise_INT32(f, horizontal);
  serialise_INT32(f, text);  //text, table, form
  serialise_INT32(f, serif);  //serif, sansserif
                                 //proportional, fixed
  serialise_INT32(f, proportional);
  serialise_INT32(f, normal);  //normal, bold
  serialise_INT32(f, upright);  //upright, italic
  serialise_INT32(f, solid);  //solid, outline
  serialise_INT32(f, black);  //black, coloured, white,
                                 //not underlined, underlined
  serialise_INT32(f, underlined);
  serialise_INT32(f, dropcaps);  //not dropcaps, dropcaps

  txt_regions.serialise_asc (f);
}


/**********************************************************************
 * TEXT_REGION::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void TEXT_REGION::de_serialise_asc(         //convert from ascii
                                   FILE *f  //file to use
                                  ) {
  ((POLY_BLOCK *) this)->de_serialise_asc (f);
                                 //unique id
  id_number = de_serialise_INT32 (f);
                                 //horizontal, vertical, skewed
  horizontal = de_serialise_INT32 (f);
  text = de_serialise_INT32 (f); //text, table, form
  serif = de_serialise_INT32 (f);//serif, sansserif
                                 //proportional, fixed
  proportional = de_serialise_INT32 (f);
                                 //normal, bold
  normal = de_serialise_INT32 (f);
                                 //upright, italic
  upright = de_serialise_INT32 (f);
  solid = de_serialise_INT32 (f);//solid, outline
  black = de_serialise_INT32 (f);//black, coloured, white,
                                 //not underlined, underlined
  underlined = de_serialise_INT32 (f);
                                 //not dropcaps, dropcaps
  dropcaps = de_serialise_INT32 (f);

  txt_regions.de_serialise_asc (f);
}
