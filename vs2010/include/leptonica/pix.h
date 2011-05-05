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

#ifndef  LEPTONICA_PIX_H
#define  LEPTONICA_PIX_H

/*
 *   pix.h
 *
 *   Contains the following structures:
 *       struct Pix
 *       struct PixColormap
 *       struct RGBA_Quad
 *       struct Pixa
 *       struct Pixaa
 *       struct Box
 *       struct Boxa
 *       struct Boxaa
 *       struct Pta
 *       struct Ptaa
 *       struct Pixacc
 *       struct PixTiling
 *       struct FPix
 *       struct FPixa
 *       struct DPix
 *       struct PixComp
 *       struct PixaComp
 *
 *   Contains definitions for:
 *       Colors for RGB
 *       Perceptual color weights
 *       Colormap conversion flags
 *       Rasterop bit flags
 *       Structure access flags (for insert, copy, clone, copy-clone)
 *       Sorting flags (by type and direction)
 *       Blending flags
 *       Graphics pixel setting flags
 *       Size filtering flags
 *       Color component selection flags
 *       Rotation and shear flags
 *       Affine transform order flags
 *       Grayscale filling flags
 *       Flags for setting to white or black
 *       Dithering flags
 *       Distance flags
 *       Statistical measures
 *       Set selection flags
 *       Text orientation flags
 *       Edge orientation flags
 *       Line orientation flags
 *       Scan direction flags
 *       Horizontal warp
 *       Pixel selection for resampling
 *       Thinning flags
 *       Runlength flags
 *       Edge filter flags
 *       Handling negative values in conversion to unsigned int
 *       Subpixel color component ordering in LCD display
 *       Relative to zero flags
 *       HSV histogram flags
 *       Region flags (inclusion, exclusion)
 *       Flags for adding text to a pix
 *       Flags for selecting display program
 */


/*-------------------------------------------------------------------------*
 *                              Basic Pix                                  *
 *-------------------------------------------------------------------------*/
struct Pix
{
    l_uint32             w;           /* width in pixels                   */
    l_uint32             h;           /* height in pixels                  */
    l_uint32             d;           /* depth in bits                     */
    l_uint32             wpl;         /* 32-bit words/line                 */
    l_uint32             refcount;    /* reference count (1 if no clones)  */
    l_int32              xres;        /* image res (ppi) in x direction    */
                                      /* (use 0 if unknown)                */
    l_int32              yres;        /* image res (ppi) in y direction    */
                                      /* (use 0 if unknown)                */
    l_int32              informat;    /* input file format, IFF_*          */
    char                *text;        /* text string associated with pix   */
    struct PixColormap  *colormap;    /* colormap (may be null)            */
    l_uint32            *data;        /* the image data                    */
};
typedef struct Pix PIX;


struct PixColormap
{
    void            *array;     /* colormap table (array of RGBA_QUAD)     */
    l_int32          depth;     /* of pix (1, 2, 4 or 8 bpp)               */
    l_int32          nalloc;    /* number of color entries allocated       */
    l_int32          n;         /* number of color entries used            */
};
typedef struct PixColormap  PIXCMAP;


    /* Colormap table entry (after the BMP version).
     * Note that the BMP format stores the colormap table exactly
     * as it appears here, with color samples being stored sequentially,
     * in the order (b,g,r,a). */
struct RGBA_Quad
{
    l_uint8     blue;
    l_uint8     green;
    l_uint8     red;
    l_uint8     reserved;
};
typedef struct RGBA_Quad  RGBA_QUAD;



/*-------------------------------------------------------------------------*
 *                             Colors for 32 bpp                           *
 *-------------------------------------------------------------------------*/
/*  Notes:
 *      (1) These are the byte indices for colors in 32 bpp images.
 *          They are used through the GET/SET_DATA_BYTE accessors.
 *          The 4th byte, typically known as the "alpha channel" and used
 *          for blending, is not explicitly used in leptonica.
 *      (2) If you redefine these values, functions that have the shifts
 *          hardcoded (instead of using the constants below) will break.
 *          These functions are labelled with "***" next to their names
 *          at the top of the files in which they are defined.
 *          Advice: Do not change these values!
 *      (3) The shifts to extract the red, green and blue components
 *          from a 32 bit pixel are defined in terms of these colors.
 */
enum {
    COLOR_RED = 0,
    COLOR_GREEN = 1,
    COLOR_BLUE = 2,
    L_ALPHA_CHANNEL = 3
};

static const l_int32  L_RED_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_RED);           /* 24 */
static const l_int32  L_GREEN_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_GREEN);         /* 16 */
static const l_int32  L_BLUE_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_BLUE);          /*  8 */
static const l_int32  L_ALPHA_SHIFT =
       8 * (sizeof(l_uint32) - 1 - L_ALPHA_CHANNEL);     /*  0 */


