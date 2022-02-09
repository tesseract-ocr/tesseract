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

#include "list.h"
#include "lsterr.h"
#include "serialis.h"

#include <cstdio>

namespace tesseract {

class ELIST2_ITERATOR;

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
 *              CLASS - ELIST2_LINK
 *
 *              Generic link class for doubly linked lists with embedded links
 *
 *  Note:  No destructor - elements are assumed to be destroyed EITHER after
 *  they have been extracted from a list OR by the ELIST2 destructor which
 *  walks the list.
 **********************************************************************/

class ELIST2_LINK {
  friend class ELIST2_ITERATOR;
  friend class ELIST2;

  ELIST2_LINK *prev;
  ELIST2_LINK *next;

public:
  ELIST2_LINK() { // constructor
    prev = next = nullptr;
  }

  ELIST2_LINK(const ELIST2_LINK &) = delete;

  // The assignment operator is required for WERD.
  void operator=(const ELIST2_LINK &) {
    prev = next = nullptr;
  }
};

/**********************************************************************
 * CLASS - ELIST2
 *
 * Generic list class for doubly linked lists with embedded links
 **********************************************************************/

class TESS_API ELIST2 {
  friend class ELIST2_ITERATOR;

  ELIST2_LINK *last = nullptr; // End of list
  //(Points to head)
  ELIST2_LINK *First() { // return first
    return last ? last->next : nullptr;
  }

public:
  // destroy all links
  void internal_clear(void (*zapper)(void *));

  bool empty() const { // is list empty?
    return !last;
  }

  bool singleton() const {
    return last ? (last == last->next) : false;
  }

  void shallow_copy(       // dangerous!!
      ELIST2 *from_list) { // beware destructors!!
    last = from_list->last;
  }

  // ptr to copier functn
  void internal_deep_copy(ELIST2_LINK *(*copier)(ELIST2_LINK *),
                          const ELIST2 *list); // list being copied

  void assign_to_sublist(        // to this list
      ELIST2_ITERATOR *start_it, // from list start
      ELIST2_ITERATOR *end_it);  // from list end

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

  void sort(          // sort elements
      int comparator( // comparison routine
          const void *, const void *));

  // Assuming list has been sorted already, insert new_link to
  // keep the list sorted according to the same comparison function.
  // Comparison function is the same as used by sort, i.e. uses double
  // indirection. Time is O(1) to add to beginning or end.
  // Time is linear to add pre-sorted items to an empty list.
  void add_sorted(int comparator(const void *, const void *), ELIST2_LINK *new_link);
};

/***********************************************************************
 *              CLASS - ELIST2_ITERATOR
 *
 *              Generic iterator class for doubly linked lists with embedded
 *links
 **********************************************************************/

class TESS_API ELIST2_ITERATOR {
  friend void ELIST2::assign_to_sublist(ELIST2_ITERATOR *, ELIST2_ITERATOR *);

  ELIST2 *list;                 // List being iterated
  ELIST2_LINK *prev;            // prev element
  ELIST2_LINK *current;         // current element
  ELIST2_LINK *next;            // next element
  ELIST2_LINK *cycle_pt;        // point we are cycling the list to.
  bool ex_current_was_last;     // current extracted was end of list
  bool ex_current_was_cycle_pt; // current extracted was cycle point
  bool started_cycling;         // Have we moved off the start?

  ELIST2_LINK *extract_sublist(   // from this current...
      ELIST2_ITERATOR *other_it); // to other current

public:
  ELIST2_ITERATOR( // constructor
      ELIST2 *list_to_iterate);

  void set_to_list( // change list
      ELIST2 *list_to_iterate);

  void add_after_then_move(   // add after current &
      ELIST2_LINK *new_link); // move to new

  void add_after_stay_put(    // add after current &
      ELIST2_LINK *new_link); // stay at current

  void add_before_then_move(  // add before current &
      ELIST2_LINK *new_link); // move to new

  void add_before_stay_put(   // add before current &
      ELIST2_LINK *new_link); // stay at current

  void add_list_after(      // add a list &
      ELIST2 *list_to_add); // stay at current

  void add_list_before(     // add a list &
      ELIST2 *list_to_add); // move to it 1st item

