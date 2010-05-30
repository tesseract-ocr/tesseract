/**********************************************************************
 * File:        elst.h  (Formerly elist.h)
 * Description: Embedded list module include file.
 * Author:      Phil Cheatle
 * Created:     Mon Jan 07 08:35:34 GMT 1991
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

#ifndef ELST_H
#define ELST_H

#include <stdio.h>
#include "host.h"
#include "serialis.h"
#include "lsterr.h"

class ELIST_ITERATOR;

/**********************************************************************
This module implements list classes and iterators.
The following list types and iterators are provided:

  List type        List Class      Iterator Class     Element Class
  ---------         ----------      --------------      -------------

    Embedded list       ELIST
              ELIST_ITERATOR
              ELIST_LINK
    (Single linked)

    Embedded list       ELIST2
              ELIST2_ITERATOR
              ELIST2_LINK
    (Double linked)

    Cons List           CLIST
              CLIST_ITERATOR
              CLIST_LINK
    (Single linked)

    Cons List           CLIST2
              CLIST2_ITERATOR
              CLIST2_LINK
    (Double linked)

An embedded list is where the list pointers are provided by a generic class.
Data types to be listed inherit from the generic class.  Data is thus linked
in only ONE list at any one time.

A cons list has a separate structure for a "cons cell".  This contains the
list pointer(s) AND a pointer to the data structure held on the list.  A
structure can be on many cons lists at the same time, and the structure does
not need to inherit from any generic class in order to be on the list.

The implementation of lists is very careful about space and speed overheads.
This is why many embedded lists are provided. The same concerns mean that
in-line type coercion is done, rather than use virtual functions.  This is
cumbersome in that each data type to be listed requires its own iterator and
list class - though macros can gererate these.  It also prevents heterogenous
lists.
**********************************************************************/

/**********************************************************************
 *                          CLASS - ELIST_LINK
 *
 *                          Generic link class for singly linked lists with embedded links
 *
 *  Note:  No destructor - elements are assumed to be destroyed EITHER after
 *  they have been extracted from a list OR by the ELIST destructor which
 *  walks the list.
 **********************************************************************/

class DLLSYM ELIST_LINK
{
  friend class ELIST_ITERATOR;
  friend class ELIST;

  ELIST_LINK *next;

  public:
    ELIST_LINK() {
      next = NULL;
    }
    //constructor

    ELIST_LINK(                       //copy constructor
               const ELIST_LINK &) {  //dont copy link
      next = NULL;
    }

    void operator= (             //dont copy links
    const ELIST_LINK &) {
      next = NULL;
    }

    void serialise_asc(  //serialise to ascii
                       FILE *f);
    void de_serialise_asc(  //de-serialise from ascii
                          FILE *f);

    /* NOTE that none of the serialise member functions are required for
    ELIST_LINKS as they are never serialised.  (We demand that the derived
    class terminates recursion - just to make sure that it defines the member
    functions anyway.)
    */
};

/**********************************************************************
 * CLASS - ELIST
 *
 * Generic list class for singly linked lists with embedded links
 **********************************************************************/

class DLLSYM ELIST
{
  friend class ELIST_ITERATOR;

  ELIST_LINK *last;              //End of list
  //(Points to head)
  ELIST_LINK *First() {  // return first
    return last ? last->next : NULL;
  }

  public:
    ELIST() {  //constructor
      last = NULL;
    }

    virtual ~ELIST() {
      // Empty
    }

    void internal_clear (        //destroy all links
                                 //ptr to zapper functn
      void (*zapper) (ELIST_LINK *));

    bool empty() {  //is list empty?
      return !last;
    }

    bool singleton() {
      return last ? (last == last->next) : FALSE;
    }

    void shallow_copy(                     //dangerous!!
                      ELIST *from_list) {  //beware destructors!!
      last = from_list->last;
    }

                                 //ptr to copier functn
    void internal_deep_copy (ELIST_LINK * (*copier) (ELIST_LINK *),
      const ELIST * list);       //list being copied

    void assign_to_sublist(                           //to this list
                           ELIST_ITERATOR *start_it,  //from list start
                           ELIST_ITERATOR *end_it);   //from list end

    inT32 length();  //# elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

