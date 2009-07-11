/* -*-C-*-
 ********************************************************************************
 *
 * File:        list.h  (Formerly list.h)
 * Description:  List processing procedures declarations.
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Dec  5 15:43:17 1990 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 ********************************************************************************
 *
 * This file contains the interface for a set of general purpose list
 * manipulation routines.  For the implementation of these routines see
 * the file "list.c".
 *
 ********************************************************************************
 *
 *                            INDEX
 *                           =======
 *
 * BASICS:
 * -------
 * first_node        - Macro to return the first list node (not the cell).
 * rest              - Macro the return the second list cell
 * pop               - Destroy one list cell
 * push              - Create one list cell and set the node and next fields
 *
 * ITERATION:
 * -----------------
 * iterate           - Macro to create a for loop to visit each cell.
 * iterate_list      - Macro to visit each cell using a local variable.
 * for_each          - Applies a function to each node.
 *
 * LIST CELL COUNTS:
 * -----------------
 * count             - Returns the number of list cells in the list.
 * second_node       - Returns the second node.
 * third             - Returns the third node.
 * fourth            - Returns the fourth node.
 * fifth             - Returns the fifth node.
 * last              - Returns the last list cell.
 * pair              - Creates a list of two elements.
 *
 * COPYING:
 * -----------------
 * copy_first        - Pushes the first element from list 1 onto list 2.
 * copy              - Create a copy of a list.
 * concat            - Creates a new list that is a copy of both input lists.
 * delete_n          - Creates a new list without the chosen elements.
 * reverse           - Creates a backwards copy of the input list.
 * sort              - Use quick sort to construct a new list.
 * transform         - Creates a new list by transforming each of the nodes.
 *
 * TRANFORMS:             (Note: These functions all modify the input list.)
 * ----------
 * join              - Concatenates list 1 and list 2.
 * delete_d          - Removes the requested elements from the list.
 * transform_d       - Modifies the list by applying a function to each node.
 * insert            - Add a new element into this spot in a list. (not NIL)
 * push_last         - Add a new element onto the end of a list.
 * reverse_d         - Reverse a list and destroy the old one.
 *
 * ASSOCIATED LISTS:
 * -----------------
 * adelete           - Remove a particular entry from an associated list.
 * assoc             - Find an entry in an associated list that matches a key.
 * match             - Return the data element of an a-list entry.
 *
 * DISPLAY:
 * -----------------
 * print_cell        - Print a hex dump of a list cell.
 * show              - Displays a string and a list (using lprint).
 *
 * SETS:
 * -----
 * adjoin            - Add a new element to list if it does not exist already.
 * intersection      - Create a new list that is the set intersection.
 * set_union         - Create a new list that is the set intersection.
 * set_difference    - Create a new list that is the set difference.
 * s_adjoin          - Add an element to a sort list if it is not there.
 * s_intersection    - Set intersection on a sorted list. Modifies old list.
 * s_union           - Set intersection on a sorted list. Modifies old list.
 * search            - Return the pointer to the list cell whose node matches.
 *
 * COMPARISONS:
 * -----------------
 * is_same           - Compares each node to the key.
 * is_not_same       - Compares each node to the key.
 * is_key            - Compares first of each node to the key.
 * is_not_key        - Compares first of each node to the key.
 *
 * CELL OPERATIONS:
 * -----------------
 * new_cell          - Obtain a new list cell from the free list. Allocate.
 * free_cell         - Return a list cell to the free list.
 * destroy           - Return all list cells in a list.
 * destroy_nodes     - Apply a function to each list cell and destroy the list.
 * set_node          - Assign the node field in a list cell.
 * set_rest          - Assign the next field in a list cell.
 *
 ***********************************************************************/

#ifndef LIST_H
#define LIST_H

#include "cutil.h"
#include "callback.h"

/*----------------------------------------------------------------------
                  T y p e s
----------------------------------------------------------------------*/
#define NIL  (LIST) 0
struct list_rec
{
  struct list_rec *node;
  struct list_rec *next;
};
typedef list_rec *LIST;

/*----------------------------------------------------------------------
                  M a c r o s
----------------------------------------------------------------------*/
/* Predefinitions */
#define rest(l)  ((l) ? (l)->next : NIL)
#define first_node(l) ((l) ? (l)->node : NIL)

/**********************************************************************
 *  c o p y   f i r s t
 *
 *  Do the appropriate kind a push operation to copy the first node from
 *  one list to another.
 *
 **********************************************************************/

