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

#ifndef  LEPTONICA_PTRA_H
#define  LEPTONICA_PTRA_H

/*
 *  Contains the following structs:
 *      struct L_Ptra
 *      struct L_Ptraa
 *
 *  Contains definitions for:
 *      L_Ptra compaction flags for removal
 *      L_Ptra shifting flags for insert
 *      L_Ptraa accessor flags
 */


/*------------------------------------------------------------------------* 
 *                     Generic Ptr Array Structs                          *
 *------------------------------------------------------------------------*/

    /* Generic pointer array */
struct L_Ptra
{
    l_int32          nalloc;    /* size of allocated ptr array         */
    l_int32          imax;      /* greatest valid index                */
    l_int32          nactual;   /* actual number of stored elements    */
    void           **array;     /* ptr array                           */
};
typedef struct L_Ptra  L_PTRA;


    /* Array of generic pointer arrays */
struct L_Ptraa
{
    l_int32          nalloc;    /* size of allocated ptr array         */
    struct L_Ptra  **ptra;      /* array of ptra                       */
};
typedef struct L_Ptraa  L_PTRAA;



/*------------------------------------------------------------------------* 
 *                              Array flags                               *
 *------------------------------------------------------------------------*/

    /* Flags for removal from L_Ptra */
enum {
    L_NO_COMPACTION = 1,        /* null the pointer only  */
    L_COMPACTION = 2            /* compact the array      */
};

    /* Flags for insertion into L_Ptra */
enum {
    L_AUTO_DOWNSHIFT = 0,       /* choose based on number of holes        */
    L_MIN_DOWNSHIFT = 1,        /* downshifts min # of ptrs below insert  */
    L_FULL_DOWNSHIFT = 2        /* downshifts all ptrs below insert       */
};

    /* Accessor flags for L_Ptraa */
enum {
    L_HANDLE_ONLY = 0,          /* ptr to L_Ptra; caller can inspect only    */
    L_REMOVE = 1                /* caller owns; destroy or save in L_Ptraa   */
};


#endif  /* LEPTONICA_PTRA_H */
