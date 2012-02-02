/**********************************************************************
 * File:        clst.c  (Formerly clist.c)
 * Description: CONS cell list handling code which is not in the include file.
 * Author:      Phil Cheatle
 * Created:     Mon Jan 28 08:33:13 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers
#include <stdlib.h>
#include "clst.h"

/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: CLIST
 *  ================================
 **********************************************************************/

/***********************************************************************
 *							CLIST::internal_deep_clear
 *
 *  Used by the "deep_clear" member function of derived list
 *  classes to destroy all the elements on the list.
 *  The calling function passes a "zapper" function which can be called to
 *  delete each data element of the list, regardless of its class.  This
 *  technique permits a generic clear function to destroy elements of
 *  different derived types correctly, without requiring virtual functions and
 *  the consequential memory overhead.
 **********************************************************************/

void
CLIST::internal_deep_clear (     //destroy all links
void (*zapper) (void *)) {       //ptr to zapper functn
  CLIST_LINK *ptr;
  CLIST_LINK *next;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST::internal_deep_clear", ABORT, NULL);
  #endif

  if (!empty ()) {
    ptr = last->next;            //set to first
    last->next = NULL;           //break circle
    last = NULL;                 //set list empty
    while (ptr) {
      next = ptr->next;
      zapper (ptr->data);
      delete(ptr);
      ptr = next;
    }
  }
}


/***********************************************************************
 *							CLIST::shallow_clear
 *
 *  Used by the destructor and the "shallow_clear" member function of derived
 *  list classes to destroy the list.
 *  The data elements are NOT destroyed.
 *
 **********************************************************************/

void CLIST::shallow_clear() {  //destroy all links
  CLIST_LINK *ptr;
  CLIST_LINK *next;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST::shallow_clear", ABORT, NULL);
  #endif

  if (!empty ()) {
    ptr = last->next;            //set to first
    last->next = NULL;           //break circle
    last = NULL;                 //set list empty
    while (ptr) {
      next = ptr->next;
      delete(ptr);
      ptr = next;
    }
  }
}

/***********************************************************************
 *							CLIST::assign_to_sublist
 *
 *  The list is set to a sublist of another list.  "This" list must be empty
 *  before this function is invoked.  The two iterators passed must refer to
 *  the same list, different from "this" one.  The sublist removed is the
 *  inclusive list from start_it's current position to end_it's current
 *  position.  If this range passes over the end of the source list then the
 *  source list has its end set to the previous element of start_it.  The
 *  extracted sublist is unaffected by the end point of the source list, its
 *  end point is always the end_it position.
 **********************************************************************/

void CLIST::assign_to_sublist(                           //to this list
                              CLIST_ITERATOR *start_it,  //from list start
                              CLIST_ITERATOR *end_it) {  //from list end
  const ERRCODE LIST_NOT_EMPTY =
    "Destination list must be empty before extracting a sublist";

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST::assign_to_sublist", ABORT, NULL);
  #endif

  if (!empty ())
    LIST_NOT_EMPTY.error ("CLIST.assign_to_sublist", ABORT, NULL);

  last = start_it->extract_sublist (end_it);
}


/***********************************************************************
 *							CLIST::length
 *
 *  Return count of elements on list
 **********************************************************************/

inT32 CLIST::length() const {  //count elements
  CLIST_ITERATOR it(const_cast<CLIST*>(this));
  inT32 count = 0;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST::length", ABORT, NULL);
  #endif

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())
    count++;
  return count;
}


/***********************************************************************
 *							CLIST::sort
 *
 *  Sort elements on list
 **********************************************************************/

