/**********************************************************************
 * File:        imgtiff.c  (Formerly tiff.c)
 * Description: Max format image reader/writer.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 14:00:21 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include          "mfcpch.h"     //precompiled headers
#ifdef __MSW32__
#include          <io.h>
#else
#include          <unistd.h>
#endif

/*
** Include automatically generated configuration file if running autoconf
*/
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#if defined(MOTOROLA_BYTE_ORDER) || defined(WORDS_BIGENDIAN)
#define __MOTO__  // Big-endian.
#endif
#endif
#ifdef USING_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(x) gettext(x)
#else
#define _(x) (x)
#endif

#include          "fileerr.h"
#include          "imgerrs.h"
#include          "img.h"
#include          "bitstrm.h"
#include          "tprintf.h"
#include          "serialis.h"
#include          "imgtiff.h"

#define INTEL       0x4949
#define MOTO        0x4d4d

/*************************************************************************
 * NOTE ON BIG-ENDIAN vs LITTLE-ENDIAN
 *
 * Intel machines store numbers with LSByte in the left position.
 * Motorola	(and PA_RISC) machines use the opposite byte ordering.
 *
 * This code is written so that:
 *   a) it will compile and run on EITHER machine type   AND
 *   b) the program (on either machine) will process tiff file written in either
 *      Motorola or Intel format.
 *
 * The code is compiled with a __NATIVE__ define which is either MOTO or INTEL.
 * MOTO and INTEL are defined (above) to be the value of the first two bytes of
 * a tiff file in either format. (This identifies the filetype).
 *
 * Subsequent reads and writes normally just reverse the byte order if the
 * machine type (__NATIVE__) is not equal to the filetype determined from the
 * first two bytes of the tiff file.
 *
 * A special case is the "value" field of the tag structure. This can contain
 * EITHER a 16bit or a 32bit value. According to the "type" field. The 4 cases
 * of machine type / file type combinations need to be treated differently in
 * the case of 16 bit values
 *************************************************************************/

#define ENTRIES       19         /*no of entries */
#define START       8            /*start of tag table */

typedef struct
{
  uinT16 tag;                    //entry tag
  uinT16 type;
  uinT32 length;
  inT32 value;
} TIFFENTRY;                     //tiff tag entry

typedef struct myrational
{
  inT32 top;
  inT32 bottom;
} MYRATIONAL;                    //type 5

//statics for the run length codes
#define EOL_CODE      0x800
#define EOL_MASK      0xfff
#define EOL_LENGTH      12       //12 bits
#define SHORT_CODE_SIZE   64     //no of short codes
#define LONG_CODE_SIZE    40     //no of long codes

