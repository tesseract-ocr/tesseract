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

#ifndef  LEPTONICA_DEWARP_H
#define  LEPTONICA_DEWARP_H

/* 
 *  dewarp.h
 *
 *     Data structure to hold arrays and results for generating
 *     a vertical disparity array based on textlines.  The disparity
 *     array is two-dimensional, and it represents a vertical
 *     displacement, relative to the flat point in the textlines.
 *     After dewarping, all points on the altered curve will have
 *     a y-value equal to the flat point.
 *
 *     The sampled vertical disparity array is expanded to full resolution,
 *     using linear interpolation, from which it is trivially applied
 *     to the input image.
 */

#define  DEWARP_VERSION_NUMBER      1

struct L_Dewarp
{
    struct Pix     *pixs;        /* source pix, 1 bpp                       */
    struct Pix     *pixd;        /* dewarped pix; 1, 8 or 32 bpp            */
    struct FPix    *sampvdispar; /* sampled vertical disparity array        */
    struct FPix    *samphdispar; /* sampled horizontal disparity array      */
    struct FPix    *fullvdispar; /* full vertical disparity array           */
    struct FPix    *fullhdispar; /* full horiztontal disparity array        */
    struct Numa    *naflats;     /* sorted flat location of each line       */
    struct Numa    *nacurves;    /* sorted curvature of each line           */
    l_int32         pageno;      /* page number; important for reuse        */
    l_int32         sampling;    /* sampling factor of disparity array      */
    l_int32         minlines;    /* min number of long lines required       */
    l_int32         applyhoriz;  /* flag for estimating horiz. disparity    */
    l_int32         nx;          /* number of sampling pts in x direction   */
    l_int32         ny;          /* number of sampling pts in y direction   */
    l_int32         extraw;      /* extra width required for hor. disparity */
    l_int32         success;     /* sets to 1 if model build succeeds       */
};
typedef struct L_Dewarp L_DEWARP;

#endif  /* LEPTONICA_DEWARP_H */