void
CLIST::sort (                    //sort elements
int comparator (                 //comparison routine
const void *, const void *)) {
  CLIST_ITERATOR it(this);
  inT32 count;
  void **base;                   //ptr array to sort
  void **current;
  inT32 i;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST::sort", ABORT, NULL);
  #endif

  /* Allocate an array of pointers, one per list element */
  count = length ();
  base = (void **) malloc (count * sizeof (void *));

  /* Extract all elements, putting the pointers in the array */
  current = base;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    *current = it.extract ();
    current++;
  }

  /* Sort the pointer array */
  qsort ((char *) base, count, sizeof (*base), comparator);

  /* Rebuild the list from the sorted pointers */
  current = base;
  for (i = 0; i < count; i++) {
    it.add_to_end (*current);
    current++;
  }
  free(base);
}

// Assuming list has been sorted already, insert new_data to
// keep the list sorted according to the same comparison function.
// Comparision function is the same as used by sort, i.e. uses double
// indirection. Time is O(1) to add to beginning or end.
// Time is linear to add pre-sorted items to an empty list.
// If unique, then don't add duplicate entries.
// Returns true if the element was added to the list.
bool CLIST::add_sorted(int comparator(const void*, const void*),
                       bool unique, void* new_data) {
  // Check for adding at the end.
  if (last == NULL || comparator(&last->data, &new_data) < 0) {
    CLIST_LINK* new_element = new CLIST_LINK;
    new_element->data = new_data;
    if (last == NULL) {
      new_element->next = new_element;
    } else {
      new_element->next = last->next;
      last->next = new_element;
    }
    last = new_element;
    return true;
  } else if (!unique || last->data != new_data) {
    // Need to use an iterator.
    CLIST_ITERATOR it(this);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      void* data = it.data();
      if (data == new_data && unique)
        return false;
      if (comparator(&data, &new_data) > 0)
        break;
    }
    if (it.cycled_list())
      it.add_to_end(new_data);
    else
      it.add_before_then_move(new_data);
    return true;
  }
  return false;
}

// Assuming that the minuend and subtrahend are already sorted with
// the same comparison function, shallow clears this and then copies
// the set difference minuend - subtrahend to this, being the elements
// of minuend that do not compare equal to anything in subtrahend.
// If unique is true, any duplicates in minuend are also eliminated.
void CLIST::set_subtract(int comparator(const void*, const void*),
                         bool unique,
                         CLIST* minuend, CLIST* subtrahend) {
  shallow_clear();
  CLIST_ITERATOR m_it(minuend);
  CLIST_ITERATOR s_it(subtrahend);
  // Since both lists are sorted, finding the subtras that are not
  // minus is a case of a parallel iteration.
  for (m_it.mark_cycle_pt(); !m_it.cycled_list(); m_it.forward()) {
    void* minu = m_it.data();
    void* subtra = NULL;
    if (!s_it.empty()) {
      subtra = s_it.data();
      while (!s_it.at_last() &&
             comparator(&subtra, &minu) < 0) {
        s_it.forward();
        subtra = s_it.data();
      }
    }
    if (subtra == NULL || comparator(&subtra, &minu) != 0)
      add_sorted(comparator, unique, minu);
  }
}


/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: CLIST_ITERATOR
 *  =========================================
 **********************************************************************/

/***********************************************************************
 *							CLIST_ITERATOR::forward
 *
 *  Move the iterator to the next element of the list.
 *  REMEMBER: ALL LISTS ARE CIRCULAR.
 **********************************************************************/

void *CLIST_ITERATOR::forward() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST_ITERATOR::forward", ABORT, NULL);
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::forward", ABORT, NULL);
  #endif
  if (list->empty ())
    return NULL;

  if (current) {                 //not removed so
                                 //set previous
    prev = current;
    started_cycling = TRUE;
    // In case next is deleted by another iterator, get next from current.
    current = current->next;
  } else {
    if (ex_current_was_cycle_pt)
      cycle_pt = next;
    current = next;
  }
  next = current->next;

  #ifndef NDEBUG
  if (!current)
    NULL_DATA.error ("CLIST_ITERATOR::forward", ABORT, NULL);
  if (!next)
    NULL_NEXT.error ("CLIST_ITERATOR::forward", ABORT,
                     "This is: %p  Current is: %p", this, current);
  #endif
  return current->data;
}


