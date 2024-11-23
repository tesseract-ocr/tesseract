/**********************************************************************
 * File:        elst2.h  (Formerly elist2.h)
 * Description: Double linked embedded list module include file.
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

#ifndef ELST2_H
#define ELST2_H

#include "lsterr.h"
#include "serialis.h"

#include <algorithm>
#include <cstdio>

namespace tesseract {

/**********************************************************************
DESIGN NOTE
===========

It would probably be possible to implement the ELIST2 classes as derived
classes from ELIST.  I haven't done this because:

a) I think it would be harder to understand the code
(Though the problem with not inheriting is that changes to ELIST must be
  reflected in ELIST2 and vice versa)

b) Most of the code is inline so:
i)  The duplication in source does not affect the run time code size - the
    code is copied inline anyway!

  ii) The compiler should have a bit less work to do!
**********************************************************************/

/**********************************************************************
 * CLASS - ELIST2
 *
 * Generic list class for doubly linked lists with embedded links
 **********************************************************************/

template <typename T>
class IntrusiveList {
public:
  /**********************************************************************
   *              CLASS - Link
   *
   *              Generic link class for doubly linked lists with embedded links
   *
   *  Note:  No destructor - elements are assumed to be destroyed EITHER after
   *  they have been extracted from a list OR by the ELIST2 destructor which
   *  walks the list.
   **********************************************************************/

  class Link {
    friend class Iterator;
    friend class IntrusiveList;

    T *prev;
    T *next;

  public:
    Link() { // constructor
      prev = next = nullptr;
    }

    Link(const Link &) = delete;

    // The assignment operator is required for WERD.
    void operator=(const Link &) {
      prev = next = nullptr;
    }
  };
  using LINK = Link; // compat

  /***********************************************************************
   *              CLASS - ELIST2_ITERATOR
   *
   *              Generic iterator class for doubly linked lists with embedded
   *links
   **********************************************************************/

  class Iterator {
    friend void IntrusiveList::assign_to_sublist(Iterator *, Iterator *);

    IntrusiveList *list;                 // List being iterated
    T *prev;            // prev element
    T *current;         // current element
    T *next;            // next element
    T *cycle_pt;        // point we are cycling the list to.
    bool ex_current_was_last;     // current extracted was end of list
    bool ex_current_was_cycle_pt; // current extracted was cycle point
    bool started_cycling;         // Have we moved off the start?
    /***********************************************************************
   *              ELIST2_ITERATOR::extract_sublist()
   *
   *  This is a private member, used only by IntrusiveList::assign_to_sublist.
   *  Given another iterator for the same list, extract the links from THIS to
   *  OTHER inclusive, link them into a new circular list, and return a
   *  pointer to the last element.
   *  (Can't inline this function because it contains a loop)
   **********************************************************************/
    T *extract_sublist(   // from this current...
      Iterator *other_it) {               // to other current
#ifndef NDEBUG
      constexpr ERRCODE BAD_EXTRACTION_PTS("Can't extract sublist from points on different lists");
      constexpr ERRCODE DONT_EXTRACT_DELETED("Can't extract a sublist marked by deleted points");
#endif
      constexpr ERRCODE BAD_SUBLIST("Can't find sublist end point in original list");

      Iterator temp_it = *this;
      T *end_of_new_list;

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
    } // to other current

  public:
    /***********************************************************************
   *              ELIST2_ITERATOR::ELIST2_ITERATOR
   *
   *  CONSTRUCTOR - set iterator to specified list;
   **********************************************************************/
    Iterator( // constructor
      IntrusiveList *list_to_iterate) {
      set_to_list(list_to_iterate);
    }

    /***********************************************************************
     *              ELIST2_ITERATOR::set_to_list
     *
     *  (Re-)initialise the iterator to point to the start of the list_to_iterate
     *  over.
     **********************************************************************/

    void set_to_list( // change list
      IntrusiveList *list_to_iterate) {
#ifndef NDEBUG
      if (!list_to_iterate) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::set_to_list", ABORT, "list_to_iterate is nullptr");
      }
#endif

      list = list_to_iterate;
      prev = list->last;
      current = list->First();
      next = current ? current->next : nullptr;
      cycle_pt = nullptr; // await explicit set
      started_cycling = false;
      ex_current_was_last = false;
      ex_current_was_cycle_pt = false;
    }
    /***********************************************************************
   *              ELIST2_ITERATOR::add_after_then_move
   *
   *  Add a new element to the list after the current element and move the
   *  iterator to the new element.
   **********************************************************************/
    void add_after_then_move(   // add after current &
      T *new_element) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_after_then_move", ABORT);
      }
      if (!new_element) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_after_then_move", ABORT, "new_element is nullptr");
      }
      if (new_element->next) {
        STILL_LINKED.error("ELIST2_ITERATOR::add_after_then_move", ABORT);
      }
