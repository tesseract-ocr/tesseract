/* -*-C-*-
###############################################################################
#
# File:         list.c
# Description:  List processing procedures.
# Author:       Mark Seaman, Software Productivity
# Created:      Thu Jul 23 13:24:09 1987
# Modified:     Thu Dec 22 10:59:52 1988 (Mark Seaman) marks@hpgrlt
# Language:     C
# Package:      N/A
# Status:       Reusable Software Component
#
# (c) Copyright 1987, Hewlett-Packard Company.
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
#
################################################################################

* Revision 1.13  90/03/06  15:37:54  15:37:54  marks (Mark Seaman)
* Look for correct file of <malloc.h> or <stdlib.h>
*
* Revision 1.12  90/02/26  17:37:36  17:37:36  marks (Mark Seaman)
* Added pop_off and join_on
*

  This file contains a set of general purpose list manipulation routines.
  These routines can be used in a wide variety of ways to provide several
  different popular data structures. A new list can be created by declaring
  a variable of type 'LIST', and can be initialized with the value 'NIL'.
  All of these routines check for the NIL condition before dereferencing
  pointers.  NOTE:  There is a users' manual available in printed form from
  Mark Seaman at (303) 350-4492 at Greeley Hard Copy.

  To implement a STACK use:

  push       to add to the Stack                  l = push (l, (LIST) "jim");
  pop          to remove items from the Stack     l = pop (l);
  first_node        to access the head                 name = (char *) first_node (l);

  To implement a QUEUE use:

  push_last    to add to the Queue                 l = push_last (l, (LIST) "jim");
  pop                  remove items from the Queue l = pop (l);
  first_node                to access the head          name = (char *) first_node (l);

  To implement LISP like functions use:

  first_node           CAR                              x = (int) first_node (l);
  rest            CDR                              l = rest (l);
  push            CONS                             l = push (l, (LIST) this);
  last            LAST                             x = last (l);
  concat          APPEND                           l = concat (r, s);
  count           LENGTH                           x = count (l);
  search          MEMBER                           if (search (l, x, NULL))

  To implement SETS use:

  adjoin                                           l  = adjoin (l, x);
  set_union                                        l = set_union (r, s);
  intersection                                     l = intersection (r, s);
  set_difference                                   l = set_difference (r, s);
  delete                                           l = delete (s, x, NULL);
  search                                           if (search (l, x, NULL))

  To Implement Associated LISTS use:

  lpush                                            l = lpush (l, p);
  assoc                                            s = assoc (l, x);
  adelete                                          l = adelete (l, x);

  The following rules of closure exist for the functions provided.
  a = first_node (push (a, b))
  b = rest (push (a, b))
  a = push (pop (a), a))        For all a <> NIL
  a = reverse (reverse (a))

******************************************************************************/
#include "oldlist.h"
#include "structures.h"
#include <stdio.h>
#if MAC_OR_DOS
#include <stdlib.h>
#else
#include "freelist.h"
#endif

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define add_on(l,x)     l = push (l,first_node (x))
#define next_one(l)     l = rest (l)

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 *  c o u n t
 *
 *  Recursively count the elements in  a list.  Return the count.
 **********************************************************************/
int count(LIST var_list) {
  int temp = 0;

  iterate (var_list) temp += 1;
  return (temp);
}


/**********************************************************************
 *  d e l e t e    d
 *
 *  Delete all the elements out of the current list that match the key.
 *  This operation destroys the original list.  The caller will supply a
 *  routine that will compare each node to the
 *  key, and return a non-zero value when they match.  If the value
 *  NULL is supplied for is_equal, the is_key routine will be used.
 **********************************************************************/
LIST delete_d(LIST list, void *key, int_compare is_equal) {
  LIST result = NIL;
  LIST last_one = NIL;

  if (is_equal == NULL)
    is_equal = is_same;

  while (list != NIL) {
    if (!(*is_equal) (first_node (list), key)) {
      if (last_one == NIL) {
        last_one = list;
        list = rest (list);
        result = last_one;
        set_rest(last_one, NIL);
      }
      else {
        set_rest(last_one, list);
        last_one = list;
        list = rest (list);
        set_rest(last_one, NIL);
      }
    }
    else {
      list = pop (list);
    }
  }
  return (result);
}

LIST delete_d(LIST list, void *key,
              ResultCallback2<int, void*, void*>* is_equal) {
  LIST result = NIL;
  LIST last_one = NIL;

  while (list != NIL) {
    if (!(*is_equal).Run (first_node (list), key)) {
      if (last_one == NIL) {
        last_one = list;
        list = rest (list);
        result = last_one;
        set_rest(last_one, NIL);
      }
      else {
        set_rest(last_one, list);
        last_one = list;
        list = rest (list);
        set_rest(last_one, NIL);
      }
    }
    else {
      list = pop (list);
    }
  }
  return (result);
}


/**********************************************************************
 *  d e s t r o y
 *
 *  Return the space taken by a list to the heap.
 **********************************************************************/
LIST destroy(LIST list) {
  LIST next;

  while (list != NIL) {
    next = rest (list);
    free_cell(list);
    list = next;
  }
  return (NIL);
}


/**********************************************************************
 *  d e s t r o y   n o d e s
 *
 *  Return the space taken by the LISTs of a list to the heap.
 **********************************************************************/
