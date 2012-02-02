/**********************************************************************
 * File:        stepblob.cpp  (Formerly cblob.c)
 * Description: Code for C_BLOB class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 10:41:13 BST 1991
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
#include "stepblob.h"
#include "allheaders.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

ELISTIZE (C_BLOB)
/**********************************************************************
 * position_outline
 *
 * Position the outline in the given list at the relevant place
 * according to its nesting.
 **********************************************************************/
static void position_outline(                          //put in place
                             C_OUTLINE *outline,       //thing to place
                             C_OUTLINE_LIST *destlist  //desstination list
                            ) {
  C_OUTLINE *dest_outline;       //outline from dest list
  C_OUTLINE_IT it = destlist;    //iterator
                                 //iterator on children
  C_OUTLINE_IT child_it = outline->child ();

  if (!it.empty ()) {
    do {
      dest_outline = it.data (); //get destination
                                 //encloses dest
      if (*dest_outline < *outline) {
                                 //take off list
        dest_outline = it.extract ();
                                 //put this in place
        it.add_after_then_move (outline);
                                 //make it a child
        child_it.add_to_end (dest_outline);
        while (!it.at_last ()) {
          it.forward ();         //do rest of list
                                 //check for other children
          dest_outline = it.data ();
          if (*dest_outline < *outline) {
                                 //take off list
            dest_outline = it.extract ();
            child_it.add_to_end (dest_outline);
            //make it a child
            if (it.empty ())
              break;
          }
        }
        return;                  //finished
      }
                                 //enclosed by dest
      else if (*outline < *dest_outline) {
        position_outline (outline, dest_outline->child ());
        //place in child list
        return;                  //finished
      }
      it.forward ();
    }
    while (!it.at_first ());
  }
  it.add_to_end (outline);       //at outer level
}


/**********************************************************************
 * plot_outline_list
 *
 * Draw a list of outlines in the given colour and their children
 * in the child colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
static void plot_outline_list(                       //draw outlines
                              C_OUTLINE_LIST *list,  //outline to draw
                              ScrollView* window,         //window to draw in
                              ScrollView::Color colour,         //colour to use
                              ScrollView::Color child_colour    //colour of children
                             ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = list;        //iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
                                 //draw it
    outline->plot (window, colour);
    if (!outline->child ()->empty ())
      plot_outline_list (outline->child (), window,
        child_colour, child_colour);
  }
}
#endif


/**********************************************************************
 * reverse_outline_list
 *
 * Reverse a list of outlines and their children.
 **********************************************************************/

static void reverse_outline_list(                      //reverse outlines
                                 C_OUTLINE_LIST *list  //outline to reverse
                                ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = list;        //iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    outline->reverse ();         //reverse it
    if (!outline->child ()->empty ())
      reverse_outline_list (outline->child ());
  }
}


/**********************************************************************
 * C_BLOB::C_BLOB
 *
 * Constructor to build a C_BLOB from a list of C_OUTLINEs.
 * The C_OUTLINEs are not copied so the source list is emptied.
 * The C_OUTLINEs are nested correctly in the blob.
 **********************************************************************/

C_BLOB::C_BLOB(                              //constructor
               C_OUTLINE_LIST *outline_list  //in random order
              ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = outline_list;//iterator

  while (!it.empty ()) {         //grab the list
    outline = it.extract ();     //get off the list
                                 //put it in place
    position_outline(outline, &outlines);
    if (!it.empty ())
      it.forward ();
  }
  it.set_to_list (&outlines);
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    if (outline->turn_direction () < 0) {
      outline->reverse ();
      reverse_outline_list (outline->child ());
      outline->set_flag (COUT_INVERSE, TRUE);
    }
    else {
      outline->set_flag (COUT_INVERSE, FALSE);
    }
  }
}

// Simpler constructor to build a blob from a single outline that has
// already been fully initialized.
C_BLOB::C_BLOB(C_OUTLINE* outline) {
  C_OUTLINE_IT it(&outlines);
  it.add_to_end(outline);
}


// Build and return a fake blob containing a single fake outline with no
// steps.
C_BLOB* C_BLOB::FakeBlob(const TBOX& box) {
  C_OUTLINE_LIST outlines;
  C_OUTLINE::FakeOutline(box, &outlines);
  return new C_BLOB(&outlines);
}

/**********************************************************************
 * C_BLOB::bounding_box
 *
 * Return the bounding box of the blob.
 **********************************************************************/