/*-------------------------------------------------------------------------*
 *                       Perceptual color weights                          *
 *-------------------------------------------------------------------------*/
/*  Notes:
 *      (1) These numbers are ad-hoc, but they do add up to 1.
 *          Unlike, for example, the weighting factor for conversion
 *          of RGB to luminance, or more specifically to Y in the
 *          YUV colorspace.  Those numbers come from the
 *          International Telecommunications Union, via ITU-R.
 */
static const l_float32  L_RED_WEIGHT =   0.3;
static const l_float32  L_GREEN_WEIGHT = 0.5;
static const l_float32  L_BLUE_WEIGHT =  0.2;


/*-------------------------------------------------------------------------*
 *                        Flags for colormap conversion                    *
 *-------------------------------------------------------------------------*/
enum {
    REMOVE_CMAP_TO_BINARY = 0,
    REMOVE_CMAP_TO_GRAYSCALE = 1,
    REMOVE_CMAP_TO_FULL_COLOR = 2,
    REMOVE_CMAP_BASED_ON_SRC = 3
};


/*-------------------------------------------------------------------------*
 *
 * The following operation bit flags have been modified from
 * Sun's pixrect.h.
 *
 * The 'op' in 'rasterop' is represented by an integer
 * composed with Boolean functions using the set of five integers
 * given below.  The integers, and the op codes resulting from
 * boolean expressions on them, need only be in the range from 0 to 15.
 * The function is applied on a per-pixel basis.
 *
 * Examples: the op code representing ORing the src and dest
 * is computed using the bit OR, as PIX_SRC | PIX_DST;  the op
 * code representing XORing src and dest is found from
 * PIX_SRC ^ PIX_DST;  the op code representing ANDing src and dest
 * is found from PIX_SRC & PIX_DST.  Note that
 * PIX_NOT(PIX_CLR) = PIX_SET, and v.v., as they must be.
 *
 * We would like to use the following set of definitions:
 *
 *      #define   PIX_SRC      0xc
 *      #define   PIX_DST      0xa
 *      #define   PIX_NOT(op)  ((op) ^ 0xf)
 *      #define   PIX_CLR      0x0
 *      #define   PIX_SET      0xf
 *
 * Now, these definitions differ from Sun's, in that Sun
 * left-shifted each value by 1 pixel, and used the least
 * significant bit as a flag for the "pseudo-operation" of
 * clipping.  We don't need this bit, because it is both
 * efficient and safe ALWAYS to clip the rectangles to the src
 * and dest images, which is what we do.  See the notes in rop.h
 * on the general choice of these bit flags.
 *
 * However, if you include Sun's xview package, you will get their
 * definitions, and because I like using these flags, we will
 * adopt the original Sun definitions to avoid redefinition conflicts.
 *
 * Then we have, for reference, the following 16 unique op flags:
 *
 *      PIX_CLR                           00000             0x0
 *      PIX_SET                           11110             0x1e
 *      PIX_SRC                           11000             0x18
 *      PIX_DST                           10100             0x14
 *      PIX_NOT(PIX_SRC)                  00110             0x06
 *      PIX_NOT(PIX_DST)                  01010             0x0a
 *      PIX_SRC | PIX_DST                 11100             0x1c
 *      PIX_SRC & PIX_DST                 10000             0x10
 *      PIX_SRC ^ PIX_DST                 01100             0x0c
 *      PIX_NOT(PIX_SRC) | PIX_DST        10110             0x16
 *      PIX_NOT(PIX_SRC) & PIX_DST        00100             0x04
 *      PIX_SRC | PIX_NOT(PIX_DST)        11010             0x1a
 *      PIX_SRC & PIX_NOT(PIX_DST)        01000             0x08
 *      PIX_NOT(PIX_SRC | PIX_DST)        00010             0x02
 *      PIX_NOT(PIX_SRC & PIX_DST)        01110             0x0e
 *      PIX_NOT(PIX_SRC ^ PIX_DST)        10010             0x12
 *
 *-------------------------------------------------------------------------*/
#define   PIX_SRC      (0xc << 1)
#define   PIX_DST      (0xa << 1)
#define   PIX_NOT(op)  ((op) ^ 0x1e)
#define   PIX_CLR      (0x0 << 1)
#define   PIX_SET      (0xf << 1)

#define   PIX_PAINT    (PIX_SRC | PIX_DST)
#define   PIX_MASK     (PIX_SRC & PIX_DST)
#define   PIX_SUBTRACT (PIX_DST & PIX_NOT(PIX_SRC))
#define   PIX_XOR      (PIX_SRC ^ PIX_DST)


