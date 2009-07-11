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

#ifndef  LEPTONICA_WATERSHED_H
#define  LEPTONICA_WATERSHED_H

/* 
 *  watershed.h
 *
 *     Simple data structure to hold watershed data.
 *     All data here is owned by the L_WShed and must be freed.
 */

struct L_WShed 
{
    struct Pix    *pixs;        /* clone of input 8 bpp pixs                */
    struct Pix    *pixm;        /* clone of input 1 bpp seed (marker) pixm  */
    l_int32        mindepth;    /* minimum depth allowed for a watershed    */
    struct Pix    *pixlab;      /* 16 bpp label pix                         */
    struct Pix    *pixt;        /* scratch pix for computing wshed regions  */
    void         **lines8;      /* line ptrs for pixs                       */
    void         **linem1;      /* line ptrs for pixm                       */
    void         **linelab32;   /* line ptrs for pixlab                     */
    void         **linet1;      /* line ptrs for pixt                       */
    struct Pixa   *pixad;       /* result: 1 bpp pixa of watersheds         */
    struct Pta    *ptas;        /* pta of initial seed pixels               */
    struct Numa   *nasi;        /* numa of seed indicators; 0 if completed  */
    struct Numa   *nash;        /* numa of initial seed heights             */
    struct Numa   *namh;        /* numa of initial minima heights           */
    struct Numa   *nalevels;    /* result: numa of watershed levels         */
    l_int32        nseeds;      /* number of seeds (markers)                */
    l_int32        nother;      /* number of minima different from seeds    */
    l_int32       *lut;         /* lut for pixel indices                    */
    struct Numa  **links;       /* back-links into lut, for updates         */
    l_int32        arraysize;   /* size of links array                      */
    l_int32        debug;       /* set to 1 for debug output                */
};
typedef struct L_WShed L_WSHED;

#endif  /* LEPTONICA_WATERSHED_H */