TBOX C_BLOB::bounding_box() {  //bounding box
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  TBOX box;                       //bounding box

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    box += outline->bounding_box ();
  }
  return box;
}


/**********************************************************************
 * C_BLOB::area
 *
 * Return the area of the blob.
 **********************************************************************/

inT32 C_BLOB::area() {  //area
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  inT32 total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->area ();
  }
  return total;
}

/**********************************************************************
 * C_BLOB::perimeter
 *
 * Return the perimeter of the top and 2nd level outlines.
 **********************************************************************/

inT32 C_BLOB::perimeter() {
  C_OUTLINE *outline;            // current outline
  C_OUTLINE_IT it = &outlines;   // outlines of blob
  inT32 total;                   // total perimeter

  total = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    outline = it.data();
    total += outline->perimeter();
  }
  return total;
}


/**********************************************************************
 * C_BLOB::outer_area
 *
 * Return the area of the blob.
 **********************************************************************/

inT32 C_BLOB::outer_area() {  //area
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  inT32 total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->outer_area ();
  }
  return total;
}


/**********************************************************************
 * C_BLOB::count_transitions
 *
 * Return the total x and y maxes and mins in the blob.
 * Chlid outlines are not counted.
 **********************************************************************/

inT32 C_BLOB::count_transitions(                 //area
                                inT32 threshold  //on size
                               ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  inT32 total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->count_transitions (threshold);
  }
  return total;
}


/**********************************************************************
 * C_BLOB::move
 *
 * Move C_BLOB by vector
 **********************************************************************/

void C_BLOB::move(                  // reposition blob
                  const ICOORD vec  // by vector
                 ) {
  C_OUTLINE_IT it(&outlines);  // iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->move (vec);      // move each outline
}

// Static helper for C_BLOB::rotate to allow recursion of child outlines.
void RotateOutlineList(const FCOORD& rotation, C_OUTLINE_LIST* outlines) {
  C_OUTLINE_LIST new_outlines;
  C_OUTLINE_IT src_it(outlines);
  C_OUTLINE_IT dest_it(&new_outlines);
  while (!src_it.empty()) {
    C_OUTLINE* old_outline = src_it.extract();
    src_it.forward();
    C_OUTLINE* new_outline = new C_OUTLINE(old_outline, rotation);
    if (!old_outline->child()->empty()) {
      RotateOutlineList(rotation, old_outline->child());
      C_OUTLINE_IT child_it(new_outline->child());
      child_it.add_list_after(old_outline->child());
    }
    delete old_outline;
    dest_it.add_to_end(new_outline);
  }
  src_it.add_list_after(&new_outlines);
}

/**********************************************************************
 * C_BLOB::rotate
 *
 * Rotate C_BLOB by rotation.
 * Warning! has to rebuild all the C_OUTLINEs.
 **********************************************************************/
void C_BLOB::rotate(const FCOORD& rotation) {
  RotateOutlineList(rotation, &outlines);
}

static void render_outline_list(C_OUTLINE_LIST *list,
                                int left, int top, Pix* pix) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->render(left, top, pix);
    if (!outline->child()->empty())
      render_outline_list(outline->child(), left, top, pix);
  }
}

static void render_outline_list_outline(C_OUTLINE_LIST *list,
                                        int left, int top, Pix* pix) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->render_outline(left, top, pix);
  }
}

// Returns a Pix rendering of the blob. pixDestroy after use.
Pix* C_BLOB::render() {
  TBOX box = bounding_box();
  Pix* pix = pixCreate(box.width(), box.height(), 1);
  render_outline_list(&outlines, box.left(), box.top(), pix);
  return pix;
}

// Returns a Pix rendering of the outline of the blob. (no fill).
// pixDestroy after use.
Pix* C_BLOB::render_outline() {
  TBOX box = bounding_box();
  Pix* pix = pixCreate(box.width(), box.height(), 1);
  render_outline_list_outline(&outlines, box.left(), box.top(), pix);
  return pix;
}

/**********************************************************************
 * C_BLOB::plot
 *
 * Draw the C_BLOB in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void C_BLOB::plot(                     //draw it
                  ScrollView* window,       //window to draw in
                  ScrollView::Color blob_colour,  //main colour
                  ScrollView::Color child_colour  //for holes
                 ) {
  plot_outline_list(&outlines, window, blob_colour, child_colour);
}
#endif
