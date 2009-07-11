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

#ifndef  LEPTONICA_JBCLASS_H
#define  LEPTONICA_JBCLASS_H

/*
 * jbclass.h
 *
 *       JbClasser
 *       JbData
 */


    /* The JbClasser struct holds all the data accumulated during the
     * classification process that can be used for a compressed
     * jbig2-type representation of a set of images.  This is created
     * in an initialization process and added to as the selected components
     * on each successive page are analyzed.   */
struct JbClasser
{
    struct Sarray   *safiles;      /* input page image file names            */
    l_int32          method;       /* JB_RANKHAUS, JB_CORRELATION            */
    l_int32          components;   /* JB_CONN_COMPS, JB_CHARACTERS or        */
                                   /* JB_WORDS                               */
    l_int32          maxwidth;     /* max component width allowed            */
    l_int32          maxheight;    /* max component height allowed           */
    l_int32          npages;       /* number of pages already processed      */
    l_int32          baseindex;    /* number of components already processed */
                                   /* on fully processed pages               */
    struct Numa     *nacomps;      /* number of components on each page      */
    l_int32          sizehaus;     /* size of square struct element for haus */
    l_float32        rankhaus;     /* rank val of haus match, each way       */
    l_float32        thresh;       /* thresh value for correlation score     */
    l_float32        weightfactor; /* corrects thresh value for heaver       */
                                   /* components; use 0 for no correction    */
    struct Numa     *naarea;       /* w * h of each template, without extra  */
                                   /* border pixels                          */
    l_int32          w;            /* max width of original src images       */
    l_int32          h;            /* max height of original src images      */
    l_int32          nclass;       /* current number of classes              */
    l_int32          keep_pixaa;   /* If zero, pixaa isn't filled            */
    struct Pixaa    *pixaa;        /* instances for each class; unbordered   */
    struct Pixa     *pixat;        /* templates for each class; bordered     */
                                   /* and not dilated                        */
    struct Pixa     *pixatd;       /* templates for each class; bordered     */
                                   /* and dilated                            */
    struct NumaHash *nahash;       /* Hash table to find templates by size   */
    struct Numa     *nafgt;        /* fg areas of undilated templates;       */
                                   /* only used for rank < 1.0               */
    struct Pta      *ptac;         /* centroids of all bordered cc           */
    struct Pta      *ptact;        /* centroids of all bordered template cc  */
    struct Numa     *naclass;      /* array of class ids for each component  */
    struct Numa     *napage;       /* array of page nums for each component  */
    struct Pta      *ptaul;        /* array of UL corners at which the       */
                                   /* template is to be placed for each      */
                                   /* component                              */
    struct Pta      *ptall;        /* similar to ptaul, but for LL corners   */
};
typedef struct JbClasser  JBCLASSER;


    /* The JbData struct holds all the data required for
     * the compressed jbig-type representation of a set of images.
     * The data can be written to file, read back, and used
     * to regenerate an approximate version of the original,
     * which differs in two ways from the original:
     *   (1) It uses a template image for each c.c. instead of the
     *       original instance, for each occurrence on each page.
     *   (2) It discards components with either a height or width larger
     *       than the maximuma, given here by the lattice dimensions
     *       used for storing the templates.   */
struct JbData
{
    struct Pix         *pix;        /* template composite for all classes    */
    l_int32             npages;     /* number of pages                       */
    l_int32             w;          /* max width of original page images     */
    l_int32             h;          /* max height of original page images    */
    l_int32             nclass;     /* number of classes                     */
    l_int32             latticew;   /* lattice width for template composite  */
    l_int32             latticeh;   /* lattice height for template composite */
    struct Numa        *naclass;    /* array of class ids for each component */
    struct Numa        *napage;     /* array of page nums for each component */
    struct Pta         *ptaul;      /* array of UL corners at which the      */
                                    /* template is to be placed for each     */
                                    /* component                             */
};
typedef struct JbData  JBDATA;


    /* Classifier methods */
enum {
   JB_RANKHAUS = 0,
   JB_CORRELATION = 1
};

    /* For jbGetComponents(): type of component to extract from images */
enum {
   JB_CONN_COMPS = 0,
   JB_CHARACTERS = 1,
   JB_WORDS = 2
};

    /* These parameters are used for naming the two files
     * in which the jbig2-like compressed data is stored.  */
#define   JB_TEMPLATE_EXT      ".templates.png"
#define   JB_DATA_EXT          ".data"


#endif  /* LEPTONICA_JBCLASS_H */
