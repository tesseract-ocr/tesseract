/**********************************************************************
 * File:        imgio.c  (Formerly imageio.c)
 * Description: Controls image input/output and selection of format.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 11:47:26 BST 1990
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
#include          <fcntl.h>
#include          <sys/types.h>
#include          <sys/stat.h>
#include          <string.h>
#include          "scanutils.h"
#include          "stderr.h"
#include          "fileerr.h"
#include          "imgerrs.h"
#include          "memry.h"
#include          "imgs.h"
#include          "imgbmp.h"
#include          "imgtiff.h"
#include          "imgio.h"

#define DEFAULTIMAGETYPE  "tif"  //default to im files

typedef struct
{
  const char *string;            //extension
  IMAGE_OPENER opener;           //opening function
  IMAGE_READER reader;           //reading function
  IMAGE_WRITER writer;           //writing function
} IMAGETYPE;                     //image type record

static IMAGETYPE imagetypes[] = { {
    "TIF",
    open_tif_image,
    read_tif_image,
    write_moto_tif
  },
  {
    "itf",
    open_tif_image,
    read_tif_image,
    write_inverse_tif
  },
  {
    "tif",
    open_tif_image,
    read_tif_image,
    write_intel_tif
  },
  {
    "TIFF",
    open_tif_image,
    read_tif_image,
    write_moto_tif
  },
  {
    "tiff",
    open_tif_image,
    read_tif_image,
    write_intel_tif
  },
  {
    "bmp",
    open_bmp_image,
    read_bmp_image,
    write_bmp_image
  },
  {
    "BMP",
    open_bmp_image,
    read_bmp_image,
    write_bmp_image
  },
};                               //image readers/writers

#define MAXIMAGETYPES   (sizeof(imagetypes)/sizeof(IMAGETYPE))

/**********************************************************************
 * name_to_image_type
 *
 * Convert a file name to an image type, picking defaults if it is
 * has no extension, and complaining if the extension is not supported.
 **********************************************************************/

static inT8 name_to_image_type(                  //get image type
                               const char *name  //name of image
                              ) {
  const char *nametype;          //type part of name
  inT8 type;                     //imagetypes index

  nametype = strrchr (name, '.');//find extension
  if (nametype != NULL)
    nametype++;                  //ptr to extension
  else
    nametype = DEFAULTIMAGETYPE; //had none

                                 //find type of image
  for (type = 0; type < MAXIMAGETYPES && strcmp (imagetypes[type].string, nametype); type++);
  if (type >= MAXIMAGETYPES) {
                                 //unrecognized type
    BADIMAGETYPE.error ("name_to_image_type", TESSLOG, name);
    return -1;
  }
  return type;
}


/**********************************************************************
 * read_header
 *
 * Read the header of an image, typed according to the extension of
 * the name.  Return is 0 for success, -1 for failure.
 **********************************************************************/

inT8 IMAGE::read_header(                  //get file header
                        const char *name  //name of image
                       ) {
  inT8 type;                     //image type

  destroy();  //destroy old image
                                 //get type
  type = name_to_image_type (name);
  if (type < 0 || imagetypes[type].opener == NULL) {
    CANTREADIMAGETYPE.error ("IMAGE::read_header", TESSLOG, name);
    return -1;                   //read not supported
  }
  #ifdef __UNIX__
  if ((fd = open (name, O_RDONLY)) < 0)
  #endif
  #if defined (__MSW32__) || defined (__MAC__)
    if ((fd = open (name, O_RDONLY | O_BINARY)) < 0)
  #endif
  {
    CANTOPENFILE.error ("IMAGE::read_header", TESSLOG, name);
    return -1;                   //failed
  }
  lineskip =
    (*imagetypes[type].opener) (fd, &xsize, &ysize, &bpp, &photo_interp,
    &res);
  if (lineskip == -1) {
                                 //get header
    bpp = 0;                     //still empty
    close(fd);
    fd = -1;
    return -1;                   //failed
  }
  if (res <= 0)
    res = image_default_resolution;
  //      fprintf(stderr,"Image size=(%d,%d), bpp=%d\n",
  //              xsize,ysize,bpp);
                                 //bytes per line
  xdim = COMPUTE_IMAGE_XDIM (xsize, bpp);
  bps = bpp == 24 ? 8 : bpp;
  bytespp = (bpp + 7) / 8;
                                 //funtion to read with
  reader = imagetypes[type].reader;
  return 0;                      //success
}


/**********************************************************************
 * read
 *
 * Read a previously opened image file into memory.
 * If buflines is 0, the whole image is read in one go.
 * If buflines>0, memory space is reserved for reading just that many
 * lines at once.
 * As soon as a request is made to get a line past the end of the buffer,
 * the buffer is re-read with a 50% overlap.
 * Backward seeks are not allowed.
 * Read returns -1 in case of failure or 0 if successful.
 **********************************************************************/

