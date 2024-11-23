/**********************************************************************
 * File:        clst.h  (Formerly clist.h)
 * Description: CONS cell list module include file.
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

#ifndef CLST_H
#define CLST_H

#include "lsterr.h"
#include "serialis.h"

#include <algorithm>
#include <cstdio>

namespace tesseract {

/**********************************************************************
 * CLASS - CLIST
 *
 * Generic list class for singly linked CONS cell lists
 **********************************************************************/

template <typename T>
class ConsList {
  friend class Link;

public:
  /**********************************************************************
   *              CLASS - Link
   *
   *              Generic link class for singly linked CONS cell lists
   *
   *  Note:  No destructor - elements are assumed to be destroyed EITHER after
   *  they have been extracted from a list OR by the ConsList destructor which
   *  walks the list.
   **********************************************************************/
  struct Link {
    Link *next{};
    T *data{};

    Link() = default;
    Link(const Link &) = delete;
    void operator=(const Link &) = delete;
  };

  /***********************************************************************
   *              CLASS - Iterator
   *
   *              Generic iterator class for singly linked lists with embedded
   *links
   **********************************************************************/
  class Iterator {
    ConsList *list;                  // List being iterated
    Link *prev;             // prev element
    Link *current;          // current element
    Link *next;             // next element
    Link *cycle_pt;         // point we are cycling the list to.
    bool ex_current_was_last;     // current extracted was end of list
    bool ex_current_was_cycle_pt; // current extracted was cycle point
    bool started_cycling;         // Have we moved off the start?