void destroy_nodes(LIST list, void_dest destructor) {
  if (destructor == NULL)
    destructor = memfree;

  while (list != NIL) {
    (*destructor) (first_node (list));
    list = pop (list);
  }
}


/**********************************************************************
 *  i n s e r t
 *
 *  Create a list element and rearange the pointers so that the first
 *  element in the list is the second aurgment.
 **********************************************************************/
void insert(LIST list, void *node) {
  LIST element;

  if (list != NIL) {
    element = push (NIL, node);
    set_rest (element, rest (list));
    set_rest(list, element);
    node = first_node (list);
    list->node = first_node (rest (list));
    list->next->node = (LIST) node;
  }
}


/**********************************************************************
 *  i s   s a m e   n o d e
 *
 *  Compare the list node with the key value return TRUE (non-zero)
 *  if they are equivalent strings.  (Return FALSE if not)
 **********************************************************************/
int is_same_node(void *item1, void *item2) {
  return (item1 == item2);
}


/**********************************************************************
 *  i s   s a m e
 *
 *  Compare the list node with the key value return TRUE (non-zero)
 *  if they are equivalent strings.  (Return FALSE if not)
 **********************************************************************/
int is_same(void *item1, void *item2) {
  return (!strcmp ((char *) item1, (char *) item2));
}


/**********************************************************************
 *  j o i n
 *
 *  Join the two lists together. This function is similar to concat
 *  except that concat creates a new list.  This function returns the
 *  first list updated.
 **********************************************************************/
LIST join(LIST list1, LIST list2) {
  if (list1 == NIL)
    return (list2);
  set_rest (last (list1), list2);
  return (list1);
}


/**********************************************************************
 *  l a s t
 *
 *  Return the last list item (this is list type).
 **********************************************************************/
LIST last(LIST var_list) {
  while (rest (var_list) != NIL)
    var_list = rest (var_list);
  return (var_list);
}


/**********************************************************************
 *  n t h   c e l l
 *
 *  Return nth list cell in the list.
 **********************************************************************/
void *nth_cell(LIST var_list, int item_num) {
  int x = 0;
  iterate(var_list) {
    if (x++ == item_num)
      return (var_list);
  }
  return (var_list);
}


/**********************************************************************
 *  p o p
 *
 *  Return the list with the first element removed.  Destroy the space
 *  that it occupied in the list.
 **********************************************************************/
LIST pop(LIST list) {
  LIST temp;

  temp = rest (list);

  if (list != NIL) {
    free_cell(list);
  }
  return (temp);
}


/**********************************************************************
 *  p u s h
 *
 *  Create a list element.  Push the second parameter (the node) onto
 *  the first parameter (the list). Return the new list to the caller.
 **********************************************************************/
LIST push(LIST list, void *element) {
  LIST t;

  t = new_cell ();
  t->node = (LIST) element;
  set_rest(t, list);
  return (t);
}


/**********************************************************************
 *  p u s h   l a s t
 *
 *  Create a list element. Add the element onto the end of the list.
 **********************************************************************/
LIST push_last(LIST list, void *item) {
  LIST t;

  if (list != NIL) {
    t = last (list);
    t->next = push (NIL, item);
    return (list);
  }
  else
    return (push (NIL, item));
}


/**********************************************************************
 *  r e v e r s e
 *
 *  Create a new list with the elements reversed. The old list is not
 *  destroyed.
 **********************************************************************/
LIST reverse(LIST list) {
  LIST newlist = NIL;

  iterate (list) copy_first (list, newlist);
  return (newlist);
}


/**********************************************************************
 *  r e v e r s e   d
 *
 *  Create a new list with the elements reversed. The old list is
 *  destroyed.
 **********************************************************************/
LIST reverse_d(LIST list) {
  LIST result = reverse (list);
  destroy(list);
  return (result);
}


/**********************************************************************
 *  s   a d j o i n
 *
 *  Adjoin an element to an assorted list.  The original list is
 *  modified.  Returns the modified list.
 **********************************************************************/
LIST s_adjoin(LIST var_list, void *variable, int_compare compare) {
  LIST l;
  int result;

  if (compare == NULL)
    compare = (int_compare) strcmp;

  l = var_list;
  iterate(l) {
    result = (*compare) (variable, first_node (l));
    if (result == 0)
      return (var_list);
    else if (result < 0) {
      insert(l, variable);
      return (var_list);
    }
  }
  return (push_last (var_list, variable));
}


/**********************************************************************
 *   s e a r c h
 *
 *  Search list, return NIL if not found. Return the list starting from
 *  the item if found.  The compare routine "is_equal" is passed in as
 *  the third paramter to this routine.   If the value NULL is supplied
 *  for is_equal, the is_key routine will be used.
 **********************************************************************/
LIST search(LIST list, void *key, int_compare is_equal) {
  if (is_equal == NULL)
    is_equal = is_same;

  iterate (list) if ((*is_equal) (first_node (list), key))
  return (list);
  return (NIL);
}

LIST search(LIST list, void *key, ResultCallback2<int, void*, void*>* is_equal) {
  iterate (list) if ((*is_equal).Run(first_node (list), key))
  return (list);
  return (NIL);
}
