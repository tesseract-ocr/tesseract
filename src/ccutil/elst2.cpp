/**********************************************************************
 * File:        elst2.cpp  (Formerly elist2.c)
 * Description: Doubly linked embedded list code not in the include file.
 * Author:      Phil Cheatle
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

#include "elst2.h"

#include <cstdlib>

namespace tesseract {

/***********************************************************************
 *              ELIST2::internal_clear
 *
 *  Used by the destructor and the "clear" member function of derived list
 *  classes to destroy all the elements on the list.
 *  The calling function passes a "zapper" function which can be called to
 *  delete each element of the list, regardless of its derived type.  This
 *  technique permits a generic clear function to destroy elements of
 *  different derived types correctly, without requiring virtual functions and
 *  the consequential memory overhead.
 **********************************************************************/

void ELIST2::internal_clear( // destroy all links
    void (*zapper)(void *)) {
  // ptr to zapper functn
  ELIST2_LINK *ptr;
  ELIST2_LINK *next;

  if (!empty()) {
    ptr = last->next;     // set to first
    last->next = nullptr; // break circle
    last = nullptr;       // set list empty
    while (ptr) {
      next = ptr->next;
      zapper(ptr);
      ptr = next;
    }
  }
}

/***********************************************************************
 *              ELIST2::assign_to_sublist
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

void ELIST2::assign_to_sublist( // to this list
    ELIST2_ITERATOR *start_it,  // from list start
    ELIST2_ITERATOR *end_it) {  // from list end
  constexpr ERRCODE LIST_NOT_EMPTY("Destination list must be empty before extracting a sublist");

  if (!empty()) {
    LIST_NOT_EMPTY.error("ELIST2.assign_to_sublist", ABORT);
  }

  last = start_it->extract_sublist(end_it);
}

/***********************************************************************
 *              ELIST2::sort
 *
 *  Sort elements on list
 *  NB If you don't like the const declarations in the comparator, coerce yours:
 *   (int (*)(const void *, const void *)
 **********************************************************************/

void ELIST2::sort(  // sort elements
    int comparator( // comparison routine
        const void *, const void *)) {
  // Allocate an array of pointers, one per list element.
  auto count = length();
  if (count > 0) {
    // ptr array to sort
    std::vector<ELIST2_LINK *> base;
    base.reserve(count);

    ELIST2_ITERATOR it(this);

    // Extract all elements, putting the pointers in the array.
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      base.push_back(it.extract());
    }

    // Sort the pointer array.
    qsort(&base[0], count, sizeof(base[0]), comparator);

    // Rebuild the list from the sorted pointers.
    for (auto current : base) {
      it.add_to_end(current);
    }
  }
}

// Assuming list has been sorted already, insert new_link to
// keep the list sorted according to the same comparison function.
// Comparison function is the same as used by sort, i.e. uses double
// indirection. Time is O(1) to add to beginning or end.
// Time is linear to add pre-sorted items to an empty list.
void ELIST2::add_sorted(int comparator(const void *, const void *), ELIST2_LINK *new_link) {
  // Check for adding at the end.
  if (last == nullptr || comparator(&last, &new_link) < 0) {
    if (last == nullptr) {
      new_link->next = new_link;
      new_link->prev = new_link;
    } else {
      new_link->next = last->next;
      new_link->prev = last;
      last->next = new_link;
      new_link->next->prev = new_link;
    }
    last = new_link;
  } else {
    // Need to use an iterator.
    ELIST2_ITERATOR it(this);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      ELIST2_LINK *link = it.data();
      if (comparator(&link, &new_link) > 0) {
        break;
      }
    }
    if (it.cycled_list()) {
      it.add_to_end(new_link);
    } else {
      it.add_before_then_move(new_link);
    }
  }
}

/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: ELIST2_ITERATOR
 *  ==========================================
 **********************************************************************/

/***********************************************************************
 *              ELIST2_ITERATOR::forward
 *
 *  Move the iterator to the next element of the list.
 *  REMEMBER: ALL LISTS ARE CIRCULAR.
 **********************************************************************/

ELIST2_LINK *ELIST2_ITERATOR::forward() {
#ifndef NDEBUG
  if (!list)
    NO_LIST.error("ELIST2_ITERATOR::forward", ABORT);
#endif
  if (list->empty()) {
    return nullptr;
  }

  if (current) { // not removed so
                 // set previous
    prev = current;
    started_cycling = true;
    // In case next is deleted by another iterator, get it from the current.
    current = current->next;
  } else {
    if (ex_current_was_cycle_pt) {
      cycle_pt = next;
    }
    current = next;
  }

#ifndef NDEBUG
  if (!current)
    NULL_DATA.error("ELIST2_ITERATOR::forward", ABORT);
#endif

  next = current->next;

#ifndef NDEBUG
  if (!next) {
    NULL_NEXT.error("ELIST2_ITERATOR::forward", ABORT,
                    "This is: %p  Current is: %p",
                    static_cast<void *>(this),
                    static_cast<void *>(current));
  }
#endif

  return current;
}

