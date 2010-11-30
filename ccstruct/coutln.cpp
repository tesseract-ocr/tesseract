/**********************************************************************
 * File:        coutln.c  (Formerly coutline.c)
 * Description: Code for the C_OUTLINE class.
 * Author:                  Ray Smith
 * Created:                 Mon Oct 07 16:01:57 BST 1991
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
#include <string.h>
#ifdef __UNIX__
#include <assert.h>
#endif
#include "coutln.h"
#include "allheaders.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

ELISTIZE (C_OUTLINE)
ICOORD C_OUTLINE::step_coords[4] = {
  ICOORD (-1, 0), ICOORD (0, -1), ICOORD (1, 0), ICOORD (0, 1)
};

/**********************************************************************
 * C_OUTLINE::C_OUTLINE
 *
 * Constructor to build a C_OUTLINE from a CRACKEDGE LOOP.
 **********************************************************************/

C_OUTLINE::C_OUTLINE (
//constructor
CRACKEDGE * startpt,             //outline to convert
ICOORD bot_left,                 //bounding box
ICOORD top_right, inT16 length   //length of loop
):box (bot_left, top_right), start (startpt->pos) {
  inT16 stepindex;               //index to step
  CRACKEDGE *edgept;             //current point

  stepcount = length;            //no of steps
  if (length == 0) {
    steps = NULL;
    return;
  }
                                 //get memory
  steps = (uinT8 *) alloc_mem (step_mem());
  memset(steps, 0, step_mem());
  edgept = startpt;

  for (stepindex = 0; stepindex < length; stepindex++) {
                                 //set compact step
    set_step (stepindex, edgept->stepdir);
    edgept = edgept->next;
  }
}


/**********************************************************************
 * C_OUTLINE::C_OUTLINE
 *
 * Constructor to build a C_OUTLINE from a C_OUTLINE_FRAG.
 **********************************************************************/
C_OUTLINE::C_OUTLINE (
//constructor
                                 //steps to copy
ICOORD startpt, DIR128 * new_steps,
inT16 length                     //length of loop
):start (startpt) {
  inT8 dirdiff;                  //direction difference
  DIR128 prevdir;                //previous direction
  DIR128 dir;                    //current direction
  DIR128 lastdir;                //dir of last step
  TBOX new_box;                   //easy bounding
  inT16 stepindex;               //index to step
  inT16 srcindex;                //source steps
  ICOORD pos;                    //current position

  pos = startpt;
  stepcount = length;            //no of steps
                                 //get memory
  steps = (uinT8 *) alloc_mem (step_mem());
  memset(steps, 0, step_mem());

  lastdir = new_steps[length - 1];
  prevdir = lastdir;
  for (stepindex = 0, srcindex = 0; srcindex < length;
  stepindex++, srcindex++) {
    new_box = TBOX (pos, pos);
    box += new_box;
                                 //copy steps
    dir = new_steps[srcindex];
    set_step(stepindex, dir);
    dirdiff = dir - prevdir;
    pos += step (stepindex);
    if ((dirdiff == 64 || dirdiff == -64) && stepindex > 0) {
      stepindex -= 2;            //cancel there-and-back
      prevdir = stepindex >= 0 ? step_dir (stepindex) : lastdir;
    }
    else
      prevdir = dir;
  }
  ASSERT_HOST (pos.x () == startpt.x () && pos.y () == startpt.y ());
  do {
    dirdiff = step_dir (stepindex - 1) - step_dir (0);
    if (dirdiff == 64 || dirdiff == -64) {
      start += step (0);
      stepindex -= 2;            //cancel there-and-back
      for (int i = 0; i < stepindex; ++i)
        set_step(i, step_dir(i + 1));
    }
  }
  while (stepindex > 1 && (dirdiff == 64 || dirdiff == -64));
  stepcount = stepindex;
  ASSERT_HOST (stepcount >= 4);
}

/**********************************************************************
 * C_OUTLINE::C_OUTLINE
 *
 * Constructor to build a C_OUTLINE from a rotation of a C_OUTLINE.
 **********************************************************************/

