/* -*-C-*-
 ********************************************************************************
 *
 * File:         oldlist.h  (Formerly list.h)
 * Description:  List processing procedures declarations.
 * Author:       Mark Seaman, SW Productivity
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
 * list_rest         - Macro the return the second list cell
 * pop               - Destroy one list cell
 * push              - Create one list cell and set the node and next fields
 *
 * ITERATION:
 * -----------------
 * iterate           - Macro to create a for loop to visit each cell.
 *
 * COPYING:
 * -----------------
 * reverse           - (Deprecated) Creates a backwards copy of the input list.
 *
 * LIST CELL COUNTS:
 * -----------------
 * count             - Returns the number of list cells in the list.
 * last              - Returns the last list cell.
 *
 * TRANSFORMS:             (Note: These functions all modify the input list.)
 * ----------
 * delete_d          - Removes the requested elements from the list.
 * insert            - (Deprecated) Add a new element into this spot in a list.
                       (not NIL_LIST)
 * push_last         - Add a new element onto the end of a list.
 *
 * SETS:
 * -----
 * search            - Return the pointer to the list cell whose node matches.
 *
 * CELL OPERATIONS:
 * -----------------
 * destroy           - Return all list cells in a list.
 * destroy_nodes     - Apply a function to each list cell and destroy the list.
 * set_rest          - Assign the next field in a list cell.
 *
 ***********************************************************************/

#ifndef LIST_H
#define LIST_H

/*----------------------------------------------------------------------
                  T y p e s
----------------------------------------------------------------------*/

#define NIL_LIST static_cast<LIST>(nullptr)

using int_compare = int (*)(void*, void*);
using void_dest = void (*)(void*);

struct list_rec {
  list_rec* node;
  list_rec* next;
};
using LIST = list_rec*;

/*----------------------------------------------------------------------
                  M a c r o s
----------------------------------------------------------------------*/
/* Predefinitions */
#define list_rest(l) ((l) ? (l)->next : NIL_LIST)
#define first_node(l) ((l) ? (l)->node : NIL_LIST)

/**********************************************************************
 *  i t e r a t e
 *
 *  Visit each node in the list.  Replace the old list with the list
 *  minus the head.  Continue until the list is NIL_LIST.
 **********************************************************************/

#define iterate(l) for (; (l) != NIL_LIST; (l) = list_rest(l))

/**********************************************************************
 *  s e t   r e s t
 *
 *  Change the "next" field of a list element to point to a desired place.
 *
 *  #define set_rest(l,node)        l->next = node;
 **********************************************************************/

#define set_rest(l, cell) ((l)->next = (cell))

/*----------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------*/
int count(LIST var_list);

LIST delete_d(LIST list, void* key, int_compare is_equal);

LIST destroy(LIST list);

void destroy_nodes(LIST list, void_dest destructor);

void insert(LIST list, void *node);

LIST last(LIST var_list);

LIST pop(LIST list);

LIST push(LIST list, void* element);

LIST push_last(LIST list, void* item);

LIST reverse(LIST list);

LIST search(LIST list, void* key, int_compare is_equal);

#endif