const uinT16 short_white_codes[SHORT_CODE_SIZE] = {
  0xac, 0x38, 0xe, 0x1, 0xd, 0x3, 0x7, 0xf,
  0x19, 0x5, 0x1c, 0x2, 0x4, 0x30, 0xb, 0x2b,
  0x15, 0x35, 0x72, 0x18, 0x8, 0x74, 0x60, 0x10,
  0xa, 0x6a, 0x64, 0x12, 0xc, 0x40, 0xc0, 0x58,
  0xd8, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x14,
  0x94, 0x54, 0xd4, 0x34, 0xb4, 0x20, 0xa0, 0x50,
  0xd0, 0x4a, 0xca, 0x2a, 0xaa, 0x24, 0xa4, 0x1a,
  0x9a, 0x5a, 0xda, 0x52, 0xd2, 0x4c, 0xcc, 0x2c
};
const uinT8 short_white_lengths[SHORT_CODE_SIZE] = {
  8, 6, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 6, 6, 6, 6,
  6, 6, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8
};
const uinT16 short_black_codes[SHORT_CODE_SIZE] = {
  0x3b0, 0x2, 0x3, 0x1, 0x6, 0xc, 0x4, 0x18,
  0x28, 0x8, 0x10, 0x50, 0x70, 0x20, 0xe0, 0x30,
  0x3a0, 0x60, 0x40, 0x730, 0xb0, 0x1b0, 0x760, 0xa0,
  0x740, 0xc0, 0x530, 0xd30,
  0x330, 0xb30, 0x160, 0x960,
  0x560, 0xd60, 0x4b0, 0xcb0,
  0x2b0, 0xab0, 0x6b0, 0xeb0,
  0x360, 0xb60, 0x5b0, 0xdb0,
  0x2a0, 0xaa0, 0x6a0, 0xea0,
  0x260, 0xa60, 0x4a0, 0xca0,
  0x240, 0xec0, 0x1c0, 0xe40,
  0x140, 0x1a0, 0x9a0, 0xd40,
  0x340, 0x5a0, 0x660, 0xe60
};
const uinT8 short_black_lengths[SHORT_CODE_SIZE] = {
  10, 3, 2, 2, 3, 4, 4, 5,
  6, 6, 7, 7, 7, 8, 8, 9,
  10, 10, 10, 11, 11, 11, 11, 11,
  11, 11, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12
};
const uinT16 long_white_codes[LONG_CODE_SIZE] = {
  0x1b, 0x9, 0x3a, 0x76, 0x6c, 0xec, 0x26, 0xa6,
  0x16, 0xe6, 0x66, 0x166, 0x96, 0x196, 0x56, 0x156,
  0xd6, 0x1d6, 0x36, 0x136, 0xb6, 0x1b6, 0x32, 0x132,
  0xb2, 0x6, 0x1b2,
  0x80, 0x180, 0x580, 0x480, 0xc80,
  0x280, 0xa80, 0x680, 0xe80, 0x380, 0xb80, 0x780, 0xf80
};
const uinT8 long_white_lengths[LONG_CODE_SIZE] = {
  5, 5, 6, 7, 8, 8, 8, 8,
  8, 8, 9, 9, 9, 9, 9, 9,
  9, 9, 9, 9, 9, 9, 9, 9,
  9, 6, 9, 11, 11, 11, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12
};
const uinT16 long_black_codes[LONG_CODE_SIZE] = {
  0x3c0, 0x130, 0x930, 0xda0,
  0xcc0, 0x2c0, 0xac0, 0x6c0,
  0x16c0, 0xa40, 0x1a40, 0x640,
  0x1640, 0x9c0, 0x19c0, 0x5c0,
  0x15c0, 0xdc0, 0x1dc0, 0x940,
  0x1940, 0x540, 0x1540, 0xb40,
  0x1b40, 0x4c0, 0x14c0,
  0x80, 0x180, 0x580, 0x480, 0xc80,
  0x280, 0xa80, 0x680, 0xe80, 0x380, 0xb80, 0x780, 0xf80
};
const uinT8 long_black_lengths[LONG_CODE_SIZE] = {
  10, 12, 12, 12, 12, 12, 12, 13,
  13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 11, 11, 11, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12
};

/**********************************************************************
 * open_tif_image
 *
 * Read the header of a tif format image and prepare to read the rest.
 **********************************************************************/

