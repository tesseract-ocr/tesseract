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

#include <cstdio>
#include "serialis.h"
#include "lsterr.h"

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

class DLLSYM ELIST2_LINK
{
  friend class ELIST2_ITERATOR;
  friend class ELIST2;

  ELIST2_LINK *prev;
  ELIST2_LINK *next;

  public:
    ELIST2_LINK() {  //constructor
      prev = next = nullptr;
    }

    ELIST2_LINK(                // copy constructor
        const ELIST2_LINK &) {  // don't copy link
      prev = next = nullptr;
    }

    void operator=(  // don't copy links
        const ELIST2_LINK &) {
      prev = next = nullptr;
    }
};

/**********************************************************************
 * CLASS - ELIST2
 *
 * Generic list class for doubly linked lists with embedded links
 **********************************************************************/

class DLLSYM ELIST2
{
  friend class ELIST2_ITERATOR;

  ELIST2_LINK *last;             //End of list
  //(Points to head)
  ELIST2_LINK *First() {  // return first
    return last ? last->next : nullptr;
  }

  public:
    ELIST2() {  //constructor
      last = nullptr;
    }

    void internal_clear (        //destroy all links
      void (*zapper) (ELIST2_LINK *));
    //ptr to zapper functn

    bool empty() const {  //is list empty?
      return !last;
    }

    bool singleton() const {
      return last ? (last == last->next) : false;
    }

    void shallow_copy(                      //dangerous!!
                      ELIST2 *from_list) {  //beware destructors!!
      last = from_list->last;
    }

                                 //ptr to copier functn
    void internal_deep_copy (ELIST2_LINK * (*copier) (ELIST2_LINK *),
      const ELIST2 * list);      //list being copied

    void assign_to_sublist(                            //to this list
                           ELIST2_ITERATOR *start_it,  //from list start
                           ELIST2_ITERATOR *end_it);   //from list end

    int32_t length() const;  // # elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

    // Assuming list has been sorted already, insert new_link to
    // keep the list sorted according to the same comparison function.
    // Comparison function is the same as used by sort, i.e. uses double
    // indirection. Time is O(1) to add to beginning or end.
    // Time is linear to add pre-sorted items to an empty list.
    void add_sorted(int comparator(const void*, const void*),
                    ELIST2_LINK* new_link);

};

/***********************************************************************
 *              CLASS - ELIST2_ITERATOR
 *
 *              Generic iterator class for doubly linked lists with embedded
 *links
 **********************************************************************/

class DLLSYM ELIST2_ITERATOR
{
  friend void ELIST2::assign_to_sublist(ELIST2_ITERATOR *, ELIST2_ITERATOR *);

  ELIST2 *list;                  //List being iterated
  ELIST2_LINK *prev;             //prev element
  ELIST2_LINK *current;          //current element
  ELIST2_LINK *next;             //next element
  ELIST2_LINK *cycle_pt;         //point we are cycling the list to.
  bool ex_current_was_last;      //current extracted was end of list
  bool ex_current_was_cycle_pt;  //current extracted was cycle point
  bool started_cycling;          //Have we moved off the start?

  ELIST2_LINK *extract_sublist(                             //from this current...
                               ELIST2_ITERATOR *other_it);  //to other current

  public:
    ELIST2_ITERATOR(  //constructor
                    ELIST2 *list_to_iterate);

    void set_to_list(  //change list
                     ELIST2 *list_to_iterate);

    void add_after_then_move(                         //add after current &
                             ELIST2_LINK *new_link);  //move to new

    void add_after_stay_put(                         //add after current &
                            ELIST2_LINK *new_link);  //stay at current

    void add_before_then_move(                         //add before current &
                              ELIST2_LINK *new_link);  //move to new

    void add_before_stay_put(                         //add before current &
                             ELIST2_LINK *new_link);  //stay at current

    void add_list_after(                       //add a list &
                        ELIST2 *list_to_add);  //stay at current

    void add_list_before(                       //add a list &
                         ELIST2 *list_to_add);  //move to it 1st item

    ELIST2_LINK *data() {  //get current data
    #ifndef NDEBUG
      if (!current)
        NULL_DATA.error ("ELIST2_ITERATOR::data", ABORT, nullptr);
      if (!list)
        NO_LIST.error ("ELIST2_ITERATOR::data", ABORT, nullptr);
    #endif
      return current;
    }

