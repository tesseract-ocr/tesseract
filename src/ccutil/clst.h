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

#include <tesseract/serialis.h>

#include <cstdio>

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

class DLLSYM CLIST_LINK
{
  friend class CLIST_ITERATOR;
  friend class CLIST;

  CLIST_LINK *next;
  void *data;

  public:
    CLIST_LINK() {  //constructor
      data = next = nullptr;
    }

    CLIST_LINK(                // copy constructor
        const CLIST_LINK &) {  // don't copy link
      data = next = nullptr;
    }

    void operator=(  // don't copy links
        const CLIST_LINK &) {
      data = next = nullptr;
    }
};

/**********************************************************************
 * CLASS - CLIST
 *
 * Generic list class for singly linked CONS cell lists
 **********************************************************************/

class DLLSYM CLIST
{
  friend class CLIST_ITERATOR;

  CLIST_LINK *last;              //End of list
  //(Points to head)
  CLIST_LINK *First() {  // return first
    return last != nullptr ? last->next : nullptr;
  }

  public:
    CLIST() {  //constructor
      last = nullptr;
    }

    ~CLIST () {                  //destructor
      shallow_clear();
    }

    void internal_deep_clear (   //destroy all links
      void (*zapper) (void *));  //ptr to zapper functn

    void shallow_clear();  // clear list but don't
    // delete data elements

    bool empty() const {  //is list empty?
      return !last;
    }

    bool singleton() const {
      return last != nullptr ? (last == last->next) : false;
    }

    void shallow_copy(                     //dangerous!!
                      CLIST *from_list) {  //beware destructors!!
      last = from_list->last;
    }

    void assign_to_sublist(                           //to this list
                           CLIST_ITERATOR *start_it,  //from list start
                           CLIST_ITERATOR *end_it);   //from list end

    int32_t length() const;  //# elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

    // Assuming list has been sorted already, insert new_data to
    // keep the list sorted according to the same comparison function.
    // Comparison function is the same as used by sort, i.e. uses double
    // indirection. Time is O(1) to add to beginning or end.
    // Time is linear to add pre-sorted items to an empty list.
    // If unique, then don't add duplicate entries.
    // Returns true if the element was added to the list.
    bool add_sorted(int comparator(const void*, const void*),
                    bool unique, void* new_data);

    // Assuming that the minuend and subtrahend are already sorted with
    // the same comparison function, shallow clears this and then copies
    // the set difference minuend - subtrahend to this, being the elements
    // of minuend that do not compare equal to anything in subtrahend.
    // If unique is true, any duplicates in minuend are also eliminated.
    void set_subtract(int comparator(const void*, const void*), bool unique,
                      CLIST* minuend, CLIST* subtrahend);

};

/***********************************************************************
 *              CLASS - CLIST_ITERATOR
 *
 *              Generic iterator class for singly linked lists with embedded
 *links
 **********************************************************************/

class DLLSYM CLIST_ITERATOR
{
  friend void CLIST::assign_to_sublist(CLIST_ITERATOR *, CLIST_ITERATOR *);

  CLIST *list;                   //List being iterated
  CLIST_LINK *prev;              //prev element
  CLIST_LINK *current;           //current element
  CLIST_LINK *next;              //next element
  CLIST_LINK *cycle_pt;          //point we are cycling the list to.
  bool ex_current_was_last;      //current extracted was end of list
  bool ex_current_was_cycle_pt;  //current extracted was cycle point
  bool started_cycling;          //Have we moved off the start?

  CLIST_LINK *extract_sublist(                            //from this current...
                              CLIST_ITERATOR *other_it);  //to other current

  public:
    CLIST_ITERATOR() {  //constructor
      list = nullptr;
    }                            //unassigned list

    CLIST_ITERATOR(  //constructor
                   CLIST *list_to_iterate);

    void set_to_list(  //change list
                     CLIST *list_to_iterate);

