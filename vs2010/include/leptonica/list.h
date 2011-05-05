
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifndef  LEPTONICA_LIST_H
#define  LEPTONICA_LIST_H

/*
 *   list.h
 *
 *       Cell for double-linked lists
 *
 *       This allows composition of a list of cells with 
 *           prev, next and data pointers.  Generic data
 *           structures hang on the list cell data pointers.
 *
 *       The list is not circular because that would add much
 *           complexity in traversing the list under general
 *           conditions where list cells can be added and removed.
 *           The only disadvantage of not having the head point to
 *           the last cell is that the list must be traversed to
 *           find its tail.  However, this traversal is fast, and
 *           the listRemoveFromTail() function updates the tail
 *           so there is no searching overhead with repeated use.
 *
 *       The list macros are used to run through a list, and their
 *       use is encouraged.  They are invoked, e.g., as
 *
 *             DLLIST  *head, *elem;
 *             ...
 *             L_BEGIN_LIST_FORWARD(head, elem)
 *                 <do something with elem and/or elem->data >
 *             L_END_LIST
 *
 */

struct DoubleLinkedList
{
    struct DoubleLinkedList    *prev;
    struct DoubleLinkedList    *next;
    void                       *data;
};
typedef struct DoubleLinkedList    DLLIST;


    /*  Simple list traverse macros */
#define L_BEGIN_LIST_FORWARD(head, element) \
        { \
        DLLIST   *_leptvar_nextelem_; \
        for ((element) = (head); (element); (element) = _leptvar_nextelem_) { \
            _leptvar_nextelem_ = (element)->next;


#define L_BEGIN_LIST_REVERSE(tail, element) \
        { \
        DLLIST   *_leptvar_prevelem_; \
        for ((element) = (tail); (element); (element) = _leptvar_prevelem_) { \
            _leptvar_prevelem_ = (element)->prev;


#define L_END_LIST    }}


#endif  /* LEPTONICA_LIST_H */