    ELIST2_LINK *data_relative(               //get data + or - ...
                               int8_t offset);  //offset from current

    ELIST2_LINK *forward();  //move to next element

    ELIST2_LINK *backward();  //move to prev element

    ELIST2_LINK *extract();  //remove from list

                                 //go to start of list
    ELIST2_LINK *move_to_first();

    ELIST2_LINK *move_to_last();  //go to end of list

    void mark_cycle_pt();  //remember current

    bool empty() {  //is list empty?
    #ifndef NDEBUG
      if (!list)
        NO_LIST.error ("ELIST2_ITERATOR::empty", ABORT, nullptr);
    #endif
      return list->empty ();
    }

    bool current_extracted() {  //current extracted?
      return !current;
    }

    bool at_first();  //Current is first?

    bool at_last();  //Current is last?

    bool cycled_list();  //Completed a cycle?

    void add_to_end(             // add at end &
        ELIST2_LINK *new_link);  // don't move

    void exchange(                             //positions of 2 links
                  ELIST2_ITERATOR *other_it);  //other iterator

    int32_t length();  //# elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

  private:
    // Don't use the following constructor.
    ELIST2_ITERATOR();
};

/***********************************************************************
 *              ELIST2_ITERATOR::set_to_list
 *
 *  (Re-)initialise the iterator to point to the start of the list_to_iterate
 *  over.
 **********************************************************************/

inline void ELIST2_ITERATOR::set_to_list(  //change list
                                         ELIST2 *list_to_iterate) {
  #ifndef NDEBUG
  if (!list_to_iterate)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::set_to_list", ABORT,
      "list_to_iterate is nullptr");
  #endif

  list = list_to_iterate;
  prev = list->last;
  current = list->First ();
  next = current ? current->next : nullptr;
  cycle_pt = nullptr;               //await explicit set
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