  ELIST2_LINK *data() { // get current data
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

  ELIST2_LINK *data_relative( // get data + or - ...
      int8_t offset);         // offset from current

  ELIST2_LINK *forward(); // move to next element

  ELIST2_LINK *backward(); // move to prev element

  ELIST2_LINK *extract(); // remove from list

  // go to start of list
  ELIST2_LINK *move_to_first();

  ELIST2_LINK *move_to_last(); // go to end of list

  void mark_cycle_pt(); // remember current

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

  bool at_first() const; // Current is first?

  bool at_last() const; // Current is last?

  bool cycled_list() const; // Completed a cycle?

  void add_to_end(            // add at end &
      ELIST2_LINK *new_link); // don't move

  void exchange(                  // positions of 2 links
      ELIST2_ITERATOR *other_it); // other iterator

  //# elements in list
  int32_t length() const {
    return list->length();
  }

  void sort(          // sort elements
      int comparator( // comparison routine
          const void *, const void *));

private:
  // Don't use the following constructor.
  ELIST2_ITERATOR() = delete;
};

/***********************************************************************
 *              ELIST2_ITERATOR::set_to_list
 *
 *  (Re-)initialise the iterator to point to the start of the list_to_iterate
 *  over.
 **********************************************************************/

inline void ELIST2_ITERATOR::set_to_list( // change list
    ELIST2 *list_to_iterate) {
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
 *              ELIST2_ITERATOR::ELIST2_ITERATOR
 *
 *  CONSTRUCTOR - set iterator to specified list;
 **********************************************************************/

inline ELIST2_ITERATOR::ELIST2_ITERATOR(ELIST2 *list_to_iterate) {
  set_to_list(list_to_iterate);
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_after_then_move
 *
 *  Add a new element to the list after the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_after_then_move( // element to add
    ELIST2_LINK *new_element) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_after_stay_put
 *
 *  Add a new element to the list after the current element but do not move
 *  the iterator to the new element.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_after_stay_put( // element to add
    ELIST2_LINK *new_element) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_before_then_move
 *
 *  Add a new element to the list before the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_before_then_move( // element to add
    ELIST2_LINK *new_element) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_before_stay_put
 *
 *  Add a new element to the list before the current element but don't move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_before_stay_put( // element to add
    ELIST2_LINK *new_element) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_list_after
 *
 *  Insert another list to this list after the current element but don't move
 *the
 *  iterator.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_list_after(ELIST2 *list_to_add) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_list_before
 *
 *  Insert another list to this list before the current element. Move the
 *  iterator to the start of the inserted elements
 *  iterator.
 **********************************************************************/

inline void ELIST2_ITERATOR::add_list_before(ELIST2 *list_to_add) {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::extract
 *
 *  Do extraction by removing current from the list, returning it to the
 *  caller, but NOT updating the iterator.  (So that any calling loop can do
 *  this.)   The iterator's current points to nullptr.  If the extracted element
 *  is to be deleted, this is the callers responsibility.
 **********************************************************************/

inline ELIST2_LINK *ELIST2_ITERATOR::extract() {
  ELIST2_LINK *extracted_link;

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
}

/***********************************************************************
 *              ELIST2_ITERATOR::move_to_first()
 *
 *  Move current so that it is set to the start of the list.
 *  Return data just in case anyone wants it.
 **********************************************************************/

inline ELIST2_LINK *ELIST2_ITERATOR::move_to_first() {
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

inline ELIST2_LINK *ELIST2_ITERATOR::move_to_last() {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("ELIST2_ITERATOR::move_to_last", ABORT);
  }
#endif

  current = list->last;
  prev = current ? current->prev : nullptr;
  next = current ? current->next : nullptr;
  return current;
}

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

inline void ELIST2_ITERATOR::mark_cycle_pt() {
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
}

/***********************************************************************
 *              ELIST2_ITERATOR::at_first()
 *
 *  Are we at the start of the list?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::at_first() const {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("ELIST2_ITERATOR::at_first", ABORT);
  }
#endif

  // we're at a deleted
  return ((list->empty()) || (current == list->First()) ||
          ((current == nullptr) && (prev == list->last) && // NON-last pt between
           !ex_current_was_last));                         // first and last
}

/***********************************************************************
 *              ELIST2_ITERATOR::at_last()
 *
 *  Are we at the end of the list?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::at_last() const {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("ELIST2_ITERATOR::at_last", ABORT);
  }
#endif

  // we're at a deleted
  return ((list->empty()) || (current == list->last) ||
          ((current == nullptr) && (prev == list->last) && // last point between
           ex_current_was_last));                          // first and last
}

/***********************************************************************
 *              ELIST2_ITERATOR::cycled_list()
 *
 *  Have we returned to the cycle_pt since it was set?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::cycled_list() const {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("ELIST2_ITERATOR::cycled_list", ABORT);
  }
#endif

  return ((list->empty()) || ((current == cycle_pt) && started_cycling));
}

/***********************************************************************
 *              ELIST2_ITERATOR::sort()
 *
 *  Sort the elements of the list, then reposition at the start.
 *
 **********************************************************************/

inline void ELIST2_ITERATOR::sort( // sort elements
    int comparator(                // comparison routine
        const void *, const void *)) {
#ifndef NDEBUG
  if (!list) {
    NO_LIST.error("ELIST2_ITERATOR::sort", ABORT);
  }
#endif

  list->sort(comparator);
  move_to_first();
}

/***********************************************************************
 *              ELIST2_ITERATOR::add_to_end
 *
 *  Add a new element to the end of the list without moving the iterator.
 *  This is provided because a single linked list cannot move to the last as
 *  the iterator couldn't set its prev pointer.  Adding to the end is
 *  essential for implementing
              queues.
**********************************************************************/

inline void ELIST2_ITERATOR::add_to_end( // element to add
    ELIST2_LINK *new_element) {
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
}

#define ELIST2IZEH(CLASSNAME)                                                  \
  class CLASSNAME##_LIST : public X_LIST<ELIST2, ELIST2_ITERATOR, CLASSNAME> { \
    using X_LIST<ELIST2, ELIST2_ITERATOR, CLASSNAME>::X_LIST;                  \
  };                                                                           \
  struct CLASSNAME##_IT : X_ITER<ELIST2_ITERATOR, CLASSNAME> {                 \
    using X_ITER<ELIST2_ITERATOR, CLASSNAME>::X_ITER;                          \
    CLASSNAME *backward() {                                                    \
      return reinterpret_cast<CLASSNAME *>(ELIST2_ITERATOR::backward());       \
    }                                                                          \
  };

} // namespace tesseract

#endif