/***********************************************************************
 *							CLIST_ITERATOR::data_relative
 *
 *  Return the data pointer to the element "offset" elements from current.
 *  "offset" must not be less than -1.
 *  (This function can't be INLINEd because it contains a loop)
 **********************************************************************/

void *CLIST_ITERATOR::data_relative(                //get data + or - ...
                                    inT8 offset) {  //offset from current
  CLIST_LINK *ptr;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST_ITERATOR::data_relative", ABORT, NULL);
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::data_relative", ABORT, NULL);
  if (list->empty ())
    EMPTY_LIST.error ("CLIST_ITERATOR::data_relative", ABORT, NULL);
  if (offset < -1)
    BAD_PARAMETER.error ("CLIST_ITERATOR::data_relative", ABORT,
      "offset < -l");
  #endif

  if (offset == -1)
    ptr = prev;
  else
    for (ptr = current ? current : prev; offset-- > 0; ptr = ptr->next);

  #ifndef NDEBUG
  if (!ptr)
    NULL_DATA.error ("CLIST_ITERATOR::data_relative", ABORT, NULL);
  #endif

  return ptr->data;
}


/***********************************************************************
 *							CLIST_ITERATOR::move_to_last()
 *
 *  Move current so that it is set to the end of the list.
 *  Return data just in case anyone wants it.
 *  (This function can't be INLINEd because it contains a loop)
 **********************************************************************/

void *CLIST_ITERATOR::move_to_last() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST_ITERATOR::move_to_last", ABORT, NULL);
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::move_to_last", ABORT, NULL);
  #endif

  while (current != list->last)
    forward();

  if (current == NULL)
    return NULL;
  else
    return current->data;
}


/***********************************************************************
 *							CLIST_ITERATOR::exchange()
 *
 *  Given another iterator, whose current element is a different element on
 *  the same list list OR an element of another list, exchange the two current
 *  elements.  On return, each iterator points to the element which was the
 *  other iterators current on entry.
 *  (This function hasn't been in-lined because its a bit big!)
 **********************************************************************/

void CLIST_ITERATOR::exchange(                             //positions of 2 links
                              CLIST_ITERATOR *other_it) {  //other iterator
  const ERRCODE DONT_EXCHANGE_DELETED =
    "Can't exchange deleted elements of lists";

  CLIST_LINK *old_current;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("CLIST_ITERATOR::exchange", ABORT, NULL);
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::exchange", ABORT, NULL);
  if (!other_it)
    BAD_PARAMETER.error ("CLIST_ITERATOR::exchange", ABORT, "other_it NULL");
  if (!(other_it->list))
    NO_LIST.error ("CLIST_ITERATOR::exchange", ABORT, "other_it");
  #endif

  /* Do nothing if either list is empty or if both iterators reference the same
  link */

  if ((list->empty ()) ||
    (other_it->list->empty ()) || (current == other_it->current))
    return;

  /* Error if either current element is deleted */

  if (!current || !other_it->current)
    DONT_EXCHANGE_DELETED.error ("CLIST_ITERATOR.exchange", ABORT, NULL);

  /* Now handle the 4 cases: doubleton list; non-doubleton adjacent elements
  (other before this); non-doubleton adjacent elements (this before other);
  non-adjacent elements. */

                                 //adjacent links
  if ((next == other_it->current) ||
  (other_it->next == current)) {
                                 //doubleton list
    if ((next == other_it->current) &&
    (other_it->next == current)) {
      prev = next = current;
      other_it->prev = other_it->next = other_it->current;
    }
    else {                       //non-doubleton with
                                 //adjacent links
                                 //other before this
      if (other_it->next == current) {
        other_it->prev->next = current;
        other_it->current->next = next;
        current->next = other_it->current;
        other_it->next = other_it->current;
        prev = current;
      }
      else {                     //this before other
        prev->next = other_it->current;
        current->next = other_it->next;
        other_it->current->next = current;
        next = current;
        other_it->prev = other_it->current;
      }
    }
  }
  else {                         //no overlap
    prev->next = other_it->current;
    current->next = other_it->next;
    other_it->prev->next = current;
    other_it->current->next = next;
  }

  /* update end of list pointer when necessary (remember that the 2 iterators
    may iterate over different lists!) */

  if (list->last == current)
    list->last = other_it->current;
  if (other_it->list->last == other_it->current)
    other_it->list->last = current;

  if (current == cycle_pt)
    cycle_pt = other_it->cycle_pt;
  if (other_it->current == other_it->cycle_pt)
    other_it->cycle_pt = cycle_pt;

  /* The actual exchange - in all cases*/

  old_current = current;
  current = other_it->current;
  other_it->current = old_current;
}