    void add_after_then_move(                  //add after current &
                             void *new_data);  //move to new

    void add_after_stay_put(                  //add after current &
                            void *new_data);  //stay at current

    void add_before_then_move(                  //add before current &
                              void *new_data);  //move to new

    void add_before_stay_put(                  //add before current &
                             void *new_data);  //stay at current

    void add_list_after(                      //add a list &
                        CLIST *list_to_add);  //stay at current

    void add_list_before(                      //add a list &
                         CLIST *list_to_add);  //move to it 1st item

    void *data() {  //get current data
    #ifndef NDEBUG
      if (!list)
        NO_LIST.error ("CLIST_ITERATOR::data", ABORT, nullptr);
      if (!current)
        NULL_DATA.error ("CLIST_ITERATOR::data", ABORT, nullptr);
    #endif
      return current->data;
    }

    void *data_relative(               //get data + or - ...
                        int8_t offset);  //offset from current

    void *forward();  //move to next element

    void *extract();  //remove from list

    void *move_to_first();  //go to start of list

    void *move_to_last();  //go to end of list

    void mark_cycle_pt();  //remember current

    bool empty() {  //is list empty?
    #ifndef NDEBUG
      if (!list)
        NO_LIST.error ("CLIST_ITERATOR::empty", ABORT, nullptr);
    #endif
      return list->empty ();
    }

    bool current_extracted() {  //current extracted?
      return !current;
    }

    bool at_first();  //Current is first?

    bool at_last();  //Current is last?

    bool cycled_list();  //Completed a cycle?

    void add_to_end(      // add at end &
        void *new_data);  // don't move

    void exchange(                            //positions of 2 links
                  CLIST_ITERATOR *other_it);  //other iterator

    int32_t length();  //# elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

};

/***********************************************************************
 *              CLIST_ITERATOR::set_to_list
 *
 *  (Re-)initialise the iterator to point to the start of the list_to_iterate
 *  over.
 **********************************************************************/