#endif

      if (list->empty()) {
        new_element->next = new_element;
        new_element->prev = new_element;
        list->last = new_element;
        prev = next = new_element;
      } else {
        new_element->next = next;
        next->prev = new_element;

        if (current) { // not extracted
          new_element->prev = current;
          current->next = new_element;
          prev = current;
          if (current == list->last) {
            list->last = new_element;
          }
        } else { // current extracted
          new_element->prev = prev;
          prev->next = new_element;
          if (ex_current_was_last) {
            list->last = new_element;
          }
          if (ex_current_was_cycle_pt) {
            cycle_pt = new_element;
          }
        }
      }
      current = new_element;
    } // move to new
      /***********************************************************************
     *              ELIST2_ITERATOR::add_after_stay_put
     *
     *  Add a new element to the list after the current element but do not move
     *  the iterator to the new element.
     **********************************************************************/
    void add_after_stay_put(    // add after current &
      T *new_element) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_after_stay_put", ABORT);
      }
      if (!new_element) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_after_stay_put", ABORT, "new_element is nullptr");
      }
      if (new_element->next) {
        STILL_LINKED.error("ELIST2_ITERATOR::add_after_stay_put", ABORT);
      }
#endif

      if (list->empty()) {
        new_element->next = new_element;
        new_element->prev = new_element;
        list->last = new_element;
        prev = next = new_element;
        ex_current_was_last = false;
        current = nullptr;
      } else {
        new_element->next = next;
        next->prev = new_element;

        if (current) { // not extracted
          new_element->prev = current;
          current->next = new_element;
          if (prev == current) {
            prev = new_element;
          }
          if (current == list->last) {
            list->last = new_element;
          }
        } else { // current extracted
          new_element->prev = prev;
          prev->next = new_element;
          if (ex_current_was_last) {
            list->last = new_element;
            ex_current_was_last = false;
          }
        }
        next = new_element;
      }
    } // stay at current
      /***********************************************************************
     *              ELIST2_ITERATOR::add_before_then_move
     *
     *  Add a new element to the list before the current element and move the
     *  iterator to the new element.
     **********************************************************************/
    void add_before_then_move(  // add before current &
      T *new_element) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_before_then_move", ABORT);
      }
      if (!new_element) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_before_then_move", ABORT, "new_element is nullptr");
      }
      if (new_element->next) {
        STILL_LINKED.error("ELIST2_ITERATOR::add_before_then_move", ABORT);
      }
#endif

      if (list->empty()) {
        new_element->next = new_element;
        new_element->prev = new_element;
        list->last = new_element;
        prev = next = new_element;
      } else {
        prev->next = new_element;
        new_element->prev = prev;

        if (current) { // not extracted
          new_element->next = current;
          current->prev = new_element;
          next = current;
        } else { // current extracted
          new_element->next = next;
          next->prev = new_element;
          if (ex_current_was_last) {
            list->last = new_element;
          }
          if (ex_current_was_cycle_pt) {
            cycle_pt = new_element;
          }
        }
      }
      current = new_element;
    } // move to new
      /***********************************************************************
     *              ELIST2_ITERATOR::add_before_stay_put
     *
     *  Add a new element to the list before the current element but don't move the
     *  iterator to the new element.
     **********************************************************************/
    void add_before_stay_put(   // add before current &
      T *new_element) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_before_stay_put", ABORT);
      }
      if (!new_element) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_before_stay_put", ABORT, "new_element is nullptr");
      }
      if (new_element->next) {
        STILL_LINKED.error("ELIST2_ITERATOR::add_before_stay_put", ABORT);
      }
#endif

      if (list->empty()) {
        new_element->next = new_element;
        new_element->prev = new_element;
        list->last = new_element;
        prev = next = new_element;
        ex_current_was_last = true;
        current = nullptr;
      } else {
        prev->next = new_element;
        new_element->prev = prev;

        if (current) { // not extracted
          new_element->next = current;
          current->prev = new_element;
          if (next == current) {
            next = new_element;
          }
        } else { // current extracted
          new_element->next = next;
          next->prev = new_element;
          if (ex_current_was_last) {
            list->last = new_element;
          }
        }
        prev = new_element;
      }
    } // stay at current
      /***********************************************************************
     *              ELIST2_ITERATOR::add_list_after
     *
     *  Insert another list to this list after the current element but don't move
     *the
     *  iterator.
     **********************************************************************/
    void add_list_after(      // add a list &
      IntrusiveList *list_to_add) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_list_after", ABORT);
      }
      if (!list_to_add) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_list_after", ABORT, "list_to_add is nullptr");
      }