    // Assuming list has been sorted already, insert new_link to
    // keep the list sorted according to the same comparison function.
    // Comparision function is the same as used by sort, i.e. uses double
    // indirection. Time is O(1) to add to beginning or end.
    // Time is linear to add pre-sorted items to an empty list.
    void add_sorted(int comparator(const void*, const void*),
                    ELIST_LINK* new_link);

    void internal_dump (         //serialise each elem
      FILE * f,                  //to this file
      void element_serialiser (  //using this function
      FILE *, ELIST_LINK *));

    void internal_de_dump (      //de_serial each elem
      FILE * f,                  //from this file
                                 //using this function
      ELIST_LINK * element_de_serialiser (
      FILE *));

    void prep_serialise();  //change last to count

    /*  Note that dump() and de_dump() are not required as calls to dump/de_dump a
      list class should be handled by a class derived from this.

      make_serialise is not required for a similar reason.
    */
};

/***********************************************************************
 *                          CLASS - ELIST_ITERATOR
 *
 *                          Generic iterator class for singly linked lists with embedded links
 **********************************************************************/

class DLLSYM ELIST_ITERATOR
{
  friend void ELIST::assign_to_sublist(ELIST_ITERATOR *, ELIST_ITERATOR *);

  ELIST *list;                   //List being iterated
  ELIST_LINK *prev;              //prev element
  ELIST_LINK *current;           //current element
  ELIST_LINK *next;              //next element
  bool ex_current_was_last;     //current extracted
  //was end of list
  bool ex_current_was_cycle_pt; //current extracted
  //was cycle point
  ELIST_LINK *cycle_pt;          //point we are cycling
  //the list to.
  bool started_cycling;         //Have we moved off
  //the start?

  ELIST_LINK *extract_sublist(                            //from this current...
                              ELIST_ITERATOR *other_it);  //to other current

  public:
    ELIST_ITERATOR() {  //constructor
      list = NULL;
    }                            //unassigned list

    ELIST_ITERATOR(  //constructor
                   ELIST *list_to_iterate);

    void set_to_list(  //change list
                     ELIST *list_to_iterate);

    void add_after_then_move(                        //add after current &
                             ELIST_LINK *new_link);  //move to new

    void add_after_stay_put(                        //add after current &
                            ELIST_LINK *new_link);  //stay at current

    void add_before_then_move(                        //add before current &
                              ELIST_LINK *new_link);  //move to new

    void add_before_stay_put(                        //add before current &
                             ELIST_LINK *new_link);  //stay at current

    void add_list_after(                      //add a list &
                        ELIST *list_to_add);  //stay at current

    void add_list_before(                      //add a list &
                         ELIST *list_to_add);  //move to it 1st item

    ELIST_LINK *data() {  //get current data
    #ifndef NDEBUG
      if (!list)
        NO_LIST.error ("ELIST_ITERATOR::data", ABORT, NULL);
      if (!current)
        NULL_DATA.error ("ELIST_ITERATOR::data", ABORT, NULL);
    #endif
      return current;
    }

    ELIST_LINK *data_relative(               //get data + or - ...
                              inT8 offset);  //offset from current

    ELIST_LINK *forward();  //move to next element

    ELIST_LINK *extract();  //remove from list

    ELIST_LINK *move_to_first();  //go to start of list

    ELIST_LINK *move_to_last();  //go to end of list

    void mark_cycle_pt();  //remember current

    bool empty() {  //is list empty?
    #ifndef NDEBUG
      if (!list)
        NO_LIST.error ("ELIST_ITERATOR::empty", ABORT, NULL);
    #endif
      return list->empty ();
    }

    bool current_extracted() {  //current extracted?
      return !current;
    }

    bool at_first();  //Current is first?

    bool at_last();  //Current is last?

    bool cycled_list();  //Completed a cycle?

    void add_to_end(                        //add at end &
                    ELIST_LINK *new_link);  //dont move

    void exchange(                            //positions of 2 links
                  ELIST_ITERATOR *other_it);  //other iterator

    inT32 length();  //# elements in list

    void sort (                  //sort elements
      int comparator (           //comparison routine
      const void *, const void *));

};

/***********************************************************************
 *                          ELIST_ITERATOR::set_to_list
 *
 *  (Re-)initialise the iterator to point to the start of the list_to_iterate
 *  over.
 **********************************************************************/

