/**********************************************************************
 * File:        tstruct.h  (Formerly tstruct.h)
 * Description: Code to manipulate the structures of the C++/C interface.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 15:49:29 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef           TSTRUCT_H
#define           TSTRUCT_H

#include          "tessarray.h"
#include          "werd.h"
#include          "tessclas.h"
#include          "ratngs.h"
#include          "notdll.h"
#include "oldlist.h"

/*
struct TESS_LIST
{
  TESS_LIST				*node;						//data
  TESS_LIST				*next;						//next in list
};

struct TESS_CHOICE
{
  float					rating;						//scaled
  float					certainty;					//absolute
  char					permuter;					//which permuter code
  inT8					config;						//which config
  char*					string;						//really can!
};
*/
class FRAGMENT:public ELIST_LINK
{
  public:
    FRAGMENT() {  //constructor
    }
    FRAGMENT(EDGEPT *head_pt,   //start
             EDGEPT *tail_pt);  //end

    ICOORD head;                 //coords of start
    ICOORD tail;                 //coords of end
    EDGEPT *headpt;              //start point
    EDGEPT *tailpt;              //end point

    NEWDELETE2 (FRAGMENT)
};

ELISTIZEH (FRAGMENT)
WERD *make_ed_word(                  //construct word
                   TWERD *tessword,  //word to convert
                   WERD *clone       //clone this one
                  );
PBLOB *make_ed_blob(                 //construct blob
                    TBLOB *tessblob  //blob to convert
                   );
OUTLINE *make_ed_outline(                     //constructoutline
                         FRAGMENT_LIST *list  //list of fragments
                        );
void register_outline(                     //add fragments
                      TESSLINE *outline,   //tess format
                      FRAGMENT_LIST *list  //list to add to
                     );
void convert_choice_lists(                                 //convert lists
                          ARRAY tessarray,                 //list from tess
                          BLOB_CHOICE_LIST_CLIST *ratings  //list of results
                         );
void convert_choice_list(                           //convert lists
                         LIST list,                 //list from tess
                         BLOB_CHOICE_LIST &ratings  //list of results
                        );
void make_tess_row(                  //make fake row
                   DENORM *denorm,   //row info
                   TEXTROW *tessrow  //output row
                  );
TWERD *make_tess_word(              //convert owrd
                      WERD *word,   //word to do
                      TEXTROW *row  //fake row
                     );
TBLOB *make_tess_blobs(                      //make tess blobs
                       PBLOB_LIST *bloblist  //list to convert
                      );
TBLOB *make_tess_blob(               //make tess blob
                      PBLOB *blob,   //blob to convert
                      BOOL8 flatten  //flatten outline structure
                     );
TESSLINE *make_tess_outlines(                            //make tess outlines
                             OUTLINE_LIST *outlinelist,  //list to convert
                             BOOL8 flatten               //flatten outline structure
                            );
EDGEPT *make_tess_edgepts(                          //make tess edgepts
                          POLYPT_LIST *edgeptlist,  //list to convert
                          TPOINT &tl,               //bounding box
                          TPOINT &br);
#endif
