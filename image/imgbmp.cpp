/**********************************************************************
 * File:			imgbmp.c  (Formerly lz.c)
 * Description:	bmp image reader/writer.
 * Author:		Ray Smith
 * Created:		Tue Jan 06 15:31:25 GMT 1998
 *
 * (C) Copyright 1998, Ray Smith.
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
#include          <stdio.h>
#include          "img.h"
#include          "imgbmp.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

typedef struct
{                                // bmfh
  char bfType1;                  //'B'
  char bfType2;                  //'M'
} BMPHEADER0;
typedef struct
{                                // bmfh
  uinT32 bfSize;                 //filesize
  uinT16 bfReserved1;            //zero
  uinT16 bfReserved2;            //zero
  uinT32 bfOffBits;              //offset to bitmap
} BMPHEADER;

typedef struct
{                                // bmih
  uinT32 biSize;                 //size of struct
  inT32 biWidth;                 //image width
  inT32 biHeight;                //image height
  uinT16 biPlanes;               //1
  uinT16 biBitCount;             //bpp
  uinT32 biCompression;          //0 for uncompressed
  uinT32 biSizeImage;            //image size
  inT32 biXPelsPerMeter;         //res in pp metre
  inT32 biYPelsPerMeter;
  uinT32 biClrUsed;              //0 or actual size of colour table
  uinT32 biClrImportant;         //usually 0
} BMPHEADER2;

typedef struct
{                                // rgbq
  uinT8 rgbBlue;
  uinT8 rgbGreen;
  uinT8 rgbRed;
  uinT8 rgbReserved;             //0
} WIN32_RGBQUAD;

/**
 * @name open_bmp_image
 *
 * Read the header of a bmp format image and prepare to read the rest.
 */

inT8 open_bmp_image(               //read header
                    int fd,        //file to read
                    inT32 *xsize,  //size of image
                    inT32 *ysize,
                    inT8 *bpp,     //bits per pixel
                    inT8 *photo,
                    inT32 *res     //resolution
                   ) {
  uinT32 nread;                  //current bits
  BMPHEADER0 head0;              //first part of header
  BMPHEADER head1;               //first part of header
  BMPHEADER2 head2;              //first part of header

  *photo = 1;
  nread = read (fd, &head0, sizeof (head0));
  if (nread != sizeof (head0))
    return -1;
  nread = read (fd, &head1, sizeof (head1));
  if (nread != sizeof (head1))
    return -1;
  nread = read (fd, &head2, sizeof (head2));
  if (nread != sizeof (head2))
    return -1;

  if (head0.bfType1 != 'B')
    return -1;
  if (head0.bfType2 != 'M')
    return -1;
  lseek (fd, head1.bfOffBits, SEEK_SET);
  *bpp = head2.biBitCount;
  *xsize = head2.biWidth;
  *ysize = head2.biHeight;
  *res = 300;                    //make up resolution
  return -2;                     //success
}


/**
 * @name read_bmp_image
 *
 * Read a whole lz format image and close the file.
 */

inT8 read_bmp_image(                //read header
                    int fd,         //file to read
                    uinT8 *pixels,  //pixels of image
                    inT32 xsize,    //size of image
                    inT32 ysize,
                    inT8 bpp,       //bits per pixel
                    inT32           //bytes per line
                   ) {
  uinT32 bpl;                    //bytes per line
  uinT32 wpl;                    //words per line
  uinT32 nread;                  //current bits
  inT32 index;                   //to cols

  bpl = (xsize * bpp + 7) / 8;   //bytes per line
  wpl = (bpl + 3) / 4;
  wpl *= 4;
  for (index = 0; index < ysize; index++) {
    nread = read (fd, pixels + bpl * (ysize - 1 - index), bpl);
    if (nread != bpl)
      return -1;
    if (wpl != bpl)
      lseek (fd, wpl - bpl, SEEK_CUR);
  }
  return 0;
}


/**
 * @name write_bmp_image
 *
 * Write a whole lz format image and close the file.
 */

inT8 write_bmp_image(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT8,
                     inT32 res       //resolution
                    ) {
  uinT32 bpl;                    //bytes per line
  uinT32 wpl;                    //words per line
  uinT32 nread;                  //current bits
  inT32 cols;                    //entries in table
  inT32 index;                   //to cols
  BMPHEADER0 head0;              //first part of header
  BMPHEADER head1;               //first part of header
  BMPHEADER2 head2;              //first part of header
  WIN32_RGBQUAD coltab[256];     //colour table

  if (bpp == 24)
    cols = 0;
  else
    cols = 1 << bpp;             //size of colour table
  bpl = (xsize * bpp + 7) / 8;   //bytes per line
  wpl = (bpl + 3) / 4;

  head2.biSize = sizeof (head2); //size of struct
  head2.biWidth = xsize;         //image width
  head2.biHeight = ysize;        //image height
  head2.biPlanes = 1;            //1
  head2.biBitCount = bpp;        //bpp
  head2.biCompression = 0;       //0 for uncompressed
                                 //image size
  head2.biSizeImage = wpl * 4 * ysize;
                                 //res in pp metre
  head2.biXPelsPerMeter = (uinT32) (res * 39.37);
  head2.biYPelsPerMeter = (uinT32) (res * 39.37);
  head2.biClrUsed = cols;        //0 or actual size of colour table
  head2.biClrImportant = 0;      //usually 0

  head0.bfType1 = 'B';
  head0.bfType2 = 'M';
  head1.bfReserved1 = 0;         //zero
  head1.bfReserved2 = 0;         //zero
                                 //offset to bitmap
  head1.bfOffBits = sizeof (head0) + sizeof (head1) + sizeof (head2) + sizeof (WIN32_RGBQUAD) * cols;
                                 //filesize
  head1.bfSize = head1.bfOffBits + head2.biSizeImage;

  for (index = 0; index < cols; index++) {
    coltab[index].rgbBlue = index * 255 / (cols - 1);
    coltab[index].rgbGreen = coltab[index].rgbBlue;
    coltab[index].rgbRed = coltab[index].rgbBlue;
    coltab[index].rgbReserved = 0;
  }

  nread = write (fd, &head0, sizeof (head0));
  if (nread != sizeof (head0))
    return -1;
  nread = write (fd, &head1, sizeof (head1));
  if (nread != sizeof (head1))
    return -1;
  nread = write (fd, &head2, sizeof (head2));
  if (nread != sizeof (head2))
    return -1;
  nread = write (fd, coltab, cols * sizeof (WIN32_RGBQUAD));
  if (nread != cols * sizeof (WIN32_RGBQUAD))
    return -1;
  for (index = 0; index < ysize; index++) {
    nread = write (fd, pixels + bpl * (ysize - 1 - index), wpl * 4);
    if (nread != wpl * 4)
      return -1;
  }
  close(fd);  //done it
  return 0;
}