inline void ELIST2_ITERATOR::add_after_then_move(  // element to add
                                                 ELIST2_LINK *new_element) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_after_then_move", ABORT, nullptr);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_after_then_move", ABORT,
      "new_element is nullptr");
  if (new_element->next)
    STILL_LINKED.error ("ELIST2_ITERATOR::add_after_then_move", ABORT, nullptr);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    new_element->prev = new_element;
    list->last = new_element;
    prev = next = new_element;
  }
  else {
    new_element->next = next;
    next->prev = new_element;

    if (current) {               //not extracted
      new_element->prev = current;
      current->next = new_element;
      prev = current;
      if (current == list->last)
        list->last = new_element;
    }
    else {                       //current extracted
      new_element->prev = prev;
      prev->next = new_element;
      if (ex_current_was_last)
        list->last = new_element;
      if (ex_current_was_cycle_pt)
        cycle_pt = new_element;
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

inline void ELIST2_ITERATOR::add_after_stay_put(  // element to add
                                                ELIST2_LINK *new_element) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_after_stay_put", ABORT, nullptr);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_after_stay_put", ABORT,
      "new_element is nullptr");
  if (new_element->next)
    STILL_LINKED.error ("ELIST2_ITERATOR::add_after_stay_put", ABORT, nullptr);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    new_element->prev = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = false;
    current = nullptr;
  }
  else {
    new_element->next = next;
    next->prev = new_element;

    if (current) {               //not extracted
      new_element->prev = current;
      current->next = new_element;
      if (prev == current)
        prev = new_element;
      if (current == list->last)
        list->last = new_element;
    }
    else {                       //current extracted
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

inline void ELIST2_ITERATOR::add_before_then_move(  // element to add
                                                  ELIST2_LINK *new_element) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_before_then_move", ABORT, nullptr);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_before_then_move", ABORT,
      "new_element is nullptr");
  if (new_element->next)
    STILL_LINKED.error ("ELIST2_ITERATOR::add_before_then_move", ABORT, nullptr);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    new_element->prev = new_element;
    list->last = new_element;
    prev = next = new_element;
  }
  else {
    prev->next = new_element;
    new_element->prev = prev;

    if (current) {               //not extracted
      new_element->next = current;
      current->prev = new_element;
      next = current;
    }
    else {                       //current extracted
      new_element->next = next;
      next->prev = new_element;
      if (ex_current_was_last)
        list->last = new_element;
      if (ex_current_was_cycle_pt)
        cycle_pt = new_element;
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

inline void ELIST2_ITERATOR::add_before_stay_put(  // element to add
                                                 ELIST2_LINK *new_element) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_before_stay_put", ABORT, nullptr);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_before_stay_put", ABORT,
      "new_element is nullptr");
  if (new_element->next)
    STILL_LINKED.error ("ELIST2_ITERATOR::add_before_stay_put", ABORT, nullptr);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    new_element->prev = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = true;
    current = nullptr;
  }
  else {
    prev->next = new_element;
    new_element->prev = prev;

    if (current) {               //not extracted
      new_element->next = current;
      current->prev = new_element;
      if (next == current)
        next = new_element;
    }
    else {                       //current extracted
      new_element->next = next;
      next->prev = new_element;
      if (ex_current_was_last)
        list->last = new_element;
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_list_after", ABORT, nullptr);
  if (!list_to_add)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_list_after", ABORT,
      "list_to_add is nullptr");
  #endif

  if (!list_to_add->empty ()) {
    if (list->empty ()) {
      list->last = list_to_add->last;
      prev = list->last;
      next = list->First ();
      ex_current_was_last = true;
      current = nullptr;
    }
    else {
      if (current) {             //not extracted
        current->next = list_to_add->First ();
        current->next->prev = current;
        if (current == list->last)
          list->last = list_to_add->last;
        list_to_add->last->next = next;
        next->prev = list_to_add->last;
        next = current->next;
      }
      else {                     //current extracted
        prev->next = list_to_add->First ();
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_list_before", ABORT, nullptr);
  if (!list_to_add)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_list_before", ABORT,
      "list_to_add is nullptr");
  #endif

  if (!list_to_add->empty ()) {
    if (list->empty ()) {
      list->last = list_to_add->last;
      prev = list->last;
      current = list->First ();
      next = current->next;
      ex_current_was_last = false;
    }
    else {
      prev->next = list_to_add->First ();
      prev->next->prev = prev;

      if (current) {             //not extracted
        list_to_add->last->next = current;
        current->prev = list_to_add->last;
      }
      else {                     //current extracted
        list_to_add->last->next = next;
        next->prev = list_to_add->last;
        if (ex_current_was_last)
          list->last = list_to_add->last;
        if (ex_current_was_cycle_pt)
          cycle_pt = prev->next;
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::extract", ABORT, nullptr);
  if (!current)                  //list empty or
                                 //element extracted
    NULL_CURRENT.error ("ELIST2_ITERATOR::extract",
      ABORT, nullptr);
  #endif

  if (list->singleton()) {
    // Special case where we do need to change the iterator.
    prev = next = list->last = nullptr;
  } else {
    prev->next = next;           //remove from list
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
  extracted_link->next = nullptr;   //for safety
  extracted_link->prev = nullptr;   //for safety
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::move_to_first", ABORT, nullptr);
  #endif

  current = list->First ();
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::move_to_last", ABORT, nullptr);
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
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::mark_cycle_pt", ABORT, nullptr);
  #endif

  if (current)
    cycle_pt = current;
  else
    ex_current_was_cycle_pt = true;
  started_cycling = false;
}

/***********************************************************************
 *              ELIST2_ITERATOR::at_first()
 *
 *  Are we at the start of the list?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::at_first() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::at_first", ABORT, nullptr);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->First ()) || ((current == nullptr) &&
    (prev == list->last) &&      //NON-last pt between
    !ex_current_was_last));      //first and last
}

/***********************************************************************
 *              ELIST2_ITERATOR::at_last()
 *
 *  Are we at the end of the list?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::at_last() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::at_last", ABORT, nullptr);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->last) || ((current == nullptr) &&
    (prev == list->last) &&      //last point between
    ex_current_was_last));       //first and last
}

/***********************************************************************
 *              ELIST2_ITERATOR::cycled_list()
 *
 *  Have we returned to the cycle_pt since it was set?
 *
 **********************************************************************/

inline bool ELIST2_ITERATOR::cycled_list() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::cycled_list", ABORT, nullptr);
  #endif

  return ((list->empty ()) || ((current == cycle_pt) && started_cycling));

}

/***********************************************************************
 *              ELIST2_ITERATOR::length()
 *
 *  Return the length of the list
 *
 **********************************************************************/

inline int32_t ELIST2_ITERATOR::length() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::length", ABORT, nullptr);
  #endif

  return list->length ();
}

/***********************************************************************
 *              ELIST2_ITERATOR::sort()
 *
 *  Sort the elements of the list, then reposition at the start.
 *
 **********************************************************************/

inline void
ELIST2_ITERATOR::sort (          //sort elements
int comparator (                 //comparison routine
const void *, const void *)) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::sort", ABORT, nullptr);
  #endif

  list->sort (comparator);
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

inline void ELIST2_ITERATOR::add_to_end(  // element to add
                                        ELIST2_LINK *new_element) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("ELIST2_ITERATOR::add_to_end", ABORT, nullptr);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST2_ITERATOR::add_to_end", ABORT,
      "new_element is nullptr");
  if (new_element->next)
    STILL_LINKED.error ("ELIST2_ITERATOR::add_to_end", ABORT, nullptr);
  #endif

  if (this->at_last ()) {
    this->add_after_stay_put (new_element);
  }
  else {
    if (this->at_first ()) {
      this->add_before_stay_put (new_element);
      list->last = new_element;
    }
    else {                       //Iteratr is elsewhere
      new_element->next = list->last->next;
      new_element->prev = list->last;
      list->last->next->prev = new_element;
      list->last->next = new_element;
      list->last = new_element;
    }
  }
}


/***********************************************************************
  QUOTE_IT   MACRO DEFINITION
  ===========================
Replace <parm> with "<parm>".  <parm> may be an arbitrary number of tokens
***********************************************************************/

#define QUOTE_IT(parm) #parm

/***********************************************************************
  ELIST2IZE(CLASSNAME) MACRO DEFINITION
  ======================================

CLASSNAME is assumed to be the name of a class which has a baseclass of
ELIST2_LINK.

NOTE:  Because we don't use virtual functions in the list code, the list code
will NOT work correctly for classes derived from this.

The macro generates:
  - An element deletion function:      CLASSNAME##_zapper
  - An E_LIST2 subclass:  CLASSNAME##_LIST
  - An E_LIST2_ITERATOR subclass:
              CLASSNAME##_IT

NOTE: Generated names are DELIBERATELY designed to clash with those for
ELISTIZE but NOT with those for CLISTIZE.

Two macros are provided: ELIST2IZE and ELIST2IZEH
The ...IZEH macros just define the class names for use in .h files
The ...IZE macros define the code use in .c files
***********************************************************************/

/***********************************************************************
  ELIST2IZEH(CLASSNAME) MACRO

ELIST2IZEH is a concatenation of 3 fragments ELIST2IZEH_A, ELIST2IZEH_B and
ELIST2IZEH_C.
***********************************************************************/

#define ELIST2IZEH_A(CLASSNAME)                                               \
                                                                              \
  extern DLLSYM void CLASSNAME##_zapper(                    /*delete a link*/ \
                                        ELIST2_LINK *link); /*link to delete*/

#define ELIST2IZEH_B(CLASSNAME)                                            \
                                                                           \
  /*********************************************************************** \
  *             CLASS -                                                    \
  *CLASSNAME##_LIST                                                        \
  *                                                                        \
  *             List class for class                                       \
  *CLASSNAME                                                               \
  *                                                                        \
  **********************************************************************/  \
                                                                           \
  class DLLSYM CLASSNAME##_LIST : public ELIST2 {                          \
   public:                                                                 \
    CLASSNAME##_LIST() : ELIST2() {}                                       \
    /* constructor */                                                      \
                                                                           \
    CLASSNAME##_LIST(                          /* don't construct */       \
                     const CLASSNAME##_LIST &) /*by initial assign*/       \
    {                                                                      \
      DONT_CONSTRUCT_LIST_BY_COPY.error(QUOTE_IT(CLASSNAME##_LIST), ABORT, \
                                        nullptr);                             \
    }                                                                      \
                                                                           \
    void clear() /* delete elements */                                     \
    {                                                                      \
      ELIST2::internal_clear(&CLASSNAME##_zapper);                         \
    }                                                                      \
                                                                           \
    ~CLASSNAME##_LIST() /* destructor */                                   \
    {                                                                      \
      clear();                                                             \
    }                                                                      \
                                                                           \
    /* Become a deep copy of src_list*/                                    \
    void deep_copy(const CLASSNAME##_LIST *src_list,                       \
                   CLASSNAME *(*copier)(const CLASSNAME *));               \
                                                                           \
    void operator=(/* prevent assign */                                    \
                   const CLASSNAME##_LIST &) {                             \
      DONT_ASSIGN_LISTS.error(QUOTE_IT(CLASSNAME##_LIST), ABORT, nullptr);    \
    }

#define ELIST2IZEH_C(CLASSNAME)                                                \
  }                                                                            \
  ;                                                                            \
                                                                               \
  /***********************************************************************     \
  *             CLASS - CLASSNAME##_IT                                         \
  *                                                                            \
  *             Iterator class for class CLASSNAME##_LIST                      \
  *                                                                            \
  *  Note: We don't need to coerce pointers to member functions input          \
  *  parameters as these are automatically converted to the type of the base   \
  *  type. ("A ptr to a class may be converted to a pointer to a public base   \
  *  class of that class")                                                     \
  **********************************************************************/      \
                                                                               \
  class DLLSYM CLASSNAME##_IT : public ELIST2_ITERATOR {                       \
   public:                                                                     \
    CLASSNAME##_IT(CLASSNAME##_LIST *list) : ELIST2_ITERATOR(list) {}          \
                                                                               \
    CLASSNAME *data() { return (CLASSNAME *)ELIST2_ITERATOR::data(); }         \
                                                                               \
    CLASSNAME *data_relative(int8_t offset) {                                  \
      return (CLASSNAME *)ELIST2_ITERATOR::data_relative(offset);              \
    }                                                                          \
                                                                               \
    CLASSNAME *forward() { return (CLASSNAME *)ELIST2_ITERATOR::forward(); }   \
                                                                               \
    CLASSNAME *backward() { return (CLASSNAME *)ELIST2_ITERATOR::backward(); } \
                                                                               \
    CLASSNAME *extract() { return (CLASSNAME *)ELIST2_ITERATOR::extract(); }   \
                                                                               \
    CLASSNAME *move_to_first() {                                               \
      return (CLASSNAME *)ELIST2_ITERATOR::move_to_first();                    \
    }                                                                          \
                                                                               \
    CLASSNAME *move_to_last() {                                                \
      return (CLASSNAME *)ELIST2_ITERATOR::move_to_last();                     \
    }                                                                          \
   private:                                                                    \
    CLASSNAME##_IT();                                                          \
  };

#define ELIST2IZEH(CLASSNAME) \
                              \
  ELIST2IZEH_A(CLASSNAME)     \
                              \
  ELIST2IZEH_B(CLASSNAME)     \
                              \
  ELIST2IZEH_C(CLASSNAME)

/***********************************************************************
  ELIST2IZE(CLASSNAME) MACRO
***********************************************************************/

#define ELIST2IZE(CLASSNAME)                                                  \
                                                                              \
  /***********************************************************************    \
  *             CLASSNAME##_zapper                                            \
  *                                                                           \
  *  A function which can delete a CLASSNAME element.  This is passed to the  \
  *  generic clear list member function so that when a list is cleared the    \
  *  elements on the list are properly destroyed from the base class, even    \
  *  though we don't use a virtual destructor function.                       \
  **********************************************************************/     \
                                                                              \
  DLLSYM void CLASSNAME##_zapper(                   /*delete a link*/         \
                                 ELIST2_LINK *link) /*link to delete*/        \
  {                                                                           \
    delete (CLASSNAME *)link;                                                 \
  }                                                                           \
                                                                              \
  /* Become a deep copy of src_list*/                                         \
  void CLASSNAME##_LIST::deep_copy(const CLASSNAME##_LIST *src_list,          \
                                   CLASSNAME *(*copier)(const CLASSNAME *)) { \
    CLASSNAME##_IT from_it(const_cast<CLASSNAME##_LIST *>(src_list));         \
    CLASSNAME##_IT to_it(this);                                               \
                                                                              \
    for (from_it.mark_cycle_pt(); !from_it.cycled_list(); from_it.forward())  \
      to_it.add_after_then_move((*copier)(from_it.data()));                   \
  }

#endif
