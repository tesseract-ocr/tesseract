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

#ifndef  LEPTONICA_BMF_H
#define  LEPTONICA_BMF_H

/* 
 *  bmf.h
 *
 *     Simple data structure to hold bitmap fonts and related data
 */

    /* Constants for deciding when text block is divided into paragraphs */
enum {
    SPLIT_ON_LEADING_WHITE = 1,    /* tab or space at beginning of line   */
    SPLIT_ON_BLANK_LINE    = 2,    /* newline with optional white space   */
    SPLIT_ON_BOTH          = 3     /* leading white space or newline      */
};


struct L_Bmf
{
    struct Pixa  *pixa;        /* pixa of bitmaps for 93 characters        */
    l_int32       size;        /* font size (in points at 300 ppi)         */
    char         *directory;   /* directory containing font bitmaps        */
    l_int32       baseline1;   /* baseline offset for ascii 33 - 57        */
    l_int32       baseline2;   /* baseline offset for ascii 58 - 91        */
    l_int32       baseline3;   /* baseline offset for ascii 93 - 126       */
    l_int32       lineheight;  /* max height of line of chars              */
    l_int32       kernwidth;   /* pixel dist between char bitmaps          */
    l_int32       spacewidth;  /* pixel dist between word bitmaps          */
    l_int32       vertlinesep; /* extra vertical space between text lines  */
    l_int32      *fonttab;     /* table mapping ascii --> font index       */
    l_int32      *baselinetab; /* table mapping ascii --> baseline offset  */
    l_int32      *widthtab;    /* table mapping ascii --> char width       */
};
typedef struct L_Bmf L_BMF;

#endif  /* LEPTONICA_BMF_H */