C_OUTLINE::C_OUTLINE(                     //constructor
                     C_OUTLINE *srcline,  //outline to
                     FCOORD rotation      //rotate
                    ) {
  TBOX new_box;                   //easy bounding
  inT16 stepindex;               //index to step
  inT16 dirdiff;                 //direction change
  ICOORD pos;                    //current position
  ICOORD prevpos;                //previous dest point

  ICOORD destpos;                //destination point
  inT16 destindex;               //index to step
  DIR128 dir;                    //coded direction
  uinT8 new_step;

  stepcount = srcline->stepcount * 2;
  if (stepcount == 0) {
    steps = NULL;
    box = srcline->box;
    box.rotate(rotation);
    return;
  }
                                 //get memory
  steps = (uinT8 *) alloc_mem (step_mem());
  memset(steps, 0, step_mem());

  for (int iteration = 0; iteration < 2; ++iteration) {
    DIR128 round1 = iteration == 0 ? 32 : 0;
    DIR128 round2 = iteration != 0 ? 32 : 0;
    pos = srcline->start;
    prevpos = pos;
    prevpos.rotate (rotation);
    start = prevpos;
    box = TBOX (start, start);
    destindex = 0;
    for (stepindex = 0; stepindex < srcline->stepcount; stepindex++) {
      pos += srcline->step (stepindex);
      destpos = pos;
      destpos.rotate (rotation);
      //  printf("%i %i %i %i ", destpos.x(), destpos.y(), pos.x(), pos.y());
      while (destpos.x () != prevpos.x () || destpos.y () != prevpos.y ()) {
        dir = DIR128 (FCOORD (destpos - prevpos));
        dir += 64;                 //turn to step style
        new_step = dir.get_dir ();
        //  printf(" %i\n", new_step);
        if (new_step & 31) {
          set_step(destindex++, dir + round1);
          prevpos += step(destindex - 1);
          if (destindex < 2
            || ((dirdiff =
            step_dir (destindex - 1) - step_dir (destindex - 2)) !=
            -64 && dirdiff != 64)) {
            set_step(destindex++, dir + round2);
            prevpos += step(destindex - 1);
          } else {
            prevpos -= step(destindex - 1);
            destindex--;
            prevpos -= step(destindex - 1);
            set_step(destindex - 1, dir + round2);
            prevpos += step(destindex - 1);
          }
        }
        else {
          set_step(destindex++, dir);
          prevpos += step(destindex - 1);
        }
        while (destindex >= 2 &&
               ((dirdiff =
                 step_dir (destindex - 1) - step_dir (destindex - 2)) == -64 ||
                dirdiff == 64)) {
          prevpos -= step(destindex - 1);
          prevpos -= step(destindex - 2);
          destindex -= 2;        // Forget u turn
        }
        //ASSERT_HOST(prevpos.x() == destpos.x() && prevpos.y() == destpos.y());
        new_box = TBOX (destpos, destpos);
        box += new_box;
      }
    }
    ASSERT_HOST (destpos.x () == start.x () && destpos.y () == start.y ());
    dirdiff = step_dir (destindex - 1) - step_dir (0);
    while ((dirdiff == 64 || dirdiff == -64) && destindex > 1) {
      start += step (0);
      destindex -= 2;
      for (int i = 0; i < destindex; ++i)
        set_step(i, step_dir(i + 1));
      dirdiff = step_dir (destindex - 1) - step_dir (0);
    }
    if (destindex >= 4)
      break;
  }
  ASSERT_HOST(destindex <= stepcount);
  stepcount = destindex;
  destpos = start;
  for (stepindex = 0; stepindex < stepcount; stepindex++) {
    destpos += step (stepindex);
  }
  ASSERT_HOST (destpos.x () == start.x () && destpos.y () == start.y ());
}

// Build a fake outline, given just a bounding box and append to the list.
void C_OUTLINE::FakeOutline(const TBOX& box, C_OUTLINE_LIST* outlines) {
  C_OUTLINE_IT ol_it(outlines);
  // Make a C_OUTLINE from the bounds. This is a bit of a hack,
  // as there is no outline, just a bounding box, but it works nicely.
  CRACKEDGE start;
  start.pos = box.topleft();
  C_OUTLINE* outline = new C_OUTLINE(&start, box.topleft(), box.botright(), 0);
  ol_it.add_to_end(outline);
}

/**********************************************************************
 * C_OUTLINE::area
 *
 * Compute the area of the outline.
 **********************************************************************/

inT32 C_OUTLINE::area() {  //winding number
  int stepindex;                 //current step
  inT32 total_steps;             //steps to do
  inT32 total;                   //total area
  ICOORD pos;                    //position of point
  ICOORD next_step;              //step to next pix
  C_OUTLINE_IT it = child ();

  pos = start_pos ();
  total_steps = pathlength ();
  total = 0;
  for (stepindex = 0; stepindex < total_steps; stepindex++) {
                                 //all intersected
    next_step = step (stepindex);
    if (next_step.x () < 0)
      total += pos.y ();
    else if (next_step.x () > 0)
      total -= pos.y ();
    pos += next_step;
  }
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    total += it.data ()->area ();//add areas of children

  return total;
}