/*-------------------------------------------------------------------------*
 *
 *   Important Notes:
 *
 *       (1) The image data is stored in a single contiguous
 *           array of l_uint32, into which the pixels are packed.
 *           By "packed" we mean that there are no unused bits
 *           between pixels, except for end-of-line padding to
 *           satisfy item (2) below.
 *
 *       (2) Every image raster line begins on a 32-bit word
 *           boundary within this array.
 *
 *       (3) Pix image data is stored in 32-bit units, with the
 *           pixels ordered from left to right in the image being
 *           stored in order from the MSB to LSB within the word,
 *           for both big-endian and little-endian machines.
 *           This is the natural ordering for big-endian machines,
 *           as successive bytes are stored and fetched progressively
 *           to the right.  However, for little-endians, when storing
 *           we re-order the bytes from this byte stream order, and
 *           reshuffle again for byte access on 32-bit entities.
 *           So if the bytes come in sequence from left to right, we
 *           store them on little-endians in byte order: 
 *                3 2 1 0 7 6 5 4 ...
 *           This MSB to LSB ordering allows left and right shift
 *           operations on 32 bit words to move the pixels properly.
 *
 *       (4) For 24-bit color images, use 32 bpp data, leaving
 *           the fourth byte unused.  Within each 4 byte pixel, the
 *           colors are ordered from MSB to LSB, as follows:
 *
 *                |  MSB  |  2nd MSB  |  3rd MSB  |  LSB  |
 *                   red      green       blue      unused
 *                    0         1           2         3   (big-endian)
 *                    3         2           1         0   (little-endian)
 *
 *           Because we use MSB to LSB ordering within the 32-bit word,
 *           the individual 8-bit samples can be accessed with 
 *           GET_DATA_BYTE and SET_DATA_BYTE macros, using the
 *           (implicitly big-ending) ordering
 *                 red:    byte 0  (MSB)
 *                 green:  byte 1  (2nd MSB)
 *                 blue:   byte 2  (3rd MSB)
 *
 *           This specific color assignment is made in this file,
 *           through the definitions of COLOR_RED, etc.  Then the R, G
 *           and B sample values can be retrieved using
 *                 redval = GET_DATA_BYTE(&pixel, COLOR_RED);
 *                 greenval = GET_DATA_BYTE(&pixel, COLOR_GREEN);
 *                 blueval = GET_DATA_BYTE(&pixel, COLOR_BLUE);
 *           and they can be set with
 *                 SET_DATA_BYTE(&pixel, COLOR_RED, redval);
 *                 SET_DATA_BYTE(&pixel, COLOR_GREEN, greenval);
 *                 SET_DATA_BYTE(&pixel, COLOR_BLUE, blueval);
 *
 *           For extra speed we extract the R, G and B colors directly
 *           by shifting and masking, explicitly using the values in
 *           L_RED_SHIFT, L_GREEN_SHIFT and L_BLUE_SHIFT:
 *                 (pixel32 >> L_RED_SHIFT) & 0xff;         (red)
 *                 (pixel32 >> L_GREEN_SHIFT) & 0xff;       (green)
 *                 (pixel32 >> L_BLUE_SHIFT) & 0xff;        (blue)
 *           All these operations work properly on both big- and little-endians.
 *
 *           For a few situations, these color shift values are hard-coded.
 *           Changing the RGB color component ordering through the assignments
 *           in this file will cause functions marked with "***" to fail.
 *
 *       (5) A reference count is held within each pix, giving the
 *           number of ptrs to the pix.  When a pixClone() call
 *           is made, the ref count is increased by 1, and
 *           when a pixDestroy() call is made, the reference count
 *           of the pix is decremented.  The pix is only destroyed
 *           when the reference count goes to zero.
 *
 *       (6) The version numbers (below) are used in the serialization
 *           of these data structures.  They are placed in the files,
 *           and rarely (if ever) change.  Provision is currently made for
 *           backward compatibility in reading from boxaa version 2.
 *
 *       (7) The serialization dependencies are as follows:
 *               pixaa  :  pixa  :  boxa
 *               boxaa  :  boxa
 *           So, for example, pixaa and boxaa can be changed without
 *           forcing a change in pixa or boxa.  However, if pixa is
 *           changed, it forces a change in pixaa, and if boxa is
 *           changed, if forces a change in the other three.
 *           We define four version numbers:
 *               PIXAA_VERSION_NUMBER
 *               PIXA_VERSION_NUMBER
 *               BOXAA_VERSION_NUMBER
 *               BOXA_VERSION_NUMBER
 *
 *-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*
 *                              Array of pix                               *
 *-------------------------------------------------------------------------*/

    /*  Serialization for primary data structures */
