/**********************************************************************
 * File:        tstruct.cpp  (Formerly tstruct.c)
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

#include "mfcpch.h"
#include "ccstruct.h"
#include "helpers.h"
#include "tfacep.h"
#include "tstruct.h"
#include "makerow.h"
#include "ocrblock.h"

/**********************************************************************
 * make_tess_blob
 *
 * Make a single Tess style blob
 **********************************************************************/

TBLOB *make_tess_blob(PBLOB *blob) {
  TBLOB* tessblob = new TBLOB;
  tessblob->outlines = make_tess_outlines(blob->out_list(), false);
  tessblob->next = NULL;
  return tessblob;
}


/**********************************************************************
 * make_tess_outlines
 *
 * Make Tess style outlines from a list of OUTLINEs.
 **********************************************************************/

TESSLINE *make_tess_outlines(OUTLINE_LIST *outlinelist,  // List to convert.
                             bool is_holes) {  // These are hole outlines.
  OUTLINE_IT it = outlinelist;   //iterator
  OUTLINE *outline;              //current outline
  TESSLINE *head;                //output list
  TESSLINE *tail;                //end of list
  TESSLINE *tessoutline;

  head = NULL;
  tail = NULL;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    outline = it.data();
    tessoutline = new TESSLINE;
    tessoutline->loop = make_tess_edgepts(outline->polypts(),
                                          tessoutline->topleft,
                                          tessoutline->botright);
    if (tessoutline->loop == NULL) {
      delete tessoutline;
      continue;
    }
    tessoutline->start = tessoutline->loop->pos;
    tessoutline->next = NULL;
    tessoutline->is_hole = is_holes;
    if (!outline->child()->empty())
      tessoutline->next = make_tess_outlines(outline->child(), true);
    else
      tessoutline->next = NULL;
    if (head)
      tail->next = tessoutline;
    else
      head = tessoutline;
    while (tessoutline->next != NULL)
      tessoutline = tessoutline->next;
    tail = tessoutline;
  }
  return head;
}


/**********************************************************************
 * make_tess_edgepts
 *
 * Make Tess style edgepts from a list of POLYPTs.
 **********************************************************************/

EDGEPT *make_tess_edgepts(                          //make tess edgepts
                          POLYPT_LIST *edgeptlist,  //list to convert
                          TPOINT &tl,               //bounding box
                          TPOINT &br) {
  inT32 index;
  POLYPT_IT it = edgeptlist;     //iterator
  POLYPT *edgept;                //current edgept
  EDGEPT *head;                  //output list
  EDGEPT *tail;                  //end of list
  EDGEPT *tessedgept;

  head = NULL;
  tail = NULL;
  tl.x = MAX_INT16;
  tl.y = -MAX_INT16;
  br.x = -MAX_INT16;
  br.y = MAX_INT16;
  for (it.mark_cycle_pt(); !it.cycled_list ();) {
    edgept = it.data();
    tessedgept = new EDGEPT;
    tessedgept->pos.x = (inT16) edgept->pos.x();
    tessedgept->pos.y = (inT16) edgept->pos.y();
    UpdateRange(tessedgept->pos.x, &tl.x, &br.x);
    UpdateRange(tessedgept->pos.y, &br.y, &tl.y);
    if (head != NULL &&
        tessedgept->pos.x == tail->pos.x &&
        tessedgept->pos.y == tail->pos.y) {
      delete tessedgept;
    }
    else {
      for (index = 0; index < EDGEPTFLAGS; index++)
        tessedgept->flags[index] = 0;
      if (head != NULL) {
        tail->vec.x = tessedgept->pos.x - tail->pos.x;
        tail->vec.y = tessedgept->pos.y - tail->pos.y;
        tessedgept->prev = tail;
      }
      tessedgept->next = head;
      if (head)
        tail->next = tessedgept;
      else
        head = tessedgept;
      tail = tessedgept;
    }
    it.forward ();
  }
  head->prev = tail;
  tail->vec.x = head->pos.x - tail->pos.x;
  tail->vec.y = head->pos.y - tail->pos.y;
  if (head == tail) {
    delete head;
    return NULL;                 //empty
  }
  return head;
}