/**********************************************************************
 * C_OUTLINE::perimeter
 *
 * Compute the perimeter of the outline and its first level children.
 **********************************************************************/

inT32 C_OUTLINE::perimeter() {
  inT32 total_steps;             // Return value.
  C_OUTLINE_IT it = child();

  total_steps = pathlength();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())
    total_steps += it.data()->pathlength();  // Add perimeters of children.

  return total_steps;
}


/**********************************************************************
 * C_OUTLINE::outer_area
 *
 * Compute the area of the outline.
 **********************************************************************/

inT32 C_OUTLINE::outer_area() {  //winding number
  int stepindex;                 //current step
  inT32 total_steps;             //steps to do
  inT32 total;                   //total area
  ICOORD pos;                    //position of point
  ICOORD next_step;              //step to next pix

  pos = start_pos ();
  total_steps = pathlength ();
  if (total_steps == 0)
    return box.area();
  total = 0;
  for (stepindex = 0; stepindex < total_steps; stepindex++) {
                                 //all intersected
    next_step = step (stepindex);
    if (next_step.x () < 0)
      total += pos.y ();
    else if (next_step.x () > 0)
      total -= pos.y ();
    pos += next_step;
  }

  return total;
}


/**********************************************************************
 * C_OUTLINE::count_transitions
 *
 * Compute the number of x and y maxes and mins in the outline.
 **********************************************************************/

inT32 C_OUTLINE::count_transitions(                 //winding number
                                   inT32 threshold  //on size
                                  ) {
  BOOL8 first_was_max_x;         //what was first
  BOOL8 first_was_max_y;
  BOOL8 looking_for_max_x;       //what is next
  BOOL8 looking_for_min_x;
  BOOL8 looking_for_max_y;       //what is next
  BOOL8 looking_for_min_y;
  int stepindex;                 //current step
  inT32 total_steps;             //steps to do
                                 //current limits
  inT32 max_x, min_x, max_y, min_y;
  inT32 initial_x, initial_y;    //initial limits
  inT32 total;                   //total changes
  ICOORD pos;                    //position of point
  ICOORD next_step;              //step to next pix

  pos = start_pos ();
  total_steps = pathlength ();
  total = 0;
  max_x = min_x = pos.x ();
  max_y = min_y = pos.y ();
  looking_for_max_x = TRUE;
  looking_for_min_x = TRUE;
  looking_for_max_y = TRUE;
  looking_for_min_y = TRUE;
  first_was_max_x = FALSE;
  first_was_max_y = FALSE;
  initial_x = pos.x ();
  initial_y = pos.y ();          //stop uninit warning
  for (stepindex = 0; stepindex < total_steps; stepindex++) {
                                 //all intersected
    next_step = step (stepindex);
    pos += next_step;
    if (next_step.x () < 0) {
      if (looking_for_max_x && pos.x () < min_x)
        min_x = pos.x ();
      if (looking_for_min_x && max_x - pos.x () > threshold) {
        if (looking_for_max_x) {
          initial_x = max_x;
          first_was_max_x = FALSE;
        }
        total++;
        looking_for_max_x = TRUE;
        looking_for_min_x = FALSE;
        min_x = pos.x ();        //reset min
      }
    }
    else if (next_step.x () > 0) {
      if (looking_for_min_x && pos.x () > max_x)
        max_x = pos.x ();
      if (looking_for_max_x && pos.x () - min_x > threshold) {
        if (looking_for_min_x) {
          initial_x = min_x;     //remember first min
          first_was_max_x = TRUE;
        }
        total++;
        looking_for_max_x = FALSE;
        looking_for_min_x = TRUE;
        max_x = pos.x ();
      }
    }
    else if (next_step.y () < 0) {
      if (looking_for_max_y && pos.y () < min_y)
        min_y = pos.y ();
      if (looking_for_min_y && max_y - pos.y () > threshold) {
        if (looking_for_max_y) {
          initial_y = max_y;     //remember first max
          first_was_max_y = FALSE;
        }
        total++;
        looking_for_max_y = TRUE;
        looking_for_min_y = FALSE;
        min_y = pos.y ();        //reset min
      }
    }
    else {
      if (looking_for_min_y && pos.y () > max_y)
        max_y = pos.y ();
      if (looking_for_max_y && pos.y () - min_y > threshold) {
        if (looking_for_min_y) {
          initial_y = min_y;     //remember first min
          first_was_max_y = TRUE;
        }
        total++;
        looking_for_max_y = FALSE;
        looking_for_min_y = TRUE;
        max_y = pos.y ();
      }
    }

  }
  if (first_was_max_x && looking_for_min_x) {
    if (max_x - initial_x > threshold)
      total++;
    else
      total--;
  }
  else if (!first_was_max_x && looking_for_max_x) {
    if (initial_x - min_x > threshold)
      total++;
    else
      total--;
  }
  if (first_was_max_y && looking_for_min_y) {
    if (max_y - initial_y > threshold)
      total++;
    else
      total--;
  }
  else if (!first_was_max_y && looking_for_max_y) {
    if (initial_y - min_y > threshold)
      total++;
    else
      total--;
  }

  return total;
}


