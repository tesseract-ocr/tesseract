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

#include "list.h"
#include "lsterr.h"
#include "serialis.h"

#include <cstdio>

namespace tesseract {

class CLIST_ITERATOR;

/**********************************************************************
 *              CLASS - CLIST_LINK
 *
 *              Generic link class for singly linked CONS cell lists
 *
 *  Note:  No destructor - elements are assumed to be destroyed EITHER after
 *  they have been extracted from a list OR by the CLIST destructor which
 *  walks the list.
 **********************************************************************/

class CLIST_LINK {
  friend class CLIST_ITERATOR;
  friend class CLIST;

  CLIST_LINK *next;
  void *data;

public:
  CLIST_LINK() { // constructor
    data = next = nullptr;
  }

  CLIST_LINK(const CLIST_LINK &) = delete;
  void operator=(const CLIST_LINK &) = delete;
};

/**********************************************************************
 * CLASS - CLIST
 *
 * Generic list class for singly linked CONS cell lists
 **********************************************************************/

class TESS_API CLIST {
  friend class CLIST_ITERATOR;

  CLIST_LINK *last = nullptr; // End of list

  //(Points to head)
  CLIST_LINK *First() { // return first
    return last != nullptr ? last->next : nullptr;
  }

  const CLIST_LINK *First() const { // return first
    return last != nullptr ? last->next : nullptr;
  }

public:
  ~CLIST() { // destructor
    shallow_clear();
  }

  void internal_deep_clear(    // destroy all links
      void (*zapper)(void *)); // ptr to zapper functn

  void shallow_clear(); // clear list but don't
  // delete data elements

  bool empty() const { // is list empty?
    return !last;
  }

  bool singleton() const {
    return last != nullptr ? (last == last->next) : false;
  }

  void shallow_copy(      // dangerous!!
      CLIST *from_list) { // beware destructors!!
    last = from_list->last;
  }

  void assign_to_sublist(       // to this list
      CLIST_ITERATOR *start_it, // from list start
      CLIST_ITERATOR *end_it);  // from list end

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

  void sort(          // sort elements
      int comparator( // comparison routine
          const void *, const void *));

  // Assuming list has been sorted already, insert new_data to
  // keep the list sorted according to the same comparison function.
  // Comparison function is the same as used by sort, i.e. uses double
  // indirection. Time is O(1) to add to beginning or end.
  // Time is linear to add pre-sorted items to an empty list.
  // If unique, then don't add duplicate entries.
  // Returns true if the element was added to the list.
  bool add_sorted(int comparator(const void *, const void *), bool unique, void *new_data);

  // Assuming that the minuend and subtrahend are already sorted with
  // the same comparison function, shallow clears this and then copies
  // the set difference minuend - subtrahend to this, being the elements
  // of minuend that do not compare equal to anything in subtrahend.
  // If unique is true, any duplicates in minuend are also eliminated.
  void set_subtract(int comparator(const void *, const void *), bool unique, CLIST *minuend,
                    CLIST *subtrahend);
};

/***********************************************************************
 *              CLASS - CLIST_ITERATOR
 *
 *              Generic iterator class for singly linked lists with embedded
 *links
 **********************************************************************/

class TESS_API CLIST_ITERATOR {
  friend void CLIST::assign_to_sublist(CLIST_ITERATOR *, CLIST_ITERATOR *);

  CLIST *list;                  // List being iterated
  CLIST_LINK *prev;             // prev element
  CLIST_LINK *current;          // current element
  CLIST_LINK *next;             // next element
  CLIST_LINK *cycle_pt;         // point we are cycling the list to.
  bool ex_current_was_last;     // current extracted was end of list
  bool ex_current_was_cycle_pt; // current extracted was cycle point
  bool started_cycling;         // Have we moved off the start?

  CLIST_LINK *extract_sublist(   // from this current...
      CLIST_ITERATOR *other_it); // to other current

public:
  CLIST_ITERATOR() { // constructor
    list = nullptr;
  } // unassigned list

  CLIST_ITERATOR( // constructor
      CLIST *list_to_iterate);

  void set_to_list( // change list
      CLIST *list_to_iterate);

  void add_after_then_move( // add after current &
      void *new_data);      // move to new

  void add_after_stay_put( // add after current &
      void *new_data);     // stay at current

  void add_before_then_move( // add before current &
      void *new_data);       // move to new

  void add_before_stay_put( // add before current &
      void *new_data);      // stay at current

  void add_list_after(     // add a list &
      CLIST *list_to_add); // stay at current

  void add_list_before(    // add a list &
      CLIST *list_to_add); // move to it 1st item