#endif

      if (!list_to_add->empty()) {
        if (list->empty()) {
          list->last = list_to_add->last;
          prev = list->last;
          next = list->First();
          ex_current_was_last = true;
          current = nullptr;
        } else {
          if (current) { // not extracted
            current->next = list_to_add->First();
            current->next->prev = current;
            if (current == list->last) {
              list->last = list_to_add->last;
            }
            list_to_add->last->next = next;
            next->prev = list_to_add->last;
            next = current->next;
          } else { // current extracted
            prev->next = list_to_add->First();
            prev->next->prev = prev;
            if (ex_current_was_last) {
              list->last = list_to_add->last;
              ex_current_was_last = false;
            }
            list_to_add->last->next = next;
            next->prev = list_to_add->last;
            next = prev->next;
          }
        }
        list_to_add->last = nullptr;
      }
    } // stay at current
      /***********************************************************************
     *              ELIST2_ITERATOR::add_list_before
     *
     *  Insert another list to this list before the current element. Move the
     *  iterator to the start of the inserted elements
     *  iterator.
     **********************************************************************/
    void add_list_before(     // add a list &
      IntrusiveList *list_to_add) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_list_before", ABORT);
      }
      if (!list_to_add) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_list_before", ABORT, "list_to_add is nullptr");
      }
#endif

      if (!list_to_add->empty()) {
        if (list->empty()) {
          list->last = list_to_add->last;
          prev = list->last;
          current = list->First();
          next = current->next;
          ex_current_was_last = false;
        } else {
          prev->next = list_to_add->First();
          prev->next->prev = prev;

          if (current) { // not extracted
            list_to_add->last->next = current;
            current->prev = list_to_add->last;
          } else { // current extracted
            list_to_add->last->next = next;
            next->prev = list_to_add->last;
            if (ex_current_was_last) {
              list->last = list_to_add->last;
            }
            if (ex_current_was_cycle_pt) {
              cycle_pt = prev->next;
            }
          }
          current = prev->next;
          next = current->next;
        }
        list_to_add->last = nullptr;
      }
    } // move to it 1st item

    T *data() { // get current data
#ifndef NDEBUG
      if (!current) {
        NULL_DATA.error("ELIST2_ITERATOR::data", ABORT);
      }
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::data", ABORT);
      }
#endif
      return current;
    }
    /***********************************************************************
   *              ELIST2_ITERATOR::data_relative
   *
   *  Return the data pointer to the element "offset" elements from current.
   *  (This function can't be INLINEd because it contains a loop)
   **********************************************************************/
    T *data_relative( // get data + or - ...
      int8_t offset) {                         // offset from current
      T *ptr;

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
    }         // offset from current
      /***********************************************************************
     *              ELIST2_ITERATOR::forward
     *
     *  Move the iterator to the next element of the list.
     *  REMEMBER: ALL LISTS ARE CIRCULAR.
     **********************************************************************/
    T *forward() {
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
    } // move to next element
      /***********************************************************************
     *              ELIST2_ITERATOR::backward
     *
     *  Move the iterator to the previous element of the list.
     *  REMEMBER: ALL LISTS ARE CIRCULAR.
     **********************************************************************/
    T *backward() {
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
    } // move to prev element
      /***********************************************************************
     *              ELIST2_ITERATOR::extract
     *
     *  Do extraction by removing current from the list, returning it to the
     *  caller, but NOT updating the iterator.  (So that any calling loop can do
     *  this.)   The iterator's current points to nullptr.  If the extracted element
     *  is to be deleted, this is the callers responsibility.
     **********************************************************************/
    T *extract() {
      T *extracted_link;

#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::extract", ABORT);
      }
      if (!current) { // list empty or
        // element extracted
        NULL_CURRENT.error("ELIST2_ITERATOR::extract", ABORT);
      }
#endif

      if (list->singleton()) {
        // Special case where we do need to change the iterator.
        prev = next = list->last = nullptr;
      } else {
        prev->next = next; // remove from list
        next->prev = prev;

        if (current == list->last) {
          list->last = prev;
          ex_current_was_last = true;
        } else {
          ex_current_was_last = false;
        }
      }
      // Always set ex_current_was_cycle_pt so an add/forward will work in a loop.
      ex_current_was_cycle_pt = (current == cycle_pt);
      extracted_link = current;
      extracted_link->next = nullptr; // for safety
      extracted_link->prev = nullptr; // for safety
      current = nullptr;
      return extracted_link;
    } // remove from list
      /***********************************************************************
     *              ELIST2_ITERATOR::move_to_first()
     *
     *  Move current so that it is set to the start of the list.
     *  Return data just in case anyone wants it.
     **********************************************************************/
     // go to start of list
    T *move_to_first() {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::move_to_first", ABORT);
      }