inline void ELIST_ITERATOR::set_to_list(  //change list
                                        ELIST *list_to_iterate) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::set_to_list", ABORT, NULL);
  if (!list_to_iterate)
    BAD_PARAMETER.error ("ELIST_ITERATOR::set_to_list", ABORT,
      "list_to_iterate is NULL");
  #endif

  list = list_to_iterate;
  prev = list->last;
  current = list->First ();
  next = current ? current->next : NULL;
  cycle_pt = NULL;               //await explicit set
  started_cycling = FALSE;
  ex_current_was_last = FALSE;
  ex_current_was_cycle_pt = FALSE;
}


/***********************************************************************
 *                          ELIST_ITERATOR::ELIST_ITERATOR
 *
 *  CONSTRUCTOR - set iterator to specified list;
 **********************************************************************/

inline ELIST_ITERATOR::ELIST_ITERATOR(ELIST *list_to_iterate) {
  set_to_list(list_to_iterate);
}


/***********************************************************************
 *                          ELIST_ITERATOR::add_after_then_move
 *
 *  Add a new element to the list after the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST_ITERATOR::add_after_then_move(  // element to add
                                                ELIST_LINK *new_element) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_after_then_move", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_after_then_move", ABORT, NULL);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_after_then_move", ABORT,
      "new_element is NULL");
  if (new_element->next)
    STILL_LINKED.error ("ELIST_ITERATOR::add_after_then_move", ABORT, NULL);
  #endif

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
 *                          ELIST_ITERATOR::add_after_stay_put
 *
 *  Add a new element to the list after the current element but do not move
 *  the iterator to the new element.
 **********************************************************************/

inline void ELIST_ITERATOR::add_after_stay_put(  // element to add
                                               ELIST_LINK *new_element) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_after_stay_put", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_after_stay_put", ABORT, NULL);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_after_stay_put", ABORT,
      "new_element is NULL");
  if (new_element->next)
    STILL_LINKED.error ("ELIST_ITERATOR::add_after_stay_put", ABORT, NULL);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = FALSE;
    current = NULL;
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
        ex_current_was_last = FALSE;
      }
    }
    next = new_element;
  }
}


/***********************************************************************
 *                          ELIST_ITERATOR::add_before_then_move
 *
 *  Add a new element to the list before the current element and move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST_ITERATOR::add_before_then_move(  // element to add
                                                 ELIST_LINK *new_element) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_before_then_move", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_before_then_move", ABORT, NULL);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_before_then_move", ABORT,
      "new_element is NULL");
  if (new_element->next)
    STILL_LINKED.error ("ELIST_ITERATOR::add_before_then_move", ABORT, NULL);
  #endif

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
 *                          ELIST_ITERATOR::add_before_stay_put
 *
 *  Add a new element to the list before the current element but dont move the
 *  iterator to the new element.
 **********************************************************************/

inline void ELIST_ITERATOR::add_before_stay_put(  // element to add
                                                ELIST_LINK *new_element) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_before_stay_put", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_before_stay_put", ABORT, NULL);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_before_stay_put", ABORT,
      "new_element is NULL");
  if (new_element->next)
    STILL_LINKED.error ("ELIST_ITERATOR::add_before_stay_put", ABORT, NULL);
  #endif

  if (list->empty ()) {
    new_element->next = new_element;
    list->last = new_element;
    prev = next = new_element;
    ex_current_was_last = TRUE;
    current = NULL;
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
 *                          ELIST_ITERATOR::add_list_after
 *
 *  Insert another list to this list after the current element but dont move the
 *  iterator.
 **********************************************************************/

inline void ELIST_ITERATOR::add_list_after(ELIST *list_to_add) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_list_after", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_list_after", ABORT, NULL);
  if (!list_to_add)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_list_after", ABORT,
      "list_to_add is NULL");
  #endif

  if (!list_to_add->empty ()) {
    if (list->empty ()) {
      list->last = list_to_add->last;
      prev = list->last;
      next = list->First ();
      ex_current_was_last = TRUE;
      current = NULL;
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
          ex_current_was_last = FALSE;
        }
        list_to_add->last->next = next;
        next = prev->next;
      }
    }
    list_to_add->last = NULL;
  }
}