#define  PIXAA_VERSION_NUMBER      2
#define  PIXA_VERSION_NUMBER       2
#define  BOXA_VERSION_NUMBER       2
#define  BOXAA_VERSION_NUMBER      3


struct Pixa
{
    l_int32             n;            /* number of Pix in ptr array        */
    l_int32             nalloc;       /* number of Pix ptrs allocated      */
    l_uint32            refcount;     /* reference count (1 if no clones)  */
    struct Pix        **pix;          /* the array of ptrs to pix          */
    struct Boxa        *boxa;         /* array of boxes                    */
};
typedef struct Pixa PIXA;


struct Pixaa
{
    l_int32             n;            /* number of Pixa in ptr array       */
    l_int32             nalloc;       /* number of Pixa ptrs allocated     */
    struct Pixa       **pixa;         /* array of ptrs to pixa             */
    struct Boxa        *boxa;         /* array of boxes                    */
};
typedef struct Pixaa PIXAA;


/*-------------------------------------------------------------------------*
 *                    Basic rectangle and rectangle arrays                 *
 *-------------------------------------------------------------------------*/
struct Box
{
    l_int32            x;
    l_int32            y;
    l_int32            w;
    l_int32            h;
    l_uint32           refcount;      /* reference count (1 if no clones)  */

};
typedef struct Box    BOX;

struct Boxa
{
    l_int32            n;             /* number of box in ptr array        */
    l_int32            nalloc;        /* number of box ptrs allocated      */
    l_uint32           refcount;      /* reference count (1 if no clones)  */
    struct Box       **box;           /* box ptr array                     */
};
typedef struct Boxa  BOXA;

struct Boxaa
{
    l_int32            n;             /* number of boxa in ptr array       */
    l_int32            nalloc;        /* number of boxa ptrs allocated     */
    struct Boxa      **boxa;          /* boxa ptr array                    */
};
typedef struct Boxaa  BOXAA;


/*-------------------------------------------------------------------------*
 *                               Array of points                           *
 *-------------------------------------------------------------------------*/
#define  PTA_VERSION_NUMBER      1

struct Pta
{
    l_int32            n;             /* actual number of pts              */
    l_int32            nalloc;        /* size of allocated arrays          */
    l_int32            refcount;      /* reference count (1 if no clones)  */
    l_float32         *x, *y;         /* arrays of floats                  */
};
typedef struct Pta PTA;


/*-------------------------------------------------------------------------*
 *                              Array of Pta                               *
 *-------------------------------------------------------------------------*/
struct Ptaa
{
    l_int32              n;           /* number of pta in ptr array        */
    l_int32              nalloc;      /* number of pta ptrs allocated      */
    struct Pta         **pta;         /* pta ptr array                     */
};
typedef struct Ptaa PTAA;


/*-------------------------------------------------------------------------*
 *                       Pix accumulator container                         *
 *-------------------------------------------------------------------------*/
struct Pixacc
{
    l_int32             w;            /* array width                       */
    l_int32             h;            /* array height                      */
    l_int32             offset;       /* used to allow negative            */
                                      /* intermediate results              */
    struct Pix         *pix;          /* the 32 bit accumulator pix        */
};
typedef struct Pixacc PIXACC;


/*-------------------------------------------------------------------------*
 *                              Pix tiling                                 *
 *-------------------------------------------------------------------------*/
struct PixTiling
{
    struct Pix          *pix;         /* input pix (a clone)               */
    l_int32              nx;          /* number of tiles horizontally      */
    l_int32              ny;          /* number of tiles vertically        */
    l_int32              w;           /* tile width                        */
    l_int32              h;           /* tile height                       */
    l_int32              xoverlap;    /* overlap on left and right         */
    l_int32              yoverlap;    /* overlap on top and bottom         */
    l_int32              strip;       /* strip for paint; default is TRUE  */
};
typedef struct PixTiling PIXTILING;


/*-------------------------------------------------------------------------*
 *                       FPix: pix with float array                        *
 *-------------------------------------------------------------------------*/
#define  FPIX_VERSION_NUMBER      1

struct FPix
{
    l_int32              w;           /* width in pixels                   */
    l_int32              h;           /* height in pixels                  */
    l_int32              wpl;         /* 32-bit words/line                 */
    l_int32              refcount;    /* reference count (1 if no clones)  */
    l_int32              xres;        /* image res (ppi) in x direction    */
                                      /* (use 0 if unknown)                */
    l_int32              yres;        /* image res (ppi) in y direction    */
                                      /* (use 0 if unknown)                */
    l_float32           *data;        /* the float image data              */
};
typedef struct FPix FPIX;


