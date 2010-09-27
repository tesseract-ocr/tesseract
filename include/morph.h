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

#ifndef  LEPTONICA_MORPH_H
#define  LEPTONICA_MORPH_H

/* 
 *  morph.h
 *
 *  Contains the following structs:
 *      struct Sel
 *      struct Sela
 *      struct Kernel
 *
 *  Contains definitions for:
 *      morphological b.c. flags
 *      structuring element types
 *      runlength flags for granulometry
 *      direction flags for grayscale morphology
 *      morphological operation flags
 *      standard border size
 *      grayscale intensity scaling flags
 *      morphological tophat flags
 *      arithmetic and logical operator flags
 *      grayscale morphology selection flags
 *      distance function b.c. flags
 *      image comparison flags
 *      color content flags
 */

/*-------------------------------------------------------------------------*
 *                             Sel and Sel array                           *
 *-------------------------------------------------------------------------*/
#define  SEL_VERSION_NUMBER    1

struct Sel
{
    l_int32       sy;          /* sel height                               */
    l_int32       sx;          /* sel width                                */
    l_int32       cy;          /* y location of sel origin                 */
    l_int32       cx;          /* x location of sel origin                 */
    l_int32     **data;        /* {0,1,2}; data[i][j] in [row][col] order  */
    char         *name;        /* used to find sel by name                 */
};
typedef struct Sel SEL;

struct Sela
{
    l_int32          n;         /* number of sel actually stored           */
    l_int32          nalloc;    /* size of allocated ptr array             */
    struct Sel     **sel;       /* sel ptr array                           */
};
typedef struct Sela SELA;


/*-------------------------------------------------------------------------*
 *                                 Kernel                                  *
 *-------------------------------------------------------------------------*/
#define  KERNEL_VERSION_NUMBER    2

struct L_Kernel
{
    l_int32       sy;          /* kernel height                            */
    l_int32       sx;          /* kernel width                             */
    l_int32       cy;          /* y location of kernel origin              */
    l_int32       cx;          /* x location of kernel origin              */
    l_float32   **data;        /* data[i][j] in [row][col] order           */
};
typedef struct L_Kernel  L_KERNEL;


/*-------------------------------------------------------------------------*
 *                 Morphological boundary condition flags                  *
 *
 *  Two types of boundary condition for erosion.
 *  The global variable MORPH_BC takes on one of these two values.
 *  See notes in morph.c for usage.
 *-------------------------------------------------------------------------*/
enum {
    SYMMETRIC_MORPH_BC = 0,
    ASYMMETRIC_MORPH_BC = 1
};


/*-------------------------------------------------------------------------*
 *                        Structuring element types                        *
 *-------------------------------------------------------------------------*/
enum {
    SEL_DONT_CARE  = 0,
    SEL_HIT        = 1,
    SEL_MISS       = 2
};


/*-------------------------------------------------------------------------*
 *                  Runlength flags for granulometry                       *
 *-------------------------------------------------------------------------*/
enum {
    L_RUN_OFF = 0,
    L_RUN_ON  = 1
};


/*-------------------------------------------------------------------------*
 *         Direction flags for grayscale morphology, granulometry,         *
 *                   composable Sels, and convolution                      *
 *-------------------------------------------------------------------------*/
enum {
    L_HORIZ            = 1,
    L_VERT             = 2,
    L_BOTH_DIRECTIONS  = 3
};


/*-------------------------------------------------------------------------*
 *                   Morphological operation flags                         *
 *-------------------------------------------------------------------------*/
enum {
    L_MORPH_DILATE    = 1,
    L_MORPH_ERODE     = 2,
    L_MORPH_OPEN      = 3,
    L_MORPH_CLOSE     = 4,
    L_MORPH_HMT       = 5
};


/*-------------------------------------------------------------------------*
 *                    Grayscale intensity scaling flags                    *
 *-------------------------------------------------------------------------*/
enum {
    L_LINEAR_SCALE  = 1,
    L_LOG_SCALE     = 2
};


/*-------------------------------------------------------------------------*
 *                      Morphological tophat flags                         *
 *-------------------------------------------------------------------------*/
enum {
    L_TOPHAT_WHITE = 0,
    L_TOPHAT_BLACK = 1
};


/*-------------------------------------------------------------------------*
 *                Arithmetic and logical operator flags                    *
 *                 (use on grayscale images and Numas)                     *
 *-------------------------------------------------------------------------*/
enum {
    L_ARITH_ADD       = 1,
    L_ARITH_SUBTRACT  = 2,
    L_ARITH_MULTIPLY  = 3,   /* on numas only */
    L_ARITH_DIVIDE    = 4,   /* on numas only */
    L_UNION           = 5,   /* on numas only */
    L_INTERSECTION    = 6,   /* on numas only */
    L_SUBTRACTION     = 7,   /* on numas only */
    L_EXCLUSIVE_OR    = 8    /* on numas only */
};


/*-------------------------------------------------------------------------*
 *                        Min/max selection flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_CHOOSE_MIN = 1,           /* useful in a downscaling "erosion"  */
    L_CHOOSE_MAX = 2,           /* useful in a downscaling "dilation" */
    L_CHOOSE_MAX_MIN_DIFF = 3   /* useful in a downscaling contrast   */
};


/*-------------------------------------------------------------------------*
 *                    Distance function b.c. flags                         *
 *-------------------------------------------------------------------------*/
enum {
    L_BOUNDARY_BG = 1,  /* assume bg outside image */
    L_BOUNDARY_FG = 2   /* assume fg outside image */
};


/*-------------------------------------------------------------------------*
 *                         Image comparison flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_COMPARE_XOR = 1,
    L_COMPARE_SUBTRACT = 2,
    L_COMPARE_ABS_DIFF = 3
};


/*-------------------------------------------------------------------------*
 *                          Color content flags                            *
 *-------------------------------------------------------------------------*/
enum {
    L_MAX_DIFF_FROM_AVERAGE_2 = 1,
    L_MAX_MIN_DIFF_FROM_2 = 2,
    L_MAX_DIFF = 3
};


/*-------------------------------------------------------------------------*
 *    Standard size of border added around images for special processing   *
 *-------------------------------------------------------------------------*/
static const l_int32  ADDED_BORDER = 32;   /* pixels, not bits */


#endif  /* LEPTONICA_MORPH_H */