#endif

      current = list->First();
      prev = list->last;
      next = current ? current->next : nullptr;
      return current;
    }
    /***********************************************************************
   *              ELIST2_ITERATOR::move_to_last()
   *
   *  Move current so that it is set to the end of the list.
   *  Return data just in case anyone wants it.
   **********************************************************************/
    T *move_to_last() {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::move_to_last", ABORT);
      }
#endif

      current = list->last;
      prev = current ? current->prev : nullptr;
      next = current ? current->next : nullptr;
      return current;
    } // go to end of list
      /***********************************************************************
     *              ELIST2_ITERATOR::mark_cycle_pt()
     *
     *  Remember the current location so that we can tell whether we've returned
     *  to this point later.
     *
     *  If the current point is deleted either now, or in the future, the cycle
     *  point will be set to the next item which is set to current.  This could be
     *  by a forward, add_after_then_move or add_after_then_move.
     **********************************************************************/
    void mark_cycle_pt() {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::mark_cycle_pt", ABORT);
      }
#endif

      if (current) {
        cycle_pt = current;
      } else {
        ex_current_was_cycle_pt = true;
      }
      started_cycling = false;
    } // remember current

    bool empty() const { // is list empty?
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::empty", ABORT);
      }
#endif
      return list->empty();
    }

    bool current_extracted() const { // current extracted?
      return !current;
    }
    /***********************************************************************
   *              ELIST2_ITERATOR::at_first()
   *
   *  Are we at the start of the list?
   *
   **********************************************************************/
    bool at_first() const {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::at_first", ABORT);
      }
#endif

      // we're at a deleted
      return ((list->empty()) || (current == list->First()) ||
        ((current == nullptr) && (prev == list->last) && // NON-last pt between
          !ex_current_was_last));                         // first and last
    } // Current is first?
      /***********************************************************************
     *              ELIST2_ITERATOR::at_last()
     *
     *  Are we at the end of the list?
     *
     **********************************************************************/
    bool at_last() const {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::at_last", ABORT);
      }
#endif

      // we're at a deleted
      return ((list->empty()) || (current == list->last) ||
        ((current == nullptr) && (prev == list->last) && // last point between
          ex_current_was_last));                          // first and last
    } // Current is last?
      /***********************************************************************
     *              ELIST2_ITERATOR::cycled_list()
     *
     *  Have we returned to the cycle_pt since it was set?
     *
     **********************************************************************/
    bool cycled_list() const {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::cycled_list", ABORT);
      }
#endif

      return ((list->empty()) || ((current == cycle_pt) && started_cycling));
    } // Completed a cycle?
      /***********************************************************************
     *              ELIST2_ITERATOR::add_to_end
     *
     *  Add a new element to the end of the list without moving the iterator.
     *  This is provided because a single linked list cannot move to the last as
     *  the iterator couldn't set its prev pointer.  Adding to the end is
     *  essential for implementing
                  queues.
    **********************************************************************/
    void add_to_end(            // add at end &
      T *new_element) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::add_to_end", ABORT);
      }
      if (!new_element) {
        BAD_PARAMETER.error("ELIST2_ITERATOR::add_to_end", ABORT, "new_element is nullptr");
      }
      if (new_element->next) {
        STILL_LINKED.error("ELIST2_ITERATOR::add_to_end", ABORT);
      }
#endif

      if (this->at_last()) {
        this->add_after_stay_put(new_element);
      } else {
        if (this->at_first()) {
          this->add_before_stay_put(new_element);
          list->last = new_element;
        } else { // Iteratr is elsewhere
          new_element->next = list->last->next;
          new_element->prev = list->last;
          list->last->next->prev = new_element;
          list->last->next = new_element;
          list->last = new_element;
        }
      }
    } // don't move
      /***********************************************************************
     *              ELIST2_ITERATOR::exchange()
     *
     *  Given another iterator, whose current element is a different element on
     *  the same list list OR an element of another list, exchange the two current
     *  elements.  On return, each iterator points to the element which was the
     *  other iterators current on entry.
     *  (This function hasn't been in-lined because its a bit big!)
     **********************************************************************/
    void exchange(                  // positions of 2 links
      Iterator *other_it) { // other iterator
      constexpr ERRCODE DONT_EXCHANGE_DELETED("Can't exchange deleted elements of lists");

      T *old_current;

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
    } // other iterator

      //# elements in list
    int32_t length() const {
      return list->length();
    }
    /***********************************************************************
   *              ELIST2_ITERATOR::sort()
   *
   *  Sort the elements of the list, then reposition at the start.
   *
   **********************************************************************/
    void sort(          // sort elements
      int comparator( // comparison routine
        const T *, const T *)) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("ELIST2_ITERATOR::sort", ABORT);
      }