struct FPixa
{
    l_int32             n;            /* number of Pix in ptr array        */
    l_int32             nalloc;       /* number of Pix ptrs allocated      */
    l_uint32            refcount;     /* reference count (1 if no clones)  */
    struct FPix       **fpix;         /* the array of ptrs to fpix         */
};
typedef struct FPixa FPIXA;


/*-------------------------------------------------------------------------*
 *                       DPix: pix with double array                       *
 *-------------------------------------------------------------------------*/
#define  DPIX_VERSION_NUMBER      1

struct DPix
{
    l_int32              w;           /* width in pixels                   */
    l_int32              h;           /* height in pixels                  */
    l_int32              wpl;         /* 32-bit words/line                 */
    l_int32              refcount;    /* reference count (1 if no clones)  */
    l_int32              xres;        /* image res (ppi) in x direction    */
                                      /* (use 0 if unknown)                */
    l_int32              yres;        /* image res (ppi) in y direction    */
                                      /* (use 0 if unknown)                */
    l_float64           *data;        /* the double image data             */
};
typedef struct DPix DPIX;


/*-------------------------------------------------------------------------*
 *                        PixComp: compressed pix                          *
 *-------------------------------------------------------------------------*/
struct PixComp
{
    l_int32              w;           /* width in pixels                   */
    l_int32              h;           /* height in pixels                  */
    l_int32              d;           /* depth in bits                     */
    l_int32              xres;        /* image res (ppi) in x direction    */
                                      /*   (use 0 if unknown)              */
    l_int32              yres;        /* image res (ppi) in y direction    */
                                      /*   (use 0 if unknown)              */
    l_int32              comptype;    /* compressed format (IFF_TIFF_G4,   */
                                      /*   IFF_PNG, IFF_JFIF_JPEG)         */
    char                *text;        /* text string associated with pix   */
    l_int32              cmapflag;    /* flag (1 for cmap, 0 otherwise)    */
    l_uint8             *data;        /* the compressed image data         */
    size_t               size;        /* size of the data array            */
};
typedef struct PixComp PIXC;


/*-------------------------------------------------------------------------*
 *                     PixaComp: array of compressed pix                   *
 *-------------------------------------------------------------------------*/
#define  PIXACOMP_VERSION_NUMBER      1

struct PixaComp
{
    l_int32              n;           /* number of PixComp in ptr array    */
    l_int32              nalloc;      /* number of PixComp ptrs allocated  */
    struct PixComp     **pixc;        /* the array of ptrs to PixComp      */
    struct Boxa         *boxa;        /* array of boxes                    */
};
typedef struct PixaComp PIXAC;


/*-------------------------------------------------------------------------*
 *                         Access and storage flags                        *
 *-------------------------------------------------------------------------*/
/*
 *  For Pix, Box, Pta and Numa, there are 3 standard methods for handling
 *  the retrieval or insertion of a struct:
 *     (1) direct insertion (Don't do this if there is another handle
 *                           somewhere to this same struct!)
 *     (2) copy (Always safe, sets up a refcount of 1 on the new object.
 *               Can be undesirable if very large, such as an image or
 *               an array of images.)
 *     (3) clone (Makes another handle to the same struct, and bumps the
 *                refcount up by 1.  Safe to do unless you're changing
 *                data through one of the handles but don't want those
 *                changes to be seen by the other handle.)
 *
 *  For Pixa and Boxa, which are structs that hold an array of clonable
 *  structs, there is an additional method:
 *     (4) copy-clone (Makes a new higher-level struct with a refcount
 *                     of 1, but clones all the structs in the array.)
 *
 *  Unlike the other structs, when retrieving a string from an Sarray,
 *  you are allowed to get a handle without a copy or clone (i.e., that
 *  you don't own!).  You must not free or insert such a string!
 *  Specifically, for an Sarray, the copyflag for retrieval is either:
 *         TRUE (or 1 or L_COPY)
 *  or
 *         FALSE (or 0 or L_NOCOPY)
 *  For insertion, the copyflag is either:
 *         TRUE (or 1 or L_COPY)
 *  or
 *         FALSE (or 0 or L_INSERT)
 *  Note that L_COPY is always 1, and L_INSERT and L_NOCOPY are always 0.
 */
enum {
    L_INSERT = 0,     /* stuff it in; no copy, clone or copy-clone    */
    L_COPY = 1,       /* make/use a copy of the object                */
    L_CLONE = 2,      /* make/use clone (ref count) of the object     */
    L_COPY_CLONE = 3  /* make a new object and fill with with clones  */
                      /* of each object in the array(s)               */
};
static const l_int32  L_NOCOPY = 0;  /* copyflag value in sarrayGetString() */