    /***********************************************************************
     *              Iterator::extract_sublist()
     *
     *  This is a private member, used only by ConsList::assign_to_sublist.
     *  Given another iterator for the same list, extract the links from THIS to
     *  OTHER inclusive, link them into a new circular list, and return a
     *  pointer to the last element.
     *  (Can't inline this function because it contains a loop)
     **********************************************************************/
    Link *extract_sublist(  // from this current
      Iterator *other_it) {              // to other current
      Iterator temp_it = *this;

      constexpr ERRCODE BAD_SUBLIST("Can't find sublist end point in original list");
#ifndef NDEBUG
      constexpr ERRCODE BAD_EXTRACTION_PTS("Can't extract sublist from points on different lists");
      constexpr ERRCODE DONT_EXTRACT_DELETED("Can't extract a sublist marked by deleted points");

      if (list != other_it->list)
        BAD_EXTRACTION_PTS.error("Iterator.extract_sublist", ABORT);
      if (list->empty())
        EMPTY_LIST.error("Iterator::extract_sublist", ABORT);

      if (!current || !other_it->current)
        DONT_EXTRACT_DELETED.error("Iterator.extract_sublist", ABORT);
#endif

      ex_current_was_last = other_it->ex_current_was_last = false;
      ex_current_was_cycle_pt = false;
      other_it->ex_current_was_cycle_pt = false;

      temp_it.mark_cycle_pt();
      do {                         // walk sublist
        if (temp_it.cycled_list()) { // can't find end pt
          BAD_SUBLIST.error("Iterator.extract_sublist", ABORT);
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
      } while (temp_it.prev != other_it->current);

      // circularise sublist
      other_it->current->next = current;
      auto end_of_new_list = other_it->current;

      // sublist = whole list
      if (prev == other_it->current) {
        list->last = nullptr;
        prev = current = next = nullptr;
        other_it->prev = other_it->current = other_it->next = nullptr;
      } else {
        prev->next = other_it->next;
        current = other_it->current = nullptr;
        next = other_it->next;
        other_it->prev = prev;
      }
      return end_of_new_list;
    }

  public:
    Iterator() { // constructor
      list = nullptr;
    } // unassigned list

  /***********************************************************************
   *              Iterator::Iterator
   *
   *  CONSTRUCTOR - set iterator to specified list;
   **********************************************************************/
    Iterator( // constructor
      ConsList *list_to_iterate) {
      set_to_list(list_to_iterate);
    }

    /***********************************************************************
     *              Iterator::set_to_list
     *
     *  (Re-)initialise the iterator to point to the start of the list_to_iterate
     *  over.
     **********************************************************************/
    void set_to_list( // change list
      ConsList *list_to_iterate) {
      list = list_to_iterate;
      prev = list->last;
      current = list->First();
      next = current != nullptr ? current->next : nullptr;
      cycle_pt = nullptr; // await explicit set
      started_cycling = false;
      ex_current_was_last = false;
      ex_current_was_cycle_pt = false;
    }

    /***********************************************************************
     *              Iterator::add_after_then_move
     *
     *  Add a new element to the list after the current element and move the
     *  iterator to the new element.
     **********************************************************************/
    void add_after_then_move( // add after current &
      T *new_data) {
#ifndef NDEBUG
      if (!new_data) {
        BAD_PARAMETER.error("Iterator::add_after_then_move", ABORT, "new_data is nullptr");
      }
#endif

      auto new_element = new Link;
      new_element->data = new_data;

      if (list->empty()) {
        new_element->next = new_element;
        list->last = new_element;
        prev = next = new_element;
      } else {
        new_element->next = next;

        if (current) { // not extracted
          current->next = new_element;
          prev = current;
          if (current == list->last) {
            list->last = new_element;
          }
        } else { // current extracted
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
    }      // move to new

    /***********************************************************************
     *              Iterator::add_after_stay_put
     *
     *  Add a new element to the list after the current element but do not move
     *  the iterator to the new element.
     **********************************************************************/
    void add_after_stay_put( // add after current &
      T *new_data) {
#ifndef NDEBUG
      if (!new_data) {
        BAD_PARAMETER.error("Iterator::add_after_stay_put", ABORT, "new_data is nullptr");
      }
#endif

      auto new_element = new Link;
      new_element->data = new_data;

      if (list->empty()) {
        new_element->next = new_element;
        list->last = new_element;
        prev = next = new_element;
        ex_current_was_last = false;
        current = nullptr;
      } else {
        new_element->next = next;

        if (current) { // not extracted
          current->next = new_element;
          if (prev == current) {
            prev = new_element;
          }
          if (current == list->last) {
            list->last = new_element;
          }
        } else { // current extracted
          prev->next = new_element;
          if (ex_current_was_last) {
            list->last = new_element;
            ex_current_was_last = false;
          }
        }
        next = new_element;
      }
    }     // stay at current

    /***********************************************************************
     *              Iterator::add_before_then_move
     *
     *  Add a new element to the list before the current element and move the
     *  iterator to the new element.
     **********************************************************************/
    void add_before_then_move( // add before current &
      T *new_data) {
#ifndef NDEBUG
      if (!new_data) {
        BAD_PARAMETER.error("Iterator::add_before_then_move", ABORT, "new_data is nullptr");
      }
#endif

      auto new_element = new Link;
      new_element->data = new_data;

      if (list->empty()) {
        new_element->next = new_element;
        list->last = new_element;
        prev = next = new_element;
      } else {
        prev->next = new_element;
        if (current) { // not extracted
          new_element->next = current;
          next = current;
        } else { // current extracted
          new_element->next = next;
          if (ex_current_was_last) {
            list->last = new_element;
          }
          if (ex_current_was_cycle_pt) {
            cycle_pt = new_element;
          }
        }
      }
      current = new_element;
    }       // move to new

    /***********************************************************************
     *              Iterator::add_before_stay_put
     *
     *  Add a new element to the list before the current element but don't move the
     *  iterator to the new element.
     **********************************************************************/
    void add_before_stay_put( // add before current &
      T *new_data) {
#ifndef NDEBUG
      if (!new_data) {
        BAD_PARAMETER.error("Iterator::add_before_stay_put", ABORT, "new_data is nullptr");
      }
#endif

      auto new_element = new Link;
      new_element->data = new_data;

      if (list->empty()) {
        new_element->next = new_element;
        list->last = new_element;
        prev = next = new_element;
        ex_current_was_last = true;
        current = nullptr;
      } else {
        prev->next = new_element;
        if (current) { // not extracted
          new_element->next = current;
          if (next == current) {
            next = new_element;
          }
        } else { // current extracted
          new_element->next = next;
          if (ex_current_was_last) {
            list->last = new_element;
          }
        }
        prev = new_element;
      }
    }      // stay at current

    /***********************************************************************
     *              Iterator::add_list_after
     *
     *  Insert another list to this list after the current element but don't move
     *the
     *  iterator.
     **********************************************************************/
    void add_list_after(     // add a list &
      ConsList *list_to_add) {
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
            if (current == list->last) {
              list->last = list_to_add->last;
            }
            list_to_add->last->next = next;
            next = current->next;
          } else { // current extracted
            prev->next = list_to_add->First();
            if (ex_current_was_last) {
              list->last = list_to_add->last;
              ex_current_was_last = false;
            }
            list_to_add->last->next = next;
            next = prev->next;
          }
        }
        list_to_add->last = nullptr;
      }
    } // stay at current