/***********************************************************************
 *                          ELIST_ITERATOR::add_list_before
 *
 *  Insert another list to this list before the current element. Move the
 *  iterator to the start of the inserted elements
 *  iterator.
 **********************************************************************/

inline void ELIST_ITERATOR::add_list_before(ELIST *list_to_add) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_list_before", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_list_before", ABORT, NULL);
  if (!list_to_add)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_list_before", ABORT,
      "list_to_add is NULL");
  #endif

  if (!list_to_add->empty ()) {
    if (list->empty ()) {
      list->last = list_to_add->last;
      prev = list->last;
      current = list->First ();
      next = current->next;
      ex_current_was_last = FALSE;
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
    list_to_add->last = NULL;
  }
}


/***********************************************************************
 *                          ELIST_ITERATOR::extract
 *
 *  Do extraction by removing current from the list, returning it to the
 *  caller, but NOT updating the iterator.  (So that any calling loop can do
 *  this.)   The iterator's current points to NULL.  If the extracted element
 *  is to be deleted, this is the callers responsibility.
 **********************************************************************/

inline ELIST_LINK *ELIST_ITERATOR::extract() {
  ELIST_LINK *extracted_link;

  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::extract", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::extract", ABORT, NULL);
  if (!current)                  //list empty or
                                 //element extracted
    NULL_CURRENT.error ("ELIST_ITERATOR::extract",
      ABORT, NULL);
  #endif

  if (list->singleton()) {
    // Special case where we do need to change the iterator.
    prev = next = list->last = NULL;
  } else {
    prev->next = next;           //remove from list

    if (current == list->last) {
      list->last = prev;
      ex_current_was_last = TRUE;
    } else {
      ex_current_was_last = FALSE;
    }
  }
  // Always set ex_current_was_cycle_pt so an add/forward will work in a loop.
  ex_current_was_cycle_pt = (current == cycle_pt) ? TRUE : FALSE;
  extracted_link = current;
  extracted_link->next = NULL;   //for safety
  current = NULL;
  return extracted_link;
}


/***********************************************************************
 *                          ELIST_ITERATOR::move_to_first()
 *
 *  Move current so that it is set to the start of the list.
 *  Return data just in case anyone wants it.
 **********************************************************************/

inline ELIST_LINK *ELIST_ITERATOR::move_to_first() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::move_to_first", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::move_to_first", ABORT, NULL);
  #endif

  current = list->First ();
  prev = list->last;
  next = current ? current->next : NULL;
  return current;
}


/***********************************************************************
 *                          ELIST_ITERATOR::mark_cycle_pt()
 *
 *  Remember the current location so that we can tell whether we've returned
 *  to this point later.
 *
 *  If the current point is deleted either now, or in the future, the cycle
 *  point will be set to the next item which is set to current.  This could be
 *  by a forward, add_after_then_move or add_after_then_move.
 **********************************************************************/

inline void ELIST_ITERATOR::mark_cycle_pt() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::mark_cycle_pt", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::mark_cycle_pt", ABORT, NULL);
  #endif

  if (current)
    cycle_pt = current;
  else
    ex_current_was_cycle_pt = TRUE;
  started_cycling = FALSE;
}


/***********************************************************************
 *                          ELIST_ITERATOR::at_first()
 *
 *  Are we at the start of the list?
 *
 **********************************************************************/

inline bool ELIST_ITERATOR::at_first() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::at_first", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::at_first", ABORT, NULL);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->First ()) || ((current == NULL) &&
    (prev == list->last) &&      //NON-last pt between
    !ex_current_was_last));      //first and last
}


/***********************************************************************
 *                          ELIST_ITERATOR::at_last()
 *
 *  Are we at the end of the list?
 *
 **********************************************************************/

inline bool ELIST_ITERATOR::at_last() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::at_last", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::at_last", ABORT, NULL);
  #endif

                                 //we're at a deleted
  return ((list->empty ()) || (current == list->last) || ((current == NULL) &&
    (prev == list->last) &&      //last point between
    ex_current_was_last));       //first and last
}


/***********************************************************************
 *                          ELIST_ITERATOR::cycled_list()
 *
 *  Have we returned to the cycle_pt since it was set?
 *
 **********************************************************************/

inline bool ELIST_ITERATOR::cycled_list() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::cycled_list", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::cycled_list", ABORT, NULL);
  #endif

  return ((list->empty ()) || ((current == cycle_pt) && started_cycling));

}


