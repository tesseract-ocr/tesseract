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

#ifndef  LEPTONICA_CCBORD_H
#define  LEPTONICA_CCBORD_H

/*
 *  ccbord.h
 *
 *           CCBord:   represents a single connected component
 *           CCBorda:  an array of CCBord
 */

    /* Use in ccbaStepChainsToPixCoords() */
enum {
      CCB_LOCAL_COORDS = 1,
      CCB_GLOBAL_COORDS = 2
};

    /* Use in ccbaGenerateSPGlobalLocs() */
enum {
      CCB_SAVE_ALL_PTS = 1,
      CCB_SAVE_TURNING_PTS = 2
};


    /* CCBord contains:
     *
     *    (1) a minimally-clipped bitmap of the component (pix),
     *    (2) a boxa consisting of:
     *          for the primary component:
     *                (xul, yul) pixel location in global coords
     *                (w, h) of the bitmap
     *          for the hole components:
     *                (x, y) in relative coordinates in primary component
     *                (w, h) of the hole border (which is 2 pixels
     *                       larger in each direction than the hole itself)
     *    (3) a pta ('start') of the initial border pixel location for each
     *        closed curve, all in relative coordinates of the primary
     *        component.  This is given for the primary component,
     *        followed by the hole components, if any.
     *    (4) a refcount of the ccbord; used internally when a ccbord
     *        is accessed from a ccborda (array of ccbord)
     *    (5) a ptaa for the chain code for the border in relative
     *        coordinates, where the first pta is the exterior border
     *        and all other pta are for interior borders (holes)
     *    (6) a ptaa for the global pixel loc rendition of the border,
     *        where the first pta is the exterior border and all other
     *        pta are for interior borders (holes).
     *        This is derived from the local or step chain code.
     *    (7) a numaa for the chain code for the border as orientation
     *        directions between successive border pixels, where
     *        the first numa is the exterior border and all other
     *        numa are for interior borders (holes).  This is derived
     *        from the local chain code.  The 8 directions are 0 - 7.
     *    (8) a pta for a single chain for each c.c., comprised of outer
     *        and hole borders, plus cut paths between them, all in
     *        local coords.
     *    (9) a pta for a single chain for each c.c., comprised of outer
     *        and hole borders, plus cut paths between them, all in
     *        global coords.
     */
struct CCBord
{
    struct Pix          *pix;            /* component bitmap (min size)      */
    struct Boxa         *boxa;           /* regions of each closed curve     */
    struct Pta          *start;          /* initial border pixel locations   */
    l_int32              refcount;       /* number of handles; start at 1    */
    struct Ptaa         *local;          /* ptaa of chain pixels (local)     */
    struct Ptaa         *global;         /* ptaa of chain pixels (global)    */
    struct Numaa        *step;           /* numaa of chain code (step dir)   */
    struct Pta          *splocal;        /* pta of single chain (local)      */
    struct Pta          *spglobal;       /* pta of single chain (global)     */
};
typedef struct CCBord CCBORD;


struct CCBorda
{
    struct Pix          *pix;            /* input pix (may be null)          */
    l_int32              w;              /* width of pix                     */
    l_int32              h;              /* height of pix                    */
    l_int32              n;              /* number of ccbord in ptr array    */
    l_int32              nalloc;         /* number of ccbord ptrs allocated  */
    struct CCBord      **ccb;            /* ccb ptr array                    */
};
typedef struct CCBorda CCBORDA;


#endif  /* LEPTONICA_CCBORD_H */