/**********************************************************************
 * C_OUTLINE::operator<
 *
 * Return TRUE if the left operand is inside the right one.
 **********************************************************************/

BOOL8
C_OUTLINE::operator< (           //winding number
const C_OUTLINE & other          //other outline
) const
{
  inT16 count = 0;               //winding count
  ICOORD pos;                    //position of point
  inT32 stepindex;               //index to cstep

  if (!box.overlap (other.box))
    return FALSE;                //can't be contained
  if (stepcount == 0)
    return other.box.contains(this->box);

  pos = start;
  for (stepindex = 0; stepindex < stepcount
    && (count = other.winding_number (pos)) == INTERSECTING; stepindex++)
    pos += step (stepindex);     //try all points
  if (count == INTERSECTING) {
                                 //all intersected
    pos = other.start;
    for (stepindex = 0; stepindex < other.stepcount
      && (count = winding_number (pos)) == INTERSECTING; stepindex++)
                                 //try other way round
      pos += other.step (stepindex);
    return count == INTERSECTING || count == 0;
  }
  return count != 0;
}


/**********************************************************************
 * C_OUTLINE::winding_number
 *
 * Return the winding number of the outline around the given point.
 **********************************************************************/

inT16 C_OUTLINE::winding_number(              //winding number
                                ICOORD point  //point to wind around
                               ) const {
  inT16 stepindex;               //index to cstep
  inT16 count;                   //winding count
  ICOORD vec;                    //to current point
  ICOORD stepvec;                //step vector
  inT32 cross;                   //cross product

  vec = start - point;           //vector to it
  count = 0;
  for (stepindex = 0; stepindex < stepcount; stepindex++) {
    stepvec = step (stepindex);  //get the step
                                 //crossing the line
    if (vec.y () <= 0 && vec.y () + stepvec.y () > 0) {
      cross = vec * stepvec;     //cross product
      if (cross > 0)
        count++;                 //crossing right half
      else if (cross == 0)
        return INTERSECTING;     //going through point
    }
    else if (vec.y () > 0 && vec.y () + stepvec.y () <= 0) {
      cross = vec * stepvec;
      if (cross < 0)
        count--;                 //crossing back
      else if (cross == 0)
        return INTERSECTING;     //illegal
    }
    vec += stepvec;              //sum vectors
  }
  return count;                  //winding number
}


/**********************************************************************
 * C_OUTLINE::turn_direction
 *
 * Return the sum direction delta of the outline.
 **********************************************************************/

inT16 C_OUTLINE::turn_direction() const {  //winding number
  DIR128 prevdir;                //previous direction
  DIR128 dir;                    //current direction
  inT16 stepindex;               //index to cstep
  inT8 dirdiff;                  //direction difference
  inT16 count;                   //winding count

  if (stepcount == 0)
    return 128;
  count = 0;
  prevdir = step_dir (stepcount - 1);
  for (stepindex = 0; stepindex < stepcount; stepindex++) {
    dir = step_dir (stepindex);
    dirdiff = dir - prevdir;
    ASSERT_HOST (dirdiff == 0 || dirdiff == 32 || dirdiff == -32);
    count += dirdiff;
    prevdir = dir;
  }
  ASSERT_HOST (count == 128 || count == -128);
  return count;                  //winding number
}


/**********************************************************************
 * C_OUTLINE::reverse
 *
 * Reverse the direction of an outline.
 **********************************************************************/