/***********************************************************************
 *                          ELIST_ITERATOR::length()
 *
 *  Return the length of the list
 *
 **********************************************************************/

inline inT32 ELIST_ITERATOR::length() {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::length", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::length", ABORT, NULL);
  #endif

  return list->length ();
}


/***********************************************************************
 *                          ELIST_ITERATOR::sort()
 *
 *  Sort the elements of the list, then reposition at the start.
 *
 **********************************************************************/

inline void
ELIST_ITERATOR::sort (           //sort elements
int comparator (                 //comparison routine
const void *, const void *)) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::sort", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::sort", ABORT, NULL);
  #endif

  list->sort (comparator);
  move_to_first();
}


/***********************************************************************
 *                          ELIST_ITERATOR::add_to_end
 *
 *  Add a new element to the end of the list without moving the iterator.
 *  This is provided because a single linked list cannot move to the last as
 *  the iterator couldn't set its prev pointer.  Adding to the end is
 *  essential for implementing
              queues.
**********************************************************************/

inline void ELIST_ITERATOR::add_to_end(  // element to add
                                       ELIST_LINK *new_element) {
  #ifndef NDEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::add_to_end", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::add_to_end", ABORT, NULL);
  if (!new_element)
    BAD_PARAMETER.error ("ELIST_ITERATOR::add_to_end", ABORT,
      "new_element is NULL");
  if (new_element->next)
    STILL_LINKED.error ("ELIST_ITERATOR::add_to_end", ABORT, NULL);
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
      list->last->next = new_element;
      list->last = new_element;
    }
  }
}


/***********************************************************************
 ********************    MACROS    **************************************
 ***********************************************************************/

/***********************************************************************
  QUOTE_IT   MACRO DEFINITION
  ===========================
Replace <parm> with "<parm>".  <parm> may be an arbitrary number of tokens
***********************************************************************/

#define QUOTE_IT( parm ) #parm

/***********************************************************************
  ELISTIZE( CLASSNAME ) MACROS
  ============================

CLASSNAME is assumed to be the name of a class which has a baseclass of
ELIST_LINK.

NOTE:  Because we dont use virtual functions in the list code, the list code
will NOT work correctly for classes derived from this.

The macros generate:
  - An element deletion function:      CLASSNAME##_zapper
  - An element serialiser function"    CLASSNAME##_serialiser
  - An element de-serialiser function" CLASSNAME##_de_serialiser
  - An E_LIST subclass: CLASSNAME##_LIST
  - An E_LIST_ITERATOR subclass:       CLASSNAME##_IT

NOTE: Generated names are DELIBERATELY designed to clash with those for
ELIST2IZE but NOT with those for CLISTIZE and CLIST2IZE

Four macros are provided: ELISTIZE, ELISTIZE_S, ELISTIZEH and ELISTIZEH_S
The ...IZEH macros just define the class names for use in .h files
The ...IZE macros define the code use in .c files
The _S versions define lists which can be serialised.  They assume that
the make_serialise() macro is used in the list element class derived from
ELIST_LINK to define serialise() and de_serialise() members for the list
elements.
***********************************************************************/

/***********************************************************************
  ELISTIZEH( CLASSNAME )  and  ELISTIZEH_S( CLASSNAME ) MACROS

These macros are constructed from 3 fragments ELISTIZEH_A, ELISTIZEH_B and
ELISTIZEH_C.  ELISTIZEH is simply a concatenation of these parts.
ELISTIZEH_S has some additional bits thrown in the gaps.
***********************************************************************/

#define ELISTIZEH_A(CLASSNAME)                                               \
                                                                             \
extern DLLSYM void CLASSNAME##_zapper(ELIST_LINK* link);

#define ELISTIZEH_B(CLASSNAME)                                               \
                                                                             \
/***********************************************************************        \
*                           CLASS - CLASSNAME##_LIST                                                                    \
*                                                                                                       \
*                           List class for class CLASSNAME                                                          \
*                                                                                                       \
**********************************************************************/         \
                                                                                                        \