inT8 IMAGE::read(                //get rest of image
                 inT32 buflines  //size of buffer
                ) {
  inT32 row;                     //image row
  BOOL8 failed;                  //read failed

  if (fd < 0 || image != NULL)
    IMAGEUNDEFINED.error ("IMAGE::read", ABORT, NULL);

  if (buflines <= 0 || buflines > ysize || reader == NULL)
    buflines = ysize;            //default to all
  bufheight = buflines;
  image =
    (uinT8 *) alloc_big_mem ((size_t) (xdim * bufheight * sizeof (uinT8)));
  if (image == NULL) {
    MEMORY_OUT.error ("IMAGE::read", TESSLOG, NULL);
    destroy();
    return -1;
  }
  captured = FALSE;
  ymax = ysize;
  ymin = ysize - buflines;       //amount of image read
  if (reader != NULL && lineskip < 0)
    failed = (*reader) (fd, image, xsize, ysize, bpp, xdim) < 0;
  else {
    if (lineskip == 0)
      failed =::read (fd, (char *) image,
        (size_t) (xdim * bufheight)) != xdim * bufheight;
    else {
      for (failed = FALSE, row = 0; row < bufheight && !failed; row++) {
        failed =::read (fd, (char *) image + row * xdim,
          (size_t) xdim) != xdim;
        failed |= lseek (fd, lineskip, SEEK_CUR) < 0;
      }
    }
  }
  if (failed) {
    READFAILED.error ("IMAGE::read", TESSLOG, NULL);
    destroy();
    return -1;                   //read failed
  }
  if (ymin <= 0) {
    close(fd);  //finished reading
    fd = -1;                     //not open now
  }
  return 0;                      //success
}


/**********************************************************************
 * bufread
 *
 * Read a bit more of an image into the buffer.
 **********************************************************************/

inT8 IMAGE::bufread(         //read more into buffer
                    inT32 y  //required coord
                   ) {
  inT32 readtop;                 //no of lines copied
  inT32 linestoread;             //no of lines to read
  inT32 row;                     //row to read
  BOOL8 failed;                  //read failed

                                 //copy needed?
  if (y + bufheight / 2 >= ymin) {
                                 //no of lines to move
    readtop = y + bufheight / 2 - ymin + 1;
                                 //copy inside it
    copy_sub_image (this, 0, ymin, xsize, readtop, this, 0, ymax - readtop, TRUE);
  }
  else
    readtop = 0;
  ymax = y + bufheight / 2;      //new top of image
  ymin = ymax - bufheight;       //possible bottom
  if (ymin < 0)
    ymin = 0;                    //clip to image size
  linestoread = ymax - ymin - readtop;
  if (lineskip == 0)
    failed =::read (fd, (char *) (image + xdim * readtop),
      (size_t) (xdim * linestoread)) != xdim * linestoread;
  else {
    for (failed = FALSE, row = 0; row < linestoread && !failed; row++) {
      failed =::read (fd, (char *) (image + (readtop + row) * xdim),
        (size_t) xdim) != xdim;
      failed |= lseek (fd, lineskip, SEEK_CUR) < 0;
    }
  }
  if (failed) {
    READFAILED.error ("IMAGE::bufread", TESSLOG, NULL);
    return -1;                   //read failed
  }
  if (ymin <= 0) {
    close(fd);  //finished reading
    fd = -1;                     //not open now
  }
  return 0;                      //success
}


/**********************************************************************
 * write
 *
 * Write an image to a file in a format determined by the name.
 **********************************************************************/

inT8 IMAGE::write(                  //write image
                  const char *name  //name to write
                 ) {
  inT8 type;                     //type of image

  if (bpp == 0 || image == NULL || bufheight != ysize)
    IMAGEUNDEFINED.error ("IMAGE::write", ABORT, NULL);
  if (fd >= 0) {
    close(fd);  //close old file
    fd = -1;                     //no longer open
  }
                                 //get image type
  type = name_to_image_type (name);
  if (type < 0 || imagetypes[type].writer == NULL) {
    CANTWRITEIMAGETYPE.error ("IMAGE::write", TESSLOG, name);
    return -1;                   //write not supported
  }
  #ifdef __UNIX__
  if ((fd = creat (name, 0666)) < 0)
  #endif
  #ifdef __MSW32__
    if ((fd = open (name, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IWRITE)) < 0)
  #endif
  #ifdef __MAC__
      if ((fd = creat (name, O_WRONLY | O_BINARY)) < 0)
  #endif
  {
    CANTCREATEFILE.error ("IMAGE::write", TESSLOG, name);
    return -1;                   //failed
  }
  if (res <= 0)
    res = image_default_resolution;
  if ((*imagetypes[type].writer) (fd, image, xsize, ysize, bpp, photo_interp,
  res) < 0) {
                                 //get header
                                 //write failed
    WRITEFAILED.error ("IMAGE::write", TESSLOG, name);
    close(fd);
    fd = -1;
    return -1;                   //failed
  }
  return 0;                      //success
}