inT8 open_tif_image(               //read header
                    int fd,        //file to read
                    inT32 *xsize,  //size of image
                    inT32 *ysize,
                    inT8 *bpp,     //bits per pixel
                    inT8 *photo,   //interpretation
                    inT32 *res     //resolution
                   ) {
  inT16 filetype;
  inT32 start;                   //start of tiff directory
  inT16 entries;                 //no of tiff entries
  inT32 imagestart;              //location of image in file
  inT32 resoffset;               //location of res
  TIFFENTRY tiffentry;           //tag table entry
  BOOL8 compressed;              //compression control
  MYRATIONAL resinfo;            //resolution
  BOOL8 strips = false;          //if in strips

  *xsize = -1;                   //illegal values
  *ysize = -1;
  *bpp = -1;
  *res = -1;
  resoffset = -1;
  if (read (fd, (char *) &filetype, sizeof filetype) != sizeof filetype
  || (filetype != INTEL && filetype != MOTO)) {
    BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Filetype");
    return -1;
  }
  lseek (fd, 4L, 0);
  if (read (fd, (char *) &start, sizeof start) != sizeof start) {
    READFAILED.error ("read_tif_image", TESSLOG, "Start of tag table");
    return -1;
  }

  if (filetype != __NATIVE__)
    start = reverse32 (start);
  if (start <= 0) {
    BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Start of tag table");
    return -1;
  }
  lseek (fd, start, 0);
  if (read (fd, (char *) &entries, sizeof (inT16)) != sizeof (inT16)) {
    BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Size of tag table");
    return -1;
  }
  if (filetype != __NATIVE__)
    entries = reverse16 (entries);
  //      printf("No of tiff directory entries=%d\n",entries);
  imagestart = 0;
  compressed = FALSE;
  int samples_per_pixel = 1;
  int bits_per_sample = 1;
  for (; entries-- > 0;) {
    if (read (fd, (char *) &tiffentry, sizeof tiffentry) !=
    sizeof tiffentry) {
      BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Tag table entry");
      return -1;
    }
    if (filetype != __NATIVE__) {
      tiffentry.type = reverse16 (tiffentry.type);
      tiffentry.tag = reverse16 (tiffentry.tag);
      tiffentry.length = reverse32 (tiffentry.length);
    }
    if (tiffentry.type != 3) {   //Full 32bit value
      if (filetype != __NATIVE__)
        tiffentry.value = reverse32 (tiffentry.value);
    }
    else {
      /* A 16bit value in 4 bytes - handle with care. SEE NOTE at start of file */
      if (__NATIVE__ == MOTO) {
        if (filetype == MOTO)    //MOTO file on MOTO Machine
          tiffentry.value = tiffentry.value >> 16;
        else                     //INTEL file on MOTO Machine
          tiffentry.value = reverse32 (tiffentry.value);
      }
      else {                     //INTEL Machine
        if (filetype == MOTO)    //MOTO file on INTEL Machine
          tiffentry.value = reverse16 ((uinT16) tiffentry.value);
        //INTEL file on INTEL Machine NO ACTION NEEDED
      }
                                 //Clear top 2 MSBytes
      tiffentry.value &= 0x0000ffff;
    }

    //              printf("Tag=%x, Type=%x, Length=%x, value=%x\n",
    //                      tiffentry.tag,tiffentry.type,tiffentry.length,tiffentry.value);
    switch (tiffentry.tag) {
      case 0x101:
        *ysize = tiffentry.value;
        break;
      case 0x100:
        *xsize = tiffentry.value;
        break;
      case 0x102:
        if (tiffentry.length == 1)
          bits_per_sample = (inT8) tiffentry.value;
        else
          bits_per_sample = 8;
        break;
      case 0x115:
        samples_per_pixel = (inT8) tiffentry.value;
        break;
      case 0x111:
        imagestart = tiffentry.value;
        strips = tiffentry.length > 1;
        break;
      case 0x103:
        if (tiffentry.value == 3) {
          compressed = TRUE;
        }
        else if (tiffentry.value != 1) {
          BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Compression");
          return -1;
        }
        break;
      case 0x11a:
      case 0x11b:
                                 //resolution
        resoffset = tiffentry.value;
        break;
      case 0x106:
        *photo = (inT8) tiffentry.value;
        break;
    }                            //endswitch
  }
  if (*xsize <= 0 || *ysize <= 0 || imagestart <= 0) {
    BADIMAGEFORMAT.error ("read_tif_image", TESSLOG, "Vital tag");
    return -1;
  }
  tprintf(_("Image has %d * %d bit%c per pixel, and size (%d,%d)\n"),
          bits_per_sample, samples_per_pixel, bits_per_sample == 1 ? ' ' : 's',
          *xsize, *ysize);
  *bpp = bits_per_sample * samples_per_pixel;
  if (resoffset >= 0) {
    lseek (fd, resoffset, 0);
    if (read (fd, (char *) &resinfo, sizeof (resinfo)) != sizeof (resinfo)) {
      READFAILED.error ("read_tif_image", TESSLOG, "Resolution");
      return -1;
    }
    if (filetype != __NATIVE__) {
      resinfo.top = reverse32 (resinfo.top);
      resinfo.bottom = reverse32 (resinfo.bottom);
    }
    *res = resinfo.top / resinfo.bottom;
    tprintf (_("Resolution=%d\n"), *res);
  }
  lseek (fd, (long) imagestart, 0);
  if (strips) {
    if (read (fd, (char *) &imagestart, sizeof (imagestart)) !=
    sizeof (imagestart)) {
      READFAILED.error ("read_tif_image", TESSLOG, "Strip offset");
      return -1;
    }
    if (filetype != __NATIVE__)
      imagestart = reverse32 (imagestart);
                                 //indirection
    lseek (fd, (long) imagestart, 0);
  }
  return compressed ? -2 : 0;
}