class DLLSYM                CLASSNAME##_LIST : public ELIST                         \
{                                                                                                       \
public:                                                                                             \
                            CLASSNAME##_LIST():ELIST() {}\
                                                        /* constructor */       \
                                                                                                        \
                            CLASSNAME##_LIST(           /* dont construct */ \
    const CLASSNAME##_LIST&)                            /*by initial assign*/\
    { DONT_CONSTRUCT_LIST_BY_COPY.error( QUOTE_IT( CLASSNAME##_LIST ),      \
                                                        ABORT, NULL ); }                            \
                                                                                                        \
void                        clear()                     /* delete elements */\
    { ELIST::internal_clear( &CLASSNAME##_zapper ); }                               \
                                                                                                        \
                                    ~CLASSNAME##_LIST() /* destructor */        \
    { clear(); }                                                                                \
\
/* Become a deep copy of src_list*/ \
void deep_copy(const CLASSNAME##_LIST* src_list, \
               CLASSNAME* (*copier)(const CLASSNAME*)); \
\
void                        operator=(                  /* prevent assign */    \
    const CLASSNAME##_LIST&)                                                                \
    { DONT_ASSIGN_LISTS.error( QUOTE_IT( CLASSNAME##_LIST ),                        \
                                            ABORT, NULL ); }

#define ELISTIZEH_C( CLASSNAME )                                                        \
};                                                                                                      \
                                                                                                        \
                                                                                                        \
                                                                                                        \
/***********************************************************************        \
*                           CLASS - CLASSNAME##_IT                                                                      \
*                                                                                                       \
*                           Iterator class for class CLASSNAME##_LIST                                           \
*                                                                                                       \
*  Note: We don't need to coerce pointers to member functions input             \
*  parameters as these are automatically converted to the type of the base      \
*  type. ("A ptr to a class may be converted to a pointer to a public base      \
*  class of that class")                                                                        \
**********************************************************************/         \
                                                                                                        \
class DLLSYM                CLASSNAME##_IT : public ELIST_ITERATOR                  \
{                                                                                                       \
public:                                                                                             \
                                CLASSNAME##_IT():ELIST_ITERATOR(){}                     \
                                                                                                        \
                                CLASSNAME##_IT(                                             \
CLASSNAME##_LIST*           list):ELIST_ITERATOR(list){}                                \
                                                                                                        \
    CLASSNAME*          data()                                                          \
        { return (CLASSNAME*) ELIST_ITERATOR::data(); }                             \
                                                                                                        \
    CLASSNAME*          data_relative(                                                  \
    inT8                    offset)                                                         \
        { return (CLASSNAME*) ELIST_ITERATOR::data_relative( offset ); }        \
                                                                                                        \
    CLASSNAME*          forward()                                                       \
        { return (CLASSNAME*) ELIST_ITERATOR::forward(); }                          \
                                                                                                        \
    CLASSNAME*          extract()                                                       \
        { return (CLASSNAME*) ELIST_ITERATOR::extract(); }                          \
                                                                                                        \
    CLASSNAME*          move_to_first()                                             \
        { return (CLASSNAME*) ELIST_ITERATOR::move_to_first(); }                    \
                                                                                                        \
    CLASSNAME*          move_to_last()                                                  \
        { return (CLASSNAME*) ELIST_ITERATOR::move_to_last(); }                 \
};

#define ELISTIZEH( CLASSNAME )                                                      \
                                                                                                        \
ELISTIZEH_A( CLASSNAME )                                                                        \
                                                                                                        \
ELISTIZEH_B( CLASSNAME )                                                                        \
                                                                                                        \
ELISTIZEH_C( CLASSNAME )

#define ELISTIZEH_S( CLASSNAME )                                                        \
                                                                                                        \
ELISTIZEH_A( CLASSNAME )                                                                        \
                                                                                                        \
extern DLLSYM void          CLASSNAME##_serialiser(                                     \
FILE*                       f,                                                                  \
ELIST_LINK*                 element);                                                       \
                                                                                                        \
extern DLLSYM ELIST_LINK*   CLASSNAME##_de_serialiser(                                  \
FILE*                       f);                                                             \
                                                                                                        \
ELISTIZEH_B( CLASSNAME )                                                                        \
                                                                                                        \
    void                    dump(                       /* dump to file */   \
    FILE*                   f)                                                                  \
    { ELIST::internal_dump( f, &CLASSNAME##_serialiser );}                      \
                                                                                                        \
    void                    de_dump(                    /* get from file */  \
    FILE*                   f)                                                                  \
    { ELIST::internal_de_dump( f, &CLASSNAME##_de_serialiser );}                \
                                                                                                        \
    void                    serialise_asc(              /*dump to ascii*/       \
    FILE*                   f);                                                                 \
    void                    de_serialise_asc(           /*de-dump from ascii*/\
    FILE*                   f);                                                                 \
                                                                                                        \
make_serialise( CLASSNAME##_LIST )                                                  \
                                                                                                        \
ELISTIZEH_C( CLASSNAME )

/***********************************************************************
  ELISTIZE( CLASSNAME )  and   ELISTIZE_S( CLASSNAME )  MACROS
ELISTIZE_S is a simple extension to ELISTIZE
***********************************************************************/

#define ELISTIZE(CLASSNAME)                                                 \
                                                                            \
/***********************************************************************    \
*                           CLASSNAME##_zapper                              \
*                                                                           \
*  A function which can delete a CLASSNAME element.  This is passed to the  \
*  generic clear list member function so that when a list is cleared the    \
*  elements on the list are properly destroyed from the base class, even    \
*  though we dont use a virtual destructor function.                        \
**********************************************************************/     \
                                                                            \
DLLSYM void CLASSNAME##_zapper(ELIST_LINK* link) {                          \
  delete reinterpret_cast<CLASSNAME*>(link);                                \
}                                                                           \
                                                                            \
/* Become a deep copy of src_list*/                                         \
void CLASSNAME##_LIST::deep_copy(const CLASSNAME##_LIST* src_list,          \
               CLASSNAME* (*copier)(const CLASSNAME*)) {                    \
                                                                            \
  CLASSNAME##_IT from_it(const_cast<CLASSNAME##_LIST*>(src_list));          \
  CLASSNAME##_IT to_it(this);                                               \
                                                                            \
  for (from_it.mark_cycle_pt(); !from_it.cycled_list(); from_it.forward())  \
    to_it.add_after_then_move((*copier)(from_it.data()));                   \
}

#define ELISTIZE_S(CLASSNAME)                                               \
                                                                            \
ELISTIZE(CLASSNAME)                                                         \
                                                                            \
void                  CLASSNAME##_LIST::serialise_asc(FILE* f) {            \
  CLASSNAME##_IT      it(this);                                             \
                                                                            \
  serialise_INT32(f, length());                                             \
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())                 \
      it.data()->serialise_asc(f);                /*serialise the list*/    \
}                                                                           \
                                                                            \
void                  CLASSNAME##_LIST::de_serialise_asc(FILE* f) {         \
  inT32               len;                        /*length to retrive*/     \
  CLASSNAME##_IT      it;                                                   \
  CLASSNAME*          new_elt = NULL;               /*list element*/        \
                                                                            \
  len = de_serialise_INT32(f);                                              \
  it.set_to_list(this);                                                     \
  for (; len > 0; len--) {                                                  \
    new_elt = new CLASSNAME;                                                \
    new_elt->de_serialise_asc(f);                                           \
    it.add_to_end(new_elt);                     /*put on the list*/         \
  }                                                                         \
  return;                                                                   \
}                                                                           \
                                                                            \
                                                                            \
/***********************************************************************   \
*                           CLASSNAME##_serialiser                         \
*                                                                          \
*  A function which can serialise an element                               \
*  This is passed to the generic dump member function so that when a list is  \
*  serialised the elements on the list are properly serialised.            \
**********************************************************************/    \
                                                                           \
DLLSYM void CLASSNAME##_serialiser(FILE* f, ELIST_LINK* element) {         \
  reinterpret_cast<CLASSNAME*>(element)->serialise(f);                     \
}                                                                          \
                                                                           \
                                                                           \
                                                                           \
/***********************************************************************   \
*                           CLASSNAME##_de_serialiser                      \
*                                                                          \
*  A function which can de-serialise an element                            \
*  This is passed to the generic de-dump member function so that when a list  \
*  is de-serialised the elements on the list are properly de-serialised.   \
**********************************************************************/    \
                                                                           \
DLLSYM ELIST_LINK* CLASSNAME##_de_serialiser(FILE* f) {                  \
  return (ELIST_LINK*) CLASSNAME::de_serialise(f);                       \
}
#endif