/*--------------------------------------------------------------------------*
 *                              Sort flags                                  *
 *--------------------------------------------------------------------------*/
enum {
    L_SORT_INCREASING = 1,        /* sort in increasing order               */
    L_SORT_DECREASING = 2         /* sort in decreasing order               */
};

enum {
    L_SORT_BY_X = 3,              /* sort box or c.c. by horiz location     */
    L_SORT_BY_Y = 4,              /* sort box or c.c. by vert location      */
    L_SORT_BY_WIDTH = 5,          /* sort box or c.c. by width              */
    L_SORT_BY_HEIGHT = 6,         /* sort box or c.c. by height             */
    L_SORT_BY_MIN_DIMENSION = 7,  /* sort box or c.c. by min dimension      */
    L_SORT_BY_MAX_DIMENSION = 8,  /* sort box or c.c. by max dimension      */
    L_SORT_BY_PERIMETER = 9,      /* sort box or c.c. by perimeter          */
    L_SORT_BY_AREA = 10,          /* sort box or c.c. by area               */
    L_SORT_BY_ASPECT_RATIO = 11   /* sort box or c.c. by width/height ratio */
};


/*-------------------------------------------------------------------------*
 *                             Blend flags                                 *
 *-------------------------------------------------------------------------*/
enum {
    L_BLEND_WITH_INVERSE = 1,     /* add some of src inverse to itself     */
    L_BLEND_TO_WHITE = 2,         /* shift src colors towards white        */
    L_BLEND_TO_BLACK = 3,         /* shift src colors towards black        */
    L_BLEND_GRAY = 4,             /* blend src directly with blender       */
    L_BLEND_GRAY_WITH_INVERSE = 5 /* add amount of src inverse to itself,  */
                                  /* based on blender pix value            */
};

enum {
    L_PAINT_LIGHT = 1,            /* colorize non-black pixels             */
    L_PAINT_DARK = 2              /* colorize non-white pixels             */
};


/*-------------------------------------------------------------------------*
 *                        Graphics pixel setting                           *
 *-------------------------------------------------------------------------*/
enum {
    L_SET_PIXELS = 1,             /* set all bits in each pixel to 1       */
    L_CLEAR_PIXELS = 2,           /* set all bits in each pixel to 0       */
    L_FLIP_PIXELS = 3             /* flip all bits in each pixel           */
};


/*-------------------------------------------------------------------------*
 *                           Size filter flags                             *
 *-------------------------------------------------------------------------*/
enum {
    L_SELECT_WIDTH = 1,           /* width must satisfy constraint         */
    L_SELECT_HEIGHT = 2,          /* height must satisfy constraint        */
    L_SELECT_IF_EITHER = 3,       /* either width or height can satisfy    */
    L_SELECT_IF_BOTH = 4          /* both width and height must satisfy    */
};

enum {
    L_SELECT_IF_LT = 1,           /* save if value is less than threshold  */
    L_SELECT_IF_GT = 2,           /* save if value is more than threshold  */
    L_SELECT_IF_LTE = 3,          /* save if value is <= to the threshold  */
    L_SELECT_IF_GTE = 4           /* save if value is >= to the threshold  */
};


/*-------------------------------------------------------------------------*
 *                     Color component selection flags                     *
 *-------------------------------------------------------------------------*/
enum {
    L_SELECT_RED = 1,             /* use red component                     */
    L_SELECT_GREEN = 2,           /* use green component                   */
    L_SELECT_BLUE = 3,            /* use blue component                    */
    L_SELECT_MIN = 4,             /* use min color component               */
    L_SELECT_MAX = 5              /* use max color component               */
};


/*-------------------------------------------------------------------------*
 *                        Rotate and shear flags                           *
 *-------------------------------------------------------------------------*/
enum {
    L_ROTATE_AREA_MAP = 1,       /* use area map rotation, if possible     */
    L_ROTATE_SHEAR = 2,          /* use shear rotation                     */
    L_ROTATE_SAMPLING = 3        /* use sampling                           */
};

enum {
    L_BRING_IN_WHITE = 1,        /* bring in white pixels from the outside */
    L_BRING_IN_BLACK = 2         /* bring in black pixels from the outside */
};

enum {
    L_SHEAR_ABOUT_CORNER = 1,    /* shear image about UL corner            */
    L_SHEAR_ABOUT_CENTER = 2     /* shear image about center               */
};


/*-------------------------------------------------------------------------*
 *                     Affine transform order flags                        *
 *-------------------------------------------------------------------------*/