/**********************************************************************
 * read_tif_image
 *
 * Read a whole tif image into memory.
 **********************************************************************/

inT8 read_tif_image(int fd,         // file to read
                    uinT8 *pixels,  // pixels of image
                    inT32 xsize,    // size of image
                    inT32 ysize,
                    inT8 bpp,       // bits per pixel
                    inT32) {        // bytes per line
  inT32 xindex;                  // indices in image
  inT32 yindex;
  inT32 length;                  // short length
  inT32 biglength;               // extender
  const uinT8 *lengths;          // current lengths
  const uinT16 *codes;           // current codes
  uinT16 codeword;               // current code word
  IMAGELINE imageline;           // current line
  IMAGE image;                   // dummy image
  R_BITSTREAM bits;              // read bitstream
  uinT8 colour;                  // current colour

  image.capture(pixels, xsize, ysize, bpp);
  codeword = bits.open(fd);      // open bitstream
  read_eol(&bits, codeword);     // find end of line
  for (yindex = ysize - 1; yindex >= 0; yindex--) {
    imageline.init();
    colour = TRUE;
    for (xindex = 0; xindex < xsize;) {
      if (colour) {
        lengths = long_white_lengths;
        codes = long_white_codes;
      }
      else {
        lengths = long_black_lengths;
        codes = long_black_codes;
      }
      for (biglength = 0; biglength < LONG_CODE_SIZE
        && (codeword & bits.masks (*lengths))
        != *codes; codes++, lengths++, biglength++);
      if (biglength < LONG_CODE_SIZE) {
        codeword = bits.read_code (*lengths);
        biglength++;
        biglength *= SHORT_CODE_SIZE;
      }
      else
        biglength = 0;
      if (colour) {
        lengths = short_white_lengths;
        codes = short_white_codes;
      }
      else {
        lengths = short_black_lengths;
        codes = short_black_codes;
      }
      for (length = 0; length < SHORT_CODE_SIZE
        && (codeword & bits.masks (*lengths))
        != *codes; codes++, lengths++, length++);
      if (length < SHORT_CODE_SIZE) {
        codeword = bits.read_code (*lengths);
        for (length += biglength; length > 0; length--, xindex++)
          imageline.pixels[xindex] = colour;
        colour = !colour;
      }
      else
        break;
    }
    if (xindex < xsize) {
      tprintf (_("%d pixels short on line %d"), xsize - xindex, yindex);
      tprintf (_(", unknown code=%x\n"), codeword);
    }
    xindex = read_eol (&bits, codeword);
    if (xindex > 0)
      tprintf (_("Discarding %d bits on line %d\n"), xindex, yindex);
    image.put_line (0, yindex, xsize, &imageline, 0);
  }
  return 0;
}


