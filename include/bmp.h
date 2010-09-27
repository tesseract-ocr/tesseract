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

#ifndef  LEPTONICA_BMP_H
#define  LEPTONICA_BMP_H

/*
 * This file is here to describe the fields in the header of
 * the BMP file.  These fields are not used directly in Leptonica.
 * The only thing we use are the sizes of these two headers.
 * Furthermore, because of potential namespace conflicts with
 * the typedefs and defined sizes, we have changed the names
 * to protect anyone who may also need to use the original definitions.
 * Thanks to J. D. Bryan for pointing out the potential problems when
 * developing on Win32 compatible systems.
 */

/*-------------------------------------------------------------*
 *                       BMP file header                       *
 *-------------------------------------------------------------*/
struct BMP_FileHeader
{
    l_int16        bfType;                /* file type; must be "BM" */
    l_int16        bfSize;                /* length of the file;
                                   sizeof(BMP_FileHeader) +
                                   sizeof(BMP_InfoHeader) +
                                   size of color table +
                                   size of DIB bits */
    l_int16        bfFill1;        /* remainder of the bfSize field */
    l_int16        bfReserved1;        /* don't care (set to 0)*/
    l_int16        bfReserved2;        /* don't care (set to 0)*/
    l_int16        bfOffBits;        /* offset from beginning of file */
    l_int16        bfFill2;        /* remainder of the bfOffBits field */
};
typedef struct BMP_FileHeader  BMP_FH;

#define BMP_FHBYTES  sizeof(BMP_FH)


/*-------------------------------------------------------------*
 *                       BMP info header                       *
 *-------------------------------------------------------------*/
struct BMP_InfoHeader
{
    l_int32        biSize;                  /* size of the BMP_InfoHeader struct */
    l_int32        biWidth;         /* bitmap width in pixels */
    l_int32        biHeight;         /* bitmap height in pixels */
    l_int16        biPlanes;         /* number of bitmap planes */
    l_int16        biBitCount;         /* number of bits per pixel */
    l_int32        biCompression;         /* compression format (0 == uncompressed) */
    l_int32        biSizeImage;         /* size of image in bytes */
    l_int32        biXPelsPerMeter; /* pixels per meter in x direction */
    l_int32        biYPelsPerMeter; /* pixels per meter in y direction */
    l_int32        biClrUsed;          /* number of colors used */
    l_int32        biClrImportant;         /* number of important colors used */
};
typedef struct BMP_InfoHeader  BMP_IH;

#define BMP_IHBYTES  sizeof(BMP_IH)

 
#endif  /* LEPTONICA_BMP_H */
