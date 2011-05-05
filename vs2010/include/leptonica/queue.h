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

#ifndef  LEPTONICA_QUEUE_H
#define  LEPTONICA_QUEUE_H

/*
 *  queue.h
 *
 *      Expandable pointer queue for arbitrary void* data.
 *
 *      The L_Queue is a fifo that implements a queue of void* pointers.
 *      It can be used to hold a queue of any type of struct.
 *
 *      Internally, it maintains two counters:
 *          nhead:  location of head (in ptrs) from the beginning
 *                  of the array.
 *          nelem:  number of ptr elements stored in the queue.
 *
 *      The element at the head of the queue, which is the next to
 *      be removed, is array[nhead].  The location at the tail of the
 *      queue to which the next element will be added is
 *      array[nhead + nelem].
 *               
 *      As items are added to the queue, nelem increases.
 *      As items are removed, nhead increases and nelem decreases.
 *      Any time the tail reaches the end of the allocated array,
 *      all the pointers are shifted to the left, so that the head
 *      is at the beginning of the array.
 *      If the array becomes more than 3/4 full, it doubles in size.
 *
 *      The auxiliary stack can be used in a wrapper for re-using
 *      items popped from the queue.  It is not made by default.
 *
 *      For further implementation details, see queue.c.
 */

struct L_Queue
{
    l_int32          nalloc;     /* size of allocated ptr array            */
    l_int32          nhead;      /* location of head (in ptrs) from the    */
                                 /* beginning of the array                 */
    l_int32          nelem;      /* number of elements stored in the queue */
    void           **array;      /* ptr array                              */
    struct L_Stack  *stack;      /* auxiliary stack                        */

};
typedef struct L_Queue L_QUEUE;


#endif  /* LEPTONICA_QUEUE_H */