/***********************************************************************
 *							CLIST_ITERATOR::extract_sublist()
 *
 *  This is a private member, used only by CLIST::assign_to_sublist.
 *  Given another iterator for the same list, extract the links from THIS to
 *  OTHER inclusive, link them into a new circular list, and return a
 *  pointer to the last element.
 *  (Can't inline this function because it contains a loop)
 **********************************************************************/

CLIST_LINK *CLIST_ITERATOR::extract_sublist(                             //from this current
                                            CLIST_ITERATOR *other_it) {  //to other current
  CLIST_ITERATOR temp_it = *this;
  CLIST_LINK *end_of_new_list;

  const ERRCODE BAD_SUBLIST = "Can't find sublist end point in original list";
  #ifndef NDEBUG
  const ERRCODE BAD_EXTRACTION_PTS =
    "Can't extract sublist from points on different lists";
  const ERRCODE DONT_EXTRACT_DELETED =
    "Can't extract a sublist marked by deleted points";

  if (!this)
    NULL_OBJECT.error ("CLIST_ITERATOR::extract_sublist", ABORT, NULL);
  if (!other_it)
    BAD_PARAMETER.error ("CLIST_ITERATOR::extract_sublist", ABORT,
      "other_it NULL");
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::extract_sublist", ABORT, NULL);
  if (list != other_it->list)
    BAD_EXTRACTION_PTS.error ("CLIST_ITERATOR.extract_sublist", ABORT, NULL);
  if (list->empty ())
    EMPTY_LIST.error ("CLIST_ITERATOR::extract_sublist", ABORT, NULL);

  if (!current || !other_it->current)
    DONT_EXTRACT_DELETED.error ("CLIST_ITERATOR.extract_sublist", ABORT,
      NULL);
  #endif

  ex_current_was_last = other_it->ex_current_was_last = FALSE;
  ex_current_was_cycle_pt = FALSE;
  other_it->ex_current_was_cycle_pt = FALSE;

  temp_it.mark_cycle_pt ();
  do {                           //walk sublist
    if (temp_it.cycled_list ())  //cant find end pt
      BAD_SUBLIST.error ("CLIST_ITERATOR.extract_sublist", ABORT, NULL);

    if (temp_it.at_last ()) {
      list->last = prev;
      ex_current_was_last = other_it->ex_current_was_last = TRUE;
    }

    if (temp_it.current == cycle_pt)
      ex_current_was_cycle_pt = TRUE;

    if (temp_it.current == other_it->cycle_pt)
      other_it->ex_current_was_cycle_pt = TRUE;

    temp_it.forward ();
  }
  while (temp_it.prev != other_it->current);

                                 //circularise sublist
  other_it->current->next = current;
  end_of_new_list = other_it->current;

                                 //sublist = whole list
  if (prev == other_it->current) {
    list->last = NULL;
    prev = current = next = NULL;
    other_it->prev = other_it->current = other_it->next = NULL;
  }
  else {
    prev->next = other_it->next;
    current = other_it->current = NULL;
    next = other_it->next;
    other_it->prev = prev;
  }
  return end_of_new_list;
}