/**********************************************************************
 * read_eol
 *
 * Take bits out of the stream until and end-of-line code is hit.
 **********************************************************************/

inT32 read_eol(                    //read end of line
               R_BITSTREAM *bits,  //bitstream to read
               uinT16 &code        //current code
              ) {
  BOOL8 anyones;                 //any 1 bits skipped
  inT32 bitcount;                //total bits skipped

  anyones = FALSE;
  bitcount = 0;
  while ((code & EOL_MASK) != EOL_CODE) {
    if (code & 1)
      anyones = TRUE;            //discarded one bit
    bitcount++;                  //total discarded bits
    code = bits->read_code (1);  //take single bits
  }
                                 //extract EOL code
  code = bits->read_code (EOL_LENGTH);

  if (!anyones)
    bitcount = 0;                //ignore filler bits
  return bitcount;
}


/**********************************************************************
 * write_moto_tif
 *
 * Write a whole tif format image and close the file.
 **********************************************************************/

inT8 write_moto_tif(                //write whole image
                    int fd,         //file to write on
                    uinT8 *pixels,  //image pixels
                    inT32 xsize,    //size of image
                    inT32 ysize,
                    inT8 bpp,       //bits per pixel
                    inT8 photo,
                    inT32 res       //resolution
                   ) {
  return write_tif_image (fd, pixels, xsize, ysize, bpp, res, MOTO, photo);
  //use moto format
}


/**********************************************************************
 * write_intel_tif
 *
 * Write a whole tif format image and close the file.
 **********************************************************************/

inT8 write_intel_tif(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT8 photo,
                     inT32 res       //resolution
                    ) {
  return write_tif_image (fd, pixels, xsize, ysize, bpp, res, INTEL, photo);
  //use intel format
}


/**********************************************************************
 * write_inverse_tif
 *
 * Write a whole tif format image and close the file.
 **********************************************************************/

inT8 write_inverse_tif(                //write whole image
                       int fd,         //file to write on
                       uinT8 *pixels,  //image pixels
                       inT32 xsize,    //size of image
                       inT32 ysize,
                       inT8 bpp,       //bits per pixel
                       inT8 photo,
                       inT32 res       //resolution
                      ) {
  return write_tif_image (fd, pixels, xsize, ysize, bpp, res, INTEL,
    1 - photo);
  //use intel format
}


/**********************************************************************
 * write_tif_image
 *
 * Write a whole tif format image and close the file.
 **********************************************************************/