enum {
    L_TR_SC_RO = 1,              /* translate, scale, rotate               */
    L_SC_RO_TR = 2,              /* scale, rotate, translate               */
    L_RO_TR_SC = 3,              /* rotate, translate, scale               */
    L_TR_RO_SC = 4,              /* translate, rotate, scale               */
    L_RO_SC_TR = 5,              /* rotate, scale, translate               */
    L_SC_TR_RO = 6               /* scale, translate, rotate               */
};


/*-------------------------------------------------------------------------*
 *                       Grayscale filling flags                           *
 *-------------------------------------------------------------------------*/
enum {
    L_FILL_WHITE = 1,           /* fill white pixels (e.g, in fg map)      */
    L_FILL_BLACK = 2            /* fill black pixels (e.g., in bg map)     */
};


/*-------------------------------------------------------------------------*
 *                   Flags for setting to white or black                   *
 *-------------------------------------------------------------------------*/
enum {
    L_SET_WHITE = 1,           /* set pixels to white                      */
    L_SET_BLACK = 2            /* set pixels to black                      */
};


/*-------------------------------------------------------------------------*
 *                           Dither parameters                             *
 *         If within this grayscale distance from black or white,          *
 *         do not propagate excess or deficit to neighboring pixels.       *
 *-------------------------------------------------------------------------*/
enum {
    DEFAULT_CLIP_LOWER_1 = 10,   /* dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_UPPER_1 = 10,   /* dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_LOWER_2 = 5,    /* dist to black with no prop; 2 bpp      */
    DEFAULT_CLIP_UPPER_2 = 5     /* dist to black with no prop; 2 bpp      */
};


/*-------------------------------------------------------------------------*
 *                             Distance flags                              *
 *-------------------------------------------------------------------------*/
enum {
    L_MANHATTAN_DISTANCE = 1,    /* L1 distance (e.g., in color space)     */
    L_EUCLIDEAN_DISTANCE = 2     /* L2 distance                            */
};


/*-------------------------------------------------------------------------*
 *                         Statistical measures                            *
 *-------------------------------------------------------------------------*/
enum {
    L_MEAN_ABSVAL = 1,           /* average of abs values                  */
    L_MEDIAN_VAL = 2,            /* median value of set                    */
    L_MODE_VAL = 3,              /* mode value of set                      */
    L_MODE_COUNT = 4,            /* mode count of set                      */
    L_ROOT_MEAN_SQUARE = 5,      /* rms of values                          */
    L_STANDARD_DEVIATION = 6,    /* standard deviation from mean           */
    L_VARIANCE = 7               /* variance of values                     */
};


/*-------------------------------------------------------------------------*
 *                          Set selection flags                            *
 *-------------------------------------------------------------------------*/
enum {
    L_CHOOSE_CONSECUTIVE = 1,    /* select 'n' consecutive                 */
    L_CHOOSE_SKIP_BY = 2         /* select at intervals of 'n'             */
};


/*-------------------------------------------------------------------------*
 *                         Text orientation flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_TEXT_ORIENT_UNKNOWN = 0,   /* low confidence on text orientation     */
    L_TEXT_ORIENT_UP = 1,        /* portrait, text rightside-up            */
    L_TEXT_ORIENT_LEFT = 2,      /* landscape, text up to left             */
    L_TEXT_ORIENT_DOWN = 3,      /* portrait, text upside-down             */
    L_TEXT_ORIENT_RIGHT = 4      /* landscape, text up to right            */
};


/*-------------------------------------------------------------------------*
 *                         Edge orientation flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_HORIZONTAL_EDGES = 0,     /* filters for horizontal edges            */
    L_VERTICAL_EDGES = 1,       /* filters for vertical edges              */
    L_ALL_EDGES = 2             /* filters for all edges                   */
};


/*-------------------------------------------------------------------------*
 *                         Line orientation flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_HORIZONTAL_LINE = 0,     /* horizontal line                          */
    L_POS_SLOPE_LINE = 1,      /* 45 degree line with positive slope       */
    L_VERTICAL_LINE = 2,       /* vertical line                            */
    L_NEG_SLOPE_LINE = 3,      /* 45 degree line with negative slope       */
    L_OBLIQUE_LINE = 4         /* neither horizontal nor vertical */
};


/*-------------------------------------------------------------------------*
 *                           Scan direction flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_FROM_LEFT = 0,           /* scan from left                           */
    L_FROM_RIGHT = 1,          /* scan from right                          */
    L_FROM_TOP = 2,            /* scan from top                            */
    L_FROM_BOTTOM = 3          /* scan from bottom                         */
};


/*-------------------------------------------------------------------------*
 *                            Horizontal warp                              *
 *-------------------------------------------------------------------------*/