  void *data() { // get current data
#ifndef NDEBUG
    if (!list) {
      NO_LIST.error("CLIST_ITERATOR::data", ABORT);
    }
#endif
    return current->data;
  }

  void *data_relative( // get data + or - ...
      int8_t offset);  // offset from current

  void *forward(); // move to next element

  void *extract(); // remove from list

  void *move_to_first(); // go to start of list

  void *move_to_last(); // go to end of list

  void mark_cycle_pt(); // remember current

  bool empty() const { // is list empty?
    return list->empty();
  }

  bool current_extracted() const { // current extracted?
    return !current;
  }

  bool at_first() const; // Current is first?

  bool at_last() const; // Current is last?

  bool cycled_list() const; // Completed a cycle?

  void add_to_end(     // add at end &
      void *new_data); // don't move

  void exchange(                 // positions of 2 links
      CLIST_ITERATOR *other_it); // other iterator

  int32_t length() const; //# elements in list

  void sort(          // sort elements
      int comparator( // comparison routine
          const void *, const void *));
};

/***********************************************************************
 *              CLIST_ITERATOR::set_to_list
 *
 *  (Re-)initialise the iterator to point to the start of the list_to_iterate
 *  over.
 **********************************************************************/

inline void CLIST_ITERATOR::set_to_list( // change list
    CLIST *list_to_iterate) {
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
 *              CLIST_ITERATOR::CLIST_ITERATOR
 *
 *  CONSTRUCTOR - set iterator to specified list;
 **********************************************************************/

inline CLIST_ITERATOR::CLIST_ITERATOR(CLIST *list_to_iterate) {
  set_to_list(list_to_iterate);
}

/***********************************************************************
 *              CLIST_ITERATOR::add_after_then_move
 *
 *  Add a new element to the list after the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void CLIST_ITERATOR::add_after_then_move( // element to add
    void *new_data) {
#ifndef NDEBUG
  if (!new_data) {
    BAD_PARAMETER.error("CLIST_ITERATOR::add_after_then_move", ABORT, "new_data is nullptr");
  }
#endif

  auto new_element = new CLIST_LINK;
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
}

/***********************************************************************
 *              CLIST_ITERATOR::add_after_stay_put
 *
 *  Add a new element to the list after the current element but do not move
 *  the iterator to the new element.
 **********************************************************************/

inline void CLIST_ITERATOR::add_after_stay_put( // element to add
    void *new_data) {
#ifndef NDEBUG
  if (!new_data) {
    BAD_PARAMETER.error("CLIST_ITERATOR::add_after_stay_put", ABORT, "new_data is nullptr");
  }
#endif

  auto new_element = new CLIST_LINK;
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
}

/***********************************************************************
 *              CLIST_ITERATOR::add_before_then_move
 *
 *  Add a new element to the list before the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void CLIST_ITERATOR::add_before_then_move( // element to add
    void *new_data) {
#ifndef NDEBUG
  if (!new_data) {
    BAD_PARAMETER.error("CLIST_ITERATOR::add_before_then_move", ABORT, "new_data is nullptr");
  }
#endif

  auto new_element = new CLIST_LINK;
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
}

/***********************************************************************
 *              CLIST_ITERATOR::add_before_stay_put
 *
 *  Add a new element to the list before the current element but don't move the
 *  iterator to the new element.
 **********************************************************************/

inline void CLIST_ITERATOR::add_before_stay_put( // element to add
    void *new_data) {
#ifndef NDEBUG
  if (!new_data) {
    BAD_PARAMETER.error("CLIST_ITERATOR::add_before_stay_put", ABORT, "new_data is nullptr");
  }
#endif

  auto new_element = new CLIST_LINK;
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
}

/***********************************************************************
 *              CLIST_ITERATOR::add_list_after
 *
 *  Insert another list to this list after the current element but don't move
 *the
 *  iterator.
 **********************************************************************/

inline void CLIST_ITERATOR::add_list_after(CLIST *list_to_add) {
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
}

/***********************************************************************
 *              CLIST_ITERATOR::add_list_before
 *
 *  Insert another list to this list before the current element. Move the
 *  iterator to the start of the inserted elements
 *  iterator.
 **********************************************************************/

inline void CLIST_ITERATOR::add_list_before(CLIST *list_to_add) {
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
}

/***********************************************************************
 *              CLIST_ITERATOR::extract
 *
 *  Do extraction by removing current from the list, deleting the cons cell
 *  and returning the data to the caller, but NOT updating the iterator.  (So
 *  that any calling loop can do this.)  The iterator's current points to
 *  nullptr.  If the data is to be deleted, this is the callers responsibility.
 **********************************************************************/

inline void *CLIST_ITERATOR::extract() {
#ifndef NDEBUG
  if (!current) { // list empty or
                  // element extracted
    NULL_CURRENT.error("CLIST_ITERATOR::extract", ABORT);
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
}

/***********************************************************************
 *              CLIST_ITERATOR::move_to_first()
 *
 *  Move current so that it is set to the start of the list.
 *  Return data just in case anyone wants it.
 **********************************************************************/

inline void *CLIST_ITERATOR::move_to_first() {
  current = list->First();
  prev = list->last;
  next = current != nullptr ? current->next : nullptr;
  return current != nullptr ? current->data : nullptr;
}

/***********************************************************************
 *              CLIST_ITERATOR::mark_cycle_pt()
 *
 *  Remember the current location so that we can tell whether we've returned
 *  to this point later.
 *
 *  If the current point is deleted either now, or in the future, the cycle
 *  point will be set to the next item which is set to current.  This could be
 *  by a forward, add_after_then_move or add_after_then_move.
 **********************************************************************/

inline void CLIST_ITERATOR::mark_cycle_pt() {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("CLIST_ITERATOR::mark_cycle_pt", ABORT);
  }
#endif

  if (current) {
    cycle_pt = current;
  } else {
    ex_current_was_cycle_pt = true;
  }
  started_cycling = false;
}

/***********************************************************************
 *              CLIST_ITERATOR::at_first()
 *
 *  Are we at the start of the list?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::at_first() const {
  // we're at a deleted
  return ((list->empty()) || (current == list->First()) ||
          ((current == nullptr) && (prev == list->last) && // NON-last pt between
           !ex_current_was_last));                         // first and last
}

/***********************************************************************
 *              CLIST_ITERATOR::at_last()
 *
 *  Are we at the end of the list?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::at_last() const {
  // we're at a deleted
  return ((list->empty()) || (current == list->last) ||
          ((current == nullptr) && (prev == list->last) && // last point between
           ex_current_was_last));                          // first and last
}

/***********************************************************************
 *              CLIST_ITERATOR::cycled_list()
 *
 *  Have we returned to the cycle_pt since it was set?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::cycled_list() const {
  return ((list->empty()) || ((current == cycle_pt) && started_cycling));
}

/***********************************************************************
 *              CLIST_ITERATOR::length()
 *
 *  Return the length of the list
 *
 **********************************************************************/

inline int32_t CLIST_ITERATOR::length() const {
  return list->length();
}

/***********************************************************************
 *              CLIST_ITERATOR::sort()
 *
 *  Sort the elements of the list, then reposition at the start.
 *
 **********************************************************************/

inline void CLIST_ITERATOR::sort( // sort elements
    int comparator(               // comparison routine
        const void *, const void *)) {
  list->sort(comparator);
  move_to_first();
}

/***********************************************************************
 *              CLIST_ITERATOR::add_to_end
 *
 *  Add a new element to the end of the list without moving the iterator.
 *  This is provided because a single linked list cannot move to the last as
 *  the iterator couldn't set its prev pointer.  Adding to the end is
 *  essential for implementing
              queues.
**********************************************************************/

inline void CLIST_ITERATOR::add_to_end( // element to add
    void *new_data) {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("CLIST_ITERATOR::add_to_end", ABORT);
  }
  if (!new_data) {
    BAD_PARAMETER.error("CLIST_ITERATOR::add_to_end", ABORT, "new_data is nullptr");
  }
#endif

  if (this->at_last()) {
    this->add_after_stay_put(new_data);
  } else {
    if (this->at_first()) {
      this->add_before_stay_put(new_data);
      list->last = prev;
    } else { // Iteratr is elsewhere
      auto new_element = new CLIST_LINK;
      new_element->data = new_data;

      new_element->next = list->last->next;
      list->last->next = new_element;
      list->last = new_element;
    }
  }
}

template <typename CLASSNAME>
class X_CLIST : public CLIST {
public:
  X_CLIST() = default;
  X_CLIST(const X_CLIST &) = delete;
  X_CLIST &operator=(const X_CLIST &) = delete;

  void deep_clear() {
    internal_deep_clear([](void *link) {delete static_cast<CLASSNAME *>(link);});
  }
};

#define CLISTIZEH(CLASSNAME)                                    \
  class CLASSNAME##_CLIST : public X_CLIST<CLASSNAME> {         \
    using X_CLIST<CLASSNAME>::X_CLIST;                          \
  };                                                            \
  struct CLASSNAME##_C_IT : X_ITER<CLIST_ITERATOR, CLASSNAME> { \
    using X_ITER<CLIST_ITERATOR, CLASSNAME>::X_ITER;            \
  };

} // namespace tesseract

#endif