#endif

      list->sort(comparator);
      move_to_first();
    }

  private:
    // Don't use the following constructor.
    Iterator() = delete;
  };
  using ITERATOR = Iterator; // compat

private:
  T *last = nullptr; // End of list
  //(Points to head)
  T *First() { // return first
    return last ? last->next : nullptr;
  }

public:
  ~IntrusiveList() {
    clear();
  }

  /* delete elements */
  void clear() {
    internal_clear();
  }

  /* Become a deep copy of src_list */
  template <typename U>
  void deep_copy(const U *src_list, T *(*copier)(const T *)) {
    Iterator from_it(const_cast<U *>(src_list));
    Iterator to_it(this);

    for (from_it.mark_cycle_pt(); !from_it.cycled_list(); from_it.forward())
      to_it.add_after_then_move((*copier)(from_it.data()));
  }

  /***********************************************************************
   *              IntrusiveList::internal_clear
   *
   *  Used by the destructor and the "clear" member function of derived list
   *  classes to destroy all the elements on the list.
   *  The calling function passes a "zapper" function which can be called to
   *  delete each element of the list, regardless of its derived type.  This
   *  technique permits a generic clear function to destroy elements of
   *  different derived types correctly, without requiring virtual functions and
   *  the consequential memory overhead.
   **********************************************************************/

   // destroy all links
  void internal_clear() {
    // ptr to zapper functn
    T *ptr;
    T *next;

    if (!empty()) {
      ptr = last->next;     // set to first
      last->next = nullptr; // break circle
      last = nullptr;       // set list empty
      while (ptr) {
        next = ptr->next;
        delete ptr;
        ptr = next;
      }
    }
  }

  bool empty() const { // is list empty?
    return !last;
  }

  bool singleton() const {
    return last ? (last == last->next) : false;
  }

  void shallow_copy(       // dangerous!!
    IntrusiveList *from_list) { // beware destructors!!
    last = from_list->last;
  }

  /***********************************************************************
 *              IntrusiveList::assign_to_sublist
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
  void assign_to_sublist(        // to this list
    Iterator *start_it, // from list start
    Iterator *end_it);  // from list end

  // # elements in list
  int32_t length() const {
    int32_t count = 0;
    if (last != nullptr) {
      count = 1;
      for (auto it = last->next; it != last; it = it->next) {
        count++;
      }
    }
    return count;
  }
  /***********************************************************************
 *              IntrusiveList::sort
 *
 *  Sort elements on list
 *  NB If you don't like the const declarations in the comparator, coerce yours:
 *   (int (*)(const void *, const void *)
 **********************************************************************/
  void sort(          // sort elements
    int comparator( // comparison routine
      const T *, const T *)) {
    // Allocate an array of pointers, one per list element.
    auto count = length();
    if (count > 0) {
      // ptr array to sort
      std::vector<T *> base;
      base.reserve(count);

      Iterator it(this);

      // Extract all elements, putting the pointers in the array.
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        base.push_back(it.extract());
      }

      // Sort the pointer array.
      std::sort(base.begin(), base.end(),
        // all current comparators return -1,0,1, so we handle this correctly for std::sort
        [&](auto &&l, auto &&r) {return comparator(l, r) < 0; });

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
  void add_sorted(int comparator(const T *, const T *), T *new_link) {
    // Check for adding at the end.
    if (last == nullptr || comparator(last, new_link) < 0) {
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
      Iterator it(this);
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        auto link = it.data();
        if (comparator(link, new_link) > 0) {
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
};

template <typename CLASSNAME>
using ELIST2 = IntrusiveList<CLASSNAME>;

// add TESS_API?
// move templated lists to public include dirs?
#define ELIST2IZEH(T)                           \
  class T##_LIST : public IntrusiveList<T> {   \
  public:                                               \
    using IntrusiveList<T>::IntrusiveList;                    \
  };                                                    \
  class T##_IT : public IntrusiveList<T>::Iterator { \
  public:                                               \
    using base = IntrusiveList<T>::Iterator;           \
    using base::base;                                   \
  };

} // namespace tesseract

#endif