void C_OUTLINE::reverse() {  //reverse drection
  DIR128 halfturn = MODULUS / 2; //amount to shift
  DIR128 stepdir;                //direction of step
  inT16 stepindex;               //index to cstep
  inT16 farindex;                //index to other side
  inT16 halfsteps;               //half of stepcount

  halfsteps = (stepcount + 1) / 2;
  for (stepindex = 0; stepindex < halfsteps; stepindex++) {
    farindex = stepcount - stepindex - 1;
    stepdir = step_dir (stepindex);
    set_step (stepindex, step_dir (farindex) + halfturn);
    set_step (farindex, stepdir + halfturn);
  }
}


/**********************************************************************
 * C_OUTLINE::move
 *
 * Move C_OUTLINE by vector
 **********************************************************************/

void C_OUTLINE::move(                  // reposition OUTLINE
                     const ICOORD vec  // by vector
                    ) {
  C_OUTLINE_IT it(&children);  // iterator

  box.move (vec);
  start += vec;

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->move (vec);      // move child outlines
}

// If this outline is smaller than the given min_size, delete this and
// remove from its list, via *it, after checking that *it points to this.
// Otherwise, if any children of this are too small, delete them.
// On entry, *it must be an iterator pointing to this. If this gets deleted
// then this is extracted from *it, so an iteration can continue.
void C_OUTLINE::RemoveSmallRecursive(int min_size, C_OUTLINE_IT* it) {
  if (box.width() < min_size || box.height() < min_size) {
    ASSERT_HOST(this == it->data());
    delete it->extract();  // Too small so get rid of it and any children.
  } else if (!children.empty()) {
    // Search the children of this, deleting any that are too small.
    C_OUTLINE_IT child_it(&children);
    for (child_it.mark_cycle_pt(); !child_it.cycled_list();
         child_it.forward()) {
      C_OUTLINE* child = child_it.data();
      child->RemoveSmallRecursive(min_size, &child_it);
    }
  }
}

// Renders the outline to the given pix, with left and top being
// the coords of the upper-left corner of the pix.
void C_OUTLINE::render(int left, int top, Pix* pix) {
  ICOORD pos = start;
  for (int stepindex = 0; stepindex < stepcount; ++stepindex) {
    ICOORD next_step = step(stepindex);
    if (next_step.y() < 0) {
      pixRasterop(pix, 0, top - pos.y(), pos.x() - left, 1,
                  PIX_NOT(PIX_DST), NULL, 0, 0);
    } else if (next_step.y() > 0) {
      pixRasterop(pix, 0, top - pos.y() - 1, pos.x() - left, 1,
                  PIX_NOT(PIX_DST), NULL, 0, 0);
    }
    pos += next_step;
  }
}

/**********************************************************************
 * C_OUTLINE::plot
 *
 * Draw the outline in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void C_OUTLINE::plot(                //draw it
                     ScrollView* window,       // window to draw in
                     ScrollView::Color colour  // colour to draw in
                    ) const {
  inT16 stepindex;               // index to cstep
  ICOORD pos;                    // current position
  DIR128 stepdir;                // direction of step

  pos = start;                   // current position
  window->Pen(colour);
  if (stepcount == 0) {
    window->Rectangle(box.left(), box.top(), box.right(), box.bottom());
    return;
  }
  window->SetCursor(pos.x(), pos.y());

  stepindex = 0;
  while (stepindex < stepcount) {
    pos += step(stepindex);    // step to next
    stepdir = step_dir(stepindex);
    stepindex++;               // count steps
    // merge straight lines
    while (stepindex < stepcount &&
           stepdir.get_dir() == step_dir(stepindex).get_dir()) {
      pos += step(stepindex);
      stepindex++;
    }
    window->DrawTo(pos.x(), pos.y());
  }
}
#endif


/**********************************************************************
 * C_OUTLINE::operator=
 *
 * Assignment - deep copy data
 **********************************************************************/

                                 //assignment
C_OUTLINE & C_OUTLINE::operator= (
const C_OUTLINE & source         //from this
) {
  box = source.box;
  start = source.start;
  if (steps != NULL)
    free_mem(steps);
  stepcount = source.stepcount;
  steps = (uinT8 *) alloc_mem (step_mem());
  memmove (steps, source.steps, step_mem());
  if (!children.empty ())
    children.clear ();
  children.deep_copy(&source.children, &deep_copy);
  return *this;
}

ICOORD C_OUTLINE::chain_step(int chaindir) {
  return step_coords[chaindir % 4];
}