#define copy_first(l1,l2)  \
(l2=push(l2, first_node(l1)))

/**********************************************************************
 *  i t e r a t e
 *
 *  Visit each node in the list.  Replace the old list with the list
 *  minus the head.  Continue until the list is NIL.
 **********************************************************************/

#define iterate(l)             \
for (; (l) != NIL; (l) = rest (l))

/**********************************************************************
 *  i t e r a t e   l i s t
 *
 *  Visit each node in the list (l).  Use a local variable (x) to iterate
 *  through all of the list cells.  This macro is identical to iterate
 *  except that it does not lose the original list.
 **********************************************************************/

#define iterate_list(x,l)  \
for ((x)=(l); (x)!=0; (x)=rest(x))

/**********************************************************************
 * j o i n   o n
 *
 * Add another list onto the tail of this one.  The list given as an input
 * parameter is modified.
 **********************************************************************/

#define JOIN_ON(list1,list2)    \
((list1) = join ((list1), (list2)))

/**********************************************************************
 * p o p   o f f
 *
 * Add a cell onto the front of a list.  The list given as an input
 * parameter is modified.
 **********************************************************************/

#define pop_off(list)    \
((list) = pop (list))

/**********************************************************************
 * p u s h   o n
 *
 * Add a cell onto the front of a list.  The list given as an input
 * parameter is modified.
 **********************************************************************/

#define push_on(list,thing)    \
((list) = push (list, (LIST) (thing)))

/**********************************************************************
 *  s e c o n d
 *
 *  Return the contents of the second list element.
 *
 *  #define second_node(l)    first_node (rest (l))
 **********************************************************************/

#define second_node(l)              \
first_node (rest (l))

/**********************************************************************
 *  s e t   r e s t
 *
 *  Change the "next" field of a list element to point to a desired place.
 *
 *  #define set_rest(l,node)        l->next = node;
 **********************************************************************/

#define set_rest(l,cell)\
((l)->next = (cell))

/**********************************************************************
 *  t h i r d
 *
 *  Return the contents of the third list element.
 *
 *  #define third(l)     first_node (rest (rest (l)))
 **********************************************************************/

#define third(l)               \
first_node (rest (rest (l)))

/*----------------------------------------------------------------------
          Public Funtion Prototypes
----------------------------------------------------------------------*/
int count(LIST var_list);

LIST delete_d(LIST list, void *key, int_compare is_equal);

LIST delete_d(LIST list, void *key,
              ResultCallback2<int, void*, void*>* is_equal);

LIST destroy(LIST list);

void destroy_nodes(LIST list, void_dest destructor);

void insert(LIST list, void *node);

int is_same_node(void *item1, void *item2);

int is_same(void *item1, void *item2);

LIST join(LIST list1, LIST list2);

LIST last(LIST var_list);

void *nth_cell(LIST var_list, int item_num);

LIST pop(LIST list);

LIST push(LIST list, void *element);

LIST push_last(LIST list, void *item);

LIST reverse(LIST list);

LIST reverse_d(LIST list);

LIST s_adjoin(LIST var_list, void *variable, int_compare compare);

LIST search(LIST list, void *key, int_compare is_equal);

LIST search(LIST list, void *key, ResultCallback2<int, void*, void*>*);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif

typedef void  (*destructor)   _ARGS((LIST l));

typedef LIST  (*list_proc)    _ARGS((LIST a));

int count
_ARGS((LIST var_list));

LIST delete_d
_ARGS((LIST list,
    LIST key,
    int_compare is_equal));

LIST destroy
_ARGS((LIST list));

LIST destroy_nodes
_ARGS((LIST list,
    void_dest destructor));

void insert
_ARGS((LIST list,
    LIST node));

int is_same_node
_ARGS((LIST s1,
    LIST s2));

int is_same
_ARGS((LIST s1,
    LIST s2));

LIST join
_ARGS((LIST list1,
    LIST list2));

LIST last
_ARGS((LIST var_list));

LIST nth_cell
_ARGS((LIST var_list,
    int item_num));

LIST pop
_ARGS((LIST list));

LIST push
_ARGS((LIST list,
    LIST element));

LIST push_last
_ARGS((LIST list,
    LIST item));

LIST reverse
_ARGS((LIST list));

LIST reverse_d
_ARGS((LIST list));

LIST s_adjoin
_ARGS((LIST var_list,
    LIST variable,
    int_compare compare));

LIST search
_ARGS((LIST list,
    LIST key,
    int_compare is_equal));

#undef _ARGS
*/
#endif