/***********************************************************************
 *              ELIST2_ITERATOR::backward
 *
 *  Move the iterator to the previous element of the list.
 *  REMEMBER: ALL LISTS ARE CIRCULAR.
 **********************************************************************/

ELIST2_LINK *ELIST2_ITERATOR::backward() {
#ifndef NDEBUG
  if (!list)
    NO_LIST.error("ELIST2_ITERATOR::backward", ABORT);
#endif
  if (list->empty()) {
    return nullptr;
  }

  if (current) { // not removed so
                 // set previous
    next = current;
    started_cycling = true;
    // In case prev is deleted by another iterator, get it from current.
    current = current->prev;
  } else {
    if (ex_current_was_cycle_pt) {
      cycle_pt = prev;
    }
    current = prev;
  }

#ifndef NDEBUG
  if (!current)
    NULL_DATA.error("ELIST2_ITERATOR::backward", ABORT);
  if (!prev) {
    NULL_PREV.error("ELIST2_ITERATOR::backward", ABORT,
                    "This is: %p  Current is: %p",
                    static_cast<void *>(this),
                    static_cast<void *>(current));
  }
#endif

  prev = current->prev;
  return current;
}

/***********************************************************************
 *              ELIST2_ITERATOR::data_relative
 *
 *  Return the data pointer to the element "offset" elements from current.
 *  (This function can't be INLINEd because it contains a loop)
 **********************************************************************/

ELIST2_LINK *ELIST2_ITERATOR::data_relative( // get data + or - ..
    int8_t offset) {                         // offset from current
  ELIST2_LINK *ptr;

#ifndef NDEBUG
  if (!list)
    NO_LIST.error("ELIST2_ITERATOR::data_relative", ABORT);
  if (list->empty())
    EMPTY_LIST.error("ELIST2_ITERATOR::data_relative", ABORT);
#endif

  if (offset < 0) {
    for (ptr = current ? current : next; offset++ < 0; ptr = ptr->prev) {
      ;
    }
  } else {
    for (ptr = current ? current : prev; offset-- > 0; ptr = ptr->next) {
      ;
    }
  }

#ifndef NDEBUG
  if (!ptr)
    NULL_DATA.error("ELIST2_ITERATOR::data_relative", ABORT);
#endif

  return ptr;
}

/***********************************************************************
 *              ELIST2_ITERATOR::exchange()
 *
 *  Given another iterator, whose current element is a different element on
 *  the same list list OR an element of another list, exchange the two current
 *  elements.  On return, each iterator points to the element which was the
 *  other iterators current on entry.
 *  (This function hasn't been in-lined because its a bit big!)
 **********************************************************************/

void ELIST2_ITERATOR::exchange(  // positions of 2 links
    ELIST2_ITERATOR *other_it) { // other iterator
  constexpr ERRCODE DONT_EXCHANGE_DELETED("Can't exchange deleted elements of lists");

  ELIST2_LINK *old_current;

#ifndef NDEBUG
  if (!list)
    NO_LIST.error("ELIST2_ITERATOR::exchange", ABORT);
  if (!other_it)
    BAD_PARAMETER.error("ELIST2_ITERATOR::exchange", ABORT, "other_it nullptr");
  if (!(other_it->list))
    NO_LIST.error("ELIST2_ITERATOR::exchange", ABORT, "other_it");
#endif

  /* Do nothing if either list is empty or if both iterators reference the same
link */

  if ((list->empty()) || (other_it->list->empty()) || (current == other_it->current)) {
    return;
  }

  /* Error if either current element is deleted */

  if (!current || !other_it->current) {
    DONT_EXCHANGE_DELETED.error("ELIST2_ITERATOR.exchange", ABORT);
  }

  /* Now handle the 4 cases: doubleton list; non-doubleton adjacent elements
(other before this); non-doubleton adjacent elements (this before other);
non-adjacent elements. */

  // adjacent links
  if ((next == other_it->current) || (other_it->next == current)) {
    // doubleton list
    if ((next == other_it->current) && (other_it->next == current)) {
      prev = next = current;
      other_it->prev = other_it->next = other_it->current;
    } else { // non-doubleton with
             // adjacent links
             // other before this
      if (other_it->next == current) {
        other_it->prev->next = current;
        other_it->current->next = next;
        other_it->current->prev = current;
        current->next = other_it->current;
        current->prev = other_it->prev;
        next->prev = other_it->current;

        other_it->next = other_it->current;
        prev = current;
      } else { // this before other
        prev->next = other_it->current;
        current->next = other_it->next;
        current->prev = other_it->current;
        other_it->current->next = current;
        other_it->current->prev = prev;
        other_it->next->prev = current;

        next = current;
        other_it->prev = other_it->current;
      }
    }
  } else { // no overlap
    prev->next = other_it->current;
    current->next = other_it->next;
    current->prev = other_it->prev;
    next->prev = other_it->current;
    other_it->prev->next = current;
    other_it->current->next = next;
    other_it->current->prev = prev;
    other_it->next->prev = current;
  }

  /* update end of list pointer when necessary (remember that the 2 iterators
  may iterate over different lists!) */

  if (list->last == current) {
    list->last = other_it->current;
  }
  if (other_it->list->last == other_it->current) {
    other_it->list->last = current;
  }

  if (current == cycle_pt) {
    cycle_pt = other_it->cycle_pt;
  }
  if (other_it->current == other_it->cycle_pt) {
    other_it->cycle_pt = cycle_pt;
  }

  /* The actual exchange - in all cases*/

  old_current = current;
  current = other_it->current;
  other_it->current = old_current;
}