    /***********************************************************************
     *              Iterator::add_list_before
     *
     *  Insert another list to this list before the current element. Move the
     *  iterator to the start of the inserted elements
     *  iterator.
     **********************************************************************/
    void add_list_before(    // add a list &
      ConsList *list_to_add) {
      if (!list_to_add->empty()) {
        if (list->empty()) {
          list->last = list_to_add->last;
          prev = list->last;
          current = list->First();
          next = current->next;
          ex_current_was_last = false;
        } else {
          prev->next = list_to_add->First();
          if (current) { // not extracted
            list_to_add->last->next = current;
          } else { // current extracted
            list_to_add->last->next = next;
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
      if (!list) {
        NO_LIST.error("Iterator::data", ABORT);
      }
#endif
      return current->data;
    }

    /***********************************************************************
     *              Iterator::data_relative
     *
     *  Return the data pointer to the element "offset" elements from current.
     *  "offset" must not be less than -1.
     *  (This function can't be INLINEd because it contains a loop)
     **********************************************************************/
    T *data_relative(  // get data + or - ...
      int8_t offset) {                 // offset from current
      Link *ptr;

#ifndef NDEBUG
      if (!list)
        NO_LIST.error("Iterator::data_relative", ABORT);
      if (list->empty())
        EMPTY_LIST.error("Iterator::data_relative", ABORT);
      if (offset < -1)
        BAD_PARAMETER.error("Iterator::data_relative", ABORT, "offset < -l");
#endif

      if (offset == -1) {
        ptr = prev;
      } else {
        for (ptr = current ? current : prev; offset-- > 0; ptr = ptr->next) {
          ;
        }
      }

      return ptr->data;
    }

    /***********************************************************************
     *              Iterator::forward
     *
     *  Move the iterator to the next element of the list.
     *  REMEMBER: ALL LISTS ARE CIRCULAR.
     **********************************************************************/
    T *forward() {
      if (list->empty()) {
        return nullptr;
      }

      if (current) { // not removed so
        // set previous
        prev = current;
        started_cycling = true;
        // In case next is deleted by another iterator, get next from current.
        current = current->next;
      } else {
        if (ex_current_was_cycle_pt) {
          cycle_pt = next;
        }
        current = next;
      }

      next = current->next;
      return current->data;
    }

    /***********************************************************************
     *              Iterator::extract
     *
     *  Do extraction by removing current from the list, deleting the cons cell
     *  and returning the data to the caller, but NOT updating the iterator.  (So
     *  that any calling loop can do this.)  The iterator's current points to
     *  nullptr.  If the data is to be deleted, this is the callers responsibility.
     **********************************************************************/
    T *extract() {
#ifndef NDEBUG
      if (!current) { // list empty or
        // element extracted
        NULL_CURRENT.error("Iterator::extract", ABORT);
      }
#endif

      if (list->singleton()) {
        // Special case where we do need to change the iterator.
        prev = next = list->last = nullptr;
      } else {
        prev->next = next; // remove from list

        if (current == list->last) {
          list->last = prev;
          ex_current_was_last = true;
        } else {
          ex_current_was_last = false;
        }
      }
      // Always set ex_current_was_cycle_pt so an add/forward will work in a loop.
      ex_current_was_cycle_pt = (current == cycle_pt);
      auto extracted_data = current->data;
      delete (current); // destroy CONS cell
      current = nullptr;
      return extracted_data;
    } // remove from list

    /***********************************************************************
     *              Iterator::move_to_first()
     *
     *  Move current so that it is set to the start of the list.
     *  Return data just in case anyone wants it.
     **********************************************************************/
    T *move_to_first() {
      current = list->First();
      prev = list->last;
      next = current != nullptr ? current->next : nullptr;
      return current != nullptr ? current->data : nullptr;
    } // go to start of list

    /***********************************************************************
     *              Iterator::move_to_last()
     *
     *  Move current so that it is set to the end of the list.
     *  Return data just in case anyone wants it.
     *  (This function can't be INLINEd because it contains a loop)
     **********************************************************************/
    T *move_to_last() {
      while (current != list->last) {
        forward();
      }

      if (current == nullptr) {
        return nullptr;
      } else {
        return current->data;
      }
    }

    /***********************************************************************
     *              Iterator::mark_cycle_pt()
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
        NO_LIST.error("Iterator::mark_cycle_pt", ABORT);
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
      return list->empty();
    }

    bool current_extracted() const { // current extracted?
      return !current;
    }

    /***********************************************************************
     *              Iterator::at_first()
     *
     *  Are we at the start of the list?
     *
     **********************************************************************/
    bool at_first() const {
      // we're at a deleted
      return ((list->empty()) || (current == list->First()) ||
        ((current == nullptr) && (prev == list->last) && // NON-last pt between
          !ex_current_was_last));                         // first and last
    } // Current is first?

    /***********************************************************************
     *              Iterator::at_last()
     *
     *  Are we at the end of the list?
     *
     **********************************************************************/
    bool at_last() const {
      // we're at a deleted
      return ((list->empty()) || (current == list->last) ||
        ((current == nullptr) && (prev == list->last) && // last point between
          ex_current_was_last));                          // first and last
    } // Current is last?

    /***********************************************************************
     *              Iterator::cycled_list()
     *
     *  Have we returned to the cycle_pt since it was set?
     *
     **********************************************************************/
    bool cycled_list() const { // Completed a cycle?
      return ((list->empty()) || ((current == cycle_pt) && started_cycling));
    }

    /***********************************************************************
     *              Iterator::add_to_end
     *
     *  Add a new element to the end of the list without moving the iterator.
     *  This is provided because a single linked list cannot move to the last as
     *  the iterator couldn't set its prev pointer.  Adding to the end is
     *  essential for implementing
                  queues.
    **********************************************************************/
    void add_to_end(  // element to add
      T *new_data) {
#ifndef NDEBUG
      if (!list) {
        NO_LIST.error("Iterator::add_to_end", ABORT);
      }
      if (!new_data) {
        BAD_PARAMETER.error("Iterator::add_to_end", ABORT, "new_data is nullptr");
      }
#endif

      if (this->at_last()) {
        this->add_after_stay_put(new_data);
      } else {
        if (this->at_first()) {
          this->add_before_stay_put(new_data);
          list->last = prev;
        } else { // Iteratr is elsewhere
          auto new_element = new Link;
          new_element->data = new_data;

          new_element->next = list->last->next;
          list->last->next = new_element;
          list->last = new_element;
        }
      }
    }

    /***********************************************************************
     *              Iterator::exchange()
     *
     *  Given another iterator, whose current element is a different element on
     *  the same list list OR an element of another list, exchange the two current
     *  elements.  On return, each iterator points to the element which was the
     *  other iterators current on entry.
     *  (This function hasn't been in-lined because its a bit big!)
     **********************************************************************/
    void exchange(                 // positions of 2 links
      Iterator *other_it) { // other iterator
      constexpr ERRCODE DONT_EXCHANGE_DELETED("Can't exchange deleted elements of lists");

      /* Do nothing if either list is empty or if both iterators reference the same
    link */

      if ((list->empty()) || (other_it->list->empty()) || (current == other_it->current)) {
        return;
      }

      /* Error if either current element is deleted */

      if (!current || !other_it->current) {
        DONT_EXCHANGE_DELETED.error("Iterator.exchange", ABORT);
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
            current->next = other_it->current;
            other_it->next = other_it->current;
            prev = current;
          } else { // this before other
            prev->next = other_it->current;
            current->next = other_it->next;
            other_it->current->next = current;
            next = current;
            other_it->prev = other_it->current;
          }
        }
      } else { // no overlap
        prev->next = other_it->current;
        current->next = other_it->next;
        other_it->prev->next = current;
        other_it->current->next = next;
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

      auto old_current = current;
      current = other_it->current;
      other_it->current = old_current;
    }

    /***********************************************************************
     *              Iterator::length()
     *
     *  Return the length of the list
     *
     **********************************************************************/
    int32_t length() const {
      return list->length();
    }

    /***********************************************************************
     *              Iterator::sort()
     *
     *  Sort the elements of the list, then reposition at the start.
     *
     **********************************************************************/
    void sort(     // sort elements
      int comparator(               // comparison routine
        const T *, const T *)) {
      list->sort(comparator);
      move_to_first();
    }
  };
  using ITERATOR = Iterator; // compat

private:
  Link *last = nullptr; // End of list

  //(Points to head)
  Link *First() { // return first
    return last != nullptr ? last->next : nullptr;
  }

  const Link *First() const { // return first
    return last != nullptr ? last->next : nullptr;
  }

public:
  ~ConsList() { // destructor
    shallow_clear();
  }

  /***********************************************************************
   *              ConsList::internal_deep_clear
   *
   *  Used by the "deep_clear" member function of derived list
   *  classes to destroy all the elements on the list.
   *  The calling function passes a "zapper" function which can be called to
   *  delete each data element of the list, regardless of its class.  This
   *  technique permits a generic clear function to destroy elements of
   *  different derived types correctly, without requiring virtual functions and
   *  the consequential memory overhead.
   **********************************************************************/
  void internal_deep_clear() {    // ptr to zapper functn
    if (!empty()) {
      auto ptr = last->next;     // set to first
      last->next = nullptr; // break circle
      last = nullptr;       // set list empty
      while (ptr) {
        auto next = ptr->next;
        delete ptr->data;
        delete (ptr);
        ptr = next;
      }
    }
  }
  void deep_clear() {
    internal_deep_clear();
  }

  /***********************************************************************
   *              ConsList::shallow_clear
   *
   *  Used by the destructor and the "shallow_clear" member function of derived
   *  list classes to destroy the list.
   *  The data elements are NOT destroyed.
   *
   **********************************************************************/
  void shallow_clear() { // destroy all links
    if (!empty()) {
      auto ptr = last->next;     // set to first
      last->next = nullptr; // break circle
      last = nullptr;       // set list empty
      while (ptr) {
        auto next = ptr->next;
        delete (ptr);
        ptr = next;
      }
    }
  }

  bool empty() const { // is list empty?
    return !last;
  }

  bool singleton() const {
    return last != nullptr ? (last == last->next) : false;
  }

  void shallow_copy(      // dangerous!!
    ConsList *from_list) { // beware destructors!!
    last = from_list->last;
  }

  /***********************************************************************
   *              ConsList::assign_to_sublist
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
  void assign_to_sublist(  // to this list
    Iterator *start_it,  // from list start
    Iterator *end_it) {  // from list end
    constexpr ERRCODE LIST_NOT_EMPTY("Destination list must be empty before extracting a sublist");

    if (!empty()) {
      LIST_NOT_EMPTY.error("ConsList.assign_to_sublist", ABORT);
    }

    last = start_it->extract_sublist(end_it);
  }

  int32_t length() const { //# elements in list
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
   *              ConsList::sort
   *
   *  Sort elements on list
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

  // Assuming list has been sorted already, insert new_data to
  // keep the list sorted according to the same comparison function.
  // Comparison function is the same as used by sort, i.e. uses double
  // indirection. Time is O(1) to add to beginning or end.
  // Time is linear to add pre-sorted items to an empty list.
  // If unique, then don't add duplicate entries.
  // Returns true if the element was added to the list.
  bool add_sorted(int comparator(const T *, const T *), bool unique, T *new_data) {
    // Check for adding at the end.
    if (last == nullptr || comparator(last->data, new_data) < 0) {
      auto *new_element = new Link;
      new_element->data = new_data;
      if (last == nullptr) {
        new_element->next = new_element;
      } else {
        new_element->next = last->next;
        last->next = new_element;
      }
      last = new_element;
      return true;
    } else if (!unique || last->data != new_data) {
      // Need to use an iterator.
      Iterator it(this);
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        auto data = it.data();
        if (data == new_data && unique) {
          return false;
        }
        if (comparator(data, new_data) > 0) {
          break;
        }
      }
      if (it.cycled_list()) {
        it.add_to_end(new_data);
      } else {
        it.add_before_then_move(new_data);
      }
      return true;
    }
    return false;
  }

  // Assuming that the minuend and subtrahend are already sorted with
  // the same comparison function, shallow clears this and then copies
  // the set difference minuend - subtrahend to this, being the elements
  // of minuend that do not compare equal to anything in subtrahend.
  // If unique is true, any duplicates in minuend are also eliminated.
  void set_subtract(int comparator(const T *, const T *), bool unique, ConsList *minuend,
    ConsList *subtrahend) {
    shallow_clear();
    Iterator m_it(minuend);
    Iterator s_it(subtrahend);
    // Since both lists are sorted, finding the subtras that are not
    // minus is a case of a parallel iteration.
    for (m_it.mark_cycle_pt(); !m_it.cycled_list(); m_it.forward()) {
      auto minu = m_it.data();
      T *subtra = nullptr;
      if (!s_it.empty()) {
        subtra = s_it.data();
        while (!s_it.at_last() && comparator(subtra, minu) < 0) {
          s_it.forward();
          subtra = s_it.data();
        }
      }
      if (subtra == nullptr || comparator(subtra, minu) != 0) {
        add_sorted(comparator, unique, minu);
      }
    }
  }
};

#define CLISTIZEH(T)                          \
  class T##_CLIST : public ConsList<T> {      \
    using ConsList<T>::ConsList;              \
  };                                          \
  using T##_C_IT = ConsList<T>::Iterator;

} // namespace tesseract

#endif