inT8 write_tif_image(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT32 res,      //resolution
                     inT16 type,     //format type
                     inT16 photo     //metric interp
                    ) {
  inT32 size;                    //line/image size
  inT16 entries;                 //no of tiff entries
  inT32 start;                   //start of tag table
  inT32 zero = 0;
  MYRATIONAL resolution;         //resolution
  TIFFENTRY entry;               //current entry

  TIFFENTRY tags[ENTRIES] = {
    {0xfe, 4, 1, 0},
    {0x100, 3, 1, 0},
    {0x101, 3, 1, 0},
    {0x102, 3, 1, 0},
    {0x103, 3, 1, 1},
    {0x106, 3, 1, 1},
    {                            /*line art */
      0x107, 3, 1, 1
    },
    {0x10a, 3, 1, 1},
    {
      0x111, 4, 1, START + ENTRIES * sizeof (TIFFENTRY)
      + sizeof (inT32) + sizeof (short) + sizeof (MYRATIONAL) * 2
    }
    ,
    {0x112, 3, 1, 1}
    ,
    {0x115, 3, 1, 1}
    ,
    {0x116, 4, 1, 0}
    ,
    {0x117, 4, 1, 0}
    ,
    {0x118, 3, 1, 0}
    ,
    {0x119, 3, 1, 1}
    ,
    {
      0x11a, 5, 1, START + ENTRIES * sizeof (TIFFENTRY)
      + sizeof (inT32) + sizeof (short)
    },
    {
      0x11b, 5, 1, START + ENTRIES * sizeof (TIFFENTRY)
      + sizeof (inT32) + sizeof (short) + sizeof (MYRATIONAL)
    }
    ,
    {0x11c, 3, 1, 1}
    ,
    {0x128, 3, 1, 2}
  };

  resolution.top = res;
  resolution.bottom = 1;
  if (write (fd, (char *) &type, sizeof type) != sizeof type
  || (type != INTEL && type != MOTO)) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Filetype");
    return -1;
  }
  start = START;
  entries = 0x002a;
  if (type != __NATIVE__)
    entries = reverse16 (entries);
  if (write (fd, (char *) &entries, sizeof entries) != sizeof entries) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Version");
    return -1;
  }
  if (type != __NATIVE__)
    start = reverse32 (start);
  if (write (fd, (char *) &start, sizeof start) != sizeof start) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Start");
    return -1;
  }
  lseek (fd, (long) START, 0);
  entries = ENTRIES;
  if (type != __NATIVE__)
    entries = reverse16 (entries);
  if (write (fd, (char *) &entries, sizeof entries) != sizeof entries) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Entries");
    return -1;
  }
                                 //line length
  size = COMPUTE_IMAGE_XDIM (xsize, bpp);
  size *= ysize;                 //total image size
  tags[1].value = xsize;
  tags[2].value = ysize;
  if (bpp == 24) {
    tags[3].value = 8;
    tags[10].value = 3;
    tags[5].value = 2;
  }
  else {
    tags[3].value = bpp;
    tags[5].value = photo;
  }
  tags[11].value = ysize;
  tags[14].value = (1 << bpp) - 1;
  tags[12].value = size;
  for (entries = 0; entries < ENTRIES; entries++) {
    entry = tags[entries];       //get an entry
    /* NB Convert entry.value BEFORE converting entry.type!!! */
    if (entry.type != 3) {       //Full 32bit value
      if (type != __NATIVE__)
        entry.value = reverse32 (entry.value);
    }
    else {
      /* A 16bit value in 4 bytes - handle with care. SEE NOTE at start of file */
      entry.value &= 0x0000ffff; //Ensure top 2 MSBytes clear
      if (__NATIVE__ == MOTO) {
        if (type == MOTO)        //MOTO file on MOTO Machine
          entry.value = entry.value << 16;
        else                     //INTEL file on MOTO Machine
          entry.value = reverse32 (entry.value);
      }
      else {                     //INTEL Machine
        if (type == MOTO)        //MOTO file on INTEL Machine
          entry.value = reverse16 ((uinT16) entry.value);
        //INTEL file on INTEL Machine NO ACTION NEEDED
      }
    }
    if (type != __NATIVE__) {
      entry.tag = reverse16 (entry.tag);
      entry.type = reverse16 (entry.type);
      entry.length = reverse32 (entry.length);
    }
    if (write (fd, (char *) &entry, sizeof (TIFFENTRY)) !=
    sizeof (TIFFENTRY)) {
      WRITEFAILED.error ("write_tif_image", TESSLOG, "Tag Table");
      return -1;
    }
  }
  if (write (fd, (char *) &zero, sizeof zero) != sizeof zero) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Tag table Terminator");
    return -1;
  }
  if (type != __NATIVE__) {
    resolution.top = reverse32 (resolution.top);
    resolution.bottom = reverse32 (resolution.bottom);
  }
  if (write (fd, (char *) &resolution, sizeof resolution) != sizeof resolution
    || write (fd, (char *) &resolution,
  sizeof resolution) != sizeof resolution) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Resolution");
    return -1;
  }
  if (write (fd, (char *) pixels, (size_t) size) != size) {
    WRITEFAILED.error ("write_tif_image", TESSLOG, "Image");
    return -1;
  }
  close(fd);
  return 0;
}
