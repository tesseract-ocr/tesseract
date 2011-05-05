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

#ifndef  LEPTONICA_IMAGEIO_H
#define  LEPTONICA_IMAGEIO_H

/* ------------------ Image file format types -------------- */
/*  
 *  The IFF_DEFAULT flag is used to write the file out in the
 *  same (input) file format that the pix was read from.  If the pix
 *  was not read from file, the input format field will be
 *  IFF_UNKNOWN and the output file format will be chosen to
 *  be compressed and lossless; namely, IFF_TIFF_G4 for d = 1
 *  and IFF_PNG for everything else.   IFF_JP2 is for jpeg2000, which
 *  is not supported in leptonica.
 *
 *  In the future, new format types that have defined extensions
 *  will be added before IFF_DEFAULT, and will be kept in sync with
 *  the file format extensions in writefile.c.  The positions of
 *  file formats before IFF_DEFAULT will remain invariant.
 */
enum {
    IFF_UNKNOWN        = 0,
    IFF_BMP            = 1,
    IFF_JFIF_JPEG      = 2,
    IFF_PNG            = 3,
    IFF_TIFF           = 4,
    IFF_TIFF_PACKBITS  = 5,
    IFF_TIFF_RLE       = 6,
    IFF_TIFF_G3        = 7,
    IFF_TIFF_G4        = 8,
    IFF_TIFF_LZW       = 9,
    IFF_TIFF_ZIP       = 10,
    IFF_PNM            = 11,
    IFF_PS             = 12,
    IFF_GIF            = 13,
    IFF_JP2            = 14,
    IFF_WEBP           = 15,
    IFF_LPDF           = 16,
    IFF_DEFAULT        = 17,
    IFF_SPIX           = 18
};


/* ---------------------- Format header ids --------------------- */
enum {
    BMP_ID             = 0x4d42,
    TIFF_BIGEND_ID     = 0x4d4d,     /* MM - for 'motorola' */
    TIFF_LITTLEEND_ID  = 0x4949      /* II - for 'intel'    */
};


/* ------------------ Gray hinting in jpeg reader --------------- */
enum {
    L_HINT_GRAY = 1,  /* only want grayscale information */
};


/* ------------------ Pdf formated encoding types --------------- */
enum {
    L_JPEG_ENCODE   = 1,    /* use dct encoding: 8 and 32 bpp, no cmap     */
    L_G4_ENCODE     = 2,    /* use ccitt g4 fax encoding: 1 bpp            */
    L_FLATE_ENCODE  = 3     /* use flate encoding: any depth, cmap ok      */
};


/* ------------------ Compressed image data --------------------- */
/*
 *  In use, either datacomp or data85 will be produced, depending
 *  on whether the data needs to be ascii85 encoded.  PostScript
 *  requires ascii85 encoding; pdf does not.
 *
 *  For the colormap (flate compression only), PostScript uses ascii85
 *  encoding and pdf uses a bracketed array of space-separated
 *  hex-encoded rgb triples.  Only tiff g4 (type == L_G4_ENCODE) uses
 *  the minisblack field.
 */
struct L_Compressed_Data
{
    l_int32            type;         /* encoding type: L_JPEG_ENCODE, etc  */
    l_uint8           *datacomp;     /* gzipped raster data                 */
    size_t             nbytescomp;   /* number of compressed bytes          */
    char              *data85;       /* ascii85-encoded gzipped raster data */
    size_t             nbytes85;     /* number of ascii85 encoded bytes     */
    char              *cmapdata85;   /* ascii85-encoded uncompressed cmap   */
    char              *cmapdatahex;  /* hex pdf array for the cmap          */
    l_int32            ncolors;      /* number of colors in cmap            */
    l_int32            w;            /* image width                         */
    l_int32            h;            /* image height                        */
    l_int32            bps;          /* bits/sample; typ. 1, 2, 4 or 8      */
    l_int32            spp;          /* samples/pixel; typ. 1 or 3          */
    l_int32            minisblack;   /* tiff g4 photometry                  */
    size_t             nbytes;       /* number of uncompressed raster bytes */
    l_int32            res;          /* resolution (ppi)                    */
};
typedef struct L_Compressed_Data  L_COMPRESSED_DATA;


/* ------------------------ Pdf multi-image flags ------------------------ */
enum {
    L_FIRST_IMAGE   = 1,    /* first image to be used                      */
    L_NEXT_IMAGE    = 2,    /* intermediate image; not first or last       */
    L_LAST_IMAGE    = 3     /* last image to be used                       */
};


/* ------------------ Intermediate pdf generation data -------------------- */
/*
 *  This accumulates data for generating a pdf of a single page consisting
 *  of an arbitrary number of images.
 *
 *  None of the strings have a trailing newline.
 */
struct L_Pdf_Data
{
    char              *title;        /* optional title for pdf              */
    l_int32            n;            /* number of images                    */
    l_int32            ncmap;        /* number of colormaps                 */
    struct L_Ptra     *cida;         /* array of compressed image data      */
    char              *id;           /* %PDF-1.2 id string                  */
    char              *obj1;         /* catalog string                      */
    char              *obj2;         /* metadata string                     */
    char              *obj3;         /* pages string                        */
    char              *obj4;         /* page string (variable data)         */
    char              *obj5;         /* content string (variable data)      */
    char              *poststream;   /* post-binary-stream string           */
    char              *trailer;      /* trailer string (variable data)      */
    struct Pta        *xy;           /* store (xpt, ypt) array              */
    struct Pta        *wh;           /* store (wpt, hpt) array              */
    struct Box        *mediabox;     /* bounding region for all images      */
    struct Sarray     *saprex;       /* pre-binary-stream xobject strings   */
    struct Sarray     *sacmap;       /* colormap pdf object strings         */
    struct Numa       *objsize;      /* sizes of each pdf string object     */
    struct Numa       *objloc;       /* location of each pdf string object  */
    l_int32            xrefloc;      /* location of xref                    */
};
typedef struct L_Pdf_Data  L_PDF_DATA;


#endif  /* LEPTONICA_IMAGEIO_H */