inline void CLIST_ITERATOR::set_to_list(  //change list
                                        CLIST *list_to_iterate) {
  #ifndef NDEBUG
  if (!list_to_iterate)
    BAD_PARAMETER.error ("CLIST_ITERATOR::set_to_list", ABORT,
      "list_to_iterate is nullptr");
  #endif

  list = list_to_iterate;
  prev = list->last;
  current = list->First ();
  next = current != nullptr ? current->next : nullptr;
  cycle_pt = nullptr;               //await explicit set
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

inline void CLIST_ITERATOR::add_after_then_move(  // element to add
                                                void *new_data) {
  CLIST_LINK *new_element;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_after_then_move", ABORT, nullptr);
  if (!new_data)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_after_then_move", ABORT,
      "new_data is nullptr");
  #endif

  new_element = new CLIST_LINK;
  new_element->data = new_data;

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
  }
  else {
    new_element->next = next;

    if (current) {               //not extracted
      current->next = new_element;
      prev = current;
      if (current == list->last)
        list->last = new_element;
    }
    else {                       //current extracted
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
 *              CLIST_ITERATOR::add_after_stay_put
 *
 *  Add a new element to the list after the current element but do not move
 *  the iterator to the new element.
 **********************************************************************/

inline void CLIST_ITERATOR::add_after_stay_put(  // element to add
                                               void *new_data) {
  CLIST_LINK *new_element;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_after_stay_put", ABORT, nullptr);
  if (!new_data)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_after_stay_put", ABORT,
      "new_data is nullptr");
  #endif

  new_element = new CLIST_LINK;
  new_element->data = new_data;

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = false;
    current = nullptr;
  }
  else {
    new_element->next = next;

    if (current) {               //not extracted
      current->next = new_element;
      if (prev == current)
        prev = new_element;
      if (current == list->last)
        list->last = new_element;
    }
    else {                       //current extracted
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

inline void CLIST_ITERATOR::add_before_then_move(  // element to add
                                                 void *new_data) {
  CLIST_LINK *new_element;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_before_then_move", ABORT, nullptr);
  if (!new_data)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_before_then_move", ABORT,
      "new_data is nullptr");
  #endif

  new_element = new CLIST_LINK;
  new_element->data = new_data;

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
  }
  else {
    prev->next = new_element;
    if (current) {               //not extracted
      new_element->next = current;
      next = current;
    }
    else {                       //current extracted
      new_element->next = next;
      if (ex_current_was_last)
        list->last = new_element;
      if (ex_current_was_cycle_pt)
        cycle_pt = new_element;
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

inline void CLIST_ITERATOR::add_before_stay_put(  // element to add
                                                void *new_data) {
  CLIST_LINK *new_element;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_before_stay_put", ABORT, nullptr);
  if (!new_data)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_before_stay_put", ABORT,
      "new_data is nullptr");
  #endif

  new_element = new CLIST_LINK;
  new_element->data = new_data;

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = true;
    current = nullptr;
  }
  else {
    prev->next = new_element;
    if (current) {               //not extracted
      new_element->next = current;
      if (next == current)
        next = new_element;
    }
    else {                       //current extracted
      new_element->next = next;
      if (ex_current_was_last)
        list->last = new_element;
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
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_list_after", ABORT, nullptr);
  if (!list_to_add)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_list_after", ABORT,
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
        if (current == list->last)
          list->last = list_to_add->last;
        list_to_add->last->next = next;
        next = current->next;
      }
      else {                     //current extracted
        prev->next = list_to_add->First ();
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
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_list_before", ABORT, nullptr);
  if (!list_to_add)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_list_before", ABORT,
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
      if (current) {             //not extracted
        list_to_add->last->next = current;
      }
      else {                     //current extracted
        list_to_add->last->next = next;
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
 *              CLIST_ITERATOR::extract
 *
 *  Do extraction by removing current from the list, deleting the cons cell
 *  and returning the data to the caller, but NOT updating the iterator.  (So
 *  that any calling loop can do this.)  The iterator's current points to
 *  nullptr.  If the data is to be deleted, this is the callers responsibility.
 **********************************************************************/

inline void *CLIST_ITERATOR::extract() {
  void *extracted_data;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::extract", ABORT, nullptr);
  if (!current)                  //list empty or
                                 //element extracted
    NULL_CURRENT.error ("CLIST_ITERATOR::extract",
      ABORT, nullptr);
  #endif

  if (list->singleton()) {
    // Special case where we do need to change the iterator.
    prev = next = list->last = nullptr;
  } else {
    prev->next = next;           //remove from list

    if (current == list->last) {
      list->last = prev;
      ex_current_was_last = true;
    } else {
      ex_current_was_last = false;
    }
  }
  // Always set ex_current_was_cycle_pt so an add/forward will work in a loop.
  ex_current_was_cycle_pt = (current == cycle_pt);
  extracted_data = current->data;
  delete(current);  //destroy CONS cell
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
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::move_to_first", ABORT, nullptr);
  #endif

  current = list->First ();
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
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::mark_cycle_pt", ABORT, nullptr);
  #endif

  if (current)
    cycle_pt = current;
  else
    ex_current_was_cycle_pt = true;
  started_cycling = false;
}

/***********************************************************************
 *              CLIST_ITERATOR::at_first()
 *
 *  Are we at the start of the list?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::at_first() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::at_first", ABORT, nullptr);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->First ()) || ((current == nullptr) &&
    (prev == list->last) &&      //NON-last pt between
    !ex_current_was_last));      //first and last
}

/***********************************************************************
 *              CLIST_ITERATOR::at_last()
 *
 *  Are we at the end of the list?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::at_last() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::at_last", ABORT, nullptr);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->last) || ((current == nullptr) &&
    (prev == list->last) &&      //last point between
    ex_current_was_last));       //first and last
}

/***********************************************************************
 *              CLIST_ITERATOR::cycled_list()
 *
 *  Have we returned to the cycle_pt since it was set?
 *
 **********************************************************************/

inline bool CLIST_ITERATOR::cycled_list() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::cycled_list", ABORT, nullptr);
  #endif

  return ((list->empty ()) || ((current == cycle_pt) && started_cycling));

}

/***********************************************************************
 *              CLIST_ITERATOR::length()
 *
 *  Return the length of the list
 *
 **********************************************************************/

inline int32_t CLIST_ITERATOR::length() {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::length", ABORT, nullptr);
  #endif

  return list->length ();
}

/***********************************************************************
 *              CLIST_ITERATOR::sort()
 *
 *  Sort the elements of the list, then reposition at the start.
 *
 **********************************************************************/

inline void
CLIST_ITERATOR::sort (           //sort elements
int comparator (                 //comparison routine
const void *, const void *)) {
  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::sort", ABORT, nullptr);
  #endif

  list->sort (comparator);
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

inline void CLIST_ITERATOR::add_to_end(  // element to add
                                       void *new_data) {
  CLIST_LINK *new_element;

  #ifndef NDEBUG
  if (!list)
    NO_LIST.error ("CLIST_ITERATOR::add_to_end", ABORT, nullptr);
  if (!new_data)
    BAD_PARAMETER.error ("CLIST_ITERATOR::add_to_end", ABORT,
      "new_data is nullptr");
  #endif

  if (this->at_last ()) {
    this->add_after_stay_put (new_data);
  }
  else {
    if (this->at_first ()) {
      this->add_before_stay_put (new_data);
      list->last = prev;
    }
    else {                       //Iteratr is elsewhere
      new_element = new CLIST_LINK;
      new_element->data = new_data;

      new_element->next = list->last->next;
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
  CLISTIZE(CLASSNAME) MACRO DEFINITION
  ======================================

CLASSNAME is assumed to be the name of a class to be used in a CONS list

NOTE:  Because we don't use virtual functions in the list code, the list code
will NOT work correctly for classes derived from this.

The macro generates:
  - An element deletion function:      CLASSNAME##_c1_zapper
  - An element copier function:
              CLASSNAME##_c1_copier
  - A CLIST subclass:   CLASSNAME##_CLIST
  - A CLIST_ITERATOR subclass:
              CLASSNAME##_C_IT

NOTE:
Generated names do NOT clash with those generated by ELISTIZE and ELIST2ISE.

Two macros are provided: CLISTIZE and CLISTIZEH
The ...IZEH macros just define the class names for use in .h files
The ...IZE macros define the code use in .c files
***********************************************************************/

/***********************************************************************
  CLISTIZEH(CLASSNAME)  MACRO

CLISTIZEH is a concatenation of 3 fragments CLISTIZEH_A, CLISTIZEH_B and
CLISTIZEH_C.
***********************************************************************/

#define CLISTIZEH_A(CLASSNAME)                                             \
                                                                           \
  extern DLLSYM void CLASSNAME##_c1_zapper(             /*delete a link*/  \
                                           void *link); /*link to delete*/ \
                                                                           \
  extern DLLSYM void                                                       \
      *CLASSNAME##_c1_copier(                    /*deep copy a link*/      \
                             void *old_element); /*source link */

#define CLISTIZEH_B(CLASSNAME)                                              \
                                                                            \
  /***********************************************************************  \
  *             CLASS -                                                     \
  *CLASSNAME##_CLIST                                                        \
  *                                                                         \
  *             List class for class                                        \
  *CLASSNAME                                                                \
  *                                                                         \
  **********************************************************************/   \
                                                                            \
  class DLLSYM CLASSNAME##_CLIST : public CLIST {                           \
   public:                                                                  \
    CLASSNAME##_CLIST() : CLIST() {}                                        \
    /* constructor */                                                       \
                                                                            \
    CLASSNAME##_CLIST(                           /* don't construct */      \
                      const CLASSNAME##_CLIST &) /*by initial assign*/      \
    {                                                                       \
      DONT_CONSTRUCT_LIST_BY_COPY.error(QUOTE_IT(CLASSNAME##_CLIST), ABORT, \
                                        nullptr);                              \
    }                                                                       \
                                                                            \
    void deep_clear() /* delete elements */                                 \
    {                                                                       \
      CLIST::internal_deep_clear(&CLASSNAME##_c1_zapper);                   \
    }                                                                       \
                                                                            \
    void operator=(/* prevent assign */                                     \
                   const CLASSNAME##_CLIST &) {                             \
      DONT_ASSIGN_LISTS.error(QUOTE_IT(CLASSNAME##_CLIST), ABORT, nullptr);    \
    }

#define CLISTIZEH_C(CLASSNAME)                                               \
  }                                                                          \
  ;                                                                          \
                                                                             \
  /***********************************************************************   \
  *             CLASS - CLASSNAME##_C_IT                                     \
  *                                                                          \
  *             Iterator class for class CLASSNAME##_CLIST                   \
  *                                                                          \
  *  Note: We don't need to coerce pointers to member functions input        \
  *  parameters as these are automatically converted to the type of the base \
  *  type. ("A ptr to a class may be converted to a pointer to a public base \
  *  class of that class")                                                   \
  **********************************************************************/    \
                                                                             \
  class DLLSYM CLASSNAME##_C_IT : public CLIST_ITERATOR {                    \
   public:                                                                   \
    CLASSNAME##_C_IT() : CLIST_ITERATOR() {}                                 \
                                                                             \
    CLASSNAME##_C_IT(CLASSNAME##_CLIST *list) : CLIST_ITERATOR(list) {}      \
                                                                             \
    CLASSNAME *data() { return (CLASSNAME *)CLIST_ITERATOR::data(); }        \
                                                                             \
    CLASSNAME *data_relative(int8_t offset) {                                  \
      return (CLASSNAME *)CLIST_ITERATOR::data_relative(offset);             \
    }                                                                        \
                                                                             \
    CLASSNAME *forward() { return (CLASSNAME *)CLIST_ITERATOR::forward(); }  \
                                                                             \
    CLASSNAME *extract() { return (CLASSNAME *)CLIST_ITERATOR::extract(); }  \
                                                                             \
    CLASSNAME *move_to_first() {                                             \
      return (CLASSNAME *)CLIST_ITERATOR::move_to_first();                   \
    }                                                                        \
                                                                             \
    CLASSNAME *move_to_last() {                                              \
      return (CLASSNAME *)CLIST_ITERATOR::move_to_last();                    \
    }                                                                        \
  };

#define CLISTIZEH(CLASSNAME) \
                             \
  CLISTIZEH_A(CLASSNAME)     \
                             \
  CLISTIZEH_B(CLASSNAME)     \
                             \
  CLISTIZEH_C(CLASSNAME)

/***********************************************************************
  CLISTIZE(CLASSNAME)  MACRO
***********************************************************************/

#define CLISTIZE(CLASSNAME)                                                  \
                                                                             \
  /***********************************************************************   \
  *             CLASSNAME##_c1_zapper                                        \
  *                                                                          \
  *  A function which can delete a CLASSNAME element.  This is passed to the \
  *  generic deep_clear list member function so that when a list is cleared  \
  *the                                                                       \
  *  elements on the list are properly destroyed from the base class, even   \
  *  though we don't use a virtual destructor function.                      \
  **********************************************************************/    \
                                                                             \
  DLLSYM void CLASSNAME##_c1_zapper(            /*delete a link*/            \
                                    void *link) /*link to delete*/           \
  {                                                                          \
    delete (CLASSNAME *)link;                                                \
  }

#endif