/***********************************************************************
 *              ELIST2_ITERATOR::extract_sublist()
 *
 *  This is a private member, used only by ELIST2::assign_to_sublist.
 *  Given another iterator for the same list, extract the links from THIS to
 *  OTHER inclusive, link them into a new circular list, and return a
 *  pointer to the last element.
 *  (Can't inline this function because it contains a loop)
 **********************************************************************/

ELIST2_LINK *ELIST2_ITERATOR::extract_sublist( // from this current
    ELIST2_ITERATOR *other_it) {               // to other current
#ifndef NDEBUG
  constexpr ERRCODE BAD_EXTRACTION_PTS("Can't extract sublist from points on different lists");
  constexpr ERRCODE DONT_EXTRACT_DELETED("Can't extract a sublist marked by deleted points");
#endif
  constexpr ERRCODE BAD_SUBLIST("Can't find sublist end point in original list");

  ELIST2_ITERATOR temp_it = *this;
  ELIST2_LINK *end_of_new_list;

#ifndef NDEBUG
  if (!other_it)
    BAD_PARAMETER.error("ELIST2_ITERATOR::extract_sublist", ABORT, "other_it nullptr");
  if (!list)
    NO_LIST.error("ELIST2_ITERATOR::extract_sublist", ABORT);
  if (list != other_it->list)
    BAD_EXTRACTION_PTS.error("ELIST2_ITERATOR.extract_sublist", ABORT);
  if (list->empty())
    EMPTY_LIST.error("ELIST2_ITERATOR::extract_sublist", ABORT);

  if (!current || !other_it->current)
    DONT_EXTRACT_DELETED.error("ELIST2_ITERATOR.extract_sublist", ABORT);
#endif

  ex_current_was_last = other_it->ex_current_was_last = false;
  ex_current_was_cycle_pt = false;
  other_it->ex_current_was_cycle_pt = false;

  temp_it.mark_cycle_pt();
  do {                         // walk sublist
    if (temp_it.cycled_list()) { // can't find end pt
      BAD_SUBLIST.error("ELIST2_ITERATOR.extract_sublist", ABORT);
    }

    if (temp_it.at_last()) {
      list->last = prev;
      ex_current_was_last = other_it->ex_current_was_last = true;
    }

    if (temp_it.current == cycle_pt) {
      ex_current_was_cycle_pt = true;
    }

    if (temp_it.current == other_it->cycle_pt) {
      other_it->ex_current_was_cycle_pt = true;
    }

    temp_it.forward();
  }
  // do INCLUSIVE list
  while (temp_it.prev != other_it->current);

  // circularise sublist
  other_it->current->next = current;
  // circularise sublist
  current->prev = other_it->current;
  end_of_new_list = other_it->current;

  // sublist = whole list
  if (prev == other_it->current) {
    list->last = nullptr;
    prev = current = next = nullptr;
    other_it->prev = other_it->current = other_it->next = nullptr;
  } else {
    prev->next = other_it->next;
    other_it->next->prev = prev;

    current = other_it->current = nullptr;
    next = other_it->next;
    other_it->prev = prev;
  }
  return end_of_new_list;
}

} // namespace tesseract