enum {
    L_WARP_TO_LEFT = 1,      /* increasing stretch or contraction to left  */
    L_WARP_TO_RIGHT = 2      /* increasing stretch or contraction to right */
};

enum {
    L_LINEAR_WARP = 1,       /* stretch or contraction grows linearly      */
    L_QUADRATIC_WARP = 2     /* stretch or contraction grows quadratically */
};


/*-------------------------------------------------------------------------*
 *                      Pixel selection for resampling                     *
 *-------------------------------------------------------------------------*/
enum {
    L_INTERPOLATED = 1,      /* linear interpolation from src pixels       */
    L_SAMPLED = 2            /* nearest src pixel sampling only            */
};


/*-------------------------------------------------------------------------*
 *                             Thinning flags                              *
 *-------------------------------------------------------------------------*/
enum {
    L_THIN_FG = 1,               /* thin foreground of 1 bpp image         */
    L_THIN_BG = 2                /* thin background of 1 bpp image         */
};


/*-------------------------------------------------------------------------*
 *                            Runlength flags                              *
 *-------------------------------------------------------------------------*/
enum {
    L_HORIZONTAL_RUNS = 0,     /* determine runlengths of horizontal runs  */
    L_VERTICAL_RUNS = 1        /* determine runlengths of vertical runs    */
};


/*-------------------------------------------------------------------------*
 *                          Edge filter flags                              *
 *-------------------------------------------------------------------------*/
enum {
    L_SOBEL_EDGE = 1,          /* Sobel edge filter                        */
    L_TWO_SIDED_EDGE = 2       /* Two-sided edge filter                    */
};


/*-------------------------------------------------------------------------*
 *          Handling negative values in conversion to unsigned int         *
 *-------------------------------------------------------------------------*/
enum {
    L_CLIP_TO_ZERO = 1,        /* Clip negative values to 0                */
    L_TAKE_ABSVAL = 2          /* Convert to positive using L_ABS()        */
};


/*-------------------------------------------------------------------------*
 *             Subpixel color component ordering in LCD display            *
 *-------------------------------------------------------------------------*/
enum {
    L_SUBPIXEL_ORDER_RGB = 1,   /* sensor order left-to-right RGB          */
    L_SUBPIXEL_ORDER_BGR = 2,   /* sensor order left-to-right BGR          */
    L_SUBPIXEL_ORDER_VRGB = 3,  /* sensor order top-to-bottom RGB          */
    L_SUBPIXEL_ORDER_VBGR = 4   /* sensor order top-to-bottom BGR          */
};


/*-------------------------------------------------------------------------*
 *                         Relative to zero flags                          *
 *-------------------------------------------------------------------------*/
enum {
    L_LESS_THAN_ZERO = 1,      /* Choose values less than zero             */
    L_EQUAL_TO_ZERO = 2,       /* Choose values equal to zero              */
    L_GREATER_THAN_ZERO = 3    /* Choose values greater than zero          */
};


/*-------------------------------------------------------------------------*
 *                          HSV histogram flags                            *
 *-------------------------------------------------------------------------*/
enum {
    L_HS_HISTO = 1,            /* Use hue-saturation histogram             */
    L_HV_HISTO = 2,            /* Use hue-value histogram                  */
    L_SV_HISTO = 3             /* Use saturation-value histogram           */
};


/*-------------------------------------------------------------------------*
 *                    Region flags (inclusion, exclusion)                  *
 *-------------------------------------------------------------------------*/
enum {
    L_INCLUDE_REGION = 1,      /* Use hue-saturation histogram             */
    L_EXCLUDE_REGION = 2       /* Use hue-value histogram                  */
};


/*-------------------------------------------------------------------------*
 *                    Flags for adding text to a pix                       *
 *-------------------------------------------------------------------------*/
enum {
    L_ADD_ABOVE = 1,           /* Add text above the image                 */
    L_ADD_AT_TOP = 2,          /* Add text over the top of the image       */
    L_ADD_AT_BOTTOM = 3,       /* Add text over the bottom of the image    */
    L_ADD_BELOW = 4            /* Add text below the image                 */
};


/*-------------------------------------------------------------------------*
 *                   Flags for selecting display program                   *
 *-------------------------------------------------------------------------*/
enum {
    L_DISPLAY_WITH_XV = 1,      /* Use xv with pixDisplay()                */
    L_DISPLAY_WITH_XLI = 2,     /* Use xli with pixDisplay()               */
    L_DISPLAY_WITH_XZGV = 3,    /* Use xzgv with pixDisplay()              */
    L_DISPLAY_WITH_IV = 4       /* Use irfvanview with pixDisplay()        */
};

#endif  /* LEPTONICA_PIX_H */
